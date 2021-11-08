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
#include "osg/MatrixTransform"
#include "osgEarth/Horizon"
#include "osgEarth/ObjectIndex"
#include "simNotify/Notify.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Math.h"
#include "simVis/Antenna.h"
#include "simVis/BeamPulse.h"
#include "simVis/Constants.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/Types.h"
#include "simVis/SphericalVolume.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/Beam.h"

#define BEAM_IN_PLACE_UPDATES

// --------------------------------------------------------------------------

namespace
{
  /// check for changes that require us to rebuild the entire beam.
  bool changeRequiresRebuild(const simData::BeamPrefs* a, const simData::BeamPrefs* b)
  {
    if (a == nullptr || b == nullptr)
      return false;
    if (PB_FIELD_CHANGED(a, b, drawtype))
      return true;
    if (PB_FIELD_CHANGED(a, b, polarity) ||
        PB_FIELD_CHANGED(a, b, colorscale) ||
        PB_FIELD_CHANGED(a, b, detail) ||
        PB_FIELD_CHANGED(a, b, gain) ||
        PB_FIELD_CHANGED(a, b, frequency) ||
        PB_FIELD_CHANGED(a, b, power) ||
        PB_FIELD_CHANGED(a, b, fieldofview) ||
        PB_FIELD_CHANGED(a, b, sensitivity) ||
        PB_FIELD_CHANGED(a, b, rendercone) ||
        PB_FIELD_CHANGED(a, b, coneresolution) ||
        PB_FIELD_CHANGED(a, b, capresolution) ||
        PB_FIELD_CHANGED(a, b, beamdrawmode))
      return true;

#ifndef BEAM_IN_PLACE_UPDATES
    return (PB_FIELD_CHANGED(a, b, verticalwidth) ||
      PB_FIELD_CHANGED(a, b, horizontalwidth));
#else
    if (b->rendercone() && PB_FIELD_CHANGED(a, b, horizontalwidth))
    {
      // manage automatic change to/from cone/pyramid when horizontalwidth crosses PI threshold
      return ((a->horizontalwidth() <= M_PI && b->horizontalwidth() > M_PI) ||
        (a->horizontalwidth() > M_PI && b->horizontalwidth() <= M_PI));
    }
    return false;
#endif
  }

  /// Some updates can require a rebuild too
  bool changeRequiresRebuild(const simData::BeamUpdate* a, const simData::BeamUpdate* b)
  {
#ifdef BEAM_IN_PLACE_UPDATES
    return false;
#else
    if (a == nullptr || b == nullptr)
      return false;
    return PB_FIELD_CHANGED(a, b, range);
#endif
  }
}

// --------------------------------------------------------------------------

namespace simVis
{
BeamVolume::BeamVolume(const simData::BeamPrefs& prefs, const simData::BeamUpdate& update)
{
  setName("Beam Volume");
  beamSV_ = createBeamSV_(prefs, update);
  addChild(beamSV_);
  setBeamScale_(prefs.beamscale());

  // if blended, use BIN_BEAM & TPA, otherwise use BIN_OPAQUE_BEAM & BIN_GLOBAL_SIMSDK
  osg::Geometry* solidGeometry = simVis::SVFactory::solidGeometry(beamSV_.get());
  if (solidGeometry != nullptr)
  {
    solidGeometry->getOrCreateStateSet()->setRenderBinDetails(
      (prefs.blended() ? BIN_BEAM : BIN_OPAQUE_BEAM),
      (prefs.blended() ? BIN_TWO_PASS_ALPHA : BIN_GLOBAL_SIMSDK));
  }

  // if there is a wireframe/2nd group, it should be renderbin'd to BIN_OPAQUE_BEAM
  osg::Group* wireframeGroup = simVis::SVFactory::opaqueGroup(beamSV_.get());
  if (wireframeGroup != nullptr)
  {
    // SphericalVolume code only adds the opaque geode when it is adding a geometry or lineGroup
    assert(wireframeGroup->getNumChildren() > 0);
    wireframeGroup->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_BEAM, BIN_GLOBAL_SIMSDK);
  }
}

osg::MatrixTransform* BeamVolume::createBeamSV_(const simData::BeamPrefs& prefs, const simData::BeamUpdate& update)
{
  simVis::SVData sv;

  // defaults:
  sv.color_.set(1, 1, 0, 0.5);
  sv.shape_ = simVis::SVData::SHAPE_CONE;

  if (update.has_range())
  {
    sv.farRange_ = update.range();
  }

  if (prefs.has_horizontalwidth())
    sv.hfov_deg_ = osg::RadiansToDegrees(prefs.horizontalwidth());
  if (prefs.has_verticalwidth())
    sv.vfov_deg_ = osg::RadiansToDegrees(prefs.verticalwidth());

  if (prefs.commonprefs().useoverridecolor())
    sv.color_ = simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA);
  else
    sv.color_ = simVis::Color(prefs.commonprefs().color(), simVis::Color::RGBA);

  sv.blendingEnabled_ = prefs.blended();
  sv.lightingEnabled_ = prefs.shaded();

  // draw as pyramid when hbw > 180
  sv.shape_ = (prefs.rendercone() && sv.hfov_deg_ <= 180.) ? simVis::SVData::SHAPE_CONE : simVis::SVData::SHAPE_PYRAMID;

  // if drawing as a pyramid, coneRes_ is not used, but wallRes_ is used
  sv.coneRes_ = prefs.coneresolution();
  sv.wallRes_ = sv.coneRes_;
  sv.capRes_ = prefs.capresolution();

  sv.drawMode_ =
    prefs.beamdrawmode() == simData::BeamPrefs::WIRE ? simVis::SVData::DRAW_MODE_WIRE :
    prefs.beamdrawmode() == simData::BeamPrefs::SOLID ? simVis::SVData::DRAW_MODE_SOLID :
    (simVis::SVData::DRAW_MODE_SOLID | simVis::SVData::DRAW_MODE_WIRE);

  // only the cap is drawn in coverage draw type
  sv.drawCone_ = prefs.drawtype() != simData::BeamPrefs_DrawType_COVERAGE;

  // use a "Y-forward" direction vector because the Beam is drawn in ENU LTP space.
  return simVis::SVFactory::createNode(sv, osg::Y_AXIS);
}

void BeamVolume::setBeamScale_(double beamScale)
{
  osg::Matrix m = beamSV_->getMatrix();
  const osg::Vec3d currentScale = m.getScale();
  if (currentScale.x() > 0.0)
  {
    const double s = beamScale / currentScale.x();   // undo the old, apply the new.
    m.preMultScale(osg::Vec3d(s, s, s));
    beamSV_->setMatrix(m);
  }
}

/// update prefs that can be updated without rebuilding the whole beam.
void BeamVolume::performInPlacePrefChanges(const simData::BeamPrefs* a, const simData::BeamPrefs* b)
{
  if (a == nullptr || b == nullptr)
    return;

  if (b->commonprefs().has_useoverridecolor() && b->commonprefs().useoverridecolor())
  {
    // Check for transition between color and override color, then check for color change
    if (PB_SUBFIELD_CHANGED(a, b, commonprefs, useoverridecolor) || PB_SUBFIELD_CHANGED(a, b, commonprefs, overridecolor))
    {
      SVFactory::updateColor(beamSV_.get(), simVis::Color(b->commonprefs().overridecolor(), simVis::Color::RGBA));
    }
  }
  else
  {
    // Check for transition between color and override color, then check for color change
    if ((a->commonprefs().has_useoverridecolor() && a->commonprefs().useoverridecolor()) || PB_SUBFIELD_CHANGED(a, b, commonprefs, color))
    {
      SVFactory::updateColor(beamSV_.get(), simVis::Color(b->commonprefs().color(), simVis::Color::RGBA));
    }
  }
  if (PB_FIELD_CHANGED(a, b, shaded))
    SVFactory::updateLighting(beamSV_.get(), b->shaded());
  if (PB_FIELD_CHANGED(a, b, blended))
  {
    // if blended, use BIN_BEAM & TPA, otherwise use BIN_OPAQUE_BEAM & BIN_GLOBAL_SIMSDK
    osg::Geometry* solidGeometry = simVis::SVFactory::solidGeometry(beamSV_.get());
    if (solidGeometry != nullptr)
    {
      solidGeometry->getOrCreateStateSet()->setRenderBinDetails(
        (b->blended() ? BIN_BEAM : BIN_OPAQUE_BEAM),
        (b->blended() ? BIN_TWO_PASS_ALPHA : BIN_GLOBAL_SIMSDK));
    }
    SVFactory::updateBlending(beamSV_.get(), b->blended());
  }
#ifdef BEAM_IN_PLACE_UPDATES
  if (PB_FIELD_CHANGED(a, b, verticalwidth))
    SVFactory::updateVertAngle(beamSV_.get(), a->verticalwidth(), b->verticalwidth());
  if (PB_FIELD_CHANGED(a, b, horizontalwidth))
    SVFactory::updateHorizAngle(beamSV_.get(), a->horizontalwidth(), b->horizontalwidth());
#endif
  if (PB_FIELD_CHANGED(a, b, beamscale))
    setBeamScale_(b->beamscale());
}

void BeamVolume::performInPlaceUpdates(const simData::BeamUpdate* a, const simData::BeamUpdate* b)
{
  if (a == nullptr || b == nullptr)
    return;

#ifdef BEAM_IN_PLACE_UPDATES
  // the update method calls dirtyBound on all beam volume geometries, so no need for that here
  if (PB_FIELD_CHANGED(a, b, range))
  {
    SVFactory::updateFarRange(beamSV_.get(), b->range());
  }
#endif
}

// --------------------------------------------------------------------------

BeamNode::BeamNode(const simData::BeamProperties& props, Locator* hostLocator, const EntityNode* host, int referenceYear)
  : EntityNode(simData::BEAM),
    hasLastUpdate_(false),
    hasLastPrefs_(false),
    host_(host),
    hostMissileOffset_(0.0),
    objectIndexTag_(0)
{
  lastProps_ = props;

  // inherit the host platform's pos and ori, and add a body-local position offset.
  beamOriginLocator_ = new Locator(hostLocator, Locator::COMP_ALL);

  // if the properties call for a body-relative beam, configure that:
  if (props.has_type() && props.type() == simData::BeamProperties_BeamType_BODY_RELATIVE)
  {
    // in the BeamType_BODY_RELATIVE case, beam data is relative to platform orientation;
    // the ResolvedPositionOrientationLocator maintains the host platform pos and ori
    //  orientation data + offsets applied to this locator -will- be relative to host platform orientation
    beamOrientationLocator_ = new ResolvedPositionOrientationLocator(beamOriginLocator_.get(), Locator::COMP_ALL);
  }
  else
  {
    // for non-relative beams, we need to apply position offsets that are relative to platform orientation.
    // after having established the position offset,
    // we need to apply an orientation that is not relative to platform orientation : we need to filter out platform orientation.
    // the ResolvedPositionLocator gives us that.
    // orientation data + offsets applied to this locator -will-not- be relative to host platform orientation
    beamOrientationLocator_ = new ResolvedPositionLocator(beamOriginLocator_.get(), Locator::COMP_ALL);
  }
  setLocator(beamOrientationLocator_.get());
  setName("BeamNode");

  // create the locator node that will parent the geometry and label
  beamLocatorNode_ = new LocatorNode(getLocator());
  beamLocatorNode_->setName("Beam Locator");
  beamLocatorNode_->setEntityToMonitor(this);
  addChild(beamLocatorNode_.get());

  // create localGrid_ after beamLocatorNode_ so beamLocatorNode_ is found in findAttachment() for tethering
  localGrid_ = new LocalGridNode(getLocator(), host, referenceYear);
  addChild(localGrid_.get());

  // will be parented to the beamLocatorNode_ when shown
  antenna_ = new simVis::AntennaNode(osg::Quat(M_PI_2, osg::Vec3d(0., 0., 1.)));

  label_ = new EntityLabelNode();
  beamLocatorNode_->addChild(label_.get());

  // horizon culling: entity culling based on bounding sphere
  addCullCallback( new osgEarth::HorizonCullCallback() );
  // labels are culled based on entity center point
  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // Add a tag for picking
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(this, this);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
  // SIM-10724: Labels need to not be flattened to be displayed in overhead mode
  simVis::OverheadMode::enableGeometryFlattening(false, label_.get());
}

BeamNode::~BeamNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

void BeamNode::updateLabel_(const simData::BeamPrefs& prefs)
{
  if (hasLastUpdate_)
  {
    std::string label = getEntityName_(prefs.commonprefs(), EntityNode::DISPLAY_NAME, false);
    if (prefs.commonprefs().labelprefs().namelength() > 0)
      label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

    std::string text;
    if (prefs.commonprefs().labelprefs().draw())
      text = labelContentCallback().createString(prefs, lastUpdateFromDS_, prefs.commonprefs().labelprefs().displayfields());

    if (!text.empty())
    {
      label += "\n";
      label += text;
    }

    const float zOffset = 0.0f;
    label_->update(prefs.commonprefs(), label, zOffset);
  }
}

std::string BeamNode::popupText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
  {
    std::string prefix;
    // if alias is defined show both in the popup to match SIMDIS 9's behavior.  SIMDIS-2241
    if (!lastPrefsFromDS_.commonprefs().alias().empty())
    {
      if (lastPrefsFromDS_.commonprefs().usealias())
        prefix = getEntityName(EntityNode::REAL_NAME);
      else
        prefix = getEntityName(EntityNode::ALIAS_NAME);
      prefix += "\n";
    }
    return prefix + labelContentCallback().createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string BeamNode::hookText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return labelContentCallback().createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string BeamNode::legendText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return labelContentCallback().createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().legenddisplayfields());

  return "";
}

void BeamNode::setPrefs(const simData::BeamPrefs& prefs)
{
  // validate localgrid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());

  // if this is a target beam, and there is a change in target id, NULL our target reference (will be set on update)
  if (lastProps_.type() == simData::BeamProperties_BeamType_TARGET &&
    (!hasLastPrefs_ || PB_FIELD_CHANGED(&lastPrefsApplied_, &prefs, targetid)))
  {
    target_ = nullptr;
  }

  if (!hasLastPrefs_ || PB_FIELD_CHANGED((&lastPrefsFromDS_.commonprefs()), (&prefs.commonprefs()), acceptprojectorid))
    applyProjectorPrefs_(lastPrefsFromDS_.commonprefs(), prefs.commonprefs());

  applyPrefs_(prefs);
  updateLabel_(prefs);
  lastPrefsFromDS_ = prefs;
}

void BeamNode::applyPrefs_(const simData::BeamPrefs& prefs, bool force)
{
  if (prefsOverrides_.size() == 0)
  {
    apply_(nullptr, &prefs, force);
    lastPrefsApplied_ = prefs;
    hasLastPrefs_ = true;
  }
  else
  {
    // merge in the overrides.
    simData::BeamPrefs accumulated(prefs);
    for (std::map<std::string, simData::BeamPrefs>::iterator i = prefsOverrides_.begin(); i != prefsOverrides_.end(); ++i)
    {
      accumulated.MergeFrom(i->second);
    }
    apply_(nullptr, &accumulated, force);
    lastPrefsApplied_ = accumulated;
    hasLastPrefs_ = true;
  }

  // manage beam pulse animation, creating it when necessary
  if (prefs.animate())
  {
    if (beamPulse_ == nullptr)
      beamPulse_ = new simVis::BeamPulse(getOrCreateStateSet());

    beamPulse_->setEnabled(true);
    beamPulse_->setLength(static_cast<float>(prefs.pulselength()));
    beamPulse_->setRate(static_cast<float>(prefs.pulserate()));
    beamPulse_->setStipplePattern(prefs.pulsestipple());
  }
  else if (beamPulse_ != nullptr)
    beamPulse_->setEnabled(false);
}

void BeamNode::setHostMissileOffset(double hostMissileOffset)
{
  if (hostMissileOffset_ != hostMissileOffset)
  {
    hostMissileOffset_ = hostMissileOffset;
    // force a complete refresh
    apply_(nullptr, nullptr, true);
  }
}

bool BeamNode::isActive() const
{
  return hasLastUpdate_ &&
         hasLastPrefs_ &&
         lastPrefsApplied_.commonprefs().datadraw();
}

bool BeamNode::isVisible() const
{
  return getNodeMask() != DISPLAY_MASK_NONE;
}

simData::ObjectId BeamNode::getId() const
{
  return lastProps_.id();
}

bool BeamNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

const std::string BeamNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // if assert fails, check whether prefs are initialized correctly when entity is created
  assert(hasLastPrefs_);
  return getEntityName_(lastPrefsApplied_.commonprefs(), nameType, allowBlankAlias);
}

float BeamNode::gain(float az, float el) const
{
  if (antenna_ && antenna_->isValid())
    return antenna_->PatternGain(az, el, polarity());
  else if (hasLastPrefs_)
    return lastPrefsApplied_.gain();
  return simCore::DEFAULT_ANTENNA_GAIN;
}

simCore::PolarityType BeamNode::polarity() const
{
  return hasLastPrefs_ ? static_cast<simCore::PolarityType>(lastPrefsApplied_.polarity()) : simCore::POLARITY_UNKNOWN;
}

bool BeamNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  bool updateApplied = false;
  const simData::BeamUpdateSlice* updateSlice = static_cast<const simData::BeamUpdateSlice*>(updateSliceBase);
  assert(updateSlice);
  assert(host_.valid());

  const bool hostChangedToActive = host_->isActive() && !hasLastUpdate_;
  const bool hostChangedToInactive = !host_->isActive() && hasLastUpdate_;

  // is there an update to apply? if not hasChanged, not forcing, and not a host transition, there is no update to apply
  // Note: if entity is not interpolated, !updateSlice->hasChanged() happens a lot
  if (updateSlice->hasChanged() || force || hostChangedToActive || hostChangedToInactive)
  {
    const simData::BeamUpdate* current = updateSlice->current();
    const bool beamChangedToInactive = !current && hasLastUpdate_;

    // do not apply update if host platform is not active
    if (current && (force || host_->isActive()))
    {
      applyDataStoreUpdate_(*current, force);
      updateApplied = true;
    }
    else if (beamChangedToInactive || hostChangedToInactive)
    {
      // avoid applying a null update over and over - only apply the null update on the transition
      flush();
      updateApplied = true;
    }
  }

  // Whether updateSlice changed or not, label content may have changed, and for active beams we need to update
  if (isActive())
    updateLabel_(lastPrefsApplied_);

  return updateApplied;
}

void BeamNode::flush()
{
  hasLastUpdate_ = false;
  setNodeMask(DISPLAY_MASK_NONE);
  beamLocatorNode_->removeChild(antenna_);
  beamLocatorNode_->removeChild(beamVolume_);
  beamVolume_ = nullptr;
}

double BeamNode::range() const
{
  if (!hasLastUpdate_)
    return 0.0;

  if (!lastUpdateFromDS_.has_range())
    return 0.0;

  return lastUpdateFromDS_.range();
}

int BeamNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return beamLocatorNode_->getPosition(out_position, coordsys);
}

int BeamNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return beamLocatorNode_->getPositionOrientation(out_position, out_orientation, coordsys);
}

// This method applies the datastore update to the beam, and
// provides a wrapper around the calculation of the target beam update so that it is treated as if it were a datastore update
void BeamNode::applyDataStoreUpdate_(const simData::BeamUpdate& update, bool force)
{
  // if this is a target beam, we need to populate the update with calculated RAE
  const bool targetBeam = (lastProps_.type() == simData::BeamProperties_BeamType_TARGET);
  if (!targetBeam)
    lastUpdateFromDS_ = update;
  else
  {
    // treat this calculated RAE as if it came from DS - store its calculation in the cached ds update
    if (0 != calculateTargetBeam_(lastUpdateFromDS_))
    {
      // failed on target beam calculation
      hasLastUpdate_ = false;
      return;
    }
    lastUpdateFromDS_.set_time(update.time());
  }
  applyUpdateOverrides_(force);
}

// This method provides a wrapper around the override update capability,
// which can be used to dynamically modify the beam visualization without affecting the real beam update data (cached in the lastUpdateFromDS_)
void BeamNode::applyUpdateOverrides_(bool force)
{
  if (updateOverrides_.size() == 0)
  {
    apply_(&lastUpdateFromDS_, nullptr, force);
    lastUpdateApplied_ = lastUpdateFromDS_;
  }
  else
  {
    simData::BeamUpdate accumulated(lastUpdateFromDS_);
    for (std::map<std::string, simData::BeamUpdate>::const_iterator i = updateOverrides_.begin(); i != updateOverrides_.end(); ++i)
    {
      accumulated.MergeFrom(i->second);
    }
    apply_(&accumulated, nullptr, force);
    lastUpdateApplied_ = accumulated;
  }

  // we have applied a valid update, and both lastUpdateApplied_ and lastUpdateFromDS_ are valid
  hasLastUpdate_ = true;
  // ensure that the locator node is in sync with its locator; a no-op id they are already in sync.
  beamLocatorNode_->syncWithLocator();
}

int BeamNode::calculateTargetBeam_(simData::BeamUpdate& targetBeamUpdate)
{
  // this should only be called for target beams; if assert fails, check caller
  assert(lastProps_.type() == simData::BeamProperties_BeamType_TARGET);

  // we should only receive non-nullptr updates for target beams which have valid target ids; if assert fails check MemoryDataStore processing
  assert(lastPrefsApplied_.targetid() > 0);

  // update our target reference, for new target, or after a prefs change in target ids occur
  if (target_ == nullptr || !target_.valid())
  {
    target_ = nodeGetter_(lastPrefsApplied_.targetid());
    // we should only receive an non-nullptr update when target is valid; if assert fails check MemoryDataStore processing
    assert(target_.valid());

    if (!target_.valid())
      return 1;
  }

  // calculate target beam RAE

  // determine the beam origin position
  simCore::Vec3 sourceLla;
  if (0 != getPosition(&sourceLla, simCore::COORD_SYS_LLA))
  {
    // if target beam is just turning on (processing this update will turn beam on), then
    // the locatorNode is not activated and has not been synced, and cannot provide valid info.
    // in this case, access position via a locator
    getLocator()->getLocatorPosition(&sourceLla, simCore::COORD_SYS_LLA);
  }

  simCore::Vec3 targetLla;
  target_->getPosition(&targetLla, simCore::COORD_SYS_LLA);

  double azimuth;
  double elevation;
  // let the simCore::Calculations implementation do coordinate conversions; it guarantees that only one initialization occurs for both these calculations.
  simCore::calculateAbsAzEl(sourceLla, targetLla, &azimuth, &elevation, nullptr, simCore::TANGENT_PLANE_WGS_84, nullptr);
  const double range = simCore::calculateSlant(sourceLla, targetLla, simCore::TANGENT_PLANE_WGS_84, nullptr);
  targetBeamUpdate.set_azimuth(azimuth);
  targetBeamUpdate.set_elevation(elevation);
  targetBeamUpdate.set_range(range);
  return 0;
}

void BeamNode::applyPlatformIconOffset_(simCore::Vec3& pos) const
{
  if (host_.valid())
    pos.set(pos.x(), pos.y() + hostMissileOffset_, pos.z());
}

void BeamNode::apply_(const simData::BeamUpdate* newUpdate, const simData::BeamPrefs* newPrefs, bool force)
{
  // beam can't do anything until it has both prefs and an update
  if ((!newUpdate && !hasLastUpdate_) || (!newPrefs && !hasLastPrefs_))
  {
    setNodeMask(DISPLAY_MASK_NONE);
    return;
  }

  // if we don't have new prefs, we will use the previous prefs
  const simData::BeamPrefs* activePrefs = newPrefs ? newPrefs : &lastPrefsApplied_;
  // if we don't have new update, we will use the previous update
  const simData::BeamUpdate* activeUpdate = newUpdate ? newUpdate : &lastUpdateApplied_;

  // if datadraw is off, we do not need to do any processing
  if (activePrefs->commonprefs().datadraw() == false)
  {
    flush();
    return;
  }

  // force indicates that activePrefs and activeUpdate must be applied, the visual must be redrawn, and the locator updated
  force = force || !hasLastUpdate_ || !hasLastPrefs_ ||
    (newPrefs && PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, commonprefs, datadraw));

  if (activePrefs->drawtype() == simData::BeamPrefs_DrawType_ANTENNA_PATTERN)
  {
    force = force || (newPrefs && PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, drawtype));

    // beam visual is drawn by Antenna
    // redraw if necessary, then update range and other prefs as necessary

    // setPrefs will perform the antenna redraw as required, and its return indicates whether a redraw occurred
    const bool refreshRequiresNewNode = (force || newPrefs) && antenna_->setPrefs(*activePrefs);

    if (force || (newUpdate && PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, range)))
      antenna_->setRange(static_cast<float>(simCore::sdkMax(1.0, activeUpdate->range())));

    // force && refreshRequiresNewNode - antenna was just redrawn and needs to be added as child
    // !force && refreshRequiresNewNode - antenna was just redrawn and needs to be added as child (prefs change)
    // force && !refreshRequiresNewNode - antenna was not redrawn, but needs to be added as child (just became active)
    if (force || refreshRequiresNewNode)
    {
      // remove any old (non-antenna) beam volume
      if (beamVolume_)
      {
        beamLocatorNode_->removeChild(beamVolume_);
        beamVolume_ = nullptr;
      }
      beamLocatorNode_->addChild(antenna_);
      dirtyBound();
    }
  }
  else
  {
    // beam visual is drawn by SphericalVolume

    // gain calcs can be affected by prefs changes, even if not displaying antpattern
    if (force || newPrefs)
      antenna_->setPrefs(*activePrefs);

    const bool refreshRequiresNewNode = force ||
      changeRequiresRebuild(&lastPrefsApplied_, newPrefs) ||
      changeRequiresRebuild(&lastUpdateApplied_, newUpdate);

    // if new geometry is required, build it:
    if (!beamVolume_ || refreshRequiresNewNode)
    {
      // do not nullptr antenna, it needs to persist to provide gain calcs
      beamLocatorNode_->removeChild(antenna_);

      if (beamVolume_)
      {
        beamLocatorNode_->removeChild(beamVolume_);
        beamVolume_ = nullptr;
      }

      beamVolume_ = new BeamVolume(*activePrefs, *activeUpdate);
      beamLocatorNode_->addChild(beamVolume_);
      dirtyBound();
    }
    else
    {
      if (newPrefs)
      {
        // !hasLastPrefs_ should force execution of refreshRequiresNewNode branch; if assert fails examine refreshRequiresNewNode assignment logic
        assert(hasLastPrefs_);
        beamVolume_->performInPlacePrefChanges(&lastPrefsApplied_, newPrefs);
      }
      if (newUpdate)
      {
        // !hasLastUpdate should force execution of refreshRequiresNewNode branch; if assert fails examine refreshRequiresNewNode assignment logic
        assert(hasLastUpdate_);
        beamVolume_->performInPlaceUpdates(&lastUpdateApplied_, newUpdate);
      }
    }
  }

  // BeamOnOffCmd turns active pref on and off
  // we exit early at top if datadraw is off; if assert fails, check for changes to the early exit
  assert(activePrefs->commonprefs().datadraw());
  const bool visible = activePrefs->commonprefs().draw();
  setNodeMask(visible ? DISPLAY_MASK_BEAM : DISPLAY_MASK_NONE);

  // update locator if required (even if draw off, since gates that are drawn may depend on the locator)
  updateLocator_(newUpdate, newPrefs, force);

  // update the local grid prefs, if beam is being drawn
  if (visible && (force || newPrefs))
  {
    // localgrid created in constructor. if assert fails, check for changes.
    assert(localGrid_ != nullptr);
    localGrid_->setPrefs(activePrefs->commonprefs().localgrid(), force);
  }
}

void BeamNode::updateLocator_(const simData::BeamUpdate* newUpdate, const simData::BeamPrefs* newPrefs, bool force)
{
  const bool oriOffsetsChanged = force || (newPrefs && (
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, useoffsetbeam) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, azimuthoffset) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, elevationoffset) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, rolloffset)));

  const bool posOffsetsChanged = force || (newPrefs && (
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, useoffsetbeam) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, useoffseticon) ||
    PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, beampositionoffset, x) ||
    PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, beampositionoffset, y) ||
    PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, beampositionoffset, z)));

  const bool beamDataOriChanged = force || (newUpdate && (
    PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, azimuth) ||
    PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, elevation)));

  if (!oriOffsetsChanged && !posOffsetsChanged && !beamDataOriChanged)
    return;

  // if we don't have new prefs, we will use the previous prefs
  const simData::BeamPrefs& activePrefs = newPrefs ? *newPrefs : lastPrefsApplied_;
  // if we don't have new update, we will use the previous update
  const simData::BeamUpdate& activeUpdate = newUpdate ? *newUpdate : lastUpdateApplied_;

  // process explicit beam orientation offsets
  if (posOffsetsChanged)
  {
    // beampositionoffset and useoffseticon are additive.
    // (Platform position offsets are applied only to model, they do not affect beam position.)
    simCore::Vec3 posOffset;
    if (activePrefs.useoffsetbeam())
    {
      const simData::Position& pos = activePrefs.beampositionoffset();
      // x/y order change and minus sign are needed to match the behavior of SIMDIS 9
      posOffset.set(-pos.y(), pos.x(), pos.z());
    }
    // automatic positional offset (placed at the front of the host platform).
    if (activePrefs.useoffseticon())
      applyPlatformIconOffset_(posOffset);

    // defer locator callback/syncing; a locator update will be forced below
    beamOriginLocator_->setLocalOffsets(posOffset, simCore::Vec3(), activeUpdate.time(), false);
  }

  // process explicit beam orientation offsets or beam data updates
  if (oriOffsetsChanged || beamDataOriChanged)
  {
    // ori offset should only be applied if useoffsetbeam is set
    // beam orientation offsets are simply added to beam az/el data; they are not processed as a separate modeling transformation
    const simCore::Vec3 beamOrientation = (activePrefs.useoffsetbeam()) ?
      simCore::Vec3(activeUpdate.azimuth() + activePrefs.azimuthoffset(), activeUpdate.elevation() + activePrefs.elevationoffset(), activePrefs.rolloffset())
      : simCore::Vec3(activeUpdate.azimuth(), activeUpdate.elevation(), 0.0);

    // defer locator callback/syncing; a locator update will be forced below
    beamOrientationLocator_->setLocalOffsets(simCore::Vec3(), beamOrientation, activeUpdate.time(), false);
  }

  // something changed, and locators must be sync'd - since beamOriginLocator_ is parent, its notification will update all children
  beamOriginLocator_->endUpdate();
  dirtyBound();
}

const simData::BeamUpdate* BeamNode::getLastUpdateFromDS() const
{
  return hasLastUpdate_ ? &lastUpdateFromDS_ : nullptr;
}

void BeamNode::setPrefsOverride(const std::string& id, const simData::BeamPrefs& prefs)
{
  prefsOverrides_[id] = prefs;
  applyPrefs_(lastPrefsFromDS_);
}

void BeamNode::removePrefsOverride(const std::string& id)
{
  std::map<std::string, simData::BeamPrefs>::iterator i = prefsOverrides_.find(id);
  if (i != prefsOverrides_.end())
  {
    prefsOverrides_.erase(i);
    applyPrefs_(lastPrefsFromDS_, true);
  }
}

void BeamNode::setUpdateOverride(const std::string& id, const simData::BeamUpdate& update)
{
  updateOverrides_[id] = update;
  // only apply override when we have a valid update from datastore
  if (hasLastUpdate_)
  {
    // force = false ->allow beam logic to determine whether an in-place update can be used,
    // instead of forcing a complete rebuild of the beam.
    applyUpdateOverrides_(false);
  }
}

void BeamNode::removeUpdateOverride(const std::string& id)
{
  std::map<std::string, simData::BeamUpdate>::iterator i = updateOverrides_.find(id);
  if (i != updateOverrides_.end())
  {
    updateOverrides_.erase(i);
    if (hasLastUpdate_)
      applyUpdateOverrides_(true);
  }
}

double BeamNode::getClosestPoint(const simCore::Vec3& toLla, simCore::Vec3& closestLla) const
{
  // Get start position
  simCore::Vec3 startPosition;
  simCore::Vec3 ori;
  if (0 != getPositionOrientation(&startPosition, &ori, simCore::COORD_SYS_LLA))
  {
    closestLla = simCore::Vec3();
    return 0.;
  }

  simCore::Vec3 endPosition;
  simCore::calculateGeodeticEndPoint(startPosition, ori.yaw(), ori.pitch(), lastUpdateFromDS_.range(), endPosition);

  double distanceToBeam = simCore::getClosestPoint(startPosition, endPosition, toLla, closestLla);
  const double distanceAlongBeam = simCore::sodanoInverse(startPosition.lat(), startPosition.lon(), startPosition.alt(), closestLla.lat(), closestLla.lon(), nullptr, nullptr);

  // Subtract the beam width from the distanceToBeam
  const double x = distanceAlongBeam * sin(0.5 * lastPrefsFromDS_.horizontalwidth()) * cos(ori.pitch());
  const double y = distanceAlongBeam * sin(0.5 * lastPrefsFromDS_.verticalwidth()) * sin(ori.pitch());
  const double offset = sqrt(simCore::square(x) + simCore::square(y));

  if (offset > distanceToBeam)
    distanceToBeam = 0.;
  else
    distanceToBeam -= offset;

  return distanceToBeam;
}

unsigned int BeamNode::objectIndexTag() const
{
  return objectIndexTag_;
}

}
