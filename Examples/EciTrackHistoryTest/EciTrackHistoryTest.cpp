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
 * ECI Track History TEST
 * Test app for the various features of the ECI Track History feature.
 */
#include "simData/DataTable.h"
#include "simData/LinearInterpolator.h"
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
#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#ifdef HAVE_IMGUI
#include "SimExamplesGui.h"
#include "OsgImGuiHandler.h"
#endif

namespace
{
  std::string SAYBOOL(bool x)
  {
    return x ? "ON" : "OFF";
  }
}

static float SIM_START = 0.f;
static float SIM_END = 60.f;
static float SIM_HZ = 5.f;

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(200); func("##" label, __VA_ARGS__)

class ControlPanel : public simExamples::SimExamplesGui
{
public:
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId platId, simUtil::SimulatorEventHandler* simHandler, simVis::View* view, osg::Node* platformModel)
    : simExamples::SimExamplesGui("ECI Track History Example"),
    ds_(ds),
    platId_(platId),
    simHandler_(simHandler),
    view_(view),
    platformModel_(platformModel)
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
      // Track Mode combo box
      ImGui::TableNextColumn(); ImGui::Text("Track Mode"); ImGui::TableNextColumn();
      static const char* TRACKMODES[] = { "OFF", "POINT", "LINE", "RIBBON", "BRIDGE" };
      static int currentModeIdx = static_cast<int>(trackMode_);
      if (ImGui::BeginCombo("##modes", TRACKMODES[currentModeIdx], 0))
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
      if (currentModeIdx != static_cast<int>(trackMode_))
      {
        needUpdate = true;
        trackMode_ = static_cast<simData::TrackPrefs::Mode>(currentModeIdx);
      }

      // Alt mode
      bool altMode = altMode_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Alt Mode", &altMode_);
      if (altMode != altMode_)
        needUpdate = true;

      // Draw Style combo box
      ImGui::TableNextColumn(); ImGui::Text("Draw Style"); ImGui::TableNextColumn();
      static const char* DRAWSTYLES[] = { "OFF", "POINT", "LINE" };
      static int currentStyleIdx = static_cast<int>(drawStyle_);
      if (ImGui::BeginCombo("##style", DRAWSTYLES[currentStyleIdx], 0))
      {
        for (int i = 0; i < IM_ARRAYSIZE(DRAWSTYLES); i++)
        {
          const bool isSelected = (currentStyleIdx == i);
          if (ImGui::Selectable(DRAWSTYLES[i], isSelected))
            currentStyleIdx = i;

          // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      if (currentStyleIdx != static_cast<int>(drawStyle_))
      {
        needUpdate = true;
        drawStyle_ = static_cast<simData::TimeTickPrefs::DrawStyle>(currentStyleIdx);
      }

      ImGui::TableNextColumn(); ImGui::Text("Transport"); ImGui::TableNextColumn();
      if (ImGui::Button("<<")) { rewind_(15.f); } ImGui::SameLine();
      if (ImGui::Button("<")) { rewind_(5.f); } ImGui::SameLine();
      if (ImGui::Button(">")) { ff_(5.f); } ImGui::SameLine();
      if (ImGui::Button(">>")) { ff_(15.f); }

      // Reverse mode
      bool reverseMode = reverseMode_;
      IMGUI_ADD_ROW(ImGui::Checkbox, "Reverse Mode", &reverseMode_);
      if (reverseMode != reverseMode_)
      {
        if (reverseMode_)
          ds_.getBoundClock()->playReverse();
        else
          ds_.getBoundClock()->playForward();
      }

      // Time
      float time = time_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Time", &time_, SIM_START, SIM_END, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (time != time_)
        simHandler_->setTime(time_);

      ImGui::TableNextColumn(); ImGui::TableNextColumn();
      if (ImGui::Button("Reset Tether"))
      {
        view_->tetherCamera(nullptr);
        view_->tetherCamera(platformModel_.get());
        view_->setFocalOffsets(45, -45, 2e4);
      }

      if (needUpdate)
        update_();

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  /** Update the track prefs with the current values */
  void update_()
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* platformPrefs = ds_.mutable_platformPrefs(platId_, &xaction);
    simData::TrackPrefs* trackPrefs = platformPrefs->mutable_trackprefs();
    simData::TimeTickPrefs* timeTickPrefs = trackPrefs->mutable_timeticks();

    trackPrefs->set_trackdrawmode(trackMode_);
    trackPrefs->set_linewidth(2.0);
    trackPrefs->set_altmode(altMode_);
    timeTickPrefs->set_drawstyle(drawStyle_);
    timeTickPrefs->set_interval(2.);
    timeTickPrefs->set_linelength(1000.);

    xaction.complete(&platformPrefs);
  }

  /** Rewind the time by the specified amount of seconds */
  void rewind_(double seconds)
  {
    if (ds_.getBoundClock()->timeDirection() == simCore::REVERSE)
      seconds = -seconds;
    simHandler_->setTime(simHandler_->getTime() - seconds);
  }

  /** Fast forward the time by the specified amount of seconds */
  void ff_(double seconds)
  {
    if (ds_.getBoundClock()->timeDirection() == simCore::REVERSE)
      seconds = -seconds;
    simHandler_->setTime(simHandler_->getTime() + seconds);
  }

  simData::MemoryDataStore& ds_;
  simData::ObjectId platId_;
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler_;
  osg::ref_ptr<simVis::View> view_;
  osg::ref_ptr<osg::Node> platformModel_;
  simData::TrackPrefs::Mode trackMode_ = simData::TrackPrefs::Mode::POINT;
  simData::TimeTickPrefs::DrawStyle drawStyle_ = simData::TimeTickPrefs::DrawStyle::POINT;
  float time_ = SIM_START;
  bool altMode_ = false;
  bool reverseMode_ = false;
};

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
    prefs->set_ecidatamode(true);
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
  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // creates a world map.
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(map.get());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // data source which will provide positions for the platform
  // based on the simulation time.

  simData::LinearInterpolator interpolator;
  simData::MemoryDataStore dataStore;
  dataStore.setInterpolator(&interpolator);
  dataStore.enableInterpolation(true);
  dataStore.bindToClock(new simCore::ClockImpl());
  scene->getScenario()->bind(&dataStore);

  // add in the platform and beam
  simData::ObjectId platformId = addPlatform(dataStore);

  // simulator will compute time-based updates for our platform (and any beams it is hosting)
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);

  // create some waypoints (lat, lon, alt, duration)
  sim->addWaypoint(simUtil::Waypoint(51.5, 0.0, 30000, 800.0)); // London
  sim->addWaypoint(simUtil::Waypoint(0.0, 0.0, 30000, 800.0)); // 0 0

  // Install frame update handler that will update track positions over time.
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simMgr = new simUtil::PlatformSimulatorManager(&dataStore);
  simMgr->addSimulator(sim.get());
  simMgr->simulate(SIM_START, SIM_END, SIM_HZ);
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simMgr.get(), SIM_START, SIM_END);
  viewer->addEventHandler(simHandler.get());
  osg::ref_ptr<osg::Node> platformModel = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  // Tether camera to platform
  viewer->getMainView()->tetherCamera(platformModel.get());

#ifdef HAVE_IMGUI
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, platformId, simHandler.get(), viewer->getMainView(), platformModel.get()));
#endif

  // set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(45, -45, 2e6);

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

