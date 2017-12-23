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
 * Overhead Example provides a tool for testing overhead mode functionality. A framework for demonstrating various overhead mode
 * combinations, including cases where insets and the main view have different overhead mode states.
 */

#include "osgEarth/NodeUtils"
#include "osgEarthUtil/Controls"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/NavigationModes.h"
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

namespace ui = osgEarth::Util::Controls;

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

// An event handler to assist in testing the Inset functionality.
struct MouseAndMenuHandler : public osgGA::GUIEventHandler
{
  MouseAndMenuHandler(simVis::Viewer* viewer, simVis::CreateInsetEventHandler* handler,
    simUtil::MouseDispatcher* mouseDispatcher, ui::LabelControl* status, simData::DataStore& dataStore,
    simData::ObjectId centeredPlat, bool showElevation)
  : viewer_(viewer),
    handler_(handler),
    statusLabel_(status),
    mouseDispatcher_(mouseDispatcher),
    dataStore_(dataStore),
    centeredPlat_(centeredPlat),
    showElevation_(showElevation),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true),
    border_(0)
  {
    mouseDispatcher_->setViewManager(NULL);
    latLonElevListener_.reset(new LatLonElevListener());
    setUpMouseManip_(viewer_);
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

    centeredPlat_ = getCenteredPlatformId_(focusedView);
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
    mouseOs << "Mouse lat:" << latLonElevListener_->lat() << ", lon:" << latLonElevListener_->lon();
    if (showElevation_)
      mouseOs << ", elev:" << latLonElevListener_->elev();
    text += mouseOs.str() + "\n";

    statusLabel_->setText(text);
  }

  simData::ObjectId getCenteredPlatformId_(const simVis::View* view) const
  {
    osg::Node* tether = view->getCameraTether();
    if (tether == NULL)
      return 0;
    else
    {
      simVis::PlatformModelNode* model = dynamic_cast<simVis::PlatformModelNode*>(tether);
      assert(model != NULL);

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

  void setUpMouseManip_(simVis::Viewer* viewer)
  {
    if (viewer == NULL || viewer->getSceneManager() == NULL || !mouseDispatcher_)
      return;
    mouseManip_.reset(new simUtil::MousePositionManipulator(viewer->getSceneManager()->getMapNode(), viewer->getSceneManager()->getOrCreateAttachPoint("Map Callbacks")));
    mouseManip_->setTerrainResolution(0.0001);
    mouseDispatcher_->setViewManager(viewer);
    mouseDispatcher_->addManipulator(0, mouseManip_);
    mouseManip_->addListener(latLonElevListener_.get(), showElevation_);
  }

  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::CreateInsetEventHandler> handler_;
  osg::observer_ptr<ui::LabelControl> statusLabel_;
  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher_;
  std::shared_ptr<LatLonElevListener> latLonElevListener_;
  std::shared_ptr<simUtil::MousePositionManipulator> mouseManip_;
  simData::DataStore& dataStore_;
  simData::ObjectId centeredPlat_;
  bool showElevation_;
  bool removeAllRequested_;
  bool insertViewPortMode_;
  bool dynamicScaleOn_;
  bool labelsOn_;
  int border_;
};

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

  // add help and status labels
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new ui::LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  ui::LabelControl* statusLabel = new ui::LabelControl("STATUS", 14, osg::Vec4f(.8, .8, .8, 1));
  vbox->addControl(statusLabel);
  hud->addOverlayControl(vbox);

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

  std::shared_ptr<simUtil::MouseDispatcher> mouseDispatcher;
  mouseDispatcher.reset(new simUtil::MouseDispatcher);

  // Install a handler to respond to the demo keys in this sample.
  osg::ref_ptr<MouseAndMenuHandler> mouseHandler = new MouseAndMenuHandler(viewer.get(), createInsetsHandler.get(), mouseDispatcher.get(), statusLabel, dataStore, centeredPlat, showElevation);
  mainView->getCamera()->addEventCallback(mouseHandler);

  // hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(viewer->getSceneManager()));

  // for status and debugging
  viewer->installDebugHandlers();

  dataStore.update(9.);
  return viewer->run();
}
