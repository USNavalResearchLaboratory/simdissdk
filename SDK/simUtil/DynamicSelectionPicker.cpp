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
#include "simVis/Scenario.h"
#include "simVis/View.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simUtil/ScreenCoordinateCalculator.h"
#include "simUtil/DynamicSelectionPicker.h"

namespace simUtil {

/** GUI Event Handler that forwards events to the picker */
class DynamicSelectionPicker::RepickEventHandler : public osgGA::GUIEventHandler
{
public:
  RepickEventHandler(DynamicSelectionPicker& picker)
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
      picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
      picker_.mouseXy_.set(ea.getX(), ea.getY());
      repickNeeded_ = true;
      break;

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
    minimumValidRange_(100.0), // pixels
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
  double closestRangePx = osg::square(minimumValidRange_);
  simVis::EntityNode* closest = NULL;

  // Loop through all entities
  for (auto i = allEntities.begin(); i != allEntities.end(); ++i)
  {
    if (!isPickable_(i->get()))
      continue;

    // Calculate the position on the platform
    const simUtil::ScreenCoordinate pos = calc.calculate(*(*i).get());
    // Ignore objects that are off screen or behind the camera
    if (pos.isBehindCamera() || pos.isOffScreen() || pos.isOverHorizon())
      continue;

    // Choose the closest object
    const double rangeSquared = (mouseXy_ - pos.position()).length2();
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

void DynamicSelectionPicker::setRange(double pixelsFromCenter)
{
  minimumValidRange_ = pixelsFromCenter;
}

void DynamicSelectionPicker::setPickMask(osg::Node::NodeMask pickMask)
{
  pickMask_ = pickMask;
}

}
