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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgDB/ReadFile"
#include "osgEarth/ImageLayer"
#include "osgEarth/Random"
#include "osgEarth/VirtualProgram"
#include "simUtil/Shaders.h"
#include "simUtil/VelocityParticleLayer.h"

// Register with the osgEarth loader so we can load velocity particle layers from .earth files
REGISTER_OSGEARTH_LAYER(velocityparticleimage, simUtil::VelocityParticleLayer);

namespace simUtil {

/** Image width and height for the RTT that takes velocity output and generates a texture */
static const unsigned int RTT_OUTPUT_IMAGE_WIDTH = 4096;
static const unsigned int RTT_OUTPUT_IMAGE_HEIGHT = 2048;

osg::Node* makeQuad(int width, int height, const osg::Vec4& color)
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

  osg::Geode* geode = new osg::Geode;
  geode->addDrawable(geometry);
  geode->setCullingActive(false);
  return geode;
}

osg::Geometry* createRTTInstancedGeometry(int nInstances, unsigned int particleDimension)
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
  shaderPackage.load(vp, shaderPackage.velocityParticleLayerRttParticleVertex());
  shaderPackage.load(vp, shaderPackage.velocityParticleLayerParticleFragment());
  geom->getOrCreateStateSet()->setDefine("RTT_OUTPUT_IMAGE_WIDTH", std::to_string(RTT_OUTPUT_IMAGE_WIDTH));
  geom->getOrCreateStateSet()->setDefine("RTT_OUTPUT_IMAGE_HEIGHT", std::to_string(RTT_OUTPUT_IMAGE_HEIGHT));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("positionSampler", 0));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("directionSampler", 1));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSprite", 2));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSize", 1.0f));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("resolution", osg::Vec2f(particleDimension, particleDimension)));
  geom->getOrCreateStateSet()->setMode(GL_PROGRAM_POINT_SIZE, 1);

  return geom;
}

osg::Geometry* createInstancedGeometry(int nInstances, unsigned int particleDimension)
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
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("positionSampler", 0));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("directionSampler", 1));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSprite", 2));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("pointSize", 1.0f));
  geom->getOrCreateStateSet()->addUniform(new osg::Uniform("resolution", osg::Vec2f(particleDimension, particleDimension)));
  geom->getOrCreateStateSet()->setMode(GL_PROGRAM_POINT_SIZE, 1);
  geom->setCullingActive(false);

  return geom;
}

osg::Texture2D* createPositionTexture(unsigned int particleDimension)
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
    // No life left, regenerate in shader immediately
    *ptr = -1.f;
  }

  osg::Texture2D* tex = new osg::Texture2D(positionImage);
  tex->setResizeNonPowerOfTwoHint(false);
  tex->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
  tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
  tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
  return tex;
}

osg::Texture2D* createDirectionTexture(unsigned int particleDimension)
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

// Helper node that will run a fragment shader, taking one texture as input and writing to another texture.
// And then it flips on each frame to use the previous input.
class ComputeNode : public osg::Group
{
public:
  ComputeNode(osg::Texture2D* velocityTexture, unsigned int particleDimension) :
    velocityTexture_(velocityTexture),
    particleDimension_(particleDimension),
    needDirection_(false)
  {
    const Shaders shaderPackage;
    // Cache the source code for the shaders so that swapping doesn't require us to re-search for files
    vertexSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputeVertex(), shaderPackage);
    positionFragmentSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputePositionFragment(), shaderPackage);
    directionFragmentSource_ = osgEarth::ShaderLoader::load(shaderPackage.velocityParticleLayerComputeDirectionFragment(), shaderPackage);

    setName("Compute Node");
    inputPosition_ = createPositionTexture(particleDimension_);
    outputPosition_ = createPositionTexture(particleDimension_);
    inputDirection_ = createDirectionTexture(particleDimension_);
    outputDirection_ = createDirectionTexture(particleDimension_);

    buildCamera_();
  }

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

    ss->setTextureAttributeAndModes(0, inputPosition_.get(), osg::StateAttribute::ON);
    ss->setTextureAttributeAndModes(1, velocityTexture_.get(), osg::StateAttribute::ON);
    // Significant banding occurs with GL_BLEND on
    ss->setMode(GL_BLEND, osg::StateAttribute::OFF);

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
    quad_ = makeQuad(particleDimension_, particleDimension_, osg::Vec4(1, 1, 1, 1));
    quad_->setCullingActive(false);
    quad_->setStateSet(createStateSet_(fragShader));
    camera->addChild(quad_);

    return camera;
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

  VelocityTextureNode(osg::Texture2D* velocityTexture, bool includeRtt, unsigned int particleDimension)
    : minColor_(0.0, 1.0, 0.0, 1.0), // green
    maxColor_(1.0, 0.0, 0.0, 1.0), // red: higher velocities are more dangerous
    altitude_(9000.0f),
    boundingBox_(0.f, 0.f, 1.f, 1.f),
    particleDimension_(particleDimension)
  {
    setName("Velocity Texture Node");
    computeNode_ = new ComputeNode(velocityTexture, particleDimension);
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("dieSpeed", 10.0f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("speedFactor", 1.f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("dropChance", 0.0f));
    computeNode_->getOrCreateStateSet()->addUniform(new osg::Uniform("boundingBox", boundingBox_));

    // Create the render target
    if (includeRtt)
    {
      createRttOutputTexture_();
      createRttCamera_();
      addChild(rttCamera_.get());
    }
    createPointsNode_();

    addChild(computeNode_.get());
    addChild(pointsNode_.get());

    getOrCreateStateSet()->addUniform(new osg::Uniform("minColor", minColor_));
    getOrCreateStateSet()->addUniform(new osg::Uniform("maxColor", maxColor_));
    getOrCreateStateSet()->addUniform(new osg::Uniform("altitude", altitude_));
    // For convenience and likeness of purpose, we reuse the osgEarth uniform name for opacity; it does not carry forward
    // because this node is not under the VisibleLayer in the node tree, so we need our own.
    getOrCreateStateSet()->addUniform(new osg::Uniform("oe_VisibleLayer_opacityUniform", 1.f));

    setCullingActive(false);
    addUpdateCallback(new SwapCallback(this));
  }

  void setPointSprite(osg::Texture2D* pointSprite)
  {
    pointSpriteTexture_ = pointSprite;
    if (points_.valid())
      points_->getOrCreateStateSet()->setTextureAttributeAndModes(2, pointSpriteTexture_, osg::StateAttribute::ON);
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
    if (points_.valid())
      points_->getOrCreateStateSet()->getUniform("pointSize")->set(value);
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

  const osg::Vec4& getMinColor() const
  {
    return minColor_;
  }

  void setMinColor(const osg::Vec4& minColor)
  {
    if (minColor_ != minColor)
    {
      minColor_ = minColor;
      getOrCreateStateSet()->getUniform("minColor")->set(minColor_);
    }
  }

  const osg::Vec4& getMaxColor() const
  {
    return maxColor_;
  }

  void setMaxColor(const osg::Vec4& maxColor)
  {
    if (maxColor_ != maxColor)
    {
      maxColor_ = maxColor;
      getOrCreateStateSet()->getUniform("maxColor")->set(maxColor_);
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

  void setOpacity(float value)
  {
    // Note that for convenience and likeness of purpose, we reuse the osgEarth opacity uniform name
    getOrCreateStateSet()->getUniform("oe_VisibleLayer_opacityUniform")->set(value);
  }

  osg::Texture2D* getOutputTexture() const
  {
    return outputTexture_.get();
  }

  void swap()
  {
    computeNode_->swap();
  }

private:
  /** Creates the output texture that gets used for the RTT approach, to show the velocity on a texture. */
  void createRttOutputTexture_()
  {
    outputTexture_ = new osg::Texture2D();
    outputTexture_->setTextureSize(RTT_OUTPUT_IMAGE_WIDTH, RTT_OUTPUT_IMAGE_HEIGHT);
    outputTexture_->setInternalFormat(GL_RGBA8);
    outputTexture_->setSourceFormat(GL_RGBA);
    outputTexture_->setSourceType(GL_UNSIGNED_BYTE);
    outputTexture_->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    outputTexture_->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
  }

  /** Adds the RTT Camera, which renders the output points in the form of pixels.  This is optional and not required for points drawing. */
  void createRttCamera_()
  {
    rttCamera_ = new osg::Camera();
    rttCamera_->setName("RTT Camera Output");
    rttCamera_->setRenderOrder(osg::Camera::PRE_RENDER);
    rttCamera_->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
    rttCamera_->setViewport(0, 0, outputTexture_->getTextureWidth(), outputTexture_->getTextureHeight());
    rttCamera_->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    rttCamera_->setClearColor(osg::Vec4(0, 0, 0, 0));
    rttCamera_->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0), outputTexture_.get());
    rttCamera_->setCullingMode(rttCamera_->getCullingMode() & ~osg::CullSettings::SMALL_FEATURE_CULLING);
    rttCamera_->setProjectionMatrixAsOrtho2D(0.0, RTT_OUTPUT_IMAGE_WIDTH, 0.0, RTT_OUTPUT_IMAGE_HEIGHT);
    rttCamera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

    points_ = createRTTInstancedGeometry(particleDimension_ * particleDimension_, particleDimension_);
    points_->setName("RTT Points Output");
    // Attach the output of the compute node as the texture to feed the positions on the instanced geometry.
    points_->getOrCreateStateSet()->setTextureAttributeAndModes(0, computeNode_->outputPosition(), osg::StateAttribute::ON);
    points_->getOrCreateStateSet()->setTextureAttributeAndModes(1, computeNode_->outputDirection(), osg::StateAttribute::ON);
    points_->getOrCreateStateSet()->setTextureAttributeAndModes(2, pointSpriteTexture_, osg::StateAttribute::ON);
    rttCamera_->addChild(points_.get());
  }

  void createPointsNode_()
  {
    pointsNode_ = createInstancedGeometry(particleDimension_ * particleDimension_, particleDimension_);
    pointsNode_->setName("Instanced Points");
    // Attach the output of the compute node as the texture to feed the positions on the instanced geometry.
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(0, computeNode_->outputPosition(), osg::StateAttribute::ON);
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(1, computeNode_->outputDirection(), osg::StateAttribute::ON);
    pointsNode_->getOrCreateStateSet()->setTextureAttributeAndModes(2, pointSpriteTexture_, osg::StateAttribute::ON);
  }

  osg::ref_ptr<osg::Texture2D> pointSpriteTexture_;
  osg::ref_ptr<osg::Texture2D> outputTexture_;
  osg::ref_ptr<osg::Camera> rttCamera_;
  osg::ref_ptr<osg::Geometry> pointsNode_;

  osg::ref_ptr<ComputeNode> computeNode_;
  osg::ref_ptr<osg::Geometry> points_;
  osg::Vec4 minColor_;
  osg::Vec4 maxColor_;
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
  conf.set("render_rtt_image", _renderRttImage);
  conf.set("min_color", _minColor);
  conf.set("max_color", _maxColor);
  conf.set("sprite_uri", _spriteUri);
  conf.set("uri", _velocityTextureUri);
  if (_boundingBox.isSet())
  {
    osgEarth::Config bbConf;
    bbConf.set("west", _boundingBox->xMin());
    bbConf.set("east", _boundingBox->xMax());
    bbConf.set("south", _boundingBox->yMin());
    bbConf.set("north", _boundingBox->yMax());
    conf.add("bounding_box", bbConf);
  }
  return conf;
}

void VelocityParticleLayer::Options::fromConfig(const osgEarth::Config& conf)
{
  _particleDimension.setDefault(256);
  _dieSpeed.setDefault(10.f);
  _speedFactor.setDefault(1.f);
  _pointSize.setDefault(1.f);
  _dropChance.setDefault(0.f);
  _particleAltitude.setDefault(5000.f);
  _renderRttImage.setDefault(false);
  _minColor.setDefault(osgEarth::Color::Lime);
  _maxColor.setDefault(osgEarth::Color::Red);

  conf.get("particle_dimension", _particleDimension);
  conf.get("die_speed", _dieSpeed);
  conf.get("speed_factor", _speedFactor);
  conf.get("point_size", _pointSize);
  conf.get("drop_chance", _dropChance);
  conf.get("particle_altitude", _particleAltitude);
  conf.get("render_rtt_image", _renderRttImage);
  conf.get("min_color", _minColor);
  conf.get("max_color", _maxColor);
  conf.get("sprite_uri", _spriteUri);
  conf.get("uri", _velocityTextureUri);

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
    _boundingBox->set(west, south, east, north);
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
  setOptionThatRequiresReopen(velocityTexture_, texture);
}

void VelocityParticleLayer::setVelocityTexture(const osgEarth::URI& uri)
{
  setOptionThatRequiresReopen(_options->velocityTextureUri(), uri);
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

const osg::Vec4& VelocityParticleLayer::getMinColor() const
{
  return *_options->minColor();
}

void VelocityParticleLayer::setMinColor(const osg::Vec4& minColor)
{
  _options->minColor() = minColor;
  VPL_SET_NODE(setMinColor, minColor);
}

const osg::Vec4& VelocityParticleLayer::getMaxColor() const
{
  return *_options->maxColor();
}

void VelocityParticleLayer::setMaxColor(const osg::Vec4& maxColor)
{
  _options->maxColor() = maxColor;
  VPL_SET_NODE(setMaxColor, maxColor);
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

bool VelocityParticleLayer::getRenderRttImage() const
{
  return *_options->renderRttImage();
}

void VelocityParticleLayer::setRenderRttImage(bool value)
{
  setOptionThatRequiresReopen(_options->renderRttImage(), value);
}

void VelocityParticleLayer::setOpacity(float value)
{
  ImageLayer::setOpacity(value);
  VPL_SET_NODE(setOpacity, value);
}

osgEarth::Status VelocityParticleLayer::openImplementation()
{
  // Recreate the velocity texture if we have to
  if (_options->velocityTextureUri().isSet())
  {
    osg::ref_ptr<osg::Image> velocityImage = osgDB::readRefImageFile(_options->velocityTextureUri()->full());
    if (velocityImage.valid())
    {
      // Create the texture that will be sent to the velocity layer
      velocityTexture_ = new osg::Texture2D(velocityImage.get());
      velocityTexture_->setResizeNonPowerOfTwoHint(false);
      velocityTexture_->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
      velocityTexture_->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    }
  }
  // Return error if we are not configured with a velocity texture
  if (!velocityTexture_.valid())
    return osgEarth::Status(osgEarth::Status::ResourceUnavailable, "Not configured with a valid velocity texture");

  VelocityTextureNode* velocityNode = new VelocityTextureNode(velocityTexture_.get(), *_options->renderRttImage(), *_options->particleDimension());
  node_ = velocityNode;

  // Set the bounding box on the velocity node, and update data extents
  setProfile(osgEarth::Profile::create("global-geodetic"));
  osgEarth::GeoExtent geoExtent = getProfile()->getExtent();
  if (_options->boundingBox().isSet() && _options->boundingBox()->isValid())
  {
    velocityNode->setBoundingBox(*_options->boundingBox());
    geoExtent = osgEarth::GeoExtent(osgEarth::SpatialReference::get("wgs84"), *_options->boundingBox());
  }
  dataExtents().clear();
  dataExtents().push_back(osgEarth::DataExtent(geoExtent, 0, 0));

  // Create the sprite if needed
  recreatePointSprite_();

  velocityNode->setPointSprite(pointSprite_.get());
  velocityNode->setDieSpeed(*_options->dieSpeed());
  velocityNode->setSpeedFactor(*_options->speedFactor());
  velocityNode->setPointSize(*_options->pointSize());
  velocityNode->setDropChance(*_options->dropChance());
  velocityNode->setParticleAltitude(*_options->particleAltitude());
  velocityNode->setMinColor(*_options->minColor());
  velocityNode->setMaxColor(*_options->maxColor());

  setUseCreateTexture();

  return osgEarth::Status::OK();
}

osgEarth::TextureWindow VelocityParticleLayer::createTexture(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const
{
  // Set the texture matrix corresponding to the tile key:
  osg::Matrixf textureMatrix;
  key.getExtent().createScaleBias(getProfile()->getExtent(), textureMatrix);
  return osgEarth::TextureWindow(getNode_()->getOutputTexture(), textureMatrix);
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
  if (!_options->spriteUri().isSet())
    return;

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

}
