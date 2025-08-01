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
 * Image Icons Example
 *
 * Demonstrate how to use 2D image icons and the icon rotation flags.
 */

// Version check against the SDK DLL
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simData/EnumerationText.h"
#include "simData/MemoryDataStore.h"

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"

// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/TrackHistory.h"

// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

// the RangeTool API.
#include "simVis/RangeTool.h"

// paths to models
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
using namespace osgEarth::Util::Controls;
#endif


//----------------------------------------------------------------------------


/// index of currently visible calculation
static simData::IconRotation s_iconRotation = simData::IconRotation::IR_2D_YAW;

/// first line, describe the program
static const std::string s_title = "Image Icons Example";

#ifdef HAVE_IMGUI

struct ControlPanel : public simExamples::SimExamplesGui
{
  ControlPanel(simData::DataStore& dataStore, simData::ObjectId platId)
    : simExamples::SimExamplesGui(s_title),
    dataStore_(dataStore),
    platId_(platId)
  {
    addKeyFunc_(ImGuiKey_1, [this]()
      {
        // Cycle the value
        switch (s_iconRotation)
        {
        case simData::IconRotation::IR_2D_UP:
          s_iconRotation = simData::IconRotation::IR_2D_YAW;
          break;
        case simData::IconRotation::IR_2D_YAW:
          s_iconRotation = simData::IconRotation::IR_3D_YPR;
          break;
        case simData::IconRotation::IR_3D_YPR:
          s_iconRotation = simData::IconRotation::IR_3D_NORTH;
          break;
        case simData::IconRotation::IR_3D_NORTH:
          s_iconRotation = simData::IconRotation::IR_3D_YAW;
          break;
        case simData::IconRotation::IR_3D_YAW:
          s_iconRotation = simData::IconRotation::IR_2D_UP;
          break;
        }

        // Apply the setting
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (prefs)
        {
          prefs->set_rotateicons(s_iconRotation);
          txn.complete(&prefs);
        }
      });
    addKeyFunc_(ImGuiKey_2, [this]()
      {
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (!prefs)
          return;
        prefs->set_drawcirclehilight(!prefs->drawcirclehilight());
        txn.complete(&prefs);
      });
    addKeyFunc_(ImGuiKey_3, [this]()
      {
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (!prefs)
          return;
        switch (prefs->circlehilightshape())
        {
        case simData::CircleHilightShape::CH_PULSING_CIRCLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_CIRCLE);
          break;
        case simData::CircleHilightShape::CH_CIRCLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_DIAMOND);
          break;
        case simData::CircleHilightShape::CH_DIAMOND:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_SQUARE);
          break;
        case simData::CircleHilightShape::CH_SQUARE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_SQUARE_RETICLE);
          break;
        case simData::CircleHilightShape::CH_SQUARE_RETICLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_COFFIN);
          break;
        case simData::CircleHilightShape::CH_COFFIN:
        default:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_PULSING_CIRCLE);
          break;
        }
        txn.complete(&prefs);
      });
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    static std::unique_ptr<simData::EnumerationText> names;
    if (firstDraw_)
    {
      names = simData::EnumerationText::makeIconRotationName();
      ImGui::SetNextWindowPos(ImVec2(5, 25));
      firstDraw_ = false;
    }
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("1 : cycle through rotation types");
    ImGui::Text("2 : toggle highlight");
    ImGui::Text("3 : cycle through highlight styles");

    std::stringstream ss;
    ss << "Currently viewing: " << names->text(static_cast<size_t>(s_iconRotation));
    ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "%s", ss.str().c_str());

    ImGui::End();

    handlePressedKeys_();
  }


private:
  simData::DataStore& dataStore_;
  simData::ObjectId platId_;
};

#else
/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl = nullptr;

/// label displaying the name of the current calculation
static osg::ref_ptr<LabelControl> s_iconRotationLabel = nullptr;


//----------------------------------------------------------------------------
/// create an overlay with some helpful information
Control* createHelp()
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);

  vbox->addControl(new LabelControl(s_title, 20, simVis::Color::Yellow));

  vbox->addControl(new LabelControl("1 : cycle through rotation types", 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl("2 : toggle highlight", 14, simVis::Color::Silver));
  vbox->addControl(new LabelControl("3 : cycle through highlight styles", 14, simVis::Color::Silver));
  s_iconRotationLabel = new LabelControl("Currently viewing: " + IconRotation_Name(s_iconRotation),
    14, simVis::Color::Yellow);
  s_iconRotationLabel->setMargin(Gutter(0, 0, 10, 0));
  vbox->addControl(s_iconRotationLabel.get());

  s_helpControl = vbox;
  return vbox;
}


//----------------------------------------------------------------------------
/// event handler for keyboard commands to alter symbology at runtime
struct MenuHandler : public osgGA::GUIEventHandler
{
  /// constructor grabs all the state it needs for updating
  MenuHandler(simData::DataStore& ds, const simData::ObjectId& platId)
    : dataStore_(ds),
      platId_(platId)
  {
    //nop
  }

  /// callback to process user input
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case '?' : // toggle help
        s_helpControl->setVisible(!s_helpControl->visible());
        handled = true;
        break;

      case '1' : // cycle rotate mode
      {

        // Cycle the value
        switch (s_iconRotation)
        {
        case simData::IconRotation::IR_2D_UP:
          s_iconRotation = simData::IconRotation::IR_2D_YAW;
          break;
        case simData::IconRotation::IR_2D_YAW:
          s_iconRotation = simData::IconRotation::IR_3D_YPR;
          break;
        case simData::IconRotation::IR_3D_YPR:
          s_iconRotation = simData::IconRotation::IR_3D_NORTH;
          break;
        case simData::IconRotation::IR_3D_NORTH:
          s_iconRotation = simData::IconRotation::IR_3D_YAW;
          break;
        case simData::IconRotation::IR_3D_YAW:
          s_iconRotation = simData::IconRotation::IR_2D_UP;
          break;
        }

        // Apply the setting
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (prefs)
        {
          prefs->set_rotateicons(s_iconRotation);
          txn.complete(&prefs);
        }

        s_iconRotationLabel->setText("Currently viewing: " + IconRotation_Name(s_iconRotation));
        break;
      }

      case '2': // toggle circle highlight
      {
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (!prefs)
          break;
        prefs->set_drawcirclehilight(!prefs->drawcirclehilight());
        txn.complete(&prefs);
        break;
      }

      case '3': // cycle circle highlight shape
      {
        simData::DataStore::Transaction txn;
        simData::PlatformPrefs* prefs = dataStore_.mutable_platformPrefs(platId_, &txn);
        if (!prefs)
          break;
        switch (prefs->circlehilightshape())
        {
        case simData::CircleHilightShape::CH_PULSING_CIRCLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_CIRCLE);
          break;
        case simData::CircleHilightShape::CH_CIRCLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_DIAMOND);
          break;
        case simData::CircleHilightShape::CH_DIAMOND:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_SQUARE);
          break;
        case simData::CircleHilightShape::CH_SQUARE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_SQUARE_RETICLE);
          break;
        case simData::CircleHilightShape::CH_SQUARE_RETICLE:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_COFFIN);
          break;
        case simData::CircleHilightShape::CH_COFFIN:
        default:
          prefs->set_circlehilightshape(simData::CircleHilightShape::CH_PULSING_CIRCLE);
          break;
        }
        txn.complete(&prefs);
        break;
      }

      }
    }

    return handled;
  }

protected: // data
  simData::DataStore& dataStore_;
  simData::ObjectId platId_;
};

#endif

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
  sim1->addWaypoint(simUtil::Waypoint(51.5,   0.5, 40000, 200.0)); // London
  sim1->addWaypoint(simUtil::Waypoint(38.8, -77.0, 40000, 200.0)); // DC
  sim1->setSimulateRoll(true);
  sim1->setSimulatePitch(true);
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
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // data source which will provide positions for the platform
  // based on the simulation time.
  // (the simulator data store populates itself from a number of waypoints)
  simData::MemoryDataStore dataStore;

  // bind dataStore to the scenario manager
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&dataStore);

  // Create a platform to visualize:
  simData::ObjectId obj1 = createPlatform(dataStore);

  {
    // Set up and apply preferences for the platform
    simData::DataStore::Transaction txn;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(obj1, &txn);
    prefs->set_dynamicscale(true);
    prefs->set_scale(3);
    prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs::Mode::POINT);
    prefs->mutable_trackprefs()->set_linewidth(1);
    prefs->mutable_commonprefs()->set_name("Image");
    std::string iconFile;
    simExamples::readArg("--icon", argc, argv, iconFile);
    prefs->set_icon(iconFile.empty() ? EXAMPLE_IMAGE_ICON : iconFile);
    prefs->set_rotateicons(s_iconRotation);
    txn.complete(&prefs);
  }

  // Set up a simulation for our two platforms.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  viewer->addEventHandler(createSimulation(*simMgr, obj1));

  // Tether camera to platform
  osg::observer_ptr<simVis::EntityNode> obj1Node = scene->getScenario()->find(obj1);
  viewer->getMainView()->tetherCamera(obj1Node.get());

  // set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 5e5);


  // hovering the mouse over the platform should trigger a popup
  viewer->addEventHandler(new simVis::PopupHandler(scene.get()));

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, obj1));
#else
  // handle key press events
  viewer->addEventHandler(new MenuHandler(dataStore, obj1));
  // show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());
#endif

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

