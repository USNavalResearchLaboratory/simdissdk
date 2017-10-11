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
 * GOG READER EXAMPLE - SIMDIS SDK
 *
 * Demonstrates the loading and display of SIMDIS .gog format vector overlay data.
 */
#include "simData/MemoryDataStore.h"
#include "simVis/GOG/GOG.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Parser.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"

#include "simVis/Viewer.h"
#include "simVis/OverheadMode.h"
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simUtil/ExampleResources.h"
#include "osgEarthAnnotation/PlaceNode"
#include "osgEarthAnnotation/LabelNode"
#include "osgEarthUtil/MouseCoordsTool"

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Annotation;

typedef std::shared_ptr<simVis::GOG::GogNodeInterface> GogNodeInterfacePtr;
static std::vector<GogNodeInterfacePtr> s_overlayNodes;

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore, const std::string& iconFile)
{
  simData::ObjectId platformId;

  // create the new platform:
  {
    /// all DataStore operations require a transaction (to avoid races)
    simData::DataStore::Transaction transaction;

    /// create the platform, and get the properties for it
    simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

    /// save the platform id for our return value
    platformId = newProps->id();

    /// done
    transaction.complete(&newProps);
  }

  // now set up the platform:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->mutable_commonprefs()->set_name("Simulated Platform");
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->set_icon(iconFile);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return platformId;
}

simVis::PlatformNode* setupSimulation(
                simUtil::PlatformSimulatorManager& simMgr,
                simData::ObjectId                  platformId,
                simData::DataStore&                dataStore,
                simVis::Viewer*                    viewer)
{
  /// simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  /// create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(30.0,  -120.0, 30000, 2000.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 2000.0)); // DC

  sim->setSimulateRoll(true);

  /// Install frame update handler that will update track positions over time.
  simMgr.addSimulator(sim);
  simMgr.simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simVis::SimulatorEventHandler> simHandler = new simVis::SimulatorEventHandler(&simMgr, 0.0, 120.0);
  viewer->addEventHandler(simHandler);

  /// Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = viewer->getSceneManager()->getScenario()->find<simVis::PlatformNode>(platformId);

  return platformNode;
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  if (argc < 2)
  {
    std::cout << "Usage: example_gogreader <gogfile> [--mark] [--iconFile <icon file>]" << std::endl;
    return 0;
  }

  // Start by creating a map.
  simExamples::configureSearchPaths();
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // whether to add a push pin to each feature
  osg::ArgumentParser ap(&argc, argv);

  // start up a SIMDIS viewer->
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(ap);
  viewer->setMap(map);
  viewer->installDebugHandlers();
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  if (ap.read("--sky"))
    simExamples::addDefaultSkyNode(viewer);

  bool mark = ap.read("--mark");

  osg::ref_ptr<osg::Image> pin;
  if (mark)
    pin = URI("http://www.osgearth.org/chrome/site/pushpin_yellow.png").getImage();

  GeoPoint go;

  std::string iconFile = EXAMPLE_IMAGE_ICON;

  // add the gog file vector layers.
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--iconFile" && argc > i)
    {
      iconFile = argv[++i];
      continue;
    }

    simVis::GOG::Parser::OverlayNodeVector gogs;
    std::vector<simVis::GOG::GogFollowData> followData;
    simVis::GOG::Parser parser(scene->getMapNode());

    // sets a default reference location for relative GOGs:
    parser.setReferenceLocation(simVis::GOG::BSTUR);

    std::ifstream is(arg.c_str());
    if (!is.is_open())
    {
      std::string fileName(argv[i]);
      SIM_ERROR <<"Could not open GOG file " << fileName << "\n";
      return 1;
    }

    if (parser.loadGOGs(is, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData))
    {
      for (simVis::GOG::Parser::OverlayNodeVector::iterator i = gogs.begin(); i != gogs.end(); ++i)
      {
        GogNodeInterfacePtr gogInterface(*i);
        osg::Node* gog = (*i)->osgNode();
        scene->getScenario()->addChild(gog);
        // manage the GogNodeInterface object memory
        s_overlayNodes.push_back(gogInterface);

        if (mark)
        {
          osg::Vec3d ecef0 = gog->getBound().center();
          simCore::Vec3 ecef(ecef0.x(), ecef0.y(), ecef0.z());
          simCore::Vec3 lla;
          simCore::CoordinateConverter::convertEcefToGeodeticPos(ecef, lla);

          std::string label = gog->getName();

          GeoPoint location(
            scene->getMapNode()->getMapSRS(),
            osg::RadiansToDegrees(lla.lon()),
            osg::RadiansToDegrees(lla.lat()),
            0.0,
            ALTMODE_ABSOLUTE);

          osg::ref_ptr<AnnotationNode> marker;
          if (label.empty())
            marker = new PlaceNode(scene->getMapNode(), location, pin.get(), label);
          else
            marker = new LabelNode(scene->getMapNode(), location, label);

          scene->getScenario()->addChild(marker);

          go = location;
        }
      }
    }
    else
    {
      SIM_WARN << "Unable to load GOG data from \"" << argv[i] << "\"" << std::endl;
    }
  }

  // mouse coords readout
  osg::ref_ptr<LabelControl> readout = new LabelControl("");
  ControlCanvas::getOrCreate(viewer->getMainView())->addControl(readout.get());
  viewer->getMainView()->addEventHandler(new MouseCoordsTool(scene->getMapNode(), readout));

  if (mark && go.isValid())
  {
    Viewpoint vp;
    vp.focalPoint() = go;
    vp.pitch()->set(-80.0, Units::DEGREES);
    vp.range()->set(scene->getScenario()->getBound().radius(), Units::METERS);
    viewer->getMainView()->setViewpoint( vp );
  }

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform
  simData::ObjectId platformId = addPlatform(dataStore, iconFile);

  /// simulate it so we have something to attach GOGs to
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  osg::ref_ptr<simVis::PlatformNode> platform = setupSimulation(*simMgr, platformId, dataStore, viewer);

  viewer->addEventHandler(new simVis::ToggleOverheadMode(viewer->getMainView(), 'O', 'C'));

  viewer->run();
}

