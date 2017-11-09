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
#include "osg/Geode"
#include "simCore/Calc/Angle.h"
#include "simData/DataTypes.h"
#include "simNotify/Notify.h"

#include "simVis/Beam.h"
#include "simVis/Constants.h"
#include "simVis/EntityLabel.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/SphericalVolume.h"
#include "simVis/OverheadMode.h"
#include "simVis/Gate.h"

// --------------------------------------------------------------------------

namespace
{
  osg::MatrixTransform* createNode(
    const simData::GateProperties* props,
    const simData::GatePrefs*      prefs,
    const simData::GateUpdate*     update)
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
}

// --------------------------------------------------------------------------

namespace simVis
{

GateCentroid::GateCentroid(const simData::GateUpdate& update)
{
  osg::Geometry* geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  osg::Vec4Array* c = new osg::Vec4Array(1);
  (*c)[0] = osg::Vec4f(1.0f, 1.0f, 1.0f, 1.0f); // always white
  geom->setColorArray(c);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);

  osg::Vec3Array* v = new osg::Vec3Array(6);
  geom->setVertexArray(v);

  updateCentroid_(v, update);

  osg::DrawElementsUShort* centroid = new osg::DrawElementsUShort(GL_LINES, 6);
  for (unsigned int i = 0; i < 6; ++i)
    centroid->setElement(i, i);
  geom->addPrimitiveSet(centroid);

  geom->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);

  osg::Geode* geodeSolid = new osg::Geode();
  geodeSolid->addDrawable(geom);
  addChild(geodeSolid);
}

// perform an in-place update to an existing centroid
void GateCentroid::update(const simData::GateUpdate& update)
{
  osg::Geometry*  geom = static_cast<osg::Geode*>(getChild(0))->getDrawable(0)->asGeometry();
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  updateCentroid_(verts, update);
  geom->dirtyBound();
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


GateNode::GateNode(const simData::GateProperties& props, Locator* hostLocator, const EntityNode* host, int referenceYear) :
EntityNode(simData::GATE),
hasLastUpdate_(false),
hasLastPrefs_(false),
visible_(false), // gets set on first refresh
gateMatrixTransform_(NULL),
centroid_(NULL),
host_(host),
localGrid_(NULL),
label_(NULL),
contentCallback_(new NullEntityCallback())
{
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
  Locator* gateVolumeLocator = NULL;
  Locator* centroidLocator = NULL;

  // if the properties call for a body-relative beam, reconfigure locators to include platform orientation
  if (props.has_type() && props.type() == simData::GateProperties_GateType_BODY_RELATIVE)
  {
    // for body beam, inherit from beam's locator, including its position offset, but stripping out only beam orientation offset (keeping platform orientation).
    gateVolumeLocator = new ResolvedPositionOrientationLocator(
      hostLocator, Locator::COMP_ALL);

    // for body beam, inherit from beam's locator, including its position offset, but stripping out only beam orientation offset (keeping platform orientation).
    baseLocator_ = new ResolvedPositionOrientationLocator(
      hostLocator, Locator::COMP_ALL);

    // this locator sets the centroid position offset from the platform, using the gate orientation offsets
    centroidPositionOffsetLocator_ = new Locator(baseLocator_, Locator::COMP_ALL);

    // inherit the gate centroid and the platform orientation, without beam orientation offsets, then adds back (as local offsets) the gate orientation
    centroidLocator = new ResolvedPositionOrientationLocator(
      centroidPositionOffsetLocator_, Locator::COMP_ALL);
  }
  else
  {
    // inherit from beam's locator, including its position offset, but stripping out all orientation
    gateVolumeLocator = new ResolvedPositionLocator(
      hostLocator, Locator::COMP_ALL);

    // inherit from beam's locator, including its position offset, but stripping out all orientation
    baseLocator_ = new ResolvedPositionLocator(
      hostLocator, Locator::COMP_ALL);

    // this locator sets the centroid position offset from the platform, using the gate orientation offsets
    centroidPositionOffsetLocator_ = new Locator(baseLocator_, Locator::COMP_ALL);

    // this locator starts with the resolved centroid position, with identity orientation, then adds back (as local offsets) the gate orientation
    centroidLocator = new ResolvedPositionLocator(
      centroidPositionOffsetLocator_, Locator::COMP_ALL);
  }

  // create a locatorNode to parent the gate volume visual
  gateLocatorNode_ = new LocatorNode(gateVolumeLocator);
  gateLocatorNode_->setName("Gate");
  this->addChild(gateLocatorNode_);

  // the gate's locator represents the position and orientation of the gate centroid
  setLocator(centroidLocator);

  centroidLocatorNode_ = new LocatorNode(centroidLocator);
  centroidLocatorNode_->setName("GateCentroid");
  this->addChild(centroidLocatorNode_);

  localGrid_ = new LocalGridNode(centroidLocatorNode_->getLocator(), host, referenceYear);
  this->addChild(localGrid_);

  // set up a state set.
  // carefully set the rendering order for gates. We want to render them
  // before everything else (including the terrain) since they are
  // transparent and potentially self-blending
  osg::StateSet* stateSet = this->getOrCreateStateSet();
  stateSet->setRenderBinDetails(BIN_GATE, BIN_GLOBAL_SIMSDK); //"RenderBin");

  // depth-writing is disabled for the gates.
  // the gates draw before anything else (including the terrain) along with beams.
  depthAttr_ = new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false);
  stateSet->setAttributeAndModes(depthAttr_, osg::StateAttribute::ON);

  osg::Group* labelRoot = new LocatorNode(new Locator(getLocator(), Locator::COMP_POSITION));
  label_ = new EntityLabelNode(labelRoot);
  this->addChild(labelRoot);

  // horizon culling:
  this->addCullCallback( new osgEarth::HorizonCullCallback() );

  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setHorizon(new osgEarth::Horizon(*getLocator()->getSRS()->getEllipsoid()));
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
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
    for (PrefsOverrides::iterator i = prefsOverrides_.begin(); i != prefsOverrides_.end(); ++i)
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
  return getNodeMask() != DISPLAY_MASK_NONE && (gateMatrixTransform_ != NULL) && gateMatrixTransform_->getNodeMask() != DISPLAY_MASK_NONE;
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

      // draw the gate if hasLastUpdate_(valid update) and visible_ (prefs)
      if (visible_)
        setNodeMask(DISPLAY_MASK_GATE);
      else
      {
        // if commands/prefs have turned the gate off, DISPLAY_MASK_NONE will already be set
        assert(getNodeMask() == DISPLAY_MASK_NONE);
      }
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
}

double GateNode::range() const
{
  return (hasLastUpdate_ ? lastUpdateFromDS_.centroid() : 0.0);
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
    for (UpdateOverrides::iterator i = updateOverrides_.begin(); i != updateOverrides_.end(); ++i)
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
    return;

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
    visible_ = false;
    setNodeMask(DISPLAY_MASK_NONE);
    return;
  }

  // force indicates that activePrefs and activeUpdate must be applied, the visual must be redrawn, and the locator updated
  force = force || !hasLastUpdate_ || !hasLastPrefs_ || gateMatrixTransform_ == NULL ||
    (newPrefs && PB_SUBFIELD_CHANGED(&lastPrefsApplied_, newPrefs, commonprefs, datadraw));

  // do we need to redraw gate volume visual?
  const bool refreshRequiresNewNode = force || changeRequiresRebuild_(newUpdate, newPrefs);
  if (refreshRequiresNewNode)
  {
    // alpha or stipple fill pattern should use BIN_GATE, but if outline is on, it should be written (separately) to BIN_OPAQUE_GATE
    const bool isOpaque = activePrefs->fillpattern() == simData::GatePrefs_FillPattern_WIRE ||
                          activePrefs->fillpattern() == simData::GatePrefs_FillPattern_SOLID;
    getOrCreateStateSet()->setRenderBinDetails((isOpaque ? BIN_OPAQUE_GATE : BIN_GATE), BIN_GLOBAL_SIMSDK);

    // blending is off for solid or outline-only, so depth writing is on, otherwise off
    depthAttr_->setWriteMask(isOpaque);

    gateMatrixTransform_ = createNode(&lastProps_, activePrefs, activeUpdate);
    gateMatrixTransform_->setNodeMask(DISPLAY_MASK_GATE);

    osg::Geometry* outlineGeometry = simVis::SVFactory::outlineGeometry(gateMatrixTransform_);
    if (outlineGeometry != NULL)
    {
      outlineGeometry->getOrCreateStateSet()->setRenderBinDetails(BIN_OPAQUE_GATE, BIN_GLOBAL_SIMSDK);
    }
    if (gateLocatorNode_->getNumChildren() > 0)
      gateLocatorNode_->replaceChild(gateLocatorNode_->getChild(0), gateMatrixTransform_);
    else
      gateLocatorNode_->addChild(gateMatrixTransform_);

    dirtyBound();
  }
  else
  {
    if (newPrefs)
      performInPlacePrefChanges_(&lastPrefsApplied_, newPrefs, gateMatrixTransform_);

    if (newUpdate)
      performInPlaceUpdates_(&lastUpdateApplied_, newUpdate, gateMatrixTransform_);
  }

  // process changes that affect centroid visual
  if (!centroid_ &&
      (activePrefs->drawcentroid() ||
      activePrefs->fillpattern() == simData::GatePrefs_FillPattern_CENTROID))
  {
    centroid_ = new GateCentroid(*activeUpdate);
    centroid_->setNodeMask(DISPLAY_MASK_GATE);
    centroidLocatorNode_->addChild(centroid_);
  }
  else if (centroid_)
  {
    if (!activePrefs->drawcentroid() &&
      activePrefs->fillpattern() != simData::GatePrefs_FillPattern_CENTROID)
    {
      centroidLocatorNode_->removeChild(centroid_);
      centroid_ = NULL;
    }
    else if (force ||
      (newUpdate && (
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, minrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, maxrange) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, width) ||
      PB_FIELD_CHANGED(&lastUpdateApplied_, newUpdate, height))))
    {
      // activeUpdate is always valid, and points to the new update if there is a new update, or the previous update otherwise
      centroid_->update(*activeUpdate);
    }
  }

  // GateOnOffCmd turns datadraw pref on and off
  // we exit early at top if datadraw is off; if assert fails, check for changes to the early exit
  assert(activePrefs->commonprefs().datadraw());
  visible_ = activePrefs->commonprefs().draw();
  setNodeMask(visible_ ? DISPLAY_MASK_GATE : DISPLAY_MASK_NONE);

  // is a locator update required?
  updateLocator_(newUpdate, newPrefs, force);

  // update the local grid prefs, if gate is being drawn
  if (newPrefs && visible_)
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
    gateLocatorNode_->getLocator()->setLocalOffsets(
      simCore::Vec3(0, 0, 0),
      simCore::Vec3(activePrefs->gateazimuthoffset(), activePrefs->gateelevationoffset(), activePrefs->gaterolloffset()),
      activeUpdate->time());
  }
  else
  {
    // not a coverage gate, so apply the local orientation
    gateLocatorNode_->getLocator()->setLocalOffsets(
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
  centroidLocatorNode_->getLocator()->setLocalOffsets(
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

/// prefs that can be applied without rebuilding the whole gate
void GateNode::performInPlacePrefChanges_(const simData::GatePrefs* a,
                                          const simData::GatePrefs* b,
                                          osg::MatrixTransform* node)
{
  if (b->commonprefs().useoverridecolor())
  {
    // Check for transition between color and override color than check for color change
    if (PB_SUBFIELD_CHANGED(a, b, commonprefs, useoverridecolor) || PB_SUBFIELD_CHANGED(a, b, commonprefs, overridecolor))
    {
      SVFactory::updateColor(node, simVis::Color(b->commonprefs().overridecolor(), simVis::Color::RGBA));
    }
  }
  else
  {
    // Check for transition between color and override color than check for color change
    if (a->commonprefs().useoverridecolor() || PB_SUBFIELD_CHANGED(a, b, commonprefs, color))
    {
      SVFactory::updateColor(node, simVis::Color(b->commonprefs().color(), simVis::Color::RGBA));
    }
  }

  if (PB_FIELD_CHANGED(a, b, gatelighting))
  {
    SVFactory::updateLighting(node, b->gatelighting());
  }
}

/// updates that can be updated without rebuilding the whole gate
void GateNode::performInPlaceUpdates_(const simData::GateUpdate* a,
                                     const simData::GateUpdate* b,
                                     osg::MatrixTransform*      node)
{
  if (PB_FIELD_CHANGED(a, b, minrange))
  {
    SVFactory::updateNearRange(node, b->minrange());
  }
  if (PB_FIELD_CHANGED(a, b, maxrange))
  {
    SVFactory::updateFarRange(node, b->maxrange());
  }
  if (PB_FIELD_CHANGED(a, b, width) && PB_BOTH_HAVE_FIELD(a, b, width))
  {
    SVFactory::updateHorizAngle(node, a->width(), b->width());
  }
  if (PB_FIELD_CHANGED(a, b, height) && PB_BOTH_HAVE_FIELD(a, b, height))
  {
    SVFactory::updateVertAngle(node, a->height(), b->height());
  }
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
  PrefsOverrides::iterator i = prefsOverrides_.find(id);
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
  UpdateOverrides::iterator i = updateOverrides_.find(id);
  if (i != updateOverrides_.end())
  {
    updateOverrides_.erase(i);

    // re-apply the update state with the override removed
    if (hasLastUpdate_)
      applyUpdateOverrides_(true);
  }
}
}

