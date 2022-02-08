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
#include "osg/Geode"
#include "osgEarth/Horizon"
#include "osgEarth/LineDrawable"
#include "osgEarth/ObjectIndex"
#include "osgEarth/Registry"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simData/DataTypes.h"
#include "simNotify/Notify.h"
#include "simVis/Beam.h"
#include "simVis/Constants.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/SphericalVolume.h"
#include "simVis/OverheadMode.h"
#include "simVis/Gate.h"

/**
 * Gate updates are currently disabled as SphericalVolume needs updates in order
 * to correctly reposition outline geometry with new osgEarth::LineDrawable.
 */
#define GATE_IN_PLACE_UPDATES

namespace simVis
{

GateVolume::GateVolume(simVis::Locator* locator, const simData::GatePrefs* prefs, const simData::GateUpdate* update)
  : LocatorNode(locator)
{
  setName("Gate Volume Locator");
  gateSV_ = createNode_(prefs, update);
  setNodeMask(DISPLAY_MASK_GATE);
  addChild(gateSV_);

  // alpha fill pattern should use BIN_GATE & TPA, all other patterns are opaque patterns and use BIN_OPAQUE_GATE
  // if outline is on, it should be written (separately) to BIN_OPAQUE_GATE

  osg::Geometry* solidGeometry = simVis::SVFactory::solidGeometry(gateSV_.get());
  if (solidGeometry != nullptr)
  {
    const bool isOpaque = prefs->fillpattern() == simData::GatePrefs_FillPattern_WIRE ||
                          prefs->fillpattern() == simData::GatePrefs_FillPattern_SOLID ||
                          prefs->fillpattern() == simData::GatePrefs_FillPattern_STIPPLE;

    solidGeometry->getOrCreateStateSet()->setRenderBinDetails(
      (isOpaque ? BIN_OPAQUE_GATE   : BIN_GATE),
      (isOpaque ? BIN_GLOBAL_SIMSDK : BIN_TWO_PASS_ALPHA));
  }

  osg::Group* outlineGroup = simVis::SVFactory::opaqueGroup(gateSV_.get());
  if (outlineGroup != nullptr)
  {
    // SphericalVolume code only adds the opaque group when it is adding a geometry or lineGroup
    assert(outlineGroup->getNumChildren() > 0);
    outlineGroup->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);
  }
}

GateVolume::~GateVolume()
{
}

/// prefs that can be applied without rebuilding the whole gate
void GateVolume::performInPlacePrefChanges(const simData::GatePrefs* a, const simData::GatePrefs* b)
{
  if (a == nullptr || b == nullptr)
    return;

  if (b->commonprefs().useoverridecolor())
  {
    // Check for transition between color and override color than check for color change
    if (PB_SUBFIELD_CHANGED(a, b, commonprefs, useoverridecolor) || PB_SUBFIELD_CHANGED(a, b, commonprefs, overridecolor))
    {
      SVFactory::updateColor(gateSV_.get(), simVis::Color(b->commonprefs().overridecolor(), simVis::Color::RGBA));
    }
  }
  else
  {
    // Check for transition between color and override color than check for color change
    if (a->commonprefs().useoverridecolor() || PB_SUBFIELD_CHANGED(a, b, commonprefs, color))
    {
      SVFactory::updateColor(gateSV_.get(), simVis::Color(b->commonprefs().color(), simVis::Color::RGBA));
    }
  }

  if (PB_FIELD_CHANGED(a, b, gatelighting))
  {
    SVFactory::updateLighting(gateSV_.get(), b->gatelighting());
  }
}

/// updates that can be updated without rebuilding the whole gate
void GateVolume::performInPlaceUpdates(const simData::GateUpdate* a, const simData::GateUpdate* b)
{
  if (a == nullptr || b == nullptr)
    return;

#ifdef GATE_IN_PLACE_UPDATES
  // each update method calls dirtyBound on all gate volume geometries, so no need for that here
  if (PB_FIELD_CHANGED(a, b, minrange))
  {
    SVFactory::updateNearRange(gateSV_.get(), b->minrange());
  }
  if (PB_FIELD_CHANGED(a, b, maxrange))
  {
    SVFactory::updateFarRange(gateSV_.get(), b->maxrange());
  }
  if (PB_FIELD_CHANGED(a, b, width) && PB_BOTH_HAVE_FIELD(a, b, width))
  {
    SVFactory::updateHorizAngle(gateSV_.get(), b->width());
  }
  if (PB_FIELD_CHANGED(a, b, height) && PB_BOTH_HAVE_FIELD(a, b, height))
  {
    SVFactory::updateVertAngle(gateSV_.get(), b->height());
  }
#endif
}

SphericalVolume* GateVolume::createNode_(const simData::GatePrefs* prefs, const simData::GateUpdate* update)
{
  simVis::SVData sv;

  // gate defaults first:
  sv.color_.set(1, 0, 0, 0.5);
  sv.shape_    = simVis::SVData::SHAPE_PYRAMID;

  // both update and prefs are required; if assert fails, check calling code
  assert(update);
  assert(prefs);

  sv.lightingEnabled_ = prefs->gatelighting();
  sv.blendingEnabled_ = true;

  switch (prefs->fillpattern())
  {
  case simData::GatePrefs_FillPattern_STIPPLE:
    sv.drawMode_ = (prefs->drawoutline()) ? simVis::SVData::DRAW_MODE_STIPPLE | simVis::SVData::DRAW_MODE_OUTLINE :  simVis::SVData::DRAW_MODE_STIPPLE;
    break;
  case simData::GatePrefs_FillPattern_SOLID:
    sv.drawMode_ = (prefs->drawoutline()) ? simVis::SVData::DRAW_MODE_SOLID | simVis::SVData::DRAW_MODE_OUTLINE : simVis::SVData::DRAW_MODE_SOLID;
    sv.blendingEnabled_ = false;
    break;
  case simData::GatePrefs_FillPattern_ALPHA:
    sv.drawMode_ = (prefs->drawoutline()) ? simVis::SVData::DRAW_MODE_SOLID | simVis::SVData::DRAW_MODE_OUTLINE : simVis::SVData::DRAW_MODE_SOLID;
    break;
  case simData::GatePrefs_FillPattern_WIRE:
    sv.drawMode_ = simVis::SVData::DRAW_MODE_OUTLINE;
    sv.blendingEnabled_ = false;
    break;
  case simData::GatePrefs_FillPattern_CENTROID:
    sv.drawMode_ = simVis::SVData::DRAW_MODE_NONE;
    break;
  }

  if (prefs->commonprefs().useoverridecolor())
    sv.color_ = simVis::Color(prefs->commonprefs().overridecolor(), simVis::Color::RGBA);
  else
    sv.color_ = simVis::Color(prefs->commonprefs().color(), simVis::Color::RGBA);

  assert(update->has_azimuth() &&
         update->has_elevation() &&
         update->has_width() &&
         update->has_height() &&
         update->has_minrange() &&
         update->has_maxrange());
  sv.azimOffset_deg_ = simCore::RAD2DEG * update->azimuth();
  sv.elevOffset_deg_ = simCore::RAD2DEG * update->elevation();
  sv.hfov_deg_ = simCore::RAD2DEG * update->width();
  sv.vfov_deg_ = simCore::RAD2DEG * update->height();

  // scale capRes based on fov
  const float maxFov = simCore::sdkMax(sv.hfov_deg_, sv.vfov_deg_);
  // 1 tesselation per 5 degrees of gate fov
  // clamped at the bottom to ensure good visuals for common smaller gate sizes
  // clamped at the top to prevent perf hit for large gates
  const float capRes = osg::clampBetween((maxFov / 5.f), 5.f, 24.f);
  sv.capRes_ = static_cast<unsigned int>(0.5f + capRes);

  // gate walls don't need much tesselation, so reduce processing/memory load
  sv.wallRes_ = 3;

  sv.nearRange_ = update->minrange();
  sv.farRange_ = update->maxrange();

  // do not draw near-face and sides/walls of gate when:
  //   the gate has no thickness, or is in FOOTPRINT drawmode
  sv.drawCone_ = (update->minrange() < update->maxrange()) && (prefs->gatedrawmode() != simData::GatePrefs_DrawMode_FOOTPRINT);

  // coverage gates are sphere segments (absolute start/end degrees instead of
  // elevation and span)
  sv.drawAsSphereSegment_ = prefs->gatedrawmode() == simData::GatePrefs_DrawMode_COVERAGE;

  // use a Y-forward directional vector to correspond with the gate's locator.
  SphericalVolume* node = simVis::SVFactory::createNode(sv, osg::Y_AXIS);
  return node;
}

// --------------------------------------------------------------------------

GateCentroid::GateCentroid(simVis::Locator* locator)
  : LocatorNode(locator)
{
  setName("Centroid Locator");
  geom_ = new osgEarth::LineDrawable(GL_LINES);
  geom_->setName("simVis::GateCentroid Geometry");
  geom_->setColor(simVis::Color::White);
  geom_->setDataVariance(osg::Object::DYNAMIC);
  geom_->allocate(6);
  geom_->dirty();

  geom_->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);

  osg::Group* solidLines = new osgEarth::LineGroup();
  solidLines->setName("Solid LineGroup");
  solidLines->addChild(geom_);
  addChild(solidLines);
}

GateCentroid::~GateCentroid()
{
}

void GateCentroid::setVisible(bool visible)
{
  // setting the geometry node mask can turn the draw off without turning off the centroid/locator node
  geom_->setNodeMask(visible ? DISPLAY_MASK_GATE : DISPLAY_MASK_NONE);
}

// perform an in-place update to an existing centroid
void GateCentroid::update(const simData::GateUpdate& update)
{
  updateCentroid_(update);
}

// calculate centroid verts from update
void GateCentroid::updateCentroid_(const simData::GateUpdate& update)
{
  // scale centroid relative to gate width & height
  const double sinWidth  = (update.width() >= M_PI_2) ? 1.0 : sin(update.width());
  const double sinHeight = (update.height() >= M_PI_2) ? 1.0 : sin(update.height());
  const double xSize = sinWidth * update.maxrange() / 8.0;
  const double ySize = (update.maxrange() - update.minrange()) / 8.0;
  const double zSize = sinHeight * update.maxrange() / 8.0;
  geom_->setVertex(0, osg::Vec3(-xSize, 0.0f, 0.0f));
  geom_->setVertex(1, osg::Vec3(xSize, 0.0f, 0.0f));
  geom_->setVertex(2, osg::Vec3(0.0f, -ySize, 0.0f));
  geom_->setVertex(3, osg::Vec3(0.0f, ySize, 0.0f));
  geom_->setVertex(4, osg::Vec3(0.0f, 0.0f, -zSize));
  geom_->setVertex(5, osg::Vec3(0.0f, 0.0f, zSize));
}

// --------------------------------------------------------------------------

GateNode::GateNode(const simData::GateProperties& props, Locator* hostLocator, const EntityNode* host, int referenceYear)
  : EntityNode(simData::GATE),
    hasLastUpdate_(false),
    hasLastPrefs_(false),
    host_(host),
    objectIndexTag_(0)
{
  setNodeMask(DISPLAY_MASK_NONE);
  lastProps_ = props;
  setName("GateNode");

  // 1) the gate's locator: centroidLocator_ (returned by getLocator()), represents the position and orientation of the gate centroid

  // 2) gate volume is drawn relative to platform/beam position, not centroid position
  //  so, the gate volume needs a locator without the position offset to the centroid
  //  this is what gateVolumeLocator and baseLocator_ do

  // 3) gate includes its own orientation, so beam orientation must be stripped out
  //  this is what COMP_RESOLVED_POSITION does for gateVolumeLocator and baseLocator_

  // 4) due to special handling of coverage gates, the gate volume
  //  needs the gateVolumeLocator, which is not inherited by the other locators
  //  gateVolumeLocator and baseLocator_ are duplicative except in the one special case

  // 5) the centroid visual must be parented by a different locatorNode than the gate visual
  //  so that centroid can be correctly drawn when the gate visual is coverage gate

  // gates can be hosted by platforms or beams
  const BeamNode* beam = dynamic_cast<const BeamNode*>(host_.get());
  if (beam != nullptr && hostLocator)
  {
    // body and range gates are positioned relative to beam origin, but never relative to beam orientation.
    // in some cases, the locator that beam provides via getLocator() (beamOrientationLocator_) strips out platform orientation;
    // in those cases, it can't be used for BODY gates, which are relative to platform orientation.
    // beam->getLocator()'s parent (beamOriginLocator_) always provides the the beam origin with platform orientation.
    // gate locators here maintain or strip out platform orientation as required.
    hostLocator = hostLocator->getParentLocator();
  }

  // if the properties call for a body-relative beam, reconfigure locators to include platform orientation
  if (props.has_type() && props.type() == simData::GateProperties_GateType_BODY_RELATIVE)
  {
    // for body beam, inherit from beam's position offset, and keep platform orientation.
    gateVolumeLocator_ = new ResolvedPositionOrientationLocator(hostLocator, Locator::COMP_ALL);

    // for body beam, inherit from beam's position offset, and keep platform orientation.
    baseLocator_ = new ResolvedPositionOrientationLocator(hostLocator, Locator::COMP_ALL);

    // this locator sets the centroid position offset from the platform, using the gate orientation offsets
    centroidPositionOffsetLocator_ = new Locator(baseLocator_.get(), Locator::COMP_ALL);

    // inherit the gate centroid and the platform orientation, without gate orientation offsets, then add back (as local offsets) the gate orientation
    centroidLocator_ = new ResolvedPositionOrientationLocator(centroidPositionOffsetLocator_.get(), Locator::COMP_ALL);
  }
  else
  {
    // inherit from beam's locator, including its position offset, but stripping out all orientation
    gateVolumeLocator_ = new ResolvedPositionLocator(hostLocator, Locator::COMP_ALL);

    // inherit from beam's locator, including its position offset, but stripping out all orientation
    baseLocator_ = new ResolvedPositionLocator(hostLocator, Locator::COMP_ALL);

    // this locator sets the centroid position offset from the platform, using the gate orientation offsets
    centroidPositionOffsetLocator_ = new Locator(baseLocator_.get(), Locator::COMP_ALL);

    // this locator starts with the resolved centroid position, with identity orientation, then adds back (as local offsets) the gate orientation
    centroidLocator_ = new ResolvedPositionLocator(centroidPositionOffsetLocator_.get(), Locator::COMP_ALL);
  }

  // the gate's locator represents the position and orientation of the gate centroid
  setLocator(centroidLocator_.get());

  localGrid_ = new LocalGridNode(centroidLocator_.get(), host, referenceYear);
  addChild(localGrid_);

  // horizon culling: entity culling based on bounding sphere
  addCullCallback( new osgEarth::HorizonCullCallback() );

  // Create the centroid - gate tethering depends on the centroid, so it must always exist (when gate exists) even if centroid is not drawn
  centroid_ = new GateCentroid(centroidLocator_.get());
  centroid_->setEntityToMonitor(this);
  addChild(centroid_);

  // centroid provides a persistent locatornode to parent our label node
  label_ = new EntityLabelNode();
  centroid_->addChild(label_);

  // labels are culled based on centroid center point
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

GateNode::~GateNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

void GateNode::updateLabel_(const simData::GatePrefs& prefs)
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

std::string GateNode::popupText() const
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

std::string GateNode::hookText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return labelContentCallback().createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string GateNode::legendText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return labelContentCallback().createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().legenddisplayfields());
  return "";
}

void GateNode::setPrefs(const simData::GatePrefs& prefs)
{
  // validate localgrid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());

  applyProjectorPrefs_(lastPrefsFromDS_.commonprefs(), prefs.commonprefs());

  applyPrefs_(prefs);
  updateLabel_(prefs);
  lastPrefsFromDS_ = prefs;
}

void GateNode::applyPrefs_(const simData::GatePrefs& prefs, bool force)
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
    simData::GatePrefs accumulated(prefs);
    for (std::map<std::string, simData::GatePrefs>::iterator i = prefsOverrides_.begin(); i != prefsOverrides_.end(); ++i)
    {
      accumulated.MergeFrom(i->second);
    }
    apply_(nullptr, &accumulated, force);
    lastPrefsApplied_ = accumulated;
    hasLastPrefs_ = true;
  }
}

bool GateNode::isActive() const
{
  return hasLastUpdate_ && hasLastPrefs_ && lastPrefsApplied_.commonprefs().datadraw();
}

bool GateNode::isVisible() const
{
  return (getNodeMask() != DISPLAY_MASK_NONE);
}

simData::ObjectId GateNode::getId() const
{
  return lastProps_.id();
}

bool GateNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

const std::string GateNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // if assert fails, check whether prefs are initialized correctly when entity is created
  assert(hasLastPrefs_);
  return getEntityName_(lastPrefsApplied_.commonprefs(), nameType, allowBlankAlias);

}

bool GateNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  bool updateApplied = false;
  const simData::GateUpdateSlice* updateSlice = static_cast<const simData::GateUpdateSlice*>(updateSliceBase);
  assert(updateSlice);
  assert(host_.valid());

  const bool hostChangedToActive = host_->isActive() && !hasLastUpdate_;
  const bool hostChangedToInactive = !host_->isActive() && hasLastUpdate_;

  // if not hasChanged, not forcing, and not a host transition, there is no update to apply
  // Note: if entity is not interpolated, !updateSlice->hasChanged() happens a lot
  if (updateSlice->hasChanged() || force || hostChangedToActive || hostChangedToInactive)
  {
    const simData::GateUpdate* current = updateSlice->current();
    const bool gateChangedToInactive = (current == nullptr && hasLastUpdate_);

    // do not apply update if host is not active
    if (current && (force || host_->isActive()))
    {
      // apply the new update
      applyDataStoreUpdate_(*current, force);
      updateApplied = true;
    }
    else if (gateChangedToInactive || hostChangedToInactive)
    {
      // avoid applying a null update over and over - only apply the null update on the transition
      flush();
      updateApplied = true;
    }
  }

  // Whether updateSlice changed or not, label content may have changed, and for active gates we need to update
  if (isActive())
    updateLabel_(lastPrefsApplied_);

  return updateApplied;
}

void GateNode::flush()
{
  hasLastUpdate_ = false;
  setNodeMask(DISPLAY_MASK_NONE);
  removeChild(gateVolume_);
  gateVolume_ = nullptr;
}

double GateNode::range() const
{
  return (hasLastUpdate_ ? lastUpdateFromDS_.centroid() : 0.0);
}

int GateNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return centroid_->getPosition(out_position, coordsys);
}

int GateNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return centroid_->getPositionOrientation(out_position, out_orientation, coordsys);
}

const simData::GateUpdate* GateNode::getLastUpdateFromDS() const
{
  return hasLastUpdate_ ? &lastUpdateFromDS_ : nullptr;
}

// This method applies the datastore update to the gate, and
// provides a wrapper around the calculation of the target gate update so that it is treated as if it were a datastore update,
// provides a wrapper around a gate that uses beam's beamwidth, so that it is treated as if it were a datastore update
void GateNode::applyDataStoreUpdate_(const simData::GateUpdate& update, bool force)
{
  // if this is a target gate, we need to populate the update with calculated RAE
  const bool targetGate = (lastProps_.type() == simData::GateProperties_GateType_TARGET);
  if (!targetGate)
    lastUpdateFromDS_ = update;
  else
  {
    // treat this calculated RAE as if it came from DS - store its calculation in the cached ds update
    if (0 != calculateTargetGate_(update, lastUpdateFromDS_))
    {
      hasLastUpdate_ = false;
      return;
    }
  }
  // If width angles are zero or less, use the host beamwidth angles as per Appendix A of the SIMDIS User Manual
  if (lastUpdateFromDS_.height() <= 0.0 || lastUpdateFromDS_.width() <= 0.0)
  {
    const BeamNode* beam = dynamic_cast<const BeamNode*>(host_.get());
    if (beam != nullptr)
    {
      if (lastUpdateFromDS_.height() <= 0.0)
        lastUpdateFromDS_.set_height(beam->getPrefs().verticalwidth());
      if (lastUpdateFromDS_.width() <= 0.0)
        lastUpdateFromDS_.set_width(beam->getPrefs().horizontalwidth());
    }
  }

  applyUpdateOverrides_(force);
}

// This method provides a wrapper around the override update capability,
// which can be used to dynamically modify the gate visualization without affecting the real gate update data (cached in the lastUpdateFromDS_)
void GateNode::applyUpdateOverrides_(bool force)
{
  if (updateOverrides_.size() == 0)
  {
    // apply the new update with no overrides.
    apply_(&lastUpdateFromDS_, nullptr, force);
    lastUpdateApplied_ = lastUpdateFromDS_;
  }
  else
  {
    // add any overrides to the new update and apply the accumulated result.
    simData::GateUpdate accumulated(lastUpdateFromDS_);
    for (std::map<std::string, simData::GateUpdate>::iterator i = updateOverrides_.begin(); i != updateOverrides_.end(); ++i)
    {
      accumulated.MergeFrom(i->second);
    }
    apply_(&accumulated, nullptr, force);
    lastUpdateApplied_ = accumulated;
  }
  // we have applied a valid update, and both lastUpdateApplied_ and lastUpdateFromDS_ are valid
  hasLastUpdate_ = true;
  // ensure that the centroid is in sync with its locator; this will be a no-op if they are already in sync.
  centroid_->syncWithLocator();
}

int GateNode::calculateTargetGate_(const simData::GateUpdate& update, simData::GateUpdate& targetGateUpdate)
{
  // this should only be called for target beams; if assert fails, check caller
  assert(lastProps_.type() == simData::GateProperties_GateType_TARGET);

  if (!host_.valid())
  {
    // we should not receive updates for a target gate when host is not valid; if assert fails check MemoryDataStore processing
    assert(0);
    return 1;
  }
  const BeamNode* beam = dynamic_cast<const BeamNode*>(host_.get());
  if (beam == nullptr)
  {
    // target gate require a host beam; host is not a beam, so exit.
    return 1;
  }

  assert(beam->getProperties().type() == simData::BeamProperties_BeamType_TARGET);
  // the target beam should have the correct RAE; will be nullptr if target beam could not calculate
  const simData::BeamUpdate* beamUpdate = beam->getLastUpdateFromDS();
  if (beamUpdate == nullptr)
    return 1;

  targetGateUpdate.set_time(update.time());
  targetGateUpdate.set_width(update.width());
  targetGateUpdate.set_height(update.height());

  targetGateUpdate.set_azimuth(beamUpdate->azimuth());
  targetGateUpdate.set_elevation(beamUpdate->elevation());
  const double range = beamUpdate->range();
  targetGateUpdate.set_minrange(range + update.minrange());
  targetGateUpdate.set_maxrange(range + update.maxrange());
  targetGateUpdate.set_centroid(range + update.centroid());
  return 0;
}

void GateNode::apply_(const simData::GateUpdate* newUpdate, const simData::GatePrefs* newPrefs, bool force)
{
  // gate can't do anything until it has both prefs and an update
  if ((!newUpdate && !hasLastUpdate_) || (!newPrefs && !hasLastPrefs_))
  {
    setNodeMask(DISPLAY_MASK_NONE);
    return;
  }

  // if we don't have new prefs, we will use the previous prefs
  const simData::GatePrefs* activePrefs = newPrefs ? newPrefs : &lastPrefsApplied_;
  // if we don't have new update, we will use the previous update
  const simData::GateUpdate* activeUpdate = newUpdate ? newUpdate : &lastUpdateApplied_;

  // if assert fails, check that   if ((!newUpdate && !hasLastUpdate_) || (!newPrefs && !hasLastPrefs_))  test at top of this routine has not been changed
  assert(activePrefs != nullptr);
  assert(activeUpdate != nullptr);

  // if datadraw is off, we do not need to do any processing
  if (activePrefs->commonprefs().datadraw() == false)
  {
    flush();
    return;
  }

  // force indicates that activePrefs and activeUpdate must be applied, the visual must be redrawn, and the locator updated
  force = force || !hasLastUpdate_ || !hasLastPrefs_ ||
    (newPrefs && PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, commonprefs, datadraw));

  // do we need to redraw gate volume visual?
  const bool refreshRequiresNewNode = force || changeRequiresRebuild_(newUpdate, newPrefs);
  if (refreshRequiresNewNode)
  {
    if (gateVolume_)
    {
      removeChild(gateVolume_);
      gateVolume_ = nullptr;
    }

    if (activePrefs->fillpattern() != simData::GatePrefs_FillPattern_CENTROID)
    {
      gateVolume_ = new GateVolume(gateVolumeLocator_.get(), activePrefs, activeUpdate);
      addChild(gateVolume_);
    }
    // explicit dirtyBound call probably only necessary in the case that the volume is removed and only the centroid is left
    dirtyBound();
  }
  else if (gateVolume_)
  {
    if (newPrefs)
      gateVolume_->performInPlacePrefChanges(&lastPrefsApplied_, newPrefs);

    if (newUpdate)
    {
      // dirtyBound calls are handled in the performInPlaceUpdates call stream, no need to do it here
      gateVolume_->performInPlaceUpdates(&lastUpdateApplied_, newUpdate);
    }
  }

  // Fix the draw flag on the centroid - note that the logic here means that: if in fillpattern centroid, drawcentroid pref toggle does not hide it
  const bool drawCentroid = activePrefs->drawcentroid() || activePrefs->fillpattern() == simData::GatePrefs_FillPattern_CENTROID;
  centroid_->setVisible(drawCentroid);

  // centroid must be kept up-to-date, even if it is not shown, due to gate tethering/picking dependency on centroid
  // update the centroid for changes in size; locator takes care of centroid positioning
  if (force ||
    (newUpdate && (
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, minrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, maxrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, width) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, height))))
  {
    // activeUpdate is always valid, and points to the new update if there is a new update, or the previous update otherwise
    centroid_->update(*activeUpdate);
  }

  // GateOnOffCmd turns datadraw pref on and off
  // we exit early at top if datadraw is off; if assert fails, check for changes to the early exit
  assert(activePrefs->commonprefs().datadraw());
  const bool visible = activePrefs->commonprefs().draw();
  setNodeMask(visible ? DISPLAY_MASK_GATE : DISPLAY_MASK_NONE);

  // is a locator update required?
  updateLocator_(newUpdate, newPrefs, force);

  // update the local grid prefs, if gate is being drawn
  if (visible && (force || newPrefs))
  {
    // localgrid created in constructor. if assert fails, check for changes.
    assert(localGrid_ != nullptr);
    localGrid_->setPrefs(activePrefs->commonprefs().localgrid(), force);
  }
}

void GateNode::updateLocator_(const simData::GateUpdate* newUpdate, const simData::GatePrefs* newPrefs, bool force)
{
  // !hasLastUpdate_ requires force == true; if assert fails check for changes to apply_ above
  assert(force || hasLastUpdate_);
  // !hasLastPrefs_ requires force == true; if assert fails check for changes to apply_ above
  assert(force || hasLastPrefs_);

  const bool locatorUpdateRequired = force ||
    (newUpdate && (
    PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, centroid) ||
    PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, azimuth) ||
    PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, elevation))) ||
    (newPrefs && (
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gatedrawmode) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gateazimuthoffset) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gateelevationoffset) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gaterolloffset)));

  if (!locatorUpdateRequired)
    return;

  // if we don't have new prefs, we will use the previous prefs
  const simData::GatePrefs* activePrefs = newPrefs ? newPrefs : &lastPrefsApplied_;
  // if we don't have new update, we will use the previous update
  const simData::GateUpdate* activeUpdate = newUpdate ? newUpdate : &lastUpdateApplied_;

  const double azimuth = activeUpdate->azimuth() + activePrefs->gateazimuthoffset();
  const double elevation = activeUpdate->elevation() + activePrefs->gateelevationoffset();
  const double roll = activePrefs->gaterolloffset();

  // For a COVERAGE gate, the az/el is baked into the geometry,
  // so do not apply it to the locator.
  if (activePrefs->gatedrawmode() == simData::GatePrefs::COVERAGE)
  {
    // apply the local gate orientation
    gateVolumeLocator_->setLocalOffsets(
      simCore::Vec3(0, 0, 0),
      simCore::Vec3(activePrefs->gateazimuthoffset(), activePrefs->gateelevationoffset(), activePrefs->gaterolloffset()),
      activeUpdate->time());
  }
  else
  {
    // not a coverage gate, so apply the local orientation
    gateVolumeLocator_->setLocalOffsets(
      simCore::Vec3(0, 0, 0),
      simCore::Vec3(azimuth, elevation, roll),
      activeUpdate->time());
  }

  // apply the local gate orientation (in the Coverage draw type case, this diverges from the gateLocatorNode_
  baseLocator_->setLocalOffsets(
    simCore::Vec3(0, 0, 0),
    simCore::Vec3(azimuth, elevation, roll),
    activeUpdate->time(), false);

  // set grid locator offset to gate centroid position
  centroidPositionOffsetLocator_->setLocalOffsets(
    simCore::Vec3(0.0, activeUpdate->centroid(), 0.0),
    simCore::Vec3(),
    activeUpdate->time(), false);

  // apply the local orientation
  centroidLocator_->setLocalOffsets(
    simCore::Vec3(0, 0, 0),
    simCore::Vec3(azimuth, elevation, roll),
    activeUpdate->time(), false);

  // baseLocator_ is parent to centroidPositionOffsetLocator_ and centroidLocatorNode_, its notification will include them
  baseLocator_->endUpdate();

  dirtyBound();
}

/// determine if new update/new prefs can be handled with in-place-update (without complete rebuild)
bool GateNode::changeRequiresRebuild_(const simData::GateUpdate* newUpdate, const simData::GatePrefs* newPrefs) const
{
  // this can only be called when prefs and updates are already set; if assert fails, check callers
  assert(hasLastUpdate_ && hasLastPrefs_);

  if (newPrefs != nullptr &&
    (PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, fillpattern) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gatedrawmode) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, drawoutline)))
  {
    return true;
  }

  if (newUpdate != nullptr)
  {
#ifdef GATE_IN_PLACE_UPDATES
    // changing a gate minrange to/from 0.0 requires a rebuild due to simplified shape
    if (PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, minrange) &&
      (newUpdate->minrange() == 0.0 || lastUpdateApplied_.minrange() == 0.0))
      return true;
#else
    if (PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, minrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, maxrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, width) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, height))
      return true;
#endif

    // changes to coverage gates require rebuild (instead of in-place updates)
    const simData::GatePrefs* activePrefs = newPrefs ? newPrefs : &lastPrefsApplied_;
    if (activePrefs->gatedrawmode() == simData::GatePrefs::COVERAGE &&
      (PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, azimuth)  ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, elevation) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, width)     ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, height)))
    {
      return true;
    }
  }
  return false;
}

void GateNode::setPrefsOverride(const std::string& id, const simData::GatePrefs& prefs)
{
  prefsOverrides_[id] = prefs;

  // re-apply the prefs state with the new override
  if (hasLastPrefs_)
    applyPrefs_(lastPrefsFromDS_);
}

void GateNode::removePrefsOverride(const std::string& id)
{
  std::map<std::string, simData::GatePrefs>::iterator i = prefsOverrides_.find(id);
  if (i != prefsOverrides_.end())
  {
    prefsOverrides_.erase(i);

    // re-apply the prefs state without this override
    if (hasLastPrefs_)
      applyPrefs_(lastPrefsFromDS_, true);
  }
}

void GateNode::setUpdateOverride(const std::string& id, const simData::GateUpdate& update)
{
  updateOverrides_[id] = update;

  // re-apply the update state with the new override in place
  if (hasLastUpdate_)
    applyUpdateOverrides_();
}

void GateNode::removeUpdateOverride(const std::string& id)
{
  std::map<std::string, simData::GateUpdate>::iterator i = updateOverrides_.find(id);
  if (i != updateOverrides_.end())
  {
    updateOverrides_.erase(i);

    // re-apply the update state with the override removed
    if (hasLastUpdate_)
      applyUpdateOverrides_(true);
  }
}

Locator* GateNode::getVolumeLocator() const
{
  return gateVolumeLocator_.get();
}

unsigned int GateNode::objectIndexTag() const
{
  return objectIndexTag_;
}

}
