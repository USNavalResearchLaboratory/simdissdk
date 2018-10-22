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
#include "simCore/Calc/Math.h"
#include "simVis/LobGroup.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Scenario.h"
#include "simVis/View.h"
#include "simUtil/ScreenCoordinateCalculator.h"
#include "simUtil/DynamicSelectionPicker.h"

namespace simUtil {

// pixel amount to test if a mouse position has moved enough to initiate a new pick
static const float MOUSE_MOVEMENT_PICK_THRESHOLD = 10.0;

/** GUI Event Handler that forwards events to the picker */
class DynamicSelectionPicker::RepickEventHandler : public osgGA::GUIEventHandler
{
public:
  explicit RepickEventHandler(DynamicSelectionPicker& picker)
    : picker_(picker),
      repickNeeded_(false)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
      // on move or drag, update position and indicate a repick is needed
      picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
      picker_.mouseXy_.set(ea.getX(), ea.getY());
      repickNeeded_ = true;
      break;

    case osgGA::GUIEventAdapter::PUSH:
      // if mouse position has changed, initiated a repick immediately, since push event can occur before move/drag has updated repick state
      if (fabs(ea.getX() - picker_.mouseXy_.x()) > MOUSE_MOVEMENT_PICK_THRESHOLD ||
        fabs(ea.getY() - picker_.mouseXy_.y()) > MOUSE_MOVEMENT_PICK_THRESHOLD)
      {
        picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
        picker_.mouseXy_.set(ea.getX(), ea.getY());
        picker_.pickThisFrame_();
      }

    case osgGA::GUIEventAdapter::FRAME:
      // If the mouse moved, we need to re-pick to capture movement
      if (repickNeeded_)
      {
        repickNeeded_ = false;
        picker_.pickThisFrame_();
      }
      break;

    default:
      // Most events: do nothing
      break;
    }
    // Never intercept an event
    return false;
  }

private:
  DynamicSelectionPicker& picker_;
  bool repickNeeded_;
  osg::Vec2d mouseXy_;
};


///////////////////////////////////////////////////////

DynamicSelectionPicker::DynamicSelectionPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager)
  : Picker(scenarioManager->getOrCreateStateSet()),
    viewManager_(viewManager),
    scenario_(scenarioManager),
    maximumValidRange_(100.0), // pixels
    pickMask_(simVis::DISPLAY_MASK_PLATFORM|simVis::DISPLAY_MASK_PLATFORM_MODEL)
{
  // By default, only platforms are picked.  Gates are feasibly pickable though.
  guiEventHandler_ = new RepickEventHandler(*this);
  addHandlerToViews_ = new simVis::AddEventHandlerToViews(guiEventHandler_.get());
  if (viewManager_.valid())
  {
    addHandlerToViews_->addToViews(*viewManager_);
    viewManager_->addCallback(addHandlerToViews_.get());
  }
}

DynamicSelectionPicker::~DynamicSelectionPicker()
{
  if (viewManager_.valid())
  {
    addHandlerToViews_->removeFromViews(*viewManager_);
    viewManager_->removeCallback(addHandlerToViews_.get());
  }
}

void DynamicSelectionPicker::pickThisFrame_()
{
  // Create a calculator for screen coordinates
  simUtil::ScreenCoordinateCalculator calc;
  calc.updateMatrix(*lastMouseView_);

  // Request all entities from the scenario
  simVis::EntityVector allEntities;
  scenario_->getAllEntities(allEntities);

  // We square the range to avoid sqrt() in a tight loop
  double closestRangePx = osg::square(maximumValidRange_);
  simVis::EntityNode* closest = NULL;

  // Loop through all entities
  for (auto i = allEntities.begin(); i != allEntities.end(); ++i)
  {
    if (!isPickable_(i->get()))
      continue;

    // Ask the calculator for the range from the mouse position
    double rangeSquared;
    if (calculateSquaredRange_(calc, *i->get(), rangeSquared) != 0)
      continue;

    // Choose the closest object
    if (rangeSquared < closestRangePx)
    {
      closestRangePx = rangeSquared;
      closest = i->get();
    }
  }

  // Pick the platform
  if (closest)
    setPicked_(closest->objectIndexTag(), closest);
  else
    setPicked_(0, closest);
}

bool DynamicSelectionPicker::isPickable_(const simVis::EntityNode* entityNode) const
{
  // Avoid NULL and things that don't match the mask
  if (entityNode == NULL || (entityNode->getNodeMask() & pickMask_) == 0)
    return false;
  // Only pick entities with object index tags
  if (entityNode->objectIndexTag() == 0)
    return false;

  // Do not pick inactive or invisible entities
  return entityNode->isActive() && entityNode->isVisible();
}

int DynamicSelectionPicker::calculateSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::EntityNode& entityNode, double& rangeSquared) const
{
  // Fall back to the LOB case if it's requesting a LOB, since it picks individual points on the lines shown
  const simVis::LobGroupNode* lobNode = dynamic_cast<const simVis::LobGroupNode*>(&entityNode);
  if (lobNode)
    return calculateLobSquaredRange_(calc, *lobNode, rangeSquared);

  const simUtil::ScreenCoordinate& pos = calc.calculate(entityNode);
  // Ignore objects that are off screen or behind the camera
  if (pos.isBehindCamera() || pos.isOffScreen() || pos.isOverHorizon())
    return 1;
  rangeSquared = (mouseXy_ - pos.position()).length2();
  return 0;
}

int DynamicSelectionPicker::calculateLobSquaredRange_(simUtil::ScreenCoordinateCalculator& calc, const simVis::LobGroupNode& lobNode, double& rangeSquared) const
{
  rangeSquared = std::numeric_limits<double>::max();
  // Pull out the vector of all endpoints on the LOB that are visible
  std::vector<osg::Vec3d> ecefVec;
  lobNode.getVisibleEndPoints(ecefVec);
  if (ecefVec.empty())
    return 1;

  for (auto i = ecefVec.begin(); i != ecefVec.end(); ++i)
  {
    const simUtil::ScreenCoordinate& pos = calc.calculateEcef(simCore::Vec3(i->x(), i->y(), i->z()));
    // Ignore objects that are off screen or behind the camera
    if (pos.isBehindCamera() || pos.isOffScreen() || pos.isOverHorizon())
      continue;
    rangeSquared = simCore::sdkMin(rangeSquared, (mouseXy_ - pos.position()).length2());
  }

  return (rangeSquared == std::numeric_limits<double>::max()) ? 1 : 0;
}

void DynamicSelectionPicker::setRange(double pixelsFromCenter)
{
  maximumValidRange_ = pixelsFromCenter;
}

void DynamicSelectionPicker::setPickMask(osg::Node::NodeMask pickMask)
{
  pickMask_ = pickMask;
}

osg::Node::NodeMask DynamicSelectionPicker::pickMask() const
{
  return pickMask_;
}

}
