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
/* -*-c++-*- */

#ifndef QTDOCKABLEVIEWS_MAINWINDOW_H
#define QTDOCKABLEVIEWS_MAINWINDOW_H

#include <QMainWindow>
#include <osg/ref_ptr>

namespace simVis {
  class SceneManager;
  class ViewManager;
  class View;
}
namespace simQt { class ViewerWidgetAdapter; }

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

private Q_SLOTS:
  void createViewDialog_();
  void createViewDockable_();
  void createMainView_();

private:
  simVis::View* createView_(simVis::ViewManager& viewManager, const QString& name) const;
  simQt::ViewerWidgetAdapter* newWidget_(const QString& viewName);

  int viewCounter_ = 1;
  int timerInterval_ = 33;
  std::vector<osg::ref_ptr<simVis::ViewManager>> viewManagers_;
  osg::ref_ptr<simVis::SceneManager> sceneMan_;
};

#endif
