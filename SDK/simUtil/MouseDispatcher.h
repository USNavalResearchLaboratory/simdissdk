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
#ifndef SIMUTIL_MOUSEDISPATCHER_H
#define SIMUTIL_MOUSEDISPATCHER_H

#include <map>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simUtil/MouseManipulator.h"

namespace simVis { class ViewManager; }

namespace simUtil {

class AddEventHandlerToViews;

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

  /** Changes the view manager and sets up the callbacks required for intercepting the mouse */
  void setViewManager(simVis::ViewManager* viewManager);

  /** Lower weight number means the manipulator will be serviced before others with higher weight numbers. */
  void addManipulator(int weight, MouseManipulatorPtr manipulator);
  /** Removes the manipulator from the list.  Note this should not be called from a MouseManipulator to avoid iterator invalidation. */
  void removeManipulator(MouseManipulatorPtr manipulator);

private:
  /** Stores a reference to the view manager */
  osg::observer_ptr<simVis::ViewManager> viewManager_;

  /** Defines the storage for weight + manipulator */
  typedef std::multimap<int, MouseManipulatorPtr> PriorityMap;
  /** Instance of a priority map on the mouse manipulators */
  PriorityMap priorityMap_;

  /** Encapsulates the GUI Event Handler in OSG */
  class EventHandler;
  /** Encapsulation of a osgGA::GUIEventHandler */
  osg::ref_ptr<EventHandler> eventHandler_;

  /** Instance of the observer of views added/deleted */
  osg::ref_ptr<AddEventHandlerToViews> viewObserver_;
};

}

#endif /* SIMUTIL_MOUSEDISPATCHER_H */
