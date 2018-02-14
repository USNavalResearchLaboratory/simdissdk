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

#include <QWidget>

#include "osgEarthUtil/EarthManipulator"
#include "osgGA/StateSetManipulator"
#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"

#include "simQt/ViewWidget.h"

namespace simQt
{

ViewWidget::ViewWidget(osgViewer::ViewerBase* viewer)
  : viewer_(viewer)
{
  if (!viewer_.valid())
  {
    createViewer_();
  }
  else
  {
    // reconfigure all the viewer's views to use a Qt graphics context.
    osgViewer::ViewerBase::Views views;
    getViews_(views);
    for (osgViewer::ViewerBase::Views::iterator v = views.begin(); v != views.end(); ++v)
    {
      reconfigure_(*v);
    }

    // disable event setting on the viewer.
    viewer->setKeyEventSetsDone(0);
    viewer->setQuitEventSetsDone(false);
  }
}

ViewWidget::ViewWidget(osgViewer::View* view)
  : osgQt::GLWidget(),
    view_(view)
{
  init_(NULL);
}

ViewWidget::ViewWidget(osgViewer::View* view, const QGLFormat& format)
  : osgQt::GLWidget(format),
    view_(view)
{
  init_(NULL);
}

ViewWidget::~ViewWidget()
{
  if (viewer_.valid())
  {
    viewer_->stopThreading();
    viewer_ = NULL;
  }
}

void ViewWidget::paintEvent(QPaintEvent* e)
{
  if (viewer_.valid() && (viewer_->getRunFrameScheme() == osgViewer::ViewerBase::CONTINUOUS ||
                           viewer_->checkNeedToDoFrame()))
    viewer_->frame();
}

void ViewWidget::getViews_(osgViewer::ViewerBase::Views& views) const
{
  if (!viewer_.valid())
    return;
  osgViewer::ViewerBase::Views temp;
  viewer_->getViews(temp);
  views.insert(views.end(), temp.begin(), temp.end());
}

void ViewWidget::createViewer_()
{
  // creates a simple basic viewer.
  osgViewer::Viewer* viewer = new osgViewer::Viewer();

  viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
  viewer->setCameraManipulator(new osgEarth::Util::EarthManipulator());

  viewer->addEventHandler(new osgViewer::StatsHandler());
  viewer->addEventHandler(new osgGA::StateSetManipulator());
  viewer->addEventHandler(new osgViewer::ThreadingHandler());

  viewer->setKeyEventSetsDone(0);
  viewer->setQuitEventSetsDone(false);

  reconfigure_(viewer);

  viewer_ = viewer;
}

void ViewWidget::reconfigure_(osgViewer::View* view)
{
  if (!gc_.valid())
  {
    // create the Qt graphics context if necessary; it will be shared across all views.
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);

    traits->readDISPLAY();
    if (traits->displayNum < 0)
      traits->displayNum = 0;

    traits->windowName = "ViewWidget";
    traits->windowDecoration = false;
    traits->x = x();
    traits->y = y();
    traits->width = width();
    traits->height = height();
    traits->doubleBuffer = true;
    traits->inheritedWindowData = new osgQt::GraphicsWindowQt::WindowData(this);

    gc_ = new osgQt::GraphicsWindowQt(traits.get());
  }

  // reconfigure this view's camera to use the Qt GC if necessary.
  osg::Camera* camera = view->getCamera();
  if (camera->getGraphicsContext() != gc_.get())
  {
    camera->setGraphicsContext(gc_.get());
    if (!camera->getViewport())
      camera->setViewport(new osg::Viewport(0,0, gc_->getTraits()->width, gc_->getTraits()->height));

    if (camera->getViewport()->height() != 0)
      camera->setProjectionMatrixAsPerspective(30.0f, camera->getViewport()->width() / camera->getViewport()->height(), 1.0f, 10000.0f);
  }
}

void ViewWidget::init_(osg::GraphicsContext* gc)
{
  if (!view_.valid())
    return;

  gc = createOrShareGC_(gc);

  osg::Camera* camera = view_->getCamera();
  if (!camera)
  {
    camera = new osg::Camera();
    view_->setCamera(camera);
  }
  camera->setGraphicsContext(gc);
  camera->setViewport(new osg::Viewport(0, 0, gc->getTraits()->width, gc->getTraits()->height));
  if (gc->getTraits()->height != 0)
    camera->setProjectionMatrixAsPerspective(30.0f, gc->getTraits()->width / gc->getTraits()->height, 1.0f, 10000.0f);
}

osg::GraphicsContext* ViewWidget::createOrShareGC_(osg::GraphicsContext* gc)
{
  if (!gc)
    gc->createNewContextID();

  osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);

  traits->readDISPLAY();
  if (traits->displayNum<0)
    traits->displayNum = 0;

  traits->windowDecoration = false;
  traits->x = 0;
  traits->y = 0;
  traits->width = 100;
  traits->height = 100;
  traits->doubleBuffer = true;
  traits->alpha = ds->getMinimumNumAlphaBits();
  traits->stencil = ds->getMinimumNumStencilBits();
  traits->sampleBuffers = ds->getMultiSamples();
  traits->samples = ds->getNumMultiSamples();
  traits->sharedContext = gc;
  traits->inheritedWindowData = new osgQt::GraphicsWindowQt::WindowData(this);

  return new osgQt::GraphicsWindowQt(traits.get());
}

}
