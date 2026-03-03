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
 * Beam TEST
 * Test app for the various features of the BeamNode.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simVis/Platform.h"
#include "simVis/Beam.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simData/MemoryDataStore.h"
#include "simUtil/PlatformSimulator.h"
/// paths to models
#include "simUtil/ExampleResources.h"

#include <osgEarth/StringUtils>

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
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId beamId, simVis::View* view)
    : simExamples::SimExamplesGui("Beam Example"),
    ds_(ds),
    beamId_(beamId),
    view_(view)
  {
    update_();
  }

  void draw(osg::RenderInfo& ri) override
  {
    if (!isVisible())
      return;

    ImGui::SetNextWindowPos(ImVec2(5, 25), ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), visible(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      simData::DataStore::Transaction xaction;
      const simData::BeamProperties* props = ds_.beamProperties(beamId_, &xaction);
      std::string type = (props->type() == simData::BeamProperties::Type::ABSOLUTE_POSITION ? "ABSOLUTE" : "BODY RELATIVE");
      xaction.complete(&props);

      ImGui::TableNextColumn(); ImGui::Text("Type"); ImGui::TableNextColumn(); ImGui::Text("%s", type.c_str());

      // Draw mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Draw Mode"); ImGui::TableNextColumn();
      static const char* DRAWMODE[] = { "WIRE", "SOLID", "WIRE ON SOLID" };
      static int currentModeIdx = 0;
      if (ImGui::BeginCombo("##drawMode", DRAWMODE[currentModeIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(DRAWMODE); i++)
        {
          const bool isSelected = (currentModeIdx == i);
          if (ImGui::Selectable(DRAWMODE[i], isSelected))
            currentModeIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentModeIdx != static_cast<int>(drawMode_))
      {
        needUpdate = true;
        drawMode_ = static_cast<simData::BeamPrefs::DrawMode>(currentModeIdx);
      }

      // Range
      float range = range_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Range", &range_, 0.f, 2500.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (range != range_)
        needUpdate = true;

      // Horizontal width
      float horzSize = horzSize_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Horiz. Size", &horzSize_, .01f, 400.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (horzSize != horzSize_)
        needUpdate = true;

      // Vertical size
      float vertSize = vertSize_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Vert. Size", &vertSize_, .01f, 200.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
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

      // Cap Resolution
      int capRes = capRes_;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Cap Res.", &capRes_, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (capRes != capRes_)
        needUpdate = true;

      // Cone Resolution
      int coneRes = coneRes_;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Cone Res.", &coneRes_, 4, 40, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (coneRes != coneRes_)
        needUpdate = true;

      // Use Offset
      bool useOffset = useOffset_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Offset", &useOffset_);
      if (useOffset != useOffset_)
        needUpdate = true;

      // Shaded
      bool shaded = shaded_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Shaded", &shaded_);
      if (shaded != shaded_)
        needUpdate = true;

      // Blended
      bool blended = blended_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Blended", &blended_);
      if (blended != blended_)
        needUpdate = true;

      // Render Cone
      bool renderCone = renderCone_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Render Cone", &renderCone_);
      if (renderCone != renderCone_)
        needUpdate = true;

      // Animate
      bool animate = animate_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Animate", &animate_);
      if (animate != animate_)
        needUpdate = true;

      // Global Toggle
      bool globalToggle = globalToggle_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Global Beam Toggle", &globalToggle_);
      if (globalToggle != globalToggle_)
        needUpdate = true;

      if (needUpdate)
        update_();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  /** Update the beam's prefs with the current values */
  void update_()
  {
    time_ += 1.f;

    simData::DataStore::Transaction xaction;
    simData::BeamPrefs* prefs = ds_.mutable_beamPrefs(beamId_, &xaction);
    prefs->mutable_commonprefs()->set_draw(true);
    prefs->mutable_commonprefs()->set_color(simVis::Color(color_[0], color_[1], color_[2], color_[3]).as(simVis::Color::RGBA));
    prefs->set_beamdrawmode(drawMode_);
    prefs->set_horizontalwidth(simCore::DEG2RAD * horzSize_);
    prefs->set_verticalwidth(simCore::DEG2RAD * vertSize_);
    prefs->set_useoffseticon(useOffset_);
    prefs->set_shaded(shaded_);
    prefs->set_blended(blended_);
    prefs->set_rendercone(renderCone_);
    prefs->set_capresolution(static_cast<unsigned int>(capRes_));
    prefs->set_coneresolution(static_cast<unsigned int>(coneRes_));
    prefs->set_animate(animate_);
    prefs->set_pulserate(0.1);
    prefs->set_pulsestipple(0xfff0);
    xaction.complete(&prefs);

    // apply update
    {
      simData::BeamUpdate* update = ds_.addBeamUpdate(beamId_, &xaction);
      update->set_time(time_);

      update->set_range(range_);
      update->set_azimuth(azimuth_ * simCore::DEG2RAD);
      update->set_elevation(elevation_ * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_.update(time_);

    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_ ? (displayMask | simVis::DISPLAY_MASK_BEAM) : (displayMask & ~simVis::DISPLAY_MASK_BEAM));
  }

  simData::MemoryDataStore& ds_;
  simData::ObjectId beamId_;
  osg::ref_ptr<simVis::View> view_;
  simData::BeamPrefs::DrawMode drawMode_ = simData::BeamPrefs::DrawMode::WIRE;
  float time_ = 0.f;
  float range_ = 250.f;
  float horzSize_ = 45.f;
  float vertSize_ = 45.f;
  float azimuth_ = 0.f;
  float elevation_ = 0.f;
  int capRes_ = 15;
  int coneRes_ = 30;
  bool useOffset_ = false;
  bool shaded_ = false;
  bool blended_ = true;
  bool renderCone_ = true;
  bool animate_ = false;
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
    simCore::Vec3 pos(simCore::DEG2RAD*51.0, 0.0, 200.0);

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

simData::ObjectId addBeam(simData::DataStore& ds,
                          simData::ObjectId   hostId,
                          int                 argc,
                          char**              argv)
{
  // see if they user wants body-relative mode
  simData::BeamProperties::Type type = simExamples::hasArg("--br", argc, argv)?
    simData::BeamProperties::Type::BODY_RELATIVE :
    simData::BeamProperties::Type::ABSOLUTE_POSITION;

  simData::ObjectId beamId;

  // create the beam
  {
    simData::DataStore::Transaction xaction;
    simData::BeamProperties* props = ds.addBeam(&xaction);
    beamId = props->id();
    props->set_hostid(hostId);
    props->set_type(type);
    xaction.complete(&props);
  }

  // tick the clock
  ds.update(0);

  return beamId;
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
  /// usage?
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

  /// add in the platform and beam
  simData::ObjectId hostId = addPlatform(dataStore, argc, argv);
  simData::ObjectId beamId = addBeam(dataStore, hostId, argc, argv);

  osg::observer_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(hostId);
  viewer->getMainView()->tetherCamera(platformModel.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, beamId, viewer->getMainView()));
#endif

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

