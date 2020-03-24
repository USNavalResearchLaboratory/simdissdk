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
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*
*/

#ifndef SIMQT_VIEW_WIDGET_H
#define SIMQT_VIEW_WIDGET_H

#include "osgQt/GraphicsWindowQt"
#include "simCore/Common/Common.h"

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

private:
  /** When widget is wrapping a view, initializes that view to use appropriate graphics context */
  void init_(osgViewer::View* view);
  /** Create a graphics context to associate with OSG Cameras/Views */
  osg::GraphicsContext* createGraphicsContext_();
};

}

#endif
