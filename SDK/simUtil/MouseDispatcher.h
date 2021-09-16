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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMUTIL_MOUSEDISPATCHER_H
#define SIMUTIL_MOUSEDISPATCHER_H

#include <map>
#include <set>
#include <vector>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simVis/ViewManager.h"
#include "simUtil/MouseManipulator.h"

namespace simUtil {

/**
 * Responsible for delegating mouse functionality in serial amongst several registered
 * and prioritized mouse manipulators.  Works similar to a GUIEventHandler (and uses one
 * for its implementation), but differs by providing a prioritization feature for the
 * mouse manipulators (as opposed to the built in Chain of Responsibility in OSG).
 */
class SDKUTIL_EXPORT MouseDispatcher
{
public:
  MouseDispatcher();
  virtual ~MouseDispatcher();

  /** Weight associated with the exclusive (one and only one active at a time) mouse manipulator */
  static const int EXCLUSIVE_MOUSE_WEIGHT = 25;

  /** Changes the view manager and sets up the callbacks required for intercepting the mouse */
  void setViewManager(simVis::ViewManager* viewManager);

  /** Retrieves a pointer to the dispatcher event handler. */
  osgGA::GUIEventHandler* eventHandler() const;

  /** Lower weight number means the manipulator will be serviced before others with higher weight numbers. */
  void addManipulator(int weight, MouseManipulatorPtr manipulator);
  /**
   * Adds a mouse manipulator that changes the click ability.  Exclusive manipulators can become active and
   * inactive and only one is active at a time.  These manipulators are mutually exclusive; 0 or 1 is active.
   */
  void addExclusiveManipulator(MouseManipulatorPtr manipulator);
  /** Removes the manipulator from the list.  Note this should not be called from a MouseManipulator to avoid iterator invalidation. */
  void removeManipulator(MouseManipulatorPtr manipulator);

  /** Activates a single exclusive manipulator, deactivating all other mutually exclusive manipulators. */
  int activateExclusive(MouseManipulatorPtr manipulator);
  /** Deactivates an exclusive manipulator.  If it was active, then the active exclusive manipulator is set to nullptr. */
  int deactivateExclusive(MouseManipulatorPtr manipulator);
  /** Retrieves the currently active exclusive manipulator; might be nullptr */
  MouseManipulatorPtr activeExclusiveManipulator() const;

  /** Observable events on the Mouse Dispatcher */
  class Observer
  {
  public:
    virtual ~Observer() {}
    /** Manipulator has changed.  May be nullptr. */
    virtual void activeExclusiveManipulatorChanged(MouseManipulatorPtr active, MouseManipulatorPtr oldActive) = 0;
  };

  /** Add an observer */
  void addObserver(std::shared_ptr<Observer> observer);
  /** Remove an observer */
  void removeObserver(std::shared_ptr<Observer> observer);

private:
  /** Fires the observable event */
  void fireActiveExclusiveManipulatorChanged_(MouseManipulatorPtr active, MouseManipulatorPtr oldActive);

  /** Stores a reference to the view manager */
  osg::observer_ptr<simVis::ViewManager> viewManager_;

  /** Defines the storage for weight + manipulator */
  typedef std::multimap<int, MouseManipulatorPtr> PriorityMap;
  /** Instance of a priority map on the mouse manipulators */
  PriorityMap priorityMap_;

  /** Encapsulates the GUI Event Handler in OSG */
  class EventHandler : public osgGA::GUIEventHandler
  {
  public:
    /** Constructor */
    explicit EventHandler(const MouseDispatcher& dispatch);
    /** Handle events, return true if handled, false otherwise. */
    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv);

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simUtil"; }
    /** Return the class name */
    virtual const char* className() const { return "MouseDispatcher::EventHandler"; }

  protected:
    /** Derived from osg::Referenced */
    virtual ~EventHandler();

  private:
    /** Reference back to the owner */
    const MouseDispatcher& dispatch_;
  };

  /** Encapsulation of a osgGA::GUIEventHandler */
  osg::ref_ptr<EventHandler> eventHandler_;

  /** Instance of the observer of views added/deleted */
  osg::ref_ptr<simVis::AddEventHandlerToViews> viewObserver_;

  /** Stores all mutually exclusive manipulators */
  std::set<MouseManipulatorPtr> allExclusive_;
  /** Proxy that switches between the current mutually exclusive manipulator */
  std::shared_ptr<MouseManipulatorProxy> exclusiveProxy_;

  /** Observers */
  std::vector<std::shared_ptr<Observer> > observers_;
};

}

#endif /* SIMUTIL_MOUSEDISPATCHER_H */
