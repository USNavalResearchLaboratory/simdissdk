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
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/NavigationModes.h"
#include "simVis/PlatformModel.h"
#include "simVis/Popup.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

#define LC "[Overhead Example] "

namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static const double START_TIME = 0.0;
static const double END_TIME = 200.0;

static std::string s_title = "Overhead Example";
static std::string s_help =
  "o : toggle overhead mode in focused view \n"
  "i : toggles the mode for creating a new inset\n"
  "v : toggle visibility of all insets\n"
  "r : remove all insets \n"
  "c : center on next platform in focused view\n"
  "n : toggle labels for all platforms\n"
  "d : toggle dynamic scale for all platforms\n";


//----------------------------------------------------------------------------
// Demonstrates the use of the simVis::ViewManager::ViewCallback to respond to
// view events.
struct ViewReportCallback : public simVis::ViewManager::Callback
{
  ViewReportCallback(osg::Callback* cb)
    : cb_(cb)
  {}

  void operator()(simVis::View* view, const EventType& e)
  {
    switch (e)
    {
      case VIEW_ADDED:
        view->getCamera()->addEventCallback(cb_.get());
        SIM_NOTICE << LC << "View '" << view->getName() << "' added" << std::endl;
        break;

      case VIEW_REMOVED:
        SIM_NOTICE << LC << "View '" << view->getName() << "' removed" << std::endl;
        break;
    }
  }

private:
  osg::observer_ptr<osg::Callback> cb_;
};

//----------------------------------------------------------------------------

// An event handler to assist in testing the Inset functionality.
struct MouseAndMenuHandler : public osgGA::GUIEventHandler
{
  MouseAndMenuHandler(simVis::Viewer* viewer, simVis::InsetViewEventHandler* handler, ui::LabelControl* status, simData::DataStore& dataStore, simData::ObjectId centeredPlat)
  : viewer_(viewer),
    handler_(handler),
    statusLabel_(status),
    dataStore_(dataStore),
    centeredPlat_(centeredPlat),
    removeAllRequested_(false),
    insertViewPortMode_(false),
    dynamicScaleOn_(true),
    labelsOn_(true),
    border_(0)
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
      handler_->setAddInsetMode(insertViewPortMode_);
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
    os << std::setprecision(2) << "Camera Distance: " << focusedView->getViewpoint().range().value().getValue() << " m";
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


  osg::ref_ptr<simVis::Viewer> viewer_;
  osg::observer_ptr<simVis::InsetViewEventHandler> handler_;
  osg::observer_ptr<ui::LabelControl> statusLabel_;
  simData::DataStore& dataStore_;
  simData::ObjectId centeredPlat_;
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
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // create a sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Demonstrate the view-drawing service.  This is used to create new inset views with the
  // mouse.
  osg::ref_ptr<simVis::InsetViewEventHandler> insetHandler = new simVis::InsetViewEventHandler(viewer->getMainView());
  insetHandler->setFocusActions(simVis::InsetViewEventHandler::ACTION_CLICK_SCROLL | simVis::InsetViewEventHandler::ACTION_TAB);
  viewer->getMainView()->addEventHandler(insetHandler);

  dynamic_cast<osgEarth::Util::EarthManipulator*>(viewer->getMainView()->getCameraManipulator())
      ->getSettings()->setTerrainAvoidanceEnabled(false);

  simVis::View* hud = new simVis::View();
  hud->setUpViewAsHUD(viewer->getMainView());
  viewer->getMainView()->getViewManager()->addView(hud);

  // add help and status labels
  ui::VBox* vbox = new ui::VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new ui::LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new ui::LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  ui::LabelControl* statusLabel = new ui::LabelControl("STATUS", 14, osg::Vec4f(.8, .8, .8, 1));
  vbox->addControl(statusLabel);
  hud->addOverlayControl(vbox);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  /// (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;

  /// bind dataStore to the scenario manager
  viewer->getSceneManager()->getScenario()->bind(&dataStore);

  // Create platforms
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  simUtil::Waypoint obj1Start(70., 145., 400000., 100.);
  simUtil::Waypoint obj1End(70., 145., 400000., 100.);
  simData::ObjectId obj1 = createPlatform(dataStore, *simMgr, "SuperHigh 400km", EXAMPLE_AIRPLANE_ICON, obj1Start, obj1End, 0);
  simUtil::Waypoint obj2Start(70., 145., 0., 100.);
  simUtil::Waypoint obj2End(70., 145., 0., 100.);
  simData::ObjectId obj2 = createPlatform(dataStore, *simMgr, "Ground 0m", EXAMPLE_TANK_ICON, obj2Start, obj2End, 30);
  simUtil::Waypoint obj3Start(69.8, 145., 100000., 100.);
  simUtil::Waypoint obj3End(69.8, 145., 100000., 100.);
  simData::ObjectId obj3 = createPlatform(dataStore, *simMgr, "Medium High 100km", EXAMPLE_MISSILE_ICON, obj3Start, obj3End, 0);

  simMgr->simulate(START_TIME, END_TIME, 60.0);
  viewer->addEventHandler(new simVis::SimulatorEventHandler(simMgr.get(), START_TIME, END_TIME));

  // start centered on a platform in overhead mode
  osg::observer_ptr<simVis::EntityNode> obj1Node = viewer->getSceneManager()->getScenario()->find(obj1);
  simVis::View* mainView = viewer->getMainView();
  mainView->tetherCamera(obj1Node.get());
  mainView->setFocalOffsets(0, -90, 5000);
  mainView->enableOverheadMode(true);

  // Install a handler to respond to the demo keys in this sample.
  osg::ref_ptr<MouseAndMenuHandler> mouseHandler = new MouseAndMenuHandler(viewer.get(), insetHandler.get(), statusLabel, dataStore, obj2);
  viewer->getMainView()->getCamera()->addEventCallback(mouseHandler);

  // Demonstrate the view callback. This notifies us whenever new inset views are created or
  // removed or get focus.
  viewer->addCallback(new ViewReportCallback(mouseHandler.get()));

  /// hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(viewer->getSceneManager()));

  // for status and debugging
  viewer->installDebugHandlers();

  dataStore.update(9.);
  viewer->run();
}

