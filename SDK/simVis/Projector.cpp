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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Notify"
#include "osg/CullFace"
#include "osg/Geode"
#include "osg/ImageStream"
#include "osg/PolygonOffset"
#include "osg/Point"
#include "osg/LineWidth"
#include "osg/Depth"
#include "osg/MatrixTransform"
#include "osgDB/ReadFile"
#include "osgEarth/Horizon"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/String/Format.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/SphericalVolume.h"
#include "simVis/ClockOptions.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "simVis/Projector.h"

using namespace simVis;
using namespace osgEarth::Symbology;

static const double DEFAULT_PROJECTOR_FOV_IN_DEG = 45.0;
static const float DEFAULT_ALPHA_VALUE = 0.1f;

namespace
{
  // simple hook to update the frustum graphics when the locator changes
  struct LocatorChanged : public simVis::LocatorCallback
  {
    explicit LocatorChanged(ProjectorNode* node): node_(node) {}
    osg::observer_ptr<ProjectorNode> node_;
    void operator()(const class Locator* locator)
    {
      if (node_.valid())
        node_->refresh();
    }
  };

  // draws the geometry of the projection frustum.
  // (NOTE: some of this code is borrowed from OSG's osgthirdpersonview example)
  void makeFrustum(const osg::Matrixd& proj, const osg::Matrixd& mv, osg::MatrixTransform* mt)
  {
    osg::Geode* geode = NULL;
    osg::Geometry* geom = NULL;
    osg::Vec3Array* v = NULL;

    if (mt->getNumChildren() > 0)
    {
      geode = dynamic_cast<osg::Geode*>(mt->getChild(0));
      geom = geode->getDrawable(0)->asGeometry();
      v = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
    }
    else
    {
      geom = new osg::Geometry();
      geom->setUseVertexBufferObjects(true);
      geom->setUseDisplayList(false);
      v = new osg::Vec3Array(9);
      geom->setVertexArray(v);

      osg::Vec4Array* c = new osg::Vec4Array;
      c->push_back(osg::Vec4(1., 1., 1., 1.));
      geom->setColorArray(c);
      geom->setColorBinding(osg::Geometry::BIND_OVERALL);

      GLubyte idxLines[8] = { 0, 5, 0, 6, 0, 7, 0, 8 };
      GLubyte idxLoops0[4] = { 1, 2, 3, 4 };
      GLubyte idxLoops1[4] = { 5, 6, 7, 8 };
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINES, 8, idxLines));
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINE_LOOP, 4, idxLoops0));
      geom->addPrimitiveSet(new osg::DrawElementsUByte(osg::PrimitiveSet::LINE_LOOP, 4, idxLoops1));

      geode = new osg::Geode();
      geode->addDrawable(geom);
      simVis::setLighting(geode->getOrCreateStateSet(), osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

      mt->addChild(geode);
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

};

//-------------------------------------------------------------------

ProjectorTextureImpl::ProjectorTextureImpl()
{
}

void ProjectorTextureImpl::setImage(osg::Image *image)
{
  if (texture_.valid())
  {
    texture_->setImage(image);
  }
}

void ProjectorTextureImpl::setTexture(osg::Texture2D *texture)
{
  texture_ = texture;
}

//-------------------------------------------------------------------

ProjectorNode::ProjectorNode(const simData::ProjectorProperties& props, simVis::Locator* hostLocator, const simVis::EntityNode* host)
  : EntityNode(simData::PROJECTOR, new simVis::Locator(hostLocator)),
    lastProps_(props),
    host_(host),
    contentCallback_(new NullEntityCallback()),
    hasLastUpdate_(false),
    hasLastPrefs_(false),
    projectorTextureImpl_(new ProjectorTextureImpl())
{
  init_();
}

ProjectorNode::~ProjectorNode() {}

void ProjectorNode::init_()
{
  // listen for locator changes so we can update the matrices
  getLocator()->addCallback(new LocatorChanged(this));

  // Set this node to be active
  setNodeMask(DISPLAY_MASK_PROJECTOR);

  // Create matrix transform node that houses graphics frustum and set the node mask to off
  graphics_ = new osg::MatrixTransform();
  addChild(graphics_);
  graphics_->setNodeMask(DISPLAY_MASK_NONE);

  // create the uniforms that will control the texture projection:
  projectorActive_        = new osg::Uniform(osg::Uniform::BOOL,       "projectorActive");
  projectorAlpha_         = new osg::Uniform(osg::Uniform::FLOAT,      "projectorAlpha");
  texGenMatUniform_       = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "simProjTexGenMat");
  texProjPosUniform_      = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "simProjPos");
  texProjDirUniform_      = new osg::Uniform(osg::Uniform::FLOAT_VEC3, "simProjDir");

  projectorActive_->set(false);
  projectorAlpha_->set(DEFAULT_ALPHA_VALUE);

  // Set texture to default broken image
  texture_ = new osg::Texture2D(simVis::makeBrokenImage());

  // Set texture filters
  texture_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
  texture_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  texture_->setResizeNonPowerOfTwoHint(false);

  projectorTextureImpl_->setTexture(texture_.get());

  label_ = new EntityLabelNode(getLocator());
  addChild(label_);

  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setHorizon(new osgEarth::Horizon(*getLocator()->getSRS()->getEllipsoid()));
  callback->setProxyNode(this);
  label_->addCullCallback(callback);
}

void ProjectorNode::updateLabel_(const simData::ProjectorPrefs& prefs)
{
  if (!hasLastUpdate_)
    return;

  std::string label = getEntityName(EntityNode::DISPLAY_NAME);
  if (prefs.commonprefs().labelprefs().namelength() > 0)
    label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

  std::string text;
  if (prefs.commonprefs().labelprefs().draw())
    text = contentCallback_->createString(prefs, lastUpdate_, prefs.commonprefs().labelprefs().displayfields());

  if (!text.empty())
  {
    label += "\n";
    label += text;
  }

  const float zOffset = 0.0f;
  label_->update(prefs.commonprefs(), label, zOffset);
}

void ProjectorNode::setLabelContentCallback(LabelContentCallback* cb)
{
  if (cb == NULL)
    contentCallback_ = new NullEntityCallback();
  else
    contentCallback_ = cb;
}

LabelContentCallback* ProjectorNode::labelContentCallback() const
{
  return contentCallback_.get();
}

std::string ProjectorNode::hookText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return contentCallback_->createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());

  return "";
}

std::string ProjectorNode::legendText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return contentCallback_->createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());

  return "";
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

  updateLabel_(prefs);
  lastPrefs_ = prefs;
  hasLastPrefs_ = true;
}

bool ProjectorNode::readVideoFile_(const std::string& filename)
{
   osg::Node* result = NULL;

  // Make sure we have the clock which is needed for the video node.
  simCore::Clock* clock = simVis::Registry::instance()->getClock();
  if (clock != NULL)
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
  imageProvider_ = NULL;
  texture_->setImage(image);
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

  // Return last FOV sent as an update
  if (lastUpdate_.has_fov())
    return lastUpdate_.fov() * simCore::RAD2DEG;

  // Set default if projector is active, but FOV has not been updated
  return DEFAULT_PROJECTOR_FOV_IN_DEG;
}

void ProjectorNode::getMatrices_(osg::Matrixd& projection, osg::Matrixd& locatorMat, osg::Matrixd& modelView)
{
  const double ar = static_cast<double>(texture_->getImage()->s()) / texture_->getImage()->t();
  projection.makePerspective(getVFOV(), ar, 1.0, 1e7);
  getLocator()->getLocatorMatrix(locatorMat);
  modelView.invert(locatorMat);
}

void ProjectorNode::refresh()
{
  osg::Matrixd projectionMat, locatorMat, modelMat;
  getMatrices_(projectionMat, locatorMat, modelMat);

  // The model matrix coordinate system of the projector is a normal tangent plane,
  // which means the projector will point straight down by default (since the view vector
  // is -Z in view space). We want the projector to point along the entity vector, so
  // we create a view matrix that rotates the view to point along the +Y axis.
  osg::Matrix viewMat;
  viewMat.makeRotate(-osg::PI_2, osg::Vec3d(1,0,0));

  // flip the image if it's upside down
  double flip = texture_->getImage()->getOrigin() == osg::Image::TOP_LEFT? -1.0 : 1.0;

  // construct the model view matrix:
  osg::Matrix modelViewMat = modelMat * viewMat;

  // the coordinate generator for our projected texture:
  osg::Matrix texGenMat =
    modelViewMat *
    projectionMat *
    osg::Matrix::translate(1, flip, 1) *        // bias
    osg::Matrix::scale(0.5, 0.5*flip, 0.5);     // scale
  texGenMatUniform_->set(texGenMat);

  // the texture projector's position and directional vector in world space:
  osg::Vec3d eye, cen, up;
  modelViewMat.getLookAt(eye, cen, up);
  texProjPosUniform_->set(osg::Vec3f(eye));
  texProjDirUniform_->set(osg::Vec3f(cen-eye));

  // update the frustum geometry
  makeFrustum(projectionMat, modelViewMat, graphics_);
}

bool ProjectorNode::isActive() const
{
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

  switch (nameType)
  {
  case EntityNode::REAL_NAME:
    return lastPrefs_.commonprefs().name();
  case EntityNode::ALIAS_NAME:
    return lastPrefs_.commonprefs().alias();
  case EntityNode::DISPLAY_NAME:
    if (lastPrefs_.commonprefs().usealias())
    {
      if (!lastPrefs_.commonprefs().alias().empty() || allowBlankAlias)
        return lastPrefs_.commonprefs().alias();
    }
    return lastPrefs_.commonprefs().name();
  }
  return "";
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
    const bool projectorChangedToInactive = (current == NULL && hasLastUpdate_);

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
      refresh();
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

void ProjectorNode::traverse(osg::NodeVisitor& nv)
{
  EntityNode::traverse(nv);
}

unsigned int ProjectorNode::objectIndexTag() const
{
  // Not supported for projectors
  return 0;
}
