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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
/* -*-c++-*- */

#ifndef QTDOCKABLEVIEWS_MAINWINDOW_H
#define QTDOCKABLEVIEWS_MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>
#include <osg/ref_ptr>

class QTimer;
class QWindow;
class QGLWidget;
namespace simVis
{
  class SceneManager;
  class ViewManager;
  class View;
}

/**
 * A simple MainWindow derivative that provides a showcase for creating simVis::Views in different Qt widgets.
 * Contains a simVis::ViewManager and simVis::SceneManager so that all views share the same scene. Provides a
 * toolbar with buttons to generate QDialogs, QDockWidget, or simple QWidgets in the main layout.
 */
class MyMainWindow : public QMainWindow
{
  Q_OBJECT;
public:
  explicit MyMainWindow(int framerate);
  virtual ~MyMainWindow();

  void setGlWidget(QGLWidget* glWidget);
  simVis::ViewManager* getViewManager();

protected:
  virtual void paintEvent(QPaintEvent* e);

private Q_SLOTS:
  void createViewDialog_();
  void createViewDockable_();
  void createMainView_();

private:

  simVis::View* createView_(const QString& name) const;

  int                                 viewCounter_;
  QTimer*                             timer_;
  osg::ref_ptr<simVis::ViewManager>   viewMan_;
  osg::ref_ptr<simVis::SceneManager>  sceneMan_;
  QPointer<QWindow>                   lastCreatedGlWindow_;
};

#endif
