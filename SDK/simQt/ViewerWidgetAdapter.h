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
#ifndef SIMQT_VIEWERWIDGETADAPTER_H
#define SIMQT_VIEWERWIDGETADAPTER_H

#include <QWidget>
#include "simCore/Common/Common.h"

class QOpenGLContext;
class QOpenGLWidget;
class QOpenGLWindow;
class QSurfaceFormat;
namespace osgViewer {
  class GraphicsWindow;
  class ViewerBase;
}
namespace osg { class GraphicsContext; }

namespace simQt {

class GlPlatformInterface;

/** Specifies the OpenGL implementation to use with the ViewerWidgetAdapter. */
enum class GlImplementation {
  /**
   * Use the osgQOpenGLWindow-based implementation. This implementation offers
   * potentially faster rendering performance but may exhibit issues with
   * Qt::WidgetWithChildrenShortcut contexts because of QWindow vs QWidget issues.
   * Window mode also does not cleanly integrate with drag-and-drop, and you may
   * get more drag enter/leave messages than anticipated.
   */
  Window,

  /**
   * Use the osgQOpenGLWidget-based implementation. This implementation ensures
   * correct behavior with Qt::WidgetWithChildrenShortcut contexts but may have
   * slower rendering performance compared to the Window implementation. The widget
   * implementation is not valid until it is first shown, and cannot be created
   * without being shown. To work around this, an off-screen GL context is created
   * if the create() function is called before initialization. This context is
   * temporary and will be deleted on initialization.
   */
  Widget
};

/**
 * @brief Adapter QWidget that allows seamless switching between an osgQOpenGLWindow-based or an osgQOpenGLWidget-based rendering solution.
 *
 * This class provides a unified interface for embedding either an `osgQOpenGLWindow` or an `osgQOpenGLWidget`
 * within a Qt application. It addresses the challenges of integrating these third-party osgQOpenGL components
 * into a standard QWidget-based layout and event handling system.
 *
 * When configured to use `osgQOpenGLWindow`, this adapter offers potentially faster rendering performance.
 * However, due to the underlying QWindow nature of `osgQOpenGLWindow`, standard Qt shortcut handling
 * using `Qt::WidgetWithChildrenShortcut` contexts may not function correctly. This limitation is mitigated
 * when using the `osgQOpenGLWidget` configuration.
 *
 * When configured to use `osgQOpenGLWidget`, this configuration may have slower rendering performance than
 * the `osgQOpenGLWindow` based configuration.
 *
 * This class is intended to be a versatile replacement for direct usage of either `osgQOpenGLWindow` or
 * `osgQOpenGLWidget`, providing a flexible and adaptable solution for various use cases.
 *
 * @note It is recommended to use `simVis::ViewManager::setUseMultipleViewers(true)` when using this with a
 *       ViewManager instance, especially if expecting to use multiple widgets with a single ViewManager.
 */
class SDKQT_EXPORT ViewerWidgetAdapter : public QWidget
{
  Q_OBJECT;

public:
  explicit ViewerWidgetAdapter(GlImplementation glImpl, QWidget* parent = nullptr);
  virtual ~ViewerWidgetAdapter();
  SDK_DISABLE_COPY_MOVE(ViewerWidgetAdapter);

  /** Retrieve the OSG Viewer */
  osgViewer::ViewerBase* getViewer() const;
  /**
   * Set the OSG Viewer. Note that a single Viewer may not be able to be used across more
   * than one ViewerWidgetAdapter due to limitations in osgQOpenGL. This is because the
   * graphics context supplied to OSG from osgQOpenGL cannot makeCurrent() different
   * widgets, and the locus of control for rendering the frame now lies with osgQOpenGL
   * instead of OSG itself.
   */
  void setViewer(osgViewer::ViewerBase* viewer);

  /** Sets the timer interval for updating the graphics, in ms */
  void setTimerInterval(int intervalMs);

  /** Retrieve the graphics context */
  osg::GraphicsContext* getGraphicsContext() const;
  /** Retrieve the graphics window */
  osgViewer::GraphicsWindow* getGraphicsWindow() const;

  /** Retrieve the OpenGL Widget class; only if constructed with Window mode. */
  QOpenGLWidget* glWidget() const;
  /** Retrieve the OpenGL Window class; only if constructed with Widget mode. */
  QOpenGLWindow* glWindow() const;
  /** Retrieves the OpenGL Context, in Qt format; works in both configurations. */
  QOpenGLContext* qtGraphicsContext() const;

  /** Sets the graphics format on the window */
  void setFormat(const QSurfaceFormat& format);
  /** Makes the GL context current */
  void makeCurrent();
  /** Notify that the previous makeCurrent() is complete. */
  void doneCurrent();
  /** Returns true if the context is valid */
  bool isValid() const;
  /** Attempts to create the graphics context; a QCoreApplication::processEvents() may be required */
  void create();
  /** Combines create() with QCoreApplication::processEvents(), as a convenience */
  void createAndProcessEvents();

  // From QWidget:
  virtual QSize sizeHint() const override;

Q_SIGNALS:
  void initialized();
  void glResized(int w, int h);
  void aboutToPaintGl();
  void glPainted();
  void frameSwapped();

private Q_SLOTS:
  /** Called when the graphics context initializes, before initialized() emitted, applying graphics context fixes */
  void postGlInitialize_();

private:
  /** Initializes the surface format based on OSG defaults */
  void initializeSurfaceFormat_();

  GlPlatformInterface* glPlatform_ = nullptr;
};

}

#endif /* SIMQT_VIEWERWIDGETADAPTER_H */
