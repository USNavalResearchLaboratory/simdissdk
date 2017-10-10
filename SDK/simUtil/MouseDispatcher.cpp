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
#include <utility>
#include "osgGA/GUIEventHandler"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simUtil/MouseDispatcher.h"

namespace simUtil {

/// Mask of the various osgGA mouse events
static const int MOUSE_EVENT_MASK = osgGA::GUIEventAdapter::PUSH | osgGA::GUIEventAdapter::RELEASE |
  osgGA::GUIEventAdapter::MOVE | osgGA::GUIEventAdapter::DRAG | osgGA::GUIEventAdapter::DOUBLECLICK |
  osgGA::GUIEventAdapter::SCROLL | osgGA::GUIEventAdapter::FRAME;

/// Encapsulates the GUI Event Handler operation as it adapts it to the MouseManipulator interface
class MouseDispatcher::EventHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit EventHandler(const MouseDispatcher& dispatch)
    : GUIEventHandler(),
      dispatch_(dispatch)
  {
  }

  /** Handle events, return true if handled, false otherwise. */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv)
  {
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

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "MouseDispatcher::EventHandler"; }

protected:
  /** Derived from osg::Referenced */
  virtual ~EventHandler()
  {
  }

private:
  /** Reference back to the owner */
  const MouseDispatcher& dispatch_;
};

///////////////////////////////////////////////////////////////////////////

MouseDispatcher::MouseDispatcher()
{
  eventHandler_ = new EventHandler(*this);
  viewObserver_ = new simVis::AddEventHandlerToViews(eventHandler_);
}

MouseDispatcher::~MouseDispatcher()
{
  setViewManager(NULL);
  viewObserver_ = NULL;
  eventHandler_ = NULL;
}

void MouseDispatcher::setViewManager(simVis::ViewManager* viewManager)
{
  // Don't do anything on no-ops
  if (viewManager_ == viewManager)
    return;

  // Remove all observers and GUI handlers
  if (viewManager_ != NULL)
  {
    viewManager_->removeCallback(viewObserver_);
    viewObserver_->removeFromViews(*viewManager_);
  }
  viewManager_ = viewManager;

  // Add back in the observers and GUI handlers to the new view manager
  if (viewManager_ != NULL)
  {
    viewManager_->addCallback(viewObserver_);
    viewObserver_->addToViews(*viewManager_);
  }
}

void MouseDispatcher::addManipulator(int weight, MouseManipulatorPtr manipulator)
{
  // Don't add NULLs
  if (manipulator == NULL)
    return;
  priorityMap_.insert(std::make_pair(weight, manipulator));
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
}

}
