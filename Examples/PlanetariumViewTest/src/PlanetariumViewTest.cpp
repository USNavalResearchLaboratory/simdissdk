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
#include "simVis/Locator.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"

#include "osgEarthUtil/Controls"
namespace ui = osgEarth::Util::Controls;

#define LC "[Planetarium Test] "

//----------------------------------------------------------------------------

struct AppData
{
  osg::ref_ptr<simVis::PlanetariumViewTool> planetarium;

  simData::MemoryDataStore dataStore;
  osg::ref_ptr<simVis::SceneManager>    scene;
  osg::ref_ptr<simVis::ScenarioManager> scenario;
  simData::ObjectId        platformId;

  osg::ref_ptr<ui::CheckBoxControl>     toggleCheck;
  osg::ref_ptr<ui::CheckBoxControl>     vectorCheck;
  osg::ref_ptr<ui::HSliderControl>      rangeSlider;
  osg::ref_ptr<ui::LabelControl>        rangeLabel;
  osg::ref_ptr<ui::HSliderControl>      colorSlider;
  osg::ref_ptr<ui::LabelControl>        colorLabel;

  std::vector< std::pair<simVis::Color, std::string> > colors;
  int colorIndex;

  AppData()
  {
    colors.push_back(std::make_pair(simVis::Color(0xffffff3f), "White"));
    colors.push_back(std::make_pair(simVis::Color(0x00ff003f), "Green"));
    colors.push_back(std::make_pair(simVis::Color(0xff7f003f), "Orange"));
    colors.push_back(std::make_pair(simVis::Color(0xffffff00), "Invisible"));
    colors.push_back(std::make_pair(simVis::Color(0xffff003f), "Yellow"));
    colorIndex = colors.size()-1;
  }
};

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
  top->addControl(new ui::LabelControl("PlanetariumViewTool - Test App", 22.0f, osg::Vec4(1, 1, 0, 1)));

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

  // force a width.
  app.rangeSlider->setHorizFill(true, 200);

  return top;
}

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
simData::ObjectId addPlatform(simData::DataStore& dataStore, const char* iconFile)
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

void simulate(simData::ObjectId hostId, std::vector<simData::ObjectId>& targetIds, simData::DataStore& ds, simVis::Viewer* viewer)
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

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int numBeams   = 10;
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
    roll = -5.0 + double(::rand() % 10);
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
  osg::observer_ptr<simVis::PlatformNode> platform = app.scenario->find<simVis::PlatformNode>(app.platformId);
  app.planetarium = new simVis::PlanetariumViewTool(platform.get());
  app.planetarium->setRange(75000);

  // set up the controls
  osg::observer_ptr<simVis::View> view = viewer->getMainView();
  view->addOverlayControl(createUI(app));
  view->setLighting(false);

  // zoom the camera
  view->tetherCamera(platform.get());
  view->setFocalOffsets(0, -45, 350000);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

