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
#else
#include <osgEarth/Controls>
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
        drawMode_ = static_cast<simData::BeamPrefs_DrawMode>(currentModeIdx);
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
  simData::BeamPrefs_DrawMode drawMode_ = simData::BeamPrefs_DrawMode_WIRE;
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

#else

struct AppData
{
  osg::ref_ptr<ui::HSliderControl> typeSlider_;
  osg::ref_ptr<ui::LabelControl>   typeLabel_;

  osg::ref_ptr<ui::HSliderControl> modeSlider_;
  osg::ref_ptr<ui::LabelControl>   modeLabel_;

  osg::ref_ptr<ui::HSliderControl> rangeSlider_;
  osg::ref_ptr<ui::LabelControl>   rangeLabel_;

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

  osg::ref_ptr<ui::HSliderControl> capResSlider_;
  osg::ref_ptr<ui::LabelControl>   capResLabel_;

  osg::ref_ptr<ui::HSliderControl> coneResSlider_;
  osg::ref_ptr<ui::LabelControl>   coneResLabel_;

  osg::ref_ptr<ui::CheckBoxControl> useOffsetIconCheck_;
  osg::ref_ptr<ui::CheckBoxControl> shadedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> blendedCheck_;
  osg::ref_ptr<ui::CheckBoxControl> renderConeCheck_;
  osg::ref_ptr<ui::CheckBoxControl> animateCheck_;

  osg::ref_ptr<ui::CheckBoxControl> globalToggle_;

  std::vector< std::pair<simData::BeamProperties_BeamType, std::string> > types_;
  std::vector< std::pair<simData::BeamPrefs_DrawMode,      std::string> > modes_;
  std::vector< std::pair<simVis::Color, std::string> >                    colors_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  simData::ObjectId    beamId_;
  osg::ref_ptr<simVis::View> view_;
  double               t_;

  AppData()
   : typeSlider_(nullptr),
     typeLabel_(nullptr),
     modeSlider_(nullptr),
     modeLabel_(nullptr),
     rangeSlider_(nullptr),
     rangeLabel_(nullptr),
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
     capResSlider_(nullptr),
     capResLabel_(nullptr),
     coneResSlider_(nullptr),
     coneResLabel_(nullptr),
     useOffsetIconCheck_(nullptr),
     shadedCheck_(nullptr),
     blendedCheck_(nullptr),
     renderConeCheck_(nullptr),
     animateCheck_(nullptr),
     ds_(nullptr),
     hostId_(0),
     beamId_(0),
     view_(nullptr),
     t_(0.0)
  {
    types_.push_back(std::make_pair(simData::BeamProperties::Type::ABSOLUTE_POSITION, "ABSOLUTE"));
    types_.push_back(std::make_pair(simData::BeamProperties::Type::BODY_RELATIVE,     "BODY RELATIVE"));

    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_SOLID, "SOLID"));
    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_WIRE,  "WIRE"));
    modes_.push_back(std::make_pair(simData::BeamPrefs_DrawMode_WIRE_ON_SOLID, "WIRE ON SOLID"));

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
    int capRes     = simCore::sdkMax(1, (int)floor(capResSlider_->getValue()));
    int coneRes    = simCore::sdkMax(1, (int)floor(coneResSlider_->getValue()));

    // fetch properties:
    {
      const simData::BeamProperties* props = ds_->beamProperties(beamId_, &xaction);
      typeIndex = props->type() == simData::BeamProperties::Type::ABSOLUTE_POSITION ? 0 : 1;
      xaction.complete(&props);
    }

    // apply preferences:
    {
      simData::BeamPrefs* prefs = ds_->mutable_beamPrefs(beamId_, &xaction);
      prefs->mutable_commonprefs()->set_draw(true);

      prefs->mutable_commonprefs()->set_color(colors_[colorIndex].first.as(simVis::Color::RGBA));

      prefs->set_beamdrawmode(modes_[modeIndex].first);
      prefs->set_horizontalwidth(horizSlider_->getValue() * simCore::DEG2RAD);
      prefs->set_verticalwidth(vertSlider_->getValue() * simCore::DEG2RAD);
      prefs->set_useoffseticon(useOffsetIconCheck_->getValue());
      prefs->set_shaded(shadedCheck_->getValue());
      prefs->set_blended(blendedCheck_->getValue());
      prefs->set_rendercone(renderConeCheck_->getValue());
      prefs->set_capresolution(capRes);
      prefs->set_coneresolution(coneRes);
      prefs->set_animate(animateCheck_->getValue());
      prefs->set_pulserate(0.1);
      prefs->set_pulsestipple(0xfff0);

      xaction.complete(&prefs);
    }

    // apply update:
    {
      simData::BeamUpdate* update = ds_->addBeamUpdate(beamId_, &xaction);
      update->set_time(t_);

      update->set_range(rangeSlider_->getValue());
      update->set_azimuth(azimuthSlider_->getValue() * simCore::DEG2RAD);
      update->set_elevation(elevSlider_->getValue() * simCore::DEG2RAD);

      xaction.complete(&update);
    }

    ds_->update(t_);

    // update labels.
    typeLabel_->setText(types_[typeIndex].second);
    modeLabel_->setText(modes_[modeIndex].second);
    colorLabel_->setText(colors_[colorIndex].second);

    // global mask toggle.
    unsigned displayMask = view_->getDisplayMask();
    view_->setDisplayMask(globalToggle_->getValue() ?
      (displayMask |  simVis::DISPLAY_MASK_BEAM) :
      (displayMask & ~simVis::DISPLAY_MASK_BEAM));

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
  top->addControl(new ui::LabelControl("Beams - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  ui::Grid* grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Type"));
  app.typeLabel_ = grid->setControl(c+1, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Draw Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 2, applyUI.get()));
  app.modeSlider_->setHorizFill(true, 250);
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());

  r++;
  grid->setControl(c, r, new ui::LabelControl("Range"));
  app.rangeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0.0, 2500.0, 250.0, applyUI.get()));
  app.rangeLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.rangeSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Horiz. Size"));
  app.horizSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(.01, 400.0, 45.0, applyUI.get()));
  app.horizLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.horizSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Vert. Size"));
  app.vertSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(.01, 200.0, 45.0, applyUI.get()));
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
  grid->setControl(c, r, new ui::LabelControl("Cap Res."));
  app.capResSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(1, 20, 15, applyUI.get()));
  app.capResLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.capResSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Cone Res."));
  app.coneResSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(4, 40, 30, applyUI.get()));
  app.coneResLabel_  = grid->setControl(c+2, r, new ui::LabelControl(app.coneResSlider_.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Use Offset"));
  app.useOffsetIconCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Shaded"));
  app.shadedCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Blended"));
  app.blendedCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Render Cone"));
  app.renderConeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Animate"));
  app.animateCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Global Beam Toggle"));
  app.globalToggle_ = grid->setControl(c+1, r, new ui::CheckBoxControl(true, applyUI.get()));

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

#ifndef HAVE_IMGUI
  /// Set up the application data
  AppData app;
  app.ds_     = &dataStore;
  app.view_   = viewer->getMainView();
  app.hostId_ = hostId;
  app.beamId_ = beamId;
#endif

  osg::observer_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(hostId);
  viewer->getMainView()->tetherCamera(platformModel.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(-45, -45, 500.0);

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, beamId, viewer->getMainView()));
#else
  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();
#endif

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

