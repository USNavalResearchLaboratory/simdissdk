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

#include <QGLFormat>
#include "osgQt/GraphicsWindowQt"
#include "osgViewer/ViewerBase"
#include "osgEarth/Map"
#include "simCore/Common/Common.h"

namespace simQt
{

/**
 * A wrapper class to encapsulate an osgViewer::View or an osgViewer::Viewer in a Qt widget.
 * Adapted with permission from deprecated osgEarth::QtGui::ViewerWidget and osgEarth::QtGui::ViewWidget.
 */
class SDKQT_EXPORT ViewWidget : public osgQt::GLWidget
{
  Q_OBJECT;

public:
  /**
   * Constructs a new ViewWidget, attaching an existing viewer.
   * @param[in] viewer Viewer to attach to this widget.  The widget will install
   *     a new camera in the viewer.  (NOTE: this widget does not take
   *     ownership of the Viewer.  You are still responsible for its destruction)
   */
  explicit ViewWidget(osgViewer::ViewerBase* viewer);

  /**
   * Constructs a new ViewWidget, attaching an existing view.
   * @param[in] view View to attach to this widget.  The widget will install a new camera
   *     in the View.  (NOTE: this widget does not take ownership of the View.
   *     You are still responsible for its destruction)
   */
  explicit ViewWidget(osgViewer::View* view);

  /**
  * Constructs a new ViewWidget, attaching an existing view, with specific GL Format.
  * @param[in] view View to attach to this widget.  The widget will install a new camera
  *     in the View.  (NOTE: this widget does not take ownership of the View.
  *     You are still responsible for its destruction)
  * @param[in] format GL format for the graphics context.  Use this parameter to specify
  *     multisampling, for example.
  */
  ViewWidget(osgViewer::View* view, const QGLFormat& format);

  /** Destructor */
  virtual ~ViewWidget();

protected:
  /** Override from QWidget.  If widget has a valid viewer that needs to do next frame, does next frame. */
  virtual void paintEvent(QPaintEvent* e);

private:
  /** If wrapping a viewer, populates the incoming collection with the views comprising wrapped viewer */
  void getViews_(osgViewer::ViewerBase::Views& views) const;
  /** Creates a simple basic viewer */
  void createViewer_();
  /** Reconfigure the given view to use this widget's graphics context.  Creates a new graphics context if none exists */
  void reconfigure_(osgViewer::View* view);

  /** When widget is wrapping a view, initializes that view to use appropriate graphics context */
  void init_(osg::GraphicsContext* gc);
  /** Get or create graphics context */
  osg::GraphicsContext* createOrShareGC_(osg::GraphicsContext* gc);

  osg::observer_ptr<osgViewer::ViewerBase> viewer_;
  osg::ref_ptr<osg::GraphicsContext> gc_;

  osg::observer_ptr<osgViewer::View> view_;
};

}

#endif
