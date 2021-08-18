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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/Common/Version.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"

int main(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // Set up OSG features if supported
  osg::DisplaySettings::instance()->setNumMultiSamples(4);

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // start in a windowed mode
  viewer->getMainView()->setUpViewInWindow(100, 100, 1024, 768);
  // set an initial viewpoint
  viewer->getMainView()->lookAt(38.89511, -77.03637, 0, 0, -89, 5e6);

  // add debug handlers like stats and fullscreen mode (s and f hotkeys)
  viewer->installDebugHandlers();

  return viewer->run();
}
