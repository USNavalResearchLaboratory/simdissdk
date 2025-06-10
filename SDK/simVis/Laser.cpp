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
#include "osg/Geometry"
#include "osgEarth/Horizon"
#include "osgEarth/LineDrawable"
#include "osgEarth/ObjectIndex"
#include "simCore/Calc/Math.h"
#include "simData/DataTypes.h"
#include "simNotify/Notify.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/Utils.h"
#include "simVis/OverheadMode.h"
#include "simVis/Laser.h"

namespace simVis
{

LaserNode::LaserNode(const simData::LaserProperties& props, Locator* hostLocator, const EntityNode* host, int referenceYear)
  : EntityNode(simData::LASER),
    hasLastUpdate_(false),
    node_(nullptr),
    host_(host),
    localGrid_(nullptr),
    hasLastPrefs_(false),
    label_(nullptr),
    objectIndexTag_(0)
{
  lastProps_ = props;
  Locator* locator = nullptr;

  if (!props.has_azelrelativetohostori() || !props.azelrelativetohostori())
  {
    // for non-relative case, we need to apply position offsets that are relative to platform orientation.
    // after having established the position offset,
    // we need to apply an orientation that is not relative to platform orientation : we need to filter out platform orientation.
    // the combination of these two locators gives us that.

    laserXYZOffsetLocator_ = new Locator(hostLocator, Locator::COMP_ALL);
    locator = new ResolvedPositionLocator(laserXYZOffsetLocator_.get(), Locator::COMP_ALL);
  }
  else
  {
    // in the azelrelativetohostori case, only a single locator is needed,
    // b/c position and orientation offsets are both relative to platform orientation.
    laserXYZOffsetLocator_ = nullptr;
    locator = new ResolvedPositionOrientationLocator(hostLocator, Locator::COMP_ALL);
  }

  setLocator(locator);
  setNodeMask(DISPLAY_MASK_NONE);
  locatorNode_ = new LocatorNode(locator);
  locatorNode_->setName("Laser");
  locatorNode_->setEntityToMonitor(this);
  addChild(locatorNode_);
  setName("LaserNode");

  localGrid_ = new LocalGridNode(getLocator(), host, referenceYear);
  addChild(localGrid_);

  label_ = new EntityLabelNode();
  locatorNode_->addChild(label_);

  // horizon culling: entity culling based on bounding sphere
  addCullCallback( new osgEarth::HorizonCullCallback() );
  // labels are culled based on entity center point
  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
  // SIM-10724: Labels need to not be flattened to be displayed in overhead mode
  simVis::OverheadMode::enableGeometryFlattening(false, label_.get());

  // Add a tag for picking
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(this, this);
}

LaserNode::~LaserNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

void LaserNode::updateLabel_(const simData::LaserPrefs& prefs)
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

  const float zOffset = 0.0f;
  label_->update(prefs.commonprefs(), label, zOffset);
}

std::string LaserNode::popupText() const
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

std::string LaserNode::hookText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string LaserNode::legendText() const
{
  if (hasLastUpdate_ && hasLastPrefs_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  return "";
}

void LaserNode::setPrefs(const simData::LaserPrefs& prefs)
{
  // validate localgrid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());

  refresh_(nullptr, &prefs);

  applyProjectorPrefs_(lastPrefs_.commonprefs(), prefs.commonprefs());
  updateLabel_(prefs);
  lastPrefs_ = prefs;
  hasLastPrefs_ = true;
}

bool LaserNode::isActive() const
{
  return hasLastUpdate_ && lastPrefs_.commonprefs().datadraw();
}

bool LaserNode::isVisible() const
{
  return getNodeMask() != DISPLAY_MASK_NONE && (node_ != nullptr) && node_->getNodeMask() != DISPLAY_MASK_NONE;
}

simData::ObjectId LaserNode::getId() const
{
  return lastProps_.id();
}

bool LaserNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

const std::string LaserNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // lastPrefs_ will have no meaningful default if never set
  if (!hasLastPrefs_)
    return "";

  return getEntityName_(lastPrefs_.commonprefs(), nameType, allowBlankAlias);
}

bool LaserNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  bool updateApplied = false;
  const simData::LaserUpdateSlice* updateSlice = static_cast<const simData::LaserUpdateSlice*>(updateSliceBase);
  assert(updateSlice);
  assert(host_.valid());

  const bool hostChangedToActive = host_->isActive() && !hasLastUpdate_;
  const bool hostChangedToInactive = !host_->isActive() && hasLastUpdate_;

  // if not hasChanged, not forcing, and not a host transition, there is no update to apply
  // Note: if entity is not interpolated, !updateSlice->hasChanged() happens a lot
  if (updateSlice->hasChanged() || force || hostChangedToActive || hostChangedToInactive)
  {
    const simData::LaserUpdate* current = updateSlice->current();
    const bool laserChangedToInactive = (current == nullptr && hasLastUpdate_);

    // do not apply update if host platform is not active
    if (current && (force || host_->isActive()))
    {
      refresh_(current, nullptr);
      lastUpdate_ = *current;
      hasLastUpdate_ = true;
      updateApplied = true;
      // ensure that the locator node is in sync with its locator; this will be a no-op if they are already in sync.
      locatorNode_->syncWithLocator();
    }
    else if (laserChangedToInactive || hostChangedToInactive)
    {
      // avoid applying a null update over and over - only apply the null update on the transition
      flush();
      updateApplied = true;
    }
  }

  // Whether updateSlice changed or not, label content may have changed, and for active beams we need to update
  if (isActive())
    updateLabel_(lastPrefs_);

  return updateApplied;
}

void LaserNode::flush()
{
  hasLastUpdate_ = false;
  setNodeMask(DISPLAY_MASK_NONE);
}

double LaserNode::range() const
{
  if (!lastPrefs_.has_maxrange())
    return 0.0;

  return lastPrefs_.maxrange();
}

const simData::LaserUpdate* LaserNode::getLastUpdateFromDS() const
{
  return hasLastUpdate_ ? &lastUpdate_ : nullptr;
}

int LaserNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return locatorNode_->getPosition(out_position, coordsys);
}

int LaserNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return locatorNode_->getPositionOrientation(out_position, out_orientation, coordsys);
}

void LaserNode::refresh_(const simData::LaserUpdate* newUpdate, const simData::LaserPrefs* newPrefs)
{
  // can't do anything until laser has props, prefs and an update
  // props are init'd in constructor, we assume prefs are set immediately after construction.
  if (!newUpdate && !hasLastUpdate_)
    return;

  // if we don't have new prefs, we will use the previous prefs
  const simData::LaserPrefs* activePrefs = newPrefs ? newPrefs : &lastPrefs_;
  // if we don't have new update, we will use the previous update
  const simData::LaserUpdate* activeUpdate = newUpdate ? newUpdate : &lastUpdate_;

  // if assert fails, check for changes in processing of prefs or updates
  assert(activePrefs && activeUpdate);

  // if datadraw is off, we do not need to do any processing
  if (activePrefs->commonprefs().datadraw() == false)
  {
    setNodeMask(DISPLAY_MASK_NONE);
    return;
  }

  // force indicates that activePrefs and activeUpdate must be applied, the visual must be redrawn, and the locator updated
  const bool force = !hasLastUpdate_ || !hasLastPrefs_ || node_ == nullptr ||
    (newPrefs && PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, commonprefs, datadraw));

  // if new geometry is required, build it
  const bool refreshRequiresNewNode = force ||
    (newPrefs && PB_FIELD_CHANGED(&lastPrefs_, newPrefs, maxrange));

  if (refreshRequiresNewNode)
  {
    osg::ref_ptr<osg::Node> oldNode = node_;
    node_ = createGeometry_(*activePrefs);
    node_->setCullingActive(false);
    node_->setNodeMask(DISPLAY_MASK_LASER);

    if (oldNode.valid())
      locatorNode_->replaceChild(oldNode, node_);
    else
      locatorNode_->addChild(node_);
    dirtyBound();
  }
  else
  {
    // Laser color & width changes do not require rebuilding geometry
    const bool requiresUpdate = newPrefs &&
      (PB_FIELD_CHANGED(&lastPrefs_, newPrefs, laserwidth) ||
        PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, commonprefs, color) ||
        PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, commonprefs, useoverridecolor) ||
        PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, commonprefs, overridecolor));

    if (requiresUpdate)
      updateLaser_(*newPrefs);
  }

  // update the visibility:
  // LaserOn turns datadraw pref on and off
  // we exit early (just above) if datadraw is off; if assert fails, check for changes to the early exit
  assert(activePrefs->commonprefs().datadraw());
  const bool visible = activePrefs->commonprefs().draw();
  setNodeMask(visible ? DISPLAY_MASK_LASER : DISPLAY_MASK_NONE);

  // update our locator, if required
  updateLocator_(newUpdate, newPrefs, force);

  // update the local grid prefs, if laser is being drawn
  if (visible && (force || newPrefs))
  {
    assert(localGrid_ != nullptr);
    localGrid_->setPrefs(activePrefs->commonprefs().localgrid(), force);
  }
}

void LaserNode::updateLocator_(const simData::LaserUpdate* newUpdate, const simData::LaserPrefs* newPrefs, bool force)
{
  const bool locatorUpdateRequired = force ||
    (newUpdate &&
      (newUpdate->yaw() != lastUpdate_.yaw() ||
      newUpdate->pitch() != lastUpdate_.pitch())) ||
    (newPrefs &&
      (PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, laserxyzoffset, x) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, laserxyzoffset, y) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, newPrefs, laserxyzoffset, z)));

  if (locatorUpdateRequired)
  {
    // if we don't have new prefs, we will use the previous prefs
    const simData::LaserPrefs* activePrefs = newPrefs ? newPrefs : &lastPrefs_;
    // if we don't have new update, we will use the previous update
    const simData::LaserUpdate* activeUpdate = newUpdate ? newUpdate : &lastUpdate_;

    // x/y order change and minus sign are needed to match the behavior of SIMDIS 9
    const simCore::Vec3 posOffset(-activePrefs->laserxyzoffset().y(),
                            activePrefs->laserxyzoffset().x(),
                            activePrefs->laserxyzoffset().z());

    const simCore::Vec3 oriOffset(activeUpdate->yaw(),
                                  activeUpdate->pitch(),
                                  0.0);

    if (!lastProps_.has_azelrelativetohostori() || !lastProps_.azelrelativetohostori())
    {
      // if assert fails, check that constructor creates this locator for non-relative lasers
      assert(laserXYZOffsetLocator_ != nullptr);

      // laser xyz offsets are relative to host platform orientation;
      laserXYZOffsetLocator_->setLocalOffsets(posOffset, simCore::Vec3(), activeUpdate->time(), false);
      // laser orientation is not-relative to host platform orientation;
      getLocator()->setLocalOffsets(simCore::Vec3(), oriOffset, activeUpdate->time(), false);
      // laserXYZOffsetLocator_ is parent to getLocator, its update will update both
      laserXYZOffsetLocator_->endUpdate();
    }
    else
    {
      getLocator()->setLocalOffsets(posOffset, oriOffset, activeUpdate->time());
    }

    dirtyBound();
  }
}

osg::Group* LaserNode::createGeometry_(const simData::LaserPrefs &prefs)
{
  const float length = prefs.maxrange();
  const double segmentLength = simCore::sdkMin(prefs.maxrange(), MAX_SEGMENT_LENGTH);
  const unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(length / segmentLength)));

  osgEarth::LineDrawable* g = new osgEarth::LineDrawable(GL_LINE_STRIP);
  g->setDataVariance(osg::Object::DYNAMIC);
  g->setName("simVis::LaserNode");

  // allocate the desired number of points, then generate them
  g->allocate(numSegs + 1);
  VectorScaling::generatePoints(*g, osg::Vec3(), osg::Vec3(0.0f, length, 0.0f));
  g->setColor(simVis::ColorUtils::RgbaToVec4(
    prefs.commonprefs().useoverridecolor() ? prefs.commonprefs().overridecolor() : prefs.commonprefs().color()));
  g->setLineWidth(prefs.laserwidth());

  // done
  osg::Group* lineGroup = new osgEarth::LineGroup();
  lineGroup->addChild(g);
  return lineGroup;
}

void LaserNode::updateLaser_(const simData::LaserPrefs &prefs)
{
  if (node_ == nullptr || node_->getNumChildren() == 0)
    return;
  osgEarth::LineDrawable* geom = dynamic_cast<osgEarth::LineDrawable*>(node_->getChild(0));
  if (!geom)
    return;

  const osg::Vec4f color = simVis::ColorUtils::RgbaToVec4(
    prefs.commonprefs().useoverridecolor() ? prefs.commonprefs().overridecolor() : prefs.commonprefs().color());
  geom->setColor(color);

  // update the laser width
  geom->setLineWidth(prefs.laserwidth());
}

void LaserNode::getVisibleEndPoints(std::vector<osg::Vec3d>& ecefVec) const
{
  ecefVec.clear();
  if (!isActive() || !hasLastPrefs_)
    return;

  // Pull the origin node from the locator node. This is more efficient than a matrix multiply
  simCore::Vec3 firstPos;
  locatorNode_->getPosition(&firstPos, simCore::COORD_SYS_ECEF);
  ecefVec.push_back(osg::Vec3d(firstPos.x(), firstPos.y(), firstPos.z()));

  // Use the full matrix of the locator node to calculate the correctly-oriented end point
  const float length = lastPrefs_.maxrange();
  const auto& locatorMatrix = locatorNode_->getMatrix();
  ecefVec.push_back(osg::Vec3d(0., length, 0.) * locatorMatrix);
}

unsigned int LaserNode::objectIndexTag() const
{
  return objectIndexTag_;
}

}
