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
 * Gate TEST
 * Test app for the various features of the GateNode.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simVis/EntityLabel.h"
#include "simVis/Gate.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simData/MemoryDataStore.h"
#include "simUtil/PlatformSimulator.h"
/// paths to models
#include "simUtil/ExampleResources.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

//----------------------------------------------------------------------------

namespace
{
  std::string SAYBOOL(bool x)
  {
    return x ? "ON" : "OFF";
  }
}

//----------------------------------------------------------------------------

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId platformId, simData::ObjectId gateId, simVis::View* view, simVis::ScenarioManager* scenario)
    : simExamples::SimExamplesGui("Gate Example"),
    ds_(ds),
    platformId_(platformId),
    gateId_(gateId),
    view_(view),
    scenario_(scenario)
  {
    addKeyFunc_(ImGuiKey_C, [this]() { view_->tetherCamera(scenario_->find(platformId_)); });
    addKeyFunc_(ImGuiKey_G, [this]() { view_->tetherCamera(scenario_->find(gateId_)); });
    update_();
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

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      simData::DataStore::Transaction xaction;
      const simData::GateProperties* props = ds_.gateProperties(gateId_, &xaction);
      std::string type = (props->type() == simData::GateProperties::Type::ABSOLUTE_POSITION ? "ABSOLUTE" : "BODY RELATIVE");
      xaction.complete(&props);
      ImGui::TableNextColumn(); ImGui::Text("Type"); ImGui::TableNextColumn(); ImGui::Text("%s", type.c_str());

      // Draw mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Draw Mode"); ImGui::TableNextColumn();
      static const char* DRAWMODES[] = { "RANGE", "FOOTPRINT", "COVERAGE" };
      static int currentModeIdx = 0;
      if (ImGui::BeginCombo("##type", DRAWMODES[currentModeIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(DRAWMODES); i++)
        {
          const bool isSelected = (currentModeIdx == i);
          if (ImGui::Selectable(DRAWMODES[i], isSelected))
            currentModeIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentModeIdx != drawModeIdx_)
      {
        needUpdate = true;
        drawModeIdx_ = currentModeIdx;
      }

      // Fill pattern combo box
      ImGui::TableNextColumn(); ImGui::Text("Fill Pattern"); ImGui::TableNextColumn();
      static const char* PATTERNS[] = { "STIPPLE", "SOLID", "ALPHA", "WIRE", "CENTROID" };
      static int currentPatternIdx = static_cast<int>(fillPattern_);
      if (ImGui::BeginCombo("##patterns", PATTERNS[currentPatternIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(PATTERNS); i++)
        {
          const bool isSelected = (currentPatternIdx == i);
          if (ImGui::Selectable(PATTERNS[i], isSelected))
            currentPatternIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentPatternIdx != static_cast<int>(fillPattern_))
      {
        needUpdate = true;
        fillPattern_ = static_cast<simData::GatePrefs::FillPattern>(currentPatternIdx);
      }

      // Min Range
      float minRange = minRange_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Min Range", &minRange_, 0.f, 2500.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (minRange != minRange_)
        needUpdate = true;

      // Max Range
      float maxRange = maxRange_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Max Range", &maxRange_, 0.f, 2500.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (maxRange != maxRange_)
        needUpdate = true;

      // Horizontal width
      float horzSize = horzSize_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Horiz. Size", &horzSize_, 1.f, 400.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (horzSize != horzSize_)
        needUpdate = true;

      // Vertical size
      float vertSize = vertSize_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Vert. Size", &vertSize_, 1.f, 200.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (vertSize != vertSize_)
        needUpdate = true;

      // Azimuth
      float azimuth = azimuth_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Azimuth", &azimuth_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (azimuth != azimuth_)
        needUpdate = true;

      // Elevation
      float elevation = elevation_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Elevation", &elevation_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (elevation != elevation_)
        needUpdate = true;

      // Color
      ImGui::TableNextColumn(); ImGui::Text("Color"); ImGui::TableNextColumn();
      float oldColor[4] = { color_[0], color_[1], color_[2], color_[3] };
      ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions;
      ImGui::ColorEdit4("##color", &color_[0], flags);
      for (size_t k = 0; k < 4; ++k)
      {
        if (color_[k] != oldColor[k])
          needUpdate = true;
      }

      // Centroid
      bool centroid = centroid_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Centroid", &centroid_);
      if (centroid != centroid_)
        needUpdate = true;

      // Lighting
      bool lighting = lighting_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Lighting", &lighting_);
      if (lighting != lighting_)
        needUpdate = true;

      // Global Toggle
      bool globalToggle = globalToggle_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Global Gate Toggle", &globalToggle_);
      if (globalToggle != globalToggle_)
        needUpdate = true;

      ImGui::EndTable();
    }

    ImGui::Text("C: Center on Platform");
    ImGui::Text("G: Center on Gate");

    if (needUpdate)
      update_();

    ImGui::End();

    handlePressedKeys_();
  }

private:
  /** Update the beam's prefs with the current values */
  void update_()
  {
    time_ += 1.0;

    {
      simData::DataStore::Transaction xaction;
      simData::GatePrefs* prefs = ds_.mutable_gatePrefs(gateId_, &xaction);
      prefs->mutable_commonprefs()->set_draw(true);

      prefs->mutable_commonprefs()->set_color(simVis::Color(color_[0], color_[1], color_[2], color_[3]).as(simVis::Color::RGBA));
      prefs->set_fillpattern(fillPattern_);

      simData::GatePrefs::DrawMode drawMode = simData::GatePrefs::DrawMode::RANGE;
      if (drawModeIdx_ == 1)
        drawMode = simData::GatePrefs::DrawMode::FOOTPRINT;
      if (drawModeIdx_ == 2)
        drawMode = simData::GatePrefs::DrawMode::COVERAGE;

      prefs->set_gatedrawmode(drawMode);
      prefs->set_gatelighting(lighting_);
      prefs->set_drawcentroid(centroid_);
      xaction.complete(&prefs);
    }

    {
      simData::DataStore::Transaction xaction;
      simData::GateUpdate* update = ds_.addGateUpdate(gateId_, &xaction);
      update->set_time(time_);

      update->set_minrange(minRange_);
      update->set_maxrange(maxRange_);
      update->set_centroid(0.5 * (maxRange_ + minRange_));
      update->set_azimuth(azimuth_ * simCore::DEG2RAD);
      update->set_elevation(elevation_ * simCore::DEG2RAD);
      update->set_width(horzSize_ * simCore::DEG2RAD);
      update->set_height(vertSize_ * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_.update(time_);

    unsigned int displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_ ? (displayMask | simVis::DISPLAY_MASK_GATE) : (displayMask & ~simVis::DISPLAY_MASK_GATE));
  }

  simData::MemoryDataStore& ds_;
  simData::ObjectId platformId_;
  simData::ObjectId gateId_;
  osg::ref_ptr<simVis::View> view_;
  osg::ref_ptr<simVis::ScenarioManager> scenario_;
  int drawModeIdx_ = 0;
  simData::GatePrefs::FillPattern fillPattern_ = simData::GatePrefs::FillPattern::STIPPLE;
  float time_ = 0.f;
  float minRange_ = 100.f;
  float maxRange_ = 350.f;
  float horzSize_ = 45.f;
  float vertSize_ = 45.f;
  float azimuth_ = 0.f;
  float elevation_ = 0.f;
  bool centroid_ = true;
  bool lighting_ = false;
  bool globalToggle_ = true;
  float color_[4] = { 1.f, 1.f, 1.f, .5f };
};

#endif

//----------------------------------------------------------------------------

/// Add a platform to use for the test.
simData::ObjectId addPlatform(simData::DataStore& ds,
                              int                 argc,
                              char**              argv)
{
  simData::ObjectId               hostId;
  simData::DataStore::Transaction xaction;

  // create the platform
  {
    simData::PlatformProperties* props = ds.addPlatform(&xaction);
    hostId = props->id();
    xaction.complete(&props);
  }

  // configure initial preferences
  {
    simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(hostId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(1.0);
    prefs->set_dynamicscale(false);
    prefs->mutable_commonprefs()->set_name("My Platform");
    prefs->mutable_commonprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // place it somewhere.
  {
    simCore::Vec3 pos(simCore::DEG2RAD*51.0, 0.0, 25000.0);

    simCore::Vec3 ori = simExamples::hasArg("--br", argc, argv) ?
      simCore::Vec3(simCore::DEG2RAD*45.0, simCore::DEG2RAD*45.0, 0.0) :
      simCore::Vec3(0.0, 0.0, 0.0);

    simCore::Coordinate lla(simCore::COORD_SYS_LLA, pos, ori);
    simCore::Coordinate ecef;
    simCore::CoordinateConverter conv;
    conv.convert(lla, ecef, simCore::COORD_SYS_ECEF);

    simData::PlatformUpdate* update = ds.addPlatformUpdate(hostId, &xaction);
    update->setPosition(ecef.position());
    update->setOrientation(ecef.orientation());
    update->set_time(0);
    xaction.complete(&update);

    // Note that each property update ticks 1 second; make the platform persist from
    // time 0 to time 1e5, allowing for 1e5 updates before the platform disappears.
    update = ds.addPlatformUpdate(hostId, &xaction);
    update->setPosition(ecef.position());
    update->setOrientation(ecef.orientation());
    update->set_time(1e5);
    xaction.complete(&update);
  }

  // tick the clock.
  ds.update(0);

  return hostId;
}

simData::ObjectId addGate(simData::DataStore& ds,
                          simData::ObjectId   hostId,
                          int                 argc,
                          char**              argv)
{
  // see if they user wants body-relative mode
  simData::GateProperties::Type type = simExamples::hasArg("--br", argc, argv)?
    simData::GateProperties::Type::BODY_RELATIVE :
    simData::GateProperties::Type::ABSOLUTE_POSITION;

  simData::ObjectId gateId;

  // create the beam
  {
    simData::DataStore::Transaction xaction;
    simData::GateProperties* props = ds.addGate(&xaction);
    gateId = props->id();
    props->set_hostid(hostId);
    props->set_type(type);
    xaction.complete(&props);
  }

  // tick the clock
  ds.update(0);

  return gateId;
}

//----------------------------------------------------------------------------

int usage(char** argv)
{
  SIM_NOTICE << "USAGE: " << argv[0] << "\n"
    << "    --help               : this message\n"
    << "    --br                 : body-relative mode\n";
  return 0;
}

int main(int argc, char** argv)
{
  /// usage
  if (simExamples::hasArg("--help", argc, argv))
    return usage(argv);

  simCore::checkVersionThrow();
  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // disable lighting on the map node.
  simVis::setLighting(scene->getMapNode()->getOrCreateStateSet(), 0);

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  scene->getScenario()->bind(&dataStore);

  simData::ObjectId platformId = addPlatform(dataStore, argc, argv);
  simData::ObjectId gateId = addGate(dataStore, platformId, argc, argv);
  osg::observer_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  viewer->getMainView()->tetherCamera(platformModel.get());

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, platformId, gateId, viewer->getMainView(), scene->getScenario()));
#endif

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);
  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  viewer->run();
}

