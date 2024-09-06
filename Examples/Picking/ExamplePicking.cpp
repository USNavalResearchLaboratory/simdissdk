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
 * Picking Example
 *
 * Demonstrate how to use the picker with the SIMDIS SDK.
 */
#include <cstdlib>
#include "osgEarth/NodeUtils"
#include "osgEarth/ObjectIndex"
#include "osgEarth/Registry"
#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/GOG/Parser.h"
#include "simCore/Time/ClockImpl.h"
#include "simData/LinearInterpolator.h"
#include "simData/MemoryDataStore.h"
#include "simVis/EarthManipulator.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/OverheadMode.h"
#include "simVis/Picker.h"
#include "simVis/PlatformIconFactory.h"
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Loader.h"
#include "simUtil/DynamicSelectionPicker.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/PlatformPopupManipulator.h"
#include "CustomRender.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
namespace ui = osgEarth::Util::Controls;
#endif

static const double MAX_TIME = 600.0; // seconds of data
static const double MIN_X = -2000.0;  // X position minimum, meters, tangent plane
static const double MAX_X = 2000.0;   // X position maximum, meters, tangent plane
static const double MIN_Y = -1000.0;  // Y position minimum, meters, tangent plane
static const double MAX_Y = 1000.0;   // Y position maximum, meters, tangent plane
static const double LAT = 35.0;       // Scenario origin degrees
static const double LON = -87.0;      // Scenario origin degrees
static const int NUM_PLATFORMS = 100; // Number of platforms to generate

static const std::string NO_PICK = "-";  // Text to show when nothing is picked

/** Data structure that contains variables used throughout the application */
struct Application
{
#ifdef HAVE_IMGUI
  std::string pickLabel = NO_PICK;
#else
  osg::ref_ptr<ui::LabelControl> pickLabel;
#endif
  osg::ref_ptr<simVis::View> mainView;
  osg::ref_ptr<simVis::View> mainRttView;
  osg::ref_ptr<simVis::View> insetView;
  osg::ref_ptr<simVis::View> insetRttView;
  osg::ref_ptr<simVis::Picker> picker;
  osg::ref_ptr<simVis::PickerHighlightShader> highlightShader;
};

/** Prints help text */
int usage(char** argv)
{
  SIM_NOTICE << argv[0] << " [--rtt|--intersect|--dynamic]\n"
    << "\n"
    << "  --rtt         Enable render-to-texture picking\n"
    << "  --intersect   Enable intersection picking\n"
    << "  --dynamic     Enable dynamic selection algorithm picking\n"
    << std::endl;

  return 0;
}

/** Random number generator */
double randomBetween(double min, double max)
{
  return min + (max - min) * static_cast<double>(rand()) / RAND_MAX;
}

/** Handles presses for the menu, and also handles mouse click events */
class MenuHandler : public osgGA::GUIEventHandler
{
public:
  MenuHandler(simCore::Clock& clock, Application& app)
    : clock_(clock),
      app_(app),
      blockMouseUntilRelease_(false)
  {
    //nop
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    // Handle key presses
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      return handleKeyPress(ea.getKey());
    // Handle mouse presses
    if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
    {
      simVis::View* asView = dynamic_cast<simVis::View*>(&aa);
      if (asView)
      {
        blockMouseUntilRelease_ = handleMouseClick(asView);
        return blockMouseUntilRelease_;
      }
    }

    // Ignore mouse motion, double click, and pushes, until we get a release
    if (blockMouseUntilRelease_)
    {
      // Eat push, drag, move, and double click
      if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH ||
        ea.getEventType() == osgGA::GUIEventAdapter::DRAG ||
        ea.getEventType() == osgGA::GUIEventAdapter::MOVE ||
        ea.getEventType() == osgGA::GUIEventAdapter::DOUBLECLICK)
        return true;
      // On release, stop blocking mouse
      if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
        blockMouseUntilRelease_ = false;
    }
    return false;
  }

  /** End user hit a key on their keyboard */
  bool handleKeyPress(int key) const
  {
    switch (key)
    {
    case 'p': // Toggle clock playing
      if (clock_.isPlaying())
        clock_.stop();
      else
        clock_.playForward();
      return true;

    case 'h': // Toggle highlighting
      app_.highlightShader->setEnabled(!app_.highlightShader->isEnabled());
      return true;

    case 'v': // Swap Viewpoints
    {
      // Fix overhead first
      const bool mainOverhead = app_.mainView->isOverheadEnabled();
      app_.mainView->enableOverheadMode(app_.insetView->isOverheadEnabled());
      app_.insetView->enableOverheadMode(mainOverhead);

      // Swap viewpoints next
      const simVis::Viewpoint mainViewpoint = app_.mainView->getViewpoint();
      app_.mainView->setViewpoint(app_.insetView->getViewpoint());
      app_.insetView->setViewpoint(mainViewpoint);
      return true;
    }

    case '1': // Toggle RTT MainView visibility
      if (app_.mainRttView.valid())
      {
        app_.mainRttView->setVisible(!app_.mainRttView->isVisible());
        return true;
      }
      break;
    case '2': // Toggle RTT Inset visibility
      if (app_.insetRttView.valid())
      {
        app_.insetRttView->setVisible(!app_.insetRttView->isVisible());
        return true;
      }
      break;

    case 'd':
      if (app_.insetView)
      {
        if (app_.insetRttView)
        {
          app_.mainView->removeInset(app_.insetRttView.get());
          app_.insetRttView = nullptr;
        }
        app_.mainView->removeInset(app_.insetView.get());
        app_.insetView = nullptr;
      }
      break;

    case 't':
      if (app_.insetView)
        app_.insetView->setVisible(!app_.insetView->isVisible());
      break;
    }

    return false;
  }

  /** End user clicked on a view */
  bool handleMouseClick(simVis::View* view) const
  {
    // Recenter the view on the clicked platform, if there is a platform
    simVis::EntityNode* entity = app_.picker->pickedEntity();
    if (entity)
    {
      view->tetherCamera(entity);
      return true;
    }

    // Try to find an annotation node child and change its attributes
    osgEarth::AnnotationNode* anno =
      osgEarth::findTopMostNodeOfType<osgEarth::AnnotationNode>(app_.picker->pickedNode());
    if (!anno)
      return false;

    auto style = anno->getStyle();
    auto lineSymbol = style.getOrCreateSymbol<osgEarth::LineSymbol>();
    // Change some line aspects to indicate we picked correctly
    lineSymbol->stroke()->color() = randomColor();
    lineSymbol->stroke()->width() = randomBetween(1.0, 7.0);
    anno->setStyle(style);
    return true;
  }

  /** Returns a random color, used by the click-on-GOG */
  simVis::Color randomColor() const
  {
    return simVis::Color(randomBetween(0.0, 1.0), randomBetween(0.0, 1.0), randomBetween(0.0, 1.0), 1.f);
  }

private: // data
  simCore::Clock& clock_;
  Application& app_;
  bool blockMouseUntilRelease_;
};


/** When the picker selects new items, this callback is triggered */
class UpdateLabelPickCallback : public simVis::Picker::Callback
{
public:
#ifdef HAVE_IMGUI
  explicit UpdateLabelPickCallback(std::string& label)
    : label_(label)
  {
  }
#else
  explicit UpdateLabelPickCallback(ui::LabelControl* label)
    : label_(label)
  {
  }
#endif

  /** Update the label when new items are picked */
  virtual void pickChanged(unsigned int pickedId, osg::Referenced* picked) override
  {
    osg::Node* node = dynamic_cast<osg::Node*>(picked);
    simVis::EntityNode* entity = osgEarth::findFirstParentOfType<simVis::EntityNode>(node);
    if (entity)
      setText_(entity->getEntityName(simVis::EntityNode::REAL_NAME));
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
      setText_(newLabel.str());
    }
    else
    {
      setText_(NO_PICK);
    }
  }

  /** Empty function to satisfy interface. If we wanted to get all entities under mouse instead of the best match, we would use this signature */
  virtual void pickChanged(const std::vector<simVis::Picker::PickedEntity>& picked) override
  {
    // noop
  }

private:
  void setText_(const std::string& text)
  {
#ifdef HAVE_IMGUI
    label_ = text;
#else
    label_->setText(text);
#endif
  }
#ifdef HAVE_IMGUI
  std::string& label_;
#else
  ui::LabelControl* label_;
#endif
};

#ifdef HAVE_IMGUI
struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel(Application& app, bool rttEnabled)
    : simExamples::SimExamplesGui("Picking Example"),
    app_(app),
    rttEnabled_(rttEnabled)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    if (firstDraw_)
    {
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("h : Toggle highlighting");
    ImGui::Text("O : Toggle overhead mode");
    ImGui::Text("p : Pause playback");
    ImGui::Text("v : Swap viewpoints");
    ImGui::Text("d : Delete inset");
    ImGui::Text("t : Toggle inset");
    if (rttEnabled_)
    {
      ImGui::Text("1 : Toggle RTT 1 display");
      ImGui::Text("2 : Toggle RTT 2 display");
    }
    ImGui::Text("Picked: "); ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "%s", app_.pickLabel.c_str());

    ImGui::End();
  }

private:
  Application& app_;
  bool rttEnabled_;
};
#else
/** Creates an overlay that will show information to the user. */
ui::Control* createUi(osg::ref_ptr<ui::LabelControl>& pickLabel, bool rttEnabled)
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl("Picking Example", 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl("h: Toggle highlighting", 14, simVis::Color::White));
  vbox->addControl(new ui::LabelControl("O: Toggle overhead mode", 14, simVis::Color::White));
  vbox->addControl(new ui::LabelControl("p: Pause playback", 14, simVis::Color::White));
  vbox->addControl(new ui::LabelControl("v: Swap viewpoints", 14, simVis::Color::White));
  vbox->addControl(new ui::LabelControl("d: Delete inset", 14, simVis::Color::White));
  vbox->addControl(new ui::LabelControl("t: Toggle inset", 14, simVis::Color::White));
  if (rttEnabled)
  {
    vbox->addControl(new ui::LabelControl("1: Toggle RTT 1 display", 14, simVis::Color::White));
    vbox->addControl(new ui::LabelControl("2: Toggle RTT 2 display", 14, simVis::Color::White));
  }

  ui::Grid* grid = vbox->addControl(new ui::Grid);
  grid->setControl(0, 0, new ui::LabelControl("Picked:", 14, simVis::Color::White));
  pickLabel = grid->setControl(1, 0, new ui::LabelControl(NO_PICK, 14, simVis::Color::Lime));

  // Move it down just a bit
  vbox->setPosition(10, 10);
  // Don't absorb events
  vbox->setAbsorbEvents(false);

  return vbox;
}
#endif

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

/** Configures common prefs between platforms and custom renders */
void configureCommonPrefs(simData::CommonPrefs& prefs, const std::string& namePrefix, simData::ObjectId id, int labelOffsetY)
{
  prefs.set_name(osgEarth::Stringify() << namePrefix << " " << id);
  prefs.mutable_labelprefs()->set_draw(true);
  prefs.mutable_labelprefs()->set_offsety(labelOffsetY);
  prefs.mutable_labelprefs()->set_overlayfontpointsize(10);
  prefs.mutable_labelprefs()->set_backdroptype(simData::BDT_SHADOW_BOTTOM_RIGHT);
  prefs.mutable_labelprefs()->set_textoutline(simData::TO_THICK);
}

/** Creates a single platform and sets its properties */
simData::ObjectId createPlatform(simData::DataStore& dataStore)
{
  simData::DataStore::Transaction txn;
  simData::PlatformProperties* props = dataStore.addPlatform(&txn);
  const simData::ObjectId id = props->id();
  txn.complete(&props);

  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(id, &txn);
  prefs->set_dynamicscale(true);
  prefs->set_lighted(false);
  if (id != 1 && rand() < RAND_MAX / 2) // Don't use image icon on platform #1, we're tethering to it later
  {
    prefs->set_icon(EXAMPLE_IMAGE_ICON);
    prefs->set_scale(2.0);
  }
  else
  {
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(3.5);
  }
  prefs->mutable_trackprefs()->set_tracklength(4);
  configureCommonPrefs(*prefs->mutable_commonprefs(), "Platform", id, 18);
  txn.complete(&prefs);
  return id;
}

/** Creates a new custom rendering entity in the DataStore, setting default prefs. */
simData::ObjectId addCustomRendering(simData::ObjectId hostId, simData::DataStore &dataStore, uint32_t prefsColor)
{
  simData::DataStore::Transaction txn;

  // Create the render object and set its properties
  simData::CustomRenderingProperties* customProps = dataStore.addCustomRendering(&txn);
  const simData::ObjectId result = customProps->id();
  customProps->set_hostid(hostId);
  // Set the renderer name.  This allows for multiple custom render engines
  customProps->set_renderer(ExamplePicking::RENDERER_NAME);
  txn.complete(&customProps);

  // Configure the prefs for the render object
  simData::CustomRenderingPrefs* prefs = dataStore.mutable_customRenderingPrefs(result, &txn);
  prefs->mutable_commonprefs()->set_color(prefsColor);
  configureCommonPrefs(*prefs->mutable_commonprefs(), "Custom Render", result, 35);
  txn.complete(&prefs);

  return result;
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
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser, mapNode);
  loader.setReferencePosition(simCore::Vec3(LON, LAT, 0.0));
  ss.seekg(0);

  // Load the GOG into the parser
  simVis::GOG::Loader::GogNodeVector gogs;
  loader.loadGogs(ss, "Picking", false, gogs);

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

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  if (arguments.read("--help"))
    return usage(argv);

  // Determine pick mode; the default is dynamic
  enum PickType {
    PickRtt,
    PickIntersect,
    PickDynamic
  } pickType = PickDynamic;
  if (arguments.read("--rtt"))
    pickType = PickRtt;
  else if (arguments.read("--intersect"))
    pickType = PickIntersect;

  // RTT does not support Platform Icon Factory optimizations
  if (pickType == PickRtt)
    simVis::PlatformIconFactory::instance()->setEnabled(false);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // Add sky node
  simExamples::addDefaultSkyNode(sceneMan.get());

  // We need a view manager. This handles all of our Views.
  osg::ref_ptr<simVis::ViewManager> viewMan = new simVis::ViewManager(arguments);

  // Set up the logarithmic depth buffer for all views
  osg::ref_ptr<simVis::ViewManagerLogDbAdapter> logDb = new simVis::ViewManagerLogDbAdapter;
  logDb->install(viewMan.get());

  // Create view and connect them to our scene.
  Application app;
  app.mainView = new simVis::View();
  app.mainView->setName("Main View");
  app.mainView->setSceneManager(sceneMan.get());
  app.mainView->setUpViewInWindow(50, 50, 800, 600);

  // Add it to the view manager
  viewMan->addView(app.mainView.get());

  // Create a "Super HUD" that shows on top of the main view
  osg::ref_ptr<simVis::View> superHud = new simVis::View;
  superHud->setName("SuperHUD");
  superHud->setUpViewAsHUD(app.mainView.get());
  viewMan->addView(superHud.get());

  // Create an inset view
  app.insetView = new simVis::View;
  app.insetView->setName("Inset");
  app.insetView->setExtentsAsRatio(0.67f, 0.67f, 0.33f, 0.33f);
  app.insetView->setSceneManager(sceneMan.get());
  app.insetView->applyManipulatorSettings(*app.mainView);
  app.mainView->addInset(app.insetView.get()); // auto-added to viewMan

  // Create several platforms
  simData::MemoryDataStore dataStore;
  simCore::ClockImpl clock;
  sceneMan->addUpdateCallback(new simExamples::IdleClockCallback(clock, dataStore));
  dataStore.bindToClock(&clock);
  simVis::ScenarioManager* scenarioManager = sceneMan->getScenario();
  scenarioManager->bind(&dataStore);
  // Add 100 platforms
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(LAT, LON, 100.0);

  // Configure a listener to attach graphics for custom rendering entities
  dataStore.addListener(simData::DataStore::ListenerPtr(new ExamplePicking::AttachRenderGraphics(scenarioManager)));

  // Seed the random number generator for more deterministic results
  srand(0);
  for (int k = 0; k < NUM_PLATFORMS; ++k)
  {
    uint64_t id = createPlatform(dataStore);
    addDataPoints(cc, dataStore, id);

    // Add custom render nodes on these iterations
    if (k == 10)
      addCustomRendering(id, dataStore, 0xFFFF0080); // Yellow
    else if (k == 59)
      addCustomRendering(id, dataStore, 0xFF804080); // Orange-ish
  }

  // Apply the interpolator
  simData::LinearInterpolator interpolator;
  dataStore.setInterpolator(&interpolator);
  dataStore.enableInterpolation(true);

  // Add a GOG file with a few shapes
  addGog(scenarioManager, sceneMan->getMapNode());

  // Start playing
  clock.setMode(simCore::Clock::MODE_REALTIME);
  clock.setStartTime(simCore::TimeStamp(dataStore.referenceYear(), 0.0));
  clock.setEndTime(simCore::TimeStamp(dataStore.referenceYear(), 600.0));
  clock.playForward();

  // Add various event handlers
  app.mainView->installDebugHandlers();
  app.mainView->addEventHandler(new MenuHandler(clock, app));
  app.insetView->addEventHandler(new MenuHandler(clock, app));
  app.mainView->addEventHandler(new simVis::ToggleOverheadMode(app.mainView.get(), 'O', 'C'));
  app.insetView->addEventHandler(new simVis::ToggleOverheadMode(app.insetView.get(), 'O', 'C'));

  // Set the initial viewpoints
  simVis::Viewpoint viewpoint;
  viewpoint.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::get("wgs84"), LON, LAT, 0.0, osgEarth::ALTMODE_ABSOLUTE);
  viewpoint.heading()->set(0.0, osgEarth::Units::DEGREES);
  viewpoint.pitch()->set(-89.0, osgEarth::Units::DEGREES);
  viewpoint.range()->set(2500, osgEarth::Units::METERS);
  app.mainView->setViewpoint(viewpoint);

  // Configure the inset to be tethered in cockpit mode
  viewpoint.pitch()->set(-15.0, osgEarth::Units::DEGREES);
  viewpoint.range()->set(15, osgEarth::Units::METERS);
  app.insetView->setViewpoint(viewpoint);
  // Turn on cockpit mode for the inset
  app.insetView->enableCockpitMode(scenarioManager->find(1));
  app.insetView->getEarthManipulator()->setHeadingLocked(true);
  app.insetView->getEarthManipulator()->setPitchLocked(false);

  // Enable highlighting for the picker
  app.highlightShader = new simVis::PickerHighlightShader(scenarioManager->getOrCreateStateSet());
  simVis::PickerHighlightShader::installShaderProgram(scenarioManager->getOrCreateStateSet(), true);

  // Add the picker itself
  if (pickType == PickIntersect)
    app.picker = new simVis::IntersectPicker(viewMan.get(), scenarioManager);
  else if (pickType == PickDynamic)
  {
    // Configure the dynamic selection picker to also pick Custom Render entities
    simUtil::DynamicSelectionPicker* dynamicPicker = new simUtil::DynamicSelectionPicker(viewMan.get(), scenarioManager);
    dynamicPicker->setPickMask(dynamicPicker->pickMask() | simVis::DISPLAY_MASK_CUSTOM_RENDERING);
    app.picker = dynamicPicker;
  }
  else // pickType == PickRtt
  {
    // Create the RTT picker
    simVis::RTTPicker* rttPicker = new simVis::RTTPicker(viewMan.get(), scenarioManager, 256);

    // Add GOG to the pickable mask
    osgEarth::Util::RTTPicker* osgEarthPicker = rttPicker->rttPicker();
    osgEarthPicker->setCullMask(osgEarthPicker->getCullMask() | simVis::DISPLAY_MASK_GOG | simVis::DISPLAY_MASK_CUSTOM_RENDERING);
    app.picker = rttPicker;

    // Make a view that lets us see what the picker sees for Main View
    app.mainRttView = new simVis::View();
    app.mainRttView->setExtentsAsRatio(0.67f, 0.f, 0.33f, 0.335f);
    app.mainView->addInset(app.mainRttView.get());
    rttPicker->setUpViewWithDebugTexture(app.mainRttView.get(), app.mainView.get());

    // Make a view that lets us see what the picker sees for Inset View
    app.insetRttView = new simVis::View();
    app.insetRttView->setExtentsAsRatio(0.67f, 0.335f, 0.33f, 0.335f);
    app.mainView->addInset(app.insetRttView.get());
    rttPicker->setUpViewWithDebugTexture(app.insetRttView.get(), app.insetView.get());
  }

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewMan->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewMan->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();

  // Because RTT requires rendering the view to a texture, ImGui would get called twice (and assert)
  // if associated with the Main View. Instead we add it to Super HUD to work around the problem.
  auto imGuiView = (pickType == PickRtt) ? superHud : app.mainView;
  imGuiView->addEventHandler(gui);

  gui->add(new ControlPanel(app, pickType == PickRtt));
#else
  app.mainView->addOverlayControl(createUi(app.pickLabel, (pickType == PickRtt)));
#endif

#ifdef HAVE_IMGUI
  app.picker->addCallback(new UpdateLabelPickCallback(app.pickLabel));
#else
  // When a new item is picked, update the label
  app.picker->addCallback(new UpdateLabelPickCallback(app.pickLabel.get()));
#endif

  // Add a popup handler to demonstrate its use of the picker
  osg::ref_ptr<simVis::PopupHandler> popupHandler = new simVis::PopupHandler(app.picker.get(), superHud.get());
  popupHandler->setPopupLocation(simVis::PopupLocation::LOWER_RIGHT);
  popupHandler->setBackColor(simVis::Color(0.f, 0.f, 0.f, 0.8f));
  popupHandler->setBorderColor(simVis::Color::Green);
  popupHandler->setTitleColor(simVis::Color::Lime);
  popupHandler->setLimitVisibility(false);
  superHud->addEventHandler(popupHandler.get());

  // Run until the user quits by hitting ESC.
  return viewMan->run();
}
