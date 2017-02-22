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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
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

class QTimer;
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
  void paintEvent(QPaintEvent* e);
  simVis::ViewManager* getViewManager();

private slots:
  void createViewDialog_();
  void createViewDockable_();
  void createMainView_();

private:

  simVis::View* createView_(const QString& name) const;

  int                                 viewCounter_;
  QTimer*                             timer_;
  osg::ref_ptr<simVis::ViewManager>   viewMan_;
  osg::ref_ptr<simVis::SceneManager>  sceneMan_;
};

#endif
