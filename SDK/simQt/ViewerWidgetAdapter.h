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

class osgQOpenGLWindow;
class QOpenGLContext;
class QSurfaceFormat;
namespace osgViewer {
  class GraphicsWindow;
  class ViewerBase;
}
namespace osg { class GraphicsContext; }

namespace simQt {

class SignalingGlWindow;

/**
 * Adapter QWidget that will adapt an owned osgQt osgQOpenGLWindow to a widget. This
 * results in a faster rendering than using osgQOpenGLWidget natively, and fixes
 * various issues with using osgQOpenGLWindow without the adapter (such as mouse
 * interactions with the window).
 *
 * This class is intended to be a direct replacement for osgQOpenGLWindow, and should
 * be more effective in every typical use case than either osgQOpenGLWindow or
 * osgQOpenGLWidget.
 */
class SDKQT_EXPORT ViewerWidgetAdapter : public QWidget
{
  Q_OBJECT;

public:
  explicit ViewerWidgetAdapter(QWidget* parent = nullptr);
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

  /** Retrieve the OpenGL Window class */
  osgQOpenGLWindow* glWindow() const;

  /** Sets the graphics format on the window */
  void setFormat(const QSurfaceFormat& format);
  /** Retrieves the OpenGL Context, in Qt format */
  QOpenGLContext* qtGraphicsContext() const;
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

Q_SIGNALS:
  void initialized();
  void glResized(int w, int h);
  void aboutToPaintGl();
  void glPainted();
  void frameSwapped();

  /**
   * Emitted when a drag-and-drop event on the QOpenGLWindow is generated. This
   * may be a drag, drop, move, or leave event. If you need drag and drop support,
   * connect this signal to a function that executes your event() function.
   */
  void dragDropEventIntercepted(QEvent* evt);

private Q_SLOTS:
  /** Called when the graphics context initializes, before initialized() emitted, applying graphics context fixes */
  void postGlInitialize_();

private:
  /** Initializes the surface format based on OSG defaults */
  void initializeSurfaceFormat_();

  SignalingGlWindow* glWindow_ = nullptr;
};

}

#endif /* SIMQT_VIEWERWIDGETADAPTER_H */
