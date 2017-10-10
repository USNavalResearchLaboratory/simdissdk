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
      // Only pick on mouse move
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
    minimumValidRange_(100.0) // pixels
{
  guiEventHandler_ = new RepickEventHandler(*this);
  addHandlerToViews_ = new simVis::AddEventHandlerToViews(guiEventHandler_);
  if (viewManager_.valid())
  {
    addHandlerToViews_->addToViews(*viewManager_);
    viewManager_->addCallback(addHandlerToViews_);
  }
}

DynamicSelectionPicker::~DynamicSelectionPicker()
{
  if (viewManager_.valid())
  {
    addHandlerToViews_->removeFromViews(*viewManager_);
    viewManager_->removeCallback(addHandlerToViews_);
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
  simVis::PlatformNode* closest = NULL;

  // Loop through all entities
  for (auto i = allEntities.begin(); i != allEntities.end(); ++i)
  {
    // Hit every platform node -- skip if not a platform
    simVis::PlatformNode* plat = dynamic_cast<simVis::PlatformNode*>((*i).get());
    if (!plat || !plat->isActive())
      continue;

    // Calculate the position on the platform
    const simUtil::ScreenCoordinate pos = calc.calculate(*plat);
    // Ignore objects that are off screen or behind the camera
    if (pos.isBehindCamera() || pos.isOffScreen())
      continue;

    // NOTE: No horizon checks are done here.  It might be worthwhile to do a horizon cull

    // Choose the closest object
    const double rangeSquared = (mouseXy_ - pos.position()).length2();
    if (rangeSquared < closestRangePx)
    {
      closestRangePx = rangeSquared;
      closest = plat;
    }
  }

  // Pick the platform
  if (closest)
    setPicked_(closest->getModel()->objectIndexTag(), closest);
  else
    setPicked_(0, closest);
}

void DynamicSelectionPicker::setRange(double pixelsFromCenter)
{
  minimumValidRange_ = pixelsFromCenter;
}

}
