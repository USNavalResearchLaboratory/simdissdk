/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgDB/ReadFile"
#include "osgEarth/ImageLayer"
#include "osgEarth/Random"
#include "osgEarth/Version"
#include "osgEarth/VirtualProgram"
#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simVis/GradientShader.h"
#include "simUtil/Shaders.h"
#include "simUtil/VelocityParticleLayer.h"

// Register with the osgEarth loader so we can load velocity particle layers from .earth files
REGISTER_OSGEARTH_LAYER(velocityparticleimage, simUtil::VelocityParticleLayer);

namespace simUtil {

/** Default color mapping for velocity to color. */
static const std::map<float, osg::Vec4f> DEFAULT_COLOR_MAP = {
  { 0.f, osgEarth::Color::Blue },
  { 8.f, osgEarth::Color::Cyan },
  { 13.f, osgEarth::Color::Lime },
  { 18.f, osgEarth::Color::Yellow },
  { 50.f, osgEarth::Color::Red },
  { 75.f, osgEarth::Color::Purple },
};

// Helper node that will run a fragment shader, taking one texture as input and writing to another texture.
// And then it flips on each frame to use the previous input.
class ComputeNode : public osg::Group
{
public:
  ComputeNode(osg::Texture2D* velocityTexture, unsigned int particleDimension)
    : velocityTexture_(velocityTexture),
    particleDimension_(particleDimension),
    needDirection_(false)
  {
    const Shaders shaderPackage;
    // Cache the source code for the shaders so that swapping doesn't require us to re-search for files
    vertexSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputeVertex(), shaderPackage);
    positionFragmentSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputePositionFragment(), shaderPackage);
    directionFragmentSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputeDirectionFragment(), shaderPackage);

    setName("Compute Node");
    inputPosition_ = createPositionTexture_(particleDimension_);
    outputPosition_ = createPositionTexture_(particleDimension_);
    inputDirection_ = createDirectionTexture_(particleDimension_);
    outputDirection_ = createDirectionTexture_(particleDimension_);

    buildCamera_();
  }

  /** Change the velocity texture, which alters the direction of particles live */
  void setVelocityTexture(osg::Texture2D* velocityTexture)
  {
    velocityTexture_ = velocityTexture;
    // Content will update on the next swap() when buildCamera_() gets called.
  }

  /** Change the texel-to-velocity function.  Default maps R to VX(-25,25), and G to VY (-25,25).  @see simUtil::VelocityParticleLayer::setTexelToVelocityFragment() */
  void setTexelToVelocityFragment(const std::string& glslFragment)
  {
    texelToVelocityFragment_ = glslFragment;
    // Content will update on the next swap() when buildCamera_() gets called.
  }

  /** Output becomes input, input is now the new output.  Rebuilds cameras to reflect new state.  Call once per frame. */
  void swap()
  {
    std::swap(inputPosition_, outputPosition_);
    std::swap(inputDirection_, outputDirection_);
    buildCamera_();
  }

  /** Retrieves the output position texture, which includes the velocity positions. */
  osg::Texture2D* outputPosition() const
  {
    return outputPosition_.get();
  }

  /** 1D texture of the current velocity direction, with 0.0 north, clockwise to 1.0.  e.g. 0.25 is east */
  osg::Texture2D* outputDirection() const
  {
    return outputDirection_.get();
  }

  /** Changes whether we need position, which means adding a second camera and could have performance hits.  Needed to rotate icons. */
  void setNeedDirection(bool needDirection)
  {
    needDirection_ = needDirection;
  }

private:
  osg::StateSet* createStateSet_(const std::string& fragShader) const
  {
    osg::Program* program = new osg::Program;
    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexSource_));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragShader));

    osg::StateSet* ss = new osg::StateSet;
    ss->setAttributeAndModes(program);

    ss->addUniform(new osg::Uniform("texturePosition", 0));
    ss->addUniform(new osg::Uniform("velocityMap", 1));
    ss->addUniform(new osg::Uniform("resolution", osg::Vec2f(particleDimension_, particleDimension_)));
    if (!texelToVelocityFragment_.empty())
      ss->setDefine("TEXEL_TO_VELXY(t)", texelToVelocityFragment_);

    ss->setTextureAttributeAndModes(0, inputPosition_.get(), osg::StateAttribute::ON);
    ss->setTextureAttributeAndModes(1, velocityTexture_.get(), osg::StateAttribute::ON);
    // Significant banding occurs with GL_BLEND on
    ss->setMode(GL_BLEND, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

    return ss;
  }

  /** Creates the RTT camera that renders the velocities. */
  osg::Camera* createRttCamera_(const std::string& fragShader)
  {
    osg::Camera* camera = new osg::Camera;

    camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera->setClearColor(osg::Vec4(1.0, 1.0, 1.0, 1.0f));

    // set view
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    // set viewport
    camera->setViewport(0, 0, particleDimension_, particleDimension_);

    // set the camera to render before the main camera.
    camera->setRenderOrder(osg::Camera::PRE_RENDER);

    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    // set up projection.
    camera->setProjectionMatrixAsOrtho2D(0.0, particleDimension_, 0.0, particleDimension_);
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    // Make a full screen quad
    quad_ = makeQuad_(particleDimension_, particleDimension_, osg::Vec4(1, 1, 1, 1));
    quad_->setCullingActive(false);
    quad_->setStateSet(createStateSet_(fragShader));
    camera->addChild(quad_);

    return camera;
  }

  /** Creates a new quad of the given width and height, using GL_TRIANGLES to render. */
  osg::Node* makeQuad_(int width, int height, const osg::Vec4& color) const
  {
    osg::Geometry* geometry = new osg::Geometry;
    osg::Vec3Array* verts = new osg::Vec3Array();
    verts->push_back(osg::Vec3(0, 0, 0));
    verts->push_back(osg::Vec3(width, 0, 0));
    verts->push_back(osg::Vec3(width, height, 0));
    verts->push_back(osg::Vec3(0, height, 0));
    geometry->setVertexArray(verts);
    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->push_back(color);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    auto de = new osg::DrawElementsUByte(GL_TRIANGLES);
    geometry->addPrimitiveSet(de);
    de->push_back(0); de->push_back(1); de->push_back(2);
    de->push_back(0); de->push_back(2); de->push_back(3);
    geometry->setCullingActive(false);
    return geometry;
  }

  /** Allocates an image and generates the texture for the position, life, and velocity values. */
  osg::Texture2D* createPositionTexture_(unsigned int particleDimension) const
  {
    osg::Image* positionImage = new osg::Image;
    positionImage->allocateImage(particleDimension, particleDimension, 1, GL_RGBA, GL_FLOAT);
    positionImage->setInternalTextureFormat(GL_RGBA32F_ARB);
    GLfloat* ptr = reinterpret_cast<GLfloat*>(positionImage->data());

    for (unsigned int i = 0; i < particleDimension * particleDimension; i++)
    {
      // Start off the map
      *ptr++ = -1.f;
      *ptr++ = -1.f;
      *ptr++ = 0.f;
      // No life left, regenerates in shader immediately
      *ptr++ = -1.f;
    }

    osg::Texture2D* tex = new osg::Texture2D(positionImage);
    tex->setResizeNonPowerOfTwoHint(false);
    tex->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
    tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    return tex;
  }

  /** Creates an output texture for the direction of particles */
  osg::Texture2D* createDirectionTexture_(unsigned int particleDimension) const
  {
    osg::Texture2D* tex = new osg::Texture2D;
    tex->setTextureWidth(particleDimension);
    tex->setTextureHeight(particleDimension);
    tex->setInternalFormat(GL_R16F);
    tex->setResizeNonPowerOfTwoHint(false);
    tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    return tex;
  }

  /** Creates a new camera and attaches its output to the output position texture */
  void buildCamera_()
  {
    if (positionCamera_.valid())
      removeChild(positionCamera_.get());
    positionCamera_ = createRttCamera_(positionFragmentSource_);
    positionCamera_->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), outputPosition_.get());
    positionCamera_->setName("Position RTT Camera");
    addChild(positionCamera_.get());

    // Only add the direction camera if we need direction
    if (directionCamera_.valid())
    {
      removeChild(directionCamera_.get());
      directionCamera_ = nullptr;
    }
    if (needDirection_)
    {
      directionCamera_ = createRttCamera_(directionFragmentSource_);
      directionCamera_->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), outputDirection_.get());
      directionCamera_->setName("Direction RTT Camera");
      addChild(directionCamera_.get());
    }
  }

  osg::ref_ptr<osg::Texture2D> inputPosition_;
  osg::ref_ptr<osg::Texture2D> outputPosition_;
  osg::ref_ptr<osg::Texture2D> inputDirection_;
  osg::ref_ptr<osg::Texture2D> outputDirection_;
  osg::ref_ptr<osg::Camera> positionCamera_;
  osg::ref_ptr<osg::Camera> directionCamera_;
  osg::ref_ptr<osg::Node> quad_;
  osg::ref_ptr<osg::Texture2D> velocityTexture_;

  unsigned int particleDimension_;
  bool needDirection_;

  std::string vertexSource_;
  std::string positionFragmentSource_;
  std::string directionFragmentSource_;
  std::string texelToVelocityFragment_;
};

/** Holds nodes containing the compute node for particle position, the RTT for texture content, and the geometry with texture of particles. */
class VelocityTextureNode : public osg::Group
{
public:
  /** Performs a swap() every update. */
  class SwapCallback : public osg::Callback
  {
  public:
    explicit SwapCallback(VelocityTextureNode* velocityTextureNode)
      : velocityTextureNode_(velocityTextureNode)
    {
    }

    virtual bool run(osg::Object* object, osg::Object* data)
    {
      if (velocityTextureNode_.valid())
        velocityTextureNode_->swap();
      return traverse(object, data);
    }

  private:
    osg::observer_ptr<VelocityTextureNode> velocityTextureNode_;
  };

  VelocityTextureNode(osg::Texture2D* velocityTexture, unsigned int particleDimension)
    : altitude_(9000.0f),
    boundingBox_(0.f, 0.f, 1.f, 1.f),
    particleDimension_(particleDimension)
  {
    setName("Velocity Texture Node");
    computeNode_ = new ComputeNode(velocityTexture, particleDimension);
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("dieSpeed", 10.0f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("speedFactor", 1.f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("dropChance", 0.0f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("boundingBox", boundingBox_));

    // Create the render target (particle node)
    createPointsNode_();

    addChild(computeNode_.get());
    addChild(pointsNode_.get());

    getOrCreateStateSet()->addUniform(new osg::Uniform("altitude", altitude_));

    setCullingActive(false);
    addUpdateCallback(new SwapCallback(this));
  }

  /** Changes the underlying texture for the velocity texture.  Updates in-place, particles start moving in new directions on next frame. */
  void setVelocityTexture(osg::Texture2D* velocityTexture)
  {
    computeNode_->setVelocityTexture(velocityTexture);
  }

  /** Changes the point sprite. Pass in a null sprite to use point particles instead of sprites. */
  void setPointSprite(osg::Texture2D* pointSprite)
  {
    pointSpriteTexture_ = pointSprite;
    if (pointsNode_.valid())
      pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(2, pointSpriteTexture_, osg::StateAttribute::ON);
    getOrCreateStateSet()->addUniform(new osg::Uniform("usePointSprite", pointSpriteTexture_.valid()));
    // Only need direction if we have a point sprite texture
    computeNode_->setNeedDirection(pointSpriteTexture_.valid());
  }

  float getDieSpeed() const
  {
    float value;
    computeNode_->getOrCreateStateSet()->getUniform("dieSpeed")->get(value);
    return value;
  }

  void setDieSpeed(float value)
  {
    computeNode_->getOrCreateStateSet()->getUniform("dieSpeed")->set(value);
  }

  float getSpeedFactor() const
  {
    float value;
    computeNode_->getOrCreateStateSet()->getUniform("speedFactor")->get(value);
    return value;
  }

  void setSpeedFactor(float value)
  {
    computeNode_->getOrCreateStateSet()->getUniform("speedFactor")->set(value);
  }

  float getDropChance() const
  {
    float value;
    computeNode_->getOrCreateStateSet()->getUniform("dropChance")->get(value);
    return value;
  }

  void setDropChance(float value)
  {
    computeNode_->getOrCreateStateSet()->getUniform("dropChance")->set(value);
  }

  float getPointSize() const
  {
    float value;
    pointsNode_->getOrCreateStateSet()->getUniform("pointSize")->get(value);
    return value;
  }

  void setPointSize(float value)
  {
    pointsNode_->getOrCreateStateSet()->getUniform("pointSize")->set(value);
  }

  float getParticleAltitude() const
  {
    return altitude_;
  }

  void setParticleAltitude(float value)
  {
    if (altitude_ != value)
    {
      altitude_ = value;
      getOrCreateStateSet()->getUniform("altitude")->set(altitude_);
    }
  }

  void setBoundingBox(const osgEarth::Bounds& bounds)
  {
    if (!bounds.valid())
    {
      computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("boundingBox", osg::Vec4(0.0, 0.0, 1.0, 1.0)));
      return;
    }
    // Convert from LLA to percentage of earth
    const osg::Vec4 vec(0.5 + bounds.xMin() / 360., 0.5 + bounds.yMin() / 180., 0.5 + bounds.xMax() / 360., 0.5 + bounds.yMax() / 180.);
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("boundingBox", vec));
  }

  void setGradient(const simVis::GradientShader gradient)
  {
    // Need to make a copy in order to get the function name correct
    simVis::GradientShader localGradient(gradient);
    localGradient.setFunctionName("su_vel2color");
    const auto& code = localGradient.buildShader();
    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(pointsNode_->getOrCreateStateSet());
    vp->setShader(localGradient.functionName(), new osg::Shader(osg::Shader::VERTEX, code));
  }

  void setTexelToVelocityFragment(const std::string& glslFragment)
  {
    computeNode_->setTexelToVelocityFragment(glslFragment);
  }

  void swap()
  {
    computeNode_->swap();
  }

private:
  /** Creates the node that holds the points particles */
  void createPointsNode_()
  {
    pointsNode_ = createInstancedGeometry_(particleDimension_ * particleDimension_, particleDimension_);
    pointsNode_->setName("Instanced Points");
    // Attach the output of the compute node as the texture to feed the positions on the instanced geometry.
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(0, computeNode_->outputPosition(), osg::StateAttribute::ON);
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(1, computeNode_->outputDirection(), osg::StateAttribute::ON);
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(2, pointSpriteTexture_, osg::StateAttribute::ON);
  }

  /** Creates a geometry that renders nInstances points, with the particle shader attached. */
  osg::Geometry* createInstancedGeometry_(int nInstances, unsigned int particleDimension) const
  {
    osg::Geometry* geom = new osg::Geometry;
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);

    // Points
    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize(1);
    geom->setVertexArray(v);
    (*v)[0] = osg::Vec3(0.0, 0.0, 0.0);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, 1, nInstances));

    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(geom->getOrCreateStateSet());
    simUtil::Shaders shaderPackage;
    shaderPackage.load(vp, shaderPackage.velocityParticleLayerParticleVertex());
    shaderPackage.load(vp, shaderPackage.velocityParticleLayerParticleFragment());

    // Set up the initial gradient shader
    simVis::GradientShader shader;
    shader.setFunctionName("su_vel2color");
    shader.setColorMap(DEFAULT_COLOR_MAP);
    shader.setDiscrete(false);
    auto code = shader.buildShader();
    vp->setShader(shader.functionName(), new osg::Shader(osg::Shader::VERTEX, code));

    geom->getOrCreateStateSet()->addUniform(new osg::Uniform("positionSampler", 0));
    geom->getOrCreateStateSet()->addUniform(new osg::Uniform("directionSampler", 1));
    geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSprite", 2));
    geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSize", 1.0f));
    geom->getOrCreateStateSet()->addUniform(new osg::Uniform("resolution", osg::Vec2f(particleDimension, particleDimension)));
    geom->getOrCreateStateSet()->setMode(GL_PROGRAM_POINT_SIZE, 1);
    geom->setCullingActive(false);

    return geom;
  }

  osg::ref_ptr<osg::Texture2D> pointSpriteTexture_;
  osg::ref_ptr<osg::Geometry> pointsNode_;

  osg::ref_ptr<ComputeNode> computeNode_;
  float altitude_;
  osg::Vec4 boundingBox_;
  unsigned int particleDimension_;
};

///////////////////////////////////////////////////////////////////////////////////////

osgEarth::Config VelocityParticleLayer::Options::getConfig() const
{
  osgEarth::Config conf = ImageLayer::Options::getConfig();
  conf.set("particle_dimension", _particleDimension);
  conf.set("die_speed", _dieSpeed);
  conf.set("speed_factor", _speedFactor);
  conf.set("point_size", _pointSize);
  conf.set("drop_chance", _dropChance);
  conf.set("particle_altitude", _particleAltitude);
  conf.set("sprite_uri", _spriteUri);
  conf.set("uri", _velocityTextureUri);
  conf.set("texel_to_velocity_fragment", _texelToVelocityFragment);
  if (_boundingBox.isSet())
  {
    osgEarth::Config bbConf;
    bbConf.set("west", _boundingBox->xMin());
    bbConf.set("east", _boundingBox->xMax());
    bbConf.set("south", _boundingBox->yMin());
    bbConf.set("north", _boundingBox->yMax());
    conf.set("bounding_box", bbConf);
  }

  // Save the gradient in a new block
  if (_gradient.isSet())
  {
    osgEarth::Config colorConf;
    colorConf.add("discrete", _gradient->isDiscrete());

    // Colors are listed one per line, <stop> <r> <g> <b> <a>.  Format matches GDAL color ramp files
    std::stringstream ss;
    const auto& colorMap = _gradient->colorMap();
    bool first = true;
    for (const auto& valueColor : colorMap)
    {
      const auto& color = valueColor.second;
      if (!first)
        ss << "\n";
      first = false;
      ss << valueColor.first << " " << osgEarth::vec4fToHtmlColor(valueColor.second);
    }
    colorConf.add("colors", ss.str());
    conf.set("gradient", colorConf);
  }

  return conf;
}

/** Helper function to decode a single line in the <gradient><colors> tag.  Cannot add as private method to Options. */
int decodeColorLine(const std::string& line, float& value, osg::Vec4f& color)
{
  std::stringstream ss(simCore::StringUtils::trim(line));
  if (!(ss >> value))
    return 1;
  std::string colorString;
  if (!(ss >> colorString))
    return 1;
  color = osgEarth::htmlColorToVec4f(colorString);
  return 0;
}

void VelocityParticleLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  _particleDimension.setDefault(256);
  _dieSpeed.setDefault(10.f);
  _speedFactor.setDefault(1.f);
  _pointSize.setDefault(1.f);
  _dropChance.setDefault(0.f);
  _particleAltitude.setDefault(5000.f);
  simVis::GradientShader gradient;
  gradient.setColorMap(DEFAULT_COLOR_MAP);
  gradient.setDiscrete(false);
  _gradient.setDefault(gradient);

  conf.get("particle_dimension", _particleDimension);
  conf.get("die_speed", _dieSpeed);
  conf.get("speed_factor", _speedFactor);
  conf.get("point_size", _pointSize);
  conf.get("drop_chance", _dropChance);
  conf.get("particle_altitude", _particleAltitude);
  conf.get("sprite_uri", _spriteUri);
  conf.get("uri", _velocityTextureUri);
  conf.get("texel_to_velocity_fragment", _texelToVelocityFragment);

  if (conf.hasChild("bounding_box"))
  {
    double west = -180.0;
    double east = 180.0;
    double south = -90.0;
    double north = 90.0;
    const osgEarth::Config& bbConf = conf.child("bounding_box");
    bbConf.get("west", west);
    bbConf.get("east", east);
    bbConf.get("south", south);
    bbConf.get("north", north);
#if OSGEARTH_SOVERSION >= 138
    _boundingBox->set(west, south, 0.0, east, north, 0.0);
#else
    _boundingBox->set(west, south, east, north);
#endif
  }

  if (conf.hasChild("gradient"))
  {
    const osgEarth::Config& gConf = conf.child("gradient");
    bool discrete = false;
    gConf.get("discrete", discrete);
    std::string colorString;
    gConf.get("colors", colorString);

    // Convert the color string into a map
    simVis::GradientShader::ColorMap colors;
    std::stringstream colorStream(colorString);
    while (colorStream.good())
    {
      std::string line;
      if (std::getline(colorStream, line))
      {
        float value;
        osg::Vec4f color;
        if (decodeColorLine(line, value, color) == 0)
          colors[value] = color;
      }
    }

    simVis::GradientShader gradient;
    gradient.setDiscrete(discrete);
    gradient.setColorMap(colors);
    _gradient = gradient;
  }
}

///////////////////////////////////////////////////////////////////////////////////////

// Macro to help call a function on the node, if node is non-null
#define VPL_SET_NODE(FUNC, VALUE) {auto* node = getNode_(); if (node) node->FUNC(VALUE); }

VelocityParticleLayer::~VelocityParticleLayer()
{
}

void VelocityParticleLayer::setVelocityTexture(osg::Texture2D* texture)
{
  _options->velocityTextureUri().clear();
  if (velocityTexture_ == texture)
    return;

  velocityTexture_ = texture;
  VPL_SET_NODE(setVelocityTexture, velocityTexture_.get());
}

void VelocityParticleLayer::setVelocityTexture(const osgEarth::URI& uri)
{
  if (_options->velocityTextureUri().isSet() && uri == *_options->velocityTextureUri())
    return;
  _options->velocityTextureUri() = uri;
  if (!isOpen())
    return;

  // Attempt to replace the texture live; if the texture does not exist, we must close because we can't draw anything
  if (readAndSetVelocityTexture_() != 0)
  {
    SIM_ERROR << "Setting URI " << uri.full() << " to Velocity Particle Layer failed, file not found.\n";
    close();
  }
}

int VelocityParticleLayer::readAndSetVelocityTexture_()
{
  osg::ref_ptr<osg::Image> velocityImage = osgDB::readRefImageFile(_options->velocityTextureUri()->full());
  if (!velocityImage.valid())
  {
    velocityTexture_ = nullptr;
    return 1;
  }

  // Create the texture that will be sent to the velocity layer
  velocityTexture_ = new osg::Texture2D(velocityImage.get());
  velocityTexture_->setResizeNonPowerOfTwoHint(false);
  velocityTexture_->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
  velocityTexture_->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
  VPL_SET_NODE(setVelocityTexture, velocityTexture_.get());
  return 0;
}

const osgEarth::URI& VelocityParticleLayer::getVelocityTexture() const
{
  return *_options->velocityTextureUri();
}

void VelocityParticleLayer::setPointSprite(osg::Texture2D* texture)
{
  pointSprite_ = texture;
  _options->spriteUri().clear();
  VPL_SET_NODE(setPointSprite, pointSprite_.get());
}

void VelocityParticleLayer::setPointSprite(const osgEarth::URI& uri)
{
  _options->spriteUri() = uri;
  recreatePointSprite_();
  VPL_SET_NODE(setPointSprite, pointSprite_.get());
}

const simVis::GradientShader& VelocityParticleLayer::getGradient() const
{
  return *_options->gradient();
}

void VelocityParticleLayer::setGradient(const simVis::GradientShader& gradient)
{
  _options->gradient() = gradient;
  VPL_SET_NODE(setGradient, gradient);
}

const osgEarth::URI& VelocityParticleLayer::getPointSprite() const
{
  return *_options->spriteUri();
}

osgEarth::Bounds VelocityParticleLayer::getBoundingBox() const
{
  return *_options->boundingBox();
}

void VelocityParticleLayer::setBoundingBox(const osgEarth::Bounds& bounds)
{
  _options->boundingBox() = bounds;
  VPL_SET_NODE(setBoundingBox, bounds);
}

unsigned int VelocityParticleLayer::getParticleDimension() const
{
  return *_options->particleDimension();
}

void VelocityParticleLayer::setParticleDimension(unsigned int value)
{
  setOptionThatRequiresReopen(_options->particleDimension(), value);
}

float VelocityParticleLayer::getDieSpeed() const
{
  return *_options->dieSpeed();
}

void VelocityParticleLayer::setDieSpeed(float value)
{
  _options->dieSpeed() = value;
  VPL_SET_NODE(setDieSpeed, value);
}

float VelocityParticleLayer::getSpeedFactor() const
{
  return *_options->speedFactor();
}

void VelocityParticleLayer::setSpeedFactor(float value)
{
  _options->speedFactor() = value;
  VPL_SET_NODE(setSpeedFactor, value);
}

float VelocityParticleLayer::getPointSize() const
{
  return *_options->pointSize();
}

void VelocityParticleLayer::setPointSize(float value)
{
  _options->pointSize() = value;
  VPL_SET_NODE(setPointSize, value);
}

float VelocityParticleLayer::getDropChance() const
{
  return *_options->dropChance();
}

void VelocityParticleLayer::setDropChance(float value)
{
  _options->dropChance() = value;
  VPL_SET_NODE(setDropChance, value);
}

float VelocityParticleLayer::getParticleAltitude() const
{
  return *_options->particleAltitude();
}

void VelocityParticleLayer::setParticleAltitude(float value)
{
  _options->particleAltitude() = value;
  VPL_SET_NODE(setParticleAltitude, value);
}

std::string VelocityParticleLayer::getTexelToVelocityFragment() const
{
  return *_options->texelToVelocityFragment();
}

void VelocityParticleLayer::setTexelToVelocityFragment(const std::string& glslFragment)
{
  _options->texelToVelocityFragment() = glslFragment;
  VPL_SET_NODE(setTexelToVelocityFragment, glslFragment);
}

osgEarth::Status VelocityParticleLayer::openImplementation()
{
  // Recreate the velocity texture if we have to; treat empty string for velocity texture same as not-set
  if (_options->velocityTextureUri().isSet() && !_options->velocityTextureUri()->empty())
    readAndSetVelocityTexture_();

  // Return error if we are not configured with a velocity texture
  if (!velocityTexture_.valid())
    return osgEarth::Status(osgEarth::Status::ResourceUnavailable, "Not configured with a valid velocity texture");

  VelocityTextureNode* velocityNode = new VelocityTextureNode(velocityTexture_.get(), *_options->particleDimension());
  node_ = velocityNode;

  // Set the bounding box on the velocity node, and update data extents
  setProfile(osgEarth::Profile::create("global-geodetic"));
  osgEarth::GeoExtent geoExtent = getProfile()->getExtent();
#if OSGEARTH_SOVERSION >= 138
  if (_options->boundingBox().isSet() && _options->boundingBox()->valid())
#else
  if (_options->boundingBox().isSet() && _options->boundingBox()->isValid())
#endif
  {
    velocityNode->setBoundingBox(*_options->boundingBox());
    geoExtent = osgEarth::GeoExtent(osgEarth::SpatialReference::get("wgs84"), *_options->boundingBox());
  }

#if OSGEARTH_SOVERSION >= 142
  osgEarth::DataExtentList allExtents;
  allExtents.push_back(osgEarth::DataExtent(geoExtent, 0, 0));
  setDataExtents(allExtents);
#else
  dataExtents().clear();
  dataExtents().push_back(osgEarth::DataExtent(geoExtent, 0, 0));
#endif

  // Create the sprite if needed
  recreatePointSprite_();

  velocityNode->setPointSprite(pointSprite_.get());
  velocityNode->setDieSpeed(*_options->dieSpeed());
  velocityNode->setSpeedFactor(*_options->speedFactor());
  velocityNode->setPointSize(*_options->pointSize());
  velocityNode->setDropChance(*_options->dropChance());
  velocityNode->setParticleAltitude(*_options->particleAltitude());
  velocityNode->setGradient(*_options->gradient());
  velocityNode->setTexelToVelocityFragment(*_options->texelToVelocityFragment());

  setUseCreateTexture();

  return osgEarth::Status::OK();
}

osg::Node* VelocityParticleLayer::getNode() const
{
  return node_.get();
}

VelocityTextureNode* VelocityParticleLayer::getNode_() const
{
  return static_cast<VelocityTextureNode*>(node_.get());
}

void VelocityParticleLayer::recreatePointSprite_()
{
  if (!_options->spriteUri().isSet() || _options->spriteUri()->empty())
  {
    pointSprite_ = nullptr;
    return;
  }

  osg::ref_ptr<osg::Image> pointSpriteImage = osgDB::readRefImageFile(_options->spriteUri()->full());
  if (pointSpriteImage.valid())
  {
    // Create the texture that will be sent to the velocity layer
    pointSprite_ = new osg::Texture2D(pointSpriteImage.get());
    pointSprite_->setResizeNonPowerOfTwoHint(false);
    pointSprite_->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    pointSprite_->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
  }
  else
    pointSprite_ = nullptr;
}

void VelocityParticleLayer::init()
{
  ImageLayer::init();
  // Avoid crash from invalid texture access
  setRenderType(RENDERTYPE_CUSTOM);
}

}
