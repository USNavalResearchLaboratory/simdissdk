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
 * Picking Example
 *
 * Demonstrate how to use the picker with the SIMDIS SDK.
 */
#include <cstdlib>
#include "osgEarth/NodeUtils"
#include "osgEarthUtil/RTTPicker"
#include "osgEarth/Registry"
#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Time/ClockImpl.h"
#include "simData/MemoryDataStore.h"
#include "simData/LinearInterpolator.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/SceneManager.h"
#include "simVis/OverheadMode.h"
#include "simVis/GOG/Parser.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simUtil/ExampleResources.h"


namespace ui = osgEarth::Util::Controls;

static const double MAX_TIME = 600.0; // seconds of data
static const double MIN_X = -2000.0;  // X position minimum, meters, tangent plane
static const double MAX_X = 2000.0;   // X position maximum, meters, tangent plane
static const double MIN_Y = -1000.0;  // Y position minimum, meters, tangent plane
static const double MAX_Y = 1000.0;   // Y position maximum, meters, tangent plane
static const double LAT = 35.0;       // Scenario origin degrees
static const double LON = -87.0;      // Scenario origin degrees
static const int NUM_PLATFORMS = 100; // Number of platforms to generate

ui::LabelControl* g_PickLabel = NULL;
osg::Uniform* g_HighlightIdUniform = NULL;
osg::Uniform* g_HighlightEnabledUniform = NULL;
static const std::string NO_PICK = "-";  // Text to show when nothing is picked

/** Vertex shader that assigns an output variable if the vertex is part of the selected object */
const char* HIGHLIGHT_VERTEX_SHADER =
  "#version " GLSL_VERSION_STR "\n"
  // Object ID provided via uniform that should be highlighted
  "uniform uint sdk_objectid_to_highlight; \n"
  // Uniform for enabling and disabling highlighting
  "uniform bool sdk_highlight_enabled; \n"
  // osgEarth-provided Object ID of the current vertex
  "uint oe_index_objectid;      // Stage global containing object id \n"
  // Output to fragment shader to mark an object selected
  "flat out int sdk_isselected; \n"
  // Assigns sdk_isselected based on input values
  "void checkForHighlight(inout vec4 vertex) \n"
  "{ \n"
  "  sdk_isselected = sdk_highlight_enabled && (sdk_objectid_to_highlight > 0u && sdk_objectid_to_highlight == oe_index_objectid) ? 1 : 0; \n"
  "} \n";

/** Fragment shader that applies a glow to the output color if object is selected */
const char* HIGHLIGHT_FRAGMENT_SHADER =
  "#version " GLSL_VERSION_STR "\n"
  // Input from the vertex shader
  "flat in int sdk_isselected; \n"
  // OSG built-in for frame time
  "uniform float osg_FrameTime; \n"
  "void highlightFragment(inout vec4 color) \n"
  "{ \n"
  "  if (sdk_isselected == 1) {\n"
  // Borrow code fromGlowHighlight.frag.glsl and modified to fit mouse operation a bit better
  "    float glowPct = sin(osg_FrameTime * 12.0); \n"
  "    color.rgb += 0.2 + 0.2 * glowPct; \n"
  //  Make the glow slightly blue-ish
  "    color *= vec4(0.7, 0.7, 1.0, 1.0); \n"
  "  } \n"
  "} \n";


/** Prints help text */
int usage(char** argv)
{
  SIM_NOTICE << argv[0] << "\n"
    << std::endl;

  return 0;
}

/** Random number generator */
double randomBetween(double min, double max)
{
  return min + (max - min) * static_cast<double>(rand()) / RAND_MAX;
}

/** Handles hotkey changes */
struct MenuHandler : public osgGA::GUIEventHandler
{
  MenuHandler(simCore::Clock& clock, simVis::View* mainView)
    : clock_(clock),
      mainView_(mainView)
  {
    //nop
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case 'p':
        if (clock_.isPlaying())
          clock_.stop();
        else
          clock_.playForward();
        return true;
      case 'h':
      {
        bool oldState = true;
        g_HighlightEnabledUniform->get(oldState);
        g_HighlightEnabledUniform->set(!oldState);
        return true;
      }
      }
    }

    return false;
  }

protected: // data
  simCore::Clock& clock_;
  osg::ref_ptr<simVis::View> mainView_;
};

/** Responds to callbacks from the RTT picker */
struct UpdateLabelPickCallback : public osgEarth::Util::RTTPicker::Callback
{
  void onHit(osgEarth::ObjectID id)
  {
    osg::Node* node = osgEarth::Registry::objectIndex()->get<osg::Node>(id);
    simVis::EntityNode* entity = osgEarth::findFirstParentOfType<simVis::EntityNode>(node);
    if (entity)
      g_PickLabel->setText(entity->getEntityName(simVis::EntityNode::REAL_NAME));
    else if (node)
    {
      // Since we know we're tagging GOGs, pull out the user values we encoded before
      std::string objectType;
      node->getUserValue("objectType", objectType);
      int gogIndex = 0;
      node->getUserValue("index", gogIndex);

      // Create a label to display information about the GOG
      std::stringstream newLabel;
      newLabel << node->getName() << " / " << objectType << " index " << gogIndex;
      g_PickLabel->setText(newLabel.str());
    }
    else
      onMiss();
    g_HighlightIdUniform->set(id);
  }

  void onMiss()
  {
    g_PickLabel->setText(NO_PICK);
    g_HighlightIdUniform->set(0u);
  }

  // pick whenever the mouse moves.
  bool accept(const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa)
  {
    return true;
  }
};

/** Creates an overlay that will show information to the user. */
ui::Control* createUi()
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl("Picking Example", 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new ui::LabelControl("p: Pause playback", 14, osgEarth::Color::White));
  vbox->addControl(new ui::LabelControl("O: Toggle overhead mode", 14, osgEarth::Color::White));
  vbox->addControl(new ui::LabelControl("h: Toggle highlighting", 14, osgEarth::Color::White));

  ui::Grid* grid = vbox->addControl(new ui::Grid);
  grid->setControl(0, 0, new ui::LabelControl("Picked:", 14, osgEarth::Color::White));
  g_PickLabel = grid->setControl(1, 0, new ui::LabelControl(NO_PICK, 14, osgEarth::Color::White));

  // Move it down just a bit
  vbox->setPosition(10, 10);

  return vbox;
}

/** Adds data points to a platform to bounce around inside a box */
void addDataPoints(simCore::CoordinateConverter& cc, simData::DataStore& dataStore, uint64_t id)
{
  // Pick a random value from 0 to 1000, with a given velocity
  double x = randomBetween(MIN_X, MAX_X);
  double y = randomBetween(MIN_Y, MAX_Y);
  double speed = randomBetween(50, 100);
  double angle = randomBetween(0, M_PI);
  simCore::Vec3 velocity;
  simCore::calculateVelocity(speed, angle, 0.0, velocity);

  for (int k = 0; k < 600; ++k)
  {
    simCore::Coordinate xeast(simCore::COORD_SYS_XEAST, simCore::Vec3(x, y, 0.0));
    xeast.setVelocity(velocity);
    xeast.setOrientation(angle, 0.0, 0.0);
    simCore::Coordinate ecef;
    cc.convert(xeast, ecef, simCore::COORD_SYS_ECEF);

    // Add the point to the data store
    simData::DataStore::Transaction txn;
    simData::PlatformUpdate* update = dataStore.addPlatformUpdate(id, &txn);
    update->set_time(k);
    update->set_x(ecef.x());
    update->set_y(ecef.y());
    update->set_z(ecef.z());
    update->set_vx(ecef.vx());
    update->set_vy(ecef.vy());
    update->set_vz(ecef.vz());
    update->set_psi(ecef.psi());
    update->set_theta(ecef.theta());
    update->set_phi(ecef.phi());
    txn.complete(&update);

    // Calculate next position
    x += velocity.x();
    y += velocity.y();
    // If over an arbitrary boundary, flip
    if (x > MAX_X || x < MIN_X)
      velocity.setX(-velocity.x());
    if (y > MAX_Y || y < MIN_Y)
      velocity.setY(-velocity.y());
    // Convert velocity to an angle
    simCore::Vec3 fpa;
    simCore::calculateFlightPathAngles(velocity, fpa);
    angle = fpa.yaw();
  }
}

/** Creates a single platform and sets its properties */
uint64_t createPlatform(simData::DataStore& dataStore)
{
  simData::DataStore::Transaction txn;
  simData::PlatformProperties* props = dataStore.addPlatform(&txn);
  const uint64_t id = props->id();
  txn.complete(&props);

  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->set_dynamicscale(true);
  prefs->set_lighted(false);
  if (rand() < RAND_MAX / 2)
  {
    prefs->set_icon(EXAMPLE_IMAGE_ICON);
    prefs->set_scale(2.0);
  }
  else
  {
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(3.5);
  }
  prefs->mutable_commonprefs()->set_name(osgEarth::Stringify() << "Platform " << id);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(18);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(10);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_backdroptype(simData::BDT_SHADOW_BOTTOM_RIGHT);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_textoutline(simData::TO_THICK);
  prefs->mutable_trackprefs()->set_tracklength(4);
  txn.complete(&prefs);
  return id;
}

/** Creates a GOG */
void addGog(osg::Group* parentNode, osgEarth::MapNode* mapNode)
{
  // Note that the GOGs are all 300 meters in the air so overhead mode has an impact on apparent visual placement
  static const double GOG_ALT = 300.0;
  std::stringstream ss;
  ss << "version 2\n";
  ss << "start\n";
  ss << "line\n";
  ss << "3d name First Line\n";
  ss << "depthbuffer on\n";
  ss << "altitudeunits meters\n";
  ss << "linewidth 3\n";
  ss << "ll " << LAT << " " << LON << " " << GOG_ALT << "\n";
  ss << "ll " << LAT + 0.01 << " " << LON + 0.01 << "\n";
  ss << "end\n";

  ss << "start\n";
  ss << "circle\n";
  ss << "3d name First Circle - outlined\n";
  ss << "depthbuffer on\n";
  ss << "altitudeunits meters\n";
  ss << "linecolor green\n";
  ss << "linewidth 2\n";
  ss << "centerll " << LAT << " " << LON - 0.01 << " " << GOG_ALT << "\n";
  ss << "radius " << (0.25 * (MAX_X - MIN_X)) << "\n";
  ss << "end\n";

  ss << "start\n";
  ss << "circle\n";
  ss << "3d name Second Circle - filled\n";
  ss << "depthbuffer on\n";
  ss << "altitudeunits meters\n";
  ss << "filled\n";
  ss << "linecolor white\n";
  ss << "fillcolor blue 0x80ff4040\n";
  ss << "centerll " << LAT - 0.005 << " " << LON + 0.01 << " " << GOG_ALT << "\n";
  ss << "radius " << (0.12 * (MAX_X - MIN_X)) << "\n";
  ss << "end\n";

  // Configure the parser
  simVis::GOG::Parser parser(mapNode);
  parser.setReferenceLocation(osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), LON, LAT, 0.0, osgEarth::ALTMODE_ABSOLUTE));
  ss.seekg(0);

  // Load the GOG into the parser
  simVis::GOG::Parser::OverlayNodeVector gogs;
  std::vector<simVis::GOG::GogFollowData> followData;
  if (!parser.loadGOGs(ss, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData))
  {
    SIM_WARN << "Unable to load GOG data.\n";
    return;
  }

  // Add the GOG nodes generated in the parser
  int index = 0;
  for (auto i = gogs.begin(); i != gogs.end(); ++i)
  {
    osg::Node* gog = (*i)->osgNode();
    // Add some user values that we can pull out in the picker
    gog->setUserValue("objectType", std::string("GOG"));
    gog->setUserValue("index", index++);
    // Tagging the GOG makes it selectable by the RTT Picker
    auto objectId = osgEarth::Registry::objectIndex()->tagNode(gog, gog);
    gog->setUserValue("registryId", objectId);
    parentNode->addChild(gog);
  }
}

void installHighlighter(osg::StateSet* stateSet, int attrLocation)
{
  // This shader program will highlight the selected object.
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
  vp->setFunction("checkForHighlight", HIGHLIGHT_VERTEX_SHADER, osgEarth::ShaderComp::LOCATION_VERTEX_CLIP);
  vp->setFunction("highlightFragment", HIGHLIGHT_FRAGMENT_SHADER, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING);

  // Since we're accessing object IDs, we need to load the indexing shader as well
  osgEarth::Registry::objectIndex()->loadShaders(vp);

  // A uniform that will tell the shader which object to highlight:
  g_HighlightIdUniform = new osg::Uniform("sdk_objectid_to_highlight", 0u);
  stateSet->addUniform(g_HighlightIdUniform);
  g_HighlightEnabledUniform = new osg::Uniform("sdk_highlight_enabled", true);
  stateSet->addUniform(g_HighlightEnabledUniform);
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map);

  // Add sky node
  simExamples::addDefaultSkyNode(sceneMan);

  // We need a view manager. This handles all of our Views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager(arguments);

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan);

  // Create view and connect them to our scene.
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneMan.get());
  mainView->setUpViewInWindow(50, 50, 800, 600);

  // Set the viewpoint
  simVis::Viewpoint viewpoint;
  viewpoint.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), LON, LAT, 0.0, osgEarth::ALTMODE_ABSOLUTE);
  viewpoint.heading()->set(0.0, osgEarth::Units::DEGREES);
  viewpoint.pitch()->set(-89.0, osgEarth::Units::DEGREES);
  viewpoint.range()->set(2500, osgEarth::Units::METERS);
  mainView->setViewpoint(viewpoint);

  // Add it to the view manager
  viewMan->addView(mainView);

  // Create several platforms
  simData::MemoryDataStore dataStore;
  simCore::ClockImpl clock;
  sceneMan->addUpdateCallback(new simExamples::IdleClockCallback(clock, dataStore));
  dataStore.bindToClock(&clock);
  sceneMan->getScenario()->bind(&dataStore);
  // Add 100 platforms
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(LAT, LON, 100.0);

  // Seed the random number generator for more deterministic results
  srand(0);
  for (int k = 0; k < NUM_PLATFORMS; ++k)
  {
    uint64_t id = createPlatform(dataStore);
    addDataPoints(cc, dataStore, id);
  }

  // Apply the interpolator
  simData::LinearInterpolator interpolator;
  dataStore.setInterpolator(&interpolator);
  dataStore.enableInterpolation(true);

  // Add a GOG file with a few shapes
  addGog(sceneMan->getScenario(), sceneMan->getMapNode());

  // Start playing
  clock.setMode(simCore::Clock::MODE_REALTIME);
  clock.setStartTime(simCore::TimeStamp(dataStore.referenceYear(), 0.0));
  clock.setEndTime(simCore::TimeStamp(dataStore.referenceYear(), 600.0));
  clock.playForward();

  mainView->addOverlayControl(createUi());
  mainView->addEventHandler(new MenuHandler(clock, mainView));
  mainView->addEventHandler(new simVis::ToggleOverheadMode(mainView, 'O', 'C'));
  mainView->installDebugHandlers();

  // Add the picker
  osgEarth::Util::RTTPicker* picker = new osgEarth::Util::RTTPicker();
  mainView->addEventHandler(picker);
  picker->addChild(sceneMan->getScenario());
  // Install a callback that controls the picker and listens for hits.
  picker->setDefaultCallback(new UpdateLabelPickCallback());

  // Add a highlighter as mouse picks items
  installHighlighter(sceneMan->getScenario()->getOrCreateStateSet(),
    osgEarth::Registry::objectIndex()->getObjectIDAttribLocation());

  // Run until the user quits by hitting ESC.
  viewMan->run();
}
