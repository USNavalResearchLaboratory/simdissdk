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

/**
 * Demonstrates how to use the Line of Bearing object, which display multiple time
 * stamped lines emanating from a platform's history trail.
 */

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/CoordinateConverter.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Viewer.h"

/// Animated lines
#include "simVis/LobGroup.h"
#include "simVis/GOG/Annotation.h"

/// paths to models
#include "simUtil/ExampleResources.h"

//----------------------------------------------------------------------------

// first line, describe the program
static const std::string s_title = "LOB Group Test";

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore)
{
  // create the platform:
  simData::ObjectId platformId;
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
    platformId = props->id();
    xaction.complete(&props);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return platformId;
}

//----------------------------------------------------------------------------

simData::ObjectId addLobGroup(simData::ObjectId p1, simData::DataStore& ds)
{
  simData::ObjectId lobID;

  // make a LOB group
  {
    simData::DataStore::Transaction xaction;
    simData::LobGroupProperties* props = ds.addLobGroup(&xaction);
    lobID = props->id();
    props->set_hostid(p1);
    xaction.complete(&props);
  }

  // initial prefs
  {
    simData::DataStore::Transaction xaction;
    simData::LobGroupPrefs* prefs = ds.mutable_lobGroupPrefs(lobID, &xaction);
    prefs->set_color1(0xFF0000FF);
    prefs->set_color2(0x00FF00FF);
    prefs->set_stipple1(0xFF00);
    prefs->set_stipple2(0x00FF);
    prefs->set_lobwidth(4);
    prefs->mutable_commonprefs()->set_datadraw(true);
    prefs->mutable_commonprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return lobID;
}

//----------------------------------------------------------------------------

void simulatePlatform(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(21.5, -158.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(21.5, -157.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim.get());
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0);
  viewer->addEventHandler(simHandler.get());
}

//----------------------------------------------------------------------------

void simulateLobGroup(simData::ObjectId lobID, simData::DataStore& ds)
{
  for (double i = 0.0; i <= 12.0; i += 1.0)
  {
    simData::DataStore::Transaction xaction;
    simData::LobGroupUpdate* update = ds.addLobGroupUpdate(lobID, &xaction);
    update->set_time(i*3.0);
    {
      simData::LobGroupUpdatePoint* p = update->add_datapoints();
      p->set_time(i * 3.0);
      p->set_range(100000.0);
      p->set_azimuth(-1.57 + (3.14 * i / 12.0));
      p->set_elevation(-.39 + (0.78 * i / 12.0));
    }
    xaction.complete(&update);
  }

  // Color constants in RGBA format
  static const uint32_t RED = 0xff0000ff;
  static const uint32_t GREEN = 0x00ff00ff;
  static const uint32_t BLUE = 0x0000ffff;
  static const uint32_t WHITE = 0xffffffff;

  // Add color changes
  // First color change: red/white, alternating colors
  {
    simData::DataStore::Transaction xaction;
    simData::LobGroupCommand* cmd = ds.addLobGroupCommand(lobID, &xaction);

    cmd->set_time(12.0);
    cmd->mutable_updateprefs()->set_color1(RED);
    cmd->mutable_updateprefs()->set_color2(WHITE);
    cmd->mutable_updateprefs()->set_stipple1(0xff00);
    cmd->mutable_updateprefs()->set_stipple2(0x00ff);
    xaction.complete(&cmd);
  }

  // Second color change: Blue/green, with a gap in the middle
  {
    simData::DataStore::Transaction xaction;
    simData::LobGroupCommand* cmd = ds.addLobGroupCommand(lobID, &xaction);

    cmd->set_time(24.0);
    cmd->mutable_updateprefs()->set_color1(BLUE);
    cmd->mutable_updateprefs()->set_color2(GREEN);
    cmd->mutable_updateprefs()->set_stipple1(0xf00f);
    cmd->mutable_updateprefs()->set_stipple2(0x00f0);
    xaction.complete(&cmd);
  }
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // Set up the data:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  simData::ObjectId platform2 = addPlatform(dataStore);

  // put platform 2 in motion
  simulatePlatform(platform2, dataStore, viewer.get());

  // make some lobs
  simData::ObjectId lobID = addLobGroup(platform2, dataStore);
  simulateLobGroup(lobID, dataStore);

  // tick the sim
  dataStore.update(0);

  // zoom the camera
  viewer->getMainView()->tetherCamera(scene->getScenario()->find(platform2));
  viewer->getMainView()->setFocalOffsets(0, -45, 4e5);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

