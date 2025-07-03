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
#include <memory>
#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QTimer>
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

/** Simple helper function to clone a drag-and-drop event. */
std::unique_ptr<QEvent> cloneDragDropEvent_(const QEvent* evt)
{
  switch (evt->type())
  {
  case QEvent::DragEnter:
  {
    const QDragEnterEvent* dragEnterEvent = static_cast<const QDragEnterEvent*>(evt);
    return std::make_unique<QDragEnterEvent>(dragEnterEvent->pos(), dragEnterEvent->possibleActions(), dragEnterEvent->mimeData(), dragEnterEvent->mouseButtons(), dragEnterEvent->keyboardModifiers());
  }
  case QEvent::DragLeave:
    return std::make_unique<QDragLeaveEvent>();
  case QEvent::DragMove:
  {
    const QDragMoveEvent* dragMoveEvent = static_cast<const QDragMoveEvent*>(evt);
    return std::make_unique<QDragMoveEvent>(dragMoveEvent->pos(), dragMoveEvent->possibleActions(), dragMoveEvent->mimeData(), dragMoveEvent->mouseButtons(), dragMoveEvent->keyboardModifiers());
  }
  case QEvent::Drop:
  {
    const QDropEvent* dropEvent = static_cast<const QDropEvent*>(evt);
    return std::make_unique<QDropEvent>(dropEvent->pos(), dropEvent->possibleActions(), dropEvent->mimeData(), dropEvent->mouseButtons(), dropEvent->keyboardModifiers());
  }
  default:
    break;
  }
  return {};
}

///////////////////////////////////////////////////////////////////////

/**
 * Forwards drag/drop events to a given lambda. Useful, for example, to capture drag/drop from a
 * QOpenGLWindow using its QWidget holder. Recommended usage:
 * <code>
 *   widget->installEventFilter(new simQt::DragDropEventFilter(
 *     [this](QEvent* evt) { return event(evt); },
 *     widget));
 * </code>
 */
class DragDropEventFilter : public QObject
{
public:
  explicit DragDropEventFilter(const std::function<bool(QEvent*)>& lambda, QObject* parent = nullptr)
    : lambda_(lambda)
  {
  }

protected:
  // From QObject:
  virtual bool eventFilter(QObject* watched, QEvent* event) override
  {
    if (lambda_)
    {
      switch (event->type())
      {
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::DragLeave:
      case QEvent::Drop:
        return lambda_(event);
      default:
        break;
      }
    }
    return QObject::eventFilter(watched, event); // Let other events pass through
  }

private:
  std::function<bool(QEvent*)> lambda_;
};

///////////////////////////////////////////////////////////////////////

/**
 * Base class that adapts both osgQOpenGLWidget and osgQOpenGLWindow. Nearly all methods are
 * forwarded practically as-is to the appropriate adapted widget/window.
 */
class GlPlatformInterface
{
public:
  virtual ~GlPlatformInterface() = default;

  virtual QWidget* widget() const = 0;
  virtual QOpenGLWidget* glWidget() const = 0;
  virtual QOpenGLWindow* glWindow() const = 0;
  virtual QOpenGLContext* qtGraphicsContext() const = 0;
  virtual QSurfaceFormat format() const = 0;
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

/** GlPlatformInterface instance for a osgQOpenGLWindow. */
class GlWindowPlatform : public GlPlatformInterface
{
public:
  explicit GlWindowPlatform(QWidget* parent = nullptr);
  virtual ~GlWindowPlatform();
  SDK_DISABLE_COPY_MOVE(GlWindowPlatform);

  virtual QWidget* widget() const override;
  virtual QOpenGLWidget* glWidget() const override;
  virtual QOpenGLWindow* glWindow() const override;
  virtual QOpenGLContext* qtGraphicsContext() const override;
  virtual QSurfaceFormat format() const override;
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

/** GlPlatformInterface instance for a osgQOpenGLWidget. */
class GlWidgetPlatform : public GlPlatformInterface
{
public:
  explicit GlWidgetPlatform(QWidget* parent = nullptr);
  virtual ~GlWidgetPlatform() override;
  SDK_DISABLE_COPY_MOVE(GlWidgetPlatform);

  virtual QWidget* widget() const override;
  virtual QOpenGLWidget* glWidget() const override;
  virtual QOpenGLWindow* glWindow() const override;
  virtual QOpenGLContext* qtGraphicsContext() const override;
  virtual QSurfaceFormat format() const override;
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
  std::unique_ptr<QOpenGLContext> proxyContext_;
  std::unique_ptr<QOffscreenSurface> offscreenSurface_;
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

QWidget* GlWindowPlatform::widget() const
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

QSurfaceFormat GlWindowPlatform::format() const
{
  return glWindow_->format();
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

  // Set up a first connection to the initialized signal, so that we delete and
  // clear out our surface and proxy context if it was previously created.
  QMetaObject::Connection delProxyContext;
  delProxyContext = QObject::connect(adaptedWidget_, &SignalingGlWidget::initialized, [this, delProxyContext]() {
    proxyContext_.reset();
    offscreenSurface_.reset();
    // Only ever initialize once
    QTimer::singleShot(0, [delProxyContext, this]() { adaptedWidget_->disconnect(delProxyContext); });
    });
}

GlWidgetPlatform::~GlWidgetPlatform()
{
  delete adaptedWidget_;
}

QWidget* GlWidgetPlatform::widget() const
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

QSurfaceFormat GlWidgetPlatform::format() const
{
  return adaptedWidget_->format();
}

void GlWidgetPlatform::setFormat(const QSurfaceFormat& format)
{
  adaptedWidget_->setFormat(format);
  if (proxyContext_ && offscreenSurface_)
  {
    offscreenSurface_->setFormat(format);
    proxyContext_->setFormat(format);
  }
}

QOpenGLContext* GlWidgetPlatform::qtGraphicsContext() const
{
  // If the widget is not valid, then context will be null
  if (adaptedWidget_->isValid())
    return adaptedWidget_->context();
  // Fall back on the proxy context, which may be null without create()
  return proxyContext_.get();
}

void GlWidgetPlatform::makeCurrent()
{
  if (adaptedWidget_->isValid())
    adaptedWidget_->makeCurrent();
  else if (proxyContext_ && offscreenSurface_)
    proxyContext_->makeCurrent(offscreenSurface_.get());
}

void GlWidgetPlatform::doneCurrent()
{
  if (adaptedWidget_->isValid())
    adaptedWidget_->doneCurrent();
  else if (proxyContext_ && offscreenSurface_)
    proxyContext_->doneCurrent();
}

bool GlWidgetPlatform::isValid() const
{
  return adaptedWidget_->isValid() ||
    (proxyContext_ && proxyContext_->isValid());
}

void GlWidgetPlatform::create()
{
  // Avoid no-op
  if (adaptedWidget_->isValid())
    return;

  // There is no way to create a QOpenGLWidget without showing it. But we can
  // create a proxy graphics context on an offscreen surface and create that,
  // then set it up as a shared context. Lazy creation, only when needed
  if (offscreenSurface_ && proxyContext_ && proxyContext_->isValid())
    return;
  // Creating one, creates the other
  assert(!offscreenSurface_ && !proxyContext_);

  offscreenSurface_ = std::make_unique<QOffscreenSurface>();
  offscreenSurface_->setFormat(adaptedWidget_->format());
  offscreenSurface_->create();
  proxyContext_ = std::make_unique<QOpenGLContext>();
  proxyContext_->setFormat(adaptedWidget_->format());
  proxyContext_->create();
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
  QWidget* windowOrWidgetContainer = glPlatform_->widget();
  layout->addWidget(windowOrWidgetContainer);
  setLayout(layout);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);
  // The widget also needs strong focus for OSG key processing to work
  windowOrWidgetContainer->setFocusPolicy(Qt::StrongFocus);

  // Intercept drag and drop events for the window, try to process as our own
  if (glImpl == GlImplementation::Window)
  {
    const auto& evtFilter = [windowOrWidgetContainer, this](QEvent* evt) -> bool {
      auto clonedEvent = cloneDragDropEvent_(evt);
      if (clonedEvent)
        QCoreApplication::postEvent(this, clonedEvent.release());
      return true;
      };
    glPlatform_->installEventFilter(new simQt::DragDropEventFilter(evtFilter, this));
    windowOrWidgetContainer->setAcceptDrops(true);
  }

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

QSurfaceFormat ViewerWidgetAdapter::format() const
{
  return glPlatform_->format();
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
  create();
  QCoreApplication::processEvents();
}

void ViewerWidgetAdapter::initializeSurfaceFormat_()
{
  QSurfaceFormat surfaceFormat = QSurfaceFormat::defaultFormat();
  surfaceFormat.setSamples(4); // Default to 4x MSAA
  surfaceFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
  // If we are stuck with core profile, go with version 3.3; if built
  // with compatibility profile support, don't specify version
  surfaceFormat.setVersion(3, 3);
  surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
#endif
  surfaceFormat.setSwapInterval(1); // vsync
  surfaceFormat.setRenderableType(QSurfaceFormat::OpenGL);
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

QSize ViewerWidgetAdapter::sizeHint() const
{
  auto oldHint = QWidget::sizeHint();
  if (oldHint.width() == 0 && oldHint.height() == 0)
    return QSize(640, 480);
  return oldHint;
}

}
