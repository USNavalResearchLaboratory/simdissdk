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

/**
 * Angle Test Example
 *
 * Unit test verifying that Earth Centered Earth Fixed (ECEF) angle composition
 * is rendered correctly in OSG.
 */
/// the simulator provides time/space data for our platform
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/ScenarioDataStoreAdapter.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

#include "osgEarth/StringUtils"
#include "osgEarth/Style"
#include "osgEarth/LatLongFormatter"
#include "osgEarth/MGRSFormatter"

#ifdef HAVE_IMGUI
#include "BaseGui.h"
#include "OsgImGuiHandler.h"
#else
#include "osgEarth/Controls"
#endif

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

/** Test program for update angle computations. */

static simData::DataStore* s_dataStore;

static simData::ObjectId s_id;

static double s_time = 0.0;

#ifdef HAVE_IMGUI

// ImGui has this annoying habit of putting text associated with GUI elements like sliders and check boxes on
// the right side of the GUI elements instead of on the left. Helper macro puts a label on the left instead,
// while adding a row to a two column table started using ImGui::BeginTable(), which emulates a QFormLayout.
// Forces a width of 150 on the sliders. Otherwise, the sliders claim no horizontal space and are unusable.
#define IMGUI_ADD_ROW(func, label, ...) ImGui::TableNextColumn(); ImGui::Text(label); ImGui::TableNextColumn(); ImGui::SetNextItemWidth(150); func("##" label, __VA_ARGS__)

class ControlPanel : public GUI::BaseGui
{
public:
  ControlPanel(simData::MemoryDataStore& ds, simData::ObjectId id)
    : GUI::BaseGui("Angle Test Example"),
    ds_(ds),
    id_(id),
    yawDeg_(0.f),
    pitchDeg_(0.f),
    rollDeg_(0.f),
    latDeg_(0.f),
    lonDeg_(0.f),
    time_(0.f)
  {
    update_();
  }

  void draw(osg::RenderInfo& ri) override
  {
    ImGui::SetNextWindowPos(ImVec2(15, 15));
    ImGui::SetNextWindowBgAlpha(.6f);
    ImGui::Begin(name(), 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    bool needUpdate = false;

    if (ImGui::BeginTable("Table", 2))
    {
      // Yaw
      float yaw = yawDeg_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Yaw", &yawDeg_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (yaw != yawDeg_)
        needUpdate = true;

      // Pitch
      float pitch = pitchDeg_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Pitch", &pitchDeg_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (pitch != pitchDeg_)
        needUpdate = true;

      // Roll
      float roll = rollDeg_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Roll", &rollDeg_, -90.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (roll != rollDeg_)
        needUpdate = true;

      // Latitude
      float lat = latDeg_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Lat", &latDeg_, -89.f, 89.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (lat != latDeg_)
        needUpdate = true;

      // Longitude
      float lon = lonDeg_;
      IMGUI_ADD_ROW(ImGui::SliderFloat, "Lon", &lonDeg_, -180.f, 180.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
      if (lon != lonDeg_)
        needUpdate = true;

      ImGui::EndTable();
    }

    if (needUpdate)
      update_();

    ImGui::End();
  }

private:
  /** Send a platform update using the current values */
  void update_()
  {
    time_ += 1.0;

    simCore::Coordinate lla(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * latDeg_, simCore::DEG2RAD * lonDeg_, 10000.0),
      simCore::Vec3(simCore::DEG2RAD * yawDeg_, simCore::DEG2RAD * pitchDeg_, simCore::DEG2RAD * rollDeg_));
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);

    simData::DataStore::Transaction t;
    simData::PlatformUpdate* u = ds_.addPlatformUpdate(id_, &t);
    if (u)
    {
      u->set_time(time_);
      u->set_x(ecef.x());
      u->set_y(ecef.y());
      u->set_z(ecef.z());
      u->set_psi(ecef.psi());
      u->set_theta(ecef.theta());
      u->set_phi(ecef.phi());
      t.complete(&u);
    }

    ds_.update(time_);
  }

  simData::MemoryDataStore& ds_;
  simData::ObjectId id_;
  float yawDeg_;
  float pitchDeg_;
  float rollDeg_;
  float latDeg_;
  float lonDeg_;
  float time_;
};

#else
/// keep a handle, for toggling
static osg::ref_ptr<Control> s_helpControl;
static HSliderControl* yaw, * pitch, * roll, * lat, * lon;

struct SetUpdate : public ControlEventHandler
{
  void onValueChanged(Control*, float value)
  {
    s_time += 1.0;

    simCore::Coordinate lla(
      simCore::COORD_SYS_LLA,
      simCore::Vec3(simCore::DEG2RAD * lat->getValue(), simCore::DEG2RAD * lon->getValue(), 10000.0),
      simCore::Vec3(simCore::DEG2RAD * yaw->getValue(), pitch->getValue() * simCore::DEG2RAD, roll->getValue() * simCore::DEG2RAD));
    simCore::Coordinate ecef;
    simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecef);

    simData::DataStore::Transaction t;
    simData::PlatformUpdate* u = s_dataStore->addPlatformUpdate(s_id, &t);
    if (u)
    {
      u->set_time(s_time);
      u->set_x(ecef.x());
      u->set_y(ecef.y());
      u->set_z(ecef.z());
      u->set_psi(ecef.psi());
      u->set_theta(ecef.theta());
      u->set_phi(ecef.phi());
      t.complete(&u);
    }

    s_dataStore->update(s_time);
  }
};

static Control* createHelp()
{
  osg::ref_ptr<Grid> g = new Grid();
  g->setChildSpacing(5);

  g->setControl(0, 0, new LabelControl("Yaw:"));
  yaw = g->setControl(1, 0, new HSliderControl(-180, 180, 0.0));
  yaw->setSize(300, 35);
  yaw->addEventHandler(new SetUpdate());

  g->setControl(0, 1, new LabelControl("Pitch:"));
  pitch = g->setControl(1, 1, new HSliderControl(-90, 90, 0.0));
  pitch->setSize(300, 35);
  pitch->addEventHandler(new SetUpdate());

  g->setControl(0, 2, new LabelControl("Roll:"));
  roll = g->setControl(1, 2, new HSliderControl(-90, 90, 0.0));
  roll->setSize(300, 35);
  roll->addEventHandler(new SetUpdate());

  g->setControl(0, 3, new LabelControl("Lat:"));
  lat = g->setControl(1, 3, new HSliderControl(-89, 89, 0.0));
  lat->setSize(300, 35);
  lat->addEventHandler(new SetUpdate());

  g->setControl(0, 4, new LabelControl("Long:"));
  lon = g->setControl(1, 4, new HSliderControl(-180, 180, 0.0));
  lon->setSize(300, 35);
  lon->addEventHandler(new SetUpdate());

  s_helpControl = g.get();
  return g.release();
}

#endif

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
  simCore::checkVersionThrow();

  /// set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  /// creates a world map.
  osg::ref_ptr<Map> map = simExamples::createDefaultExampleMap();

  /// Simdis viewer to display the scene
  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  viewer->setMap(map.get());

  /// data source which will provide positions for the platform
  /// based on the simulation time.
  simData::MemoryDataStore dataStore;
  s_dataStore = &dataStore;

  /// bind dataStore to the scenario manager
  // TODO (perhaps this should be automatic, or a scene->getScenario()->bind(dataStore) instead -gw)
  //simVis::ScenarioDataStoreAdapter adapter(&dataStore, scene->getScenario());

  osg::ref_ptr<simVis::ScenarioManager> scenario = viewer->getSceneManager()->getScenario();

  scenario->bind(&dataStore);

  {
    simData::DataStore::Transaction transaction;
    simData::PlatformProperties* newProps = s_dataStore->addPlatform(&transaction);
    s_id = newProps->id();
    transaction.complete(&newProps);
  }

  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = s_dataStore->mutable_platformPrefs(s_id, &xaction);
    prefs->mutable_commonprefs()->set_name("Simulated Platform");
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_scale(20.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  /// Tether camera to platform
  osg::observer_ptr<simVis::PlatformNode> platformNode = scenario->find<simVis::PlatformNode>(s_id);
  viewer->getMainView()->tetherCamera(platformNode.get());

  /// set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(0, -45, 4e5);

#ifdef HAVE_IMGUI
  // Pass in existing realize operation as parent op, parent op will be called first
  viewer->getViewer()->setRealizeOperation(new GUI::OsgImGuiHandler::RealizeOperation(viewer->getViewer()->getRealizeOperation()));
  GUI::OsgImGuiHandler* gui = new GUI::OsgImGuiHandler();
  viewer->getMainView()->getEventHandlers().push_front(gui);
  gui->add(new ControlPanel(dataStore, s_id));
#else
  /// show the instructions overlay
  viewer->getMainView()->addOverlayControl(createHelp());

  /// Prime it
  SetUpdate().onValueChanged(nullptr, 0.0);
#endif

  /// add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

