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
* Demonstrates and tests the display of a RF Propagation pattern.
* Allows the operator to configure the input values to the propagation on the fly.
*/

#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include "osg/Depth"
#include "osgGA/GUIActionAdapter"
#include "osgDB/FileUtils"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/LUT/LUT2.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Interpolation.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simNotify/Notify.h"
#include "simVis/Viewer.h"
#include "simVis/Utils.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/RFProp/ArepsLoader.h"
#include "simVis/RFProp/ProfileManager.h"
#include "simVis/RFProp/LUTProfileDataProvider.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/ThresholdColorProvider.h"
#include "simVis/RFProp/GradientColorProvider.h"
#include "simUtil/ExampleResources.h"

using namespace osgEarth::Util;
#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
using namespace osgEarth::Util::Controls;
#endif
using namespace simCore::LUT;

/**************************************************************/

/**
* Compute the free space loss
http://en.wikipedia.org/wiki/Free-space_path_loss
* @param distance Distance in meters, should be non-zero
* @param frequency Frequency in MHz, should be non-zero
* @return free space loss in dB
*/
double freeSpaceLoss(double distance, double freq)
{
  // Avoid log10(0)
  if (distance == 0.0 || freq == 0.0)
    return std::numeric_limits<double>::max();
  return 20 * log10(distance) + 20 * log10(freq) - 27.55;
}

/**
* Determine bearing angle from AREPS filename
* @param infilename AREPS filename
* @return bearing angle in radians
*/
double getBearingAngle(const std::string& infilename)
{
  // According to SPAWAR, the bearing angle is used in making the
  // file name, hence it is not found in the AREPS ASCII file.
  double bearing = -1;
  if (infilename.empty()) return bearing;

  // tokenize based on "_", which are used to delineate bearing angle
  std::vector<std::string> tmpFileVec;
  simCore::stringTokenizer(tmpFileVec, simCore::StringUtils::beforeLast(infilename, ".txt"), "_");

  // at a minimum, two tokens should be expected
  if (tmpFileVec.size() < 2)
    return bearing;

  // process tokens, transfer all tokens to the right of "APM"
  std::vector<std::string>::const_reverse_iterator riter;
  std::vector<std::string> bearingVec;
  for (riter = tmpFileVec.rbegin(); riter != tmpFileVec.rend(); ++riter)
  {
    if (simCore::stringCaseFind(*riter, "APM") != std::string::npos)
    {
      break;
    }
    else
    {
      bearingVec.push_back(*riter);
    }
  }

  // reverse order of bearing calc due to vector "push_back"
  switch (bearingVec.size())
  {
  case 1: // degrees
  {
    bearing = atof(bearingVec[0].c_str());
    break;
  }
  case 2: // degrees minutes
  {
    bearing = atof(bearingVec[1].c_str()) + (atof(bearingVec[0].c_str()) / 60.);
    break;
  }
  case 3: // degrees minutes seconds
  {
    bearing = atof(bearingVec[2].c_str()) + (atof(bearingVec[1].c_str()) / 60.) + (atof(bearingVec[0].c_str()) / 3600.);
    break;
  }
  }
  // convert degrees to radians
  return simCore::angFix2PI(bearing * simCore::DEG2RAD);
}

static bool s_autoBearing = false;
static float s_alphaValue = 1.f;

/**
*Automatically adjusts the bearing each frame
*/
struct AutoBearingHandler : public osgGA::GUIEventHandler
{
  explicit AutoBearingHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    double rate = osg::DegreesToRadians(20.0);
    osg::Timer_t time = osg::Timer::instance()->tick();
    if (s_autoBearing)
    {
      pm_->setBearing(pm_->getBearing() + rate * (osg::Timer::instance()->delta_s(lastTick_, time)));
    }
    lastTick_ = time;
    return false;
  }

  osg::Timer_t lastTick_;
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

struct ControlPanel : public GUI::BaseGui
{
  float minHeight;
  float maxHeight;
  int numHeights;
  float minFsl;
  float maxFsl;
  osg::ref_ptr<simRF::GradientColorProvider> podColorProvider;
  osg::ref_ptr<simRF::GradientColorProvider> lossColorProvider;
  osg::ref_ptr<simRF::GradientColorProvider> heatColorProvider;

  ControlPanel(simRF::ProfileManager* pm, simRF::ThresholdColorProvider* tcp)
    : GUI::BaseGui("RF Prop Example"),
    pm_(pm),
    tcp_(tcp)
  {
    history_ = pm_->getHistory();
    agl_ = pm_->getAGL();
    bearing_ = pm_->getBearing();
    sphericalEarth_ = pm_->getSphericalEarth();
    height_ = pm_->getHeight();
    thickness_ = pm_->getDisplayThickness();
    elevAngle_ = pm_->getElevAngle();
    alpha_ = pm_->getAlpha();
    threshold_ = 130.f;
    tcp_->setThreshold(threshold_);
    tcp_->setMode(simRF::ThresholdColorProvider::COLORMODE_ABOVE_AND_BELOW);
    pm_->setColorProvider(tcp_.get());
  }

  void draw(osg::RenderInfo& ri) override
  {
    // This GUI positions bottom left instead of top left, need the size of the window
    ImVec2 viewSize = ImGui::GetMainViewport()->WorkSize;
    ImGui::SetNextWindowPos(ImVec2(15, viewSize.y - 15), 0, ImVec2(0, 1));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing);

    if (ImGui::BeginTable("Table", 2))
    {
      float history = history_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "History", &history_, 0.f, 360.f, "", ImGuiSliderFlags_AlwaysClamp);
      if (history != history_)
        pm_->setHistory(osg::DegreesToRadians(history_));

      bool agl = agl_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "AGL", &agl_);
      if (agl != agl_)
        pm_->setAGL(agl_);

      float bearing = bearing_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Bearing", &bearing_, 0.f, 360.f, "", ImGuiSliderFlags_AlwaysClamp);
      if (bearing != bearing_)
        pm_->setBearing(osg::DegreesToRadians(bearing_));

      IMGUI_ADD_ROW(ImGui::Checkbox, "Auto Bearing", &s_autoBearing);

      bool sphericalEarth = sphericalEarth_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Spherical Earth", &sphericalEarth_);
      if (sphericalEarth != sphericalEarth_)
        pm_->setSphericalEarth(sphericalEarth_);

      float height = height_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Height", &height_, minHeight, maxHeight, "", ImGuiSliderFlags_AlwaysClamp);
      if (height != height_)
      {
        pm_->setHeight(height_);
        std::cout << "Hgt: " << height_ << std::endl;
      }

      int thickness = thickness_;
      IMGUI_ADD_ROW(ImGui::SliderInt, "Thickness", &thickness_, 1, numHeights, "", ImGuiSliderFlags_AlwaysClamp);
      if (thickness != thickness_)
        pm_->setDisplayThickness(thickness_);

      float elevAngle = elevAngle_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Elev Angle", &elevAngle_, 0.f, 90.f, "", ImGuiSliderFlags_AlwaysClamp);
      if (elevAngle != elevAngle_)
        pm_->setElevAngle(osg::DegreesToRadians(elevAngle_));

      float alpha = s_alphaValue;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Alpha", &s_alphaValue, 0.f, 1.f, "", ImGuiSliderFlags_AlwaysClamp);
      if (alpha != s_alphaValue)
        pm_->setAlpha(alpha);

      // Draw mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Draw Mode"); ImGui::TableNextColumn();
      static const char* DRAWMODES[] = { "2D Horz", "2D Vert", "Tee", "3D", "3D Texture", "3D Points", "RAE" };
      static int currentModeIdx = static_cast<int>(drawMode_);
      if (ImGui::BeginCombo("##drawmodes", DRAWMODES[currentModeIdx], 0))
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
      if (currentModeIdx != static_cast<int>(drawMode_))
      {
        drawMode_ = static_cast<simRF::Profile::DrawMode>(currentModeIdx);
        pm_->setMode(drawMode_);
      }

      // Color scheme combo box
      ImGui::TableNextColumn(); ImGui::Text("Color Scheme"); ImGui::TableNextColumn();
      static const char* COLORSCHEMES[] = { "Threshold", "Grad1", "Grad2", "Grad3" };
      int currentColorIdx = colorSchemeIdx_;
      if (ImGui::BeginCombo("##colorschemes", COLORSCHEMES[currentColorIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(COLORSCHEMES); i++)
        {
          const bool isSelected = (currentColorIdx == i);
          if (ImGui::Selectable(COLORSCHEMES[i], isSelected))
            currentColorIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentColorIdx != colorSchemeIdx_)
      {
        colorSchemeIdx_ = currentColorIdx;
        switch (colorSchemeIdx_)
        {
        case 0:
          pm_->setColorProvider(tcp_.get());
          break;
        case 1:
          pm_->setColorProvider(podColorProvider.get());
          podColorProvider->setDiscrete(discrete_);
          break;
        case 2:
          pm_->setColorProvider(lossColorProvider.get());
          lossColorProvider->setDiscrete(discrete_);
          break;
        case 3:
          pm_->setColorProvider(heatColorProvider.get());
          heatColorProvider->setDiscrete(discrete_);
          break;
        default:
          assert(0); // Unexpected input value
          break;
        }
      }

      float threshold = threshold_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Threshold", &threshold_, minFsl, maxFsl, "", ImGuiSliderFlags_AlwaysClamp);
      if (threshold != threshold_)
      {
        tcp_->setThreshold(threshold_);
        std::cout << "Threshold: " << threshold_ << std::endl;
      }

      // Threshold mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Threshold Mode"); ImGui::TableNextColumn();
      static const char* THRESHOLDMODES[] = { "Below", "Above", "Above & Below" };
      static int currentTModeIdx = static_cast<int>(thresholdMode_);
      if (ImGui::BeginCombo("##threshmodes", THRESHOLDMODES[currentTModeIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(THRESHOLDMODES); i++)
        {
          const bool isSelected = (currentTModeIdx == i);
          if (ImGui::Selectable(THRESHOLDMODES[i], isSelected))
            currentTModeIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentTModeIdx != static_cast<int>(thresholdMode_))
      {
        thresholdMode_ = static_cast<simRF::ColorProvider::ColorMode>(currentTModeIdx);
        tcp_->setMode(thresholdMode_);
      }

      bool discrete = discrete_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Discrete Gradient", &discrete_);
      if (discrete != discrete_)
      {
        podColorProvider->setDiscrete(discrete_);
        lossColorProvider->setDiscrete(discrete_);
        heatColorProvider->setDiscrete(discrete_);
      }

      bool depthTest = depthTest_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Depth Test", &depthTest_);
      if (depthTest != depthTest_)
      {
        osg::StateSet* stateset = pm_->getOrCreateStateSet();
        stateset->setMode(GL_DEPTH_TEST, depthTest_ ? osg::StateAttribute::ON : osg::StateAttribute::OFF);
        stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, depthTest_));
      }

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  osg::ref_ptr<simRF::ProfileManager> pm_;
  osg::ref_ptr<simRF::ThresholdColorProvider> tcp_;
  float history_;
  bool agl_;
  float bearing_;
  bool sphericalEarth_;
  float height_;
  int thickness_;
  float elevAngle_;
  float alpha_;
  simRF::Profile::DrawMode drawMode_ = simRF::Profile::DRAWMODE_2D_HORIZONTAL;
  float threshold_;
  simRF::ColorProvider::ColorMode thresholdMode_ = simRF::ColorProvider::COLORMODE_ABOVE_AND_BELOW;
  bool discrete_ = true;
  bool depthTest_ = false;
  int colorSchemeIdx_ = 0;
};
#else
static osg::ref_ptr<Grid> s_controlGrid = nullptr;
std::vector<osg::ref_ptr<LabelControl> > s_modes;
std::vector<osg::ref_ptr<LabelControl> > s_colorProviders;
std::vector<osg::ref_ptr<LabelControl> > s_ThresholdModes;
osg::ref_ptr<CheckBoxControl> s_DiscreteGradientCheck;

void createControlPanel(osgViewer::View* view)
{
  ControlCanvas* canvas = ControlCanvas::get(view);

  // the outer container:
  s_controlGrid = new Grid();
  s_controlGrid->setBackColor(0, 0, 0, 0.5);
  s_controlGrid->setMargin(10);
  s_controlGrid->setPadding(10);
  s_controlGrid->setChildSpacing(10);
  s_controlGrid->setChildVertAlign(Control::ALIGN_CENTER);
  s_controlGrid->setAbsorbEvents(true);
  s_controlGrid->setVertAlign(Control::ALIGN_BOTTOM);

  canvas->addControl(s_controlGrid.get());
}

struct HistoryHandler : public ControlEventHandler
{
  explicit HistoryHandler(simRF::ProfileManager* pm) :
    pm_(pm) {}

  void onValueChanged(Control* control, float value)
  {
    pm_->setHistory(osg::DegreesToRadians(value));
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct BearingHandler : public ControlEventHandler
{
  explicit BearingHandler(simRF::ProfileManager* pm) :
    pm_(pm) {}

  void onValueChanged(Control* control, float value)
  {
    pm_->setBearing(osg::DegreesToRadians(value));
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct ElevAngleHandler : public ControlEventHandler
{
  explicit ElevAngleHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, float value)
  {
    pm_->setElevAngle(osg::DegreesToRadians(value));
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct AutoBearingEnabledHandler : public ControlEventHandler
{
  AutoBearingEnabledHandler() {}

  void onValueChanged(Control* control, bool value)
  {
    s_autoBearing = value;
  }
};

struct AGLHandler : public ControlEventHandler
{
  explicit AGLHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, bool value)
  {
    pm_->setAGL(value);
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct SphericalEarthHandler : public ControlEventHandler
{
  explicit SphericalEarthHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, bool value)
  {
    pm_->setSphericalEarth(value);
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct ThicknessHandler : public ControlEventHandler
{
  explicit ThicknessHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, float value)
  {
    pm_->setDisplayThickness(value);
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct AlphaHandler : public ControlEventHandler
{
  explicit AlphaHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, float value)
  {
    s_alphaValue = value;
    pm_->setAlpha(s_alphaValue);
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct HeightHandler : public ControlEventHandler
{
  explicit HeightHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, float value)
  {
    pm_->setHeight(value);
    std::cout << "Hgt: " << value << std::endl;
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct ThresholdHandler : public ControlEventHandler
{
  ThresholdHandler(simRF::ThresholdColorProvider* cp, simRF::ProfileManager* pm) :
    cp_(cp),
    pm_(pm)
  { }

  void onValueChanged(Control* control, float value) {
    cp_->setThreshold(value);
    std::cout << "Threshold: " << value << std::endl;
  }
  osg::ref_ptr<simRF::ThresholdColorProvider> cp_;
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct ModeSelectHandler : public ControlEventHandler
{
  ModeSelectHandler(simRF::ProfileManager* pm, simRF::Profile::DrawMode mode) :
    pm_(pm),
    mode_(mode)
  {
  }

  void onClick(Control* control)
  {
    //Disable all the mode controls
    for (unsigned int i = 0; i < s_modes.size(); i++)
    {
      s_modes[i]->setForeColor(simVis::Color::White);
    }

    //Enable this mode
    control->setForeColor(simVis::Color::Lime);

    pm_->setMode(mode_);
  }

  osg::ref_ptr<simRF::ProfileManager> pm_;
  simRF::Profile::DrawMode mode_;
};

struct ThresholdModeHandler : public ControlEventHandler
{
  ThresholdModeHandler(simRF::ProfileManager* pm, simRF::ThresholdColorProvider* provider, simRF::ColorProvider::ColorMode mode) :
    pm_(pm),
    mode_(mode),
    provider_(provider)
  {
  }

  void onClick(Control* control)
  {
    //Disable all the mode controls
    for (unsigned int i = 0; i < s_ThresholdModes.size(); i++)
    {
      s_ThresholdModes[i]->setForeColor(simVis::Color::White);
    }

    //Enable this mode
    control->setForeColor(simVis::Color::Lime);

    provider_->setMode(mode_);
  }

  osg::ref_ptr<simRF::ProfileManager> pm_;
  simRF::ColorProvider::ColorMode mode_;
  osg::ref_ptr<simRF::ThresholdColorProvider> provider_;
};

struct ColorProviderSelectHandler : public ControlEventHandler
{
  ColorProviderSelectHandler(simRF::ProfileManager* pm, simRF::ColorProvider* cp) :
    pm_(pm),
    cp_(cp)
  {
  }

  void onClick(Control* control)
  {
    //Disable all the mode controls
    for (unsigned int i = 0; i < s_colorProviders.size(); i++)
    {
      s_colorProviders[i]->setForeColor(simVis::Color::White);
    }

    //Enable this mode
    control->setForeColor(simVis::Color::Lime);

    pm_->setColorProvider(cp_.get());
    // Update the gradient to whatever the state of the checkbox is
    simRF::GradientColorProvider* gradient = dynamic_cast<simRF::GradientColorProvider*>(cp_.get());
    if (gradient)
      gradient->setDiscrete(s_DiscreteGradientCheck->getValue());
  }

  osg::ref_ptr<simRF::ProfileManager> pm_;
  osg::ref_ptr<simRF::ColorProvider> cp_;
};

struct DiscreteHandler : public ControlEventHandler
{
  explicit DiscreteHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, bool value)
  {
    simRF::GradientColorProvider* cp = dynamic_cast<simRF::GradientColorProvider*>(pm_->getColorProvider());
    if (cp)
    {
      cp->setDiscrete(value);
      OSG_NOTICE << "Setting discrete " << value << std::endl;
    }
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

struct DepthTestHandler : public ControlEventHandler
{
  explicit DepthTestHandler(simRF::ProfileManager* pm)
    : pm_(pm)
  {
  }

  void onValueChanged(Control* control, bool value)
  {
    OSG_NOTICE << "Setting depth test " << value << std::endl;
    osg::StateSet* stateset = pm_->getOrCreateStateSet();
    if (value)
    {
      stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
      stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, true));
    }
    else
    {
      stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
      stateset->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
    }
  }
  osg::ref_ptr<simRF::ProfileManager> pm_;
};

Control* createModeSelect(const std::string& name, bool enabled, simRF::Profile::DrawMode mode, simRF::ProfileManager* pm)
{
  LabelControl* c = new LabelControl(name);
  c->setFontSize(16);
  c->setForeColor(enabled ? simVis::Color::Lime : simVis::Color::White);
  c->addEventHandler(new ModeSelectHandler(pm, mode));
  s_modes.push_back(c);
  return c;
}

Control* createColorProviderSelect(const std::string& name, bool enabled, simRF::ColorProvider* cp, simRF::ProfileManager* pm)
{
  LabelControl* c = new LabelControl(name);
  c->setFontSize(16);
  c->setForeColor(enabled ? simVis::Color::Lime : simVis::Color::White);
  c->addEventHandler(new ColorProviderSelectHandler(pm, cp));
  s_colorProviders.push_back(c);
  return c;
}

Control* createThresholdModeSelect(const std::string& name, bool enabled, simRF::ThresholdColorProvider* cp, simRF::ColorProvider::ColorMode mode, simRF::ProfileManager* pm)
{
  LabelControl* c = new LabelControl(name);
  c->setFontSize(16);
  c->setForeColor(enabled ? simVis::Color::Lime : simVis::Color::White);
  c->addEventHandler(new ThresholdModeHandler(pm, cp, mode));
  s_ThresholdModes.push_back(c);
  return c;
}

HSliderControl* createSlider(float min, float max, float value)
{
  HSliderControl* slider = new HSliderControl(min, max, value);
  slider->setWidth(125);
  slider->setHeight(12);
  slider->setPadding(0);
  slider->setMargin(0);
  slider->setVertAlign(Control::ALIGN_CENTER);
  return slider;
}
#endif

int main(int argc, char** argv)
{
  simCore::checkVersionThrow();
  osg::ArgumentParser arguments(&argc, argv);
  simExamples::configureSearchPaths();

  const double lat = 33.026669;
  const double lon = -118.578636;
  double alt = 1000;

  double minRange = 100;
  double maxRange = 100000;
  unsigned int numRanges = 75;

  double minHeight = 0;
  double maxHeight = 10000;
  unsigned int numHeights = 75;

  std::vector<std::string> arepsFiles;

  for (int i = 1; i < argc; i++)
  {
    std::string cmdLineArg = argv[i];
    if (cmdLineArg.find("--range") != std::string::npos)
    {
      i++;
      assert(i < argc);
      minRange = atof(argv[i]);
      i++;
      assert(i < argc);
      maxRange = atof(argv[i]);
    }
    else if (cmdLineArg.find("--height") != std::string::npos)
    {
      i++;
      assert(i < argc);
      minHeight = atof(argv[i]);
      i++;
      assert(i < argc);
      maxHeight = atof(argv[i]);
    }
    else if (cmdLineArg.find("--numRanges") != std::string::npos)
    {
      i++;
      assert(i < argc);
      numRanges = static_cast<unsigned int>(atoi(argv[i]));
    }
    else if (cmdLineArg.find("--numHeights") != std::string::npos)
    {
      i++;
      assert(i < argc);
      numHeights = static_cast<unsigned int>(atoi(argv[i]));
    }
    else if (cmdLineArg.find("--files") != std::string::npos)
    {
      i++;
      std::string filename;
      bool found = true;
      while (i < argc && found)
      {
        assert(i < argc);
        filename = argv[i];
        if (filename.find(".txt") != std::string::npos)
        {
          arepsFiles.push_back(filename);
          i++;
        }
        else
        {
          // See if it's a directory
          if (osgDB::fileType(filename) == osgDB::DIRECTORY)
          {
            osgDB::DirectoryContents contents = osgDB::getDirectoryContents(filename);
            for (unsigned int j = 0; j < contents.size(); j++)
            {
              std::string f = contents[j];
              if (f.find(".txt") != std::string::npos)
              {
                arepsFiles.push_back(f);
              }
            }
            found = true;
            i++;
          }
          else
          {
            found = false;
          }
        }
      }
    }
  }

  // initialize a SIMDIS viewer and load a planet.
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer(arguments);
  viewer->setLogarithmicDepthBufferEnabled(true);
  viewer->setMap(simExamples::createDefaultExampleMap());

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  osg::ref_ptr<osg::Group> root = new osg::Group();
  osg::ref_ptr<simRF::ProfileManager> profileManager = new simRF::ProfileManager(nullptr);

  // min & max loss values for gradient color setting in dB; valid loss data is [0,300]
  const double minFSL = 0;
  const double maxFSL = 300;

  const unsigned int numProfiles = 180;
  const double beamWidth = 0.7 * M_PI * 2.0 / numProfiles;
  const double bearingStep = M_PI * 2.0 / numProfiles;

  // Determine if AREPS files have been loaded
  if (arepsFiles.empty())
  {
    const double rangeStep = (maxRange - minRange) / numRanges;
    const double heightStep = (maxHeight - minHeight) / numHeights;
    //Generate some fake terrain heights for the AGL mode
    std::map<float, float> terrain;
    for (unsigned i = 0; i < numRanges; i++)
    {
      float range = static_cast<float>(minRange + rangeStep * i);
      terrain[range] = static_cast<float>(5000 * sin(3.0 * M_PI * ((2.0 * i) / numRanges)));
    }

    // Generate propagation data based on free space model
    const double freq = 3000; //MHz
    for (unsigned int i = 0; i < numProfiles; i++)
    {
      LUT2<short> * loss = new LUT2<short>();
      loss->initialize(minHeight, maxHeight, numHeights, minRange, maxRange, numRanges);

      // Only respect the height on every 5th profile -- we fake height processing below
      const bool respectHeight = (i % 5 == 0);

      // Loop through heights
      for (unsigned int h = 0; h < numHeights; h++)
      {
        // Maintain the max terrain height seen
        double maxTerrainHeightSeen = 0.0;

        // Loop through each range cell for this height
        for (unsigned int r = 0; r < numRanges; r++)
        {
          const double range = minRange + rangeStep * r;
          // compute loss in dB
          const double fsl = freeSpaceLoss(range, freq);

          // Reduce the DB by an amount if we're respecting height
          double reducedFsl = fsl;
          if (respectHeight)
          {
            const auto heightIter = terrain.lower_bound(range);
            if (heightIter != terrain.end())
              maxTerrainHeightSeen = simCore::sdkMax(maxTerrainHeightSeen, static_cast<double>(heightIter->second));
            // Reduce max height by 3% of range, to give a bad simulation of ducting effects on graphics
            maxTerrainHeightSeen = simCore::sdkMax(0.0, maxTerrainHeightSeen - rangeStep * 0.03);

            const float sliceHeight = static_cast<float>(minHeight + heightStep * h);
            const float hat = sliceHeight - maxTerrainHeightSeen;
            if (hat < 0.0)
              reducedFsl = 300.0; // db; complete loss
            else if (hat < 100.0) // scale down to 300db, linearly from 100m to 0m
              reducedFsl = simCore::linearInterpolate(300.0, fsl, 0.0, hat, 100.0);
          }

          // We scale the value by 10 so it fits well in a short for memory efficiency
          const short scaledFsl = static_cast<short>(reducedFsl * simRF::SCALE_FACTOR);

          // store loss as cB (centibels) for each height slot
          (*loss)(h, r) = scaledFsl;
        }
      }

      // loss data provided must be populated prior to assigning to profile
      osg::ref_ptr<simRF::CompositeProfileProvider> cProvider = new simRF::CompositeProfileProvider();
      cProvider->addProvider(new simRF::LUTProfileDataProvider(loss, simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS, 1.0 / simRF::SCALE_FACTOR));
      osg::ref_ptr<simRF::Profile> profile = new simRF::Profile(cProvider.get());
      profile->setHalfBeamWidth(beamWidth / 2.0);
      profile->setBearing(bearingStep * i);
      profile->setTerrainHeights(terrain);
      profileManager->addProfile(profile.get());
    }
  }
  else
  {
    // load command line AREPS files

    setNotifyLevel(simNotify::NOTIFY_INFO);
    simRF::ArepsLoader loader;

    // Process AREPS files
    for (size_t ii = 0; ii < arepsFiles.size(); ii++)
    {
      osg::ref_ptr<simRF::Profile> profile = new simRF::Profile(new simRF::CompositeProfileProvider());
      const bool firstFile = (ii == 0);
      if (0 != loader.loadFile(arepsFiles[ii], *profile, firstFile))
      {
        // failed to load a file
        break;
      }

      // successfully loaded the file, created the profile, and populated the profile with data
      assert(profile);
      profileManager->addProfile(profile.get());

      if (firstFile)
      {
        alt = loader.getAntennaHeight();
        const simRF::ProfileDataProvider* provider = profile->getDataProvider()->getProvider(simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS);
        if (provider)
        {
          minHeight = provider->getMinHeight();
          maxHeight = provider->getMaxHeight();
          numHeights = provider->getNumHeights();
        }
      }
    }
    setNotifyLevel(simNotify::NOTIFY_NOTICE);
  }

  profileManager->setThresholdType(simRF::ProfileDataProvider::THRESHOLDTYPE_LOSS);
  simVis::setLighting(profileManager->getOrCreateStateSet(), osg::StateAttribute::OFF);

  osg::ref_ptr<simRF::ThresholdColorProvider> thresholdColorProvider = new simRF::ThresholdColorProvider(simVis::Color::Red, simVis::Color::Lime, (maxFSL - minFSL) / 2.0);
  profileManager->setColorProvider(thresholdColorProvider.get());

#ifdef HAVE_IMGUI
  ControlPanel* controlPanel = new ControlPanel(profileManager, thresholdColorProvider);

  // Gradient 1 is based on POD from AREPS
  simRF::GradientColorProvider::ColorMap podColors;
  podColors[0.] = simVis::Color::Red;
  podColors[300. * 0.1] = simVis::Color::Yellow;
  podColors[300. * 0.2] = simVis::Color::Magenta;
  podColors[300. * 0.3] = simVis::Color::Blue;
  podColors[300. * 0.4] = simVis::Color::Lime;
  podColors[300. * 0.5] = simVis::Color::Orange;
  podColors[300. * 0.6] = simVis::Color::Teal;
  podColors[300. * 0.7] = simVis::Color::Green;
  podColors[300. * 0.8] = simVis::Color::Navy;
  podColors[300. * 0.9] = simVis::Color::Gray;
  controlPanel->podColorProvider = new simRF::GradientColorProvider();
  controlPanel->podColorProvider->setColorMap(podColors);

  // Gradient 2 is based on Loss from AREPS
  simRF::GradientColorProvider::ColorMap lossColors;
  lossColors[0.] = simVis::Color::Red; // < 110 -> red
  lossColors[110.] = simVis::Color::Yellow;
  lossColors[115.] = simVis::Color::Magenta;
  lossColors[120.] = simVis::Color::Blue;
  lossColors[125.] = simVis::Color::Lime;
  lossColors[130.] = simVis::Color::Orange;
  lossColors[135.] = simVis::Color::Teal;
  lossColors[140.] = simVis::Color::Green;
  lossColors[145.] = simVis::Color::Navy;
  lossColors[150.] = simVis::Color::Gray;
  lossColors[155.] = simVis::Color::Cyan;
  lossColors[160.] = simVis::Color::Purple; // > 160 -> purple
  controlPanel->lossColorProvider = new simRF::GradientColorProvider();
  controlPanel->lossColorProvider->setColorMap(lossColors);

  // Gradient 3 is based on a heat scale from blue to red
  simRF::GradientColorProvider::ColorMap heatColors;
  heatColors[0.] = simVis::Color::Red;
  heatColors[300 * 0.2] = simVis::Color::Yellow;
  heatColors[300 * 0.4] = simVis::Color::Lime;
  heatColors[300 * 0.6] = simVis::Color::Cyan;
  heatColors[300 * 0.8] = simVis::Color::Blue;
  controlPanel->heatColorProvider = new simRF::GradientColorProvider();
  controlPanel->heatColorProvider->setColorMap(heatColors);

  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  controlPanel->minHeight = minHeight;
  controlPanel->maxHeight = maxHeight;
  controlPanel->numHeights = numHeights;
  controlPanel->minFsl = minFSL;
  controlPanel->maxFsl = maxFSL;
  gui->add(controlPanel);
#else
  //Create the control panel and add some controls
  createControlPanel(viewer->getMainView());

  unsigned int row = 0;

  s_controlGrid->setAbsorbEvents(false);

  //Add a history slider that goes from 0 to 360 degrees
  s_controlGrid->setControl(0, row, new LabelControl("History"));
  osg::ref_ptr<HSliderControl> historySlider = createSlider(0, 360, osg::RadiansToDegrees(profileManager->getHistory()));
  historySlider->addEventHandler(new HistoryHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, historySlider.get());
  row++;

  //Add a checkbox to determine whether the AGL height is used or not
  s_controlGrid->setControl(0, row, new LabelControl("AGL"));
  osg::ref_ptr<CheckBoxControl> aglCheckbox = new CheckBoxControl(profileManager->getAGL());
  aglCheckbox->addEventHandler(new AGLHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, aglCheckbox.get());
  row++;

  //Add a bearing slider
  s_controlGrid->setControl(0, row, new LabelControl("Bearing"));
  osg::ref_ptr<HSliderControl> bearingSlider = createSlider(0, 360, osg::RadiansToDegrees(profileManager->getBearing()));
  bearingSlider->addEventHandler(new BearingHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, bearingSlider.get());
  row++;

  //Add a checkbox to control whether the bearing is updated automatically each frame.
  s_controlGrid->setControl(0, row, new LabelControl("Auto Bearing"));
  osg::ref_ptr<CheckBoxControl> autoBearingCheckbox = new CheckBoxControl(s_autoBearing);
  autoBearingCheckbox->addEventHandler(new AutoBearingEnabledHandler());
  s_controlGrid->setControl(1, row, autoBearingCheckbox.get());
  row++;

  //Add a checkbox to control whether the profiles are spherical earth or not
  s_controlGrid->setControl(0, row, new LabelControl("Spherical Earth"));
  osg::ref_ptr<CheckBoxControl> sphericalEarthCheckbox = new CheckBoxControl(true);
  sphericalEarthCheckbox->addEventHandler(new SphericalEarthHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, sphericalEarthCheckbox.get());
  row++;

  //Add a height slider
  s_controlGrid->setControl(0, row, new LabelControl("Height"));
  osg::ref_ptr<HSliderControl> heightSlider = createSlider(minHeight, maxHeight, profileManager->getHeight());
  heightSlider->addEventHandler(new HeightHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, heightSlider.get());
  row++;

  //Add a thickness slider; used for 3D
  s_controlGrid->setControl(0, row, new LabelControl("Thickness"));
  osg::ref_ptr<HSliderControl> thicknessSlider = createSlider(1, numHeights, profileManager->getDisplayThickness());
  thicknessSlider->addEventHandler(new ThicknessHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, thicknessSlider.get());
  row++;

  //Add a elevation angle slider
  s_controlGrid->setControl(0, row, new LabelControl("Elev Angle"));
  osg::ref_ptr<HSliderControl> elevAngleSlider = createSlider(0, 90, osg::RadiansToDegrees(profileManager->getElevAngle()));
  elevAngleSlider->addEventHandler(new ElevAngleHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, elevAngleSlider.get());
  row++;

  //Add an alpha slider
  s_controlGrid->setControl(0, row, new LabelControl("Alpha"));
  osg::ref_ptr<HSliderControl> alphaSlider = createSlider(0.0, 1.0, s_alphaValue);
  alphaSlider->addEventHandler(new AlphaHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, alphaSlider.get());
  row++;

  //Add some mode selectors
  s_controlGrid->setControl(0, row, new LabelControl("Draw Mode"));
  s_controlGrid->setControl(1, row, createModeSelect("2D Horz", true, simRF::Profile::DRAWMODE_2D_HORIZONTAL, profileManager.get()));
  s_controlGrid->setControl(2, row, createModeSelect("2D Vert", false, simRF::Profile::DRAWMODE_2D_VERTICAL, profileManager.get()));
  s_controlGrid->setControl(3, row, createModeSelect("Tee", false, simRF::Profile::DRAWMODE_2D_TEE, profileManager.get()));
  s_controlGrid->setControl(4, row, createModeSelect("3D", false, simRF::Profile::DRAWMODE_3D, profileManager.get()));
  row++;
  s_controlGrid->setControl(1, row, createModeSelect("3D Texture", false, simRF::Profile::DRAWMODE_3D_TEXTURE, profileManager.get()));
  s_controlGrid->setControl(2, row, createModeSelect("3D Points", false, simRF::Profile::DRAWMODE_3D_POINTS, profileManager.get()));
  s_controlGrid->setControl(3, row, createModeSelect("RAE", false, simRF::Profile::DRAWMODE_RAE, profileManager.get()));

  row++;

  s_controlGrid->setControl(0, row, new LabelControl("Color Scheme"));


  s_controlGrid->setControl(1, row, createColorProviderSelect("Threshold", true, thresholdColorProvider.get(), profileManager.get()));

  // Gradient 1 is based on POD from AREPS
  {
  simRF::GradientColorProvider::ColorMap podColors;
  podColors[0.] = simVis::Color::Red;
  podColors[300. * 0.1] = simVis::Color::Yellow;
  podColors[300. * 0.2] = simVis::Color::Magenta;
  podColors[300. * 0.3] = simVis::Color::Blue;
  podColors[300. * 0.4] = simVis::Color::Lime;
  podColors[300. * 0.5] = simVis::Color::Orange;
  podColors[300. * 0.6] = simVis::Color::Teal;
  podColors[300. * 0.7] = simVis::Color::Green;
  podColors[300. * 0.8] = simVis::Color::Navy;
  podColors[300. * 0.9] = simVis::Color::Gray;
  osg::ref_ptr<simRF::GradientColorProvider> colorProvider = new simRF::GradientColorProvider();
  colorProvider->setColorMap(podColors);
  s_controlGrid->setControl(2, row, createColorProviderSelect("Grad1", false, colorProvider.get(), profileManager.get()));
  }

  // Gradient 2 is based on Loss from AREPS
  {
  simRF::GradientColorProvider::ColorMap lossColors;
  lossColors[0.] = simVis::Color::Red; // < 110 -> red
  lossColors[110.] = simVis::Color::Yellow;
  lossColors[115.] = simVis::Color::Magenta;
  lossColors[120.] = simVis::Color::Blue;
  lossColors[125.] = simVis::Color::Lime;
  lossColors[130.] = simVis::Color::Orange;
  lossColors[135.] = simVis::Color::Teal;
  lossColors[140.] = simVis::Color::Green;
  lossColors[145.] = simVis::Color::Navy;
  lossColors[150.] = simVis::Color::Gray;
  lossColors[155.] = simVis::Color::Cyan;
  lossColors[160.] = simVis::Color::Purple; // > 160 -> purple
  simRF::GradientColorProvider* colorProvider = new simRF::GradientColorProvider();
  colorProvider->setColorMap(lossColors);
  s_controlGrid->setControl(3, row, createColorProviderSelect("Grad2", false, colorProvider, profileManager.get()));
  }

  // Gradient 3 is based on a heat scale from blue to red
  {
  simRF::GradientColorProvider::ColorMap heatColors;
  heatColors[0.] = simVis::Color::Red;
  heatColors[300 * 0.2] = simVis::Color::Yellow;
  heatColors[300 * 0.4] = simVis::Color::Lime;
  heatColors[300 * 0.6] = simVis::Color::Cyan;
  heatColors[300 * 0.8] = simVis::Color::Blue;
  simRF::GradientColorProvider* colorProvider = new simRF::GradientColorProvider();
  colorProvider->setColorMap(heatColors);
  s_controlGrid->setControl(4, row, createColorProviderSelect("Grad3", false, colorProvider, profileManager.get()));
  }

  row++;

  //Setup the threshold slider
  s_controlGrid->setControl(0, row, new LabelControl("Threshold"));
  osg::ref_ptr<HSliderControl> threshSlider = createSlider(minFSL, maxFSL, 130.0);
  threshSlider->addEventHandler(new ThresholdHandler(thresholdColorProvider.get(), profileManager.get()));
  thresholdColorProvider->setThreshold(130.0);
  s_controlGrid->setControl(1, row, threshSlider.get());

  row++;

  s_controlGrid->setControl(0, row, new LabelControl("Threshold mode"));
  s_controlGrid->setControl(1, row, createThresholdModeSelect("Above & Below", true, thresholdColorProvider.get(), simRF::ColorProvider::COLORMODE_ABOVE_AND_BELOW, profileManager.get()));
  s_controlGrid->setControl(2, row, createThresholdModeSelect("Above", false, thresholdColorProvider.get(), simRF::ColorProvider::COLORMODE_ABOVE, profileManager.get()));
  s_controlGrid->setControl(3, row, createThresholdModeSelect("Below", false, thresholdColorProvider.get(), simRF::ColorProvider::COLORMODE_BELOW, profileManager.get()));

  row++;

  s_controlGrid->setControl(0, row, new LabelControl("Discrete Gradient"));
  s_DiscreteGradientCheck = new CheckBoxControl(true);
  s_DiscreteGradientCheck->addEventHandler(new DiscreteHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, s_DiscreteGradientCheck.get());
  row++;

  s_controlGrid->setControl(0, row, new LabelControl("Depth Test"));
  osg::ref_ptr<CheckBoxControl> depthTestCheck = new CheckBoxControl(false);
  depthTestCheck->addEventHandler(new DepthTestHandler(profileManager.get()));
  s_controlGrid->setControl(1, row, depthTestCheck.get());
  row++;
#endif

  viewer->addEventHandler(new AutoBearingHandler(profileManager.get()));

  profileManager->setRefCoord(osg::DegreesToRadians(lat), osg::DegreesToRadians(lon), alt);
  root->addChild(profileManager.get());
  profileManager->setDisplay(true);

  viewer->getSceneManager()->getScenario()->addChild(root);
  viewer->getMainView()->setViewpoint(Viewpoint("start", lon, lat, 1000, 0, -90, 100000));

  // for status and debugging
  viewer->installDebugHandlers();
  viewer->run();
}
