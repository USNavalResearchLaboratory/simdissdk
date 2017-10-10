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

/**
 * DB READER EXAMPLE - SIMDIS SDK
 *
 * Demonstrates loading and displaying a SIMDIS 9 SQLite terrain or imagery .db file.
 */
#include "osgEarth/Map"
#include "osgEarth/ImageLayer"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simUtil/ExampleResources.h"
#include "simVis/Viewer.h"
#include "simVis/DBOptions.h"
#include "simVis/osgEarthVersion.h"
#include "simUtil/ExampleResources.h"

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  if (argc < 2)
  {
    std::cout << "USAGE: example_dbreader [--elevation <dbfile>] [<dbfile>] ..." << std::endl;
    return -1;
  }

  // Start by creating an empty map.
  osg::ref_ptr<osgEarth::Map> map = new osgEarth::Map();

  for (int i = 1; i < argc; ++i)
  {
    std::string token = argv[i];

    if (token == "--debug")
    {
      // Add the debug driver
      simVis::DBOptions driverOptions;
      driverOptions.setDriver("debug");
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
      map->addLayer(new osgEarth::ImageLayer("debug", driverOptions));
#else
      map->addImageLayer(new osgEarth::ImageLayer("debug", driverOptions));
#endif

      // advance the token
      continue;
    }

    // add the db layers on the command line. if a file is preceded by the
    // --elevation tag, load it as elevation data.
    bool isElevation = false;
    if (token == "--elevation")
    {
      isElevation = true;
      // advance the token
      ++i;
      if (i >= argc)
        break;
      token = argv[i];
    }

    simVis::DBOptions driverOptions;
    driverOptions.url() = token;

#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    if (isElevation)
      map->addLayer(new osgEarth::ElevationLayer(token, driverOptions));
    else
      map->addLayer(new osgEarth::ImageLayer(token, driverOptions));
#else
    if (isElevation)
      map->addElevationLayer(new osgEarth::ElevationLayer(token, driverOptions));
    else
      map->addImageLayer(new osgEarth::ImageLayer(token, driverOptions));
#endif
  }

  // start up a SIMDIS viewer.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  viewer->setMap(map);

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  viewer->installDebugHandlers();
  viewer->run();
}

