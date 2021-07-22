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
 * Custom Rendering Example
 *
 * Demonstrates how to inject a custom entity into a scene.  This example
 * creates a unit circle and scales the size by a counter.  The unit
 * circle could represent an error ellipse.
 */

#include "simCore/Common/Version.h"
#include "simCore/Time/ClockImpl.h"

// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"

// include definitions for objects of interest
#include "simVis/CustomRendering.h"
#include "simVis/Locator.h"
#include "simVis/LocatorNode.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"

// some basic components
#include "simVis/LabelContentManager.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"

// paths to models
#include "simUtil/ExampleResources.h"

namespace {

/// String name for the Custom Rendering "Renderer" property
static const std::string RENDERER_NAME = "example_custom_render";

//----------------------------------------------------------------------------

/** Creates a new platform in the data store provided. */
simData::ObjectId addPlatform(simData::DataStore &dataStore, const std::string& name)
{
  // all DataStore operations require a transaction (to avoid races)
  simData::DataStore::Transaction transaction;

  // create the platform, and get the properties for it
  simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);
  // save the platform id for our return value
  const simData::ObjectId platformId = newProps->id();
  // done with properties
  transaction.complete(&newProps);

  // Configure the prefs next
  simData::DataStore::Transaction xaction;
  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);

  prefs->mutable_commonprefs()->set_name(name);
  prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs->set_scale(3.0f);
  prefs->set_dynamicscale(true);
  prefs->set_circlehilightcolor(0xffffffff);
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);
  // Completing the transaction writes out the prefs to the data store
  xaction.complete(&prefs);

  return platformId;
}

/** Creates a custom rendering entity and add it to the data store. */
simData::ObjectId addCustomRendering(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  simData::DataStore::Transaction transaction;

  simData::CustomRenderingProperties *customProps = dataStore.addCustomRendering(&transaction);
  simData::ObjectId result = customProps->id();
  customProps->set_hostid(hostId);

  // Set the renderer name.  By setting this, we can use it to discriminate which rendering
  // engine to use for the Custom Rendering entity.  This is very useful in cases where more
  // than one engine exists at a time in your application.
  customProps->set_renderer(RENDERER_NAME);

  transaction.complete(&customProps);

  simData::CustomRenderingPrefs *prefs = dataStore.mutable_customRenderingPrefs(result, &transaction);
  prefs->mutable_commonprefs()->set_name("Custom Entity");
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(200);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(0xFFFF00FF);

  transaction.complete(&prefs);

  return result;
}

/**
 * Callback used to form the label contents.  This callback gets associated directly
 * with the Custom Rendering node by the AttachRenderGraphics code, which is the same
 * code that detects the presence of new Custom Rendering nodes in the scene.
 */
class LabelCallback : public simVis::NullEntityCallback
{
public:
  LabelCallback() {}

  /** Override the Custom Rendering version of the string to create a custom label. */
  virtual std::string createString(simData::ObjectId id, const simData::CustomRenderingPrefs& prefs, const simData::LabelPrefs_DisplayFields& fields)
  {
    // The default implementation for a Custom Rendering node's Entity Callback only shows
    // the name pref.  This implementation shows a custom string instead.  You can query
    // prefs to form a string representing the label for the entity in its current state
    // in this block of code.
    return "Create an application specific string here";
  }
};

/**
 * The UpdateCallback is the engine that drives the Custom Rendering routines.
 *
 * Although a single Custom Rendering node can only have a single UpdateCallback,
 * different Custom Rendering entities could have different instances of an
 * UpdateCallback.  For example, one engine might draw ellipses, and another
 * render engine might draw 3D Models or scene graph text.  In this code, it
 * is the responsibility of the AttachRenderGraphics (DataStore Listener) to
 * correctly identify the right engine to use.  The Custom Rendering property
 * "renderer" is provided to help with this decision.
 *
 * This rendering engine retains a single unit circle graphic and scales it
 * per frame to demonstrate change in the graphics, during update().  The color
 * of the line is automatically applied by the simVis::CustomRenderingNode,
 * using simVis::OverrideColor and relying on the CommonPrefs.color preference.
 * To use a different color, you'll need to either update the color preference
 * to white (so that it multiplies out to your incoming color value), or explicitly
 * disable the Override Color code in your stateset.
 */
class RenderEngine : public simVis::CustomRenderingNode::UpdateCallback
{
public:
  RenderEngine()
    : scale_(100.f, 100.f, 1.f)
  {
  }

  /**
   * This method is called automatically and regularly to update the Custom Rendering
   * entity.  This method is responsible for updating graphics to the latest data
   * for the entity.  A real example might pull data from the Data Store's data
   * tables for the entity and piece together either new graphics or modify existing
   * graphics that this Render Engine is maintaining.
   */
  virtual bool update(const simData::DataSliceBase* updateSlice, bool force = false)
  {
    // Break out if the node isn't currently valid
    if (!node_.valid())
      return false;

    // Create the geometry if it hasn't been created yet
    if (!transform_.valid())
    {
      simVis::LocatorNode* locatorNode = node_->locatorNode();
      locatorNode->removeChildren(0, locatorNode->getNumChildren());

      // In this example do a simple unit circle.  It gets scaled below.
      osg::Geometry* geom = makeUnitCircle_();
      transform_ = new osg::MatrixTransform;
      transform_->addChild(geom);
      locatorNode->addChild(transform_);
      node_->setCustomActive(true);
      locatorNode->dirtyBound();

      // Configure a render bin that is appropriate for opaque graphics.  It is the
      // responsibility of the engine to set an appropriate render bin for the graphics.
      // The following is a good guideline:
      //   Transparent:  setRenderBinDetails(simVis::BIN_CUSTOM_RENDER, simVis::BIN_TWO_PASS_ALPHA);
      //   Opaque:       setRenderBinDetails(simVis::BIN_OPAQUE_CUSTOM_RENDER, simVis::BIN_GLOBAL_SIMSDK);
      transform_->getOrCreateStateSet()->setRenderBinDetails(simVis::BIN_OPAQUE_CUSTOM_RENDER, simVis::BIN_GLOBAL_SIMSDK);
    }

    // In this example scale the size of the customer rendering.  It is possible
    // to change the color, shape, or any other property here.  Remember that
    // the color, by default, is pulled from the "color" preference on the entity
    // and is multiplied against the geometry's color.
    osg::Matrix matrix;
    matrix.makeScale(scale_);
    scale_.x() += 3.0;
    if (scale_.x() > 200.0)
      scale_.x() = 100;
    scale_.y() += 2.0;
    if (scale_.y() > 200.0)
      scale_.y() = 100;
    transform_->setMatrix(matrix);

    // Adjust the coordinates of the locator to match that of the host
    auto host = node_->host();
    if (host != nullptr)
    {
      simCore::Coordinate coord;
      host->getLocator()->getCoordinate(&coord);
      // In this example the custom rendering is tracking the host platform.  It is
      // possible to add offsets or to set a completely independent location.
      node_->getLocator()->setCoordinate(coord, host->getLocator()->getTime());
      node_->dirtyBound();
    }

    return true;
  }

  /** Configure the node that this engine is maintaining */
  void setNode(simVis::CustomRenderingNode* node)
  {
    node_ = node;
  }

private:
  /** Creates a line loop of a unit circle, for the rendering graphics */
  osg::Geometry* makeUnitCircle_() const
  {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    // Set a white color to the geometry so that the prefs.color value, when
    // multiplied against white, shows up exactly as white.  Changing this
    // color value will tint the prefs.color value.  If you want to render
    // graphics with custom colors, there are two main options:
    //  1) Set the prefs.color value to white, and alter the colors of your geometry.
    //  2) Turn off the simVis::OverrideColor shader in your stateset, and alter the
    //     colors of your geometry.
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colors)[0] = osg::Vec4(1, 1, 1, 1);
    geom->setColorArray(colors.get());

    // Build 4 quarters at once to reduce number of calls to sin() and cos()
    const int pointsPerQuarter = 20;
    osg::ref_ptr<osg::Vec3Array> fillVerts = new osg::Vec3Array(4 * pointsPerQuarter);
    for (auto ii = 0; ii < pointsPerQuarter; ++ii)
    {
      float arg = static_cast<float>(ii) / static_cast<float>(pointsPerQuarter)* static_cast<float>(M_PI_2);
      float x = static_cast<float>(cos(arg));
      float y = static_cast<float>(sin(arg));

      (*fillVerts)[ii].set(x, y, 0);
      (*fillVerts)[ii + pointsPerQuarter].set(-y, x, 0);
      (*fillVerts)[ii + 2 * pointsPerQuarter].set(-x, -y, 0);
      (*fillVerts)[ii + 3 * pointsPerQuarter].set(y, -x, 0);
    }

    // Set up the rest of the geometry
    geom->setVertexArray(fillVerts.get());
    geom->setDataVariance(osg::Object::DYNAMIC);
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, 0, 4 * pointsPerQuarter));
    return geom.release();
  }

  osg::observer_ptr<simVis::CustomRenderingNode> node_;
  osg::ref_ptr<osg::MatrixTransform> transform_;
  osg::Vec3f scale_;
};

/**
 * This DataStore Listener is a callback that checks new entities to see if they are
 * Custom Rendering entities.  If so, the Renderer above is associated with that class.
 * Without this, Custom Rendering entities would still exist, but no graphics would be
 * associated with them.
 *
 * The Listener should be cautious to only associate an Update Callback in appropriate
 * circumstances.  The property "renderer" is provided in the DataStore to let the user
 * supply a string to identify the appropriate UpdateCallback to use.  A well-behaved
 * Listener instance should check this property value before assigning an UpdateCallback.
 */
class AttachRenderGraphics : public simData::DataStore::DefaultListener
{
public:
  explicit AttachRenderGraphics(simVis::ScenarioManager* manager)
    : manager_(manager),
      callback_(new LabelCallback)
  {
  }

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    // Break out if not a custom rendering; we don't care about those entities here
    if (!manager_.valid() || ot != simData::CUSTOM_RENDERING)
      return;

    simData::DataStore::Transaction txn;
    const simData::CustomRenderingProperties* props = source->customRenderingProperties(newId, &txn);
    // Only attach to OUR custom rendering objects by comparing renderer engine names
    if (!props || props->renderer() != RENDERER_NAME)
      return;
    txn.complete(&props);

    // Pick out the node from the scene (created by the ScenarioDataStoreAdapter automatically)
    simVis::CustomRenderingNode* node = manager_->find<simVis::CustomRenderingNode>(newId);
    if (node != nullptr)
    {
      // A real render engine would need to account for multiple Custom Rendering nodes here,
      // either by creating a separate updater per entity, or configuring the updater to
      // correctly handle multiple entities.  Here, we create a new RenderEngine per
      // node.  While this works, it may not be the most efficient use of resources if
      // you plan on having many different entities with shared geometry.
      RenderEngine* updater = new RenderEngine();
      updater->setNode(node);
      node->setUpdateCallback(updater);
      node->setLabelContentCallback(callback_.get());
      node->setCustomActive(true);
    }
  }

private:
  osg::observer_ptr<simVis::ScenarioManager> manager_;
  osg::ref_ptr<LabelCallback> callback_;
};

}

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // SIMDIS viewer to display the scene
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

  // create a clock so clock-based features will work (e.g. EphemerisVector)
  simCore::Clock* clock = new simCore::ClockImpl;
  simVis::Registry::instance()->setClock(clock);
  clock->setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1970, simCore::getSystemTime()));

  // add in the platform and beam
  simData::ObjectId platformId = addPlatform(dataStore, "Simulated Platform");

  // Add a listener to the Data Store.  This listener is responsible for
  // detecting newly created Custom Rendering entities and setting their
  // nodes up with label callbacks, update callbacks, and other minutia.
  dataStore.addListener(simData::DataStore::ListenerPtr(new AttachRenderGraphics(scene->getScenario())));

  // Add a Custom Rendering entity to the data store.  This will trigger the
  // AttachRenderGraphics DataStore Listener to detect a newly created entity,
  // triggering the creation of the rendering engine.
  addCustomRendering(platformId, dataStore);

  // simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  // create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC
  sim->addWaypoint(simUtil::Waypoint(-33.4, -70.8, 30000, 200.0)); // Santiago
  sim->addWaypoint(simUtil::Waypoint(-34.0,  18.5, 30000, 200.0)); // Capetown

  sim->setSimulateRoll(true);
  sim->setSimulatePitch(false);

  // Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);

  // Start the simulation
  simMgr->addSimulator(sim.get());
  simMgr->simulate(0.0, 120.0, 60.0);

  // Attach the simulation updater to OSG timer events
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simMgr.get(), 0.0, 120.0);
  viewer->addEventHandler(simHandler.get());

  // Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformNode.get());

  // set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 400);

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}
