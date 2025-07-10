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
#include "simQt/AutoRepeatFilter.h"

class MultiTouchEventFilter;
namespace osg { class GraphicsContext; }
namespace osgViewer { class View; }

namespace simQt
{

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
  [[deprecated("This class is deprecated due to Qt deprecating GLWidget. See simQt::ViewerWidgetAdapter instead.")]]
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
