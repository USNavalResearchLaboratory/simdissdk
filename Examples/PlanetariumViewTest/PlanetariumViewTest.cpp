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
#include "simVis/Projector.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#include "osgDB/ReadFile"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;
#endif

#define LC "[Planetarium Test] "

// Uncomment to show debug textures for shadow mapping (IMGUI only)
// #define SHOW_SHADOW_MAP_DEBUG_TEXTURES

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<simVis::PlanetariumViewTool> planetarium;

  simData::MemoryDataStore dataStore;
  osg::ref_ptr<simVis::Viewer> viewer;
  osg::ref_ptr<simVis::SceneManager> scene;
  osg::ref_ptr<simVis::ScenarioManager> scenario;
  simData::ObjectId platformId = 0;

  simData::ObjectId projHost1Id = 0;
  simData::ObjectId projHost2Id = 0;

  simData::ObjectId proj1Id = 0; // external projector, pointing in
  simData::ObjectId proj2Id = 0; // external projector, pointing in
  simData::ObjectId proj3Id = 0; // internal projector, pointing out

#ifndef HAVE_IMGUI
  osg::ref_ptr<ui::CheckBoxControl>     toggleCheck;
  osg::ref_ptr<ui::CheckBoxControl>     vectorCheck;
  osg::ref_ptr<ui::HSliderControl>      rangeSlider;
  osg::ref_ptr<ui::LabelControl>        rangeLabel;
  osg::ref_ptr<ui::HSliderControl>      colorSlider;
  osg::ref_ptr<ui::LabelControl>        colorLabel;
  osg::ref_ptr<ui::CheckBoxControl>     ldbCheck;
  osg::ref_ptr<ui::CheckBoxControl>     doubleSidedCheck;
#endif

  std::vector< std::pair<simVis::Color, std::string> > colors;
  int colorIndex = 0;

  AppData()
  {
    colors.push_back(std::make_pair(simVis::Color(0xffffff3fu), "White"));
    colors.push_back(std::make_pair(simVis::Color(0x00ff003fu), "Green"));
    colors.push_back(std::make_pair(simVis::Color(0xff7f003fu), "Orange"));
    colors.push_back(std::make_pair(simVis::Color(0xffffff00u), "Invisible"));
    colors.push_back(std::make_pair(simVis::Color(0xffff003fu), "Yellow"));
    colorIndex = colors.size()-1;
  }

  void setShadowMapping(bool shadowMapping)
  {
    for (const auto& projId : { proj1Id, proj2Id, proj3Id })
    {
      simData::DataStore::Transaction txn;
      auto* prefs = dataStore.mutable_projectorPrefs(projId, &txn);
      if (prefs)
      {
        prefs->set_shadowmapping(shadowMapping);
        txn.complete(&prefs);
      }
    }
  }

  void setProjectorsVisible(bool visible)
  {
    for (const auto& projId : { proj1Id, proj2Id, proj3Id })
    {
      simData::DataStore::Transaction txn;
      auto* prefs = dataStore.mutable_commonPrefs(projId, &txn);
      if (prefs)
      {
        prefs->set_draw(visible);
        txn.complete(&prefs);
      }
    }
  }

  void setDoubleSidedProjection(bool value)
  {
    for (const auto& projId : { proj1Id, proj2Id, proj3Id })
    {
      simData::DataStore::Transaction txn;
      auto* prefs = dataStore.mutable_projectorPrefs(projId, &txn);
      if (prefs)
      {
        prefs->set_doublesided(value);
        txn.complete(&prefs);
      }
    }
  }
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
    : simExamples::SimExamplesGui("Planetarium View Example"),
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
          app_.scenario->addTool(app_.planetarium.get());
        else
          app_.scenario->removeTool(app_.planetarium.get());
      }

      // Sector
      bool sector = sector_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Sector", &sector_);
      if (sector != sector_)
        app_.planetarium->setUseSector(sector_);

      // Sector controls are only visible in sector mode
      if (sector_)
      {
        // Azimuth
        float sectorAzDeg = sectorAzDeg_;
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Azimuth", &sectorAzDeg_, 0.f, 360.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
        if (sectorAzDeg != sectorAzDeg_)
          app_.planetarium->setSectorAzimuth(sectorAzDeg_);
        // Elevation
        float sectorElDeg = sectorElDeg_;
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Elevation", &sectorElDeg_, 0.f, 90.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
        if (sectorElDeg != sectorElDeg_)
          app_.planetarium->setSectorElevation(sectorElDeg_);
        // Width
        float sectorWidthDeg = sectorWidthDeg_;
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Width", &sectorWidthDeg_, 0.01f, 360.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
        if (sectorWidthDeg != sectorWidthDeg_)
          app_.planetarium->setSectorWidth(sectorWidthDeg_);
        // Height
        float sectorHeightDeg = sectorHeightDeg_;
        IMGUI_ADD_ROW(ImGui::SliderFloat, "Height", &sectorHeightDeg_, 0.f, 180.f, "%.3f deg", ImGuiSliderFlags_AlwaysClamp);
        if (sectorHeightDeg != sectorHeightDeg_)
          app_.planetarium->setSectorHeight(sectorHeightDeg_);
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
      bool needColorUpdate = false;
      for (size_t k = 0; k < 4; ++k)
      {
        if (color_[k] != oldColor[k])
          needColorUpdate = true;
      }
      if (needColorUpdate)
        app_.planetarium->setColor(osg::Vec4f(color_[0], color_[1], color_[2], color_[3]));

      // LDB
      bool ldb = ldb_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "LDB", &ldb_);
      if (ldb != ldb_)
        app_.viewer->setLogarithmicDepthBufferEnabled(!app_.viewer->isLogarithmicDepthBufferEnabled());

      // Beam History
      bool beamHistory = beamHistory_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Beam History", &beamHistory_);
      if (beamHistory != beamHistory_)
        app_.planetarium->setDisplayBeamHistory(beamHistory_);

      // Display Gates
      bool displayGates = displayGates_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Display Gates", &displayGates_);
      if (displayGates != displayGates_)
        app_.planetarium->setDisplayGates(displayGates_);

      // Display Projectors
      bool displayProjectors = displayProjectors_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Display Projectors", &displayProjectors_);
      if (displayProjectors != displayProjectors_)
        app_.setProjectorsVisible(displayProjectors_);

      // Shadow Mapping
      bool shadowMapping = shadowMapping_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Shadow Mapping", &shadowMapping_);
      if (shadowMapping != shadowMapping_)
        app_.setShadowMapping(shadowMapping_);

      // Double-sided projection
      bool doubleSided = doubleSided_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Double-sided Projection", &doubleSided_);
      if (doubleSided != doubleSided_)
        app_.setDoubleSidedProjection(doubleSided_);

      // Use Gradient
      bool useGradient = useGradient_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Gradient", &useGradient_);
      if (useGradient != useGradient_)
        app_.planetarium->setUseGradient(useGradient_);

      if (on_)
      {
        // Texture-only mode
        bool textureOnly = textureOnly_;
        IMGUI_ADD_ROW(ImGui::Checkbox, "Texture-only Mode", &textureOnly_);
        if (textureOnly != textureOnly_)
          app_.planetarium->setTextureOnlyMode(textureOnly_);

        using TextureUnit = simVis::PlanetariumViewTool::TextureUnit;
        // Image 1
        bool showImage1 = showImage1_;
        IMGUI_ADD_ROW(ImGui::Checkbox, "Show Image 1", &showImage1_);
        if (showImage1 != showImage1_)
          app_.planetarium->setTextureEnabled(TextureUnit::UNIT0, showImage1_);
        if (showImage1_)
        {
          float image1Alpha = image1Alpha_;
          IMGUI_ADD_ROW(ImGui::SliderFloat, "Image 1 Alpha", &image1Alpha_, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
          if (image1Alpha != image1Alpha_)
            app_.planetarium->setTextureAlpha(TextureUnit::UNIT0, image1Alpha_);
        }

        // Image 2
        bool showImage2 = showImage2_;
        IMGUI_ADD_ROW(ImGui::Checkbox, "Show Image 2", &showImage2_);
        if (showImage2 != showImage2_)
          app_.planetarium->setTextureEnabled(TextureUnit::UNIT1, showImage2_);
        if (showImage2_)
        {
          float image2Alpha = image2Alpha_;
          IMGUI_ADD_ROW(ImGui::SliderFloat, "Image 2 Alpha", &image2Alpha_, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
          if (image2Alpha != image2Alpha_)
            app_.planetarium->setTextureAlpha(TextureUnit::UNIT1, image2Alpha_);

          // Coordinates for Image 2
          float image2Lat[2] = { image2Lat_[0], image2Lat_[1] };
          IMGUI_ADD_ROW(ImGui::SliderFloat2, "Image 2 Latitude", image2Lat_, -90.f, 90.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
          float image2Lon[2] = { image2Lon_[0], image2Lon_[1] };
          IMGUI_ADD_ROW(ImGui::SliderFloat2, "Image 2 Longitude", image2Lon_, -360.f, 360.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
          if (image2Lat[0] != image2Lat_[0] || image2Lat[1] != image2Lat_[1] ||
            image2Lon[0] != image2Lon_[0] || image2Lon[1] != image2Lon_[1])
          {
            app_.planetarium->setTextureCoords(TextureUnit::UNIT1, image2Lat_[0], image2Lat_[1],
              image2Lon_[0], image2Lon_[1]);
          }
        }
      }

      ImGui::EndTable();

#ifdef SHOW_SHADOW_MAP_DEBUG_TEXTURES
      if (displayProjectors_ && shadowMapping_)
      {
        auto p1 = app_.scenario->find<simVis::ProjectorNode>(app_.proj1Id);
        if (p1)
        {
          ImGui::Text("Projector 1 shadow map:");
          ImGuiUtil::Texture(p1->getShadowMap(), ri);
        }
        auto p2 = app_.scenario->find<simVis::ProjectorNode>(app_.proj2Id);
        if (p2)
        {
          ImGui::Separator();
          ImGui::Text("Projector 2 shadow map:");
          ImGuiUtil::Texture(p2->getShadowMap(), ri);
        }
      }
#endif
    }

    ImGui::End();
  }

private:
  AppData& app_;
  bool on_ = false;
  bool sector_ = false;
  float sectorAzDeg_ = 0.;
  float sectorElDeg_ = 0.;
  float sectorWidthDeg_ = 90.;
  float sectorHeightDeg_ = 60.;
  bool targetVecs_ = true;
  float range_ = 90000.f;
  float color_[4] = { 1.f, 1.f, 1.f, .5f };
  bool ldb_ = true;
  bool beamHistory_ = false;
  bool displayGates_ = false;
  bool useGradient_ = false;
  bool displayProjectors_ = false;
  bool shadowMapping_ = true;
  bool doubleSided_ = false;

  bool textureOnly_ = false;
  bool showImage1_ = false;
  float image1Alpha_ = 0.75f;
  bool showImage2_ = false;
  float image2Alpha_ = 0.5f;
  float image2Lat_[2] = { 0.f, 40.f };
  float image2Lon_[2] = { 80.f, 150.f };
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

struct ToggleProjectors : public ui::ControlEventHandler
{
  explicit ToggleProjectors(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    a.setProjectorsVisible(value);
  }
};

struct ToggleShadowMapping : public ui::ControlEventHandler
{
  explicit ToggleShadowMapping(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    a.setShadowMapping(value);
  }
};

struct ToggleDoubleSidedProjection : public ui::ControlEventHandler
{
  explicit ToggleDoubleSidedProjection(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    a.setDoubleSidedProjection(value);
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

  r++;
  grid->setControl(c, r, new ui::LabelControl("Projectors:"));
  grid->setControl(c + 1, r, new ui::CheckBoxControl(false, new ToggleProjectors(app)));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Shadow Map:"));
  grid->setControl(c + 1, r, new ui::CheckBoxControl(true, new ToggleShadowMapping(app)));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Double-sided:"));
  grid->setControl(c + 1, r, new ui::CheckBoxControl(false, new ToggleDoubleSidedProjection(app)));

  // force a width.
  app.rangeSlider->setHorizFill(true, 200);

  return top;
}
#endif

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const std::string& iconFile, const std::string& name)
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
    if (!name.empty())
      prefs->mutable_commonprefs()->set_name(name);
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
  props->set_type(simData::BeamProperties::Type::ABSOLUTE_POSITION);
  xaction.complete(&props);

  simData::BeamPrefs* prefs = dataStore.mutable_beamPrefs(result, &xaction);
  prefs->set_azimuthoffset(simCore::DEG2RAD * az);
  prefs->set_elevationoffset(simCore::DEG2RAD * el);
  prefs->set_useoffsetbeam(true);
  prefs->set_verticalwidth(simCore::DEG2RAD * 20.0);
  prefs->set_horizontalwidth(simCore::DEG2RAD * 30.0);
  prefs->set_rendercone(true);
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->set_datadraw(true);
  prefs->mutable_commonprefs()->set_color(0xffff0080); // yellow
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
  prefs->mutable_commonprefs()->set_draw(true);
  xaction.complete(&prefs);

  return result;
}

simData::ObjectId addProjector(simData::DataStore& dataStore, simData::ObjectId platformHost, double angleRad, double elevRad, const std::string& projIcon, double fovRad)
{
  // Create a hosting beam ID with very short range
  auto beamId = addBeam(platformHost, dataStore, 0., 0.);
  simData::DataStore::Transaction txn;
  auto beamPoint = dataStore.addBeamUpdate(beamId, &txn);
  beamPoint->set_time(0.);
  beamPoint->set_azimuth(angleRad);
  beamPoint->set_elevation(elevRad);
  beamPoint->set_range(0.1);
  txn.complete(&beamPoint);

  // Create the projector
  simData::ProjectorProperties* projProps = dataStore.addProjector(&txn);
  projProps->set_hostid(beamId);
  const simData::ObjectId projId = projProps->id();
  txn.complete(&projProps);

  // Configure prefs appropriately
  simData::ProjectorPrefs* prefs = dataStore.mutable_projectorPrefs(projId, &txn);
  prefs->set_rasterfile(projIcon);
  prefs->set_showfrustum(false);
  prefs->set_projectoralpha(0.8f);
  prefs->set_shadowmapping(true);
  txn.complete(&prefs);

  // Set the FOV
  simData::ProjectorUpdate* update = dataStore.addProjectorUpdate(projId, &txn);
  update->set_time(0.);
  update->set_fov(fovRad);
  txn.complete(&update);
  return projId;
}

void acceptProjectors(simData::DataStore& dataStore, simData::ObjectId platform, const std::vector<simData::ObjectId>& projectors)
{
  simData::DataStore::Transaction txn;
  auto* prefs = dataStore.mutable_platformPrefs(platform, &txn);
  *prefs->mutable_commonprefs()->mutable_acceptprojectorids() = projectors;

  txn.complete(&prefs);
}

//----------------------------------------------------------------------------

void simulate(const AppData& app, std::vector<simData::ObjectId>& targetIds, simData::DataStore& ds, simVis::Viewer* viewer)
{
  SIM_NOTICE << LC << "Building simulation.... please wait." << std::endl;

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);

  // set up a simple simulation to move the platform.
  {
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(app.platformId);
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

  // Add projector platforms that point towards the planetarium; note planetarium is 40km to 120km wide
  { // Projector 1: North of main platform, flies a little faster from east to west
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(app.projHost1Id);
    sim->addWaypoint(simUtil::Waypoint(2.0, -29.8, 80000.0, 100));
    sim->addWaypoint(simUtil::Waypoint(2.0, -31.1, 60000.0, 100));
    simman->addSimulator(sim.get());
  }
  { // Projector 2: Also north, flies a little slower from east to west
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(app.projHost2Id);
    sim->addWaypoint(simUtil::Waypoint(2.4, -30.2, 60000.0, 100));
    sim->addWaypoint(simUtil::Waypoint(2.1, -29.5, 90000.0, 100));
    simman->addSimulator(sim.get());
  }

  simman->simulate(0.0, 30.0, 5.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0, true);
  viewer->addEventHandler(simHandler.get());

  SIM_NOTICE << LC << "...simulation complete." << std::endl;
}

//----------------------------------------------------------------------------

int main(int argc, char** argv)
{
  int numBeams = 10;
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
  app.viewer = viewer.get();
  app.scene = viewer->getSceneManager();
  app.scenario = app.scene->getScenario();
  app.scenario->bind(&app.dataStore);

  // place a platform and put it in motion
  app.platformId = addPlatform(app.dataStore, EXAMPLE_SHIP_ICON, "Host");

  // place some random beams.
  ::srand(time(nullptr));
  for (int i = 0; i < numBeams; ++i)
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

  // Add projectors onto the planetarium surface
  app.projHost1Id = addPlatform(app.dataStore, EXAMPLE_MISSILE_ICON, "Proj Host 1");
  app.projHost2Id = addPlatform(app.dataStore, EXAMPLE_MISSILE_ICON, "Proj Host 2");

  // Add projector from center of planetarium, pointing out

  // make some targets flying around.
  std::vector<simData::ObjectId> targetIds;
  for (int i = 0; i < numTargets; ++i)
  {
    simData::ObjectId targetId = addPlatform(app.dataStore, EXAMPLE_AIRPLANE_ICON, "");
    targetIds.push_back(targetId);
  }
  simulate(app, targetIds, app.dataStore, viewer.get());
  app.dataStore.update(0);

  // Add projectors, make the host (and therefore planetarium) accept them, and hide the projectors (GUI control)
  app.proj1Id = addProjector(app.dataStore, app.projHost1Id, M_PI, -M_PI / 10., "A6V.png", M_PI / 4.);
  app.proj2Id = addProjector(app.dataStore, app.projHost2Id, M_PI, -M_PI / 30., "AIS.png", M_PI / 10.);
  app.proj3Id = addProjector(app.dataStore, app.platformId, -M_PI / 4, M_PI / 8., "earthcolor.jpg", M_PI / 5.);
  acceptProjectors(app.dataStore, app.platformId, { app.proj1Id, app.proj2Id, app.proj3Id });
  app.setProjectorsVisible(false);
  app.setShadowMapping(true);

  // the planetarium view:
  osg::observer_ptr<simVis::PlatformNode> platform = app.scenario->find<simVis::PlatformNode>(app.platformId);
  app.planetarium = new simVis::PlanetariumViewTool(platform.get(), app.dataStore);
  app.planetarium->setRange(75000);

  // Add planetarium textures. These can be edited only in IMGUI configuration
  app.planetarium->setTextureOnlyMode(false);
  using TextureUnit = simVis::PlanetariumViewTool::TextureUnit;
  app.planetarium->setTextureImage(TextureUnit::UNIT0, osgDB::readImageFile("earthcolor.jpg"));
  app.planetarium->setTextureEnabled(TextureUnit::UNIT0, false);
  app.planetarium->setTextureAlpha(TextureUnit::UNIT0, 0.75);
  app.planetarium->setTextureImage(TextureUnit::UNIT1, osgDB::readImageFile("moon_1024x512.jpg"));
  app.planetarium->setTextureEnabled(TextureUnit::UNIT1, false);
  app.planetarium->setTextureAlpha(TextureUnit::UNIT1, 0.5);
  app.planetarium->setTextureCoords(TextureUnit::UNIT1, 0, 40, 80, 150);

  // set up the controls
  osg::observer_ptr<simVis::View> view = viewer->getMainView();
#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
  if (view.valid())
  {
#else
  if (view.valid())
  {
    view->addOverlayControl(createUI(app));
#endif
    view->setLighting(false);

    // zoom the camera
    view->tetherCamera(platform.get());
    view->setFocalOffsets(180, -45, 350000);
  }

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  const int rv = viewer->run();
  // Remove the planetarium on exit so it can deregister from the data store
  app.scenario->removeTool(app.planetarium.get());
  return rv;
}
