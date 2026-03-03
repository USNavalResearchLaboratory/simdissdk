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
 * GOG Attachments Example.
 *
 * Demonstrates and tests each of the GOG node types (Arc, Circle, etc.) and how to attach them to a platform.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/GOG/Parser.h"
#include "simCore/String/UtfUtils.h"
#include "simData/MemoryDataStore.h"
#include "simUtil/PlatformSimulator.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Viewer.h"

/// GOG
#include "simVis/GOG/Annotation.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/Hemisphere.h"
#include "simVis/GOG/Line.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/Loader.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/Sphere.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osg/Switch"
#include "osgEarth/Version"
#include "osgEarth/StringUtils"
#include "osgEarth/Style"
#include "osgEarth/Sky"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif
using namespace osgEarth;
using namespace osgEarth::Util;

//----------------------------------------------------------------------------

/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "GOG Attachment Example";

/// later lines, document the keyboard controls
static const std::string s_help =
" g : cycle through the various GOG types";

/// keep a handle, for toggling
static osg::NodeList s_attachments;
typedef std::shared_ptr<simVis::GOG::GogNodeInterface> GogNodeInterfacePtr;
static std::vector<GogNodeInterfacePtr> s_overlayNodes;

//----------------------------------------------------------------------------

static void makeStar(Geometry* geom, bool close)
{
  const unsigned n = 5;  // n-pointed star
  const double a = osg::PI / double(n);
  for (unsigned i = 0; i < 2 * n + (close?1:0); ++i)
  {
    const double r = (i&1) == 0 ? 1000.0 : 400.0;
    geom->push_back(osg::Vec3d(cos(double(i)*a)*r, sin(double(i)*a)*r, 0.));
  }
}

#ifdef HAVE_IMGUI

struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel()
    : simExamples::SimExamplesGui("GOG Attachments Example"),
    swChild_(static_cast<unsigned int>(~0))
  {
    addKeyFunc_(ImGuiKey_G, [this]()
      {
        if (swChild_ != static_cast<unsigned>(~0))
          s_attachments[swChild_]->setNodeMask(0);
        if (++swChild_ == s_attachments.size())
          swChild_ = 0;
        s_attachments[swChild_]->setNodeMask(~0);
        nowViewing_ = ("Now viewing: " + s_attachments[swChild_]->getName());
      });
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("g : cycle through the various GOG types");

    if (!nowViewing_.empty())
      ImGui::Text("%s", nowViewing_.c_str());

    ImGui::End();

    handlePressedKeys_();
  }

private:
  unsigned int swChild_;
  std::string nowViewing_;
};

#endif
//----------------------------------------------------------------------------

/// create a platform and add it to 'dataStore'
///@return id for the new platform
simData::ObjectId addPlatform(simData::DataStore &dataStore)
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
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
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
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC

  sim->setSimulateRoll(true);

  /// Install frame update handler that will update track positions over time.
  simMgr.addSimulator(sim.get());
  simMgr.simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(&simMgr, 0.0, 120.0);
  viewer->addEventHandler(simHandler.get());

  /// Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = viewer->getSceneManager()->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -30, 10000);

  return platformNode.get();
}

//----------------------------------------------------------------------------

void setupGOGAttachments(simVis::PlatformNode* platform)
{
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);

  // Arc:
  {
    std::string def = R"(
      start
      arc
      radius 1500
      anglestart 45
      angledeg 270
      linecolor yellow 0xff00ffff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 0, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Arc");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Circle:
  {
    std::string def = R"(
      start
      circle
      radius 1500
      filled
      fillcolor cyan 0x7fffff00
      linecolor yellow 0xff00ffff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 1, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Circle");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);

  }

  // Cylinder:
  {
    std::string def = R"(
      start
      cylinder
      rangeunits km
      radius 1
      height 200
      anglestart 45
      angleend 315
      linecolor red 0x7f0000ff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 2, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Cylinder");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Ellipse:
  {
    std::string def = R"(
      start
      ellipse
      rangeunits km
      majoraxis 1
      minoraxis 0.5
      fillcolor orange 0x7f00a5ff
      linecolor yellow 0xff00ffff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 3, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Ellipse");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Hemisphere:
  {
    std::string def = R"(
      start
      hemisphere
      rangeunits nm
      radius 1
      linecolor purple 0x7ff020a0
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 4, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Hemisphere");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // LatLonAltBox:
  // NOP. You cannot attach a latlonalt box to an entity.
  // It only exists in geographic coordinates.

  // Line:
  {
    std::string def = R"(
      start
      line
      xy -1000 -1000
      xy -1000 1000
      xy 1000 1000
      xy 1000 -1000
      linecolor yellow 0xff00ffff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 5, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Line");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // LineSegs:
  {
    std::string def = R"(
      start
      linesegs
      xyz 0 250 0
      xyz 0 1500 0
      xyz 250 0 0
      xyz 1500 0 0
      xyz 0 0 250
      xyz 0 0 1500
      linestyle dash
      linecolor yellow 0xff00ffff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 6, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("LineSegs");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Points:
  {
    std::string def = R"(
      start
      points
      xy -1000 -200
      xy -800 -200
      xy -600 -200
      xy -400 -200
      xy -200 -200
      xy 0 -200
      pointsize 7.5
      linecolor lime 0xff00ffbf
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 7, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Points");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Polygon:
  {
    std::string def = R"(
      start
      poly
      xy -1000 -1000
      xy -1000 1000
      xy 1000 1000
      xy 1000 -1000
      linecolor orange 0x7f007fff
      filled
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 8, true, gogs);

    auto gog = gogs.front();
    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Sphere:
  {
    std::string def = R"(
      start
      sphere
      rangeunits nm
      radius 1
      linecolor yellow 0xff00ffff
      linecolor red 0x7f0000ff
      end
      )";

    simVis::GOG::Loader::GogNodeVector gogs;
    loader.loadShape(def, "", 9, true, gogs);

    auto gog = gogs.front();
    gog->osgNode()->setName("Sphere");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // make them all invisible for starters
  for (osg::NodeList::iterator i = s_attachments.begin(); i != s_attachments.end(); ++i)
    i->get()->setNodeMask(0);
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

  // add a sky to the scene.
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  osg::ref_ptr<SkyNode> sky = SkyNode::create();
  sky->attach(viewer->getMainView());
  sky->setDateTime(osgEarth::DateTime(2011, 10, 1, 10.0));
  scene->getScenario()->addChild(sky);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform
  simData::ObjectId platformId = addPlatform(dataStore);

  /// simulate it so we have something to attach GOGs to
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  osg::ref_ptr<simVis::PlatformNode> platform = setupSimulation(*simMgr, platformId, dataStore, viewer.get());

  /// make some example GOGs.
  setupGOGAttachments(platform.get());

  /// attach the GOGs to the platform. You can set a custom LocatorComponents enum
  /// to designate how the GOGs should track the platform.
  for (osg::NodeList::iterator i = s_attachments.begin(); i != s_attachments.end(); ++i)
  {
    platform->attach(i->get());
  }

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel());
#endif

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  const int status = viewer->run();

  // clean up resources held in static containers
  s_attachments.clear();
  s_overlayNodes.clear();

  return status;
}

