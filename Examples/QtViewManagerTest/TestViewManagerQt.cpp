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
 * ViewManager Qt Test -
 * Demonstrates basic use of the simVis::ViewManager class with a Qt UI.
 */

#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simCore/System/Utils.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simQt/ViewerWidgetAdapter.h"
#include "simUtil/ExampleResources.h"

#include <QApplication>
#include <QLayout>
#include <QMainWindow>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --views [n]         : open 'n' views"
    << std::endl;

  return 0;
}


int main(int argc, char** argv)
{
  simCore::initializeSimdisEnvironmentVariables();
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

  QMainWindow win;
  win.setGeometry(50, 50, 400*numViews, 400);
  QWidget* center = new QWidget();
  center->setLayout(new QHBoxLayout());
  win.setCentralWidget(center);

  // Retain the view managers so they can get cleaned up on exit
  osg::ref_ptr<simVis::ViewManager> viewManager(new simVis::ViewManager);
  // View Manager will support multiple top level CompositeViewer instances for osgQOpenGL
  viewManager->setUseMultipleViewers(true);

  // Create views and connect them to our scene.
  for (int i = 0; i < numViews; ++i)
  {
    // Make a view, hook it up, and add it to the view manager.
    osg::ref_ptr<simVis::View> mainview = new simVis::View();
    // attach the scene manager and add it to the view manager.
    mainview->setSceneManager(sceneMan.get());
    viewManager->addView(mainview.get());

    // Make a Qt Widget to hold our view, and add that widget to the
    // main window.
    auto* viewWidget = new simQt::ViewerWidgetAdapter(&win);
    viewWidget->setViewer(viewManager->getViewer(mainview));
    viewWidget->setTimerInterval(10);
    center->layout()->addWidget(viewWidget);

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
