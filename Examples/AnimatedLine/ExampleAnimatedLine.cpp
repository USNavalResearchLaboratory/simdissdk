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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

/**
 * Animated Line Example.
 *
 * Demonstrates simVis animated lines. An animated line is a geometric platform-relative
 * line that shows a stipple pattern that can optionally animated over time, giving the
 * appearance of the stipple "moving" along the line. It is useful for indicating a
 * directional relationship such as a communication link.
 */

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"

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
#include "simVis/AnimatedLine.h"
#include "simVis/GOG/Annotation.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth;
using namespace osgEarth::Util;

//----------------------------------------------------------------------------

// first line, describe the program
static const std::string s_title = "Animated Line Example";

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const std::string& entityName, double lat, double lon, double alt, bool stationary)
{
  // create the platform:
  simData::ObjectId platformId;
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties *newProps = dataStore.addPlatform(&xaction);
    platformId = newProps->id();
    xaction.complete(&newProps);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->set_name(entityName);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // now place it somewhere
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformUpdate* update = dataStore.addPlatformUpdate(platformId, &xaction);

    simCore::Coordinate lla(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat, simCore::DEG2RAD * lon, alt),
      simCore::Vec3(0, 0, 0));

    simCore::CoordinateConverter conv;
    simCore::Coordinate ecef;
    conv.convert(lla, ecef, simCore::COORD_SYS_ECEF);

    update->set_time(0.0);
    update->set_x(ecef.x());
    update->set_y(ecef.y());
    update->set_z(ecef.z());
    update->set_psi(ecef.psi());
    update->set_theta(ecef.theta());
    update->set_phi(ecef.phi());

    xaction.complete(&update);

    if (stationary)
    {
      update = dataStore.addPlatformUpdate(platformId, &xaction);
      update->set_time(30.0);
      update->set_x(ecef.x());
      update->set_y(ecef.y());
      update->set_z(ecef.z());
      update->set_psi(ecef.psi());
      update->set_theta(ecef.theta());
      update->set_phi(ecef.phi());
      xaction.complete(&update);
    }
  }

  return platformId;
}

//----------------------------------------------------------------------------

void simulate(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.5, -0.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0.5,  0.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim.get());
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0);
  viewer->addEventHandler(simHandler.get());
}

//----------------------------------------------------------------------------

void addAnimatedLines(simVis::EntityNode* node1, simVis::EntityNode* node2, osg::Group* parent, MapNode* mapNode)
{
  // platform to platform:
  if (node1 && node2)
  {
    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(node1->getLocator(), node2->getLocator());
    parent->addChild(line);
  }

  // platform to fixed world coordinate:
  if (node1 && node2)
  {
    simCore::Coordinate coord(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD* -0.0, simCore::DEG2RAD* -1.0, 10000.));

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(node2->getLocator(), coord);
    line->setColor1(simVis::Color::Red);
    line->setColor2(simVis::Color::Yellow);
    line->setShiftsPerSecond(40.0);      // speed

    parent->addChild(line);
  }

  // fixed world coordinate to fixed world coordinate:
  if (!node1 && !node2)
  {
    simCore::Coordinate coord1(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD* -0.5, simCore::DEG2RAD* -1.0, 10000.));

    simCore::Coordinate coord2(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD* -0.5, simCore::DEG2RAD* 1.0, 7000.));

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(coord1, coord2);
    line->setColor1(simVis::Color::Orange); // orange
    line->setColor2(osg::Vec4());   // transparent
    line->setShiftsPerSecond(-30.0);       // negative speed reverses the direction.

    parent->addChild(line);
  }

  // local offset locator-to-locator with an orientation and a translation:
  if (node1 && node2)
  {
    osg::ref_ptr<simVis::Locator> lob = new simVis::Locator(
      node2->getLocator(),
      simVis::Locator::COMP_POSITION);

    lob->setLocalOffsets(
      simCore::Vec3(),
      simCore::Vec3(0, 22.5*simCore::DEG2RAD, 0));

    osg::ref_ptr<simVis::Locator> lob2 = new simVis::Locator(lob.get());

    lob2->setLocalOffsets(
      simCore::Vec3(100000, 0, 0),
      simCore::Vec3());

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(node2->getLocator(), lob2.get());
    line->setStipple1(0xF0F0);
    line->setStipple2(0x0F00);
    line->setColor1(simVis::Color::Lime);
    line->setColor2(simVis::Color::White);

    parent->addChild(line);
  }


  // fixed coordinate with a line of bearing
  if (!node1 && !node2)
  {
    simCore::Coordinate fixedCoord(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD* -0.25, simCore::DEG2RAD* 0.25, 10000.));

    simCore::Coordinate bearingLine(
      simCore::COORD_SYS_XEAST,
      simCore::Vec3(50000., -50000., -10000.));

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(fixedCoord, bearingLine);
    line->setColor1(simVis::Color::Aqua);
    line->setColor2(simVis::Color::Red);
    line->setStipple1(0xF0F0);
    line->setStipple2(0x0F0F);

    parent->addChild(line);
  }

  // platform to fixed world coordinate 2:
  if (node1 && node2)
  {
    simCore::Coordinate bearingLine(
      simCore::COORD_SYS_XEAST,
      simCore::Vec3(1000000., -1000000., 0));

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode();
    line->setEndPoints(node2->getLocator(), bearingLine);
    line->setColor1(simVis::Color::Red);
    line->setColor2(simVis::Color::Yellow);
    line->setShiftsPerSecond(40.0);      // speed

    parent->addChild(line);
  }

  // Over the horizon animated line
  if (!node1 && !node2)
  {
    simCore::Coordinate coord1(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(0, 0, 10000.));

    simCore::Coordinate coord2(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD* 37, simCore::DEG2RAD* -78, 10000.));

    osg::ref_ptr<simVis::AnimatedLineNode> line = new simVis::AnimatedLineNode(2.5f);
    line->setEndPoints(coord1, coord2);
    line->setColor1(simVis::Color::Fuchsia);
    line->setColor2(osg::Vec4());   // transparent
    line->setStipple1(0xf0ff);
    line->setStipple2(0x0);
    line->setShiftsPerSecond(20.0);

    parent->addChild(line);
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

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  simData::ObjectId platform1 = addPlatform(dataStore, "Platform 1", 0.1, 0, 20000, true);
  simData::ObjectId platform2 = addPlatform(dataStore, "Platform 2", 0, -0.5, 20000, false);

  simData::ObjectId surf1 = addPlatform(dataStore, "Surface 1", 0.45, 0.3, 0.0, true);
  simData::ObjectId surf2 = addPlatform(dataStore, "Surface 2", -0.75, -0.6, 0.0, true);

  // put platform 2 in motion
  simulate(platform2, dataStore, viewer.get());

  // Look up the platform models:
  osg::observer_ptr<simVis::EntityNode> node1 = scene->getScenario()->find(platform1);
  osg::observer_ptr<simVis::EntityNode> node2 = scene->getScenario()->find(platform2);
  osg::observer_ptr<simVis::EntityNode> node3 = scene->getScenario()->find(surf1);
  osg::observer_ptr<simVis::EntityNode> node4 = scene->getScenario()->find(surf2);

  // Make the lines:
  addAnimatedLines(node1.get(), node2.get(), scene->getScenario(), scene->getMapNode());
  addAnimatedLines(node3.get(), node4.get(), scene->getScenario(), scene->getMapNode());
  addAnimatedLines(nullptr, nullptr, scene->getScenario(), scene->getMapNode());

  // tick the sim
  dataStore.update(0);

  // zoom the camera
  viewer->getMainView()->tetherCamera(node1.get());
  viewer->getMainView()->setFocalOffsets(0, -45, 8e5);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
