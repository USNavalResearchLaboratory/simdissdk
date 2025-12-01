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
#include <QTouchEvent>
#include "osgViewer/GraphicsWindow"
#include "osgGA/EventQueue"
#include "simQt/MultiTouchEventFilter.h"

namespace simQt {

namespace {

/** Anonymous function to convert from Qt touch state to OSG touch state */
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
osgGA::GUIEventAdapter::TouchPhase toTouchPhase(Qt::TouchPointState state)
#else
osgGA::GUIEventAdapter::TouchPhase toTouchPhase(QEventPoint::State state)
#endif
{
  switch (state)
  {
  case Qt::TouchPointPressed:
    return osgGA::GUIEventAdapter::TOUCH_BEGAN;
  case Qt::TouchPointMoved:
    return osgGA::GUIEventAdapter::TOUCH_MOVED;
  case Qt::TouchPointStationary:
    return osgGA::GUIEventAdapter::TOUCH_STATIONERY;
  case Qt::TouchPointReleased:
    return osgGA::GUIEventAdapter::TOUCH_ENDED;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
  case Qt::TouchPointUnknownState:
    return osgGA::GUIEventAdapter::TOUCH_UNKNOWN;
#endif
  }

  // Unknown state, return a moved
  assert(0);
  return osgGA::GUIEventAdapter::TOUCH_MOVED;
}

}

MultiTouchEventFilter::MultiTouchEventFilter(QObject* parent)
  : QObject(parent)
{
}

MultiTouchEventFilter::~MultiTouchEventFilter()
{
}

bool MultiTouchEventFilter::eventFilter(QObject* obj, QEvent* evt)
{
  // Do not process empty events
  if (!evt)
    return false;

  // Drop mouse events if we're touching the screen, preventing mouse noise while touching
  if (currentlyTouching_ && allowEvents_ != AllowAll)
  {
    switch (evt->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
      evt->accept();
      return true;

    case QEvent::MouseButtonDblClick:
      // Double click events are potentially useful (e.g. recenter)
      if (allowEvents_ == AllowDoubleClickOnly)
        break;
      evt->accept();
      return true;

    default:
      break;
    }
  }

  // Process touch events by farming to helper methods; intercept if event accepted
  switch (evt->type())
  {
  case QEvent::TouchBegin:
    return touchBeginEvent_(static_cast<QTouchEvent*>(evt));

  case QEvent::TouchUpdate:
    return touchUpdateEvent_(static_cast<QTouchEvent*>(evt));

  case QEvent::TouchEnd:
    return touchEndEvent_(static_cast<QTouchEvent*>(evt));

  case QEvent::TouchCancel:
    return touchCancelEvent_(static_cast<QTouchEvent*>(evt));

  default:
    break;
  }

  // Non-touch events are not intercepted
  return false;
}

bool MultiTouchEventFilter::touchBeginEvent_(QTouchEvent* evt)
{
  // Must have a valid event queue, or we ignore the touch event because we
  // can't really do anything with it.
  auto* eventQueue = eventQueue_();
  if (!eventQueue)
    return false;

  // Must be touching at least one finger
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const auto& touchPoints = evt->touchPoints();
#else
  const auto& touchPoints = evt->points();
#endif
  if (touchPoints.empty())
    return false;

  // Keep track of whether we're currently touching. If we're touching and
  // a mouse event comes in, that mouse event might need to be dropped.
  currentlyTouching_ = true;

  // Accept the event, which should stop touch processing down the line. This is
  // particularly important for touch-begin events, because by default Qt will generate
  // a synthetic mouse event for any touch that is not accepted by the widget, and we
  // do not want that synthetic mouse event.
  evt->accept();

  // Queue a touchBegan() event
  const auto& firstPoint = touchPoints.constFirst();
  osgGA::GUIEventAdapter* osgEvent = eventQueue->touchBegan(firstPoint.id(), toTouchPhase(firstPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    firstPoint.pos().x(), firstPoint.pos().y());
#else
    firstPoint.position().x(), firstPoint.position().y());
#endif

  // If more than one finger is present, add each finger's touch data
  for (int k = 1; k < touchPoints.size(); ++k)
  {
    const auto& thisPoint = touchPoints[k];
    osgEvent->addTouchPoint(thisPoint.id(), toTouchPhase(thisPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      thisPoint.pos().x(), thisPoint.pos().y());
#else
      thisPoint.position().x(), thisPoint.position().y());
#endif
  }
  return true;
}

bool MultiTouchEventFilter::touchUpdateEvent_(QTouchEvent* evt)
{
  // Must have a valid event queue, or we ignore the touch event because we
  // can't really do anything with it.
  auto* eventQueue = eventQueue_();
  if (!eventQueue)
    return false;

  // If widget is missing WA_AcceptTouchEvents events, touch-begin is not sent out
  // but touch-update IS sent out. Simulate a touch-begin in this case.
  if (!currentlyTouching_)
    touchBeginEvent_(evt);

  // Must be touching at least one finger
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const auto& touchPoints = evt->touchPoints();
#else
  const auto& touchPoints = evt->points();
#endif
  if (touchPoints.empty())
    return false;

  evt->accept();
  const auto& firstPoint = touchPoints.constFirst();
  osgGA::GUIEventAdapter* osgEvent = eventQueue->touchMoved(firstPoint.id(), toTouchPhase(firstPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    firstPoint.pos().x(), firstPoint.pos().y());
#else
    firstPoint.position().x(), firstPoint.position().y());
#endif

  // Add the rest of the fingers
  for (int k = 1; k < touchPoints.size(); ++k)
  {
    const auto& thisPoint = touchPoints[k];
    osgEvent->addTouchPoint(thisPoint.id(), toTouchPhase(thisPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      thisPoint.pos().x(), thisPoint.pos().y());
#else
      thisPoint.position().x(), thisPoint.position().y());
#endif
  }
  return true;
}

bool MultiTouchEventFilter::touchEndEvent_(QTouchEvent* evt)
{
  // On an end event, we definitely aren't touching, so make sure to at least
  // update our internal state regardless of the rest of the impl.
  currentlyTouching_ = false;

  // Must have a valid event queue, or we ignore the touch event because we
  // can't really do anything with it.
  auto* eventQueue = eventQueue_();
  if (!eventQueue)
    return false;

  // Must be touching at least one finger, even for end events
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  const auto& touchPoints = evt->touchPoints();
#else
  const auto& touchPoints = evt->points();
#endif
  if (touchPoints.empty())
    return false;

  evt->accept();
  const auto& firstPoint = touchPoints.constFirst();
  // No double tap support at this time, so use 1 for push times. Double tap can
  // work using the MouseDoubleClick event generated by the system.
  osgGA::GUIEventAdapter* osgEvent = eventQueue->touchEnded(firstPoint.id(), toTouchPhase(firstPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    firstPoint.pos().x(), firstPoint.pos().y(), 1u);
#else
    firstPoint.position().x(), firstPoint.position().y(), 1u);
#endif

  // Add the rest of the fingers
  for (int k = 1; k < touchPoints.size(); ++k)
  {
    const auto& thisPoint = touchPoints[k];
    osgEvent->addTouchPoint(thisPoint.id(), toTouchPhase(thisPoint.state()),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      thisPoint.pos().x(), thisPoint.pos().y());
#else
      thisPoint.position().x(), thisPoint.position().y());
#endif
  }
  return true;
}

bool MultiTouchEventFilter::touchCancelEvent_(QTouchEvent* evt)
{
  // On an end event, we definitely aren't touching, so make sure to at least
  // update our internal state regardless of the rest of the impl.
  currentlyTouching_ = false;

  // Cancel events don't have a representation in OSG, and I couldn't trigger it
  // in practice in Qt. Accept the event only if the event queue is configured.
  if (eventQueue_() == nullptr)
    return false;
  evt->accept();
  return true;
}

void MultiTouchEventFilter::setAllowedMouseEvents(AllowedMouseEvents allowEvents)
{
  allowEvents_ = allowEvents;
}

MultiTouchEventFilter::AllowedMouseEvents MultiTouchEventFilter::allowedMouseEvents() const
{
  return allowEvents_;
}

osgGA::EventQueue* MultiTouchEventFilter::eventQueue_() const
{
  if (graphicsWindow_.valid())
    return graphicsWindow_->getEventQueue();
  return nullptr;
}

void MultiTouchEventFilter::setGraphicsWindow(osgViewer::GraphicsWindow* window)
{
  graphicsWindow_ = window;

  // Turn off the "first touch emulates mouse" capability so that we have more
  // control over the touch functionality. Without this, OSG will emulate a
  // left button press, which is interpreted by osgEarth as a left click.
  osgGA::EventQueue* eventQueue = eventQueue_();
  if (eventQueue)
    eventQueue->setFirstTouchEmulatesMouse(false);
}

}
