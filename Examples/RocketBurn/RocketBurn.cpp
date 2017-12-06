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
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Platform.h"
#include "simVis/RocketBurnStorage.h"
#include "simVis/VaporTrailStorage.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

namespace
{
/// get time and platform removal notifications
class TimeListener : public simData::DataStore::DefaultListener
{
public:
  TimeListener(simVis::RocketBurnStorage& rbStorage, simVis::VaporTrailStorage& vtStorage)
    : rbStorage_(rbStorage),
    vtStorage_(vtStorage)
  {
  }

  /// current time has been changed
  virtual void onTimeChange(simData::DataStore *source)
  {
    rbStorage_.update(source->updateTime());
    vtStorage_.update(source->updateTime());
  }

private:
  simVis::RocketBurnStorage& rbStorage_;
  simVis::VaporTrailStorage& vtStorage_;
};

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore, simVis::ScenarioManager &scenarioManager)
{
  // all DataStore operations require a transaction (to avoid races)
  simData::DataStore::Transaction transaction;

  // create the platform, and get the properties for it
  simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

  // save the platform id for our return value
  const simData::ObjectId platformId = newProps->id();

  // commit the new platform
  transaction.complete(&newProps);

  // pull its prefs
  simData::PlatformPrefs *prefs = dataStore.mutable_platformPrefs(platformId, &transaction);
  prefs->mutable_commonprefs()->set_name("Simulated Platform");
  prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);
  prefs->mutable_commonprefs()->set_datalimitpoints(600);
  prefs->mutable_commonprefs()->set_datalimittime(120.0);
  prefs->set_dynamicscale(true);
  prefs->set_scale(5);
  transaction.complete(&prefs);

  return platformId;
}

simUtil::PlatformSimulator* addSimulatedData(simData::ObjectId platformId)
{
  // simulator will compute time-based updates for our platform (and any beams it is hosting)
  simUtil::PlatformSimulator *sim = new simUtil::PlatformSimulator(platformId);
  const float fiveHours = 5 * 3600.0f;

  // create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(51.5, 0.0, 30000, fiveHours)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, fiveHours)); // DC
  sim->addWaypoint(simUtil::Waypoint(-33.4, -70.8, 30000, fiveHours)); // Santiago
  sim->addWaypoint(simUtil::Waypoint(-34.0, 18.5, 30000, fiveHours)); // Capetown

  sim->setSimulateRoll(true);
  sim->setSimulatePitch(false);

  return sim;
}

void addRocketBurnData(simVis::RocketBurnStorage &rocketBurnStorage, simData::ObjectId platId, float platformLength)
{
  simVis::RocketBurnStorage::Update updateData;

  // some initial data
  simVis::RocketBurn::ShapeData &rocketBurnShape = updateData.shapeData;
  rocketBurnShape.radiusFar = 0.001;
  rocketBurnShape.radiusNear = 1;
  rocketBurnShape.length = 10;
  rocketBurnShape.scaleAlpha = true;

  updateData.positionOffset = simCore::Vec3(0, -platformLength, 0);

  const uint64_t burnId = 0;

  // time 0
  rocketBurnStorage.addBurnData(platId, burnId, 0, updateData);

  // time 5 - reverse shape
  rocketBurnShape.radiusFar = 1;
  rocketBurnShape.radiusNear = 0.001;
  rocketBurnStorage.addBurnData(platId, burnId, 5, updateData);

  // time 10 - change color and direction
  rocketBurnShape.color = osg::Vec4(1, 0, 0, 1);
  updateData.pointingAngle = simCore::Vec3(1.57, 0, 0);
  rocketBurnStorage.addBurnData(platId, burnId, 10, updateData);

  // time 15 - change length
  rocketBurnShape.length = 20;
  updateData.pointingAngle = simCore::Vec3(-1.57, 0, 0);
  rocketBurnStorage.addBurnData(platId, burnId, 15, updateData);
}

void addVaporTrail(simVis::VaporTrailStorage &storage, simData::ObjectId platId)
{
  std::vector< osg::ref_ptr<osg::Texture2D> > textures;
  const std::string texture1 = "p.rgb";

  const std::string foundFile = simVis::Registry::instance()->findModelFile(texture1);
  if (foundFile.empty())
  {
    SIM_WARN << "Failed to find specified texture " << texture1 << "'." << std::endl;
    return;
  }
  else
  {
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
    texture->setImage(osgDB::readImageFile(foundFile));
    textures.push_back(texture);
  }

  unsigned int vaporTrailId = 0;
  simVis::VaporTrail::VaporTrailData vaporTrailData;
  simVis::VaporTrail::VaporPuffData vaporPuffData;
  vaporTrailData.startTime = 16.0;
  vaporTrailData.endTime = 29.0;
  vaporTrailData.metersBehindCurrentPosition = 15.0;
  vaporTrailData.numRadiiFromPreviousSmoke = 1.2;
  vaporPuffData.initialRadiusM = 8.0;
  vaporPuffData.radiusExpansionRate = 10.0;
  storage.addVaporTrail(platId, vaporTrailId, vaporTrailData, vaporPuffData, textures);
}
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // data source which will provide positions for the platform
  // based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  simVis::RocketBurnStorage rocketBurnStorage(dataStore, *scene->getScenario());
  simVis::VaporTrailStorage vaporTrailStorage(dataStore, *scene->getScenario());
  dataStore.addListener(simData::DataStore::ListenerPtr(new TimeListener(rocketBurnStorage, vaporTrailStorage)));

  // add a platform
  simData::ObjectId platformId = addPlatform(dataStore, *scene->getScenario());
  osg::ref_ptr<simVis::PlatformNode> platformNode = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  addRocketBurnData(rocketBurnStorage, platformId, platformNode->getActualSize().yMax());
  addVaporTrail(vaporTrailStorage, platformId);
  osg::ref_ptr<simUtil::PlatformSimulator> sim = addSimulatedData(platformId);

  // Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  const double endTime = 30.0;
  simMgr->addSimulator(sim.get());
  simMgr->simulate(0.0, endTime, 60.0);

  // Attach the simulation updater to OSG timer events
  osg::ref_ptr<simVis::SimulatorEventHandler> simHandler = new simVis::SimulatorEventHandler(simMgr.get(), 0.0, endTime);
  viewer->addEventHandler(simHandler.get());

  // Tether camera to platform
  viewer->getMainView()->tetherCamera(platformNode.get());

  // set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 130);

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  // turn control over to viewer
  return viewer->run();
}

