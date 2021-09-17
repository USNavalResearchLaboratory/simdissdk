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
 * Tests the Planetarium View Tool.
 */

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"

#include "simData/MemoryDataStore.h"

#include "simVis/PlanetariumViewTool.h"
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
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif

#define LC "[Planetarium Test] "

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<simVis::PlanetariumViewTool> planetarium;

  simData::MemoryDataStore dataStore;
  osg::ref_ptr<simVis::Viewer> viewer;
  osg::ref_ptr<simVis::SceneManager> scene;
  osg::ref_ptr<simVis::ScenarioManager> scenario;
  simData::ObjectId platformId;

#ifndef HAVE_IMGUI
  osg::ref_ptr<ui::CheckBoxControl>     toggleCheck;
  osg::ref_ptr<ui::CheckBoxControl>     vectorCheck;
  osg::ref_ptr<ui::HSliderControl>      rangeSlider;
  osg::ref_ptr<ui::LabelControl>        rangeLabel;
  osg::ref_ptr<ui::HSliderControl>      colorSlider;
  osg::ref_ptr<ui::LabelControl>        colorLabel;
  osg::ref_ptr<ui::CheckBoxControl>     ldbCheck;
#endif

  std::vector< std::pair<simVis::Color, std::string> > colors;
  int colorIndex;

  AppData()
  {
    colors.push_back(std::make_pair(simVis::Color(0xffffff3f), "White"));
    colors.push_back(std::make_pair(simVis::Color(0x00ff003f), "Green"));
    colors.push_back(std::make_pair(simVis::Color(0xff7f003f), "Orange"));
    colors.push_back(std::make_pair(simVis::Color(0xffffff00), "Invisible"));
    colors.push_back(std::make_pair(simVis::Color(0xffff003f), "Yellow"));
    colorIndex = colors.size()-1;
  }
};

#ifdef HAVE_IMGUI
// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public GUI::BaseGui
{
public:
  explicit ControlPanel(AppData& app)
    : GUI::BaseGui("Planetarium View Example"),
    app_(app)
  {
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::BeginTable("Table", 2))
    {
      // On/off
      bool on = on_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "On/Off", &on_);
      if (on != on_)
      {
        if (on_)
          app_.scenario->addTool(app_.planetarium.get());
        else
          app_.scenario->removeTool(app_.planetarium.get());
      }

      // Target Vecs
      bool targetVecs = targetVecs_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Target Vecs", &targetVecs_);
      if (targetVecs != targetVecs_)
        app_.planetarium->setDisplayTargetVectors(targetVecs_);

      // Range
      float range = range_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Range", &range_, 40000.f, 120000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
      if (range != range_)
        app_.planetarium->setRange(range_);

      // Color
      ImGui::TableNextColumn(); ImGui::Text("Color"); ImGui::TableNextColumn();
      float oldColor[4] = { color_[0], color_[1], color_[2], color_[3] };
      ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions;
      ImGui::ColorEdit4("##color", &color_[0], flags);
      if (color_ != oldColor)
        app_.planetarium->setColor(osg::Vec4f(color_[0], color_[1], color_[2], color_[3]));

      // LDB
      bool ldb = ldb_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "LDB", &ldb_);
      if (ldb != ldb_)
        app_.viewer->setLogarithmicDepthBufferEnabled(!app_.viewer->isLogarithmicDepthBufferEnabled());

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  AppData& app_;
  bool on_ = false;
  bool targetVecs_ = true;
  float range_ = 90000.f;
  float color_[4] = { 1.f, 1.f, 1.f, .5f };
  bool ldb_ = true;
};
#else
struct Toggle : public ui::ControlEventHandler
{
  explicit Toggle(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    if (value)
      a.scenario->addTool(a.planetarium.get());
    else
      a.scenario->removeTool(a.planetarium.get());
  }
};

struct ToggleVectors : public ui::ControlEventHandler
{
  explicit ToggleVectors(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    a.planetarium->setDisplayTargetVectors(value);
  }
};

struct ToggleLDB : public ui::ControlEventHandler
{
  explicit ToggleLDB(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    a.viewer->setLogarithmicDepthBufferEnabled(!a.viewer->isLogarithmicDepthBufferEnabled());
  }
};

struct SetColor : public ui::ControlEventHandler
{
  explicit SetColor(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, double value)
  {
    a.planetarium->setColor(a.colors[ int(value) % a.colors.size() ].first);
    a.colorLabel->setText(a.colors[ int(value) % a.colors.size() ].second);
  }
};

struct SetRange : public ui::ControlEventHandler
{
  explicit SetRange(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, double value)
  {
    a.planetarium->setRange(value);
  }
};

//----------------------------------------------------------------------------
ui::Control* createUI(AppData& app)
{
  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("PlanetariumViewTool - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  osg::ref_ptr<ui::Grid> grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("ON/OFF:"));
  app.toggleCheck = grid->setControl(c+1, r, new ui::CheckBoxControl(false, new Toggle(app)));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Target Vecs:"));
  app.vectorCheck = grid->setControl(c+1, r, new ui::CheckBoxControl(true, new ToggleVectors(app)));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Range:"));
  app.rangeSlider = grid->setControl(c+1, r, new ui::HSliderControl(40000, 120000, 90000, new SetRange(app)));
  app.rangeLabel  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeSlider.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Color:"));
  app.colorSlider = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors.size()-1, 0, new SetColor(app)));
  app.colorLabel  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("LDB:"));
  app.ldbCheck = grid->setControl(c+1, r, new ui::CheckBoxControl(true, new ToggleLDB(app)));

  // force a width.
  app.rangeSlider->setHorizFill(true, 200);

  return top;
}
#endif

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const char* iconFile)
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
    prefs->set_icon(iconFile);
    prefs->set_scale(1.0f);
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
  prefs->set_fillpattern(simData::GatePrefs_FillPattern_STIPPLE);
  prefs->set_gateazimuthoffset(simCore::DEG2RAD * az);
  prefs->set_gateelevationoffset(simCore::DEG2RAD * el);
  prefs->set_gaterolloffset(simCore::DEG2RAD * roll);
  xaction.complete(&prefs);

  return result;
}


//----------------------------------------------------------------------------

void simulate(simData::ObjectId hostId, std::vector<simData::ObjectId>& targetIds, simData::DataStore& ds, simVis::Viewer* viewer)
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

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int numBeams   = 10;
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
  app.viewer   = viewer.get();
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
    roll = -5.0 + double(::rand() % 10);
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
  osg::observer_ptr<simVis::PlatformNode> platform = app.scenario->find<simVis::PlatformNode>(app.platformId);
  app.planetarium = new simVis::PlanetariumViewTool(platform.get());
  app.planetarium->setRange(75000);

  // set up the controls
  osg::observer_ptr<simVis::View> view = viewer->getMainView();
#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  view->addOverlayControl(createUI(app));
#endif
  view->setLighting(false);

  // zoom the camera
  view->tetherCamera(platform.get());
  view->setFocalOffsets(0, -45, 350000);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

