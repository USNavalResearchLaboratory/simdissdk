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
#else
#include <osgEarth/Controls>
#include <osgEarth/StringUtils>
namespace ui = osgEarth::Util::Controls;
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
      std::string type = (props->type() == simData::GateProperties_GateType_ABSOLUTE_POSITION ? "ABSOLUTE" : "BODY RELATIVE");
      xaction.complete(&props);
      ImGui::TableNextColumn(); ImGui::Text("Type"); ImGui::TableNextColumn(); ImGui::Text(type.c_str());

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
        fillPattern_ = static_cast<simData::GatePrefs_FillPattern>(currentPatternIdx);
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

      simData::GatePrefs_DrawMode drawMode = simData::GatePrefs_DrawMode_RANGE;
      if (drawModeIdx_ == 1)
        drawMode = simData::GatePrefs_DrawMode_FOOTPRINT;
      if (drawModeIdx_ == 2)
        drawMode = simData::GatePrefs_DrawMode_COVERAGE;

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
  simData::GatePrefs_FillPattern fillPattern_ = simData::GatePrefs_FillPattern_STIPPLE;
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

#else

struct AppData
{
  osg::ref_ptr<ui::HSliderControl> typeSlider_;
  osg::ref_ptr<ui::LabelControl>   typeLabel_;

  osg::ref_ptr<ui::HSliderControl> modeSlider_;
  osg::ref_ptr<ui::LabelControl>   modeLabel_;

  osg::ref_ptr<ui::HSliderControl> fillPatternSlider_;
  osg::ref_ptr<ui::LabelControl>   fillPatternLabel_;

  osg::ref_ptr<ui::HSliderControl> rangeMinSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeMinLabel_;

  osg::ref_ptr<ui::HSliderControl> rangeMaxSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeMaxLabel_;

  osg::ref_ptr<ui::HSliderControl> horizSlider_;
  osg::ref_ptr<ui::LabelControl>   horizLabel_;

  osg::ref_ptr<ui::HSliderControl> vertSlider_;
  osg::ref_ptr<ui::LabelControl>   vertLabel_;

  osg::ref_ptr<ui::HSliderControl> azimuthSlider_;
  osg::ref_ptr<ui::LabelControl>   azimuthLabel_;

  osg::ref_ptr<ui::HSliderControl> elevSlider_;
  osg::ref_ptr<ui::LabelControl>   elevLabel_;

  osg::ref_ptr<ui::HSliderControl> colorSlider_;
  osg::ref_ptr<ui::LabelControl>   colorLabel_;

  osg::ref_ptr<ui::CheckBoxControl> centroidCheck_;
  osg::ref_ptr<ui::CheckBoxControl> lightedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> globalToggle_;

  std::vector< std::pair<simData::GateProperties_GateType, std::string> > types_;
  std::vector< std::pair<simData::GatePrefs_DrawMode,      std::string> > modes_;
  std::vector< std::pair<simData::GatePrefs_FillPattern,   std::string> > fillPatterns_;
  std::vector< std::pair<simVis::Color, std::string> >                    colors_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  simData::ObjectId    gateId_;
  osg::ref_ptr<simVis::View> view_;
  osg::ref_ptr<simVis::ScenarioManager> scenario_;
  double               t_;

  AppData()
   : typeSlider_(nullptr),
     typeLabel_(nullptr),
     modeSlider_(nullptr),
     modeLabel_(nullptr),
     fillPatternSlider_(nullptr),
     fillPatternLabel_(nullptr),
     rangeMinSlider_(nullptr),
     rangeMinLabel_(nullptr),
     rangeMaxSlider_(nullptr),
     rangeMaxLabel_(nullptr),
     horizSlider_(nullptr),
     horizLabel_(nullptr),
     vertSlider_(nullptr),
     vertLabel_(nullptr),
     azimuthSlider_(nullptr),
     azimuthLabel_(nullptr),
     elevSlider_(nullptr),
     elevLabel_(nullptr),
     colorSlider_(nullptr),
     colorLabel_(nullptr),
     centroidCheck_(nullptr),
     lightedCheck_(nullptr),
     ds_(nullptr),
     hostId_(0),
     gateId_(0),
     view_(nullptr),
     t_(0.0)
  {
    types_.push_back(std::make_pair(simData::GateProperties_GateType_ABSOLUTE_POSITION, "ABSOLUTE"));
    types_.push_back(std::make_pair(simData::GateProperties_GateType_BODY_RELATIVE,     "BODY RELATIVE"));

    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_RANGE,    "RANGE"));
    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_COVERAGE, "COVERAGE"));
    modes_.push_back(std::make_pair(simData::GatePrefs_DrawMode_FOOTPRINT, "FOOTPRINT"));

    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_STIPPLE,  "STIPPLE"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_SOLID,    "SOLID"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_ALPHA,    "ALPHA"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_WIRE,     "WIRE"));
    fillPatterns_.push_back(std::make_pair(simData::GatePrefs_FillPattern_CENTROID, "CENTROID"));

    colors_.push_back(std::make_pair(simVis::Color(0xffffff7fu), "White"));
    colors_.push_back(std::make_pair(simVis::Color(0x00ff007fu), "Green"));
    colors_.push_back(std::make_pair(simVis::Color(0xff00007fu), "Red"));
    colors_.push_back(std::make_pair(simVis::Color(0xff7f007fu), "Orange"));
    colors_.push_back(std::make_pair(simVis::Color(0xffff007fu), "Yellow"));
  }

  void apply()
  {
    simData::DataStore::Transaction xaction;

    t_ += 1.0;
    int typeIndex;
    int modeIndex  = simCore::sdkMax(0, (int)floor(modeSlider_->getValue()));
    int colorIndex = simCore::sdkMax(0, (int)floor(colorSlider_->getValue()));
    int fillPatternIndex = simCore::sdkMax(0, (int)floor(fillPatternSlider_->getValue()));

    // fetch properties:
    {
      const simData::GateProperties* props = ds_->gateProperties(gateId_, &xaction);
      typeIndex = props->type() == simData::GateProperties_GateType_ABSOLUTE_POSITION ? 0 : 1;
      xaction.complete(&props);
    }

    // apply preferences:
    {
      simData::GatePrefs* prefs = ds_->mutable_gatePrefs(gateId_, &xaction);
      prefs->mutable_commonprefs()->set_draw(true);

      prefs->mutable_commonprefs()->set_color(colors_[colorIndex].first.as(simVis::Color::RGBA));
      prefs->set_fillpattern(fillPatterns_[fillPatternIndex].first);

      prefs->set_gatedrawmode(modes_[modeIndex].first);
      prefs->set_gatelighting(lightedCheck_->getValue());
      prefs->set_drawcentroid(centroidCheck_->getValue());
      xaction.complete(&prefs);
    }

    // apply update:
    {
      simData::GateUpdate* update = ds_->addGateUpdate(gateId_, &xaction);
      update->set_time(t_);

      update->set_minrange(rangeMinSlider_->getValue());
      update->set_maxrange(rangeMaxSlider_->getValue());
      update->set_centroid(0.5 * (rangeMaxSlider_->getValue() + rangeMinSlider_->getValue()));
      update->set_azimuth(azimuthSlider_->getValue() * simCore::DEG2RAD);
      update->set_elevation(elevSlider_->getValue() * simCore::DEG2RAD);
      update->set_width(horizSlider_->getValue() * simCore::DEG2RAD);
      update->set_height(vertSlider_->getValue() * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_->update(t_);

    // update labels.
    typeLabel_->setText(types_[typeIndex].second);
    modeLabel_->setText(modes_[modeIndex].second);
    fillPatternLabel_->setText(fillPatterns_[fillPatternIndex].second);
    colorLabel_->setText(colors_[colorIndex].second);

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_->getValue() ?
      (displayMask |  simVis::DISPLAY_MASK_GATE) :
      (displayMask & ~simVis::DISPLAY_MASK_GATE));
  }
};

//----------------------------------------------------------------------------

struct ApplyUI : public ui::ControlEventHandler
{
  explicit ApplyUI(AppData* app) : app_(app) { }
  AppData* app_;
  void onValueChanged(ui::Control* c, bool value) { app_->apply(); }
  void onValueChanged(ui::Control* c, float value) { app_->apply(); }
  void onValueChanged(ui::Control* c, double value) { onValueChanged(c, (float)value); }
  void onClick(ui::Control* c, int buttons) { }
};

ui::Control* createUI(AppData& app)
{
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(&app);

  // top is returned to caller, memory owned by caller
  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("GATES - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  ui::Grid* grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Type"));
  app.typeLabel_ = grid->setControl(c+1, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Draw Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 0, applyUI.get()));
  app.modeSlider_->setHorizFill(true, 250);
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Fill Pattern"));
  app.fillPatternSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.fillPatterns_.size(), 0, applyUI.get()));
  app.fillPatternSlider_->setHorizFill(true, 250);
  app.fillPatternLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Min Range"));
  app.rangeMinSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 100.0, applyUI.get()));
  app.rangeMinLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeMinSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Max Range"));
  app.rangeMaxSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 350.0, applyUI.get()));
  app.rangeMaxLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeMaxSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Horiz. Size"));
  app.horizSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 400.0, 45.0, applyUI.get()));
  app.horizLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.horizSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Vert. Size"));
  app.vertSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1.0, 200.0, 45.0, applyUI.get()));
  app.vertLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.vertSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Azimuth"));
  app.azimuthSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-180.0, 180.0, 0.0, applyUI.get()));
  app.azimuthLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.azimuthSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Elevation"));
  app.elevSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-90, 90.0, 0.0, applyUI.get()));
  app.elevLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.elevSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Color"));
  app.colorSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors_.size()-1, 0, applyUI.get()));
  app.colorLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Centroid"));
  app.centroidCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Lighted"));
  app.lightedCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Global Gate Toggle"));
  app.globalToggle_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  // Add some hotkey support text
  top->addControl(new ui::LabelControl("C: Center on Platform", 16.f));
  top->addControl(new ui::LabelControl("G: Center on Gate", 16.f));

  return top;
}

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
  simData::GateProperties_GateType type = simExamples::hasArg("--br", argc, argv)?
    simData::GateProperties_GateType_BODY_RELATIVE :
    simData::GateProperties_GateType_ABSOLUTE_POSITION;

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

#ifndef HAVE_IMGUI

/** Handles keypresses */
class KeyHandler : public osgGA::GUIEventHandler
{
public:
  explicit KeyHandler(const AppData& app)
    : app_(app)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case osgGA::GUIEventAdapter::KEY_C:  // Center on host
        app_.view_->tetherCamera(app_.scenario_->find(app_.hostId_));
        break;
      case osgGA::GUIEventAdapter::KEY_G:  // Center on gate
        app_.view_->tetherCamera(app_.scenario_->find(app_.gateId_));
        break;
      }
    }

    return false;
  }

private:
  const AppData& app_;
};

#endif

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
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, platformId, gateId, viewer->getMainView(), scene->getScenario()));
#else
  /// Set up the application data
  AppData app;
  app.ds_ = &dataStore;
  app.view_ = viewer->getMainView();
  app.scenario_ = scene->getScenario();

  /// add in the platform and beam
  app.hostId_ = platformId;
  app.gateId_ = gateId;

  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  /// adds a hotkey handler for centering on entities
  viewer->addEventHandler(new KeyHandler(app));
  app.apply();
#endif

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);
  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  viewer->run();
}

