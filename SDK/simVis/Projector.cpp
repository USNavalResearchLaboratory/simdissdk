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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Geometry"
#include "osg/ImageStream"
#include "osg/MatrixTransform"
#include "osg/Notify"
#include "osg/CullFace"
#include "osg/PolygonOffset"
#include "osgDB/ReadFile"
#include "osgUtil/CullVisitor"
#include "osgEarth/EllipsoidIntersector"
#include "osgEarth/Horizon"
#include "osgEarth/Shadowing"
#include "osgEarth/NodeUtils"
#include "osgEarth/CameraUtils"
#include "osgEarth/TerrainEngineNode"
#include "osgEarth/LogarithmicDepthBuffer"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/String/Format.h"
#include "simVis/ClockOptions.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Platform.h"
#include "simVis/ProjectorManager.h"
#include "simVis/Registry.h"
#include "simVis/Shaders.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/Projector.h"

namespace
{

static const double DEFAULT_PROJECTOR_FOV_IN_DEG = 45.0;
static const float DEFAULT_ALPHA_VALUE = 0.1f;

  // draws the geometry of the projection frustum.
  // (NOTE: some of this code is borrowed from OSG's osgthirdpersonview example)
  void makeFrustum(const osg::Matrixd& proj, const osg::Matrixd& mv, osg::MatrixTransform* mt)
  {
    osg::ref_ptr<osg::Geometry> geom;
    osg::ref_ptr<osg::Vec3Array> v;

    if (mt->getNumChildren() > 0)
    {
      geom = dynamic_cast<osg::Geometry*>(mt->getChild(0));
      v = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    }
    else
    {
      geom = new osg::Geometry();
      v = new osg::Vec3Array(9);
      v->setDataVariance(osg::Object::DYNAMIC);
      geom->setVertexArray(v.get());
      geom->setDataVariance(osg::Object::DYNAMIC);

      osg::ref_ptr<osg::Vec4Array> c = new osg::Vec4Array(osg::Array::BIND_OVERALL);
      c->push_back(simVis::Color::White);
      geom->setColorArray(c.get());

      GLubyte idxLines[8] = { 0, 5, 0, 6, 0, 7, 0, 8 };
      GLubyte idxLoops0[4] = { 1, 2, 3, 4 };
      GLubyte idxLoops1[4] = { 5, 6, 7, 8 };
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINES, 8, idxLines));
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINE_LOOP, 4, idxLoops0));
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINE_LOOP, 4, idxLoops1));

      simVis::setLighting(geom->getOrCreateStateSet(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

      mt->addChild(geom.get());
    }

    // Get near and far from the Projection matrix.
    const double nz = proj(3, 2) / (proj(2, 2)-1.0);
    const double fz = proj(3, 2) / (1.0+proj(2, 2));

    // Get the sides of the near plane.
    const double nLeft = nz * (proj(2, 0)-1.0) / proj(0, 0);
    const double nRight = nz * (1.0+proj(2, 0)) / proj(0, 0);
    const double nTop = nz * (1.0+proj(2, 1)) / proj(1, 1);
    const double nBottom = nz * (proj(2, 1)-1.0) / proj(1, 1);

    // Get the sides of the far plane.
    const double fLeft = fz * (proj(2, 0)-1.0) / proj(0, 0);
    const double fRight = fz * (1.0+proj(2, 0)) / proj(0, 0);
    const double fTop = fz * (1.0+proj(2, 1)) / proj(1, 1);
    const double fBottom = fz * (proj(2, 1)-1.0) / proj(1, 1);

    (*v)[0].set(0., 0., 0.);
    (*v)[1].set(nLeft, nBottom, -nz);
    (*v)[2].set(nRight, nBottom, -nz);
    (*v)[3].set(nRight, nTop, -nz);
    (*v)[4].set(nLeft, nTop, -nz);
    (*v)[5].set(fLeft, fBottom, -fz);
    (*v)[6].set(fRight, fBottom, -fz);
    (*v)[7].set(fRight, fTop, -fz);
    (*v)[8].set(fLeft, fTop, -fz);
    v->dirty();

    mt->setMatrix(osg::Matrixd::inverse(mv));
  }

  /**
   * Identical to the UpdateProjMatrix found in ProjectorManager.cpp -
   * but with a different base class. In osgEarth 3.x we expect that
   * this will be consolidated and osgEarth::Layer::TraversalCallback
   * will be replaced with osg::Callback, after which these two can be
   * unified.
   */
#define SIM_MAX_NODE_PROJECTORS 4

  class ProjectOnNodeUpdater : public osg::NodeCallback
  {
    using ProjectorObserver = osg::observer_ptr<simVis::ProjectorNode>;
    using ProjectorObservers = std::vector<ProjectorObserver>;

  public:
    ProjectOnNodeUpdater()
    {
      //nop
    }

    //! Adds a projector from this callback.
    //! Returns the number of projectors being managed by the callback, or
    //! -1 on error.
    int add(simVis::ProjectorNode* node)
    {
      OE_SOFT_ASSERT_AND_RETURN(node != nullptr, -1);
      OE_SOFT_ASSERT_AND_RETURN(projectors_.size() < SIM_MAX_NODE_PROJECTORS, -1);

      prune();

      if (std::find(projectors_.begin(), projectors_.end(), node) == projectors_.end())
      {
        projectors_.push_back(node);
        return projectors_.size();
      }
      else
      {
        // already installed - do nothing
        return -1;
      }
    }

    //! Removes a projector from this callback
    //! Return the number of remaining projectors
    int remove(simVis::ProjectorNode* node)
    {
      auto iter = std::find(projectors_.begin(), projectors_.end(), node);
      if (iter != projectors_.end())
        projectors_.erase(iter);
      prune();
      return projectors_.size();
    }

    //! prune any orphaned nodes from the set
    void prune()
    {
      if (projectors_.empty())
        return;

      ProjectorObservers keep;
      for (auto& projector : projectors_)
      {
        if (projector.valid())
          keep.push_back(projector);
      }
      projectors_.swap(keep);
    }

    void configureStateSet(osg::StateSet* ss)
    {
      prune();

      osgEarth::Util::ArrayUniform sampler(
        "simProjSampler",
        osg::Uniform::SAMPLER_2D,
        ss,
        SIM_MAX_NODE_PROJECTORS);

      unsigned count = projectors_.size();
      unsigned index = 0;

      for (auto& proj : projectors_)
      {
        sampler.setElement(index, (int)(simVis::ProjectorManager::getTextureImageUnit() + index));
        //ss->getOrCreateUniform("simProjSampler", osg::Uniform::SAMPLER_2D, SIM_MAX_NODE_PROJECTORS)
        //  ->setElement(index, (int)(simVis::ProjectorManager::getTextureImageUnit() + index));

        ss->setTextureAttribute(
          simVis::ProjectorManager::getTextureImageUnit() + index,
          proj->getTexture());

        ++index;
      }

      ss->setDefine("SIMVIS_NUM_PROJECTORS", std::to_string(count));

      updateUniforms(ss);
    }

    void updateUniforms(osg::StateSet* ss)
    {
      prune();
      unsigned index = 0;
      for (auto& proj : projectors_)
      {
        proj->copyUniformsTo(ss, projectors_.size(), index);
        ++index;
      }
    }

    //! Prunes the projector list and updates all texgen matrices
    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
      prune();

      if (!projectors_.empty())
      {
        // TODO: can we just put this on the node's stateset??
        osg::ref_ptr<osg::StateSet> ss = new osg::StateSet();
        osg::Uniform* u = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "simProjTexGenMat", SIM_MAX_NODE_PROJECTORS);
        ss->addUniform(u);

        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        osg::Matrixd inverseViewMatrix = cv->getCurrentCamera()->getInverseViewMatrix();

        unsigned count = 0;
        for (auto& proj : projectors_)
        {
          osg::Matrixf matrix = inverseViewMatrix * proj->getTexGenMatrix();
          u->setElement(count++, matrix);
        }

        // update all the individual project uniform values
        // TODO: will this work in the current frame??
        updateUniforms(node->getOrCreateStateSet());

        cv->pushStateSet(ss.get());
        traverse(node, nv);
        cv->popStateSet();
      }
    }

  private:
    ProjectorObservers projectors_;
  };
}

namespace simVis
{

ProjectorTextureImpl::ProjectorTextureImpl()
{
}

void ProjectorTextureImpl::setImage(osg::Image *image)
{
  if (texture_.valid())
  {
    texture_->setImage(image);
    simVis::fixTextureForGlCoreProfile(texture_.get());
  }
}

void ProjectorTextureImpl::setTexture(osg::Texture2D *texture)
{
  texture_ = texture;
}

//-------------------------------------------------------------------

ProjectorNode::ProjectorNode(const simData::ProjectorProperties& props, simVis::Locator* hostLocator, const simVis::EntityNode* host)
  : EntityNode(simData::PROJECTOR, new Locator()),
  lastProps_(props),
  host_(host),
  hostLocator_(hostLocator),
  hasLastUpdate_(false),
  hasLastPrefs_(false),
  projectorTextureImpl_(new ProjectorTextureImpl()),
  graphics_(nullptr),
  stateDirty_(false)
{
  init_();
}

ProjectorNode::~ProjectorNode()
{
  if (hostLocator_.valid())
    hostLocator_.get()->removeCallback(locatorCallback_.get());

  auto localCopy = projectedNodes_;
  for (auto node : localCopy)
  {
    osg::ref_ptr<osg::Node> lock;
    if (node.first.lock(lock))
      removeProjectionFromNode(node.first.get());
  }
}

void ProjectorNode::init_()
{
  // create the locator node that will support tethering and host/position the label.
  projectorLocatorNode_ = new LocatorNode(getLocator());
  projectorLocatorNode_->setEntityToMonitor(this);
  addChild(projectorLocatorNode_);

  // projector is inactive until prefs and updates make it active
  setNodeMask(DISPLAY_MASK_NONE);

  // listen for host locator changes so we can update the matrices
  locatorCallback_ = new simVis::SyncLocatorCallback<ProjectorNode>(this);
  if (hostLocator_.valid())
    hostLocator_.get()->addCallback(locatorCallback_.get());

  // Create matrix transform node that houses graphics frustum and set the node mask to off
  graphics_ = new osg::MatrixTransform();
  addChild(graphics_);
  graphics_->setNodeMask(DISPLAY_MASK_NONE);

  // create the uniforms that will control the texture projection:
  projectorActive_        = new osg::Uniform(osg::Uniform::BOOL,       "projectorActive");
  projectorAlpha_         = new osg::Uniform(osg::Uniform::FLOAT,      "projectorAlpha");
  texProjPosUniform_      = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "simProjPos");
  texProjDirUniform_      = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "simProjDir");
  useColorOverrideUniform_= new osg::Uniform(osg::Uniform::BOOL,       "projectorUseColorOverride");
  colorOverrideUniform_   = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "projectorColorOverride");
  projectorMaxRangeSquaredUniform_ = new osg::Uniform(osg::Uniform::FLOAT, "projectorMaxRangeSquared");

  projectorActive_->set(false);
  projectorAlpha_->set(DEFAULT_ALPHA_VALUE);
  useColorOverrideUniform_->set(false);
  projectorMaxRangeSquaredUniform_->set(0.f);

  // Set texture to default broken image
  texture_ = new osg::Texture2D(simVis::makeBrokenImage());

  // Set texture filters
  texture_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
  texture_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  texture_->setResizeNonPowerOfTwoHint(false);

  projectorTextureImpl_->setTexture(texture_.get());

  label_ = new EntityLabelNode();
  projectorLocatorNode_->addChild(label_);
  // labels are positioned on ellipsoid, culled based on label center point
  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  label_->addCullCallback(callback);

  // Set up an RTT camera that will generate a "shadow map"
  // The purpose of this shadow map is to prevent projected
  // textures from bleeding through to secondary surfaces.
  const unsigned w = 256, h = 256;

  shadowMap_ = new osg::Texture2D();
  shadowMap_->setTextureSize(w, h);
  shadowMap_->setInternalFormat(GL_DEPTH_COMPONENT);
  shadowMap_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
  shadowMap_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  shadowMap_->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
  shadowMap_->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
  shadowMap_->setBorderColor(osg::Vec4(1, 1, 1, 1));

  shadowCam_ = new osg::Camera();
  shadowCam_->setReferenceFrame(osg::Camera::ABSOLUTE_RF_INHERIT_VIEWPOINT);
  shadowCam_->setClearDepth(1.0);
  shadowCam_->setClearMask(GL_DEPTH_BUFFER_BIT);
  shadowCam_->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
  shadowCam_->setViewport(0, 0, shadowMap_->getTextureWidth(), shadowMap_->getTextureHeight());
  shadowCam_->setRenderOrder(osg::Camera::PRE_RENDER);
  shadowCam_->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
  shadowCam_->setImplicitBufferAttachmentMask(0, 0);
  shadowCam_->attach(osg::Camera::DEPTH_BUFFER, shadowMap_.get());

  // optimize depth rendering by disabling texturing and lighting
  osgEarth::CameraUtils::setIsDepthCamera(shadowCam_.get());

  osg::StateSet* ss = shadowCam_->getOrCreateStateSet();

  // ignore any uber shaders (like the LDB or Sky)
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(ss);
  vp->setInheritShaders(false);

  // only draw back faces to the shadow depth map
  ss->setAttributeAndModes(
    new osg::CullFace(osg::CullFace::FRONT),
    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

  ss->setAttributeAndModes(
    new osg::PolygonOffset(1, 1), //-1, -1),
    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

  // install a shadow-to-primary xform matrix (per frame) so verts match up when morphing
  shadowToPrimaryMatrix_ = ss->getOrCreateUniform(
    "oe_shadowToPrimaryMatrix", osg::Uniform::FLOAT_MAT4);
}

void ProjectorNode::updateLabel_(const simData::ProjectorPrefs& prefs)
{
  if (!hasLastUpdate_)
    return;

  std::string label = getEntityName_(prefs.commonprefs(), EntityNode::DISPLAY_NAME, false);
  if (prefs.commonprefs().labelprefs().namelength() > 0)
    label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

  std::string text;
  if (prefs.commonprefs().labelprefs().draw())
    text = labelContentCallback().createString(prefs, lastUpdate_, prefs.commonprefs().labelprefs().displayfields());

  if (!text.empty())
  {
    label += "\n";
    label += text;
  }

  // projector label is typically set to intersection of projector with ellipsoid, so an offset is needed
  const float zOffset = 1.0f;
  label_->update(prefs.commonprefs(), label, zOffset);
}

const simData::ProjectorUpdate* ProjectorNode::getLastUpdateFromDS() const
{
  return hasLastUpdate_ ? &lastUpdate_ : nullptr;
}

void ProjectorNode::applyToStateSet(osg::StateSet* stateSet) const
{
  stateSet->addUniform(projectorActive_.get());
  stateSet->addUniform(projectorAlpha_.get());
  stateSet->addUniform(texProjDirUniform_.get());
  stateSet->addUniform(texProjPosUniform_.get());
  stateSet->addUniform(useColorOverrideUniform_.get());
  stateSet->addUniform(colorOverrideUniform_.get());
  stateSet->addUniform(projectorMaxRangeSquaredUniform_.get());

  if (hasLastUpdate_ && lastPrefs_.shadowmapping())
    stateSet->setDefine("SIMVIS_PROJECT_USE_SHADOWMAP");
  else
    stateSet->removeDefine("SIMVIS_PROJECT_USE_SHADOWMAP");

  stateDirty_ = false;
}

void ProjectorNode::removeFromStateSet(osg::StateSet* stateSet) const
{
  stateSet->removeUniform(projectorActive_.get());
  stateSet->removeUniform(projectorAlpha_.get());
  stateSet->removeUniform(texProjDirUniform_.get());
  stateSet->removeUniform(texProjPosUniform_.get());
  stateSet->removeUniform(useColorOverrideUniform_.get());
  stateSet->removeUniform(colorOverrideUniform_.get());
  stateSet->removeUniform(projectorMaxRangeSquaredUniform_.get());

  stateSet->removeDefine("SIMVIS_PROJECT_USE_SHADOWMAP");
}

namespace
{
  template<typename T> void copyUniform(osg::StateSet* ss, osg::Uniform* src, unsigned size, unsigned index)
  {
    T temp;
    src->get(temp);
    osgEarth::Util::ArrayUniform u(src->getName(), src->getType(), ss, size);
    u.setElement(index, temp);
    //ss->getOrCreateUniform(src->getName(), src->getType(), size)
    //  ->setElement(index, temp);
  }
}

void ProjectorNode::copyUniformsTo(osg::StateSet* stateSet, unsigned size, unsigned index) const
{
  copyUniform<bool>(stateSet, projectorActive_.get(), size, index);
  copyUniform<float>(stateSet, projectorAlpha_.get(), size, index);
  copyUniform<osg::Vec3f>(stateSet, texProjDirUniform_.get(), size, index);
  copyUniform<osg::Vec3f>(stateSet, texProjPosUniform_.get(), size, index);
  copyUniform<bool>(stateSet, useColorOverrideUniform_.get(), size, index);
  copyUniform<osg::Vec4f>(stateSet, colorOverrideUniform_.get(), size, index);
  copyUniform<float>(stateSet, projectorMaxRangeSquaredUniform_.get(), size, index);
}

std::string ProjectorNode::popupText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
  {
    std::string prefix;
    // if alias is defined show both in the popup to match SIMDIS 9's behavior.  SIMDIS-2241
    if (!lastPrefs_.commonprefs().alias().empty())
    {
      if (lastPrefs_.commonprefs().usealias())
        prefix = getEntityName(EntityNode::REAL_NAME);
      else
        prefix = getEntityName(EntityNode::ALIAS_NAME);
      prefix += "\n";
    }
    return prefix + labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string ProjectorNode::hookText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string ProjectorNode::legendText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  return "";
}

const simData::ProjectorProperties& ProjectorNode::getProperties() const
{
  return lastProps_;
}

void ProjectorNode::setPrefs(const simData::ProjectorPrefs& prefs)
{
  if (PB_FIELD_CHANGED(&lastPrefs_, &prefs, rasterfile))
  {
    loadRequestedFile_(prefs.rasterfile());
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_, &prefs, showfrustum))
  {
    if (prefs.showfrustum())
      graphics_->setNodeMask(DISPLAY_MASK_PROJECTOR);
    else
      graphics_->setNodeMask(DISPLAY_MASK_NONE);
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_.commonprefs(), &prefs.commonprefs(), draw) ||
    (PB_FIELD_CHANGED(&lastPrefs_.commonprefs(), &prefs.commonprefs(), datadraw)))
  {
    if (prefs.commonprefs().draw() && prefs.commonprefs().datadraw() && host_.valid() && host_->isActive())
    {
      projectorActive_->set(true);
      setNodeMask(DISPLAY_MASK_PROJECTOR);
    }
    else if (!prefs.commonprefs().datadraw())
    {
      flush();
    }
    else
    {
      projectorActive_->set(false);
      setNodeMask(DISPLAY_MASK_NONE);
    }
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_, &prefs, projectoralpha))
  {
    projectorAlpha_->set(prefs.projectoralpha());
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_, &prefs, maxdrawrange))
  {
    if (prefs.maxdrawrange() <= 0.)
      projectorMaxRangeSquaredUniform_->set(0.f);
    else
      projectorMaxRangeSquaredUniform_->set(prefs.maxdrawrange() * prefs.maxdrawrange());
  }

  updateOverrideColor_(prefs);

  // If override FOV changes, update the FOV with a sync-with-locator call
  bool syncAfterPrefsUpdate = false;
  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_, &prefs, overridefov) ||
    PB_FIELD_CHANGED(&lastPrefs_, &prefs, overridefovangle))
  {
    syncAfterPrefsUpdate = true;
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED((&lastPrefs_.commonprefs()), (&prefs.commonprefs()), acceptprojectorid))
    applyProjectorPrefs_(lastPrefs_.commonprefs(), prefs.commonprefs());

  if (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefs_, &prefs, shadowmapping))
  {
      if (prefs.shadowmapping())
      {
        addChild(shadowCam_);
      }
      else if (shadowCam_.valid())
      {
        removeChild(shadowCam_);
      }

      stateDirty_ = true;
  }

  updateLabel_(prefs);
  lastPrefs_ = prefs;
  hasLastPrefs_ = true;

  // Apply the sync after prefs are updated, so that overridden FOV can be retrieved correctly
  if (hasLastUpdate_ && syncAfterPrefsUpdate)
    syncWithLocator();
}

const simData::ProjectorPrefs& ProjectorNode::getPrefs() const
{
  return lastPrefs_;
}

void ProjectorNode::updateOverrideColor_(const simData::ProjectorPrefs& prefs)
{
  if (hasLastPrefs_ &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, useoverridecolor) &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, overridecolor) &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, color))
    return;

  // using an override color?
  auto color = simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA);
  colorOverrideUniform_->set(color);
  useColorOverrideUniform_->set(prefs.commonprefs().useoverridecolor());
}

bool ProjectorNode::readVideoFile_(const std::string& filename)
{
  osg::Node* result = nullptr;

  // Make sure we have the clock which is needed for the video node.
  simCore::Clock* clock = simVis::Registry::instance()->getClock();
  if (clock)
  {
    osg::ref_ptr<simVis::ClockOptions> options = new simVis::ClockOptions(clock);
    options->setPluginData("ProjectorTextureProvider", projectorTextureImpl_.get());
    options->setObjectCacheHint(osgDB::Options::CACHE_NONE);
    result = osgDB::readNodeFile(filename, options.get());
  }

  // Save loaded video node
  if (result)
  {
    imageProvider_ = result;
    return true;
  }

  return false;
}

bool ProjectorNode::readRasterFile_(const std::string& filename)
{
  bool imageLoaded = false;
  if (filename.empty())
    return imageLoaded;

  osg::Image *image = osgDB::readImageFile(filename);
  if (image)
  {
    // if the image is a stream (i.e. a video), start it playing.
    if (dynamic_cast<osg::ImageStream*>(image))
      dynamic_cast<osg::ImageStream*>(image)->play();

    // Set image to projector texture
    setImage(image);
    imageLoaded = true;
  }
  return imageLoaded;
}

void ProjectorNode::loadRequestedFile_(const std::string& newFilename)
{
  std::string absURL = simVis::Registry::instance()->findModelFile(newFilename);
  bool imageLoaded = false;

  // If the file is a video file (TMD or LST), load node file via plugin and set projector interface
  if (simCore::hasExtension(newFilename, ".tmd") || simCore::hasExtension(newFilename, ".lst"))
  {
    imageLoaded = readVideoFile_(absURL);
  }
  else // Otherwise, load a static file
  {
    imageLoaded = readRasterFile_(absURL);
  }

  // if there's not image, use a default "broken" image.
  if (!imageLoaded)
  {
    SIM_ERROR << "Could not load \"" << newFilename << "\" into projector " << lastProps_.id() << "\n";
    setImage(simVis::makeBrokenImage());
  }
}

void ProjectorNode::setImage(osg::Image* image)
{
  // Reset video node if one is set.
  imageProvider_ = nullptr;
  texture_->setImage(image);
  simVis::fixTextureForGlCoreProfile(texture_.get());
}

const osg::Matrixd& ProjectorNode::getTexGenMatrix() const
{
  return texGenMatrix_;
}

const osg::Matrixd& ProjectorNode::getShadowMapMatrix() const
{
  return shadowMapMatrix_;
}

osg::Texture2D* ProjectorNode::getTexture() const
{
  return texture_.get();
}

double ProjectorNode::getVFOV() const
{
  // Not active, so return 0.0
  if (!hasLastUpdate_)
    return 0.0;

  // Allow for override
  if (hasLastPrefs_ && lastPrefs_.overridefov() && lastPrefs_.overridefovangle() > 0.)
    return lastPrefs_.overridefovangle() * simCore::RAD2DEG;

  // Return last FOV sent as an update
  if (lastUpdate_.has_fov())
    return lastUpdate_.fov() * simCore::RAD2DEG;

  // Set default if projector is active, but FOV has not been updated
  return DEFAULT_PROJECTOR_FOV_IN_DEG;
}

osg::Texture2D* ProjectorNode::getShadowMap() const
{
  return shadowMap_.get();
}


void ProjectorNode::getMatrices_(osg::Matrixd& projection, osg::Matrixd& locatorMat, osg::Matrixd& modelView) const
{
  const double ar = static_cast<double>(texture_->getImage()->s()) / texture_->getImage()->t();
  projection.makePerspective(getVFOV(), ar, 1.0, 1e7);
  if (hostLocator_.valid())
  {
    hostLocator_.get()->getLocatorMatrix(locatorMat);
  }
  else
  {
    // it is believed that the host locator cannot go missing
    assert(0);
  }
  modelView.invert(locatorMat);
}

void ProjectorNode::syncWithLocator()
{
  if (!isActive())
    return;
  if (!hostLocator_.valid())
    assert(0);

  // establish the view matrix:
  osg::Matrixd locatorMat;
  hostLocator_.get()->getLocatorMatrix(locatorMat);
  osg::Matrixd viewMat_temp = osg::Matrixd::inverse(locatorMat);

  // establish the projection matrix:
  osg::Matrixd projectionMat;
  const double ar = static_cast<double>(texture_->getImage()->s()) / texture_->getImage()->t();
  projectionMat.makePerspective(getVFOV(), ar, 1.0, 1.0e7);

  // The model matrix coordinate system of the projector is a normal tangent plane,
  // which means the projector will point straight down by default (since the view vector
  // is -Z in view space). We want the projector to point along the entity vector, so
  // we create a view matrix that rotates the view to point along the +Y axis.
  const osg::Matrix& rotateUp90Mat = osg::Matrix::rotate(-osg::PI_2, osg::Vec3d(1.0, 0.0, 0.0));
  viewMat_ = viewMat_temp * rotateUp90Mat;

  // flip the image if it's upside down
  const double flip = texture_->getImage()->getOrigin() == osg::Image::TOP_LEFT ? -1.0 : 1.0;

  // the coordinate generator for our projected texture -
  // during traversal, multiply the inverse view matrix by this
  // matrix to set a texture projection uniform that transform
  // verts from view space to texture space
  texGenMatrix_ =
    viewMat_ *
    projectionMat *
    osg::Matrix::translate(1.0, flip, 1.0) *      // bias
    osg::Matrix::scale(0.5, 0.5 * flip, 0.5);     // scale

  // same as the texgen matrix but without the flipping.
  shadowMapMatrix_ =
    viewMat_ *
    projectionMat *
    osg::Matrix::translate(1.0, 1.0, 1.0) * // bias
    osg::Matrix::scale(0.5, 0.5, 0.5);      // scale

  // the texture projector's position and directional vector in world space:
  osg::Vec3d eye, cen, up;
  viewMat_.getLookAt(eye, cen, up);
  texProjPosUniform_->set(osg::Vec3f(eye));
  texProjDirUniform_->set(osg::Vec3f(cen-eye));

  // determine the best available position for the projector
  double eciRefTime = 0.;
  double time = 0.;
  simCore::Vec3 hostPos;
  // obtain current time and eci ref time from host
  if (hostLocator_.valid())
  {
    const Locator* loc = hostLocator_.get();
    eciRefTime = loc->getEciRefTime();
    time = loc->getTime();
  }
  // if ellipsoid intersection can be calculated, use that result as the projector position
  osg::Vec3d ellipsoidIntersection;
  if (calculator_->intersectLine(eye, cen, ellipsoidIntersection))
  {
    const simCore::Vec3& intersection = convertToSim(ellipsoidIntersection);
    const simCore::Coordinate projPosition(simCore::COORD_SYS_ECEF, intersection);
    getLocator()->setCoordinate(projPosition, time, eciRefTime);
  }
  else
  {
    // default to "Null Island" if ellipsoid intersection is not calculable; but use host position if it is available
    simCore::Vec3 hostPosEcef(simCore::EARTH_RADIUS, 0., 0.);
    if (hostLocator_.valid())
      hostLocator_->getLocatorPosition(&hostPosEcef);
    const simCore::Coordinate projPosition(simCore::COORD_SYS_ECEF, hostPosEcef);
    getLocator()->setCoordinate(projPosition, time, eciRefTime);
  }

  // update the shadow camera
  if (shadowCam_.valid())
  {
    shadowCam_->setViewMatrix(viewMat_);
    shadowCam_->setProjectionMatrix(projectionMat);
  }

  // update the frustum geometry
  makeFrustum(projectionMat, viewMat_, graphics_);
}

bool ProjectorNode::isActive() const
{
  // Projector is "active" when it has datadraw, and a valid update,
  // and can be active even if draw is off;
  // that means: projector maintains valid internal state even if draw is off.
  return hasLastUpdate_ && hasLastPrefs_ && lastPrefs_.commonprefs().datadraw();
}

bool ProjectorNode::isVisible() const
{
  bool isVisible;
  projectorActive_.get()->get(isVisible);
  return isVisible;
}

simData::ObjectId ProjectorNode::getId() const
{
  return lastProps_.id();
}

bool ProjectorNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

const std::string ProjectorNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  if (!hasLastPrefs_)
    return "";

  return getEntityName_(lastPrefs_.commonprefs(), nameType, allowBlankAlias);
}

bool ProjectorNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  bool updateApplied = false;
  const simData::ProjectorUpdateSlice* updateSlice = static_cast<const simData::ProjectorUpdateSlice*>(updateSliceBase);
  assert(updateSlice);
  assert(host_.valid());

  // Check if host status has also changed and we need to update or not
  const bool hostChangedToActive = host_->isActive() && !hasLastUpdate_;
  const bool hostChangedToInactive = !host_->isActive() && hasLastUpdate_;

  // if not hasChanged, not forcing, and not a host transition, there is no update to apply
  if (updateSlice->hasChanged() || force ||hostChangedToActive || hostChangedToInactive)
  {
    const simData::ProjectorUpdate* current = updateSlice->current();
    const bool projectorChangedToInactive = (!current && hasLastUpdate_);

    // do not apply update if host is not active
    if (current && (force || host_->isActive()))
    {
      // Make sure to set projector to active if draw preferences are on.
      if (lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw())
      {
        projectorActive_->set(true);
        setNodeMask(DISPLAY_MASK_PROJECTOR);
      }
      else
      {
        projectorActive_->set(false);
        setNodeMask(DISPLAY_MASK_NONE);
      }

      lastUpdate_ = *current;
      hasLastUpdate_ = true;
      updateApplied = true;

      // Update matrices
      syncWithLocator();
    }
    else if (projectorChangedToInactive || hostChangedToInactive)
    {
      // If host not active or update doesn't exist, turn projector off
      flush();
      updateApplied = true;
    }
  }

  // Update label
  updateLabel_(lastPrefs_);

  return updateApplied;
}

void ProjectorNode::flush()
{
  projectorActive_->set(false);
  setNodeMask(DISPLAY_MASK_NONE);
  hasLastUpdate_ = false;
}

double ProjectorNode::range() const
{
  // Projector has no concept of range so should not be making this call
  assert(false);
  return 0.0;
}

int ProjectorNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return projectorLocatorNode_->getPosition(out_position, coordsys);
}

int ProjectorNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return projectorLocatorNode_->getPositionOrientation(out_position, out_orientation, coordsys);
}

void ProjectorNode::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == nv.CULL_VISITOR)
  {
    // set the primary-camera-to-shadow-camera transformation matrix,
    // which lets you perform vertex shader operations from the perspective
    // of the primary camera (morphing, etc.) so that things match up
    // between the two cameras.
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (cv)
    {
      shadowToPrimaryMatrix_->set(
        osg::Matrixd::inverse(viewMat_) *
        *cv->getModelViewMatrix());
    }
  }
  EntityNode::traverse(nv);
}

void ProjectorNode::setMapNode(osgEarth::MapNode* mapNode)
{
  if (shadowCam_.valid())
  {
    shadowCam_->removeChildren(0, shadowCam_->getNumChildren());
    if (mapNode)
    {
      shadowCam_->addChild(mapNode->getTerrainEngine()->getNode());
    }
  }
}

osgEarth::MapNode* ProjectorNode::getMapNode()
{
  return nullptr;
}

unsigned int ProjectorNode::objectIndexTag() const
{
  // Not supported for projectors
  return 0;
}

void ProjectorNode::setCalculator(std::shared_ptr<osgEarth::Util::EllipsoidIntersector> calculator)
{
  calculator_ = calculator;
}

int ProjectorNode::addProjectionToNode(osg::Node* entity, osg::Node* attachmentPoint)
{
  if (!attachmentPoint)
    return 1;

  // If there is already an update callback installed, find it:
  ProjectOnNodeUpdater* projOnNodeCallback = nullptr;
  osg::Callback* nestedCallback = attachmentPoint->getCullCallback();
  while (nestedCallback && !projOnNodeCallback)
  {
    projOnNodeCallback = dynamic_cast<ProjectOnNodeUpdater*>(nestedCallback);
    nestedCallback = nestedCallback->getNestedCallback();
  }

  // not found? create one and install it
  if (!projOnNodeCallback)
  {
    projOnNodeCallback = new ProjectOnNodeUpdater();
    attachmentPoint->addCullCallback(projOnNodeCallback);
  }

  // Add this projector node to the entity node's callback.
  // This will return the total number of projectors projecting
  // on this entity, or -1 upon error.
  int count = projOnNodeCallback->add(this);
  if (count > 0)
  {
    projectedNodes_[entity] = attachmentPoint;

    // install the texture application snippet.
    // TODO: optimize by creating this VP once and sharing across all projectors (low priority)
    osg::StateSet* stateSet = attachmentPoint->getOrCreateStateSet();

    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
    simVis::Shaders package;
    package.load(vp, package.projectorOnEntity());

    projOnNodeCallback->configureStateSet(stateSet);
  }

  return 0;
}

int ProjectorNode::removeProjectionFromNode(osg::Node* node)
{
  if (!node)
    return 1;

  auto attachmentPoint = projectedNodes_.find(node);
  if (attachmentPoint == projectedNodes_.end())
    return 1;

  // Find the management callback:
  ProjectOnNodeUpdater* projOnNodeCallback = nullptr;
  osg::Callback* nestedCallback = attachmentPoint->second->getCullCallback();
  while (nestedCallback && !projOnNodeCallback)
  {
    projOnNodeCallback = dynamic_cast<ProjectOnNodeUpdater*>(nestedCallback);
    nestedCallback = nestedCallback->getNestedCallback();
  }

  // This is actually a failed assertion! (should not happen)
  if (!projOnNodeCallback)
    return 1;

  // Remove from the updater:
  int count = projOnNodeCallback->remove(this);

  osg::StateSet* stateSet = attachmentPoint->second->getStateSet();
  if (stateSet)
  {
    // Was that the last one? If so, remove all the state info
    if (count == 0)
    {
      osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::get(stateSet);
      if (vp)
      {
        simVis::Shaders package;
        package.unload(vp, package.projectorOnEntity());
      }

      stateSet->removeDefine("SIMVIS_NUM_PROJECTORS");
      stateSet->removeUniform("simProjSampler");
      stateSet->removeTextureAttribute(ProjectorManager::getTextureImageUnit(), getTexture());

      removeFromStateSet(stateSet);
    }
    else
    {
      projOnNodeCallback->configureStateSet(stateSet);

      // remove the last one
      stateSet->removeTextureAttribute(count, osg::StateAttribute::TEXTURE);
    }
  }

  attachmentPoint->second->removeCullCallback(projectOnNodeCallback_.get());
  projectedNodes_.erase(attachmentPoint);
  return 0;
}

bool ProjectorNode::isStateDirty_() const
{
  return stateDirty_;
}

void ProjectorNode::resetStateDirty_()
{
  stateDirty_ = false;
}

}
