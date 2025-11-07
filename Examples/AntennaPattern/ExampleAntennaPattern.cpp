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
 * Antenna Pattern Example.
 *
 * Demonstrates the visualization of the antenna pattern associated with a beam.
 * simVis supports a variety of settings for the antenna pattern algorithm,
 * polarity, sensitivity, frequency, gain, power, beam size, and more.
 * This example lets you adjust each property and visualize the calculated 3D pattern.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"

#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Beam.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"
#include "osgEarth/Sky"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static const std::string s_title = "Antenna Pattern Example";

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId beamId)
    : simExamples::SimExamplesGui("Antenna Pattern Example"),
    ds_(ds),
    beamId_(beamId)
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
      const simData::BeamPrefs* prefs = ds_.beamPrefs(beamId_, &xaction);

      // Algorithm combo box
      ImGui::TableNextColumn(); ImGui::Text("Algorithm"); ImGui::TableNextColumn();
      static const char* ALGORITHMS[] = { "PEDESTAL", "GAUSS", "CSCSQ", "SINXX", "OMNI" };
      static int currentAlgIdx = 0;
      if (ImGui::BeginCombo("##alg", ALGORITHMS[currentAlgIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(ALGORITHMS); i++)
        {
          const bool isSelected = (currentAlgIdx == i);
          if (ImGui::Selectable(ALGORITHMS[i], isSelected))
            currentAlgIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentAlgIdx + 1 != static_cast<int>(alg_))
      {
        needUpdate = true;
        alg_ = static_cast<simData::AntennaPatterns::Algorithm>(currentAlgIdx + 1);
      }

      // Polarity combo box
      ImGui::TableNextColumn(); ImGui::Text("Polarity"); ImGui::TableNextColumn();
      static const char* POLARITY[] = { "UNKNOWN", "HORIZONTAL", "VERTICAL", "CIRCULAR", "HORZVERT", "VERTHORZ", "LEFTCIRC", "RIGHTCIRC", "LINEAR" };
      static int currentPolIdx = 0;
      if (ImGui::BeginCombo("##pol", POLARITY[currentPolIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(POLARITY); i++)
        {
          const bool isSelected = (currentPolIdx == i);
          if (ImGui::Selectable(POLARITY[i], isSelected))
            currentPolIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentPolIdx != static_cast<int>(polarity_))
      {
        needUpdate = true;
        polarity_ = static_cast<simData::Polarity>(currentPolIdx);
      }

      // Sensitivity
      float sensitivity = sensitivity_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Sensitivity", &sensitivity_, -100.f, 0.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (sensitivity != sensitivity_)
        needUpdate = true;

      // Frequency
      float frequency = frequency_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Frequency", &frequency_, 0.f, 10000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (frequency != frequency_)
        needUpdate = true;

      // Gain
      float gain = gain_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Gain", &gain_, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (gain != gain_)
        needUpdate = true;

      // Power
      float power = power_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Power", &power_, 0.f, 20000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (power != power_)
        needUpdate = true;

      // Beam Width
      float beamWidth = beamWidth_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Beam Width", &beamWidth_, 1.f, 45.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (beamWidth != beamWidth_)
        needUpdate = true;

      // Beam Height
      float beamHeight = beamHeight_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Beam Height", &beamHeight_, 1.f, 45.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (beamHeight != beamHeight_)
        needUpdate = true;

      // Scale
      float scale = scale_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Scale", &beamHeight_, 1.f, 1000.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (scale != scale_)
        needUpdate = true;

      // Field of View
      float fov = fov_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Field of View", &fov_, 1.f, 360.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (fov != fov_)
        needUpdate = true;

      // Detail Angle
      float detailAngle = detailAngle_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Detail Angle", &detailAngle_, 1.f, 15.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (detailAngle != detailAngle_)
        needUpdate = true;

      // Weighting
      bool weighting = weighting_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Weighting", &weighting_);
      if (weighting != weighting_)
        needUpdate = true;

      // Color Scale
      bool colorScale = colorScale_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Color Scale", &colorScale_);
      if (colorScale != colorScale_)
        needUpdate = true;

      // Blending
      bool blending = blending_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Blending", &blending_);
      if (blending != blending_)
        needUpdate = true;

      // Lighting
      bool lighting = lighting_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Lighting", &lighting_);
      if (lighting != lighting_)
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
    std::vector<std::pair<simData::AntennaPatterns::Algorithm, std::string> > algs;
    algs.push_back(std::make_pair(simData::AntennaPatterns::Algorithm::PEDESTAL, "PEDESTAL"));
    algs.push_back(std::make_pair(simData::AntennaPatterns::Algorithm::GAUSS, "GAUSS"));
    algs.push_back(std::make_pair(simData::AntennaPatterns::Algorithm::CSCSQ, "CSCSQ"));
    algs.push_back(std::make_pair(simData::AntennaPatterns::Algorithm::SINXX, "SINXX"));
    algs.push_back(std::make_pair(simData::AntennaPatterns::Algorithm::OMNI, "OMNI"));

    simData::DataStore::Transaction xaction;
    simData::BeamPrefs* prefs = ds_.mutable_beamPrefs(beamId_, &xaction);
    prefs->set_drawtype(simData::BeamPrefs::DrawType::ANTENNA_PATTERN);
    prefs->mutable_antennapattern()->set_type(simData::AntennaPatterns::Type::ALGORITHM);
    prefs->mutable_antennapattern()->set_algorithm(alg_);
    prefs->mutable_antennapattern()->set_filename(algs[static_cast<int>(alg_) - 1].second);
    prefs->set_polarity(polarity_);
    prefs->set_sensitivity(sensitivity_);
    prefs->set_fieldofview(simCore::DEG2RAD * fov_);
    prefs->set_horizontalwidth(simCore::DEG2RAD * beamWidth_);
    prefs->set_verticalwidth(simCore::DEG2RAD * beamHeight_);
    prefs->set_gain(gain_);
    prefs->set_detail(detailAngle_);
    prefs->set_power(power_);
    prefs->set_frequency(frequency_);
    prefs->set_weighting(weighting_);
    prefs->set_colorscale(colorScale_);
    prefs->set_beamscale(scale_);
    prefs->set_blended(blending_);
    prefs->set_shaded(lighting_);
    prefs->mutable_commonprefs()->set_draw(true);
    prefs->mutable_commonprefs()->set_datadraw(true);
    xaction.complete(&prefs);
  }

  simData::MemoryDataStore& ds_;
  simData::ObjectId beamId_;
  simData::AntennaPatterns::Algorithm alg_ = simData::AntennaPatterns::Algorithm::PEDESTAL;
  simData::Polarity polarity_ = simData::Polarity::POL_UNKNOWN;
  float sensitivity_ = -50.f;
  float frequency_ = 7000.f;
  float gain_ = 20.f;
  float power_ = 2000.f;
  float beamWidth_ = 3.f;
  float beamHeight_ = 3.f;
  float scale_ = 1.f;
  float fov_ = 85.f;
  float detailAngle_ = 5.f;
  bool weighting_ = true;
  bool colorScale_ = true;
  bool blending_ = true;
  bool lighting_ = false;
};

#endif

//----------------------------------------------------------------------------

void simulate(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.5, -0.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0.5,  0.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim.get());
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0);
  viewer->addEventHandler(simHandler.get());
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  // Install the map:
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());
  if (scene->getSkyNode())
    scene->getSkyNode()->setDateTime(osgEarth::DateTime(2012, 0, 0, 11.0));

#ifdef HAVE_IMGUI
  ::GUI::OsgImGuiHandler* gui = new ::GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);

  simData::MemoryDataStore ds;
  simData::ObjectId platformId;
  simData::ObjectId beamId;
  scene->getScenario()->bind(&ds);

  // create the platform:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* newProps = ds.addPlatform(&xaction);
    platformId = newProps->id();
    xaction.complete(&newProps);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs =ds.mutable_platformPrefs(platformId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // create the beam:
  {
    simData::DataStore::Transaction xaction;
    simData::BeamProperties* props = ds.addBeam(&xaction);
    props->set_hostid(platformId);
    props->set_type(simData::BeamProperties::Type::ABSOLUTE_POSITION);
    beamId = props->id();
    xaction.complete(&props);
  }

  // make the sim
  simulate(platformId, ds, viewer.get());
  // zoom the camera
  viewer->getMainView()->tetherCamera(scene->getScenario()->find(platformId));
  viewer->getMainView()->setFocalOffsets(0, -45, 250000.);

  gui->add(new ControlPanel(ds, beamId));
#endif

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

