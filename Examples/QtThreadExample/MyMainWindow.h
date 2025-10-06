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
#ifndef SDK_QTHREAD_EXAMPLE_MYMAINWINDOW_H
#define SDK_QTHREAD_EXAMPLE_MYMAINWINDOW_H

#include <QMainWindow>
#include "simVis/ViewManager.h"
#include "simUtil/StatsHandler.h"

namespace simData { class DataStore; }

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

public Q_SLOTS:
  /** Display the user interface */
  void showGenerateDialog();

protected:
  osg::ref_ptr<simVis::ViewManager> viewManager_;
  simData::DataStore& dataStore_;
  osg::ref_ptr<simUtil::StatsHandler> statsHandler_;
  SdkQThreadExample::Reader* reader_ = nullptr;
  SdkQThreadExample::Gui* generatorDialog_ = nullptr;
};

}
#endif
