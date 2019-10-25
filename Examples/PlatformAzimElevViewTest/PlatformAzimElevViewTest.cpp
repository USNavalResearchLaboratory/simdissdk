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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

/**
 * Tests the Platform Azim/Elev View Tool.
 */

#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/Angle.h"

#include "simData/MemoryDataStore.h"

#include "simVis/PlatformAzimElevViewTool.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#include "osgEarth/Controls"
namespace ui = osgEarth::Util::Controls;

#define LC "[PlatformAzimElevViewTest] "

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<simVis::PlatformAzimElevViewTool> azimElevView;

  simData::MemoryDataStore dataStore;
  osg::ref_ptr<simVis::View>     view;
  osg::ref_ptr<simVis::SceneManager>    scene;
  osg::ref_ptr<simVis::ScenarioManager> scenario;
  simData::ObjectId        platformId;
  osg::ref_ptr<ui::HSliderControl>      rangeSlider;
  osg::ref_ptr<ui::CheckBoxControl>     toggleCheck;
  osg::ref_ptr<ui::HSliderControl>      elevLabelAngle;
  osg::ref_ptr<osg::Uniform>            scaleUniform;

  AppData() { }
};

struct Toggle : public ui::ControlEventHandler
{
  explicit Toggle(AppData& app) : a(app) { }
  AppData& a;
  void onValueChanged(ui::Control* c, bool value)
  {
    if (value)
    {
      a.scenario->addTool(a.azimElevView.get());
      a.view->tetherCamera(a.scenario->find<simVis::PlatformNode>(a.platformId));
      a.view->setFocalOffsets(0.0, -90.0, a.azimElevView->getRange() * 7.0);
      a.view->enableOverheadMode(true);
      a.view->enableOrthographic(true);
    }
    else
    {
      a.scenario->removeTool(a.azimElevView.get());
      a.view->setFocalOffsets(0.0, -35.0, a.azimElevView->getRange() * 7.0);
      a.view->enableOverheadMode(false);
      a.view->enableOrthographic(false);
    }
  }
};

struct SetRange : public ui::ControlEventHandler
{
  explicit SetRange(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, double value)
  {
    a.azimElevView->setRange(value);
  }
};

struct SetElevLabelAngle : public ui::ControlEventHandler
{
  explicit SetElevLabelAngle(AppData& app) : a(app) {}
  AppData& a;
  void onValueChanged(ui::Control* c, double value)
  {
    a.azimElevView->setElevLabelAngle(value);
  }
};

//----------------------------------------------------------------------------

ui::Control* createUI(AppData& app)
{
  ui::VBox* top = new ui::VBox();
  top->setAbsorbEvents(true);
  top->setMargin(ui::Gutter(5.0f));
  top->setBackColor(osg::Vec4(0, 0, 0, 0.5));
  top->addControl(new ui::LabelControl("Platform Azim/Elev View - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  osg::ref_ptr<ui::Grid> grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("ON/OFF:"));
  app.toggleCheck = grid->setControl(c+1, r, new ui::CheckBoxControl(false, new Toggle(app)));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Range:"));
  app.rangeSlider = grid->setControl(c+1, r, new ui::HSliderControl(40000, 225000, 150000, new SetRange(app)));
  grid->setControl(c+2, r, new ui::LabelControl(app.rangeSlider.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("Label Angle:"));
  app.elevLabelAngle = grid->setControl(c+1, r, new ui::HSliderControl(0.0f, osg::PI*2.0, osg::PI_2, new SetElevLabelAngle(app)));
  grid->setControl(c+2, r, new ui::LabelControl(app.elevLabelAngle.get()));

  // force a width.
  app.rangeSlider->setHorizFill(true, 200);

  return top;
}

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const std::string& icon)
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
    prefs->set_icon(icon);
    prefs->set_scale(2.0f);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
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
  xaction.complete(&props);

  simData::BeamPrefs* prefs = dataStore.mutable_beamPrefs(result, &xaction);
  prefs->set_azimuthoffset(simCore::DEG2RAD * az);
  prefs->set_elevationoffset(simCore::DEG2RAD * el);
  prefs->set_verticalwidth(simCore::DEG2RAD * 20.0);
  prefs->set_horizontalwidth(simCore::DEG2RAD * 30.0);
  prefs->set_rendercone(true);
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
  prefs->set_fillpattern(simData::GatePrefs_FillPattern_STIPPLE);
  prefs->set_gateazimuthoffset(simCore::DEG2RAD * az);
  prefs->set_gateelevationoffset(simCore::DEG2RAD * el);
  prefs->set_gaterolloffset(simCore::DEG2RAD * roll);
  xaction.complete(&prefs);

  return result;
}


//----------------------------------------------------------------------------

void simulate(
         simData::ObjectId               hostId,
         std::vector<simData::ObjectId>& targetIds,
         simData::DataStore&             ds,
         simVis::Viewer*                 viewer)
{
  SIM_NOTICE << LC << "Building simulation.... please wait." << std::endl;

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);

  // set up a simple simulation to move the platform.
  {
    osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(hostId);
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

  simman->simulate(0.0, 30.0, 5.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman.get(), 0.0, 30.0, true);
  viewer->addEventHandler(simHandler.get());

  SIM_NOTICE << LC << "...simulation complete." << std::endl;
}

#if 0
//----------------------------------------------------------------------------

void simulatePlatform(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.0, -30.0, 0.0, 1000));
  sim->addWaypoint(simUtil::Waypoint(0.0, -35.0, 0.0, 1000));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim);
  simman->simulate(0.0, 60.0, 30.0);

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler = new simUtil::SimulatorEventHandler(simman, 0.0, 60.0);
  viewer->addEventHandler(simHandler);
}
#endif

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int numBeams = 20;
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
  app.view     = viewer->getMainView();
  app.scene    = viewer->getSceneManager();
  app.scenario = app.scene->getScenario();
  app.scenario->bind(&app.dataStore);

  // place a platform and put it in motion
  app.platformId = addPlatform(app.dataStore, EXAMPLE_SHIP_ICON);

  // place some random beams.
  ::srand(time(NULL));
  for (int i=0; i<numBeams; ++i)
  {
    double az, el, roll;

    // randomize some values and add a beam:
    az = -180.0 + double(::rand() % 360);
    el = double(::rand() % 70);
    simData::ObjectId beamId = addBeam(app.platformId, app.dataStore, az, el);

    // add a randomized gate offset
    az += -10.0 + double(::rand() % 20);
    el += -10.0 + double(::rand() % 20);
    roll = -22.5 + double(::rand()% 45);
    addGate(beamId, app.dataStore, az, el, roll);
  }

  // make some targets flying around.
  std::vector<simData::ObjectId> targetIds;
  for (int i = 0; i < numTargets; ++i)
  {
    simData::ObjectId targetId = addPlatform(app.dataStore, EXAMPLE_AIRPLANE_ICON);
    targetIds.push_back(targetId);
  }

  simulate(app.platformId, targetIds, app.dataStore, viewer.get());
  app.dataStore.update(0);

  // the planetarium view:
  osg::observer_ptr<simVis::EntityNode> platform = app.scenario->find(app.platformId);
  app.azimElevView = new simVis::PlatformAzimElevViewTool(platform.get());
  app.azimElevView->setRange(75000);

  // set up the controls
  osg::observer_ptr<simVis::View> view = viewer->getMainView();
  view->addOverlayControl(createUI(app));
  view->setLighting(false);

  // zoom the camera
  view->tetherCamera(platform.get());
  view->setFocalOffsets(0, -45, 250000);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
