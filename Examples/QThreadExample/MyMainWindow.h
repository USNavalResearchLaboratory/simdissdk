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
#ifndef SDK_QTHREAD_EXAMPLE_MYMAINWINDOW_H
#define SDK_QTHREAD_EXAMPLE_MYMAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTimer>
#include <QSignalMapper>

#include "simData/DataStore.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simUtil/StatsHandler.h"

namespace SdkQThreadExample
{

class Gui;
class Reader;

// custom MainWindow that runs a timer against the ViewManager.
class MyMainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  MyMainWindow(simVis::ViewManager* viewMan, simData::DataStore& dataStore);
  virtual ~MyMainWindow();

  /** Redraw the view and update the user interface */
  void paintEvent(QPaintEvent* e);

public slots:
  /** Display the user interface */
  void showGenerateDialog();

protected:
  QTimer redrawTimer_;
  osg::ref_ptr<simVis::ViewManager> viewManager_;
  simData::DataStore& dataStore_;
  osg::ref_ptr<simUtil::StatsHandler> _statsHandler;
  SdkQThreadExample::Reader* reader_;
  SdkQThreadExample::Gui* generatorDialog_;
};

}
#endif
