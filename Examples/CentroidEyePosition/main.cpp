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
 * Centroid Eye Position Example
 *
 * Demonstrates the use of a simVis::AveragePositionNode
 * to center the view on a collection of entities.
 */

#include "osg/Depth"
#include "osg/ShapeDrawable"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simVis/AveragePositionNode.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

using namespace osgEarth::Util::Controls;

static const simCore::Coordinate DEFAULT_POS_LLA(simCore::COORD_SYS_LLA,
  simCore::Vec3(simCore::DEG2RAD*(0), simCore::DEG2RAD*(0), 0),
  simCore::Vec3(0.0, 0.0, 0.0));

static const osg::Vec4f GREEN(0.0, 0.8, 0.0, 1.0);

//----------------------------------------------------------------------------

struct App
{
  osg::ref_ptr<simVis::Viewer> viewer;
  osg::ref_ptr<simVis::View> mainView;
  osg::ref_ptr<Control> helpBox;
  osg::ref_ptr<simVis::AveragePositionNode> centroidNode;
  osg::ref_ptr<osg::MatrixTransform> sphereXform;
  simData::DataStore* dataStore;
};

/** Update callback that updates the sphere transform's scale on each update cycle */
class UpdateScaleCallback : public osg::Callback
{
public:
  explicit UpdateScaleCallback(const App& app)
    : app_(app)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    // update the sphere transform
    const double r = app_.centroidNode->boundingSphereRadius();
    app_.sphereXform->setMatrix(osg::Matrix::scale(r, r, r));

    return traverse(object, data);
  }

private:
  const App& app_;
};

/** Event handler to process user key presses */
struct MenuHandler : public osgGA::GUIEventHandler
{
  MenuHandler(App& app)
    : app_(app),
    tracking4_(false)
  {
  }

  /// Callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case '?': // Toggle help
        app_.helpBox->setVisible(!app_.helpBox->visible());
        return true;

      case 'c': // Center on centroid node
      {
        auto vp = app_.mainView->getViewpoint();
        vp.setNode(app_.centroidNode.get());
        app_.mainView->setViewpoint(vp);
        return true;
      }

      case 'o': // Toggle overhead mode
        app_.mainView->enableOverheadMode(!app_.mainView->isOverheadEnabled());
        return true;

      case '1':
        toggleTrackNode_(1);
        return true;
      case '2':
        toggleTrackNode_(2);
        return true;
      case '3':
        toggleTrackNode_(3);
        return true;
      case '4':
        toggleTrackNode_(4);
        return true;
      case '5':
        toggleTrackNode_(5);
        return true;
      case '6':
        toggleTrackNode_(6);
        return true;
      }
    }

    return false;
  }

private: // data

  void toggleTrackNode_(simData::ObjectId id)
  {
    uint32_t color;
    osg::observer_ptr<simVis::EntityNode> objNode = app_.viewer->getSceneManager()->getScenario()->find(id);
    if (app_.centroidNode->isTrackingNode(objNode.get()))
    {
      app_.centroidNode->removeTrackedNode(objNode.get());
      color = 0xFFFFFFFF;
    }
    else
    {
      app_.centroidNode->addTrackedNode(objNode.get());
      color = GREEN.asABGR();
    }
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = app_.dataStore->mutable_platformPrefs(id, &txn);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(color);
    txn.complete(&prefs);
  }

  App& app_;
  bool tracking4_;
};

//----------------------------------------------------------------------------

Control* createControls(App& app)
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->setMargin(10);
  vbox->setVertAlign(Control::ALIGN_BOTTOM);

  vbox->addControl(new LabelControl("Centroid Eye Position Example", 20, simVis::Color::Yellow));
  vbox->addControl(new LabelControl("Green labels are tracked, while", 14.f, GREEN));
  vbox->addControl(new LabelControl("white labels are not tracked", 14.f, simVis::Color::White));
  vbox->addControl(new LabelControl("c: Center camera on centroid", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("o: Toggle overhead mode", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("--------------------------------", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("1: Toggle Tracking of Platform 1", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("2: Toggle Tracking of Platform 2", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("3: Toggle Tracking of Platform 3", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("4: Toggle Tracking of Platform 4", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("5: Toggle Tracking of Platform 5", 14.f, simVis::Color::Silver));
  vbox->addControl(new LabelControl("6: Toggle Tracking of Platform 6", 14.f, simVis::Color::Silver));

  app.helpBox = vbox;
  return vbox;
}

void initializeDrawables(App& app)
{
  // Create a sphere that will represent the bounding sphere
  osg::Geode* geode = new osg::Geode();
  osg::ShapeDrawable* sphere = new osg::ShapeDrawable(new osg::Sphere());
  sphere->setColor(osg::Vec4(0.0, 1.0, 0.0, 0.3)); // Green
  geode->addDrawable(sphere);

  // Turn on blending and lighting, turn off back-face culling and depth writes
  osg::StateSet* ss = sphere->getOrCreateStateSet();
  ss->setMode(GL_BLEND, osg::StateAttribute::ON);
  ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
  ss->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false), osg::StateAttribute::ON);
  simVis::setLighting(ss, osg::StateAttribute::ON);

  // Create a matrix transform for the sphere
  osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
  xform->addChild(geode);
  xform->addUpdateCallback(new UpdateScaleCallback(app));
  app.sphereXform = xform;
  // Add the transform to the centroid node
  app.centroidNode->addChild(xform);

  // Draw a red dot at the center of the sphere
  osg::ShapeDrawable* centerDot = new osg::ShapeDrawable(new osg::Sphere());
  centerDot->setColor(osg::Vec4(1.0, 0.0, 0.0, 1.0));
  app.centroidNode->addChild(centerDot);
}

/// Simulates a ship platform traveling back and forth
simUtil::SimulatorEventHandler* createShipSim(simUtil::PlatformSimulatorManager& simMgr, simData::ObjectId id)
{
  osg::ref_ptr<simUtil::PlatformSimulator> sim1 = new simUtil::PlatformSimulator(id);
  sim1->addWaypoint(simUtil::Waypoint(0.001, -0.005, 1.0, 30.0));
  sim1->addWaypoint(simUtil::Waypoint(0.001, 0.005, 1.0, 30.0));
  sim1->setSimulateRoll(false);
  sim1->setSimulatePitch(false);
  simMgr.addSimulator(sim1.get());

  return new simUtil::SimulatorEventHandler(&simMgr, 0.0, 60.0);
}

/// Simulates an air platform flying overhead
simUtil::SimulatorEventHandler* createAirSim(simUtil::PlatformSimulatorManager& simMgr, simData::ObjectId id)
{
  osg::ref_ptr<simUtil::PlatformSimulator> sim1 = new simUtil::PlatformSimulator(id);
  sim1->addWaypoint(simUtil::Waypoint(0.002, 0.0012, 300.0, 10.0));
  sim1->addWaypoint(simUtil::Waypoint(0.0, 0.002, 300.0, 10.0));
  sim1->addWaypoint(simUtil::Waypoint(-0.002, 0.0012, 300.0, 10.0));
  sim1->addWaypoint(simUtil::Waypoint(-0.002, -0.0012, 300.0, 10.0));
  sim1->addWaypoint(simUtil::Waypoint(0.0, -0.002, 300.0, 10.0));
  sim1->addWaypoint(simUtil::Waypoint(0.002, -0.0012, 300.0, 10.0));
  sim1->setSimulateRoll(false);
  sim1->setSimulatePitch(false);
  simMgr.addSimulator(sim1.get());

  return new simUtil::SimulatorEventHandler(&simMgr, 0.0, 60.0);
}

simData::ObjectId createPlatform(simData::DataStore& dataStore, bool ship = false)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
  simData::ObjectId id = props->id();
  xaction.complete(&props);

  // Set up and apply preferences for the platform
  simData::DataStore::Transaction txn;
  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->set_dynamicscale(true);
  prefs->set_nodepthicons(false);
  prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_LINE);
  prefs->mutable_trackprefs()->set_linewidth(2.0);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(0xFFFFFFFF);
  std::stringstream ss;
  ss << "Platform " << id;
  prefs->mutable_commonprefs()->set_name(ss.str());
  prefs->set_icon(ship ? EXAMPLE_SHIP_ICON : EXAMPLE_AIRPLANE_ICON);
  txn.complete(&prefs);

  return id;
}

void setPlatformPosition(simData::DataStore& dataStore, simData::ObjectId id, const simCore::Vec3& off)
{
  // Convert to default position to ECEF
  simCore::Coordinate ecef;
  simCore::CoordinateConverter::convertGeodeticToEcef(DEFAULT_POS_LLA, ecef);

  simData::DataStore::Transaction txn;
  simData::PlatformUpdate* newUpdate = dataStore.addPlatformUpdate(id, &txn);
  // Apply offsets to the default position
  newUpdate->set_x(ecef.x() + off.x());
  newUpdate->set_y(ecef.y() + off.y());
  newUpdate->set_z(ecef.z() + off.z());
  newUpdate->set_psi(ecef.psi());
  newUpdate->set_theta(ecef.theta());
  newUpdate->set_phi(ecef.phi());
  newUpdate->set_time(-1.0);
  txn.complete(&newUpdate);
}

/// Track the node with the specified id
void trackNode(const App& app, simData::ObjectId id)
{
  osg::observer_ptr<simVis::EntityNode> objNode = app.viewer->getSceneManager()->getScenario()->find(id);
  if (!objNode.valid())
    return;
  app.centroidNode->addTrackedNode(objNode.get());

  simData::DataStore::Transaction txn;
  simData::PlatformPrefs* prefs = app.dataStore->mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(GREEN.asABGR());
  txn.complete(&prefs);
}

/// Untrack the node with the specified id
void untrackNode(const App& app, simData::ObjectId id)
{
  osg::observer_ptr<simVis::EntityNode> objNode = app.viewer->getSceneManager()->getScenario()->find(id);
  if (!objNode.valid())
    return;
  app.centroidNode->removeTrackedNode(objNode.get());

  simData::DataStore::Transaction txn;
  simData::PlatformPrefs* prefs = app.dataStore->mutable_platformPrefs(id, &txn);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_color(0xFFFFFFFF);
  txn.complete(&prefs);
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  // Set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // Use the utility code to create a basic world map (terrain imagery and height)
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // SDK viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(simVis::Viewer::WINDOWED, 200, 100, 1024, 768);
  viewer->getViewer()->setThreadingModel(osgViewer::ViewerBase::ThreadingModel::SingleThreaded);
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // Add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Set up data store and app
  simData::MemoryDataStore dataStore;
  App app;
  app.dataStore = &dataStore;
  app.viewer = viewer;

  // Bind dataStore to the scenario manager
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&dataStore);

  // Create our centroid node
  osg::ref_ptr<simVis::AveragePositionNode> centroidNode = new simVis::AveragePositionNode();
  app.centroidNode = centroidNode;

  // Create drawables around the centroid node
  initializeDrawables(app);

  // Add centroid node to the scene
  osg::ref_ptr<osg::Group> attachPoint = viewer->getSceneManager()->getOrCreateAttachPoint("centroidNodeAttach");
  attachPoint->addChild(centroidNode);

  // Create some platforms
  simData::ObjectId id1 = createPlatform(dataStore, true);
  setPlatformPosition(*app.dataStore, id1, simCore::Vec3());
  simData::ObjectId id2 = createPlatform(dataStore, true);
  setPlatformPosition(*app.dataStore, id2, simCore::Vec3(0.0, 200.0, -20.0));
  simData::ObjectId id3 = createPlatform(dataStore, true);
  setPlatformPosition(*app.dataStore, id3, simCore::Vec3(0.0, 16.0, 300.0));
  simData::ObjectId id4 = createPlatform(dataStore);
  setPlatformPosition(*app.dataStore, id4, simCore::Vec3(150.0, -65.0, -90.0));

  // Set up a simulation for a moving air platform
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  simData::ObjectId id5 = createPlatform(dataStore);
  viewer->addEventHandler(createAirSim(*simMgr, id5));

  // Set up a simulation for a moving ship platform
  simData::ObjectId id6 = createPlatform(dataStore, true);
  viewer->addEventHandler(createShipSim(*simMgr, id6));

  // Run the simulation
  simMgr->simulate(0.0, 60.0, 60.0);

  // Track a few of the platforms by default
  trackNode(app, id1);
  trackNode(app, id2);
  trackNode(app, id4);

  // Set the camera to look at the centroid
  app.mainView = viewer->getMainView();
  auto vp = app.mainView->getViewpoint();
  vp.setNode(centroidNode.get());
  app.mainView->setViewpoint(vp);
  app.mainView->setFocalOffsets(270, -20, 650);

  // Handle key press events
  viewer->addEventHandler(new MenuHandler(app));

  // Show the controls overlay
  app.mainView->addOverlayControl(createControls(app));

  // Add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}
