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
#include <functional>
#include <QCoreApplication>
#include <QVBoxLayout>
#include "osg/DisplaySettings"
#include "osgViewer/Viewer"
#include "osgQOpenGL/osgQOpenGLWindow"
#include "simVis/Gl3Utils.h"
#include "simVis/Utils.h"
#include "simQt/AutoRepeatFilter.h"
#include "simQt/Gl3FormatGuesser.h"
#include "simQt/MultiTouchEventFilter.h"
#include "simQt/QtUtils.h"
#include "simQt/ViewerWidgetAdapter.h"

namespace simQt {

/**
 * Custom instance of an osgQOpenGLWindow, needed to apply signals to the
 * resizeGL and paintGL functionality.
 */
class SignalingGlWindow : public osgQOpenGLWindow
{
public:
  explicit SignalingGlWindow(QWidget* parent = nullptr)
    : osgQOpenGLWindow(parent)
  {
  }
  virtual ~SignalingGlWindow()
  {
  }

  void setResizeSignal(const std::function<void(int, int)>& resize)
  {
    notifyResize_ = resize;
  }
  void setPrePaintSignal(const std::function<void()>& prePaint)
  {
    notifyPrePaint_ = prePaint;
  }
  void setPostPaintSignal(const std::function<void()>& postPaint)
  {
    notifyPostPaint_ = postPaint;
  }

protected:
  // From osgQOpenGLWindow:
  virtual void resizeGL(int w, int h) override
  {
    osgQOpenGLWindow::resizeGL(w, h);
    if (notifyResize_) [[likely]]
      notifyResize_(w, h);
  }
  virtual void paintGL() override
  {
    if (notifyPrePaint_) [[likely]]
      notifyPrePaint_();
    osgQOpenGLWindow::paintGL();
    if (notifyPostPaint_) [[likely]]
      notifyPostPaint_();
  }

private:
  osg::ref_ptr<osgViewer::Viewer> viewer_;
  std::function<void(int, int)> notifyResize_;
  std::function<void()> notifyPrePaint_;
  std::function<void()> notifyPostPaint_;
};

///////////////////////////////////////////////////////////////////////

ViewerWidgetAdapter::ViewerWidgetAdapter(QWidget* parent)
  : QWidget(parent)
{
  glWindow_ = new SignalingGlWindow(this);
  glWindow_->setResizeSignal([this](int w, int h) { Q_EMIT glResized(w, h); });
  glWindow_->setPrePaintSignal([this]() { Q_EMIT aboutToPaintGl(); });
  glWindow_->setPostPaintSignal([this]() { Q_EMIT glPainted(); });
  connect(glWindow_, &QOpenGLWindow::frameSwapped, this, &ViewerWidgetAdapter::frameSwapped);
  connect(glWindow_, &osgQOpenGLWindow::initialized, this, &ViewerWidgetAdapter::postGlInitialize_);

  initializeSurfaceFormat_();

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(glWindow_->asWidget());
  setLayout(layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);

  // Intercept drag and drop events, converted to signals
  const auto& evtFilter = [this](QEvent* evt) {
    Q_EMIT dragDropEventIntercepted(evt);
    return true;
    };
  glWindow_->installEventFilter(new simQt::DragDropEventFilter(evtFilter, glWindow_));

  // Fix auto-repeat
  glWindow_->installEventFilter(new simQt::AutoRepeatFilter(this));

  // Intercept multi-touch events and queue them into OSG
  auto* multiTouchEventFilter = new simQt::MultiTouchEventFilter(this);
  glWindow_->installEventFilter(multiTouchEventFilter);

  // Set up the graphics window on initialization
  if (glWindow_->getGraphicsWindow())
    multiTouchEventFilter->setGraphicsWindow(getGraphicsWindow());
  else
  {
    connect(glWindow_, &osgQOpenGLWindow::initialized, this, [multiTouchEventFilter, this]() {
      multiTouchEventFilter->setGraphicsWindow(getGraphicsWindow());
      });
  }
}

ViewerWidgetAdapter::~ViewerWidgetAdapter()
{
}

osgViewer::ViewerBase* ViewerWidgetAdapter::getViewer() const
{
  return glWindow_->getOsgViewer();
}

void ViewerWidgetAdapter::setViewer(osgViewer::ViewerBase* viewer)
{
  glWindow_->setOsgViewer(viewer);
}

void ViewerWidgetAdapter::setTimerInterval(int intervalMs)
{
  glWindow_->setTimerInterval(intervalMs);
}

osg::GraphicsContext* ViewerWidgetAdapter::getGraphicsContext() const
{
  return glWindow_->getGraphicsContext();
}

osgViewer::GraphicsWindow* ViewerWidgetAdapter::getGraphicsWindow() const
{
  return glWindow_->getGraphicsWindow();
}

osgQOpenGLWindow* ViewerWidgetAdapter::glWindow() const
{
  return glWindow_;
}

void ViewerWidgetAdapter::setFormat(const QSurfaceFormat& format)
{
  glWindow_->setFormat(format);
}

QOpenGLContext* ViewerWidgetAdapter::qtGraphicsContext() const
{
  return glWindow_->context();
}

void ViewerWidgetAdapter::makeCurrent()
{
  glWindow_->makeCurrent();
}

void ViewerWidgetAdapter::doneCurrent()
{
  glWindow_->doneCurrent();
}

bool ViewerWidgetAdapter::isValid() const
{
  return glWindow_->isValid();
}

void ViewerWidgetAdapter::create()
{
  glWindow_->create();
}

void ViewerWidgetAdapter::createAndProcessEvents()
{
  create();
  QCoreApplication::processEvents();
}

void ViewerWidgetAdapter::initializeSurfaceFormat_()
{
  // Configure the default GL profile properly based on OSG settings
  osg::ref_ptr<osg::DisplaySettings> ds = osg::DisplaySettings::instance();
  osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);

  // Read the display parameter and fix the display number if needed
  traits->readDISPLAY();

  // Buffer sizes and other fields, from traits
  QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
  surfaceFormat.setAlphaBufferSize(traits->alpha);
  surfaceFormat.setRedBufferSize(traits->red);
  surfaceFormat.setGreenBufferSize(traits->green);
  surfaceFormat.setBlueBufferSize(traits->blue);

  surfaceFormat.setDepthBufferSize(traits->depth);
  surfaceFormat.setStencilBufferSize(traits->stencil);
  surfaceFormat.setSamples(traits->sampleBuffers ? traits->samples : 0);
  surfaceFormat.setStereo(traits->quadBufferStereo);

  surfaceFormat.setSwapBehavior(traits->doubleBuffer ? QSurfaceFormat::DoubleBuffer : QSurfaceFormat::SingleBuffer);
  surfaceFormat.setSwapInterval(traits->vsync ? 1 : 0);
  surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);

  // Apply profile and GL version
  surfaceFormat.setProfile(static_cast<QSurfaceFormat::OpenGLContextProfile>(traits->glContextProfileMask));
  unsigned int major = 0;
  unsigned int minor = 0;
  if (traits->getContextVersion(major, minor))
    surfaceFormat.setVersion(static_cast<int>(major), static_cast<int>(minor));
  surfaceFormat = simQt::Gl3FormatGuesser::getSurfaceFormat(surfaceFormat);

  // Set the default format for this adapter
  setFormat(surfaceFormat);
}

void ViewerWidgetAdapter::postGlInitialize_()
{
  auto* gc = getGraphicsContext();
  simVis::applyCoreProfileValidity(gc);
  simVis::applyMesaGeometryShaderFix(gc);

  Q_EMIT initialized();
}

}
