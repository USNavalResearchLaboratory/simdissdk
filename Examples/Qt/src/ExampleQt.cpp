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
/**
 * Qt Integration Example.
 *
 * Demonstrates embedding the SIMDIS SDK Viewer in a Qt widget.
 */
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simUtil/ExampleResources.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"

#include "osgEarthQt/ViewWidget"

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QSignalMapper>

#include "MyMainWindow.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

//----------------------------------------------------------------------------+

// custom action to set the frame rate dynamically
struct FrameRateAction : public QAction
{
  FrameRateAction(MyMainWindow* win, QSignalMapper& signalMapper, int frameRateHz)
    : QAction(win)
  {
    int interval = 0;
    if (frameRateHz != 0)
    {
      setText(QString("%1 Hertz").arg(frameRateHz));
      interval = 1000 / frameRateHz;
    }
    else
    {
      setText("Unlimited");
    }

    signalMapper.setMapping(this, interval);
    connect(this, SIGNAL(triggered()), &signalMapper, SLOT(map()));
    setCheckable(true);
  }
};


// custom action for File->Exit menu :)
struct ExitAction : public QAction
{
  explicit ExitAction(QMainWindow* win) : QAction(QString("Exit"), NULL), win_(win)
  {
    connect(this, SIGNAL(triggered()), win_, SLOT(close()));
  }
  QMainWindow* win_;
};

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  // a Map and a Scene Manager:
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map);

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan);

  // A view to embed in our widget:
  osg::ref_ptr<simVis::View> view = new simVis::View();
  view->setSceneManager(sceneMan);
  view->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  // Note that no debug handlers are installed, because we cycle through frame rate in menu

  // The ViewManager coordinates the rendering of all our views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager();

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan);

  // Add a new "top-level" view. A top-level view can have inset views, and
  // also has a HUD stack for overlay text and graphics.
  viewMan->addView(view);


#ifdef Q_WS_X11
  // required for multi-threaded viewer on Linux:
  XInitThreads();
#endif

  QApplication app(argc, argv);

  MyMainWindow win(viewMan);
  osgEarth::QtGui::ViewWidget* viewWidget = new osgEarth::QtGui::ViewWidget(view);
  win.setCentralWidget(viewWidget);
  win.setGeometry(100, 100, 1024, 800);

  QSignalMapper mapper(&app);
  QObject::connect(&mapper, SIGNAL(mapped(int)), &win, SLOT(setTimerInterval(int)));

  win.statusBar()->showMessage(
    QString("Congratulations! You've embedded the SDK Viewer in a Qt Widget."));

  QMenu* bar = win.menuBar()->addMenu(QString("File"));
  ExitAction* action = new ExitAction(&win);
  action->setShortcut(QKeySequence("Alt+Q"));
  bar->addAction(action);

  bar = win.menuBar()->addMenu(QString("Frame Rate"));

  QAction* toggleFrameRateAction = new QAction("Show Frame Rate", &win);
  toggleFrameRateAction->setShortcut(QKeySequence("Alt+F"));
  toggleFrameRateAction->setCheckable(true);
  bar->addAction(toggleFrameRateAction);
  QObject::connect(toggleFrameRateAction, SIGNAL(toggled(bool)), &win, SLOT(toggleFrameRate(bool)));
  bar->addSeparator()->setText("Rates");

  QActionGroup* actionGroup = new QActionGroup(&win);
  actionGroup->addAction(new FrameRateAction(&win, mapper, 1));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 10));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 15));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 30));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 60));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 120));
  actionGroup->addAction(new FrameRateAction(&win, mapper, 0));
  actionGroup->setExclusive(true);
  bar->addActions(actionGroup->actions());

  // Activate the 30 Hz
  actionGroup->actions().at(3)->trigger();

  win.show();
  app.exec();
  delete action;
  delete viewWidget;
  return 0;
}

