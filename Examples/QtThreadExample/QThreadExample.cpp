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
/**
 * Qt Integration Example.
 *
 * Demonstrates embedding the SIMDIS SDK Viewer in a Qt widget using a QThread to generate data
 */

#include "simCore/Common/Version.h"
#include "simCore/System/Utils.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Time/Clock.h"
#include "simCore/Time/ClockImpl.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simQt/ViewerWidgetAdapter.h"
#include "simUtil/DefaultDataStoreValues.h"
#include "simUtil/ExampleResources.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QSignalMapper>

#include "MyMainWindow.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif


//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::initializeSimdisEnvironmentVariables();
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  // a Map and a Scene Manager:
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan.get());

  // A view to embed in our widget:
  osg::ref_ptr<simVis::View> view = new simVis::View();
  view->setSceneManager(sceneMan.get());
  view->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  // Note that no debug handlers are installed, because we cycle through frame rate in menu

  // The ViewManager coordinates the rendering of all our views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager();

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan.get());

  // Add a new "top-level" view. A top-level view can have inset views, and
  // also has a HUD stack for overlay text and graphics.
  viewMan->addView(view.get());

  // Add a dataStore for the platform
  simData::MemoryDataStore dataStore;
  sceneMan->getScenario()->bind(&dataStore);

#ifdef Q_WS_X11
  // required for multi-threaded viewer on Linux:
  XInitThreads();
#endif

  QApplication app(argc, argv);

  SdkQThreadExample::MyMainWindow win(viewMan.get(), dataStore);

  // Make the ViewerWidgetAdapter
  auto* viewWidget = new simQt::ViewerWidgetAdapter(&win);
  viewWidget->setViewer(viewMan->getViewer());
  viewWidget->setTimerInterval(20);
  win.setCentralWidget(viewWidget);
  win.setGeometry(100, 100, 1024, 800);

  win.statusBar()->showMessage(QString("Congratulations! You've embedded the SDK Viewer in a Qt Widget."));

  QMenu* bar = win.menuBar()->addMenu(QString("File"));
  QAction* generateAction = new QAction(QString("Generate Data..."), &win);
  QObject::connect(generateAction, SIGNAL(triggered(bool)), &win, SLOT(showGenerateDialog()));
  bar->addAction(generateAction);

  QAction* exitAction = new QAction(QString("Exit"), &win);
  QObject::connect(exitAction, SIGNAL(triggered(bool)), &win, SLOT(close()));
  exitAction->setShortcut(QKeySequence("Alt+Q"));
  bar->addAction(exitAction);

  win.show();
  app.exec();
  delete generateAction;
  delete exitAction;
  delete viewWidget;
  return 0;
}
