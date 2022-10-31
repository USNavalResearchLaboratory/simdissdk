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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
/* -*-c++-*- */
/**
 * Track History TEST
 * Test app for the various features of the Track History feature.
 */

/// the simulator provides time/space data for our platform
#include "simUtil/PlatformSimulator.h"
#include "simData/DataTable.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Time/Clock.h"
#include "simCore/Time/ClockImpl.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simVis/Utils.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include <osgEarth/StringUtils>

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#else
#include <osgEarth/Controls>
namespace ui = osgEarth::Util::Controls;
#endif

//----------------------------------------------------------------------------

#define INIT_NUM_POINTS 100
#define SIM_START         0.0
#define SIM_END          60.0
#define SIM_HZ            5.0
#define MAX_LENGTH       60

struct AppData
{
#ifdef HAVE_IMGUI
  simData::TrackPrefs_Mode trackMode = simData::TrackPrefs_Mode_POINT;
  int size = 2;
  bool flat = false;
  bool alt = false;
  bool genTrackColor = true;
  bool usePlatformColor = false;
  bool useMultiColor = true;
  float color[4] = { 1.f, 1.f, 1.f, 1.f };
  bool useOverrideColor = false;
  float overrideColor[4] = { 1.f, 1.f, 1.f, 1.f };
  int maxPoints = 100;
  bool reverse = false;
  float time = SIM_START;
  bool globalTrackDisplay = true;
#else
  osg::ref_ptr<ui::HSliderControl>  modeSlider_;
  osg::ref_ptr<ui::HSliderControl>  sizeSlider_;
  osg::ref_ptr<ui::HSliderControl>  colorSlider_;
  osg::ref_ptr<ui::CheckBoxControl> overrideColorCheck_;
  osg::ref_ptr<ui::HSliderControl>  overrideColorSlider_;
  osg::ref_ptr<ui::HSliderControl>  maxLengthSlider_;
  osg::ref_ptr<ui::CheckBoxControl> multiColorCheck_;
  osg::ref_ptr<ui::CheckBoxControl> platformColorCheck_;
  osg::ref_ptr<ui::CheckBoxControl> generateColorCommandsCheck_;

  osg::ref_ptr<ui::LabelControl>   modeLabel_;
  osg::ref_ptr<ui::LabelControl>   sizeLabel_;
  osg::ref_ptr<ui::LabelControl>   colorLabel_;
  osg::ref_ptr<ui::LabelControl>   overrideColorLabel_;
  osg::ref_ptr<ui::LabelControl>   maxLengthLabel_;

  osg::ref_ptr<ui::ButtonControl>  rewind1_;
  osg::ref_ptr<ui::ButtonControl>  rewind2_;
  osg::ref_ptr<ui::ButtonControl>  ff1_;
  osg::ref_ptr<ui::ButtonControl>  ff2_;
  osg::ref_ptr<ui::HSliderControl> timeSlider_;
  osg::ref_ptr<ui::ButtonControl>  tether_;

  osg::ref_ptr<ui::CheckBoxControl> flatModeCheck_;
  osg::ref_ptr<ui::CheckBoxControl> altModeCheck_;
  osg::ref_ptr<ui::CheckBoxControl> globalToggle_;
  osg::ref_ptr<ui::CheckBoxControl> reverseModeCheck_;
#endif
  std::vector< std::pair<simData::TrackPrefs_Mode, std::string> > modes_;
  std::vector< std::pair<simVis::Color, std::string> >            colors_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  osg::ref_ptr<simVis::View> view_;
  osg::ref_ptr<osg::Node>    platformModel_;

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler_;

  AppData(simData::DataStore* ds, simData::ObjectId hostId)
   : ds_(ds),
     hostId_(hostId),
     view_(nullptr),
     platformModel_(nullptr)
  {
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_OFF,    "OFF"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_POINT,  "POINT"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_LINE,   "LINE"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_RIBBON, "RIBBON"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_BRIDGE, "BRIDGE"));

    colors_.push_back(std::make_pair(simVis::Color::White, "White"));
    colors_.push_back(std::make_pair(simVis::Color::Lime,  "Green"));
    colors_.push_back(std::make_pair(simVis::Color::Red,   "Red"));
    colors_.push_back(std::make_pair(simVis::Color::Cyan,  "Cyan"));
    colors_.push_back(std::make_pair(simVis::Color::Orange, "Orange"));
  }

  void apply()
  {
#ifdef HAVE_IMGUI
    // add to the data table for track history
    if (genTrackColor)
      generateColorCommand_(0);
    else
      removeColorCommands_();

    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* platformPrefs = ds_->mutable_platformPrefs(hostId_, &xaction);
    simData::TrackPrefs* trackPrefs = platformPrefs->mutable_trackprefs();

    trackPrefs->set_trackdrawmode(trackMode);
    trackPrefs->set_linewidth(size);

    trackPrefs->set_flatmode(flat);
    trackPrefs->set_altmode(alt);
    trackPrefs->set_trackcolor(simVis::Color(color[0], color[1], color[2], color[3]).as(simVis::Color::RGBA));

    trackPrefs->set_trackoverridecolor(simVis::Color(overrideColor[0], overrideColor[1], overrideColor[2], overrideColor[3]).as(simVis::Color::RGBA));
    trackPrefs->set_usetrackoverridecolor(useOverrideColor);

    trackPrefs->set_multitrackcolor(useMultiColor);
    trackPrefs->set_useplatformcolor(usePlatformColor);

    // -1 value signifies no limiting
    if (maxPoints >= -1 && maxPoints <= 512)
      trackPrefs->set_tracklength(maxPoints);
    else
      trackPrefs->clear_tracklength();

    xaction.complete(&platformPrefs);

    // time direction:
    if (reverse)
      ds_->getBoundClock()->playReverse();
    else
      ds_->getBoundClock()->playForward();

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalTrackDisplay ?
      (displayMask | simVis::DISPLAY_MASK_TRACK_HISTORY) :
      (displayMask & ~simVis::DISPLAY_MASK_TRACK_HISTORY));
#else
    int modeIndex  = simCore::sdkMax(0, (int)floor(modeSlider_->getValue()));
    int size       = simCore::sdkMax(1, (int)floor(sizeSlider_->getValue()));
    int maxlength  = simCore::sdkMax(-1, (int)floor(maxLengthSlider_->getValue()));
    int colorIndex = simCore::sdkMax(0, (int)floor(colorSlider_->getValue()));
    int overrideColorIndex = simCore::sdkMax(0, (int)floor(overrideColorSlider_->getValue()));

    // add to the data table for track history
    if (generateColorCommandsCheck_->getValue())
      generateColorCommand_(colorIndex);
    else
      removeColorCommands_();

    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* platformPrefs = ds_->mutable_platformPrefs(hostId_, &xaction);
    simData::TrackPrefs*    trackPrefs    = platformPrefs->mutable_trackprefs();

    trackPrefs->set_trackdrawmode(modes_[modeIndex].first);
    trackPrefs->set_linewidth(size);

    trackPrefs->set_flatmode(flatModeCheck_->getValue());
    trackPrefs->set_altmode(altModeCheck_->getValue());
    trackPrefs->set_trackcolor(colors_[colorIndex].first.as(simVis::Color::RGBA));

    trackPrefs->set_trackoverridecolor(colors_[overrideColorIndex].first.as(simVis::Color::RGBA));
    if (overrideColorCheck_->getValue())
      trackPrefs->set_usetrackoverridecolor(true);
    else
      trackPrefs->set_usetrackoverridecolor(false);

    trackPrefs->set_multitrackcolor(multiColorCheck_->getValue());
    trackPrefs->set_useplatformcolor(platformColorCheck_->getValue());

    // -1 value signifies no limiting
    if (maxlength >= -1 && maxlength <= MAX_LENGTH)
      trackPrefs->set_tracklength(maxlength);
    else
      trackPrefs->clear_tracklength();

    xaction.complete(&platformPrefs);

    // time direction:
    if (reverseModeCheck_->getValue() == true)
        ds_->getBoundClock()->playReverse();
    else
        ds_->getBoundClock()->playForward();

    // update labels.
    modeLabel_->setText(modes_[modeIndex].second);
    sizeLabel_->setText(osgEarth::Stringify() << size);
    colorLabel_->setText(colors_[colorIndex].second);
    overrideColorLabel_->setText(colors_[overrideColorIndex].second);
    maxLengthLabel_->setText(osgEarth::Stringify() << maxlength);

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_->getValue() ?
      (displayMask |  simVis::DISPLAY_MASK_TRACK_HISTORY) :
      (displayMask & ~simVis::DISPLAY_MASK_TRACK_HISTORY));
#endif
  }

  void rewind(double seconds)
  {
    if (ds_->getBoundClock()->timeDirection() == simCore::REVERSE)
        seconds = -seconds;

    simHandler_->setTime(simHandler_->getTime() - seconds);
  }

  void ff(double seconds)
  {
    if (ds_->getBoundClock()->timeDirection() == simCore::REVERSE)
        seconds = -seconds;

    simHandler_->setTime(simHandler_->getTime() + seconds);
  }

  void tether()
  {
    view_->tetherCamera(nullptr);
    view_->tetherCamera(platformModel_.get());
    view_->setFocalOffsets(45, -45, 2e4);
  }
private:

  void generateColorCommand_(int colorIndex)
  {
    simData::DataTable* table = ds_->dataTableManager().findTable(hostId_, simData::INTERNAL_TRACK_HISTORY_TABLE);
    simData::TableColumnId colId = 0;
    bool foundColumn = false;
    if (table == nullptr)
    {
      simData::TableStatus status =  ds_->dataTableManager().addDataTable(hostId_, simData::INTERNAL_TRACK_HISTORY_TABLE, &table);
      if (!status.isError())
      {
        simData::TableColumn* newColumn = nullptr;
        if (!table->addColumn(simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN, simData::VT_UINT32, 0, &newColumn).isError())
        {
          colId = newColumn->columnId();
          foundColumn = true;
        }
        else
        {
          SIM_ERROR << "CommandTrackColor: Could not add column: " << simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN << " to table: " << simData::INTERNAL_TRACK_HISTORY_TABLE << ".\n";
        }
      }
      else
      {
        SIM_ERROR << "CommandTrackColor: Could not add table: " << simData::INTERNAL_TRACK_HISTORY_TABLE << "; Error: " << status.what() << ".\n";
      }
    }
    else
    {
      simData::TableColumn* column = table->column(simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN);
      if (column)
      {
        colId = column->columnId();
        foundColumn = true;
      }
      else
      {
        SIM_ERROR << "CommandTrackColor: Could not find column: " << simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN << " in table: " << simData::INTERNAL_TRACK_HISTORY_TABLE << ".\n";
      }
    }
    if (foundColumn)
    {
      simData::TableRow newRow;
      newRow.setTime(simHandler_->getTime());
#ifdef HAVE_IMGUI
      simVis::Color c(color[0], color[1], color[2], color[3]);
      newRow.setValue(colId, c.as(simVis::Color::RGBA));
#else
      newRow.setValue(colId, colors_[colorIndex].first.as(simVis::Color::RGBA));
#endif
      table->addRow(newRow);
    }
  }

  void removeColorCommands_()
  {
    simData::DataTable* table = ds_->dataTableManager().findTable(hostId_, simData::INTERNAL_TRACK_HISTORY_TABLE);
    if (table != nullptr)
      ds_->dataTableManager().deleteTable(table->tableId());
  }

};

//----------------------------------------------------------------------------

#ifdef HAVE_IMGUI
// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  explicit ControlPanel(AppData& app)
    : simExamples::SimExamplesGui("Track History Example"),
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

    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      // Draw mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Draw Mode"); ImGui::TableNextColumn();
      static const char* TRACKMODES[] = { "OFF", "POINT", "LINE", "RIBBON", "BRIDGE" };
      static int currentModeIdx = app_.trackMode;
      if (ImGui::BeginCombo("##trackmode", TRACKMODES[currentModeIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(TRACKMODES); i++)
        {
          const bool isSelected = (currentModeIdx == i);
          if (ImGui::Selectable(TRACKMODES[i], isSelected))
            currentModeIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentModeIdx != app_.trackMode)
      {
        needUpdate = true;
        app_.trackMode = static_cast<simData::TrackPrefs_Mode>(currentModeIdx);
      }

      int size = app_.size;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Size", &app_.size, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (size != app_.size)
        needUpdate = true;

      bool flat = app_.flat;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Flat Mode", &app_.flat);
      if (flat != app_.flat)
        needUpdate = true;

      bool alt = app_.alt;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Alt Mode", &app_.alt);
      if (alt != app_.alt)
        needUpdate = true;

      bool genTrackColor = app_.genTrackColor;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Generate TrackColor Commands", &app_.genTrackColor);
      if (genTrackColor != app_.genTrackColor)
        needUpdate = true;

      bool usePlatformColor = app_.usePlatformColor;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Platform Color", &app_.usePlatformColor);
      if (usePlatformColor != app_.usePlatformColor)
        needUpdate = true;

      bool useMultiColor = app_.useMultiColor;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Multi-color", &app_.useMultiColor);
      if (useMultiColor != app_.useMultiColor)
        needUpdate = true;

      ImGui::TableNextColumn(); ImGui::Text("Color"); ImGui::TableNextColumn();
      float oldColor[4] = { app_.color[0], app_.color[1], app_.color[2], app_.color[3] };
      ImGuiColorEditFlags flags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions;
      ImGui::ColorEdit4("##color", &app_.color[0], flags);
      if (app_.color != oldColor)
        needUpdate = true;

      bool useOverrideColor = app_.useOverrideColor;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Use Override Color", &app_.useOverrideColor);
      if (useOverrideColor != app_.useOverrideColor)
        needUpdate = true;

      ImGui::TableNextColumn(); ImGui::Text("Override Color"); ImGui::TableNextColumn();
      float oldOverrideColor[4] = { app_.overrideColor[0], app_.overrideColor[1], app_.overrideColor[2], app_.overrideColor[3] };
      ImGui::ColorEdit4("##overrideColor", &app_.overrideColor[0], flags);
      if (app_.color != oldOverrideColor)
        needUpdate = true;

      int maxPoints = app_.maxPoints;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Max Points", &app_.maxPoints, -1, 512, "%d", ImGuiSliderFlags_AlwaysClamp);
      if (maxPoints != app_.maxPoints)
        needUpdate = true;

      ImGui::TableNextColumn(); ImGui::Text("Transport"); ImGui::TableNextColumn();
      if (ImGui::Button("<<")) { app_.rewind(15.f); } ImGui::SameLine();
      if (ImGui::Button("<")) { app_.rewind(5.f); } ImGui::SameLine();
      if (ImGui::Button(">")) { app_.ff(5.f); } ImGui::SameLine();
      if (ImGui::Button(">>")) { app_.ff(15.f); }

      bool reverse = app_.reverse;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Reverse Mode", &app_.reverse);
      if (reverse != app_.reverse)
        needUpdate = true;

      float time = app_.time;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Time", &app_.time, SIM_START, SIM_END, "", ImGuiSliderFlags_AlwaysClamp);
      if (time != app_.time)
        app_.simHandler_->setTime(time);

      bool globalTrackDisplay = app_.globalTrackDisplay;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Global Track Display", &app_.globalTrackDisplay);
      if (globalTrackDisplay != app_.globalTrackDisplay)
        needUpdate = true;

      ImGui::EndTable();
    }

    if (ImGui::Button("Reset Tether"))
      app_.tether();

    if (needUpdate)
      app_.apply();

    ImGui::End();
  }

private:

  AppData& app_;
};
#else
struct ApplyUI : public ui::ControlEventHandler
{
  explicit ApplyUI(AppData* app): app_(app) {}
  AppData* app_;
  void onValueChanged(ui::Control* c, bool value) { app_->apply(); }
  void onValueChanged(ui::Control* c, float value) { app_->apply(); }
  void onValueChanged(ui::Control* c, double value) { onValueChanged(c, (float)value); }

  void onClick(ui::Control* c, int buttons)
  {
    if (c == app_->rewind1_) app_->rewind(5.0);
    else if (c == app_->rewind2_) app_->rewind(15.0);
    else if (c == app_->ff1_)     app_->ff(5.0);
    else if (c == app_->ff2_)     app_->ff(15.0);
    else if (c == app_->tether_) app_->tether();
  }
};

struct SlideTime : public ui::ControlEventHandler
{
  explicit SlideTime(AppData* app): app_(app) {}
  AppData* app_;
  void onValueChanged(ui::Control* c, float value)
  {
    app_->simHandler_->setTime(value);
  }
};

ui::Control* createUI(AppData& app)
{
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(&app);

  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("Track History - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  osg::ref_ptr<ui::Grid> grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 1, applyUI.get()));
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());
  app.modeSlider_->setHorizFill(true, 250);

  r++;
  grid->setControl(c, r, new ui::LabelControl("Size"));
  app.sizeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1, 10, 2, applyUI.get()));
  app.sizeSlider_->setHorizFill(true, 250);
  app.sizeLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Flat mode"));
  app.flatModeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Alt mode"));
  app.altModeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Generate TrackColor Commands"));
  app.generateColorCommandsCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Use Platform Color"));
  app.platformColorCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Use Multi-color"));
  app.multiColorCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Color"));
  app.colorSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors_.size(), 0, applyUI.get()));
  app.colorSlider_->setHorizFill(true, 250);
  app.colorLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Override color"));
  app.overrideColorCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl(""));
  app.overrideColorSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.colors_.size(), 0, applyUI.get()));
  app.overrideColorSlider_->setHorizFill(true, 250);
  app.overrideColorLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Max Points"));
  app.maxLengthSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(-1, 512, INIT_NUM_POINTS, applyUI.get()));
  app.maxLengthSlider_->setHorizFill(true, 250);
  app.maxLengthLabel_ = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Transport:"));
  osg::ref_ptr<ui::HBox> buttons = grid->setControl(c+1, r, new ui::HBox());
  buttons->setChildSpacing(10.0f);
  app.rewind2_ = buttons->addControl(new ui::ButtonControl("<<", applyUI.get()));
  app.rewind1_ = buttons->addControl(new ui::ButtonControl("<", applyUI.get()));
  app.ff1_     = buttons->addControl(new ui::ButtonControl(">", applyUI.get()));
  app.ff2_     = buttons->addControl(new ui::ButtonControl(">>", applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Reverse mode:"));
  app.reverseModeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Time:"));
  app.timeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(SIM_START, SIM_END, SIM_START, new SlideTime(&app)));
  app.timeSlider_->setHorizFill(true, 250);

  r++;
  app.tether_ = grid->setControl(c+1, r, new ui::ButtonControl("Reset Tether", applyUI.get()));

  r++;
  ui::HBox* toggleBox = grid->setControl(c+1, r, new ui::HBox());
  app.globalToggle_ = toggleBox->addControl(new ui::CheckBoxControl(true, applyUI.get()));
  toggleBox->addControl(new ui::LabelControl("Global track display toggle"));

  return top;
}
#endif

//----------------------------------------------------------------------------

/// Add a platform to use for the test.
simData::ObjectId addPlatform(simData::DataStore& ds)
{
  simData::ObjectId hostId;

  // create the platform
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* props = ds.addPlatform(&xaction);
    hostId = props->id();
    xaction.complete(&props);
  }

  // configure initial preferences
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(hostId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(1000.0);  // large so we can see the ribbon.
    prefs->set_dynamicscale(false);
    prefs->mutable_commonprefs()->set_name("My Platform");
    prefs->mutable_commonprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  return hostId;
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
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

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  dataStore.bindToClock(new simCore::ClockImpl());
  scene->getScenario()->bind(&dataStore);

  /// add in the platform and beam
  simData::ObjectId platformId = addPlatform(dataStore);

  /// simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  /// create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC

  /// Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  simMgr->addSimulator(sim.get());
  simMgr->simulate(SIM_START, SIM_END, SIM_HZ);

  /// Attach the simulation updater to OSG timer events
  AppData app(&dataStore, platformId);
  app.simHandler_ = new simUtil::SimulatorEventHandler(simMgr.get(), SIM_START, SIM_END);
  viewer->addEventHandler(app.simHandler_.get());

  /// Tether camera to platform
  app.view_          = viewer->getMainView();
  app.platformModel_ = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  app.view_->tetherCamera(app.platformModel_.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(45, -45, 2e6);

  /// show the instructions overlay
#ifdef HAVE_IMGUI
    // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(app));
#else
  viewer->getMainView()->addOverlayControl(createUI(app));
#endif
  app.apply();

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

