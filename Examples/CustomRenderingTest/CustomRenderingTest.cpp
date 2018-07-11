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
 * Custom Rendering Example
 *
 * Demonstrates how to inject a custom entity into a scene.
 */

#include "simCore/Common/Version.h"
#include "simCore/Time/ClockImpl.h"

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"

/// include definitions for objects of interest
#include "simVis/CustomRendering.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"

/// some basic components
#include "simVis/LabelContentManager.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;


namespace {

//----------------------------------------------------------------------------
/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "Custom Rendering Example";


//----------------------------------------------------------------------------

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore)
{
  /// all DataStore operations require a transaction (to avoid races)
  simData::DataStore::Transaction transaction;

  /// create the platform, and get the properties for it
  simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

  /// save the platform id for our return value
  simData::ObjectId result = newProps->id();

  /// done
  transaction.complete(&newProps);
  return result;
}

/// Sets up default prefs for a platform
void configurePlatformPrefs(simData::ObjectId platformId, simData::DataStore* dataStore, const std::string& name)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformPrefs* prefs = dataStore->mutable_platformPrefs(platformId, &xaction);

  prefs->mutable_commonprefs()->set_name(name);
  prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs->set_scale(3.0f);
  prefs->set_dynamicscale(true);
  prefs->set_circlehilightcolor(0xffffffff);

  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);

  xaction.complete(&prefs);
}

//----------------------------------------------------------------------------


/// create a Custom Entity and add it to 'dataStore'
///@return id for new Custom Entity
simData::ObjectId addCustomRendering(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  simData::DataStore::Transaction transaction;

  simData::CustomRenderingProperties *customProps = dataStore.addCustomRendering(&transaction);
  simData::ObjectId result = customProps->id();
  customProps->set_hostid(hostId);
  transaction.complete(&customProps);

  simData::CustomRenderingPrefs *prefs = dataStore.mutable_customRenderingPrefs(result, &transaction);
  prefs->mutable_commonprefs()->set_name("Custom Entity");
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(200);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(0xFF0000FF);

  transaction.complete(&prefs);

  return result;
}

class LabelCallback : public simVis::NullEntityCallback
{
public:
  LabelCallback() {}
  virtual ~LabelCallback() {}

  virtual std::string createString(const simData::CustomRenderingPrefs& prefs, const simData::LabelPrefs_DisplayFields& fields)
  {
    return "Do something special here.";
  }
};

/// Handles the datastore update from the CustomRenderingNode
class UpdateFromDatastore : public simVis::CustomRenderingNode::UpdateCallback
{
public:
  explicit UpdateFromDatastore(simVis::ScenarioManager* manager)
    : manager_(manager)
  {
  }

  virtual bool update(const simData::DataSliceBase* updateSlice, bool force = false)
  {
    if (node_ == NULL)
      return false;

    if (geom_ == NULL)
    {
      simVis::LocatorNode* locatorNode = node_->locatorNode();
      locatorNode->removeChildren(0, locatorNode->getNumChildren());

      osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
      (*colors)[0] = osg::Vec4(1, 1, 1, 1);
      geom_ = new osg::Geometry;
      geom_->setColorArray(colors.get());

      geom_->setUseVertexBufferObjects(true);
      fillVerts_ = new osg::Vec3Array();
      fillVerts_->setDataVariance(osg::Object::DYNAMIC);
      geom_->setVertexArray(fillVerts_.get());
      fillVerts_->push_back(osg::Vec3(1000, -100, 0));
      fillVerts_->push_back(osg::Vec3(1000, 0, 0));
      fillVerts_->push_back(osg::Vec3(0, 0, 0));
      geom_->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 3));
      locatorNode->addChild(geom_);
      locatorNode->dirtyBound();
    }
    else
    {
      if (fillVerts_->front().y() > -1000)
        fillVerts_->front().y() -= 100;
      else
        fillVerts_->front().y() = -100;
      fillVerts_->dirty();
    }

    auto host = node_->host();
    if (host != NULL)
    {
      simCore::Coordinate coord;
      host->getLocator()->getCoordinate(&coord);
      node_->getLocator()->setCoordinate(coord);
      node_->dirtyBound();
    }

    return true;
  }

  void setId(simData::ObjectId id)
  {
    node_ = manager_->find<simVis::CustomRenderingNode>(id);
  }

protected:
  virtual ~UpdateFromDatastore() {}

private:
  osg::observer_ptr<simVis::ScenarioManager> manager_;
  osg::observer_ptr<simVis::CustomRenderingNode> node_;
  osg::ref_ptr<osg::Geometry> geom_;
  osg::ref_ptr<osg::Vec3Array> fillVerts_;;
};


// Code similar to what a extension will do; hard-coded to one Custom Entity
class UpdateGraphics : public simData::DataStore::DefaultListener
{
public:
  explicit UpdateGraphics(simVis::ScenarioManager* manager)
    : manager_(manager),
      callback_(new LabelCallback),
      update_(new UpdateFromDatastore(manager))
  {
  }

  virtual ~UpdateGraphics()
  {
  }

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    if (manager_ == NULL)
      return;

    if (ot == simData::CUSTOM_RENDERING)
    {
      auto node = manager_->find<simVis::CustomRenderingNode>(newId);
      if (node != NULL)
      {
        update_->setId(newId);
        node->setUpdateCallback(update_.get());
        node->setLabelContentCallback(callback_.get());
        node->setCustomActive(true);
      }
    }
  }

  virtual void onTimeChange(simData::DataStore *source)
  {
  }

private:
  osg::observer_ptr<simVis::ScenarioManager> manager_;
  osg::ref_ptr<LabelCallback> callback_;
  osg::ref_ptr<UpdateFromDatastore> update_;
};


}
//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;

  scene->getScenario()->bind(&dataStore);

  /// create a clock so clock-based features will work (e.g. EphemerisVector)
  simCore::Clock* clock = new simCore::ClockImpl;
  simVis::Registry::instance()->setClock(clock);
  clock->setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1970, simCore::getSystemTime()));

  /// add in the platform and beam
  simData::ObjectId platformId = addPlatform(dataStore);
  configurePlatformPrefs(platformId, &dataStore, "Simulated Platform");

// Custom specific code
  dataStore.addListener(simData::DataStore::ListenerPtr(new UpdateGraphics(scene->getScenario())));
  addCustomRendering(platformId, dataStore);

  /// simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  /// create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC
  sim->addWaypoint(simUtil::Waypoint(-33.4, -70.8, 30000, 200.0)); // Santiago
  sim->addWaypoint(simUtil::Waypoint(-34.0,  18.5, 30000, 200.0)); // Capetown

  sim->setSimulateRoll(true);
  sim->setSimulatePitch(false);

  /// Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);

  /// Start the simulation
  simMgr->addSimulator(sim.get());
  simMgr->simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simMgr.get(), 0.0, 120.0);
  viewer->addEventHandler(simHandler.get());

  /// Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 400);

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}
