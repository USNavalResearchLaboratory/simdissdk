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
#ifndef QTDOCKINGOPTIONS_MAINWINDOW_H
#define QTDOCKINGOPTIONS_MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include "osg/ref_ptr"

class QDockWidget;

namespace simVis {
  class SceneManager;
  class ViewManager;
  class View;
}
namespace simQt { class ViewerWidgetAdapter; }

/**
 * A simple MainWindow derivative that provides a showcase for simQt::DockWidgets vs baseline Qt Docking options,
 * in addition to demonstrating the behavior of children spawned by those dock widgets
 */
class MyMainWindow : public QMainWindow
{
  Q_OBJECT;
public:
  explicit MyMainWindow(int framerate);
  virtual ~MyMainWindow();

private Q_SLOTS:
  void createSimQtDockable_();
  void createQDockable_();

private:
  void createMainView_();
  simVis::View* createView_(simVis::ViewManager& viewManager, const QString& name) const;
  simQt::ViewerWidgetAdapter* newWidget_(const QString& viewName);

  int viewCounter_ = 1;
  int timerInterval_ = 33;
  osg::ref_ptr<simVis::ViewManager> viewManager_;
  osg::ref_ptr<simVis::SceneManager> sceneMan_;
  std::vector<QDockWidget*> dockables_;
};

#endif
