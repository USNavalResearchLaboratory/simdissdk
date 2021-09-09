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
 * Projectors Example
 *
 * Demonstrates how to create and control the Projector object, which projects an image onto the terrain.
 */

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"
#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simCore/Calc/Angle.h"

/// include definitions for objects of interest
#include "simVis/Beam.h"
#include "simVis/Gate.h"
#include "simVis/LocalGrid.h"
#include "simVis/Platform.h"
#include "simVis/TrackHistory.h"
#include "simVis/Projector.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

/// include interpolator
#include "simData/LinearInterpolator.h"

#define LC "[Projectors] "

using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------
/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "Projectors Example";

/// later lines, document the keyboard controls
static const std::string s_help =
" ? : toggle help";
static const std::string s_rotate =
" t : rotate through textures";
static const std::string s_interpolate =
" i : toggle interpolation";
static const std::string s_viewPlatformOne =
" 1 : reset view on platform 1 (Constant FOV)";
static const std::string s_viewPlatformTwo =
" 2 : reset view on platform 2 (Varying FOV)";
static const std::string s_viewPlatformThree =
" 3 : reset view on platform 3 (Platform projection target)";
static const std::string s_viewPlatformFour =
" 4 : reset view on platform 4 (Stationary)";

/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl;

static Control* createHelp()
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new LabelControl(s_help, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_rotate, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_interpolate, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_viewPlatformOne, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_viewPlatformTwo, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_viewPlatformThree, 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl(s_viewPlatformFour, 14, simVis::Color::Silver));
  s_helpControl = vbox;
  return vbox;
}

/// global variables for camera tethering between platforms
simData::ObjectId platformId_0;
simData::ObjectId projectorId_0;
simData::ObjectId platformId_1;
simData::ObjectId projectorId_1;
simData::ObjectId platformId_2;
simData::ObjectId projectorId_3;
simData::ObjectId platformId_3;

//----------------------------------------------------------------------------
/// event handler for keyboard commands to alter symbology at runtime
struct MenuHandler : public osgGA::GUIEventHandler
{
  /// constructor grabs all the state it needs for updating
  MenuHandler(simData::DataStore& ds, simVis::View& view, const simData::ObjectId& projId, const std::string &initialImage) :
    dataStore_(ds),
    view_(view),
    projId_(projId),
    initialImage_(initialImage),
    counter_(0)
  {
  }

  /// apply the new raster file to the projector
  void setProjectorTexture(const std::string& filename)
  {
    simData::DataStore::Transaction txn;
    simData::ProjectorPrefs* prefs = dataStore_.mutable_projectorPrefs(projId_, &txn);
    if (prefs)
    {
      prefs->set_rasterfile(filename);
      txn.complete(&prefs);
    }
  }

  void toggleInterpolate()
  {
    simData::DataStore::Transaction txn;
    simData::ProjectorPrefs* prefs = dataStore_.mutable_projectorPrefs(projId_, &txn);
    if (prefs)
    {
      prefs->set_interpolateprojectorfov(!prefs->interpolateprojectorfov());
      txn.complete(&prefs);
    }
  }

  /// tether view to selected platform ID and corresponding projector and reset texture to initial image
  bool tetherView(simData::ObjectId tetherId, simData::ObjectId projectorId)
  {
    // Don't continue if we are using the same projector
    if (projId_ == projectorId)
      return true;

    simVis::PlatformNode* plat = view_.getSceneManager()->getScenario()->find<simVis::PlatformNode>(tetherId);
    if (!plat)
      return false;
    view_.tetherCamera(plat);
    projId_ = projectorId;
    setProjectorTexture(initialImage_);
    counter_ = 0;
    return true;
  }

  bool tetherView(simData::ObjectId tetherId)
  {
    simVis::PlatformNode* plat = view_.getSceneManager()->getScenario()->find<simVis::PlatformNode>(tetherId);
    if (!plat)
      return false;
    view_.tetherCamera(plat);
    counter_ = 0;
    return true;
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    /// only handle key down
    if (ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN)
      return false;

    bool handled = false;

    switch (ea.getKey())
    {
      case '?' : // toggle help
      {
        s_helpControl->setVisible(!s_helpControl->visible());
        handled = true;
        break;
      }
      case 't' : // cycle through textures
      {
        std::string inputFilename;

        counter_++;
        if (counter_ > 2)
          counter_ = 0;

        // Set the filename based on current counter
        switch (counter_)
        {
          case 0:
            inputFilename = initialImage_;
            break;
          case 1:
            inputFilename = "skullnbones.png";
            break;
          case 2:
            inputFilename = "junk.png";
            break;
        }

        setProjectorTexture(inputFilename);
        handled = true;
        break;
      }
      case '1':
        handled = tetherView(platformId_0, projectorId_0);
        break;
      case '2':
        handled = tetherView(platformId_1, projectorId_1);
        break;
      case '3':
        handled = tetherView(platformId_2);
        break;
      case '4':
        handled = tetherView(platformId_3);
        break;
      case 'i':
        toggleInterpolate();
        handled = true;
        break;
    }

    return handled;
  }

private: // data
  simData::DataStore& dataStore_;
  simVis::View& view_;
  simData::ObjectId projId_;
  std::string initialImage_;
  unsigned int counter_;
};

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

/// Set fov to change every 10 seconds to test interpolation
void varyProjectorFov(simData::ObjectId projectorId, simData::DataStore& dataStore)
{
  simData::DataStore::Transaction txn;
  simData::ProjectorUpdate* update;

  for (int i = 0; i <= 120; i += 10)
  {
    update = dataStore.addProjectorUpdate(projectorId, &txn);
    // Switch field of view every 10 seconds
    double fov = (i % 20 == 0) ? 20.0 : 100.0;
    update->set_time(i);
    update->set_fov(fov * simCore::DEG2RAD);
    txn.complete(&update);
  }
}

simData::ObjectId addProjector(simVis::ScenarioManager* scenario,
                               simData::ObjectId        hostId,
                               simData::DataStore&      dataStore,
                               const std::string&       imageURL,
                               bool                     varyFov)
{
  simData::DataStore::Transaction txn;
  simData::ProjectorProperties* projProps = dataStore.addProjector(&txn);
  projProps->set_hostid(hostId);
  simData::ObjectId id = projProps->id();
  txn.complete(&projProps);

  simData::ProjectorPrefs* prefs = dataStore.mutable_projectorPrefs(id, &txn);
  prefs->set_rasterfile(imageURL);
  prefs->set_showfrustum(true); // Set to false to remove line frustum
  prefs->set_projectoralpha(0.8f);
  txn.complete(&prefs);

  if (varyFov)
    varyProjectorFov(id, dataStore);
  else
  {
    simData::ProjectorUpdate* update = dataStore.addProjectorUpdate(id, &txn);
    txn.complete(&update);
  }

  return id;
}

/// create a gate and add it to 'dataStore'
///@return id for new gate
simData::ObjectId addGate(simData::ObjectId hostId, simData::DataStore &dataStore)
{
  simData::DataStore::Transaction transaction;

  simData::GateProperties *gateProps = dataStore.addGate(&transaction);
  simData::ObjectId gateId = gateProps->id();
  gateProps->set_hostid(hostId);
  transaction.complete(&gateProps);

  simData::GatePrefs* gatePrefs = dataStore.mutable_gatePrefs(gateId, &transaction);
  gatePrefs->set_gateazimuthoffset(osg::DegreesToRadians(0.0));
  gatePrefs->mutable_commonprefs()->set_color(0xffffff7f); //simVis::Color::White.as(simVis::Color::RGBA));
  gatePrefs->set_fillpattern(simData::GatePrefs_FillPattern_ALPHA);
  gatePrefs->set_gatedrawmode(simData::GatePrefs_DrawMode_ANGLE);
  gatePrefs->set_gatelighting(false);
  transaction.complete(&gatePrefs);

  simData::GateUpdate* gateUpdate = dataStore.addGateUpdate(gateId, &transaction);
  gateUpdate->set_time(0.0);
  gateUpdate->set_minrange(85000.0);
  gateUpdate->set_maxrange(85000.0);
  gateUpdate->set_azimuth(90.0 * simCore::DEG2RAD);
  gateUpdate->set_elevation(0.0);
  gateUpdate->set_width(40.0 * simCore::DEG2RAD);
  gateUpdate->set_height(30.0 * simCore::DEG2RAD);
  transaction.complete(&gateUpdate);

  return gateId;
}

/// connect beam to platform, set some properties
void configurePrefs(simData::ObjectId platformId,
                    float scale,
                    simVis::ScenarioManager* scenario)
{
  osg::ref_ptr<simVis::PlatformNode> node = scenario->find<simVis::PlatformNode>(platformId);

  /// configure the platform
  simData::PlatformPrefs prefs = node->getPrefs();
  prefs.mutable_commonprefs()->set_name("Simulated Platform");
  prefs.set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs.set_scale(scale);
  prefs.set_dynamicscale(true);
  node->setPrefs(prefs);
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // load the image.
  std::string imageURL = "LandSiteV.png";
  if (argc > 1)
    imageURL = argv[1];

  /// use the utility code to create a basic world map (terrain imagery and height)
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  //simExamples::addDefaultSkyNode(viewer);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  /// (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;

  // allow interpolation
  simData::LinearInterpolator interpolator;
  dataStore.setInterpolator(&interpolator);
  dataStore.enableInterpolation(true);

  /// bind dataStore to the scenario manager
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  osg::ref_ptr<simVis::ScenarioManager> scenario = scene->getScenario();
  scenario->bind(&dataStore);

  /// add in platforms and their respective projectors
  platformId_0 = addPlatform(dataStore);
  osg::ref_ptr<simVis::EntityNode> vehicle_0 = scenario->find(platformId_0);
  projectorId_0 = addProjector(scenario.get(), vehicle_0->getId(), dataStore, imageURL, false);

  platformId_1 = addPlatform(dataStore);
  osg::ref_ptr<simVis::EntityNode> vehicle_1 = scenario->find(platformId_1);
  projectorId_1 = addProjector(scenario.get(), vehicle_1->getId(), dataStore, imageURL, true);

  // add a gate to use it as a projection surface:
  simData::ObjectId gateId = addGate(platformId_1, dataStore);
  osg::ref_ptr<simVis::GateNode> gateNode = scenario->find<simVis::GateNode>(gateId);
  osg::ref_ptr<simVis::ProjectorNode> projector_1 = scenario->find<simVis::ProjectorNode>(projectorId_1);
  if (gateNode.valid() && projector_1.valid())
      gateNode->acceptProjector(projector_1.get());

  /// platform to use as a target to test projecting on to a platform
  platformId_2 = addPlatform(dataStore);
  osg::ref_ptr<simVis::PlatformNode> vehicle_2 = scenario->find<simVis::PlatformNode>(platformId_2);
  osg::ref_ptr<simVis::ProjectorNode> projector_0 = scenario->find<simVis::ProjectorNode>(projectorId_0);
  if (vehicle_2.valid() && projector_0.valid())
  {
    vehicle_2->acceptProjector(projector_0.get());
  }

  /// platform that shines on Hawaii
  platformId_3 = addPlatform(dataStore);
  osg::ref_ptr<simVis::EntityNode> vehicle_3 = scenario->find(platformId_3);
  projectorId_3 = addProjector(scenario.get(), vehicle_3->getId(), dataStore, imageURL, false);

  /// connect them and add some additional settings
  configurePrefs(platformId_0, 2.0, scenario.get());
  configurePrefs(platformId_1, 1.0, scenario.get());
  configurePrefs(platformId_2, 12.0, scenario.get());
  configurePrefs(platformId_3, 1.0, scenario.get());

  /// simulator will compute time-based updates for the platforms
  osg::ref_ptr<simUtil::PlatformSimulator> sim_0 = new simUtil::PlatformSimulator(platformId_0);
  osg::ref_ptr<simUtil::PlatformSimulator> sim_1 = new simUtil::PlatformSimulator(platformId_1);
  osg::ref_ptr<simUtil::PlatformSimulator> sim_2 = new simUtil::PlatformSimulator(platformId_2);
  osg::ref_ptr<simUtil::PlatformSimulator> sim_3 = new simUtil::PlatformSimulator(platformId_3);

  /// create some waypoints (lat, lon, alt, duration)
  sim_0->addWaypoint(simUtil::Waypoint(0.0, -159.0, 265000, 40.0));
  sim_0->addWaypoint(simUtil::Waypoint(60.0, -159.0, 265000, 40.0));
  sim_0->setSimulateRoll(false);
  sim_0->setSimulatePitch(true);

  sim_1->addWaypoint(simUtil::Waypoint(20.0, -90.0, 120000, 20.0));
  sim_1->addWaypoint(simUtil::Waypoint(20.0,  60.0, 120000, 20.0));
  sim_1->addWaypoint(simUtil::Waypoint(20.0, 180.0, 120000, 20.0));
  sim_1->setSimulateRoll(false);
  sim_1->setSimulatePitch(false);

  /// flies just ahead of platform 1 so it can get projected upon
  sim_2->addWaypoint(simUtil::Waypoint(1.0, -159.0, 225000, 40.0));
  sim_2->addWaypoint(simUtil::Waypoint(61.0, -159.0, 225000, 40.0));

  /// just sits there pointing at Hawaii
  sim_3->addWaypoint(simUtil::Waypoint(20.0, -159.0, 1000000, -89.9, 0.0, 1.0));

  /// Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  simMgr->addSimulator(sim_0.get());
  simMgr->addSimulator(sim_1.get());
  simMgr->addSimulator(sim_2.get());
  simMgr->addSimulator(sim_3.get());
  simMgr->simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simMgr.get(), 0.0, 120.0);
  viewer->addEventHandler(simHandler.get());

  /// Tether camera to the first platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = scene->getScenario()->find<simVis::PlatformNode>(platformId_0);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 5e5);

  /// handle key press events
  viewer->addEventHandler(new MenuHandler(dataStore, *viewer->getView(0), projectorId_0, imageURL));

  /// hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(scene.get()));

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  //viewer->setNavigationMode (simVis::NAVMODE_PERSPECTIVE);

  return viewer->run();
}

