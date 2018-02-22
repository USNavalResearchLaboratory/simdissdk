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
#include "osg/Depth"
#include "osg/Geode"
#include "osgEarth/Horizon"
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

namespace simVis
{

GateVolume::GateVolume(simVis::Locator* locator, const simData::GatePrefs* prefs, const simData::GateUpdate* update)
  : LocatorNode(locator)
{
  gateSV_ = createNode_(prefs, update);
  setNodeMask(DISPLAY_MASK_GATE);
  addChild(gateSV_);

  const bool isOpaque = prefs->fillpattern() == simData::GatePrefs_FillPattern_WIRE ||
                        prefs->fillpattern() == simData::GatePrefs_FillPattern_SOLID;

  // alpha or stipple fill pattern should use BIN_GATE, but if outline is on, it should be written (separately) to BIN_OPAQUE_GATE
  //gateSV_->getOrCreateStateSet()->setRenderBinDetails((isOpaque ? BIN_OPAQUE_GATE : BIN_GATE), BIN_GLOBAL_SIMSDK);
  gateSV_->getOrCreateStateSet()->setRenderBinDetails(
      (isOpaque ? BIN_OPAQUE_GATE   : BIN_GATE),
      (isOpaque ? BIN_GLOBAL_SIMSDK : BIN_TWO_PASS_ALPHA));

  osg::Geometry* outlineGeometry = simVis::SVFactory::outlineGeometry(gateSV_.get());
  if (outlineGeometry != NULL)
    outlineGeometry->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);
}

GateVolume::~GateVolume()
{
}

/// prefs that can be applied without rebuilding the whole gate
void GateVolume::performInPlacePrefChanges(const simData::GatePrefs* a, const simData::GatePrefs* b)
{
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
    SVFactory::updateHorizAngle(gateSV_.get(), a->width(), b->width());
  }
  if (PB_FIELD_CHANGED(a, b, height) && PB_BOTH_HAVE_FIELD(a, b, height))
  {
    SVFactory::updateVertAngle(gateSV_.get(), a->height(), b->height());
  }
}

osg::MatrixTransform* GateVolume::createNode_(const simData::GatePrefs* prefs, const simData::GateUpdate* update)
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

  // draw near face and sides/walls of gate when the gate has thickness
  sv.drawCone_ = update->minrange() < update->maxrange();

  // coverage gates are sphere segments (absolute start/end degrees instead of
  // elevation and span)
  sv.drawAsSphereSegment_ = prefs->gatedrawmode() == simData::GatePrefs_DrawMode_COVERAGE;

  // use a Y-forward directional vector to correspond with the gate's locator.
  osg::MatrixTransform* node = simVis::SVFactory::createNode(sv, osg::Y_AXIS);
  return node;
}

// --------------------------------------------------------------------------

GateCentroid::GateCentroid(simVis::Locator* locator)
  : LocatorNode(locator)
{
  setActive(false);
  geom_ = new osg::Geometry();
  geom_->setUseVertexBufferObjects(true);

  osg::Vec4Array* c = new osg::Vec4Array(1);
  (*c)[0] = osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f); // always white
  geom_->setColorArray(c);
  geom_->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::Vec3Array* v = new osg::Vec3Array(6);
  geom_->setVertexArray(v);

  osg::DrawElementsUShort* centroid = new osg::DrawElementsUShort(GL_LINES, 6);
  for (unsigned int i = 0; i < 6; ++i)
    centroid->setElement(i, i);
  geom_->addPrimitiveSet(centroid);

  geom_->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);

  osg::Geode* geodeSolid = new osg::Geode();
  geodeSolid->addDrawable(geom_);
  addChild(geodeSolid);
}

GateCentroid::~GateCentroid()
{
}

void GateCentroid::setActive(bool active)
{
  // the centroid's nodemask controls locatorNode activation/deactivation
  setNodeMask(active ? DISPLAY_MASK_GATE : DISPLAY_MASK_NONE);
}

void GateCentroid::setVisible(bool visible)
{
  // setting the geometry node mask can turn the draw off without turning off the centroid/locator node
  geom_->setNodeMask(visible ? DISPLAY_MASK_GATE : DISPLAY_MASK_NONE);
}

// perform an in-place update to an existing centroid
void GateCentroid::update(const simData::GateUpdate& update)
{
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom_->getVertexArray());
  updateCentroid_(verts, update);
  geom_->dirtyBound();
}

// calculate centroid verts from update
void GateCentroid::updateCentroid_(osg::Vec3Array* verts, const simData::GateUpdate& update)
{
  // scale centroid relative to gate width & height
  const double sinWidth  = (update.width() >= M_PI_2) ? 1.0 : sin(update.width());
  const double sinHeight = (update.height() >= M_PI_2) ? 1.0 : sin(update.height());
  const double xSize = sinWidth * update.maxrange() / 8.0;
  const double ySize = (update.maxrange() - update.minrange()) / 8.0;
  const double zSize = sinHeight * update.maxrange() / 8.0;
  (*verts)[0] = (osg::Vec3(-xSize, 0.0f, 0.0f));
  (*verts)[1] = (osg::Vec3(xSize, 0.0f, 0.0f));
  (*verts)[2] = (osg::Vec3(0.0f, -ySize, 0.0f));
  (*verts)[3] = (osg::Vec3(0.0f, ySize, 0.0f));
  (*verts)[4] = (osg::Vec3(0.0f, 0.0f, -zSize));
  (*verts)[5] = (osg::Vec3(0.0f, 0.0f, zSize));
  verts->dirty();
}

// --------------------------------------------------------------------------

GateNode::GateNode(const simData::GateProperties& props, Locator* hostLocator, const EntityNode* host, int referenceYear)
  : EntityNode(simData::GATE),
    hasLastUpdate_(false),
    hasLastPrefs_(false),
    host_(host),
    contentCallback_(new NullEntityCallback()),
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

  // inherit from beam's locator, including its position offset, but stripping out all orientation

  // if the properties call for a body-relative beam, reconfigure locators to include platform orientation
  if (props.has_type() && props.type() == simData::GateProperties_GateType_BODY_RELATIVE)
  {
    // for body beam, inherit from beam's locator, including its position offset, but stripping out only beam orientation offset (keeping platform orientation).
    gateVolumeLocator_ = new ResolvedPositionOrientationLocator(hostLocator, Locator::COMP_ALL);

    // for body beam, inherit from beam's locator, including its position offset, but stripping out only beam orientation offset (keeping platform orientation).
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

  // set up a state set.
  // carefully set the rendering order for gates. We want to render them
  // before everything else (including the terrain) since they are
  // transparent and potentially self-blending
  osg::StateSet* stateSet = getOrCreateStateSet();
  //stateSet->setRenderBinDetails(BIN_GATE, BIN_GLOBAL_SIMSDK); //"RenderBin");
  stateSet->setRenderBinDetails(BIN_GATE, BIN_TWO_PASS_ALPHA); //"RenderBin");

  // depth-writing is disabled for the gates.
  // the gates draw before anything else (including the terrain) along with beams.
  //depthAttr_ = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
  //stateSet->setAttributeAndModes(depthAttr_, osg::StateAttribute::ON);

  // horizon culling:
  addCullCallback( new osgEarth::HorizonCullCallback() );

  // Create the centroid - gate tethering depends on the centroid, so it must always exist (when gate exists) even if centroid is not drawn
  centroid_ = new GateCentroid(centroidLocator_.get());
  addChild(centroid_);

  // centroid provides a persistent locatornode to parent our label node
  label_ = new EntityLabelNode();
  centroid_->addChild(label_);

  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setHorizon(new osgEarth::Horizon(*getLocator()->getSRS()->getEllipsoid()));
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // Add a tag for picking
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(this, this);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
}

GateNode::~GateNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

void GateNode::updateLabel_(const simData::GatePrefs& prefs)
{
  if (hasLastUpdate_)
  {
    std::string label = getEntityName(EntityNode::DISPLAY_NAME);
    if (prefs.commonprefs().labelprefs().namelength() > 0)
      label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

    std::string text;
    if (prefs.commonprefs().labelprefs().draw())
      text = contentCallback_->createString(prefs, lastUpdateFromDS_, prefs.commonprefs().labelprefs().displayfields());

    if (!text.empty())
    {
      label += "\n";
      label += text;
    }

    const float zOffset = 0.0f;
    label_->update(prefs.commonprefs(), label, zOffset);
  }
}

void GateNode::setLabelContentCallback(LabelContentCallback* cb)
{
  if (cb == NULL)
    contentCallback_ = new NullEntityCallback();
  else
    contentCallback_ = cb;
}

LabelContentCallback* GateNode::labelContentCallback() const
{
  return contentCallback_.get();
}

std::string GateNode::hookText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return contentCallback_->createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().hookdisplayfields());

  return "";
}

std::string GateNode::legendText() const
{
  if (hasLastPrefs_ && hasLastUpdate_)
    return contentCallback_->createString(lastPrefsFromDS_, lastUpdateFromDS_, lastPrefsFromDS_.commonprefs().labelprefs().legenddisplayfields());

  return "";
}

void GateNode::setPrefs(const simData::GatePrefs& prefs)
{
  // validate localgrid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());

  applyPrefs_(prefs);
  updateLabel_(prefs);
  lastPrefsFromDS_ = prefs;
}

void GateNode::applyPrefs_(const simData::GatePrefs& prefs, bool force)
{
  if (prefsOverrides_.size() == 0)
  {
    apply_(NULL, &prefs, force);
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
    apply_(NULL, &accumulated, force);
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
  switch (nameType)
  {
  case EntityNode::REAL_NAME:
    return lastPrefsApplied_.commonprefs().name();
  case EntityNode::ALIAS_NAME:
    return lastPrefsApplied_.commonprefs().alias();
  case EntityNode::DISPLAY_NAME:
    if (lastPrefsApplied_.commonprefs().usealias())
    {
      if (!lastPrefsApplied_.commonprefs().alias().empty() || allowBlankAlias)
        return lastPrefsApplied_.commonprefs().alias();
    }
    return lastPrefsApplied_.commonprefs().name();
  }
  return "";
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
    const bool gateChangedToInactive = (current == NULL && hasLastUpdate_);

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
  centroid_->setActive(false);
  removeChild(gateVolume_);
  gateVolume_ = NULL;
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
  return hasLastUpdate_ ? &lastUpdateFromDS_ : NULL;
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
    if (beam != NULL)
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
    apply_(&lastUpdateFromDS_, NULL, force);
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
    apply_(&accumulated, NULL, force);
    lastUpdateApplied_ = accumulated;
  }
  // we have applied a valid update, and both lastUpdateApplied_ and lastUpdateFromDS_ are valid
  hasLastUpdate_ = true;
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
  if (beam == NULL)
  {
    // target gate require a host beam; host is not a beam, so exit.
    return 1;
  }

  assert(beam->getProperties().type() == simData::BeamProperties_BeamType_TARGET);
  // the target beam should have the correct RAE; will be NULL if target beam could not calculate
  const simData::BeamUpdate* beamUpdate = beam->getLastUpdateFromDS();
  if (beamUpdate == NULL)
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
  assert(activePrefs != NULL);
  assert(activeUpdate != NULL);

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
    const bool isOpaque = activePrefs->fillpattern() == simData::GatePrefs_FillPattern_WIRE ||
                          activePrefs->fillpattern() == simData::GatePrefs_FillPattern_SOLID ||
                          activePrefs->fillpattern() == simData::GatePrefs_FillPattern_CENTROID;

    // blending is off for opaque graphics, so depth writing is on, otherwise off
    //depthAttr_->setWriteMask(isOpaque);

    if (gateVolume_)
    {
      removeChild(gateVolume_);
      gateVolume_ = NULL;
    }

    if (activePrefs->fillpattern() != simData::GatePrefs_FillPattern_CENTROID)
    {
      gateVolume_ = new GateVolume(gateVolumeLocator_.get(), activePrefs, activeUpdate);
      addChild(gateVolume_);
    }
    dirtyBound();
  }
  else if (gateVolume_)
  {
    if (newPrefs)
      gateVolume_->performInPlacePrefChanges(&lastPrefsApplied_, newPrefs);

    if (newUpdate)
      gateVolume_->performInPlaceUpdates(&lastUpdateApplied_, newUpdate);
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
    // make sure to activate the centroid locatorNode in case datadraw just turned on; updateLocator_ below will guarantee that locator node is sync'd to its locator
    centroid_->setActive(true);
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
    assert(localGrid_ != NULL);
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

  if (newPrefs != NULL &&
    (PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, fillpattern) ||
    PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, gatedrawmode) || PB_FIELD_CHANGED(&lastPrefsApplied_, newPrefs, drawoutline)))
  {
    return true;
  }

  if (newUpdate != NULL)
  {
    // changing a gate minrange to/from 0.0 requires a rebuild due to simplified shape
    if (PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, minrange) &&
      (newUpdate->minrange() == 0.0 || lastUpdateApplied_.minrange() == 0.0))
      return true;

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

unsigned int GateNode::objectIndexTag() const
{
  return objectIndexTag_;
}

}
