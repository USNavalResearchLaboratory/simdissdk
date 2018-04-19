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
#include <QGLFormat>
#include "osg/Camera"
#include "osg/DisplaySettings"
#include "osg/GraphicsContext"
#include "osg/Viewport"
#include "osgViewer/View"
#include "simQt/ViewWidget.h"

namespace simQt
{

ViewWidget::ViewWidget(osgViewer::View* view)
  : osgQt::GLWidget()
{
  init_(view);

  // Force a minimum size to prevent divide-by-zero issues w/ matrices in OSG
  setMinimumSize(QSize(2, 2));
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::init_(osgViewer::View* view)
{
  if (!view)
    return;

  // Create the graphics context
  osg::GraphicsContext* gc = createGraphicsContext_();

  // Create new camera if needed
  osg::Camera* camera = view->getCamera();
  if (!camera)
  {
    camera = new osg::Camera();
    view->setCamera(camera);
  }

  // Apply the graphics context, then fix up the matrices and buffer assignments
  camera->setGraphicsContext(gc);
  camera->setViewport(new osg::Viewport(0, 0, gc->getTraits()->width, gc->getTraits()->height));
  if (gc->getTraits()->height != 0)
    camera->setProjectionMatrixAsPerspective(30.0f, gc->getTraits()->width / gc->getTraits()->height, 1.0f, 10000.0f);
  camera->setDrawBuffer(gc->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT);
  camera->setReadBuffer(gc->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT);
}

osg::GraphicsContext* ViewWidget::createGraphicsContext_()
{
  // Create traits initialized from the default display settings
  osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);

  // Read the display parameter and fix the display number if needed
  traits->readDISPLAY();
  if (traits->displayNum < 0)
    traits->displayNum = 0;

  // Fill out some reasonable values that the Traits constructor misses
  traits->windowDecoration = false;
  traits->x = 0;
  traits->y = 0;
  traits->width = 100;
  traits->height = 100;
  traits->doubleBuffer = true;
  // Window Data points to the osgQt::GLWidget so GraphicsWindowQt can communicate
  traits->inheritedWindowData = new osgQt::GraphicsWindowQt::WindowData(this);

  // Figure out the QGLFormat that will drive the actual display properties
  QGLFormat fmt = osgQt::GraphicsWindowQt::traits2qglFormat(traits.get());

  // Fix QGLFormat for things missing from osgQt: GL Context profile mask and version
  fmt.setProfile(static_cast<QGLFormat::OpenGLContextProfile>(traits->glContextProfileMask));
  unsigned int major = 0;
  unsigned int minor = 0;
  if (traits->getContextVersion(major, minor))
    fmt.setVersion(static_cast<int>(major), static_cast<int>(minor));

  // Apply the new format to the GL Widget
  setFormat(fmt);

  // Creates the graphics window Qt, telling it which traits were used to create it
  return new osgQt::GraphicsWindowQt(traits.get());
}

}
