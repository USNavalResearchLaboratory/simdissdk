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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
/* -*-c++-*- */
/**
 * ViewManager test -
 * Demonstrates basic use of the simVis::ViewManager class.
 */

#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simUtil/ExampleResources.h"

#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/SceneManager.h"

#include <cstdlib> // rand()


int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << "    --views [n]         : open 'n' views"
    << std::endl;

  return 0;
}


int r()
{
  return 50 + (::rand()%500);
}


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

  // We need a view manager. This handles all of our Views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager(arguments);

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan.get());

  // Create views and connect them to our scene.
  osg::ref_ptr<simVis::View> firstView;
  for (int i = 0; i < numViews; ++i)
  {
    osg::ref_ptr<simVis::View> mainView = new simVis::View();
    mainView->setSceneManager(sceneMan.get());
    mainView->setUpViewInWindow(r(), r(), 640, 480);

    // Earth Manipulator settings will be copied from the first view created
    if (firstView.valid())
      mainView->applyManipulatorSettings(*firstView);
    else
      firstView = mainView;

    // Add it to the view manager
    viewMan->addView(mainView.get());
  }

  // run until the user quits by hitting ESC.
  viewMan->run();
}
