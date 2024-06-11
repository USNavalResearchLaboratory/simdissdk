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
#ifndef SIMQT_VIEW_WIDGET_H
#define SIMQT_VIEW_WIDGET_H

#include "osgQt/GraphicsWindowQt"
#include "simCore/Common/Common.h"

class MultiTouchEventFilter;
namespace osg { class GraphicsContext; }
namespace osgViewer { class View; }

namespace simQt
{

/**
 * Filter class that blocks auto-repeat keypress events from reaching the filtered object.  This is
 * useful for blocking auto-repeat keys from the GLWidget / ViewWidget.  osgEarth::EarthManipulator
 * can have poor keyboard interaction if the frame rate ever drops under the key autorepeat rate,
 * and this filter helps fix that problem.
 *
 * The following code can be used to install the filter on a widget:
 * <code>
 * AutoRepeatFilter* filter = new AutoRepeatFilter(viewWidget);
 * viewWidget->installEventFilter(filter);
 * </code>
 *
 * Note that this filter is auto-installed on simQt::ViewWidget instances, but is not automatically
 * installed on osgQt::GLWidget.  So if your application uses osgQt::GLWidget, consider using the filter.
 */
class SDKQT_EXPORT AutoRepeatFilter : public QObject
{
  Q_OBJECT;

public:
  explicit AutoRepeatFilter(QObject* parent = nullptr);

  /** Enables or disables the filtering.  If true (default), auto-repeated keys are filtered out. */
  void setEnabled(bool enabled);
  /** True if enabled (i.e., auto-repeated keys are filtered out) */
  bool isEnabled() const;

protected:
  // From QObject:
  virtual bool eventFilter(QObject* obj, QEvent* evt) override;

private:
  bool enabled_;
};

/**
 * A wrapper class to encapsulate an osgViewer::View (such as the simVis::View Main View) in a Qt widget.
 *
 * Improves osgQt::GLWidget by initializing the camera properly and applying GL version requests
 * to the QGLFormat used for graphics initialization.  To specify graphics configuration options,
 * set the osg::DisplaySettings::instance() values you want before calling the constructor.
 *
 * Adapted with permission from deprecated osgEarth::QtGui::ViewerWidget and osgEarth::QtGui::ViewWidget.
 *
 * Note that this class automatically installs a simQt::AutoRepeatFilter, so your osgGA event handler
 * will never report repeated keys from holding down keys.
 */
class SDKQT_EXPORT ViewWidget : public osgQt::GLWidget
{
  Q_OBJECT;

public:
  /**
   * Constructs a new ViewWidget, attaching an existing view.  Uses the osg::DisplaySettings::instance()
   * to drive the format used for the OpenGL context.
   * @param[in] view View to attach to this widget.  The widget will install a new camera in the View
   *   if necessary.  This widget does not take ownership of View.
   */
  explicit ViewWidget(osgViewer::View* view);

  /** Destructor */
  virtual ~ViewWidget();

  /** Enables or disables auto-repeat keyboard events.  Set false to disable auto-repeat keys.  See simQt::AutoRepeatFilter. */
  void setAllowAutoRepeatKeys(bool allowAutoRepeat);
  /** Returns true if auto-repeat keyboard events are enabled. */
  bool allowAutoRepeatKeys() const;

private:
  /** When widget is wrapping a view, initializes that view to use appropriate graphics context */
  void init_(osgViewer::View* view);
  /** Create a graphics context to associate with OSG Cameras/Views */
  osg::GraphicsContext* createGraphicsContext_();

  AutoRepeatFilter* autoRepeatFilter_ = nullptr;
  MultiTouchEventFilter* multiTouchEventFilter_ = nullptr;
};

}

#endif
