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
#ifndef SIMQT_MULTITOUCHEVENTFILTER_H
#define SIMQT_MULTITOUCHEVENTFILTER_H

#include <QObject>
#include "osg/observer_ptr"
#include "simCore/Common/Export.h"

class QTouchEvent;
namespace osgGA { class EventQueue; }
namespace osgViewer { class GraphicsWindow; }

namespace simQt
{

/**
 * Responsible for passing multi-touch events received from Qt, into the event queue
 * associated with a given graphics window. This is designed to be used with the
 * simQt::ViewWidget and its embedded GraphicsWindow.
 *
 * This filter will correctly forward raw touch messages into the OSG event queue.
 * It includes the ability to filter out sporadic mouse events on the window while
 * the touch capability is active, which can cause conflicting input data, e.g.
 * in osgEarth EarthManipulator processing.
 *
 * This filter also attempts to compensate for Qt widgets and touch. The Qt widget
 * should register for touch events with setAttribute(Qt::WA_AcceptTouchEvents),
 * and if it does not, the filter will get touch update and end events, but not
 * touch begin events. This filter corrects that behavior.
 */
class SDKQT_EXPORT MultiTouchEventFilter : public QObject
{
  Q_OBJECT;

public:
  explicit MultiTouchEventFilter(QObject* parent = nullptr);
  virtual ~MultiTouchEventFilter();

  /** Types of mouse events that are allowed through the filter when touch is active. */
  enum AllowedMouseEvents
  {
    /** Intercept no events; all events are allowed in; mouse can impact touch events. */
    AllowAll,
    /** Intercept mouse events, except double click which is allowed through. */
    AllowDoubleClickOnly,
    /** Intercept all mouse events, including double click. */
    AllowNone
  };

  /** Turns on a feature where mouse events get filtered out when touching the screen */
  void setAllowedMouseEvents(AllowedMouseEvents ignoreMouse);
  /** Returns true if the mouse is ignored when touching the screen */
  AllowedMouseEvents allowedMouseEvents() const;

  /** Sets the graphics window, required for getting event queue */
  void setGraphicsWindow(osgViewer::GraphicsWindow* window);

protected:
  // From QObject:
  virtual bool eventFilter(QObject* obj, QEvent* evt) override;

  /** Touch operation begins. If return value is true, then event is filtered/blocked. */
  virtual bool touchBeginEvent_(QTouchEvent* evt);
  /** Touch coordinates changed, after a begin. If return value is true, then event is filtered/blocked. */
  virtual bool touchUpdateEvent_(QTouchEvent* evt);
  /** Touch ended. If return value is true, then event is filtered/blocked. */
  virtual bool touchEndEvent_(QTouchEvent* evt);
  /** Touch has been canceled. If return value is true, then event is filtered/blocked. */
  virtual bool touchCancelEvent_(QTouchEvent* evt);

private:
  /** Retrieves the current event queue (might be null) */
  osgGA::EventQueue* eventQueue_() const;

  AllowedMouseEvents allowEvents_ = AllowDoubleClickOnly;
  bool currentlyTouching_ = false;
  osg::observer_ptr<osgViewer::GraphicsWindow> graphicsWindow_;
};


}

#endif /* SIMQT_MULTITOUCHEVENTFILTER_H */
