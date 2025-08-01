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
#ifndef TVMDM_MAINWINDOW_H
#define TVMDM_MAINWINDOW_H

#include <vector>
#include <QMainWindow>
#include "simVis/ViewManager.h"
#include "osg/ref_ptr"
#include "osg/observer_ptr"

class QTreeView;

namespace simVis {
  class View;
}

/**
 * A simple MainWindow derivative that shows one way to embed a
 * simVis::ViewManager configuration in a Qt UI.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT;
public:
  MainWindow();
  simVis::ViewManager* getViewManager() const;
  void addMainView(simVis::View* mainView);

public Q_SLOTS:
  void addView();
  void removeView();

private:
  QTreeView* topTreeView_ = nullptr;
  osg::ref_ptr<simVis::ViewManager> viewMan_;
  std::vector<osg::observer_ptr<simVis::View> > mainViews_;
  int numInsetsCreated_ = 0;
};

#endif /* TVMDM_MAINWINDOW_H */
