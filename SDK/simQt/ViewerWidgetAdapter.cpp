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
#include "osgQOpenGL/osgQOpenGLWidget"
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

/**
 * Custom instance of an osgQOpenGLWidget, needed to apply signals to the
 * resizeGL and paintGL functionality.
 */
class SignalingGlWidget : public osgQOpenGLWidget
{
public:
  explicit SignalingGlWidget(QWidget* parent = nullptr)
    : osgQOpenGLWidget(parent)
  {
  }
  virtual ~SignalingGlWidget()
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
  // From osgQOpenGLWidget:
  virtual void resizeGL(int w, int h) override
  {
    osgQOpenGLWidget::resizeGL(w, h);
    if (notifyResize_) [[likely]]
      notifyResize_(w, h);
  }
  virtual void paintGL() override
  {
    if (notifyPrePaint_) [[likely]]
      notifyPrePaint_();
    osgQOpenGLWidget::paintGL();
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

class GlPlatformInterface
{
public:
  virtual ~GlPlatformInterface() = default;

  virtual QWidget* widget() = 0;
  virtual QOpenGLWidget* glWidget() const = 0;
  virtual QOpenGLWindow* glWindow() const = 0;
  virtual QOpenGLContext* qtGraphicsContext() const = 0;
  virtual void setFormat(const QSurfaceFormat& format) = 0;
  virtual void makeCurrent() = 0;
  virtual void doneCurrent() = 0;
  virtual bool isValid() const = 0;
  virtual void create() = 0;

  virtual osg::GraphicsContext* getGraphicsContext() const = 0;
  virtual osgViewer::GraphicsWindow* getGraphicsWindow() const = 0;
  virtual osgViewer::ViewerBase* getOsgViewer() const = 0;
  virtual void setOsgViewer(osgViewer::ViewerBase* viewer) = 0;
  virtual void setTimerInterval(int intervalMs) = 0;
  virtual void installEventFilter(QObject* filter) = 0;

  virtual void setResizeSignal(const std::function<void(int, int)>& resize) = 0;
  virtual void setPrePaintSignal(const std::function<void()>& prePaint) = 0;
  virtual void setPostPaintSignal(const std::function<void()>& postPaint) = 0;
  virtual void connectToFrameSwappedSignal(const std::function<void()>& frameSwapped) = 0;
  virtual void connectToInitializedSignal(const std::function<void()>& initialized) = 0;
};

///////////////////////////////////////////////////////////////////////

class GlWindowPlatform : public GlPlatformInterface
{
public:
  explicit GlWindowPlatform(QWidget* parent = nullptr);
  virtual ~GlWindowPlatform();

  virtual QWidget* widget() override;
  virtual QOpenGLWidget* glWidget() const override;
  virtual QOpenGLWindow* glWindow() const override;
  virtual QOpenGLContext* qtGraphicsContext() const override;
  virtual void setFormat(const QSurfaceFormat& format) override;
  virtual void makeCurrent() override;
  virtual void doneCurrent() override;
  virtual bool isValid() const override;
  virtual void create() override;

  virtual osg::GraphicsContext* getGraphicsContext() const override;
  virtual osgViewer::GraphicsWindow* getGraphicsWindow() const override;
  virtual osgViewer::ViewerBase* getOsgViewer() const override;
  virtual void setOsgViewer(osgViewer::ViewerBase* viewer) override;
  virtual void setTimerInterval(int intervalMs) override;
  virtual void installEventFilter(QObject* filter) override;

  virtual void setResizeSignal(const std::function<void(int, int)>& resize) override;
  virtual void setPrePaintSignal(const std::function<void()>& prePaint) override;
  virtual void setPostPaintSignal(const std::function<void()>& postPaint) override;
  virtual void connectToFrameSwappedSignal(const std::function<void()>& frameSwapped) override;
  virtual void connectToInitializedSignal(const std::function<void()>& initialized) override;

private:
  SignalingGlWindow* glWindow_ = nullptr;
};

///////////////////////////////////////////////////////////////////////

class GlWidgetPlatform : public GlPlatformInterface
{
public:
  explicit GlWidgetPlatform(QWidget* parent = nullptr);
  virtual ~GlWidgetPlatform() override;

  virtual QWidget* widget() override;
  virtual QOpenGLWidget* glWidget() const override;
  virtual QOpenGLWindow* glWindow() const override;
  virtual QOpenGLContext* qtGraphicsContext() const override;
  virtual void setFormat(const QSurfaceFormat& format) override;
  virtual void makeCurrent() override;
  virtual void doneCurrent() override;
  virtual bool isValid() const override;
  virtual void create() override;

  virtual osg::GraphicsContext* getGraphicsContext() const override;
  virtual osgViewer::GraphicsWindow* getGraphicsWindow() const override;
  virtual osgViewer::ViewerBase* getOsgViewer() const override;
  virtual void setOsgViewer(osgViewer::ViewerBase* viewer) override;
  virtual void setTimerInterval(int intervalMs) override;
  virtual void installEventFilter(QObject* filter) override;

  virtual void setResizeSignal(const std::function<void(int, int)>& resize) override;
  virtual void setPrePaintSignal(const std::function<void()>& prePaint) override;
  virtual void setPostPaintSignal(const std::function<void()>& postPaint) override;
  virtual void connectToFrameSwappedSignal(const std::function<void()>& frameSwapped) override;
  virtual void connectToInitializedSignal(const std::function<void()>& initialized) override;

private:
  SignalingGlWidget* adaptedWidget_ = nullptr;
};

///////////////////////////////////////////////////////////////////////

GlWindowPlatform::GlWindowPlatform(QWidget* parent)
{
  glWindow_ = new SignalingGlWindow(parent);
}

GlWindowPlatform::~GlWindowPlatform()
{
  delete glWindow_;
}

QWidget* GlWindowPlatform::widget()
{
  return glWindow_->asWidget();
}

QOpenGLWidget* GlWindowPlatform::glWidget() const
{
  assert(0); // Dev configured as window, asking for widget
  return nullptr;
}

QOpenGLWindow* GlWindowPlatform::glWindow() const
{
  return glWindow_;
}

void GlWindowPlatform::setFormat(const QSurfaceFormat& format)
{
  glWindow_->setFormat(format);
}

QOpenGLContext* GlWindowPlatform::qtGraphicsContext() const
{
  return glWindow_->context();
}

void GlWindowPlatform::makeCurrent()
{
  glWindow_->makeCurrent();
}

void GlWindowPlatform::doneCurrent()
{
  glWindow_->doneCurrent();
}

bool GlWindowPlatform::isValid() const
{
  return glWindow_->isValid();
}

void GlWindowPlatform::create()
{
  glWindow_->create();
}

osg::GraphicsContext* GlWindowPlatform::getGraphicsContext() const
{
  return glWindow_->getGraphicsContext();
}

osgViewer::GraphicsWindow* GlWindowPlatform::getGraphicsWindow() const
{
  return glWindow_->getGraphicsWindow();
}

osgViewer::ViewerBase* GlWindowPlatform::getOsgViewer() const
{
  return glWindow_->getOsgViewer();
}

void GlWindowPlatform::setOsgViewer(osgViewer::ViewerBase* viewer)
{
  glWindow_->setOsgViewer(viewer);
}

void GlWindowPlatform::setTimerInterval(int intervalMs)
{
  glWindow_->setTimerInterval(intervalMs);
}

void GlWindowPlatform::installEventFilter(QObject* filter)
{
  glWindow_->installEventFilter(filter);
}

void GlWindowPlatform::setResizeSignal(const std::function<void(int, int)>& resize)
{
  glWindow_->setResizeSignal(resize);
}

void GlWindowPlatform::setPrePaintSignal(const std::function<void()>& prePaint)
{
  glWindow_->setPrePaintSignal(prePaint);
}

void GlWindowPlatform::setPostPaintSignal(const std::function<void()>& postPaint)
{
  glWindow_->setPostPaintSignal(postPaint);
}

void GlWindowPlatform::connectToFrameSwappedSignal(const std::function<void()>& frameSwapped)
{
  QObject::connect(glWindow_, &QOpenGLWindow::frameSwapped, [frameSwapped]() {
    frameSwapped();
    });
}

void GlWindowPlatform::connectToInitializedSignal(const std::function<void()>& initialized)
{
  QObject::connect(glWindow_, &osgQOpenGLWindow::initialized, [initialized]() {
    initialized();
    });
}

///////////////////////////////////////////////////////////////////////

GlWidgetPlatform::GlWidgetPlatform(QWidget* parent)
{
  adaptedWidget_ = new SignalingGlWidget(parent);
}

GlWidgetPlatform::~GlWidgetPlatform()
{
  delete adaptedWidget_;
}

QWidget* GlWidgetPlatform::widget()
{
  return adaptedWidget_;
}

QOpenGLWidget* GlWidgetPlatform::glWidget() const
{
  return adaptedWidget_;
}

QOpenGLWindow* GlWidgetPlatform::glWindow() const
{
  assert(0); // Dev configured as widget, asking for window
  return nullptr;
}

void GlWidgetPlatform::setFormat(const QSurfaceFormat& format)
{
  adaptedWidget_->setFormat(format);
}

QOpenGLContext* GlWidgetPlatform::qtGraphicsContext() const
{
  return adaptedWidget_->context();
}

void GlWidgetPlatform::makeCurrent()
{
  adaptedWidget_->makeCurrent();
}

void GlWidgetPlatform::doneCurrent()
{
  adaptedWidget_->doneCurrent();
}

bool GlWidgetPlatform::isValid() const
{
  return adaptedWidget_->isValid();
}

void GlWidgetPlatform::create()
{
  // QOpenGLWidget doesn't support explicit create
}

osg::GraphicsContext* GlWidgetPlatform::getGraphicsContext() const
{
  return adaptedWidget_->getGraphicsContext();
}

osgViewer::GraphicsWindow* GlWidgetPlatform::getGraphicsWindow() const
{
  return adaptedWidget_->getGraphicsWindow();
}

osgViewer::ViewerBase* GlWidgetPlatform::getOsgViewer() const
{
  return adaptedWidget_->getOsgViewer();
}

void GlWidgetPlatform::setOsgViewer(osgViewer::ViewerBase* viewer)
{
  adaptedWidget_->setOsgViewer(viewer);
}

void GlWidgetPlatform::setTimerInterval(int intervalMs)
{
  adaptedWidget_->setTimerInterval(intervalMs);
}

void GlWidgetPlatform::installEventFilter(QObject* filter)
{
  adaptedWidget_->installEventFilter(filter);
}

void GlWidgetPlatform::setResizeSignal(const std::function<void(int, int)>& resize)
{
  adaptedWidget_->setResizeSignal(resize);
}

void GlWidgetPlatform::setPrePaintSignal(const std::function<void()>& prePaint)
{
  adaptedWidget_->setPrePaintSignal(prePaint);
}

void GlWidgetPlatform::setPostPaintSignal(const std::function<void()>& postPaint)
{
  adaptedWidget_->setPostPaintSignal(postPaint);
}

void GlWidgetPlatform::connectToFrameSwappedSignal(const std::function<void()>& frameSwapped)
{
  QObject::connect(adaptedWidget_, &QOpenGLWidget::frameSwapped, [frameSwapped]() {
    frameSwapped();
    });
}

void GlWidgetPlatform::connectToInitializedSignal(const std::function<void()>& initialized)
{
  QObject::connect(adaptedWidget_, &osgQOpenGLWidget::initialized, [initialized]() {
    initialized();
    });
}

///////////////////////////////////////////////////////////////////////

GlPlatformInterface* createGlPlatform(GlImplementation glImpl, QWidget* parent)
{
  if (glImpl == GlImplementation::Widget)
    return new GlWidgetPlatform(parent);
  return new GlWindowPlatform(parent);
}

///////////////////////////////////////////////////////////////////////

ViewerWidgetAdapter::ViewerWidgetAdapter(GlImplementation glImpl, QWidget* parent)
  : QWidget(parent)
{
  glPlatform_ = createGlPlatform(glImpl, this);

  glPlatform_->setResizeSignal([this](int w, int h) { Q_EMIT glResized(w, h); });
  glPlatform_->setPrePaintSignal([this]() { Q_EMIT aboutToPaintGl(); });
  glPlatform_->setPostPaintSignal([this]() { Q_EMIT glPainted(); });
  glPlatform_->connectToFrameSwappedSignal([this]() { Q_EMIT frameSwapped(); });
  glPlatform_->connectToInitializedSignal([this]() { postGlInitialize_(); });

  initializeSurfaceFormat_();

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QWidget* glWidget = glPlatform_->widget();
  layout->addWidget(glWidget);
  setLayout(layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);
  // The widget also needs strong focus for OSG key processing to work
  glWidget->setFocusPolicy(Qt::StrongFocus);

  // Intercept drag and drop events, converted to signals
  const auto& evtFilter = [this](QEvent* evt) {
    Q_EMIT dragDropEventIntercepted(evt);
    return true;
    };
  // TODO SIM-18430: Is this required, helpful, or something we want for a widget-based solution?
  glPlatform_->installEventFilter(new simQt::DragDropEventFilter(evtFilter, glWidget));

  // Fix auto-repeat
  glPlatform_->installEventFilter(new simQt::AutoRepeatFilter(this));

  // Intercept multi-touch events and queue them into OSG
  auto* multiTouchEventFilter = new simQt::MultiTouchEventFilter(this);
  glPlatform_->installEventFilter(multiTouchEventFilter);

  // Set up the graphics window on initialization
  if (getGraphicsWindow())
    multiTouchEventFilter->setGraphicsWindow(getGraphicsWindow());
  else
  {
    glPlatform_->connectToInitializedSignal([multiTouchEventFilter, this]() {
      multiTouchEventFilter->setGraphicsWindow(getGraphicsWindow());
    });
  }
}

ViewerWidgetAdapter::~ViewerWidgetAdapter()
{
}

osgViewer::ViewerBase* ViewerWidgetAdapter::getViewer() const
{
  return glPlatform_->getOsgViewer();
}

void ViewerWidgetAdapter::setViewer(osgViewer::ViewerBase* viewer)
{
  glPlatform_->setOsgViewer(viewer);
}

void ViewerWidgetAdapter::setTimerInterval(int intervalMs)
{
  glPlatform_->setTimerInterval(intervalMs);
}

osg::GraphicsContext* ViewerWidgetAdapter::getGraphicsContext() const
{
  return glPlatform_->getGraphicsContext();
}

osgViewer::GraphicsWindow* ViewerWidgetAdapter::getGraphicsWindow() const
{
  return glPlatform_->getGraphicsWindow();
}

QOpenGLWidget* ViewerWidgetAdapter::glWidget() const
{
  return glPlatform_->glWidget();
}

QOpenGLWindow* ViewerWidgetAdapter::glWindow() const
{
  return glPlatform_->glWindow();
}

void ViewerWidgetAdapter::setFormat(const QSurfaceFormat& format)
{
  glPlatform_->setFormat(format);
}

QOpenGLContext* ViewerWidgetAdapter::qtGraphicsContext() const
{
  return glPlatform_->qtGraphicsContext();
}

void ViewerWidgetAdapter::makeCurrent()
{
  glPlatform_->makeCurrent();
}

void ViewerWidgetAdapter::doneCurrent()
{
  glPlatform_->doneCurrent();
}

bool ViewerWidgetAdapter::isValid() const
{
  return glPlatform_->isValid();
}

void ViewerWidgetAdapter::create()
{
  glPlatform_->create();
}

void ViewerWidgetAdapter::createAndProcessEvents()
{
  // TODO SIM-18431: This does not create a valid context when in widget mode. Perhaps we can
  // do something like create a temporary context?
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
