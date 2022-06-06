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
#include "osg/MatrixTransform"
#include "osgEarth/Horizon"
#include "osgEarth/ObjectIndex"
#include "simNotify/Notify.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/OverheadMode.h"
#include "simVis/OverrideColor.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/Scenario.h"
#include "simVis/CustomRendering.h"

namespace simVis
{

CustomRenderingNode::CustomRenderingNode(const ScenarioManager* scenario, const simData::CustomRenderingProperties& props, const EntityNode* host, int referenceYear)
  : EntityNode(simData::CUSTOM_RENDERING, new Locator()),
    scenario_(scenario),
    host_(host),
    lastProps_(props),
    hasLastPrefs_(false),
    customActive_(false),
    isLine_(false),
    objectIndexTag_(0)
{
  setName("CustomRenderingNode");

  localGrid_ = new LocalGridNode(getLocator(), host, referenceYear);
  addChild(localGrid_);

  label_ = new EntityLabelNode(getLocator());
  addChild(label_);

  // if hosted, note that horizon culling on host may also cull the custom rendering
  // horizon culling: entity culling based on bounding sphere
  addCullCallback(new osgEarth::HorizonCullCallback());
  // labels are culled based on entity center point
  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // create the locator node that will parent our geometry
  customLocatorNode_ = new LocatorNode(getLocator());
  customLocatorNode_->setEntityToMonitor(this);
  addChild(customLocatorNode_);

  // Apply the override color shader to the container
  overrideColor_ = new simVis::OverrideColor(customLocatorNode_->getOrCreateStateSet());
  overrideColor_->setCombineMode(OverrideColor::MULTIPLY_COLOR);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
  // SIM-10724: Labels need to not be flattened to be displayed in overhead mode
  simVis::OverheadMode::enableGeometryFlattening(false, label_.get());

  // Add a tag for picking
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(this, this);
}

CustomRenderingNode::~CustomRenderingNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);
}

void CustomRenderingNode::updateLabel_(const simData::CustomRenderingPrefs& prefs)
{
  std::string label = getEntityName_(prefs.commonprefs(), EntityNode::DISPLAY_NAME, false);
  if (prefs.commonprefs().labelprefs().namelength() > 0)
    label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

  std::string text;
  if (prefs.commonprefs().labelprefs().draw())
    text = labelContentCallback().createString(getId(), prefs, prefs.commonprefs().labelprefs().displayfields());

  if (!text.empty())
  {
    label += "\n";
    label += text;
  }

  const float zOffset = 0.0f;
  label_->update(prefs.commonprefs(), label, zOffset);
}

void CustomRenderingNode::setUpdateCallback(UpdateCallback* callback)
{
  updateCallback_ = callback;
}

CustomRenderingNode::UpdateCallback* CustomRenderingNode::updateCallback() const
{
  return updateCallback_.get();
}

double CustomRenderingNode::range() const
{
  return 0;
}

std::string CustomRenderingNode::popupText() const
{
  if (hasLastPrefs_ && customActive_)
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
    return prefix + labelContentCallback().createString(getId(), lastPrefs_, lastPrefs_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string CustomRenderingNode::hookText() const
{
  if (hasLastPrefs_)
    return labelContentCallback().createString(getId(), lastPrefs_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string CustomRenderingNode::legendText() const
{
  if (hasLastPrefs_)
    return labelContentCallback().createString(getId(), lastPrefs_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  return "";
}

void CustomRenderingNode::setPrefs(const simData::CustomRenderingPrefs& prefs)
{
  const bool prefsDraw = prefs.commonprefs().datadraw() && prefs.commonprefs().draw();
  // Visibility is determined by both customActive_ and draw state preferences
  setNodeMask((customActive_ && prefsDraw) ? simVis::DISPLAY_MASK_CUSTOM_RENDERING : simVis::DISPLAY_MASK_NONE);

  if (prefsDraw)
    updateLabel_(prefs);

  // validate localgrid prefs changes that might provide user notifications
  if (localGrid_.valid())
  {
    localGrid_->validatePrefs(prefs.commonprefs().localgrid());

    // update the local grid, only if platform drawn
    if (prefsDraw)
      localGrid_->setPrefs(prefs.commonprefs().localgrid());
  }

  updateOverrideColor_(prefs);
  applyProjectorPrefs_(lastPrefs_.commonprefs(), prefs.commonprefs());

  lastPrefs_ = prefs;
  hasLastPrefs_ = true;
}

bool CustomRenderingNode::isLine() const
{
  return isLine_;
}

void CustomRenderingNode::setIsLine(bool isLine)
{
  isLine_ = isLine;
}

void CustomRenderingNode::updateOverrideColor_(const simData::CustomRenderingPrefs& prefs)
{
  if (!overrideColor_.valid())
    return;

  if (hasLastPrefs_ &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, useoverridecolor) &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, overridecolor) &&
    !PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, commonprefs, color))
    return;

  // using an override color?
  if (prefs.commonprefs().useoverridecolor())
    overrideColor_->setColor(simVis::Color(prefs.commonprefs().overridecolor(), simVis::Color::RGBA));
  else
    overrideColor_->setColor(simVis::Color(prefs.commonprefs().color(), simVis::Color::RGBA));
}

bool CustomRenderingNode::isActive() const
{
  return customActive_;
}

bool CustomRenderingNode::isVisible() const
{
  return getNodeMask() != DISPLAY_MASK_NONE;
}

simData::ObjectId CustomRenderingNode::getId() const
{
  return lastProps_.id();
}

bool CustomRenderingNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

const std::string CustomRenderingNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  // if assert fails, check whether prefs are initialized correctly when entity is created
  assert(hasLastPrefs_);
  return getEntityName_(lastPrefs_.commonprefs(), nameType, allowBlankAlias);
}

bool CustomRenderingNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  if (updateCallback_ == nullptr)
    return false;

  if (updateCallback_->update(updateSliceBase, force))
  {
    dirtyBound();
    customLocatorNode_->dirtyBound();
    if (hasLastPrefs_)
    {
      updateOverrideColor_(lastPrefs_);
      updateLabel_(lastPrefs_);
    }
    return true;
  }

  return false;
}

void CustomRenderingNode::flush()
{
  setNodeMask(DISPLAY_MASK_NONE);
}

int CustomRenderingNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return customLocatorNode_->getPosition(out_position, coordsys);
}

int CustomRenderingNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!isActive())
    return 1;
  return customLocatorNode_->getPositionOrientation(out_position, out_orientation, coordsys);
}

unsigned int CustomRenderingNode::objectIndexTag() const
{
  return objectIndexTag_;
}

bool CustomRenderingNode::customActive() const
{
  return customActive_;
}

void CustomRenderingNode::setCustomActive(bool value)
{
  customActive_ = value;

  bool prefsDraw = customActive_;
  if (hasLastPrefs_)
    prefsDraw = lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw();
  // Visibility is determined by both customActive_ and draw state preferences
  setNodeMask((customActive_ && prefsDraw) ? DISPLAY_MASK_CUSTOM_RENDERING : DISPLAY_MASK_NONE);
}

LocatorNode* CustomRenderingNode::locatorNode() const
{
  return customLocatorNode_.get();
}

const EntityNode* CustomRenderingNode::host() const
{
  return host_.get();
}

void CustomRenderingNode::setPointPicker(const std::shared_ptr<AbstractPointPicker>& pointPicker)
{
  pointPicker_ = pointPicker;
}

void CustomRenderingNode::getPickingPoints(std::vector<osg::Vec3d>& ecefVec) const
{
  ecefVec.clear();

  if (!isActive() || !isVisible())
    return;

  if (pointPicker_)
  {
    pointPicker_->getPickingPoints(ecefVec);
    return;
  }

  // If no PointPicker was added, fall back on the locator position
  simCore::Vec3 locatorNodeEcef;
  if (getPosition(&locatorNodeEcef, simCore::COORD_SYS_ECEF) != 0)
    return;

  ecefVec.push_back(osg::Vec3d(locatorNodeEcef.x(), locatorNodeEcef.y(), locatorNodeEcef.z()));
}

}
