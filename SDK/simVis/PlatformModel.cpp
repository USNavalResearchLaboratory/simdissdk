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
#include "osg/AutoTransform"
#include "osg/ComputeBoundsVisitor"
#include "osg/CullFace"
#include "osg/Geode"
#include "osg/LOD"
#include "osg/PolygonMode"
#include "osg/PolygonStipple"
#include "osg/ShapeDrawable"
#include "osg/CullStack"
#include "osg/Viewport"
#include "osgDB/ReadFile"
#include "osgEarth/AutoScale"
#include "osgEarth/Horizon"
#include "osgEarth/ObjectIndex"
#include "osgEarthAnnotation/AnnotationUtils"

#include "simVis/Constants.h"
#include "simVis/EntityLabel.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/RCS.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simNotify/Notify.h"
#include "simCore/String/Format.h"
#include "simCore/Calc/Angle.h"
#include "simVis/PlatformModel.h"

#define LC "[PlatformModel] "

using namespace simVis;
using namespace osgEarth::Symbology;
using namespace osgEarth::Annotation;


/** OSG Mask for traversal (like the select type in SIMDIS 9) */
const int PlatformModelNode::TRAVERSAL_MASK = simVis::DISPLAY_MASK_PLATFORM_MODEL;
/** Conversion factor to convert a brightness pref value (0-100) to an ambient light value (from S9) */
static const float BRIGHTNESS_TO_AMBIENT = 0.022f;
/** Default brightness ambient value; 36 brightness is the default value */
static const osg::Vec4f DEFAULT_AMBIENT(
  36.f * BRIGHTNESS_TO_AMBIENT,
  36.f * BRIGHTNESS_TO_AMBIENT,
  36.f * BRIGHTNESS_TO_AMBIENT,
  1.f
  );

/* OSG Scene Graph Layout of This Class
 *
 *       /= labelRoot => label_              /= rcs_         /= alphaVolumeGroup_ => model_
 * this => dynamicXform_ => imageIconXform_ <=> offsetXform_ => model_
 *                                           \= other scaled children
 *
 * model_ is the representative for the 3D model or 2D image, and may be set to NULL at times.
 * It is set from the call to simVis::Registry::instance()->getOrCreateIconModel().
 *
 * The offsetXform_ handles transforms provided by the end user to orient and translate the model
 * correctly in the scene.  For example, a 90 degree yaw orientation can be used to fix models
 * that point the wrong way.  The offsets only apply to the model and not any
 * of its attachments.
 *
 * The rcs_ is the radar cross section, which might be a 2D or 3D segment.  It is colored on
 * its own based on RCS data file settings.  It's important that override color not apply
 * accidentally to the RCS, and that RCS not rotate or translate with offsets.  This is the same as
 * other scaled children.  So anything that is supposed to be scaled goes under the image icon
 * transform.
 *
 * The imageIconXform_ applies the rotate-to-screen functionality to 2-D icons.  When 2-D icons
 * are used, it will rotate the icon according to the billboarding settings in the PlatformPrefs.
 * When 3-D icons are used, the billboarding is completely disabled and it acts as a pass-through.
 *
 * dynamicXform_ deals with dynamic scaling.  See the documentation for DynamicScaleNode for more
 * details, but it will apply ScaleXYZ if available, or static scaling if disabled, or dynamic
 * scaling if enabled.  Dynamic scaling will make the icon larger based on current distance from
 * the eye, based on various scaling tweaks that are in the platform prefs.
 *
 * PlatformModelNode is a locator, and is tied to the platform's position in space.
 *
 * alphaVolumeGroup_ is only on when the alphavolume() preference is on.  It does a second pass on
 * the platform model, this time drawing the backfaces.  This is useful for things like drawing the
 * interior of an alpha sphere used in error ellipses.
 */

PlatformModelNode::PlatformModelNode(Locator* locator)
  : LocatorNode(locator),
    isImageModel_(false),
    autoRotate_(false),
    lastPrefsValid_(false),
    brightnessUniform_(new osg::Uniform("osg_LightSource[0].ambient", DEFAULT_AMBIENT)),
    objectIndexTag_(0)
{
  osg::Group* labelRoot = new osg::Group();
  labelRoot->setName("labelRoot");
  label_ = new EntityLabelNode(labelRoot);

  setName("PlatformModel");
  setNodeMask(getMask());

  // Provides an icon orientation and position on the model
  offsetXform_ = new osg::MatrixTransform();
  offsetXform_->setName("offsetXform");

  // Apply the override color shader to the container
  overrideColor_ = new simVis::OverrideColor(offsetXform_->getOrCreateStateSet());

  // Set up the transform responsible for rotating the 2-D image icons
  imageIconXform_ = new PixelAutoTransform();
  imageIconXform_->setAutoScaleToScreen(false);
  imageIconXform_->setAutoRotateMode(PixelAutoTransform::NO_ROTATION);
  imageIconXform_->setName("imageIconXform");
  imageIconXform_->dirty();

  // Horizon culler for the platform. The culler is attached to this node,
  // but uses the imageIconXform for the actual testing.
  HorizonCullCallback* hcc = new HorizonCullCallback();
  hcc->setCullByCenterPointOnly(true);
  hcc->setProxyNode(imageIconXform_);
  hcc->setName("HorizonCullCallback");
  addCullCallback(hcc);

  // the following line is necessary prior to OSG 3.4, since we are unable
  // to pass the shared Horizon down from the ScenarioManager:
  hcc->setHorizon(new Horizon(*locator->getSRS()->getEllipsoid()));

  // used to apply both dynamic and static scaling to the model.
  dynamicXform_ = new simVis::DynamicScaleTransform();
  dynamicXform_->setName("dynamicXform");

  // Configure children graph
  addChild(labelRoot);
  addChild(dynamicXform_);
  dynamicXform_->addChild(imageIconXform_);
  imageIconXform_->addChild(offsetXform_);

  // Set up the brightness factor for the entity, attaching close to the model
  offsetXform_->getOrCreateStateSet()->addUniform(brightnessUniform_, osg::StateAttribute::ON);

  // Tag the platform at the lowest unique level feasible
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(offsetXform_, offsetXform_);

  // When alpha volume is on, we turn on this node
  alphaVolumeGroup_ = new osg::Group;
  alphaVolumeGroup_->setName("Alpha Volume Group");
  offsetXform_->addChild(alphaVolumeGroup_);
  alphaVolumeGroup_->setNodeMask(0); // off by default
  // Draw the backface
  alphaVolumeGroup_->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE);
}

PlatformModelNode::~PlatformModelNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

bool PlatformModelNode::isImageModel() const
{
  return isImageModel_;
}

osg::Node* PlatformModelNode::offsetNode() const
{
  return offsetXform_;
}

unsigned int PlatformModelNode::objectIndexTag() const
{
  return objectIndexTag_;
}

bool PlatformModelNode::addScaledChild(osg::Node* node)
{
  // Scaled children go into the imageIconXform_, so that the orientation fixes
  // for the model do not accidentally swap the location/orientation of attachments
  if (imageIconXform_ == NULL)
    return false;
  return imageIconXform_->addChild(node);
}

bool PlatformModelNode::removeScaledChild(osg::Node* node)
{
  if (imageIconXform_ == NULL)
    return false;
  return imageIconXform_->removeChild(node);
}

void PlatformModelNode::syncWithLocator()
{
  // call the base class first to update the matrix.
  LocatorNode::syncWithLocator();

  // if we're in IR_2D_YAW mode we will need to configure the
  // xform with the new heading information.
  if (imageIconXform_->getRotateInScreenSpace())
  {
    simCore::Coordinate c;
    getLocator()->getCoordinate(&c, simCore::COORD_SYS_LLA);
    imageIconXform_->setScreenSpaceRotation(c.yaw());
  }
}

bool PlatformModelNode::updateModel_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, icon))
    return false;

  // if the new properties say "no model", remove any existing model.
  if (prefs.icon().empty() && model_.valid())
  {
    offsetXform_->removeChild(model_);
    alphaVolumeGroup_->removeChild(model_);
    model_ = NULL;
    return true;
  }

  // if there's an existing model, save its parent group.
  if (model_.valid())
  {
    offsetXform_->removeChild(model_);
    alphaVolumeGroup_->removeChild(model_);
  }

  std::string newModelURI = prefs.icon();

  // don't bother to create model if the icon name is empty, to support 3D Landmarks with no icons
  if (newModelURI.empty())
    return true;

  model_ = simVis::Registry::instance()->getOrCreateIconModel(newModelURI, &isImageModel_);
  // If we were not able to load the icon/model, create a box to use as a placeholder.
  if (!model_.valid())
  {
    if (!simVis::Registry::instance()->isMemoryCheck())
    {
      SIM_WARN << "Failed to find icon model: " << newModelURI << "" << std::endl;
    }

    // Use the unit cube
    osg::Geode* geode = new osg::Geode();
    osg::StateSet* stateset = new osg::StateSet();
    geode->setStateSet(stateset);
    geode->addDrawable(new osg::ShapeDrawable(new osg::Box()));
    model_ = geode;
  }

  // render order:
  osg::StateSet* modelStateSet = model_->getOrCreateStateSet();
  if (isImageModel_)
    modelStateSet->setRenderBinDetails(BIN_PLATFORM_IMAGE, simVis::BIN_GLOBAL_SIMSDK);
  else // does 0 work?
  {
    modelStateSet->setRenderBinDetails(BIN_PLATFORM_MODEL, BIN_TRAVERSAL_ORDER_SIMSDK);
  }

  // re-apply the parent group.
  offsetXform_->addChild(model_);
  alphaVolumeGroup_->addChild(model_);
  dynamicXform_->setSizingNode(model_);

  return true;
}

void PlatformModelNode::setRotateToScreen(bool value)
{
  autoRotate_ = value;

  if (autoRotate_)
  {
    imageIconXform_->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  }
  else
  {
    imageIconXform_->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
    imageIconXform_->setRotation(osg::Quat());
  }
  imageIconXform_->dirty();
}

bool PlatformModelNode::updateOffsets_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, platpositionoffset) &&
      !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, pitch) &&
      !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, yaw) &&
      !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, orientationoffset, roll))
      return false;
  osg::Matrix offsetMatrix;

  if (prefs.has_platpositionoffset())
  {
    const simData::Position& pos = prefs.platpositionoffset();
    // x/y order change and sign change needed to match the behavior of SIMDIS 9
    offsetMatrix.makeTranslate(osg::Vec3(-pos.y(), pos.x(), pos.z()));
  }
  if (prefs.has_orientationoffset())
  {
    const simData::BodyOrientation& ori = prefs.orientationoffset();
    if (ori.yaw() != 0.0 || ori.pitch() != 0.0 || ori.roll() != 0.0)
    {
      const osg::Quat& qrot = Math::eulerRadToQuat(ori.yaw(), ori.pitch(), ori.roll());
      offsetMatrix.preMultRotate(qrot);
    }
  }
  offsetXform_->setMatrix(offsetMatrix);

  // Changing icon orientation can change the reported 'actual' bounds for model; return non-zero
  return true;
}

// Recalculates the scaled and unscaled bounds of the model. We need
// to call this whenever the model or the scale setup changes.
void PlatformModelNode::updateBounds_()
{
  if (!model_.valid())
    return;

  // remove rcs to avoid inclusion in the bounds calculation
  if (rcs_.valid())
    imageIconXform_->removeChild(rcs_.get());

  // remove all children except the model, since only the model should be included in the bounds calculation
  const unsigned int numChildren = offsetXform_->getNumChildren();
  // store all children except the model locally to add back after bounds calculation
  osg::NodeList children;
  for (unsigned int i = 0; i < numChildren; i++)
  {
    children.push_back(offsetXform_->getChild(i));
  }
  // now remove all children except the model
  offsetXform_->removeChildren(0, numChildren);
  offsetXform_->addChild(model_);

  // Compute bounds, but exclude the label:
  osg::ComputeBoundsVisitor cb;
  cb.setTraversalMask(cb.getTraversalMask() |~ simVis::DISPLAY_MASK_LABEL);
  offsetXform_->accept(cb);
  unscaledBounds_ = cb.getBoundingBox();

  // Now get the scaled bounds
  cb.reset();
  dynamicXform_->accept(cb);
  bounds_ = cb.getBoundingBox();

  // add rcs back to the image transform
  if (rcs_.valid())
    imageIconXform_->addChild(rcs_.get());

  // add children back to model container in original order
  offsetXform_->removeChild(model_);
  for (osg::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    offsetXform_->addChild(*iter);
  }
}

bool PlatformModelNode::updateScale_(const simData::PlatformPrefs& prefs, bool force)
{
  // Check for ScaleXYZ first
  if (prefs.has_scalexyz())
  {
    if ((prefs.scalexyz().x() > 0.0) && (prefs.scalexyz().y() > 0.0) && (prefs.scalexyz().z() > 0.0))
    {
      return updateScaleXyz_(prefs, force);
    }

    // if ScaleXYZ just turned off than force the other scaling
    force =  force || PB_FIELD_CHANGED(&lastPrefs_, &prefs, scalexyz);
  }

  // Clear out the override scaling at this point so latent values don't take over
  dynamicXform_->clearOverrideScale();
  return updateDynamicScale_(prefs, force);
}

bool PlatformModelNode::updateScaleXyz_(const simData::PlatformPrefs& prefs, bool force)
{
  // By default scalexyz is NULL. It is used to override the scale preference which is defaulted to 1. When uninitialized, it should not override the scale pref.
  if (!prefs.has_scalexyz() || (lastPrefsValid_ && !force &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, scalexyz)))
    return false;

  if ((prefs.scalexyz().x() <= 0.0) || (prefs.scalexyz().y() <= 0.0) || (prefs.scalexyz().z() <= 0.0))
    return false;

  // update the static scaling using the scaleXYZ pref
  dynamicXform_->setOverrideScale(osg::Vec3d(prefs.scalexyz().y(), prefs.scalexyz().x(), prefs.scalexyz().z()));

  return true;
}

bool PlatformModelNode::updateDynamicScale_(const simData::PlatformPrefs& prefs, bool force)
{
  if (lastPrefsValid_ && !force &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, scale) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscale) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscalescalar) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscaleoffset))
    return false;

  const bool ds = prefs.dynamicscale();

  dynamicXform_->setDynamicScalingEnabled(ds);
  // Scale applies whether dynamic scaling is enabled or static scaling is enabled
  dynamicXform_->setStaticScalar(prefs.scale());
  // Scale scalar and scale offset only apply when dynamic scaling is on
  if (ds)
  {
    dynamicXform_->setDynamicScalar(prefs.dynamicscalescalar());
    dynamicXform_->setScaleOffset(prefs.dynamicscaleoffset());
  }

  return true;
}

void PlatformModelNode::updateImageIconRotation_(const simData::PlatformPrefs& prefs, bool force)
{
  // If neither icon or rotateicons changed, then nothing to do here.
  if (lastPrefsValid_ && !force &&
    !PB_FIELD_CHANGED(&lastPrefs_, &prefs, icon) &&
    !PB_FIELD_CHANGED(&lastPrefs_, &prefs, rotateicons))
  {
    return;
  }
  // At least icon or rotateicons has changed

  // If we're not using an image model, then reset the rotations and return
  if (!isImageModel_)
  {
    setRotateToScreen(false);
    imageIconXform_->setRotateInScreenSpace(false);
    // Reset components to inherit
    getLocator()->setComponentsToInherit(
      getLocator()->getComponentsToInherit() | simVis::Locator::COMP_ORIENTATION);
    return;
  }

  setRotateToScreen(
    prefs.rotateicons() == simData::IR_2D_UP);
  imageIconXform_->setRotateInScreenSpace(prefs.rotateicons() == simData::IR_2D_YAW);

  if (prefs.rotateicons() == simData::IR_3D_YPR)
  {
    getLocator()->setComponentsToInherit(
      getLocator()->getComponentsToInherit() | simVis::Locator::COMP_ORIENTATION);
  }
  else if (prefs.rotateicons() == simData::IR_3D_YAW || prefs.rotateicons() == simData::IR_2D_YAW)
  {
    unsigned int mask = getLocator()->getComponentsToInherit();
    mask &= ~Locator::COMP_ORIENTATION;
    mask |=  Locator::COMP_HEADING;
    getLocator()->setComponentsToInherit(mask);
  }
  else if (prefs.rotateicons() == simData::IR_3D_NORTH || prefs.rotateicons() == simData::IR_2D_UP)
  {
    unsigned int mask = getLocator()->getComponentsToInherit();
    mask &= ~Locator::COMP_ORIENTATION;
    getLocator()->setComponentsToInherit(mask);
  }
}

void PlatformModelNode::updateRCS_(const simData::PlatformPrefs& prefs)
{
  // if there is an RCS file, and no RCS model
  if (!prefs.rcsfile().empty() && !rcs_.valid())
  {
    // create it
    rcs_ = new RCSNode();
    imageIconXform_->addChild(rcs_.get());

    // scale the RCS to make it visible:
    if (model_.valid())
      rcs_->setScale(model_->getBound().radius() * 2.0);
  }
  //else if there is no RCS file, but there is a model
  else if (prefs.rcsfile().empty() && rcs_.valid())
  {
    // remove it
    imageIconXform_->removeChild(rcs_.get());
    rcs_ = NULL;
  }

  if (rcs_.valid())
    rcs_->setPrefs(prefs);
}

void PlatformModelNode::setRcsData(simCore::RadarCrossSectionPtr rcsData)
{
  if (rcs_.valid())
    rcs_->setRcs(rcsData);
}

void PlatformModelNode::updateStippling_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, usepolygonstipple) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, polygonstipple))
    return;

  if (!model_.valid())
    return;
  osg::observer_ptr<osg::Geode> geom = static_cast<osg::Geode*>(model_.get());
  osg::observer_ptr<osg::StateSet> stateSet = geom->getStateSet();

  if (!prefs.usepolygonstipple())
  {
    stateSet->removeAttribute(osg::StateAttribute::POLYGONSTIPPLE);
  }
  else
  {
    osg::ref_ptr<osg::PolygonStipple> ps = NULL;

    switch (prefs.polygonstipple())
    {
      case 1:
        ps = new osg::PolygonStipple(gPatternMask1);
        break;
      case 2:
        ps = new osg::PolygonStipple(gPatternMask2);
        break;
      case 3:
        ps = new osg::PolygonStipple(gPatternMask3);
        break;
      case 4:
        ps = new osg::PolygonStipple(gPatternMask4);
        break;
      case 5:
        ps = new osg::PolygonStipple(gPatternMask5);
        break;
      case 6:
        ps = new osg::PolygonStipple(gPatternMask6);
        break;
      case 7:
        ps = new osg::PolygonStipple(gPatternMask7);
        break;
      case 8:
        ps = new osg::PolygonStipple(gPatternMask8);
        break;
      case 9:
        ps = new osg::PolygonStipple(gPatternMask9);
        break;
      default:
        // if assert occurs, an invalid polygon stipple has been specified; SIMDIS defines 9 stipple patterns
        assert(0);
        return;
    }
    stateSet->setAttributeAndModes(ps, osg::StateAttribute::ON);
  }
}

void PlatformModelNode::updateCulling_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, usecullface) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, cullface))
    return;

  osg::StateSet* stateSet = offsetXform_->getOrCreateStateSet();
  if (!prefs.usecullface())
  {
    stateSet->removeAttribute(osg::StateAttribute::CULLFACE);
  }
  else
  {
    switch (prefs.cullface())
    {
      case simData::FRONT:
        stateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON);
        break;
      case simData::BACK:
        stateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON);
        break;
      case simData::FRONT_AND_BACK:
        stateSet->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT_AND_BACK), osg::StateAttribute::ON);
        break;
      default:
        // assert fail means an invalid face was specified
        assert(0);
        return;
    }
  }
}

void PlatformModelNode::updatePolygonMode_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, polygonmodeface) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, polygonmode) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, drawmode))
    return;

  if (!model_.valid())
    return;
  osg::observer_ptr<osg::Geode> geom = static_cast<osg::Geode*>(model_.get());
  osg::observer_ptr<osg::StateSet> stateSet = geom->getStateSet();

  // Have default values for face/mode
  osg::PolygonMode::Face face = osg::PolygonMode::FRONT_AND_BACK;
  osg::PolygonMode::Mode mode = osg::PolygonMode::FILL;

  // Note that draw mode and polygon mode conflict.  If one is set, use it
  if (!prefs.has_polygonmode() || prefs.has_polygonmodeface())
  {
    // Polygon mode is not set; rely on draw mode for functionality
    if (!prefs.has_drawmode())
    {
      // No setting for polygon mode; remove it and return
      stateSet->removeAttribute(osg::StateAttribute::POLYGONMODE);
      return;
    }
    else
    {
      // Draw mode is set, but polygon mode is not.  Use draw mode values
      face = osg::PolygonMode::FRONT_AND_BACK;
      switch (prefs.drawmode())
      {
      case simData::MDM_POINTS:
        mode = osg::PolygonMode::POINT;
        break;
      case simData::MDM_WIRE:
        mode = osg::PolygonMode::LINE;
        break;
      case simData::MDM_SOLID:
        mode = osg::PolygonMode::FILL;
        break;
      default:
        assert(0); // invalid value
        break;
      }
    }
  }
  else
  {
    // Polygon mode IS set; use these values and ignore the draw mode
    face = static_cast<osg::PolygonMode::Face>(prefs.polygonmodeface());
    mode = static_cast<osg::PolygonMode::Mode>(prefs.polygonmode());
  }

  // assert fail means an invalid face was specified;
  assert(face == osg::PolygonMode::FRONT || face == osg::PolygonMode::BACK || face == osg::PolygonMode::FRONT_AND_BACK);
  // assert fail means an invalid mode was specified;
  assert(mode == osg::PolygonMode::POINT || mode == osg::PolygonMode::LINE || mode == osg::PolygonMode::FILL);
  stateSet->setAttributeAndModes(new osg::PolygonMode(face, mode), osg::StateAttribute::ON);
}

void PlatformModelNode::updateLighting_(const simData::PlatformPrefs& prefs, bool force)
{
  if (!model_.valid())
    return;

  if (!force && lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, lighted))
    return;

  // Turn lighting on if lighting is enabled, but force it off if lighting is off.  This
  // prevents models from turning lighting on when we don't want it on.  Models can then
  // feasibly override this with the PROTECTED|ON state.
  simVis::setLighting(offsetXform_->getOrCreateStateSet(), (!isImageModel_ && prefs.lighted())
    ? osg::StateAttribute::ON
    : (osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE));
}

void PlatformModelNode::updateOverrideColor_(const simData::PlatformPrefs& prefs, bool force)
{
  if (!overrideColor_.valid())
    return;

  if (!force && lastPrefsValid_ &&
      !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, useoverridecolor) &&
      !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, overridecolor))
    return;

  // using an override color?
  overrideColor_->setColor(simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA));
  if (!prefs.commonprefs().useoverridecolor())
    overrideColor_->setCombineMode(OverrideColor::OFF);
  else
    overrideColor_->setCombineMode(OverrideColor::MULTIPLY_COLOR);
}

void PlatformModelNode::updateAlphaVolume_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ && !PB_FIELD_CHANGED(&lastPrefs_, &prefs, alphavolume))
    return;

  if (prefs.alphavolume())
  {
    // Turn off depth writes
    offsetXform_->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
    alphaVolumeGroup_->setNodeMask(getMask());
  }
  else
  {
    offsetXform_->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::DEPTH);
    alphaVolumeGroup_->setNodeMask(0);
  }
}

void PlatformModelNode::setProperties(const simData::PlatformProperties& props)
{
  lastProps_ = props;
}

void PlatformModelNode::setPrefs(const simData::PlatformPrefs& prefs)
{
  // If a new model is loaded than force a scale update
  const bool modelChanged = updateModel_(prefs);

  // Preference rules that set a high Z offset (say 4000) on image icons could be problematic; warn about them.
  // Only really care about image icons, since they have no Z depth and the offset Z moves them closer to
  // camera in a way that scales with dynamic scale and regular scale
  if (isImageModel_ && prefs.platpositionoffset().z() > 50.0 && (prefs.scale() > 1.0 || prefs.dynamicscale()))
  {
    const bool zOffsetChanged = PB_SUBFIELD_CHANGED(&prefs, &lastPrefs_, platpositionoffset, z);
    const bool scaleChanged = PB_FIELD_CHANGED(&prefs, &lastPrefs_, scale);
    const bool dynamicScaleChanged = PB_FIELD_CHANGED(&prefs, &lastPrefs_, dynamicscale);
    // Only warn when we get changes to the fields that might cause the warning, to avoid spamming
    if (zOffsetChanged || modelChanged || scaleChanged || dynamicScaleChanged)
    {
      // Pull out the name to identify the platform by checking alias.
      const auto commonPrefs = prefs.commonprefs();
      std::string name;
      if (commonPrefs.usealias() && !commonPrefs.alias().empty())
        name = commonPrefs.alias();
      else
        name = commonPrefs.name();

      // Warn the user
      SIM_WARN << "Platform [" << name << "]: Scaling image icon with large Z offset, image may disappear.  Validate Z offset.\n";
    }
  }

  bool needsBoundsUpdate = updateScale_(prefs, modelChanged) || modelChanged;
  updateImageIconRotation_(prefs, modelChanged);
  updateRCS_(prefs);
  needsBoundsUpdate = updateOffsets_(prefs) || needsBoundsUpdate;
  updateStippling_(prefs);
  updateCulling_(prefs);
  updatePolygonMode_(prefs);
  updateLighting_(prefs, modelChanged);
  updateOverrideColor_(prefs, modelChanged);
  updateAlphaVolume_(prefs);

  // Note that the brightness calculation is low cost and we do not check PB_FIELD_CHANGED on it
  const float brightnessMagnitude = prefs.brightness() * BRIGHTNESS_TO_AMBIENT;
  brightnessUniform_->set(osg::Vec4f(brightnessMagnitude, brightnessMagnitude, brightnessMagnitude, 1.f));

  if (needsBoundsUpdate)
    updateBounds_();

  lastPrefs_ = prefs;
  lastPrefsValid_ = true;
}
