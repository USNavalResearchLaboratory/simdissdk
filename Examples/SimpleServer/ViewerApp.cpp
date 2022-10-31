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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/ArgumentParser"
#include "osgGA/GUIEventHandler"
#include "osgViewer/ViewerEventHandlers"
#include "osgEarth/ScreenSpaceLayout"
#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif
#include "simCore/GOG/Parser.h"
#include "simCore/String/UtfUtils.h"
#include "simCore/Time/ClockImpl.h"
#include "simCore/Time/Utils.h"
#include "simData/DataStoreProxy.h"
#include "simData/LinearInterpolator.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Compass.h"
#include "simVis/OverheadMode.h"
#include "simVis/Platform.h"
#include "simVis/Popup.h"
#include "simVis/Registry.h"
#include "simVis/SceneManager.h"
#include "simVis/Scenario.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Loader.h"
#include "simUtil/DefaultDataStoreValues.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/HudManager.h"
#include "simUtil/Replaceables.h"
#include "simUtil/StatusText.h"
#include "InstallOcean.h"
#include "DataEngine.h"
#include "ViewerApp.h"

namespace SimpleServer {

namespace ui = osgEarth::Util::Controls;

//////////////////////////////////////////////////////////////////

static const std::string TITLE = "Simple Server SDK Example";
static const std::string HELP_TEXT =
  "c : Cycle centered platform\n"
  "C : Toggle overhead clamping\n"
  "d : Toggle dynamic scale\n"
  "D : Toggle label declutter on/off\n"
  "l : Toggle Logarithmic Depth Buffer\n"
  "n : Toggle labels\n"
  "o : Cycle time format\n"
  "O : Toggle overhead mode\n"
  "p : Play/pause\n"
  "s : Cycle OSG statistics\n"
  "t : Toggle declutter technique\n"
  "T : Cycle callout line style\n"
  "w : Toggle compass\n"
  "z : Toggle cockpit mode (if centered)\n"
  ;

//////////////////////////////////////////////////////////////////

#ifndef HAVE_IMGUI
/** Handles various shortcuts from OSG and activates features in the Viewer App */
class Shortcuts : public osgGA::GUIEventHandler
{
public:
  explicit Shortcuts(ViewerApp& app)
    : app_(app)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case osgGA::GUIEventAdapter::KEY_F4:
        if ((ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT) != 0)
        {
          app_.exit();
          return true;
        }
        break;
      case 'c': // lowercase
        app_.centerNext();
        return true;
      case 'd':
        app_.toggleDynamicScale();
        return true;
      case 'n':
        app_.toggleLabels();
        return true;
      case 'w':
        app_.toggleCompass();
        return true;
      case 'l':
        app_.toggleLogDb();
        return true;
      case 'o': // lowercase
        app_.cycleTimeFormat();
        return true;
      case 'z':
        app_.toggleCockpit();
        return true;
      case 'p':
        app_.playPause();
        return true;
      case 'D':
        app_.toggleTextDeclutter();
        return true;
      case 't':
        app_.toggleDeclutterTechnique();
        return true;
      case 'T':
        app_.cycleCalloutLineStyle();
        return true;
      }
    }
    return false;
  }

private:
  ViewerApp& app_;
};
#endif

//////////////////////////////////////////////////////////////////

#ifdef HAVE_IMGUI
struct TestPanel : public simExamples::SimExamplesGui
{
  explicit TestPanel(ViewerApp& app)
    : simExamples::SimExamplesGui("Simple Server SDK Example"),
    app_(app)
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

    ImGui::Text("c : Cycle centered platform");
    ImGui::Text("C : Toggle overhead clamping");
    ImGui::Text("d : Toggle dynamic scale");
    ImGui::Text("D : Toggle label declutter on/off");
    ImGui::Text("l : Toggle Logarithmic Depth Buffer");
    ImGui::Text("n : Toggle labels");
    ImGui::Text("o : Cycle time format");
    ImGui::Text("O : Toggle overhead mode");
    ImGui::Text("p : Play/pause");
    ImGui::Text("s : Cycle OSG statistics");
    ImGui::Text("t : Toggle declutter technique");
    ImGui::Text("T : Cycle callout line style");
    ImGui::Text("w : Toggle compass");
    ImGui::Text("z : Toggle cockpit mode (if centered)");

    auto& io = ImGui::GetIO();
    if (io.InputQueueCharacters.size() > 0)
    {
      switch (io.InputQueueCharacters.front())
      {
      case 'c': // lowercase
        app_.centerNext();
        break;
      case 'd':
        app_.toggleDynamicScale();
        break;
      case 'n':
        app_.toggleLabels();
        break;
      case 'w':
        app_.toggleCompass();
        break;
      case 'l':
        app_.toggleLogDb();
        break;
      case 'o': // lowercase
        app_.cycleTimeFormat();
        break;
      case 'z':
        app_.toggleCockpit();
        break;
      case 'p':
        app_.playPause();
        break;
      case 'D':
        app_.toggleTextDeclutter();
        break;
      case 't':
        app_.toggleDeclutterTechnique();
        break;
      case 'T':
        app_.cycleCalloutLineStyle();
        break;
      }
    }

    ImGui::End();
  }

private:
  ViewerApp& app_;
  ImFont* font_ = nullptr;
};
#endif

//////////////////////////////////////////////////////////////////

ViewerApp::ViewerApp(osg::ArgumentParser& args)
  : clock_(nullptr),
    textReplacer_(new simCore::TextReplacer),
    dataStore_(nullptr),
    interpolator_(nullptr),
    timeVariable_(nullptr),
    declutterOn_(false),
    colorIndex_(0)
{
  init_(args);
}

ViewerApp::~ViewerApp()
{
  engine_ = nullptr;
  delete dataStore_;
}

void ViewerApp::init_(osg::ArgumentParser& args)
{
  // Set up OSG features if supported
  if (args.read("--multisample"))
    osg::DisplaySettings::instance()->setNumMultiSamples(4);

  // First we need a map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // A scene manager that all our views will share.
  sceneManager_ = new simVis::SceneManager();
  sceneManager_->setMap(map.get());

  // We need a view manager. This handles all of our Views.
  viewManager_ = new simVis::ViewManager(args);

  // Set up the logarithmic depth buffer for all views
  logDb_ = new simVis::ViewManagerLogDbAdapter;
  logDb_->install(viewManager_.get());

  // Create views and connect them to our scene.
  osg::ref_ptr<simVis::View> mainView = new simVis::View();
  mainView->setSceneManager(sceneManager_.get());
  mainView->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  mainView->setUpViewInWindow(100, 100, 1024, 768);
  // Set a decent number of threads for paging terrain
  mainView->getDatabasePager()->setUpThreads(6, 4);
  mainView->addEventHandler(new simVis::ToggleOverheadMode(mainView.get(), 'O', 'C'));

  mainView->addEventHandler(new simVis::PopupHandler(sceneManager_.get()));

  // Add it to the view manager
  viewManager_->addView(mainView.get());

  // Create the SuperHUD
  superHud_ = new simVis::View;
  superHud_->setUpViewAsHUD(mainView.get());
  viewManager_->addView(superHud_.get());

  // Create a default data store, then wrap it with a proxy
  simData::MemoryDataStore* dataStoreImpl = new simData::MemoryDataStore();
  // Override some platform defaults
  simData::PlatformPrefs platformPrefs;
  platformPrefs.mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  platformPrefs.set_dynamicscale(true);
  platformPrefs.set_dynamicscalescalar(0.4); // increase size of icons for improved visibility
  dataStore_ = new simData::DataStoreProxy(dataStoreImpl);
  dataStore_->setDefaultPrefs(platformPrefs);

  // Apply the interpolator
  interpolator_ = new simData::LinearInterpolator;
  dataStore_->setInterpolator(interpolator_);
  dataStore_->enableInterpolation(true);

  // Create the timing mechanisms
  clock_ = new simCore::ClockImpl;
  simVis::Registry::instance()->setClock(clock_);
  clock_->setMode(simCore::Clock::MODE_FREEWHEEL, simCore::TimeStamp(1970, simCore::getSystemTime()));

  // Bind the data store to the scenario manager
  sceneManager_->getScenario()->bind(dataStore_);

  // Turn on data limiting because we are expecting live data
  dataStore_->bindToClock(clock_);
  dataStore_->setDataLimiting(true);

  // Set up a decent initial view
  simVis::Viewpoint vp;
  vp.focalPoint() = osgEarth::GeoPoint(osgEarth::SpatialReference::create("wgs84"), -158.996, 22.0055, 0.0, osgEarth::ALTMODE_ABSOLUTE);
  vp.heading()->set(-79, osgEarth::Units::DEGREES);
  vp.pitch()->set(-27.75, osgEarth::Units::DEGREES);
  vp.range()->set(1200, osgEarth::Units::METERS);
  mainView->setViewpoint(vp);

  // Create the compass and have it use the main view
  compass_ = new simVis::Compass("compass.png");
  compass_->setDrawView(mainView.get());
  compass_->setActiveView(mainView.get());

  // Install an ocean
  InstallOcean installOcean;
  installOcean.set(args);
  installOcean.install(*sceneManager_);

  // Install a sky node
  if (!args.read("--nosky"))
    simExamples::addDefaultSkyNode(sceneManager_.get());
  // Update the scene manager with clock time
  clock_->registerTimeCallback(simCore::Clock::TimeObserverPtr(new simExamples::SkyNodeTimeUpdater(sceneManager_.get())));

  // Update the clock on an event callback
  sceneManager_->addUpdateCallback(new simExamples::IdleClockCallback(*clock_, *dataStore_));

#ifndef HAVE_IMGUI
  // Tie in our keyboard shortcuts
  sceneManager_->addEventCallback(new Shortcuts(*this));
#endif

  // Create the data engine, which generates its own data and puts it into the data store
  engine_ = new DataEngine(*dataStore_, *sceneManager_->getScenario());

#ifndef HAVE_IMGUI
  // Create Help overlay
  mainView->addOverlayControl(createHelp_());
#endif

  // Configure the variable replacement for status text
  timeVariable_ = new simUtil::TimeVariable(*clock_);
  timeVariable_->setFormat(simCore::TIMEFORMAT_ORDINAL);
  textReplacer_->addReplaceable(timeVariable_);
  textReplacer_->addReplaceable(new simUtil::AzimuthVariable(mainView.get()));
  textReplacer_->addReplaceable(new simUtil::ElevationVariable(mainView.get()));
  textReplacer_->addReplaceable(new simUtil::LatitudeVariable(mainView.get(), 6));
  textReplacer_->addReplaceable(new simUtil::LongitudeVariable(mainView.get(), 6));
  textReplacer_->addReplaceable(new simUtil::AltitudeVariable(mainView.get()));
  textReplacer_->addReplaceable(new simUtil::CenteredVariable(mainView.get()));

  // Create status text
  cornerStatus_ = new simUtil::StatusText(superHud_.get(), textReplacer_, simUtil::StatusText::LEFT_BOTTOM);
  cornerStatus_->setStatusSpec(
    "Time:\t%TIME% \n"
    "Azimuth:\t%AZ% \n"
    "Elevation:\t%EL% \n"
    "Range:\t%ALT% \n"
    "Latitude:\t%LAT% \n"
    "Longitude:\t%LON% \n"
    "Centered:\t%CENTERED% \n");

  // Add a FPS counter
  osgViewer::StatsHandler* stats = new osgViewer::StatsHandler;
  stats->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_S);
  stats->getCamera()->setAllowEventFocus(false);
  simVis::fixStatsHandlerGl2BlockyText(stats);
  mainView->addEventHandler(stats);

  // Load missile GOGs
  loadGog_(EXAMPLE_GOG_MISSILE_LL);
  loadGog_(EXAMPLE_GOG_MISSILE_LLA);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewManager_->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewManager_->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  mainView->getEventHandlers().push_front(gui);

  gui->add(new TestPanel(*this));
#endif
}

int ViewerApp::run()
{
  return viewManager_->run();
}

void ViewerApp::exit()
{
  ::exit(0);
}

void ViewerApp::toggleDynamicScale()
{
  bool seenFirst = false;
  bool toggleOn = true;
  simData::DataStore::IdList list;
  dataStore_->idList(&list, simData::PLATFORM);
  simData::DataStore::Transaction t;
  simData::PlatformPrefs* prefs;

  // Loop through each platform
  for (simData::DataStore::IdList::const_iterator iter = list.begin();
    iter != list.end(); ++iter)
  {
    prefs = dataStore_->mutable_platformPrefs(*iter, &t);
    if (!seenFirst)
    {
      seenFirst = true;
      toggleOn = !prefs->dynamicscale();
    }
    prefs->set_dynamicscale(toggleOn);
    t.complete(&prefs);
  }
}

void ViewerApp::toggleLabels()
{
  bool seenFirst = false;
  bool toggleOn = true;
  simData::DataStore::IdList list;
  dataStore_->idList(&list, simData::PLATFORM);
  simData::DataStore::Transaction t;
  simData::CommonPrefs* prefs;

  // Loop through each platform
  for (simData::DataStore::IdList::const_iterator iter = list.begin();
    iter != list.end(); ++iter)
  {
    prefs = dataStore_->mutable_commonPrefs(*iter, &t);

    if (!seenFirst)
    {
      seenFirst = true;
      toggleOn = !prefs->labelprefs().draw();
    }
    prefs->mutable_labelprefs()->set_draw(toggleOn);
    t.complete(&prefs);
  }
}

void ViewerApp::centerNext()
{
  // Figure out what the current tether node's ID is
  simVis::View* view = viewManager_->getView(0);
  simVis::EntityNode* tetherNode = view->getEntityNode(view->getCameraTether());
  simData::ObjectId lastTetherId = 0;
  if (tetherNode != nullptr)
  {
    lastTetherId = tetherNode->getId();
  }

  // Pull the data store's platform list so we can find the next ID in the list
  simData::DataStore::IdList list;
  dataStore_->idList(&list, simData::PLATFORM);
  if (list.empty())
    return;

  // Find the next item in list
  simData::DataStore::IdList::const_iterator iter = std::upper_bound(list.begin(), list.end(), lastTetherId);
  if (iter == list.end())
    iter = list.begin();

  // Center on that item.  Note that in a real scenario you might want to check to
  // see if the platform has valid time data, is drawn, or other criteria.
  simVis::PlatformNode* plat = sceneManager_->getScenario()->find<simVis::PlatformNode>(*iter);
  if (plat)
    view->tetherCamera(plat);
}

void ViewerApp::toggleCockpit()
{
  // Figure out what the current tether node is
  simVis::View* view = viewManager_->getView(0);
  simVis::EntityNode* tetherNode = view->getEntityNode(view->getCameraTether());
  if (tetherNode == nullptr)
    return;

  if (view->getCameraTether() && !view->isCockpitEnabled())
  {
    view->enableCockpitMode(view->getCameraTether());
    simVis::Viewpoint vp(view->getViewpoint());
    vp.heading()->set(0.0, osgEarth::Units::DEGREES);
    vp.pitch()->set(0.0, osgEarth::Units::DEGREES);
    vp.range()->set(-1.0, osgEarth::Units::METERS);
    vp.positionOffset() = osg::Vec3();
    view->setViewpoint(vp);
    return;
  }
  view->enableCockpitMode(nullptr);
}

void ViewerApp::playPause()
{
  if (clock_->timeScale() == 0.0)
    clock_->setTimeScale(1.0);
  else
    clock_->setTimeScale(0.0);
}

void ViewerApp::toggleCompass()
{
  if (compass_->drawView())
    compass_->removeFromView();
  else
    compass_->setDrawView(viewManager_->getView(0));
}

void ViewerApp::toggleLogDb()
{
  if (logDb_->isInstalled())
    logDb_->uninstall(viewManager_.get());
  else
    logDb_->install(viewManager_.get());
}

void ViewerApp::cycleTimeFormat()
{
  if (timeVariable_ == nullptr)
    return;
  timeVariable_->cycleFormat();
}

ui::Control* ViewerApp::createHelp_() const
{
  // vbox is returned to caller, memory owned by caller
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(TITLE, 20.f, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(HELP_TEXT, 14, simVis::Color::Silver));
  // Move it down just a bit
  vbox->setPosition(10, 10);
  return vbox;
}

int ViewerApp::loadGog_(const std::string& filename)
{
  // Set up a search path that looks in SIMDIS_SDK-Data
  osg::ref_ptr<osgDB::Options> opts = new osgDB::Options();
  opts->setDatabasePath(simExamples::getSampleDataPath() + "/gog");
  std::string found = osgDB::findDataFile(filename, opts.get());
  if (found.empty())
    return 1;

  // Load the GOG
  std::ifstream is(simCore::streamFixUtf8(found));
  if (!is.is_open())
    return 1;

  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser, sceneManager_->getMapNode());
  loader.setReferencePosition(simCore::GOG::BSTUR);

  simVis::GOG::Loader::GogNodeVector gogs;
  loader.loadGogs(is, filename, false, gogs);
  for (auto gog : gogs)
    sceneManager_->getScenario()->addChild(gog->osgNode());

  return 1;
}

void ViewerApp::toggleTextDeclutter()
{
  declutterOn_ = !declutterOn_;
  osgEarth::ScreenSpaceLayout::setDeclutteringEnabled(declutterOn_);
  std::cout << "Decluttering " << (declutterOn_ ? "enabled" : "disabled") << "\n";
}

void ViewerApp::toggleDeclutterTechnique()
{
  auto opts = osgEarth::ScreenSpaceLayout::getOptions();
  if (opts.technique().get() == osgEarth::ScreenSpaceLayoutOptions::TECHNIQUE_LABELS)
  {
    opts.technique() = osgEarth::ScreenSpaceLayoutOptions::TECHNIQUE_CALLOUTS;
    std::cout << "Decluttering technique set to Callouts.\n";
    // Note that you can set debug of callouts with environment variable OSGEARTH_DECLUTTER_DEBUG=1
  }
  else
  {
    opts.technique() = osgEarth::ScreenSpaceLayoutOptions::TECHNIQUE_LABELS;
    std::cout << "Decluttering technique set to Labels.\n";
  }
  osgEarth::ScreenSpaceLayout::setOptions(opts);
}

void ViewerApp::cycleCalloutLineStyle()
{
  // Cycle the color index
  if (++colorIndex_ > 4)
    colorIndex_ = 0;

  auto opts = osgEarth::ScreenSpaceLayout::getOptions();
  switch (colorIndex_)
  {
  case 0:
    opts.leaderLineColor() = osgEarth::Color::White;
    std::cout << "Setting color to white.\n";
    break;
  case 1:
    opts.leaderLineColor() = osgEarth::Color::Yellow;
    std::cout << "Setting color to yellow.\n";
    break;
  case 2:
    opts.leaderLineColor() = osgEarth::Color::Red;
    std::cout << "Setting color to red.\n";
    break;
  case 3:
    opts.leaderLineColor() = osgEarth::Color::Lime;
    std::cout << "Setting color to lime.\n";
    break;
  case 4:
    opts.leaderLineColor() = osgEarth::Color::Magenta;
    std::cout << "Setting color to magenta.\n";
    break;
  }
  osgEarth::ScreenSpaceLayout::setOptions(opts);
}

}
