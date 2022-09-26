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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/AutoTransform"
#include "osg/ComputeBoundsVisitor"
#include "osg/CullFace"
#include "osg/Depth"
#include "osg/Geode"
#include "osg/LOD"
#include "osg/PolygonMode"
#include "osg/ShapeDrawable"
#include "osg/CullStack"
#include "osg/Viewport"
#include "osgDB/ReadFile"
#include "osgEarth/Horizon"
#include "osgEarth/ObjectIndex"
#include "osgEarth/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simVis/Constants.h"
#include "simVis/DynamicScaleTransform.h"
#include "simVis/EntityLabel.h"
#include "simVis/ModelCache.h"
#include "simVis/Locator.h"
#include "simVis/OverrideColor.h"
#include "simVis/PolygonStipple.h"
#include "simVis/RCS.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/PlatformIconFactory.h"
#include "simVis/PlatformModel.h"

namespace simVis {

/** OSG Mask for traversal (like the select type in SIMDIS 9) */
const unsigned int PlatformModelNode::TRAVERSAL_MASK = simVis::DISPLAY_MASK_PLATFORM_MODEL;
/** Default brightness ambient value; 36 brightness is the default value */
static const osg::Vec4f DEFAULT_AMBIENT(
  36.f * BRIGHTNESS_TO_AMBIENT,
  36.f * BRIGHTNESS_TO_AMBIENT,
  36.f * BRIGHTNESS_TO_AMBIENT,
  1.f
  );

/** Callback to ModelCache that calls setModel() when the model is ready. */
class PlatformModelNode::SetModelCallback : public simVis::ModelCache::ModelReadyCallback
{
public:
  explicit SetModelCallback(PlatformModelNode* platform)
    : platform_(platform),
      ignoreResult_(false)
  {
  }
  virtual void loadFinished(const osg::ref_ptr<osg::Node>& model, bool isImage, const std::string& uri)
  {
    osg::ref_ptr<PlatformModelNode> refPlatform;
    if (platform_.lock(refPlatform) && !ignoreResult_)
      refPlatform->setModel_(model.get(), isImage);
  }

  void ignoreResult()
  {
    ignoreResult_ = true;
  }

private:
  osg::observer_ptr<PlatformModelNode> platform_;
  bool ignoreResult_;
};

/* OSG Scene Graph Layout of This Class
 *
 *       /= label_                           /= rcs_         /= alphaVolumeGroup_ => model_
 * this => dynamicXform_ => imageIconXform_ <=> offsetXform_ => model_
 *       \                                   \= other scaled children
 *        \                                  \= fastPathIcon_
 *         \
 *          \=> fixedDynamicXform_ => fixedImageIconXform_ => highlight
 *
 * model_ is the representative for the 3D model or 2D image, and may be set to nullptr at times.
 * It is set from the call to simVis::Registry::instance()->getOrCreateIconModel().
 *
 * The offsetXform_ handles transforms provided by the end user to orient and translate the model
 * correctly in the scene.  For example, a 90 degree yaw orientation can be used to fix models
 * that point the wrong way.  The offsets only apply to the model and not any of its attachments.
 * This matrix also implements a standard alignment option to 2D image icons, reference from the
 * position. It simply applies offsets to represent right/center/left and top/center/bottom
 * alignments to the model. The alignment offsets apply on top of other offset and rotation
 * adjustments, so if the rotation of the image icon is set to follow yaw, the image icon will apply
 * alignment with respect to yaw. The offsets only apply to the model and not any of its attachments.
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
 * Both fixedDynamicXform_ and fixedImageIconXform_ deal with dynamic scaling where the icon size
 * is replaced with a user supplied numerical value in meters.  This allows highlights of different
 * platforms to be the same size independent of the icon size.

 * PlatformModelNode is a locator, and is tied to the platform's position in space.
 *
 * alphaVolumeGroup_ is only on when the alphavolume() preference is on.  It does a second pass on
 * the platform model, this time drawing the backfaces.  This is useful for things like drawing the
 * interior of an alpha sphere used in error ellipses.
 *
 * fastPathIcon_ is an optimization path to help with rendering image-only platforms very quickly
 * by reducing state set changes.  It only is available for 2D icons.  It is mutually exclusive to
 * imageAlignmentXform_.  Only one or the other is active.
 */

PlatformModelNode::PlatformModelNode(Locator* locator)
  : LocatorNode(locator),
  isImageModel_(false),
  autoRotate_(false),
  lastPrefsValid_(false),
  brightnessUniform_(new osg::Uniform(LIGHT0_AMBIENT_COLOR.c_str(), DEFAULT_AMBIENT)),
  objectIndexTag_(0),
  allowFastPath_(true)
{
  // EntityLabelNode for platformModel is a special case - a locatorNode with no locator; it gets its location from parent, the platformmodelnode (which is a locatorNode).
  label_ = new EntityLabelNode();

  setName("PlatformModel");
  setNodeMask(getMask());

  // Provides an icon orientation and position on the model
  offsetXform_ = new osg::MatrixTransform();
  offsetXform_->setName("offsetXform");

  // Apply the override color shader to the container
  overrideColor_ = new simVis::OverrideColor(offsetXform_->getOrCreateStateSet());

  // Set up the transform responsible for rotating the 2-D image icons
  imageIconXform_ = new BillboardAutoTransform();
  imageIconXform_->setAutoScaleToScreen(false);
  imageIconXform_->setAutoRotateMode(BillboardAutoTransform::NO_ROTATION);
  imageIconXform_->setName("imageIconXform");
  imageIconXform_->dirty();

  // Horizon culler for the platform (and its label). The culler is attached to this node,
  // but uses the imageIconXform for the actual testing.
  osgEarth::HorizonCullCallback* hcc = new osgEarth::HorizonCullCallback();
  hcc->setCullByCenterPointOnly(true);
  hcc->setProxyNode(imageIconXform_.get());
  hcc->setName("HorizonCullCallback");
  addCullCallback(hcc);

  // used to apply both dynamic and static scaling to the model.
  dynamicXform_ = new simVis::DynamicScaleTransform();
  dynamicXform_->setName("dynamicXform");

  // Configure children graph
  addChild(label_);
  addChild(dynamicXform_);
  dynamicXform_->addChild(imageIconXform_);
  imageIconXform_->addChild(offsetXform_);

  // Set up the brightness factor for the entity, attaching close to the model
  offsetXform_->getOrCreateStateSet()->addUniform(brightnessUniform_, osg::StateAttribute::ON);

  // Tag the platform at the lowest unique level feasible
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(offsetXform_.get(), offsetXform_.get());

  // When alpha volume is on, we turn on this node
  alphaVolumeGroup_ = new osg::Group;
  alphaVolumeGroup_->setName("Alpha Volume Group");
  offsetXform_->addChild(alphaVolumeGroup_);
  alphaVolumeGroup_->setNodeMask(0); // off by default
  // Draw the backface
  osg::ref_ptr<osg::CullFace> face = new osg::CullFace(osg::CullFace::FRONT);
  alphaVolumeGroup_->getOrCreateStateSet()->setAttributeAndModes(face.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

  // Set an initial model.  Without this, visitors expecting a node may fail early.
  setModel_(simVis::Registry::instance()->modelCache()->boxNode(), false);
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
  return offsetXform_.get();
}

unsigned int PlatformModelNode::objectIndexTag() const
{
  return objectIndexTag_;
}

bool PlatformModelNode::addScaledChild(osg::Node* node)
{
  // Scaled children go into the imageIconXform_, so that the orientation fixes
  // for the model do not accidentally swap the location/orientation of attachments
  if (imageIconXform_ == nullptr)
    return false;
  return imageIconXform_->addChild(node);
}

bool PlatformModelNode::removeScaledChild(osg::Node* node)
{
  if (imageIconXform_ == nullptr)
    return false;
  return imageIconXform_->removeChild(node);
}

bool PlatformModelNode::addFixedScaledChild(osg::Node* node)
{
  if (fixedImageIconXform_ == nullptr)
  {
    // Set up the transform responsible for rotating the 2-D image icons
    fixedImageIconXform_ = new BillboardAutoTransform();
    fixedImageIconXform_->setAutoScaleToScreen(false);
    fixedImageIconXform_->setAutoRotateMode(autoRotate_ ? osg::AutoTransform::ROTATE_TO_SCREEN : BillboardAutoTransform::NO_ROTATION);
    fixedImageIconXform_->setRotation(osg::Quat());
    fixedImageIconXform_->setName("fixedImageIconXform");
    if (!isImageModel_)
      fixedImageIconXform_->setRotateInScreenSpace(false);
    else
      fixedImageIconXform_->setRotateInScreenSpace(lastPrefs_.rotateicons() == simData::IR_2D_YAW);
    fixedImageIconXform_->dirty();

    //TODO: Is a HorizonCullCallback needed for fixedImageIconXform_?

    fixedDynamicXform_ = new simVis::DynamicScaleTransform();
    fixedDynamicXform_->setName("fixedDynamicXform");
    fixedDynamicXform_->setDynamicScaleToPixels(false);

    if (lastPrefs_.has_scalexyz())
      fixedDynamicXform_->setOverrideScale(osg::Vec3d(lastPrefs_.scalexyz().y(), lastPrefs_.scalexyz().x(), lastPrefs_.scalexyz().z()));
    else
    {
      fixedDynamicXform_->setDynamicScalingEnabled(lastPrefs_.dynamicscale());
      fixedDynamicXform_->setStaticScalar(lastPrefs_.scale());
      if (lastPrefs_.dynamicscale())
      {
        fixedDynamicXform_->setDynamicScalar(lastPrefs_.dynamicscalescalar());
        fixedDynamicXform_->setScaleOffset(lastPrefs_.dynamicscaleoffset());
      }
    }

    addChild(fixedDynamicXform_);
    fixedDynamicXform_->addChild(fixedImageIconXform_);
  }

  return fixedImageIconXform_->addChild(node);
}

bool PlatformModelNode::removeFixedScaledChild(osg::Node* node)
{
  if (fixedImageIconXform_ == nullptr)
    return false;

  bool rv = fixedImageIconXform_->removeChild(node);

  if (fixedImageIconXform_->getNumChildren() == 0)
  {
    fixedDynamicXform_->removeChild(fixedImageIconXform_.get());
    removeChild(fixedDynamicXform_.get());
    fixedDynamicXform_ = nullptr;
    fixedImageIconXform_ = nullptr;
  }

  return rv;
}

void PlatformModelNode::setFixedSize(double meters)
{
  if (fixedDynamicXform_ == nullptr)
    return;

  fixedDynamicXform_->setFixedSize(meters);

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
    if (fixedImageIconXform_.valid())
      fixedImageIconXform_->setScreenSpaceRotation(c.yaw());
  }
}

bool PlatformModelNode::updateModel_(const simData::PlatformPrefs& prefs)
{
  // Early return for fast path icon
  ModelUpdate modelUpdate = allowFastPath_ ? updateFastPathModel_(prefs) : NO_UPDATE;
  if (modelUpdate == NO_UPDATE)
    return true;

  // Slower (normal) speed path for icons -- only look at the icon() preference
  if ((modelUpdate == CHECK_FOR_UPDATE) &&
      lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, icon))
    return false;

  const simVis::Registry* registry = simVis::Registry::instance();
  if (prefs.icon().empty())
    setModel_(nullptr, false);
  else if (!registry->isMemoryCheck())
  {
    // Find the fully qualified URI
    const std::string uri = registry->findModelFile(prefs.icon());
    // Perform an asynchronous load on the model
    if (uri.empty())
    {
      SIM_WARN << "Failed to find icon model: " << prefs.icon() << "\n";
      setModel_(simVis::Registry::instance()->modelCache()->boxNode(), false);
    }
    else
    {
      // Kill off any pending async model loads
      if (lastSetModelCallback_.valid())
        lastSetModelCallback_->ignoreResult();
      lastSetModelCallback_ = new SetModelCallback(this);
      registry->modelCache()->asyncLoad(uri, lastSetModelCallback_.get());
    }
  }
  return true;
}

PlatformModelNode::ModelUpdate PlatformModelNode::updateFastPathModel_(const simData::PlatformPrefs& prefs)
{
  PlatformIconFactory* iconFactory = PlatformIconFactory::instance();
  // Attempt to use the simplified/optimized 2D icon path
  if (lastPrefsValid_ && !iconFactory->hasRelevantChanges(lastPrefs_, prefs))
    return NO_UPDATE;

  // This is potentially a synchronous load of a 2D icon, which means we don't have to worry
  // about thread interactions. If it succeeds and we need to change the icon, then the change
  // is instantaneous. We'll have to update model sizes and other internal fields manually,
  // because we cannot call setModel() in this use case. setModel() overrides the fast-path.
  osg::ref_ptr<osg::Node> newIcon = iconFactory->getOrCreate(prefs);

  // Only need to make changes if the icon is different from the old one
  if (newIcon.get() == fastPathIcon_.get())
    return fastPathIcon_.valid() ? NO_UPDATE : FORCE_UPDATE;

  // Remove old fast path icon (if applicable)
  if (fastPathIcon_.valid())
    imageIconXform_->removeChild(fastPathIcon_.get());
  // Assign and add the new fast path icon
  fastPathIcon_ = newIcon;
  const bool fastPathValid = fastPathIcon_.valid();

  // If the fast path icon is not valid, then we need to force a load of the slow/normal path icon in caller
  if (!fastPathValid)
  {
    offsetXform_->setNodeMask(getMask());
    return FORCE_UPDATE;
  }

  fastPathIcon_->setNodeMask(getMask());
  imageIconXform_->addChild(fastPathIcon_.get());

  // Do not set model_, so that a state set is not created on it. The icon
  // factory takes care of all model_-based prefs.
  if (model_.valid())
  {
    offsetXform_->removeChild(model_);
    alphaVolumeGroup_->removeChild(model_);
    model_ = nullptr;
    dynamicXform_->setSizingNode(nullptr);
  }

  // The factory can only return 2D images
  isImageModel_ = true;

  // Save the image original size
  osg::ComputeBoundsVisitor cb;
  fastPathIcon_->accept(cb);
  const osg::BoundingBox& bounds = cb.getBoundingBox();
  imageOriginalSize_.x() = bounds.xMax() - bounds.xMin();
  imageOriginalSize_.y() = bounds.yMax() - bounds.yMin();

  // Fix the sizing node for the dynamic transform to avoid initial very-large icons.
  dynamicXform_->setDynamicScaleToPixels(isImageModel_ && prefs.dynamicscalealgorithm() == simData::DSA_METERS_TO_PIXELS);
  dynamicXform_->setSizingNode(fastPathIcon_.get());

  // Kill off any pending async model loads
  if (lastSetModelCallback_.valid())
    lastSetModelCallback_->ignoreResult();

  // Either use the fast-path icon, or use the more featured, slower path in offsetXform_; don't use both
  offsetXform_->setNodeMask(fastPathValid ? 0u : getMask());
  return fastPathValid ? NO_UPDATE : CHECK_FOR_UPDATE;
}

void PlatformModelNode::setModel(osg::Node* node, bool isImage)
{
  // If necessary turn off fast draw
  if (fastPathIcon_.valid())
  {
    imageIconXform_->removeChild(fastPathIcon_.get());
    fastPathIcon_ = nullptr;
    offsetXform_->setNodeMask(getMask());
  }

  allowFastPath_ = false;
  setModel_(node, isImage);
}

void PlatformModelNode::setModel_(osg::Node* newModel, bool isImage)
{
  if (model_ == newModel && isImageModel_ == isImage)
    return;

  // Kill off any pending async model loads
  if (lastSetModelCallback_.valid())
  {
    lastSetModelCallback_->ignoreResult();
    lastSetModelCallback_ = nullptr;
  }

  isImageModel_ = isImage;

  // Remove any existing model
  if (model_.valid())
  {
    offsetXform_->removeChild(model_);
    alphaVolumeGroup_->removeChild(model_);
    model_ = nullptr;
    dynamicXform_->setSizingNode(nullptr);
  }

  // if the new properties say "no model", we're done
  model_ = newModel;
  if (newModel != nullptr)
  {
    // Set render order by setting render bin.  We set the OVERRIDE flag in case the model has renderbins set inside of it.
    // Note that osg::Depth settings are not changed here, but are instead dealt with inside updateImageDepth_() call below.
    osg::StateSet* modelStateSet = model_->getOrCreateStateSet();
    if (isImageModel_)
      modelStateSet->setRenderBinDetails(simVis::BIN_PLATFORM_IMAGE, simVis::BIN_TWO_PASS_ALPHA, osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
    else
      modelStateSet->setRenderBinDetails(simVis::BIN_PLATFORM_MODEL, simVis::BIN_TRAVERSAL_ORDER_SIMSDK);

    // re-add to the parent groups
    offsetXform_->addChild(model_.get());
    alphaVolumeGroup_->addChild(model_.get());
    if (lastPrefsValid_)
      dynamicXform_->setDynamicScaleToPixels(isImageModel_ && lastPrefs_.dynamicscalealgorithm() == simData::DSA_METERS_TO_PIXELS);
    dynamicXform_->setSizingNode(model_.get());
  }

  // for image models, cache the original size
  if (isImage)
  {
    osg::ComputeBoundsVisitor cb;
    cb.setTraversalMask(cb.getTraversalMask() | ~simVis::DISPLAY_MASK_LABEL);
    offsetXform_->accept(cb);
    const osg::BoundingBox& bounds = cb.getBoundingBox();
    imageOriginalSize_.x() = bounds.xMax() - bounds.xMin();
    imageOriginalSize_.y() = bounds.yMax() - bounds.yMin();
  }

  // Update various prefs that affect the model state and the bounding box.  If the lastPrefs_ is not
  // valid, then this code will just get called again in setPrefs(), so it's safe to call here.
  updateImageDepth_(lastPrefs_, true);
  warnOnInvalidOffsets_(lastPrefs_, true);
  updateImageIconRotation_(lastPrefs_, true);
  updateLighting_(lastPrefs_, true);
  updateOffsets_(lastPrefs_, true);
  updateBounds_();
  updateDofTransform_(lastPrefs_, true);
}

void PlatformModelNode::setRotateToScreen(bool value)
{
  autoRotate_ = value;

  if (autoRotate_)
  {
    imageIconXform_->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
    if (fixedImageIconXform_.valid())
      fixedImageIconXform_->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  }
  else
  {
    imageIconXform_->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
    imageIconXform_->setRotation(osg::Quat());
    if (fixedImageIconXform_.valid())
    {
      fixedImageIconXform_->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
      fixedImageIconXform_->setRotation(osg::Quat());
    }
  }
  imageIconXform_->dirty();
  if (fixedImageIconXform_.valid())
    fixedImageIconXform_->dirty();
}

bool PlatformModelNode::updateOffsets_(const simData::PlatformPrefs& prefs, bool force)
{
  if (!force && lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, platpositionoffset) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, iconalignment) &&
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

  // Adjust image icons by the correction for the hot spot (icon alignment)
  if (isImageModel_)
  {
    osg::Vec2f xyOffset;
    simVis::iconAlignmentToOffsets(prefs.iconalignment(), imageOriginalSize_, xyOffset);
    offsetMatrix.preMultTranslate(osg::Vec3f(xyOffset, 0.f));
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
  // compute bounds based on the offsetXform_, which is the parent of the model.
  if (fastPathIcon_)
    fastPathIcon_->accept(cb);
  else
    offsetXform_->accept(cb);
  unscaledBounds_ = cb.getBoundingBox();

  // Now get the scaled bounds
  cb.reset();
  dynamicXform_->accept(cb);
  bounds_ = cb.getBoundingBox();

  // add rcs back to the image transform
  if (rcs_.valid())
  {
    imageIconXform_->addChild(rcs_.get());
    // Adjust the RCS scale at this point too
    rcs_->setScale(model_->getBound().radius() * 2.0);
  }

  // add children back to model container in original order
  offsetXform_->removeChild(model_);
  for (osg::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    offsetXform_->addChild(*iter);
  }

  // Alert any listeners of bounds changes
  fireCallbacks_(Callback::BOUNDS_CHANGED);
}

bool PlatformModelNode::updateScale_(const simData::PlatformPrefs& prefs)
{
  // Check for ScaleXYZ first
  if (prefs.has_scalexyz())
    return updateScaleXyz_(prefs);

  // Clear out the override scaling at this point so latent values don't take over
  dynamicXform_->clearOverrideScale();
  if (fixedDynamicXform_.valid())
    fixedDynamicXform_->clearOverrideScale();
  return updateDynamicScale_(prefs);
}

bool PlatformModelNode::updateScaleXyz_(const simData::PlatformPrefs& prefs)
{
  // By default scalexyz is nullptr. It is used to override the scale preference which is defaulted to 1. When uninitialized, it should not override the scale pref.
  if (!prefs.has_scalexyz() || (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, scalexyz)))
    return false;

  // update the static scaling using the scaleXYZ pref
  dynamicXform_->setOverrideScale(osg::Vec3d(prefs.scalexyz().y(), prefs.scalexyz().x(), prefs.scalexyz().z()));
  if (fixedDynamicXform_.valid())
    fixedDynamicXform_->setOverrideScale(osg::Vec3d(prefs.scalexyz().y(), prefs.scalexyz().x(), prefs.scalexyz().z()));
  return true;
}

bool PlatformModelNode::updateDynamicScale_(const simData::PlatformPrefs& prefs)
{
  if (lastPrefsValid_ &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, scale) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscale) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscalescalar) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscaleoffset) &&
      !PB_FIELD_CHANGED(&lastPrefs_, &prefs, dynamicscalealgorithm))
    return false;

  const bool ds = prefs.dynamicscale();

  dynamicXform_->setDynamicScalingEnabled(ds);
  if (fixedDynamicXform_.valid())
    fixedDynamicXform_->setDynamicScalingEnabled(ds);
  // Scale applies whether dynamic scaling is enabled or static scaling is enabled
  dynamicXform_->setStaticScalar(prefs.scale());
  if (fixedDynamicXform_.valid())
    fixedDynamicXform_->setStaticScalar(prefs.scale());
  // Scale scalar, scale offset, and algorithm only apply when dynamic scaling is on
  if (ds)
  {
    dynamicXform_->setDynamicScalar(prefs.dynamicscalescalar());
    dynamicXform_->setScaleOffset(prefs.dynamicscaleoffset());
    dynamicXform_->setDynamicScaleToPixels(isImageModel_ && prefs.dynamicscalealgorithm() == simData::DSA_METERS_TO_PIXELS);
    if (fixedDynamicXform_.valid())
    {
      fixedDynamicXform_->setDynamicScalar(prefs.dynamicscalescalar());
      fixedDynamicXform_->setScaleOffset(prefs.dynamicscaleoffset());
    }
  }

  return true;
}

void PlatformModelNode::updateImageDepth_(const simData::PlatformPrefs& prefs, bool force) const
{
  if (!offsetXform_.valid())
    return;

  if (force || PB_FIELD_CHANGED(&prefs, &lastPrefs_, nodepthicons))
  {
    osg::StateSet* state = offsetXform_->getOrCreateStateSet();
    state->removeAttribute(osg::StateAttribute::DEPTH);
    if (!isImageModel_)
      return;
    // image models need to always pass depth test if nodepthicons is set to true
    if (prefs.nodepthicons() && isImageModel_)
    {
      osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::ALWAYS, 0, 1, true);
      state->setAttributeAndModes(depth.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
    }
    else
    {
      osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::LESS, 0, 1, true);
      state->setAttributeAndModes(depth.get(), osg::StateAttribute::ON);
    }
  }
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
    if (fixedImageIconXform_.valid())
      fixedImageIconXform_->setRotateInScreenSpace(false);
    // Reset components to inherit
    setLocator(getLocator(),
      (getLocator()->getComponentsToInherit() | simVis::Locator::COMP_ORIENTATION));
    return;
  }

  setRotateToScreen(prefs.rotateicons() == simData::IR_2D_UP);
  imageIconXform_->setRotateInScreenSpace(prefs.rotateicons() == simData::IR_2D_YAW);
  if (fixedImageIconXform_.valid())
    fixedImageIconXform_->setRotateInScreenSpace(prefs.rotateicons() == simData::IR_2D_YAW);

  if (prefs.rotateicons() == simData::IR_3D_YPR)
  {
    setLocator(getLocator(),
      (getLocator()->getComponentsToInherit() | simVis::Locator::COMP_ORIENTATION));
  }
  else if (prefs.rotateicons() == simData::IR_3D_YAW || prefs.rotateicons() == simData::IR_2D_YAW)
  {
    unsigned int mask = getLocator()->getComponentsToInherit();
    mask &= ~Locator::COMP_ORIENTATION;
    mask |=  Locator::COMP_HEADING;
    setLocator(getLocator(), mask);
  }
  else if (prefs.rotateicons() == simData::IR_3D_NORTH || prefs.rotateicons() == simData::IR_2D_UP)
  {
    unsigned int mask = getLocator()->getComponentsToInherit();
    mask &= ~Locator::COMP_ORIENTATION;
    setLocator(getLocator(), mask);
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
    rcs_ = nullptr;
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

  if (!offsetXform_.valid())
    return;
  osg::observer_ptr<osg::StateSet> stateSet = offsetXform_->getStateSet();

  // Polygon stipple index in protobuf is off-by-one
  unsigned int patternIndex = prefs.polygonstipple();
  if (patternIndex > 0)
    --patternIndex;
  simVis::PolygonStipple::setValues(stateSet.get(), prefs.usepolygonstipple(), patternIndex);
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
    {
      osg::ref_ptr<osg::CullFace> face = new osg::CullFace(osg::CullFace::FRONT);
      stateSet->setAttributeAndModes(face.get(), osg::StateAttribute::ON);
      break;
    }
    case simData::BACK:
    {
      osg::ref_ptr<osg::CullFace> face = new osg::CullFace(osg::CullFace::BACK);
      stateSet->setAttributeAndModes(face.get(), osg::StateAttribute::ON);
      break;
    }
    case simData::FRONT_AND_BACK:
    {
      osg::ref_ptr<osg::CullFace> face = new osg::CullFace(osg::CullFace::FRONT_AND_BACK);
      stateSet->setAttributeAndModes(face.get(), osg::StateAttribute::ON);
      break;
    }
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

  if (!offsetXform_.valid())
    return;
  osg::observer_ptr<osg::StateSet> stateSet = offsetXform_->getStateSet();

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
  osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode(face, mode);
  stateSet->setAttributeAndModes(polygonMode.get(), osg::StateAttribute::ON);
}

void PlatformModelNode::updateLighting_(const simData::PlatformPrefs& prefs, bool force)
{
  if (!offsetXform_.valid())
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

void PlatformModelNode::updateOverrideColor_(const simData::PlatformPrefs& prefs)
{
  if (!overrideColor_.valid())
    return;

  if (lastPrefsValid_ &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, useoverridecolor) &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, overridecolor) &&
    !PB_FIELD_CHANGED(&lastPrefs_, &prefs, overridecolorcombinemode))
    return;

  // using an override color?
  overrideColor_->setColor(simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA));
  if (!prefs.commonprefs().useoverridecolor())
    overrideColor_->setCombineMode(OverrideColor::OFF);
  else
  {
    if (prefs.overridecolorcombinemode() == simData::REPLACE_COLOR)
      overrideColor_->setCombineMode(OverrideColor::REPLACE_COLOR);
    else
      overrideColor_->setCombineMode(OverrideColor::MULTIPLY_COLOR);
  }
}

void PlatformModelNode::updateAlphaVolume_(const simData::PlatformPrefs& prefs)
{
  if (isImageModel_ || (lastPrefsValid_ && !PB_FIELD_CHANGED(&lastPrefs_, &prefs, alphavolume)))
    return;

  if (prefs.alphavolume())
  {
    // Turn off depth writes
    osg::ref_ptr<osg::Depth> depth = new osg::Depth(osg::Depth::LESS, 0, 1, false);
    offsetXform_->getOrCreateStateSet()->setAttributeAndModes(depth.get());
    alphaVolumeGroup_->setNodeMask(getMask());
  }
  else
  {
    offsetXform_->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::DEPTH);
    alphaVolumeGroup_->setNodeMask(0);
  }
}

void PlatformModelNode::updateDofTransform_(const simData::PlatformPrefs& prefs, bool force) const
{
  // Don't need to apply to image models
  if (!model_.valid() || isImageModel_)
    return;
  // Don't execute if field did not change, unless we're being forced due to model loading
  if (!force && lastPrefsValid_ && !PB_FIELD_CHANGED(&lastPrefs_, &prefs, animatedofnodes))
    return;
  simVis::EnableDOFTransform enableDof(prefs.animatedofnodes());
  model_->accept(enableDof);
}

void PlatformModelNode::setProperties(const simData::PlatformProperties& props)
{
  lastProps_ = props;
}

void PlatformModelNode::setPrefs(const simData::PlatformPrefs& prefs)
{
  // If a new model is detected, start loading it.
  const bool modelChanged = updateModel_(prefs);

  // check for updates to the nodepthicon pref
  updateImageDepth_(prefs, false);

  // Only warn on the invalid offsets if the model didn't change.  If the model DID change,
  // then the changing of the model already deals with the print out.  There are parts of the
  // prefs data structure that can impact this printout so it needs to be both here and
  // in the code that changes the model icon.
  if (!modelChanged)
    warnOnInvalidOffsets_(prefs, false);

  bool needsBoundsUpdate = updateScale_(prefs);
  updateImageIconRotation_(prefs, false);
  updateRCS_(prefs);
  needsBoundsUpdate = updateOffsets_(prefs, false) || needsBoundsUpdate;
  updateStippling_(prefs);
  updateCulling_(prefs);
  updatePolygonMode_(prefs);
  updateLighting_(prefs, false);
  updateOverrideColor_(prefs);
  updateAlphaVolume_(prefs);
  updateDofTransform_(prefs, false);

  // Note that the brightness calculation is low cost, but setting brightness uniform is not necessarily low-cost, so we do check PB_FIELD_CHANGED
  if (PB_FIELD_CHANGED(&prefs, &lastPrefs_, brightness))
  {
    const float brightnessMagnitude = prefs.brightness() * BRIGHTNESS_TO_AMBIENT;
    brightnessUniform_->set(osg::Vec4f(brightnessMagnitude, brightnessMagnitude, brightnessMagnitude, 1.f));
  }

  if (needsBoundsUpdate)
    updateBounds_();

  lastPrefs_ = prefs;
  lastPrefsValid_ = true;
}

void PlatformModelNode::warnOnInvalidOffsets_(const simData::PlatformPrefs& prefs, bool modelChanged) const
{
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
}

void PlatformModelNode::addCallback(Callback* value)
{
  if (value)
    callbacks_.push_back(value);
}

void PlatformModelNode::removeCallback(Callback* value)
{
  if (value)
    callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), value), callbacks_.end());
}

void PlatformModelNode::fireCallbacks_(Callback::EventType eventType)
{
  for (auto i = callbacks_.begin(); i != callbacks_.end(); ++i)
    i->get()->operator()(this, eventType);
}

}
