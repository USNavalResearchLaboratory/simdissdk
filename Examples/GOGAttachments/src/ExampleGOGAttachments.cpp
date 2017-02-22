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
 * GOG Attachments Example.
 *
 * Demonstrates and tests each of the GOG node types (Arc, Circle, etc.) and how to attach them to a platform.
 */

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simNotify/Notify.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"

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
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/Sphere.h"
#include "simVis/GOG/Parser.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osg/Switch"
#include "osgEarth/StringUtils"
#include "osgEarthSymbology/Style"
#include "osgEarthUtil/Sky"

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------

/// create an overlay with some helpful information

/// first line, describe the program
static const std::string s_title = "GOG Attachment Example";

/// later lines, document the keyboard controls
static const std::string s_help =
" g : cycle through the various GOG types";

/// keep a handle, for toggling
static osg::ref_ptr<Control>      s_helpControl;
static osg::ref_ptr<LabelControl> s_nowViewing;
static osg::NodeList s_attachments;
typedef std::tr1::shared_ptr<simVis::GOG::GogNodeInterface> GogNodeInterfacePtr;
static std::vector<GogNodeInterfacePtr> s_overlayNodes;

static Control* createHelp()
{
  // vbox is allocated here but memory owned by caller
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20, Color::Yellow));
  vbox->addControl(new LabelControl(s_help, 14, Color::White));
  s_nowViewing = vbox->addControl(new LabelControl("", 24, Color::White));
  s_nowViewing->setPadding(Gutter(10.0f, 0.f, 0.f, 0.f));
  s_helpControl = vbox;
  return vbox;
}

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

//----------------------------------------------------------------------------
/// event handler for keyboard commands to alter symbology at runtime
struct MenuHandler : public osgGA::GUIEventHandler
{
  /// constructor grabs all the state it needs for updating
  MenuHandler() : swChild_(static_cast<unsigned>(~0)) { }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    bool handled = false;
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
        case 'g': // cycle through the various GOG objects.
          if (swChild_ != static_cast<unsigned>(~0))
            s_attachments[swChild_]->setNodeMask(0);
          if (++swChild_ == s_attachments.size())
            swChild_ = 0;
          s_attachments[swChild_]->setNodeMask(~0);
          s_nowViewing->setText("Now viewing: " + s_attachments[swChild_]->getName());
          handled = true;
          break;
      }
    }
    return handled;
  }

protected: // data
  unsigned     swChild_;
};

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
  simMgr.addSimulator(sim);
  simMgr.simulate(0.0, 120.0, 60.0);

  /// Attach the simulation updater to OSG timer events
  osg::ref_ptr<simVis::SimulatorEventHandler> simHandler = new simVis::SimulatorEventHandler(&simMgr, 0.0, 120.0);
  viewer->addEventHandler(simHandler);

  /// Tether camera to platform
  osg::ref_ptr<simVis::PlatformNode> platformNode = viewer->getSceneManager()->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformNode);

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -30, 10000);

  return platformNode;
}

//----------------------------------------------------------------------------

void setupGOGAttachments(simVis::PlatformNode* platform)
{
  Style defaultStyle;
  defaultStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color::Yellow;
  simVis::GOG::GogFollowData followData;


  // Arc:
  {
    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("arc");
    def.push_back("radius     1500");
    def.push_back("anglestart 45");
    def.push_back("angledeg   270");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(defaultStyle);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Arc");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Circle:
  {
    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("circle");
    def.push_back("radius 1500");
    def.push_back("filled");
    def.push_back("end");

    // override the style just for fun
    Style style(defaultStyle);
    style.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Cyan, 0.5f);

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Circle");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);

  }

  // Cylinder:
  {
    // style it to be translucent-red with white lines:
    Style style(defaultStyle);
    style.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Red, 0.5);
    style.getOrCreate<LineSymbol>()->stroke()->color() = Color(Color::White, 0.4);

    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("cylinder");
    def.push_back("rangeunits km");
    def.push_back("radius 1");
    def.push_back("height 1");
    def.push_back("anglestart 45");
    def.push_back("angleend 315");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Cylinder");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Ellipse:
  {
    Style style(defaultStyle);
    style.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Orange, 0.5);

    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("ellipse");
    def.push_back("rangeunits km");
    def.push_back("majoraxis 1");
    def.push_back("minoraxis 0.5");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Ellipse");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Hemisphere:
  {
    Style style(defaultStyle);
    style.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Purple, 0.5);

    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("hemisphere");
    def.push_back("rangeunits nm");
    def.push_back("radius 1");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Hemisphere");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // LatLonAltBox:
  {
    // NOP. You cannot attach a latlonalt box to an entity.
    // It only exists in geographic coordinates.
  }

  // Line:
  {
    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("line");
    def.push_back("xy -1000 -1000");
    def.push_back("xy -1000  1000");
    def.push_back("xy  1000  1000");
    def.push_back("xy  1000 -1000");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(defaultStyle);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Line");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // LineSegs:
  {
    // set up a slipple pattern (for a dashed line)
    Style style(defaultStyle);
    style.get<LineSymbol>()->stroke()->stipple() = 0xF0F0;

    // make a list of coordinate pairs. Each pair produces a segment.
    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("linesegs");
    def.push_back("xyz    0  250    0");
    def.push_back("xyz    0 1500    0");
    def.push_back("xyz  250    0    0");
    def.push_back("xyz 1500    0    0");
    def.push_back("xyz    0    0  250");
    def.push_back("xyz    0    0 1500");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("LineSegs");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Points:
  {
    Style style(defaultStyle);
    style.getOrCreate<PointSymbol>()->size() = 7.5f;
    style.getOrCreate<PointSymbol>()->fill()->color() = Color::Lime;

    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("points");
    def.push_back("xy -1000 -200");
    def.push_back("xy  -800 -200");
    def.push_back("xy  -600 -200");
    def.push_back("xy  -400 -200");
    def.push_back("xy  -200 -200");
    def.push_back("xy     0 -200");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

    gog->osgNode()->setName("Points");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Polygon:
  {
    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("poly");
    def.push_back("xy -1000 -1000");
    def.push_back("xy -1000  1000");
    def.push_back("xy  1000  1000");
    def.push_back("xy  1000 -1000");
    def.push_back("linecolor orange 0x7f007fff");
    def.push_back("filled");
    def.push_back("end");

    simVis::GOG::Parser parser;
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));
    gog->osgNode()->setName("Polygon");

    s_attachments.push_back(gog->osgNode());
    s_overlayNodes.push_back(gog);
  }

  // Sphere:
  {
    Style style(defaultStyle);
    style.getOrCreate<PolygonSymbol>()->fill()->color() = Color(Color::Red, 0.5);

    std::vector<std::string> def;
    def.push_back("start");
    def.push_back("sphere");
    def.push_back("rangeunits nm");
    def.push_back("radius 1");
    def.push_back("end");

    simVis::GOG::Parser parser;
    parser.setStyle(style);
    GogNodeInterfacePtr gog(parser.createGOG(def, simVis::GOG::GOGNODE_HOSTED, followData));

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
  viewer->setMap(map);
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add a sky to the scene.
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  osg::ref_ptr<SkyNode> sky = SkyNode::create(scene->getMapNode());
  sky->attach(viewer->getMainView());
  sky->setDateTime(osgEarth::Util::DateTime(2011, 10, 1, 10.0));
  scene->getScenario()->addChild(sky);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  /// add in the platform
  simData::ObjectId platformId = addPlatform(dataStore);

  /// simulate it so we have something to attach GOGs to
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  osg::ref_ptr<simVis::PlatformNode> platform = setupSimulation(*simMgr, platformId, dataStore, viewer);

  /// If there's a gog file on the cmd line, use that; otherwise build some examples.
  if (argc > 1)
  {
    /// Create a parser to load the GOG file:
    simVis::GOG::Parser parser(scene->getMapNode());

    /// Load all the GOGs from the file:
    simVis::GOG::Parser::OverlayNodeVector gogs;
    std::vector<simVis::GOG::GogFollowData> followData;
    std::ifstream is(argv[1]);
    if (!is.is_open())
    {
      std::string fileName(argv[1]);
      SIM_ERROR << "Could Not Open GOG file " << fileName << "\n";
      return 1;
    }
    if (parser.loadGOGs(is,  simVis::GOG::GOGNODE_HOSTED, gogs, followData))
    {
      for (simVis::GOG::Parser::OverlayNodeVector::iterator i = gogs.begin(); i != gogs.end(); ++i)
      {
        //TODO: handle locator component selection
        platform->attach((*i)->osgNode());
      }
    }
  }

  else
  {
    /// make some example GOGs.
    setupGOGAttachments(platform);

    /// attach the GOGs to the platform. You can set a custom LocatorComponents enum
    /// to designate how the GOGs should track the platform.
    for (osg::NodeList::iterator i = s_attachments.begin(); i != s_attachments.end(); ++i)
    {
      platform->attach(i->get());
    }

    /// handle key press events
    viewer->addEventHandler(new MenuHandler());
  }

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

