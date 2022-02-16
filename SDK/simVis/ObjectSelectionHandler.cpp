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
#include "simVis/ObjectSelectionHandler.h"
#include "osgViewer/View"
#include "osgUtil/LineSegmentIntersector"

namespace simVis {
//----------------------------------------------------------------------------
ObjectSelectionHandler::ObjectSelectionHandler()
  : traversalMask_(~0),
    buttonMask_(osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON),
    modKeyMask_(0),
    action_(ACTION_CLICK),
    hoverDelay_s_(0.1),
    mouseState_(MOUSE_IDLE)
{
}

void ObjectSelectionHandler::setSelectCallback(SelectCallback *callback)
{
  acceptor_ = callback;
}

ObjectSelectionHandler::SelectCallback* ObjectSelectionHandler::getSelectCallback() const
{
  return acceptor_.get();
}

void ObjectSelectionHandler::setRoot(osg::Node *node)
{
  root_ = node;
}

osg::Node* ObjectSelectionHandler::getRoot() const
{
  return root_.get();
}

void ObjectSelectionHandler::setObjectTraversalMask(const osg::Node::NodeMask &mask)
{
  traversalMask_ = mask;
}

const osg::Node::NodeMask& ObjectSelectionHandler::getObjectTraversalMask() const
{
  return traversalMask_;
}

void  ObjectSelectionHandler::setSelectionInputMasks(
  const osgGA::GUIEventAdapter::MouseButtonMask& buttons,
  const osgGA::GUIEventAdapter::ModKeyMask& modifierKeys)
{
  buttonMask_ = buttons;
  modKeyMask_ = modifierKeys;
}

void ObjectSelectionHandler::setSelectAction(const ObjectSelectionHandler::SelectAction &action)
{
  action_ = action;
}

const ObjectSelectionHandler::SelectAction& ObjectSelectionHandler::getSelectAction() const
{
  return action_;
}

void ObjectSelectionHandler::setHoverDelaySeconds(double seconds)
{
  hoverDelay_s_ = seconds;
}

double ObjectSelectionHandler::getHoverDelaySeconds() const
{
  return hoverDelay_s_;
}

bool ObjectSelectionHandler::handle(const osgGA::GUIEventAdapter &ev, osgGA::GUIActionAdapter &aa)
{
  bool handled = false;

  if (action_ == ACTION_CLICK)
  {
    switch (ev.getEventType())
    {
    case osgGA::GUIEventAdapter::PUSH:
      mouseDownEvent_ = &ev;
      break;

    case osgGA::GUIEventAdapter::RELEASE:
      if (mouseDownEvent_.valid() &&
          mouseDownEvent_->getButtonMask() == buttonMask_ &&
          mouseDownEvent_->getModKeyMask() == modKeyMask_ &&
        isMouseClick_(ev))
      {
        handled = select_(ev.getX(), ev.getY(), aa.asView());
      }
      break;

    default:
      break;
    }
  }
  else if (action_ == ACTION_HOVER)
  {
    switch (ev.getEventType())
    {
    case osgGA::GUIEventAdapter::MOVE:
      mouseState_ = MOUSE_MOVING;
      mouseDownEvent_ = &ev;
      break;

    case osgGA::GUIEventAdapter::FRAME:
      if (mouseState_ == MOUSE_MOVING &&
        ev.getTime() - mouseDownEvent_->getTime() >= hoverDelay_s_)
      {
        mouseState_ = MOUSE_IDLE;
        handled = select_(mouseDownEvent_->getX(), mouseDownEvent_->getY(), aa.asView());
      }
      break;

    default:
      break;
    }
  }

  return handled;
}

bool ObjectSelectionHandler::isMouseClick_(const osgGA::GUIEventAdapter &upEv)
{
  return mouseDownEvent_.valid() &&
    ::fabs(upEv.getX() - mouseDownEvent_->getX()) <= 3.0f &&
    ::fabs(upEv.getY() - mouseDownEvent_->getY()) <= 3.0f;
}

static bool nodeListContains(const osg::NodeList &list, const osg::Node *node)
{
  for (osg::NodeList::const_iterator i = list.begin(); i != list.end(); ++i)
  {
    if (i->get() == node)
      return true;
  }

  return false;
}

bool ObjectSelectionHandler::select_(float mx, float my, osg::View *aaView)
{
  if (!acceptor_.valid())
    return false;

  osgViewer::View* view = dynamic_cast<osgViewer::View*>(aaView);
  if (!view)
    return false;

  bool ok;
  osgUtil::LineSegmentIntersector::Intersections r;

  if (root_.valid())
  {
    osg::NodePath path = root_->getParentalNodePaths()[0];
    ok = view->computeIntersections(mx, my, path, r, traversalMask_);
  }
  else
  {
    ok = view->computeIntersections(mx, my, r, traversalMask_);
  }

  if (!ok)
    return false;

  osg::NodeList results;

  for (osgUtil::LineSegmentIntersector::Intersections::const_iterator i = r.begin(); i != r.end(); ++i)
  {
    const osg::NodePath& p = i->nodePath;
    for (osg::NodePath::const_reverse_iterator n = p.rbegin(); n != p.rend(); ++n)
    {
      if (*n == root_.get())
        break;

      if (!nodeListContains(results, *n))
      {
        if (acceptor_->isSelectable(*(*n)))
        {
          results.push_back(*n);
          break;
        }
      }
    }
  }

  if (results.size() > 0)
  {
    acceptor_->select(results);
  }

  return false;
}

}
