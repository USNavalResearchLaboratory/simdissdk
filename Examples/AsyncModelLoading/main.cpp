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
 * Asynchronous Model Loading Example
 *
 * Demonstrates the asynchronous loading of 3D models.
 */

#include "osg/ProxyNode"
#include "osg/ShapeDrawable"
#include "osg/Sequence"
#include "osg/ValueObject"

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/MemoryDataStore.h"
#include "simVis/ModelCache.h"
#include "simVis/Platform.h"
#include "simVis/Registry.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
using namespace osgEarth::Util::Controls;
#endif

/// Icon to use to trigger box mode
static const std::string NOT_FOUND_ICON = "does/not/exist.flt";

//----------------------------------------------------------------------------

/** Add timing to the callback that async loading uses. */
class RoundTripAsyncTimer : public simVis::ReplaceChildReadyCallback
{
public:

#ifdef HAVE_IMGUI
  RoundTripAsyncTimer(std::string& str, osg::Group* parent)
    : ReplaceChildReadyCallback(parent),
    str_(str)
  {
    str_ = "N/A";
  }
#else
  RoundTripAsyncTimer(LabelControl* label, osg::Group* parent)
    : ReplaceChildReadyCallback(parent),
    label_(label)
  {
    label_->setText("N/A");
  }
#endif

  virtual void loadFinished(const osg::ref_ptr<osg::Node>& model, bool isImage, const std::string& filename)
  {
    // Call inherited method
    ReplaceChildReadyCallback::loadFinished(model, isImage, filename);

    std::stringstream ss;
    ss << timer_.elapsedTime_m() << " ms";
#ifdef HAVE_IMGUI
    str_ = ss.str();
#else
    label_->setText(ss.str());
#endif
  }

private:
#ifdef HAVE_IMGUI
  std::string& str_;
#else
  osg::ref_ptr<LabelControl> label_;
#endif
  osg::ElapsedTime timer_;
};

//----------------------------------------------------------------------------

struct App
{
  osg::ref_ptr<simVis::View> mainView;

  /// Platform's osg::Node
  osg::ref_ptr<osg::Node> entityNode;
  /// Node for asynchronously loading models
  osg::ref_ptr<osg::MatrixTransform> asyncNode;
  /// Node for synchronously loading models
  osg::ref_ptr<osg::MatrixTransform> syncNode;

#ifdef HAVE_IMGUI
  std::string entityTimerStr = "N/A";
  std::string asyncTimerStr = "N/A";
  std::string asyncRoundTripStr = "N/A";
  std::string syncTimerStr = "N/A";
#else
  /// Various timing labels
  osg::ref_ptr<LabelControl> timingEntity;
  osg::ref_ptr<LabelControl> timingAsync;
  osg::ref_ptr<LabelControl> roundTripAsync;
  osg::ref_ptr<LabelControl> timingSync;
  /// Contains all controls
  osg::ref_ptr<Control> helpBox;

  /// Labels of the different files to load for each load type
  std::vector<osg::ref_ptr<LabelControl> > entityLabels;
  std::vector<osg::ref_ptr<LabelControl> > asyncLabels;
  std::vector<osg::ref_ptr<LabelControl> > syncLabels;
#endif

  simData::DataStore* dataStore;
  simData::ObjectId platId;

#ifdef HAVE_IMGUI
  void setEntityIcon(const std::string& filename)
  {
    // Change the icon name in the prefs
    osg::ElapsedTime timer;
    simData::DataStore::Transaction txn;
    auto* prefs = dataStore->mutable_platformPrefs(platId, &txn);
    prefs->set_icon(filename);
    txn.complete(&prefs);

    // Report the elapsed time for transaction completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    entityTimerStr = ss.str();
  }

  void setAsyncIcon(const std::string& filename)
  {
    osg::ElapsedTime timer;
    simVis::Registry* reg = simVis::Registry::instance();
    simVis::ModelCache* cache = reg->modelCache();
    cache->asyncLoad(reg->findModelFile(filename), new RoundTripAsyncTimer(asyncRoundTripStr, asyncNode.get()));

    // Report the elapsed time for loading completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    asyncTimerStr = ss.str();
  }

  void setSyncIcon(const std::string& filename)
  {
    osg::ElapsedTime timer;

    simVis::Registry* reg = simVis::Registry::instance();
    osg::ref_ptr<osg::Node> newModel = reg->getOrCreateIconModel(filename);
    // If the new model is not valid, then show a box.  Note that the registry does not do
    // this for us automatically, although it does for the asynchronous load.  This difference
    // is due to backwards compatibility concerns combined with circumstances in the ProxyNode
    // implementation that encourage use of a placeholder on failure.
    if (!newModel.valid())
    {
      osg::Geode* geode = new osg::Geode();
      geode->addDrawable(new osg::ShapeDrawable(new osg::Box()));
      newModel = geode;
    }

    syncNode->removeChildren(0, syncNode->getNumChildren());
    syncNode->addChild(newModel.get());

    // Report the elapsed time for loading completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    syncTimerStr = ss.str();
  }
#endif
};

#ifdef HAVE_IMGUI

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(App& app)
    : simExamples::SimExamplesGui("Asynchronous Loading Node Example"),
    app_(app)
  {
  }

  enum CurrentIcon
  {
    IMAGE = 0,
    MISSILE,
    TANK,
    NOTFOUND
  };

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    if (firstDraw_)
    {
      // This GUI positions bottom left instead of top left, need the size of the window
      ImVec2 viewSize = ImGui::GetMainViewport()->WorkSize;
      ImGui::SetNextWindowPos(ImVec2(15, viewSize.y - 15), 0, ImVec2(0, 1));
      firstDraw_ = false;
    }

    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("c: Center Next");

    if (ImGui::BeginTable("Table", 2))
    {
      ImGui::TableNextColumn(); ImGui::Text("Platform:"); ImGui::TableNextColumn();
      addButtons_(currentEntity_, "##entity", [this](const std::string& filename) { app_.setEntityIcon(filename); });

      ImGui::TableNextColumn(); ImGui::Text("Asynchronous:"); ImGui::TableNextColumn();
      addButtons_(currentAsync_, "##async", [this](const std::string& filename) { app_.setAsyncIcon(filename); });

      ImGui::TableNextColumn(); ImGui::Text("Synchronous:"); ImGui::TableNextColumn();
      addButtons_(currentSync_, "##sync", [this](const std::string& filename) { app_.setSyncIcon(filename); });

      ImGui::TableNextColumn();
      if (ImGui::Button("Clear Cache"))
        simVis::Registry::instance()->clearModelCache();
      ImGui::TableNextColumn(); ImGui::TableNextColumn();

      pushLargeFont_();
      ImGui::Text("Timing"); ImGui::TableNextColumn(); ImGui::TableNextColumn();
      popLargeFont_();

      ImGui::Text("Platform:"); ImGui::TableNextColumn(); ImGui::Text(app_.entityTimerStr.c_str()); ImGui::TableNextColumn();
      ImGui::Text("Asynchronous:"); ImGui::TableNextColumn(); ImGui::Text(app_.asyncTimerStr.c_str()); ImGui::TableNextColumn();
      ImGui::Text("Async Round-Trip:"); ImGui::TableNextColumn(); ImGui::Text(app_.asyncRoundTripStr.c_str()); ImGui::TableNextColumn();
      ImGui::Text("Synchronous:"); ImGui::TableNextColumn(); ImGui::Text(app_.syncTimerStr.c_str()); ImGui::TableNextColumn();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  /** Adds a row of buttons for changing the icon on a platform or node */
  void addButtons_(CurrentIcon& current, const std::string& suffix, std::function<void(const std::string&)> setter)
  {
    static const ImVec4 ACTIVE_BUTTON_COLOR(0.f, 1.f, 0.f, 0.6f);
    static const std::vector<std::string> ICONS = { EXAMPLE_IMAGE_ICON, EXAMPLE_MISSILE_ICON, EXAMPLE_TANK_ICON, NOT_FOUND_ICON };
    static const std::vector<std::string> BUTTON_NAMES = { "Image", "Missile", "Tank", "Not-Found" };

    for (size_t i = 0; i < ICONS.size(); ++i)
    {
      bool pop = false;
      if (current == static_cast<CurrentIcon>(i))
      {
        ImGui::PushStyleColor(ImGuiCol_Button, ACTIVE_BUTTON_COLOR);
        pop = true;
      }
      std::string str = BUTTON_NAMES[i] + suffix;
      if (ImGui::Button(str.c_str()))
      {
        setter(ICONS[i]);
        current = static_cast<CurrentIcon>(i);
      }
      if (pop)
        ImGui::PopStyleColor();
      ImGui::SameLine();
    }
  }

  App& app_;
  CurrentIcon currentEntity_ = MISSILE;
  CurrentIcon currentAsync_ = MISSILE;
  CurrentIcon currentSync_ = MISSILE;
};

#else
//----------------------------------------------------------------------------
/** Changes the icon name of a platform entity */
struct EntitySetter : public ControlEventHandler
{
  EntitySetter(const App& app, const std::string& filename)
    : app_(app),
      filename_(filename)
  {
  }

  virtual void onClick(Control* control)
  {
    for (auto i = app_.entityLabels.begin(); i != app_.entityLabels.end(); ++i)
      (*i)->setForeColor(simVis::Color::White);
    control->setForeColor(simVis::Color::Lime);

    // Change the icon name in the prefs
    osg::ElapsedTime timer;
    simData::DataStore::Transaction txn;
    auto* prefs = app_.dataStore->mutable_platformPrefs(app_.platId, &txn);
    prefs->set_icon(filename_);
    txn.complete(&prefs);

    // Report the elapsed time for transaction completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    app_.timingEntity->setText(ss.str());
  }

  const App& app_;
  std::string filename_;
};

//----------------------------------------------------------------------------
/** Changes the icon asynchronously on a node */
struct AsyncSetter : public ControlEventHandler
{
  AsyncSetter(const App& app, const std::string& filename)
    : app_(app),
      filename_(filename)
  {
  }

  virtual void onClick(Control* control)
  {
    for (auto i = app_.asyncLabels.begin(); i != app_.asyncLabels.end(); ++i)
      (*i)->setForeColor(simVis::Color::White);
    control->setForeColor(simVis::Color::Lime);

    osg::ElapsedTime timer;
    simVis::Registry* reg = simVis::Registry::instance();
    simVis::ModelCache* cache = reg->modelCache();
    cache->asyncLoad(reg->findModelFile(filename_), new RoundTripAsyncTimer(app_.roundTripAsync.get(), app_.asyncNode.get()));

    // Report the elapsed time for loading completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    app_.timingAsync->setText(ss.str());
  }

  const App& app_;
  std::string filename_;
};

//----------------------------------------------------------------------------
/** Changes the icon synchronously on a node */
struct SyncSetter : public ControlEventHandler
{
  SyncSetter(const App& app, const std::string& filename)
    : app_(app),
      filename_(filename)
  {
  }

  virtual void onClick(Control* control)
  {
    for (auto i = app_.syncLabels.begin(); i != app_.syncLabels.end(); ++i)
      (*i)->setForeColor(simVis::Color::White);
    control->setForeColor(simVis::Color::Lime);

    osg::ElapsedTime timer;

    simVis::Registry* reg = simVis::Registry::instance();
    osg::ref_ptr<osg::Node> newModel = reg->getOrCreateIconModel(filename_);
    // If the new model is not valid, then show a box.  Note that the registry does not do
    // this for us automatically, although it does for the asynchronous load.  This difference
    // is due to backwards compatibility concerns combined with circumstances in the ProxyNode
    // implementation that encourage use of a placeholder on failure.
    if (!newModel.valid())
    {
      osg::Geode* geode = new osg::Geode();
      geode->addDrawable(new osg::ShapeDrawable(new osg::Box()));
      newModel = geode;
    }

    app_.syncNode->removeChildren(0, app_.syncNode->getNumChildren());
    app_.syncNode->addChild(newModel.get());

    // Report the elapsed time for loading completion
    std::stringstream ss;
    ss << timer.elapsedTime_m() << " ms";
    app_.timingSync->setText(ss.str());
  }

  const App& app_;
  std::string filename_;
};

//----------------------------------------------------------------------------
/** Clears the model cache when clicked */
struct ClearCacheHandler : public ControlEventHandler
{
  virtual void onClick(Control* control)
  {
    simVis::Registry::instance()->clearModelCache();
  }
};

//----------------------------------------------------------------------------
LabelControl* addEntityLabel(App& app, const std::string& text, const simVis::Color& color, const std::string& filename)
{
  LabelControl* c = new LabelControl(text, 14, color);
  c->addEventHandler(new EntitySetter(app, filename));
  app.entityLabels.push_back(c);
  return c;
}

LabelControl* addAsyncLabel(App& app, const std::string& text, const simVis::Color& color, const std::string& filename)
{
  LabelControl* c = new LabelControl(text, 14, color);
  c->addEventHandler(new AsyncSetter(app, filename));
  app.asyncLabels.push_back(c);
  return c;
}

LabelControl* addSyncLabel(App& app, const std::string& text, const simVis::Color& color, const std::string& filename)
{
  LabelControl* c = new LabelControl(text, 14, color);
  c->addEventHandler(new SyncSetter(app, filename));
  app.syncLabels.push_back(c);
  return c;
}

/// create an overlay with some helpful information
Control* createHelp(App& app)
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->setMargin(10);
  vbox->setVertAlign(Control::ALIGN_BOTTOM);

  vbox->addControl(new LabelControl("Asynchronous Loading Node Example", 20, simVis::Color::Yellow));
  vbox->addControl(new LabelControl("c: Center Next", 14.f, simVis::Color::Silver));

  Grid* iconGrid = vbox->addControl(new Grid);
  iconGrid->setControl(0, 0, new LabelControl("Platform:", simVis::Color::Silver, 14.f));
  iconGrid->setControl(1, 0, addEntityLabel(app, "Image", simVis::Color::White, EXAMPLE_IMAGE_ICON));
  iconGrid->setControl(2, 0, addEntityLabel(app, "Missile", simVis::Color::Lime, EXAMPLE_MISSILE_ICON));
  iconGrid->setControl(3, 0, addEntityLabel(app, "Tank", simVis::Color::White, EXAMPLE_TANK_ICON));
  iconGrid->setControl(4, 0, addEntityLabel(app, "Not-Found", simVis::Color::White, NOT_FOUND_ICON));

  iconGrid->setControl(0, 1, new LabelControl("Asynchronous:", simVis::Color::Silver, 14.f));
  iconGrid->setControl(1, 1, addAsyncLabel(app, "Image", simVis::Color::White, EXAMPLE_IMAGE_ICON));
  iconGrid->setControl(2, 1, addAsyncLabel(app, "Missile", simVis::Color::Lime, EXAMPLE_MISSILE_ICON));
  iconGrid->setControl(3, 1, addAsyncLabel(app, "Tank", simVis::Color::White, EXAMPLE_TANK_ICON));
  iconGrid->setControl(4, 1, addAsyncLabel(app, "Not-Found", simVis::Color::White, NOT_FOUND_ICON));

  iconGrid->setControl(0, 2, new LabelControl("Synchronous:", simVis::Color::Silver, 14.f));
  iconGrid->setControl(1, 2, addSyncLabel(app, "Image", simVis::Color::White, EXAMPLE_IMAGE_ICON));
  iconGrid->setControl(2, 2, addSyncLabel(app, "Missile", simVis::Color::Lime, EXAMPLE_MISSILE_ICON));
  iconGrid->setControl(3, 2, addSyncLabel(app, "Tank", simVis::Color::White, EXAMPLE_TANK_ICON));
  iconGrid->setControl(4, 2, addSyncLabel(app, "Not-Found", simVis::Color::White, NOT_FOUND_ICON));

  auto* clearButton = vbox->addControl(new ButtonControl("Clear Cache", new ClearCacheHandler));
  clearButton->setFontSize(14.f);

  LabelControl* timingLabel = vbox->addControl(new LabelControl("Timing", 16.f, simVis::Color::Yellow));
  timingLabel->setMargin(Control::SIDE_TOP, 10.f);

  Grid* timingGrid = vbox->addControl(new Grid);
  timingGrid->setControl(0, 0, new LabelControl("Platform:", 14.f, simVis::Color::Silver));
  app.timingEntity = timingGrid->setControl(1, 0, new LabelControl("N/A", 14.f, simVis::Color::Silver));

  timingGrid->setControl(0, 1, new LabelControl("Asynchronous:", 14.f, simVis::Color::Silver));
  app.timingAsync = timingGrid->setControl(1, 1, new LabelControl("N/A", 14.f, simVis::Color::Silver));
  timingGrid->setControl(0, 2, new LabelControl("Async Round-Trip:", 14.f, simVis::Color::Silver));
  app.roundTripAsync = timingGrid->setControl(1, 2, new LabelControl("N/A", 14.f, simVis::Color::Silver));

  timingGrid->setControl(0, 3, new LabelControl("Synchronous:", 14.f, simVis::Color::Silver));
  app.timingSync = timingGrid->setControl(1, 3, new LabelControl("N/A", 14.f, simVis::Color::Silver));

  app.helpBox = vbox;
  return vbox;
}
#endif

//----------------------------------------------------------------------------
struct MenuHandler : public osgGA::GUIEventHandler
{
  explicit MenuHandler(const App& app)
    : app_(app)
  {
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
#ifndef HAVE_IMGUI
      case '?': // toggle help
        app_.helpBox->setVisible(!app_.helpBox->visible());
        return true;
#endif
      case 'c':
        tetherNext_();
        return true;
      }
    }

    return false;
  }

private: // data

  /** Cycles through 'center' entity */
  void tetherNext_() const
  {
    auto vp = app_.mainView->getViewpoint();

    osg::ref_ptr<osg::Node> tether = vp.getNode();

    // Entity -> async -> sync
    if (tether == app_.entityNode)
      vp.setNode(app_.asyncNode.get());
    else if (tether == app_.asyncNode)
      vp.setNode(app_.syncNode.get());
    else
      vp.setNode(app_.entityNode.get());
    app_.mainView->setViewpoint(vp);
  }

  const App& app_;
};

//----------------------------------------------------------------------------

simData::ObjectId createPlatform(simData::DataStore& dataStore)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
  simData::ObjectId id = props->id();
  xaction.complete(&props);
  return id;
}

//----------------------------------------------------------------------------

simUtil::SimulatorEventHandler* createSimulation(simUtil::PlatformSimulatorManager& simMgr, simData::ObjectId obj1)
{
  osg::ref_ptr<simUtil::PlatformSimulator> sim1 = new simUtil::PlatformSimulator(obj1);
  sim1->addWaypoint(simUtil::Waypoint(51.5, 0.5, 40000, 200.0)); // London
  sim1->addWaypoint(simUtil::Waypoint(38.8, -77.0, 40000, 200.0)); // DC
  sim1->setSimulateRoll(false);
  sim1->setSimulatePitch(false);
  simMgr.addSimulator(sim1.get());

  // Run the simulations:
  simMgr.simulate(0.0, 120.0, 60.0);

  return new simUtil::SimulatorEventHandler(&simMgr, 0.0, 120.0);
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  simCore::checkVersionThrow();
  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // use the utility code to create a basic world map (terrain imagery and height)
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // SDK viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->getViewer()->setThreadingModel(osgViewer::ViewerBase::ThreadingModel::SingleThreaded);
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // data source which will provide positions for the platform
  // based on the simulation time.
  // (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;
  App app;
  app.dataStore = &dataStore;

  // bind dataStore to the scenario manager
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&dataStore);

  // Create a platform to visualize:
  app.platId = createPlatform(dataStore);

  {
    // Set up and apply preferences for the platform
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(app.platId, &txn);
    prefs->set_dynamicscale(true);
    prefs->set_nodepthicons(false);
    prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_POINT);
    prefs->mutable_trackprefs()->set_linewidth(1);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->mutable_commonprefs()->set_name("Platform");
    prefs->set_icon(EXAMPLE_MISSILE_ICON);
    txn.complete(&prefs);
  }

  // Set up a simulation for our two platforms.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  viewer->addEventHandler(createSimulation(*simMgr, app.platId));

  // Tether camera to platform
  osg::observer_ptr<simVis::EntityNode> obj1Node = scene->getScenario()->find(app.platId);
  app.entityNode = obj1Node->getChild(0); // First child is the platform model node
  app.mainView = viewer->getMainView();

  // Create a node that will serve as wing station, that is not a platform
  osg::ref_ptr<osg::Node> missileIcon = simVis::Registry::instance()->getOrCreateIconModel(EXAMPLE_MISSILE_ICON);
  osg::MatrixTransform* asyncTransform = new osg::MatrixTransform;
  asyncTransform->setMatrix(osg::Matrix::translate(osg::Vec3f(10.f, 8.f, 0.f)));
  asyncTransform->addChild(missileIcon.get());
  obj1Node->attach(asyncTransform);
  app.asyncNode = asyncTransform;

  // Create a second node wing station for synchronous loads
  osg::MatrixTransform* syncTransform = new osg::MatrixTransform;
  syncTransform->setMatrix(osg::Matrix::translate(osg::Vec3f(10.f, -8.f, -0.5f)));
  syncTransform->addChild(missileIcon.get());
  obj1Node->attach(syncTransform);
  app.syncNode = syncTransform;

  // Turn on lighting
  simVis::setLighting(obj1Node->getOrCreateStateSet(), osg::StateAttribute::ON);

  // set the camera to look at the platform
  app.mainView->tetherCamera(app.entityNode.get());
  app.mainView->setFocalOffsets(180, -15, 30);

  // handle key press events
  viewer->addEventHandler(new MenuHandler(app));

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  // show the instructions overlay
  app.mainView->addOverlayControl(createHelp(app));
#endif

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}
