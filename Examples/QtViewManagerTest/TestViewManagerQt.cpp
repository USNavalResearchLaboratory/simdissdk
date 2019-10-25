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
 * ViewManager Qt Test -
 * Demonstrates basic use of the simVis::ViewManager class with a Qt UI.
 */

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simQt/ViewWidget.h"
#include "simUtil/ExampleResources.h"

#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/SceneManager.h"

#include "osgEarth/Controls"

#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QPushButton>
#include <QLayout>
#include <QTimer>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include <cstdlib> // rand()

namespace ui = osgEarth::Util::Controls;


int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --views [n]         : open 'n' views"
    << std::endl;

  return 0;
}


/**
 * A simple MainWindow derivative that shows one way to embed a
 * simVis::ViewManager configuration in a Qt UI.
 */
struct MyMainWindow : public QMainWindow
{
  QTimer                            _timer;
  osg::ref_ptr<simVis::ViewManager> _viewMan;

  MyMainWindow()
  {
    // create a viewer manager. The "args" are optional.
    _viewMan = new simVis::ViewManager();

    // disable the default ESC-to-quit event:
    _viewMan->getViewer()->setKeyEventSetsDone(0);
    _viewMan->getViewer()->setQuitEventSetsDone(false);

    // timer fires a paint event.
    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    // timer single shot to avoid infinite loop problems in Qt on MSVC11
    _timer.setSingleShot(true);
    _timer.start(10);
  }

  void paintEvent(QPaintEvent* e)
  {
    // refresh all the views.
    _viewMan->frame();
    _timer.start();
  }

  simVis::ViewManager* getViewManager()
  {
    return _viewMan.get();
  }
};


int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // read the number of views to open:
  int numViews = 1;
  arguments.read("--views", numViews);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan.get());

  // OK, time to set up the Qt Application and windows.
  QApplication qapp(argc, argv);

  // Our custom main window contains a ViewManager.
  MyMainWindow win;
  win.setGeometry(50, 50, 400*numViews, 400);
  QWidget* center = new QWidget();
  center->setLayout(new QHBoxLayout());
  win.setCentralWidget(center);

  // Create views and connect them to our scene.
  for (int i = 0; i < numViews; ++i)
  {
    // Make a view, hook it up, and add it to the view manager.
    osg::ref_ptr<simVis::View> mainview = new simVis::View();

    // Make a Qt Widget to hold our view, and add that widget to the
    // main window.
    QWidget* viewWidget = new simQt::ViewWidget(mainview.get());
    center->layout()->addWidget(viewWidget);

    // attach the scene manager and add it to the view manager.
    mainview->setSceneManager(sceneMan.get());
    win.getViewManager()->addView(mainview.get());

    // Each top-level view needs an Inset controller so the user can draw
    // and interact with inset views.
    osg::ref_ptr<simVis::View> inset = new simVis::View();

    // set up the new inset's extents as a percentage of the parent's size.
    inset->setExtents(simVis::View::Extents(0.2, 0.2, 0.5, 0.5, true));
    inset->setSceneManager(sceneMan.get());
    inset->applyManipulatorSettings(*mainview);
    mainview->addInset(inset.get());
  }

  // fire up the GUI.
  win.show();
  qapp.exec();
}
