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
#include <cassert>
#include <algorithm>
#include "simCore/Calc/Angle.h"
#include "simVis/Entity.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/EarthManipulator.h"
#include "simUtil/ViewpointPositions.h"
#include "simUtil/ViewpointMonitor.h"

namespace simUtil
{

EyePositionState::EyePositionState(simVis::View* view)
{
  fillFromView_(view);
}

EyePositionState::~EyePositionState()
{
}

simVis::View* EyePositionState::view() const
{
  return view_.get();
}

osg::Node* EyePositionState::tetherNode() const
{
  return tetherNode_.get();
}

simVis::EntityNode* EyePositionState::watchedNode() const
{
  return watchedNode_.get();
}

bool EyePositionState::isTethered() const
{
  return isTethered_;
}

bool EyePositionState::isWatching() const
{
  return isWatching_;
}

bool EyePositionState::isOverheadMode() const
{
  return isOverheadMode_;
}

simCore::Vec3 EyePositionState::centerLla() const
{
  return centerLla_;
}

simCore::Vec3 EyePositionState::eyeLla() const
{
  return eyeLla_;
}

simCore::Vec3 EyePositionState::offsetXyz() const
{
  return offsetXyz_;
}

simCore::Vec3 EyePositionState::rangeAzEl() const
{
  return rangeAzEl_;
}

osgEarth::Util::EarthManipulator::TetherMode EyePositionState::tetherMode() const
{
  return tetherMode_;
}

bool EyePositionState::isHeadingLocked() const
{
  return headingLocked_;
}

bool EyePositionState::isPitchLocked() const
{
  return pitchLocked_;
}

void EyePositionState::fillFromView_(simVis::View* view)
{
  view_ = view;

  if (view_ == nullptr)
  {
    tetherNode_ = nullptr;
    watchedNode_ = nullptr;
    isTethered_ = false;
    isWatching_ = false;
    isOverheadMode_ = false;
    centerLla_.set(0, 0, 0);
    eyeLla_.set(0, 0, 0);
    rangeAzEl_.set(0, 0, 0);
    offsetXyz_.set(0, 0, 0);
    tetherMode_ = osgEarth::Util::EarthManipulator::TETHER_CENTER;
    headingLocked_ = false;
    pitchLocked_ = false;
    return;
  }

  // Simple view parameters
  isOverheadMode_ = view_->isOverheadEnabled();
  tetherNode_ = view_->getCameraTether();
  isTethered_ = tetherNode_.valid();
  watchedNode_ = view_->getWatchedNode();
  isWatching_ = watchedNode_.valid();

  // If watching, then the tether node returned from view may be wrong (it may be
  // nullptr due to how Watch is implemented).  Return instead the watcher node.
  if (isWatching_)
  {
    isTethered_ = true;
    tetherNode_ = view_->getWatcherNode();
  }

  // Tether mode comes from the Earth Manipulator
  const osgEarth::Util::EarthManipulator* manip = dynamic_cast<const osgEarth::Util::EarthManipulator*>(view_->getCameraManipulator());
  if (manip == nullptr || manip->getSettings() == nullptr)
    tetherMode_ = osgEarth::Util::EarthManipulator::TETHER_CENTER;
  else
    tetherMode_ = manip->getSettings()->getTetherMode();
  // Lock settings come from the simVis::EarthManipulator
  const simVis::EarthManipulator* svManip = dynamic_cast<const simVis::EarthManipulator*>(manip);
  if (svManip == nullptr)
    headingLocked_ = pitchLocked_ = false;
  else
  {
    headingLocked_ = svManip->isHeadingLocked();
    pitchLocked_ = svManip->isPitchLocked();
  }

  // Pull out parameters from the Viewpoint
  simVis::Viewpoint vp = view_->getViewpoint();
  offsetXyz_.set(vp.positionOffset()->x(), vp.positionOffset()->y(), vp.positionOffset()->z());

  // Pull out az/el from the manipulator directly
  double azToEye = 0.0;
  double elToEye = 0.0;
  if (manip != nullptr)
    manip->getCompositeEulerAngles(&azToEye, &elToEye);
  else
  {
    // Fall back to the viewpoint
    azToEye = vp.heading()->as(osgEarth::Units::RADIANS);
    elToEye = vp.pitch()->as(osgEarth::Units::RADIANS);
  }

  rangeAzEl_.set(vp.range()->as(osgEarth::Units::METERS), simCore::angFix2PI(azToEye), simCore::angFixPI2(elToEye));
  centerLla_ = ViewpointPositions::centerLla(vp);
  eyeLla_ = ViewpointPositions::eyeLla(*view_);
}

/////////////////////////////////////////////////////////////////////////

/// Binds ViewManager to ViewpointMonitor, so that new views get announcements and deleted views get removed
class ViewpointMonitor::ViewManagerObserver : public simVis::ViewManager::Callback
{
public:
  /** Constructor */
  explicit ViewManagerObserver(ViewpointMonitor& vpMonitor)
    : vpMonitor_(vpMonitor)
  {
  }

  /** Add and remove views */
  virtual void operator()(simVis::View* inset, const simVis::ViewManager::Callback::EventType& e)
  {
    switch (e)
    {
    case VIEW_REMOVED:
    {
      // Remove all records of the View
      std::map<simVis::View*, EyePositionState*>::iterator i = vpMonitor_.eyeStates_.find(inset);
      if (i != vpMonitor_.eyeStates_.end())
      {
        delete i->second;
        vpMonitor_.eyeStates_.erase(i);
      }
      break;
    }

    case VIEW_ADDED:
    {
      // Create and save the new state
      EyePositionState* newState = new EyePositionState(inset);
      vpMonitor_.eyeStates_[inset] = newState;
      // Inform all listeners that the values changed (e.g. from no value to some value)
      vpMonitor_.fireIsTetheredChanged_(inset, newState->isTethered());
      vpMonitor_.fireTetherChanged_(inset, newState->tetherNode());
      vpMonitor_.fireIsWatchingChanged_(inset, newState->isWatching());
      vpMonitor_.fireWatchedChanged_(inset, newState->watchedNode());
      vpMonitor_.fireIsOverheadChanged_(inset, newState->isOverheadMode());
      vpMonitor_.fireCenterLlaChanged_(inset, newState->centerLla());
      vpMonitor_.fireEyeLlaChanged_(inset, newState->eyeLla());
      vpMonitor_.fireRangeAzElChanged_(inset, newState->rangeAzEl());
      vpMonitor_.fireOffsetXyzChanged_(inset, newState->offsetXyz());
      vpMonitor_.fireTetherModeChanged_(inset, newState->tetherMode());
      vpMonitor_.fireMouseAxisLockChanged_(inset, newState->isHeadingLocked(), newState->isPitchLocked());
      vpMonitor_.fireChanged_(inset);
      break;
    }
    }
  }

private:
  ViewpointMonitor& vpMonitor_;
};

/////////////////////////////////////////////////////////////////////////

/**
  * On frame redraws, will detect changes in the view so the host
  * ViewpointMonitor can send out correct observer notifications
  */
class ViewpointMonitor::RedrawHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit RedrawHandler(ViewpointMonitor& vpMonitor)
    : vpMonitor_(vpMonitor)
  {
  }

  /** Handles frame updates and returns false so other handlers can process as well */
  virtual bool handle(const osgGA::GUIEventAdapter& eventAdapter, osgGA::GUIActionAdapter& actionAdapter)
  {
    if (eventAdapter.getEventType() == osgGA::GUIEventAdapter::FRAME)
      vpMonitor_.detectAllChanges_();
    // Do not intercept
    return false;
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "ViewpointMonitor::RedrawHandler"; }

private:
  ViewpointMonitor& vpMonitor_;
};

/////////////////////////////////////////////////////////////////////////

ViewpointMonitor::ViewpointMonitor(simVis::View* mainView)
  : mainView_(mainView)
{
  viewManagerObserver_ = new ViewManagerObserver(*this);

  // Initialize all the view eye position states
  simVis::ViewManager* viewManager = mainView_->getViewManager();
  if (viewManager != nullptr)
  {
    // Add an observer so we know when views are added or removed
    viewManager->addCallback(viewManagerObserver_.get());

    // Loop through all views
    std::vector<simVis::View*> views;
    viewManager->getViews(views);
    for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
    {
      // Don't register SuperHUDs
      if ((*i)->type() != simVis::View::VIEW_SUPERHUD)
        eyeStates_[*i] = new EyePositionState(*i);
    }
  }

  // Add a handler to the display so that on frame redraws we detect changes to views
  redrawHandler_ = new RedrawHandler(*this);
  if (mainView_.valid())
    mainView_->addEventHandler(redrawHandler_);
}

ViewpointMonitor::~ViewpointMonitor()
{
  // Remove our observers
  if (mainView_.valid())
  {
    if (mainView_->getViewManager())
      mainView_->getViewManager()->removeCallback(viewManagerObserver_.get());
    mainView_->removeEventHandler(redrawHandler_.get());
  }

  // Delete all the eye states
  for (std::map<simVis::View*, EyePositionState*>::const_iterator i = eyeStates_.begin(); i != eyeStates_.end(); ++i)
    delete i->second;
  eyeStates_.clear();
}

const EyePositionState* ViewpointMonitor::eyePositionState(simVis::View* view) const
{
  std::map<simVis::View*, EyePositionState*>::const_iterator i = eyeStates_.find(view);
  if (i == eyeStates_.end())
    return nullptr;
  return i->second;
}

void ViewpointMonitor::addObserver(ObserverPtr observer)
{
  observers_.push_back(observer);
}

void ViewpointMonitor::removeObserver(ObserverPtr observer)
{
  // Use erase/remove idiom to remove all instances of the observer
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

void ViewpointMonitor::detectAllChanges_()
{
  // First, set up eyeStates_ so that it's valid -- at this point, consider that all the
  // EyePositionState pointers are present and stale.
  std::map<simVis::View*, EyePositionState*> oldStates = eyeStates_;
  for (std::map<simVis::View*, EyePositionState*>::iterator i = eyeStates_.begin(); i != eyeStates_.end(); ++i)
  {
    i->second = new EyePositionState(i->first);
  }

  // Now detect changes for each individual eye state
  std::map<simVis::View*, EyePositionState*>::const_iterator newIter = eyeStates_.begin();
  std::map<simVis::View*, EyePositionState*>::const_iterator oldIter = oldStates.begin();
  while (newIter != eyeStates_.end() && oldIter != oldStates.end())
  {
    // Assumption: Iterating through the maps leads to the same keys in each one
    assert(newIter->first == oldIter->first);

    // Detect changes between the old version and new version
    detectChanges_(newIter->first, *newIter->second, *oldIter->second);
    ++newIter;

    // Clean up the pointers as we go
    delete oldIter->second;
    ++oldIter;
  }
}

void ViewpointMonitor::detectChanges_(simVis::View* view, const EyePositionState& newState, const EyePositionState& oldState)
{
  bool changed = false;
  if (newState.isTethered() != oldState.isTethered())
  {
    changed = true;
    fireIsTetheredChanged_(view, newState.isTethered());
  }
  if (newState.tetherNode() != oldState.tetherNode())
  {
    changed = true;
    fireTetherChanged_(view, newState.tetherNode());
  }
  if (newState.isWatching() != oldState.isWatching())
  {
    changed = true;
    fireIsWatchingChanged_(view, newState.isWatching());
  }
  if (newState.watchedNode() != oldState.watchedNode())
  {
    changed = true;
    fireWatchedChanged_(view, newState.watchedNode());
  }
  if (newState.isOverheadMode() != oldState.isOverheadMode())
  {
    changed = true;
    fireIsOverheadChanged_(view, newState.isOverheadMode());
  }
  if (newState.centerLla() != oldState.centerLla())
  {
    changed = true;
    fireCenterLlaChanged_(view, newState.centerLla());
  }
  if (newState.eyeLla() != oldState.eyeLla())
  {
    changed = true;
    fireEyeLlaChanged_(view, newState.eyeLla());
  }
  if (newState.rangeAzEl() != oldState.rangeAzEl())
  {
    changed = true;
    fireRangeAzElChanged_(view, newState.rangeAzEl());
  }
  if (newState.offsetXyz() != oldState.offsetXyz())
  {
    changed = true;
    fireOffsetXyzChanged_(view, newState.offsetXyz());
  }
  if (newState.tetherMode() != oldState.tetherMode())
  {
    changed = true;
    fireTetherModeChanged_(view, newState.tetherMode());
  }
  if (newState.isHeadingLocked() != oldState.isHeadingLocked() || newState.isPitchLocked() != oldState.isPitchLocked())
  {
    changed = true;
    fireMouseAxisLockChanged_(view, newState.isHeadingLocked(), newState.isPitchLocked());
  }

  if (changed)
    fireChanged_(view);
}

void ViewpointMonitor::fireIsTetheredChanged_(simVis::View* view, bool isTethered)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->isTetheredChanged(view, isTethered);
}

void ViewpointMonitor::fireTetherChanged_(simVis::View* view, osg::Node* newTether)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->tetherChanged(view, newTether);
}

void ViewpointMonitor::fireIsWatchingChanged_(simVis::View* view, bool isWatching)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->isWatchingChanged(view, isWatching);
}

void ViewpointMonitor::fireWatchedChanged_(simVis::View* view, simVis::EntityNode* watchedNode)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->watchedChanged(view, watchedNode);
}

void ViewpointMonitor::fireIsOverheadChanged_(simVis::View* view, bool isOverhead)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->isOverheadChanged(view, isOverhead);
}

void ViewpointMonitor::fireCenterLlaChanged_(simVis::View* view, const simCore::Vec3& lla)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->centerLlaChanged(view, lla);
}

void ViewpointMonitor::fireEyeLlaChanged_(simVis::View* view, const simCore::Vec3& lla)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->eyeLlaChanged(view, lla);
}

void ViewpointMonitor::fireRangeAzElChanged_(simVis::View* view, const simCore::Vec3& rangeAzEl)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->rangeAzElChanged(view, rangeAzEl);
}

void ViewpointMonitor::fireOffsetXyzChanged_(simVis::View* view, const simCore::Vec3& xyz)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->offsetXyzChanged(view, xyz);
}

void ViewpointMonitor::fireTetherModeChanged_(simVis::View* view, osgEarth::Util::EarthManipulator::TetherMode tetherMode)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->tetherModeChanged(view, tetherMode);
}

void ViewpointMonitor::fireMouseAxisLockChanged_(simVis::View* view, bool isHeadingLocked, bool isPitchLocked)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->mouseAxisLockChanged(view, isHeadingLocked, isPitchLocked);
}

void ViewpointMonitor::fireChanged_(simVis::View* view)
{
  for (std::vector<ObserverPtr>::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
    (*i)->changed(view);
}

}
