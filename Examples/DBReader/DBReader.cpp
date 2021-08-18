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

/**
 * DB READER EXAMPLE - SIMDIS SDK
 *
 * Demonstrates loading and displaying a SIMDIS 9 SQLite terrain or imagery .db file.
 */
#include "osgEarth/DebugImageLayer"
#include "osgEarth/ImageLayer"
#include "osgEarth/Map"
#include "osgEarth/Version"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simUtil/ExampleResources.h"
#include "simVis/Viewer.h"
#include "simVis/osgEarthVersion.h"
#include "simUtil/ExampleResources.h"
#include "simVis/DBFormat.h"

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
      map->addLayer(new osgEarth::Util::DebugImageLayer());

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

    if (isElevation)
    {
      simVis::DBElevationLayer* layer = new simVis::DBElevationLayer();
      layer->setURL(token);
      map->addLayer(layer);
    }
    else
    {
      simVis::DBImageLayer* layer = new simVis::DBImageLayer();
      layer->setURL(token);
      map->addLayer(layer);
    }
  }

  // start up a SIMDIS viewer.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  viewer->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  viewer->installDebugHandlers();
  viewer->run();
}

