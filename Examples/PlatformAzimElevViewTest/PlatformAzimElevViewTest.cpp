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
 * Tests the Platform Azim/Elev View Tool.
 */

#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"

#include "simData/MemoryDataStore.h"

#include "simVis/PlatformAzimElevViewTool.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif
#define LC "[PlatformAzimElevViewTest] "

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<simVis::PlatformAzimElevViewTool> azimElevView;

  simData::MemoryDataStore dataStore;
  osg::ref_ptr<simVis::View> view;
  osg::ref_ptr<simVis::SceneManager> scene;
  osg::ref_ptr<simVis::ScenarioManager> scenario;
  simData::ObjectId platformId;
  osg::ref_ptr<osg::Uniform>            scaleUniform;

  AppData() { }
};

#ifdef HAVE_IMGUI
// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(AppData& app)
    : simExamples::SimExamplesGui("Platform Azim/Elev View Example"),
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

    if (ImGui::BeginTable("Table", 2))
    {
      // On/off
      bool on = on_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "On/Off", &on_);
      if (on != on_)
      {
        if (on_)
        {
          app_.scenario->addTool(app_.azimElevView.get());
          app_.view->tetherCamera(app_.scenario->find<simVis::PlatformNode>(app_.platformId));
          app_.view->setFocalOffsets(0.0, -90.0, app_.azimElevView->getRange() * 7.0);
          app_.view->enableOverheadMode(true);
          app_.view->enableOrthographic(true);
        }
        else
        {
          app_.scenario->removeTool(app_.azimElevView.get());
          app_.view->setFocalOffsets(0.0, -35.0, app_.azimElevView->getRange() * 7.0);
          app_.view->enableOverheadMode(false);
          app_.view->enableOrthographic(false);
        }
      }

      // Range
      float range = range_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Range", &range_, 40000.f, 225000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
      if (range != range_)
        app_.azimElevView->setRange(range_);

      // Angle
      float angle = angle_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Label Angle", &angle_, 0.f, osg::PI * 2.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (angle != angle_)
        app_.azimElevView->setElevLabelAngle(angle_);

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  AppData& app_;
  bool on_ = false;
  float range_ = 150000.f;
  float angle_ = osg::PI_2;
};
#endif

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const std::string& icon)
{
  // create the platform:
  simData::ObjectId platformId;
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* props = dataStore.addPlatform(&xaction);
    platformId = props->id();
    xaction.complete(&props);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->set_icon(icon);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return platformId;
}


simData::ObjectId addBeam(const simData::ObjectId hostId, simData::DataStore& dataStore, double az, double el)
{
  simData::DataStore::Transaction xaction;

  simData::BeamProperties* props = dataStore.addBeam(&xaction);
  simData::ObjectId result = props->id();
  props->set_hostid(hostId);
  xaction.complete(&props);

  simData::BeamPrefs* prefs = dataStore.mutable_beamPrefs(result, &xaction);
  prefs->set_azimuthoffset(simCore::DEG2RAD * az);
  prefs->set_elevationoffset(simCore::DEG2RAD * el);
  prefs->set_verticalwidth(simCore::DEG2RAD * 20.0);
  prefs->set_horizontalwidth(simCore::DEG2RAD * 30.0);
  prefs->set_rendercone(true);
  xaction.complete(&prefs);

  return result;
}


simData::ObjectId addGate(const simData::ObjectId hostId, simData::DataStore& dataStore, double az, double el, double roll)
{
  simData::DataStore::Transaction xaction;

  simData::GateProperties* props = dataStore.addGate(&xaction);
  simData::ObjectId result = props->id();
  props->set_hostid(hostId);
  xaction.complete(&props);

  simData::GatePrefs* prefs = dataStore.mutable_gatePrefs(result, &xaction);
  prefs->mutable_commonprefs()->set_color(simVis::Color(1, 0, 0, 0.25f).as(simVis::Color::RGBA));
  prefs->set_gateblending(true);
  prefs->set_gatelighting(false);
  prefs->set_fillpattern(simData::GatePrefs::FillPattern::STIPPLE);
  prefs->set_gateazimuthoffset(simCore::DEG2RAD * az);
  prefs->set_gateelevationoffset(simCore::DEG2RAD * el);
  prefs->set_gaterolloffset(simCore::DEG2RAD * roll);
  xaction.complete(&prefs);

  return result;
}


//----------------------------------------------------------------------------

void simulate(
         simData::ObjectId               hostId,
         std::vector<simData::ObjectId>& targetIds,
         simData::DataStore&             ds,
         simVis::Viewer*                 viewer)
{
  SIM_NOTICE << LC << "Building simulation.... please wait." << std::endl;

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);

  // set up a simple simulation to move the platform.
  {
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(hostId);
    sim->addWaypoint(simUtil::Waypoint(0.0, -30.0, 0.0, 1000));
    sim->addWaypoint(simUtil::Waypoint(0.0, -35.0, 0.0, 1000));
    simman->addSimulator(sim.get());
  }

  // simulate the targets.
  for (unsigned i = 0; i < targetIds.size(); ++i)
  {
    simData::ObjectId targetId = targetIds[i];
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(targetId);
    double alt = 50000 + double(::rand() % 100000);
    for (int w = 0; w < 2; ++w)
    {
      double lat = -20 + double(::rand() % 40);
      double lon = -60 + double(::rand() % 60);
      sim->addWaypoint(simUtil::Waypoint(lat, lon, alt, 100));
    }
    simman->addSimulator(sim.get());
  }

  simman->simulate(0.0, 30.0, 5.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0, true);
  viewer->addEventHandler(simHandler.get());

  SIM_NOTICE << LC << "...simulation complete." << std::endl;
}

#if 0
//----------------------------------------------------------------------------

void simulatePlatform(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.0, -30.0, 0.0, 1000));
  sim->addWaypoint(simUtil::Waypoint(0.0, -35.0, 0.0, 1000));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim);
  simman->simulate(0.0, 60.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman, 0.0, 60.0);
  viewer->addEventHandler(simHandler);
}
#endif

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int numBeams = 20;
  int numTargets = 100;

  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // Set up the data:
  AppData app;
  app.view     = viewer->getMainView();
  app.scene    = viewer->getSceneManager();
  app.scenario = app.scene->getScenario();
  app.scenario->bind(&app.dataStore);

  // place a platform and put it in motion
  app.platformId = addPlatform(app.dataStore, EXAMPLE_SHIP_ICON);

  // place some random beams.
  ::srand(time(nullptr));
  for (int i=0; i<numBeams; ++i)
  {
    double az, el, roll;

    // randomize some values and add a beam:
    az = -180.0 + double(::rand() % 360);
    el = double(::rand() % 70);
    simData::ObjectId beamId = addBeam(app.platformId, app.dataStore, az, el);

    // add a randomized gate offset
    az += -10.0 + double(::rand() % 20);
    el += -10.0 + double(::rand() % 20);
    roll = -22.5 + double(::rand()% 45);
    addGate(beamId, app.dataStore, az, el, roll);
  }

  // make some targets flying around.
  std::vector<simData::ObjectId> targetIds;
  for (int i = 0; i < numTargets; ++i)
  {
    simData::ObjectId targetId = addPlatform(app.dataStore, EXAMPLE_AIRPLANE_ICON);
    targetIds.push_back(targetId);
  }

  simulate(app.platformId, targetIds, app.dataStore, viewer.get());
  app.dataStore.update(0);

  // the planetarium view:
  osg::observer_ptr<simVis::EntityNode> platform = app.scenario->find(app.platformId);
  app.azimElevView = new simVis::PlatformAzimElevViewTool(platform.get());
  app.azimElevView->setRange(75000);

  // set up the controls
  osg::observer_ptr<simVis::View> view = viewer->getMainView();
#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#endif
  if (view.valid())
  {
    view->setLighting(false);

    // zoom the camera
    view->tetherCamera(platform.get());
    view->setFocalOffsets(0, -45, 250000);
  }
  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
