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

#ifdef ENABLE_CUSTOM_RENDERING

#include "osg/Depth"
#include "osg/MatrixTransform"
#include "osgEarth/Horizon"
#include "simNotify/Notify.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/Scenario.h"
#include "simVis/CustomRendering.h"

namespace simVis
{

CustomRenderingNode::CustomRenderingNode(const ScenarioManager* scenario, const simData::CustomRenderingProperties& props, const EntityNode* host, int referenceYear)
  : EntityNode(simData::CUSTOM_RENDERING),
    scenario_(scenario),
    contentCallback_(new NullEntityCallback()),
    lastProps_(props),
    hasLastPrefs_(false),
    customActive_(false)
{
  // Independent of the host like a LOB
  setLocator(new Locator(host->getLocator()->getSRS()));
  setName("CustomRenderingNode");

  // set up a state set.
  // carefully set the rendering order for custom. We want to render them
  // before everything else (including the terrain) since they are
  // transparent and potentially self-blending
  osg::StateSet* stateSet = getOrCreateStateSet();
  stateSet->setRenderBinDetails(BIN_BEAM, BIN_TWO_PASS_ALPHA);

  localGrid_ = new LocalGridNode(getLocator(), host, referenceYear);
  addChild(localGrid_);

  label_ = new EntityLabelNode(getLocator());
  addChild(label_);

  // horizon culling:
  addCullCallback(new osgEarth::HorizonCullCallback());

  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  callback->setHorizon(new osgEarth::Horizon(*getLocator()->getSRS()->getEllipsoid()));
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // create the locator node that will parent our geometry
  customLocatorNode_ = new LocatorNode(getLocator());
  customLocatorNode_->setNodeMask(DISPLAY_MASK_NONE);
  addChild(customLocatorNode_);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);
}

CustomRenderingNode::~CustomRenderingNode()
{
}

void CustomRenderingNode::updateLabel_(const simData::CustomRenderingPrefs& prefs)
{
  std::string label = getEntityName(EntityNode::DISPLAY_NAME);
  if (prefs.commonprefs().labelprefs().namelength() > 0)
    label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

  std::string text = "This is a test, this only a test.";
  if (prefs.commonprefs().labelprefs().draw())
    text = contentCallback_->createString(getId(), prefs, prefs.commonprefs().labelprefs().displayfields());

  if (!text.empty())
  {
    label += "\n";
    label += text;
  }

  const float zOffset = 0.0f;
  label_->update(prefs.commonprefs(), label, zOffset);
}

void CustomRenderingNode::setLabelContentCallback(LabelContentCallback* cb)
{
  if (cb == NULL)
    contentCallback_ = new NullEntityCallback();
  else
    contentCallback_ = cb;
}

LabelContentCallback* CustomRenderingNode::labelContentCallback() const
{
  return contentCallback_.get();
}

double CustomRenderingNode::range() const
{
  return 0;
}

std::string CustomRenderingNode::hookText() const
{
  if (hasLastPrefs_)
    return contentCallback_->createString(getId(), lastPrefs_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());

  return "";
}

std::string CustomRenderingNode::legendText() const
{
  if (hasLastPrefs_)
    return contentCallback_->createString(getId(), lastPrefs_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());

  return "";
}

void CustomRenderingNode::setPrefs(const simData::CustomRenderingPrefs& prefs)
{
  // validate localgrid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());
  lastPrefs_ = prefs;
  hasLastPrefs_ = true;
  updateLabel_(prefs);
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

bool CustomRenderingNode::updateFromDataStore(const simData::DataSliceBase* updateSliceBase, bool force)
{
  dirtyBound();
  customLocatorNode_->dirtyBound();
  updateLabel_(lastPrefs_);

  return true;
}

void CustomRenderingNode::flush()
{
  setNodeMask(DISPLAY_MASK_NONE);
  customLocatorNode_->setNodeMask(DISPLAY_MASK_NONE);
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
  // Not supported for custom
  return 0;
}

bool CustomRenderingNode::customActive() const
{
  return customActive_;
}

void CustomRenderingNode::setCustomActive(bool value)
{
  customActive_ = value;
  customLocatorNode_->setNodeMask(customActive_ ? DISPLAY_MASK_CUSTOM_RENDERING : DISPLAY_MASK_NONE);
  setNodeMask(customActive_ ? DISPLAY_MASK_CUSTOM_RENDERING : DISPLAY_MASK_NONE);
}

LocatorNode* CustomRenderingNode::locatorNode() const
{
  return customLocatorNode_.get();
}

}

#endif // ENABLE_CUSTOM_RENDERING