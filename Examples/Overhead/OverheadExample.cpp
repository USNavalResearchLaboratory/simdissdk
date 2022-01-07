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
 * Overhead Example provides a tool for testing overhead mode functionality. A framework for demonstrating various overhead mode
 * combinations, including cases where insets and the main view have different overhead mode states.
 */

#include "osgEarth/NodeUtils"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/NavigationModes.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/DbConfigurationFile.h"
#include "simUtil/MouseDispatcher.h"
#include "simUtil/MousePositionManipulator.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

#define LC "[Overhead Example] "

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif

//----------------------------------------------------------------------------

static const double START_TIME = 0.0;
static const double END_TIME = 200.0;

static std::string s_title = " \n \nOverhead Example";
static std::string s_help =
  "o : toggle overhead mode in focused view \n"
  "i : toggles the mode for creating a new inset\n"
  "v : toggle visibility of all insets\n"
  "r : remove all insets \n"
  "c : center on next platform in focused view\n"
  "n : toggle labels for all platforms\n"
  "d : toggle dynamic scale for all platforms\n";


void loadEarthFile(const std::string& earthFile, simVis::Viewer& viewer)
{
  // Load the map -- note use of readEarthFile() to configure default options (vs osgDB::readNodeFile() directly)
  osg::ref_ptr<osg::Node> loadedModel = simUtil::DbConfigurationFile::readEarthFile(earthFile);

  // Find the MapNode and replace it.
  osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(loadedModel.get());
  if (mapNode.valid())
    viewer.setMapNode(mapNode.get());
}

simData::ObjectId getCenteredPlatformId(const simVis::View* view)
{
  osg::Node* tether = view->getCameraTether();
  if (tether == nullptr)
    return 0;
  else
  {
    simVis::PlatformModelNode* model = dynamic_cast<simVis::PlatformModelNode*>(tether);
    assert(model != nullptr);

    std::vector<osg::Group*> parents = model->getParents();
    for (auto iter = parents.begin(); iter != parents.end(); ++iter)
    {
      simVis::PlatformNode* entity = dynamic_cast<simVis::PlatformNode*>(*iter);
      if (entity)
        return entity->getId();
    }
  }
  return 0;
}

//----------------------------------------------------------------------------
// A mouse position listener to update the elevation label with the current lat/lon/elevation value under the mouse
class LatLonElevListener : public simUtil::MousePositionManipulator::Listener
{
public:
  LatLonElevListener()
    : lastLat_(0.),
      lastLon_(0.),
      lastElev_(0.)
  {
  }

  double lat() const { return lastLat_;  }
  double lon() const { return lastLon_; }
  double elev() const { return lastElev_; }

  virtual void mouseOverLatLon(double lat, double lon, double elev)
  {
    lastLat_ = lat;
    lastLon_ = lon;
    lastElev_ = elev;
  }

private:
  double lastLat_;
  double lastLon_;
  double lastElev_;
};

#ifdef HAVE_IMGUI
struct ControlPanel : public GUI::BaseGui
{
  ControlPanel(simVis::Viewer* viewer, simVis::CreateInsetEventHandler* handler,
    const LatLonElevListener* latLonElevListener, simData::DataStore& dataStore,
    simData::ObjectId centeredPlat, bool showElevation)
    : BaseGui("Overhead Example"),
    viewer_(viewer),
    handler_(handler),
    latLonElevListener_(latLonElevListener),
    dataStore_(dataStore),
    centeredPlat_(centeredPlat),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::Text("o : toggle overhead mode in focused view");
    ImGui::Text("i : toggle mode for creating a new inset");
    ImGui::Text("v : toggle visibility of all insets");
    ImGui::Text("r : remove all insets");
    ImGui::Text("c : center on next platform in focused view");
    ImGui::Text("n : toggle labels for all platforms");
    ImGui::Text("d : toggle dynamic scale for all platforms");

    ImGui::Separator();

    std::stringstream ss;
    ss << "Dynamic Scale: " << (dynamicScaleOn_ ? "ON" : "OFF");
    ImGui::Text(ss.str().c_str()); ss.str(""); ss.clear();

    const simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();
    ss << std::fixed << std::setprecision(2) << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m";
    ImGui::Text(ss.str().c_str()); ss.str(""); ss.clear();

    ss << "Centered: ";
    centeredPlat_ = getCenteredPlatformId(focusedView);
    if (centeredPlat_ == 0)
      ss << "NONE";
    else
    {
      // now get centered entity's name
      simData::DataStore::Transaction tn;
      const simData::PlatformPrefs* prefs = dataStore_.platformPrefs(centeredPlat_, &tn);
      if (prefs)
        ss << prefs->commonprefs().name();
    }
    ImGui::Text(ss.str().c_str()); ss.str(""); ss.clear();

    ss << "Focused View: " << focusedView->getName() << (focusedView->isOverheadEnabled() ? " OVERHEAD" : " PERSPECTIVE");
    ImGui::Text(ss.str().c_str()); ss.str(""); ss.clear();

    // Avoid showing the sentinel value for off-map
    if (latLonElevListener_->lat() == simUtil::MousePositionManipulator::INVALID_POSITION_VALUE)
    {
      ss << "Mouse lat: ---, lon: ---";
      if (showElevation_)
        ss << ", elev: ---";
    }
    else
    {
      ss << "Mouse lat: " << latLonElevListener_->lat() << ", lon: " << latLonElevListener_->lon();
      if (showElevation_)
        ss << ", elev: " << latLonElevListener_->elev();
    }
    ImGui::Text(ss.str().c_str());

    auto& io = ImGui::GetIO();
    if (io.InputQueueCharacters.size() > 0)
    {
      switch (io.InputQueueCharacters.front())
      {
      case 'o':
      {
        simVis::View* curView = viewer_->getMainView()->getFocusManager()->getFocusedView();
        if (curView)
          curView->enableOverheadMode(!curView->isOverheadEnabled());
        break;
      }
      case 'i':
        insertViewPortMode_ = !insertViewPortMode_;
        handler_->setEnabled(insertViewPortMode_);
        break;
      case 'v':
      {
        simVis::View* main = viewer_->getMainView();
        for (unsigned i = 0; i < main->getNumInsets(); ++i)
        {
          simVis::View* inset = main->getInset(i);
          inset->setVisible(!inset->isVisible());
        }
        break;
      }
      case 'r':
      {
        removeAllRequested_ = true;
        simVis::View::Insets insets;
        viewer_->getMainView()->getInsets(insets);
        for (unsigned i = 0; i < insets.size(); ++i)
          viewer_->getMainView()->removeInset(insets[i].get());

        SIM_NOTICE << LC << "Removed all insets." << std::endl;
        break;
      }
      case 'c':
      {
        // find the next platform to center on
        std::vector<simData::ObjectId>  ids;
        dataStore_.idList(&ids, simData::PLATFORM);
        if (centeredPlat_ == 0)
          centeredPlat_ = ids.front();
        else
        {
          for (auto iter = ids.begin(); iter != ids.end(); ++iter)
          {
            // find current centered
            if (centeredPlat_ == *iter)
            {
              ++iter;
              if (iter == ids.end())
                centeredPlat_ = ids.front();
              else
                centeredPlat_ = *iter;
              break;
            }
          }
        }

        simVis::EntityNode* plat = viewer_->getSceneManager()->getScenario()->find<simVis::EntityNode>(centeredPlat_);
        simVis::View* curView = viewer_->getMainView()->getFocusManager()->getFocusedView();
        if (curView)
        {
          simVis::Viewpoint vp = curView->getViewpoint();
          // Reset the position offset if there was one
          vp.positionOffset() = osg::Vec3();
          curView->tetherCamera(plat, vp, 0.0);
        }
        break;
      }
      case 'n': // lowercase
      {
        labelsOn_ = !labelsOn_;
        std::vector<simData::ObjectId> ids;
        dataStore_.idList(&ids, simData::PLATFORM);
        for (auto iter = ids.begin(); iter != ids.end(); ++iter)
        {
          simData::DataStore::Transaction tn;
          simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
          prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(labelsOn_);
          tn.complete(&prefs);
        }
        break;
      }
      case 'd':
      {
        dynamicScaleOn_ = !dynamicScaleOn_;
        std::vector<simData::ObjectId> ids;
        dataStore_.idList(&ids, simData::PLATFORM);
        for (auto iter = ids.begin(); iter != ids.end(); ++iter)
        {
          simData::DataStore::Transaction tn;
          simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
          prefs->set_dynamicscale(dynamicScaleOn_);
          tn.complete(&prefs);
        }
        break;
      }
      }
    }

    ImGui::End();
  }

private:
  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::CreateInsetEventHandler> handler_;
  const LatLonElevListener* latLonElevListener_;
  simData::DataStore& dataStore_;
  simData::ObjectId centeredPlat_;
  bool showElevation_;
  bool removeAllRequested_;
  bool insertViewPortMode_;
  bool dynamicScaleOn_;
  bool labelsOn_;
};
#else
// An event handler to assist in testing the Inset functionality.
struct MouseAndMenuHandler : public osgGA::GUIEventHandler
{
  MouseAndMenuHandler(simVis::Viewer* viewer, simVis::CreateInsetEventHandler* handler,
    ui::LabelControl* status, const LatLonElevListener* latLonElevListener, simData::DataStore& dataStore,
    simData::ObjectId centeredPlat, bool showElevation)
  : viewer_(viewer),
    handler_(handler),
    statusLabel_(status),
    latLonElevListener_(latLonElevListener),
    dataStore_(dataStore),
    centeredPlat_(centeredPlat),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true)
  {
    updateStatusAndLabel_();
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
      handled = handleKeyPress_(ea.getKey());

    // update the status and label every time an event occurs, which may change the status values
    updateStatusAndLabel_();
    return handled;
  }

private:

  bool handleKeyPress_(int keyPress)
  {
    bool rv = false;
    switch (keyPress)
    {
    case 'c': // center on next platform
    {
      // find the next platform to center on
      std::vector<simData::ObjectId>  ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      if (centeredPlat_ == 0)
      {
        centeredPlat_ = ids.front();
      }
      else
      {
        for (auto iter = ids.begin(); iter != ids.end(); ++iter)
        {
          // find current centered
          if (centeredPlat_ == *iter)
          {
            ++iter;
            if (iter == ids.end())
              centeredPlat_ = ids.front();
            else
              centeredPlat_ = *iter;
            break;
          }
        }
      }
      simVis::EntityNode *plat = viewer_->getSceneManager()->getScenario()->find<simVis::EntityNode>(centeredPlat_);

      simVis::View* curView = viewer_->getMainView()->getFocusManager()->getFocusedView();
      if (curView)
      {
        simVis::Viewpoint vp = curView->getViewpoint();
        // Reset the position offset if there was one
        vp.positionOffset() = osg::Vec3();
        curView->tetherCamera(plat, vp, 0.0);
        rv = true;
      }
    }
    break;

    case 'r': // remove all insets
    {
      removeAllRequested_ = true;
      simVis::View::Insets insets;
      viewer_->getMainView()->getInsets(insets);
      for (unsigned i = 0; i < insets.size(); ++i)
        viewer_->getMainView()->removeInset(insets[i].get());

      SIM_NOTICE << LC << "Removed all insets." << std::endl;
      rv = true;
    }
    break;

    case 'i': // toggle inset mode
    {
      insertViewPortMode_ = !insertViewPortMode_;
      handler_->setEnabled(insertViewPortMode_);
    }
    break;

    case 'd': // toggle dynamic scale
    {
      dynamicScaleOn_ = !dynamicScaleOn_;
      std::vector<simData::ObjectId>  ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (auto iter = ids.begin(); iter != ids.end(); ++iter)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
        prefs->set_dynamicscale(dynamicScaleOn_);
        tn.complete(&prefs);
      }
      rv = true;
    }
    break;

    case 'n': // toggle labels
    {
      labelsOn_ = !labelsOn_;
      std::vector<simData::ObjectId>  ids;
      dataStore_.idList(&ids, simData::PLATFORM);
      for (auto iter = ids.begin(); iter != ids.end(); ++iter)
      {
        simData::DataStore::Transaction tn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(*iter, &tn);
        prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(labelsOn_);
        tn.complete(&prefs);
      }
      rv = true;
    }
    break;

    case 'o': // toggle overhead on focused view
    {
      simVis::View* curView = viewer_->getMainView()->getFocusManager()->getFocusedView();
      if (curView)
      {
        curView->enableOverheadMode(!curView->isOverheadEnabled());
        rv = true;
      }
    }
    break;

    case 'v': // TOGGLE VISIBILITY of ALL INSETS (for testing)
    {
      simVis::View* main = viewer_->getMainView();
      for (unsigned i = 0; i<main->getNumInsets(); ++i)
      {
        simVis::View* inset = main->getInset(i);
        inset->setVisible(!inset->isVisible());
      }
      rv = true;
    }
    break;
    }
    return rv;
  }

  void updateStatusAndLabel_()
  {
    std::string text = insertViewPortMode_ ? "DRAWING INSETS\n" : "";

    // indicate dynamic scale state
    text += "Dynamic Scale: ";
    text += dynamicScaleOn_ ? "ON" : "OFF";
    text += "\n";

    const simVis::View* focusedView = viewer_->getMainView()->getFocusManager()->getFocusedView();

    // get camera distance
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m";
    text += os.str() + " \n";

    // get centered plat name
    text += "Centered: ";

    centeredPlat_ = getCenteredPlatformId(focusedView);
    if (centeredPlat_ == 0)
      text += "NONE\n";
    else
    {
      // now get centered entity's name
      simData::DataStore::Transaction tn;
      const simData::PlatformPrefs* prefs = dataStore_.platformPrefs(centeredPlat_, &tn);
      if (prefs)
        text += prefs->commonprefs().name() + "\n";
    }

    // get overhead mode of current focused view
    text += "Focused View: " + focusedView->getName() + " ";
    text += focusedView->isOverheadEnabled() ? "OVERHEAD" : "PERSPECTIVE";
    text += "\n";

    std::ostringstream mouseOs;
    // Avoid showing the sentinel value for off-map
    if (latLonElevListener_->lat() == simUtil::MousePositionManipulator::INVALID_POSITION_VALUE)
    {
      mouseOs << "Mouse lat: ---, lon: ---";
      if (showElevation_)
        mouseOs << ", elev: ---";
    }
    else
    {
      mouseOs << "Mouse lat: " << latLonElevListener_->lat() << ", lon: " << latLonElevListener_->lon();
      if (showElevation_)
        mouseOs << ", elev: " << latLonElevListener_->elev();
    }
    text += mouseOs.str() + "\n";

    statusLabel_->setText(text);
  }

  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::CreateInsetEventHandler> handler_;
  osg::observer_ptr<ui::LabelControl> statusLabel_;
  const LatLonElevListener* latLonElevListener_;
  simData::DataStore& dataStore_;
  simData::ObjectId centeredPlat_;
  bool showElevation_;
  bool removeAllRequested_;
  bool insertViewPortMode_;
  bool dynamicScaleOn_;
  bool labelsOn_;
};

#endif

simData::ObjectId createPlatform(simData::DataStore& dataStore, simUtil::PlatformSimulatorManager& simMgr, const std::string& name, const std::string& icon, const simUtil::Waypoint& startPos, const simUtil::Waypoint& endPos, int labelYOffset)
{
  simData::DataStore::Transaction xaction;
  simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
  simData::ObjectId id = props->id();
  xaction.complete(&props);

  simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(id, &xaction);
  prefs->set_dynamicscale(true);
  prefs->set_scale(3.0);
  prefs->mutable_commonprefs()->set_name(name);
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsetx(50);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_offsety(labelYOffset);
  prefs->set_icon(icon);
  xaction.complete(&prefs);

  osg::ref_ptr<simUtil::PlatformSimulator> sim1 = new simUtil::PlatformSimulator(id);
  sim1->addWaypoint(startPos);
  sim1->addWaypoint(endPos);
  simMgr.addSimulator(sim1.get());

  return id;
}

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  std::string earthFile;
  int numPlats = 3;
  bool showElevation = false;
  for (int index = 0; index < argc; index++)
  {
    std::string arg = argv[index];
    if (arg == "--showElevation")
      showElevation = true;
    if (arg == "--help")
    {
      std::cerr << "Usage:\n"
        " --earthFile <file> : specify earth file to load, generates default if not specified. Use relative or absolute path\n"
        " --numPlats <value> : number of platforms to generate, uses default of 3\n"
        " --showElevation : show elevation in mouse cursor position readout\n";
      return 0;
    }
    int nextIndex = index + 1;
    if (nextIndex >= argc)
      break;
    if (arg == "--earthFile")
    {
      earthFile = argv[nextIndex];
      index++;
    }
    if (arg == "--numPlats")
    {
      numPlats = atoi(argv[nextIndex]);
      index++;
    }
  }

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  if (earthFile.empty())
    viewer->setMap(simExamples::createDefaultExampleMap());
  else
    loadEarthFile(earthFile, *viewer);
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Demonstrate the view-drawing service.  This is used to create new inset views with the mouse.
  simVis::View* mainView = viewer->getMainView();
  osg::ref_ptr<simVis::InsetViewEventHandler> insetHandler = new simVis::InsetViewEventHandler(mainView);
  insetHandler->setFocusActions(simVis::InsetViewEventHandler::ACTION_CLICK_SCROLL | simVis::InsetViewEventHandler::ACTION_TAB);
  mainView->addEventHandler(insetHandler);
  osg::ref_ptr<simVis::CreateInsetEventHandler> createInsetsHandler = new simVis::CreateInsetEventHandler(mainView);
  mainView->addEventHandler(createInsetsHandler);

  dynamic_cast<osgEarth::Util::EarthManipulator*>(mainView->getCameraManipulator())
      ->getSettings()->setTerrainAvoidanceEnabled(false);

  simVis::View* hud = new simVis::View();
  hud->setUpViewAsHUD(mainView);
  mainView->getViewManager()->addView(hud);

#ifndef HAVE_IMGUI
  // add help and status labels
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, simVis::Color::Yellow));
  vbox->addControl(new ui::LabelControl(s_help, 14, simVis::Color::Silver));
  ui::LabelControl* statusLabel = new ui::LabelControl("STATUS", 14, simVis::Color::Silver);
  vbox->addControl(statusLabel);
  hud->addOverlayControl(vbox);
#endif

  // data source which will provide positions for the platform
  // based on the simulation time.
  // (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;

  // bind dataStore to the scenario manager
  viewer->getSceneManager()->getScenario()->bind(&dataStore);
  simData::ObjectId centeredPlat = 0;
  // Create platforms
  if (numPlats > 0)
  {
    osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
    simUtil::Waypoint obj1Start(70., 145., 0., 100.);
    simUtil::Waypoint obj1End(70., 145., 0., 100.);
    simData::ObjectId obj1 = createPlatform(dataStore, *simMgr, "SuperHigh 400km", EXAMPLE_AIRPLANE_ICON, obj1Start, obj1End, 0);
    centeredPlat = obj1;
    if (numPlats > 1)
    {
      simUtil::Waypoint obj2Start(70., 145., 0., 100.);
      simUtil::Waypoint obj2End(70., 145., 0., 100.);
      simData::ObjectId obj2 = createPlatform(dataStore, *simMgr, "Ground 0m", EXAMPLE_TANK_ICON, obj2Start, obj2End, 30);
    }
    if (numPlats > 2)
    {
      simUtil::Waypoint obj3Start(69.8, 145., 100000., 100.);
      simUtil::Waypoint obj3End(69.8, 145., 100000., 100.);
      simData::ObjectId obj3 = createPlatform(dataStore, *simMgr, "Medium High 100km", EXAMPLE_MISSILE_ICON, obj3Start, obj3End, 0);
    }
    if (numPlats > 3)
    {
      for (int i = 3; i < numPlats; ++i)
      {
        double lat = simCore::angFix90(i * 0.001 + 10.);
        double lon = simCore::angFix180(i * 0.001 + 5.);
        std::ostringstream os;
        os << "Plat" << i+1;
        simUtil::Waypoint objStart(lat, lon, 0., 100.);
        simUtil::Waypoint objEnd(lat, lon, 0., 100.);
        createPlatform(dataStore, *simMgr, os.str(), EXAMPLE_SHIP_ICON, objStart, objEnd, 0);
      }
    }

    simMgr->simulate(START_TIME, END_TIME, 60.0);
    viewer->addEventHandler(new simUtil::SimulatorEventHandler(simMgr.get(), START_TIME, END_TIME));

    // start centered on a platform in overhead mode
    osg::observer_ptr<simVis::EntityNode> obj1Node = viewer->getSceneManager()->getScenario()->find(obj1);
    mainView->tetherCamera(obj1Node.get());
    mainView->setFocalOffsets(0, -90, 5000);
  }
  mainView->enableOverheadMode(true);

  // Set up mouse manipulator, which ties into a mouse dispatcher that helps to manage multiple manipulators in real apps
  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher;
  mouseDispatcher.reset(new simUtil::MouseDispatcher);
  mouseDispatcher->setViewManager(viewer);
  auto mouseManip = std::make_shared<simUtil::MousePositionManipulator>(
  viewer->getSceneManager()->getMapNode(),
  viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks"));
  mouseManip->setTerrainResolution(0.0001);
  mouseDispatcher->addManipulator(0, mouseManip);
  auto latLonElevListener = std::make_shared<LatLonElevListener>();
  mouseManip->addListener(latLonElevListener.get(), showElevation);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  // Adjusted projection matrix is incorrect in ortho mode
  gui->setAutoAdjustProjectionMatrix(false);

  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(viewer.get(), createInsetsHandler.get(), latLonElevListener.get(), dataStore, centeredPlat, showElevation));
#else
  // Install a handler to respond to the demo keys in this sample.
  osg::ref_ptr<MouseAndMenuHandler> mouseHandler = new MouseAndMenuHandler(viewer.get(), createInsetsHandler.get(), statusLabel, latLonElevListener.get(), dataStore, centeredPlat, showElevation);
  mainView->getCamera()->addEventCallback(mouseHandler);
#endif

  // hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(viewer->getSceneManager()));

  // for status and debugging
  viewer->installDebugHandlers();

  dataStore.update(9.);
  return viewer->run();
}
