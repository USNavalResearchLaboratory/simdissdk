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
#include <algorithm>
#include <cassert>
#include <utility>
#include "osgGA/GUIEventHandler"
#include "simNotify/Notify.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simUtil/MouseDispatcher.h"

namespace simUtil {

/// Mask of the various osgGA mouse events
static const int MOUSE_EVENT_MASK = osgGA::GUIEventAdapter::PUSH | osgGA::GUIEventAdapter::RELEASE |
  osgGA::GUIEventAdapter::MOVE | osgGA::GUIEventAdapter::DRAG | osgGA::GUIEventAdapter::DOUBLECLICK |
  osgGA::GUIEventAdapter::SCROLL | osgGA::GUIEventAdapter::FRAME;

MouseDispatcher::EventHandler::EventHandler(const MouseDispatcher& dispatch)
  : GUIEventHandler(),
  dispatch_(dispatch)
{
}

bool MouseDispatcher::EventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv)
{
  // Divert touch events
  if (ea.isMultiTouchEvent())
  {
    for (MouseDispatcher::PriorityMap::const_iterator i = dispatch_.priorityMap_.begin();
      i != dispatch_.priorityMap_.end(); ++i)
    {
      if (!i->second)
        continue;
      // the rv gets set to non-zero if event is handled
      int rv = 0;
      switch (ea.getEventType())
      {
      case osgGA::GUIEventAdapter::PUSH:
        rv = i->second->touchBegan(ea, aa);
        break;
      case osgGA::GUIEventAdapter::DRAG:
        rv = i->second->touchMoved(ea, aa);
        break;
      case osgGA::GUIEventAdapter::RELEASE:
        rv = i->second->touchEnded(ea, aa);
        break;

      default:
        // OSG source code (EventQueue.cpp) does not allow for any other event type combination
        assert(false);
        break;
      }

      // rv will be non-zero if the event was intercepted
      if (rv != 0)
      {
        ea.setHandled(true);
        return true;
      }
    }

    // Fall back to default implementation (next in Chain of Responsibility)
    return GUIEventHandler::handle(ea, aa, object, nv);
  }

  if ((ea.getEventType() & MOUSE_EVENT_MASK) == 0)
    return false;

  // Iterate through each manipulator and give it a chance to steal
  for (MouseDispatcher::PriorityMap::const_iterator i = dispatch_.priorityMap_.begin();
    i != dispatch_.priorityMap_.end(); ++i)
  {
    if (!i->second)
      continue;
    // the rv gets set to non-zero if event is handled
    int rv = 0;
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::PUSH:
      rv = i->second->push(ea, aa);
      break;

    case osgGA::GUIEventAdapter::DRAG:
      rv = i->second->drag(ea, aa);
      break;

    case osgGA::GUIEventAdapter::MOVE:
      rv = i->second->move(ea, aa);
      break;

    case osgGA::GUIEventAdapter::RELEASE:
      rv = i->second->release(ea, aa);
      break;

    case osgGA::GUIEventAdapter::DOUBLECLICK:
      rv = i->second->doubleClick(ea, aa);
      break;

    case osgGA::GUIEventAdapter::SCROLL:
      rv = i->second->scroll(ea, aa);
      break;

    case osgGA::GUIEventAdapter::FRAME:
      rv = i->second->frame(ea, aa);
      break;

    default:
      // Don't need to pass on other events
      break;
    }

    // rv will be non-zero if the event was intercepted
    if (rv != 0)
    {
      ea.setHandled(true);
      return true;
    }
  }

  // Fall back to default implementation (next in Chain of Responsibility)
  return GUIEventHandler::handle(ea, aa, object, nv);
}

MouseDispatcher::EventHandler::~EventHandler()
{
}

///////////////////////////////////////////////////////////////////////////

MouseDispatcher::MouseDispatcher()
  : exclusiveProxy_(new MouseManipulatorProxy)
{
  eventHandler_ = new EventHandler(*this);

  addManipulator(EXCLUSIVE_MOUSE_WEIGHT, exclusiveProxy_);
}

MouseDispatcher::~MouseDispatcher()
{
  setViewManager(nullptr);
  viewObserver_ = nullptr;
  eventHandler_ = nullptr;
}

void MouseDispatcher::setViewManager(simVis::ViewManager* viewManager)
{
  // Don't do anything on no-ops
  if (viewManager_ == viewManager)
    return;

  // Create the view observer if we haven't yet
  if (!viewObserver_)
    viewObserver_ = new simVis::AddEventHandlerToViews(eventHandler_.get());

  // Remove all observers and GUI handlers
  if (viewManager_ != nullptr)
  {
    viewManager_->removeCallback(viewObserver_.get());
    viewObserver_->removeFromViews(*viewManager_);
  }
  viewManager_ = viewManager;

  // Add back in the observers and GUI handlers to the new view manager
  if (viewManager_ != nullptr)
  {
    viewManager_->addCallback(viewObserver_.get());
    viewObserver_->addToViews(*viewManager_);
  }
}

osgGA::GUIEventHandler* MouseDispatcher::eventHandler() const
{
  return eventHandler_.get();
}

void MouseDispatcher::addManipulator(int weight, MouseManipulatorPtr manipulator)
{
  // Don't add nullptrs
  if (manipulator == nullptr)
    return;
  priorityMap_.insert(std::make_pair(weight, manipulator));
}

void MouseDispatcher::addExclusiveManipulator(MouseManipulatorPtr manipulator)
{
  // Don't add nullptr and don't repeat
  if (manipulator == nullptr || allExclusive_.find(manipulator) != allExclusive_.end())
    return;
  allExclusive_.insert(manipulator);
}

void MouseDispatcher::removeManipulator(MouseManipulatorPtr manipulator)
{
  PriorityMap::iterator i = priorityMap_.begin();
  while (i != priorityMap_.end())
  {
    if (i->second == manipulator)
      priorityMap_.erase(i++);
    else
      ++i;
  }

  // Deactivate it if it's an exclusive manipulator
  if (manipulator == exclusiveProxy_->subject())
    deactivateExclusive(manipulator);
  // Remove it from the list of exclusive manipulators
  allExclusive_.erase(manipulator);
}

int MouseDispatcher::activateExclusive(MouseManipulatorPtr manipulator)
{
  auto oldSubject = exclusiveProxy_->subject();
  // Noop if no change
  if (oldSubject == manipulator)
    return 0; // not an error; it's still active

  // Return an error if this manipulator is not in our list of exclusive ones.
  if (manipulator != nullptr && allExclusive_.find(manipulator) == allExclusive_.end())
  {
    SIM_WARN << "MouseDispatcher::activateExclusive(): Please register exclusive mouse mode before calling this method.\n";
    return 1;
  }

  // Deactivate the old one
  if (oldSubject != nullptr)
    oldSubject->deactivate();
  exclusiveProxy_->setSubject(manipulator);
  if (manipulator)
    manipulator->activate();
  fireActiveExclusiveManipulatorChanged_(manipulator, oldSubject);
  return 0;
}

int MouseDispatcher::deactivateExclusive(MouseManipulatorPtr manipulator)
{
  // Avoid deactivate on nullptr (meaningless and dev error)
  if (manipulator == nullptr)
    return 1;

  // Return early if the manipulator is not active.  Perhaps someone
  // changed activeness and dev didn't notice the alert.
  if (exclusiveProxy_->subject() != manipulator)
    return 1;

  exclusiveProxy_->setSubject(MouseManipulatorPtr());
  manipulator->deactivate();
  fireActiveExclusiveManipulatorChanged_(MouseManipulatorPtr(), manipulator);
  return 0;
}

MouseManipulatorPtr MouseDispatcher::activeExclusiveManipulator() const
{
  return exclusiveProxy_->subject();
}

void MouseDispatcher::fireActiveExclusiveManipulatorChanged_(MouseManipulatorPtr active, MouseManipulatorPtr oldActive)
{
  // Create a copy of the vector to avoid a common issue of callback modifying the vector.
  auto tmpObservers = observers_;
  for (auto i = tmpObservers.begin(); i != tmpObservers.end(); ++i)
    (*i)->activeExclusiveManipulatorChanged(active, oldActive);
}

void MouseDispatcher::addObserver(std::shared_ptr<Observer> observer)
{
  observers_.push_back(observer);
}

void MouseDispatcher::removeObserver(std::shared_ptr<Observer> observer)
{
  observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
}

}
