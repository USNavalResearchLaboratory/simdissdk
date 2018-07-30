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
#include "osgEarthUtil/Sky"

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static const std::string s_title = "Antenna Pattern Example";


struct AppData
{
  AppData()
   : platformId(0),
     beamId(0),
     algorithm(NULL),
     algorithmLabel(NULL),
     polarity(NULL),
     polarityLabel(NULL),
     sensitivity(NULL),
     frequency(NULL),
     gain(NULL),
     fov(NULL),
     detail(NULL),
     power(NULL),
     width(NULL),
     height(NULL),
     scale(NULL),
     weighting(NULL),
     colorscale(NULL),
     blending(NULL),
     lighting(NULL)
  {
    algs.push_back(std::make_pair(simData::BeamPrefs_AntennaPattern_Algorithm_PEDESTAL, "PEDESTAL"));
    algs.push_back(std::make_pair(simData::BeamPrefs_AntennaPattern_Algorithm_GAUSS,    "GAUSS"));
    algs.push_back(std::make_pair(simData::BeamPrefs_AntennaPattern_Algorithm_CSCSQ,    "CSCSQ"));
    algs.push_back(std::make_pair(simData::BeamPrefs_AntennaPattern_Algorithm_SINXX,    "SINXX"));
    algs.push_back(std::make_pair(simData::BeamPrefs_AntennaPattern_Algorithm_OMNI,     "OMNI"));
  }

  std::vector< std::pair<simData::BeamPrefs_AntennaPattern_Algorithm, std::string> > algs;

  simData::MemoryDataStore ds;
  simData::ObjectId        platformId;
  simData::ObjectId        beamId;

  osg::ref_ptr<HSliderControl>   algorithm;
  osg::ref_ptr<LabelControl>     algorithmLabel;
  osg::ref_ptr<HSliderControl>   polarity;
  osg::ref_ptr<LabelControl>     polarityLabel;
  osg::ref_ptr<HSliderControl>   sensitivity;
  osg::ref_ptr<HSliderControl>   frequency;
  osg::ref_ptr<HSliderControl>   gain;
  osg::ref_ptr<HSliderControl>   fov;
  osg::ref_ptr<HSliderControl>   detail;
  osg::ref_ptr<HSliderControl>   power;
  osg::ref_ptr<HSliderControl>   width;
  osg::ref_ptr<HSliderControl>   height;
  osg::ref_ptr<HSliderControl>   scale;

  osg::ref_ptr<CheckBoxControl>  weighting;
  osg::ref_ptr<CheckBoxControl>  colorscale;
  osg::ref_ptr<CheckBoxControl>  blending;
  osg::ref_ptr<CheckBoxControl>  lighting;
};


void applyAntennaPrefs(AppData* app)
{
  int algorithmIndex = simCore::sdkMax(0, (int)floor(app->algorithm->getValue()));
  int polarityIndex  = simCore::sdkMax(0, (int)floor(app->polarity->getValue()));

  simData::DataStore::Transaction xaction;
  simData::BeamPrefs* prefs = app->ds.mutable_beamPrefs(app->beamId, &xaction);
  prefs->set_drawtype(simData::BeamPrefs_DrawType_ANTENNA_PATTERN);
  prefs->mutable_antennapattern()->set_type(simData::BeamPrefs_AntennaPattern_Type_ALGORITHM);
  prefs->mutable_antennapattern()->set_algorithm(app->algs[algorithmIndex].first);
  prefs->mutable_antennapattern()->set_filename(app->algs[algorithmIndex].second);
  prefs->set_polarity((simData::Polarity)polarityIndex);
  prefs->set_sensitivity(app->sensitivity->getValue());
  prefs->set_fieldofview(simCore::DEG2RAD * app->fov->getValue());
  prefs->set_horizontalwidth(simCore::DEG2RAD * app->width->getValue());
  prefs->set_verticalwidth(simCore::DEG2RAD * app->height->getValue());
  prefs->set_gain(app->gain->getValue());
  prefs->set_detail(app->detail->getValue());
  prefs->set_power(app->power->getValue());
  prefs->set_frequency(app->frequency->getValue());
  prefs->set_weighting(app->weighting->getValue());
  prefs->set_colorscale(app->colorscale->getValue());
  prefs->set_beamscale(app->scale->getValue());
  prefs->set_blended(app->blending->getValue());
  prefs->set_shaded(app->lighting->getValue());
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->set_datadraw(true);
  xaction.complete(&prefs);

  app->algorithmLabel->setText(app->algs[algorithmIndex].second);

  app->polarityLabel->setText(simCore::polarityString((simCore::PolarityType)polarityIndex));
}


struct ApplyUI : public ControlEventHandler
{
  explicit ApplyUI(AppData* app) : app_(app) { }
  AppData* app_;
  void onValueChanged(Control* c, bool value) { applyAntennaPrefs(app_); }
  void onValueChanged(Control* c, float value) { applyAntennaPrefs(app_); }
  void onValueChanged(Control* c, double value) { onValueChanged(c, (float)value); }
};


Control* createUI(AppData* app)
{
  // vbox is returned to the caller and will be owned by caller
  VBox* vbox = new VBox();
  vbox->setAbsorbEvents(true);
  vbox->setVertAlign(Control::ALIGN_TOP);
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20, simVis::Color::Yellow));

  // sensor parameters
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(app);

  osg::ref_ptr<Grid> g = vbox->addControl(new Grid());
  unsigned row=0, col=0;

  g->setControl(col, row, new LabelControl("Algorithm"));
  app->algorithm = g->setControl(col+1, row, new HSliderControl(0, app->algs.size(), 0, applyUI.get()));
  app->algorithmLabel = g->setControl(col+2, row, new LabelControl());
  app->algorithm->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Polarity"));
  app->polarity = g->setControl(col + 1, row, new HSliderControl(0.0, 9.0, 0.0, applyUI.get()));
  app->polarity->setHorizFill(true, 250.0);
  app->polarityLabel = g->setControl(col+2, row, new LabelControl());

  row++;
  g->setControl(col, row, new LabelControl("Sensitivity"));
  app->sensitivity = g->setControl(col+1, row, new HSliderControl(-100.0, 0.0, -50.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->sensitivity.get()));
  app->sensitivity->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Frequency"));
  app->frequency = g->setControl(col+1, row, new HSliderControl(0.0, 10000.0, 7000.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->frequency.get()));
  app->frequency->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Gain"));
  app->gain = g->setControl(col+1, row, new HSliderControl(0.0, 100.0, 20.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->gain.get()));
  app->gain->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Power"));
  app->power = g->setControl(col+1, row, new HSliderControl(0.0, 20000.0, 2000.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->power.get()));
  app->power->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Beam Width"));
  app->width = g->setControl(col+1, row, new HSliderControl(1.0, 45.0, 3.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->width.get()));
  app->width->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Beam Height"));
  app->height = g->setControl(col+1, row, new HSliderControl(1.0, 45.0, 3.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->height.get()));
  app->height->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Scale"));
  app->scale = g->setControl(col+1, row, new HSliderControl(1.0, 1000.0, 1.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->scale.get()));
  app->scale->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Field of view"));
  app->fov = g->setControl(col+1, row, new HSliderControl(1.0, 360.0, 85.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->fov.get()));
  app->fov->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Detail angle"));
  app->detail = g->setControl(col+1, row, new HSliderControl(1.0, 15.0, 5.0, applyUI.get()));
  g->setControl(col+2, row, new LabelControl(app->detail.get()));
  app->detail->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Weighting"));
  app->weighting = g->setControl(col+1, row, new CheckBoxControl(true, applyUI.get()));

  row++;
  g->setControl(col, row, new LabelControl("Color Scale"));
  app->colorscale = g->setControl(col+1, row, new CheckBoxControl(true, applyUI.get()));

  row++;
  g->setControl(col, row, new LabelControl("Blending"));
  app->blending = g->setControl(col+1, row, new CheckBoxControl(true, applyUI.get()));

  row++;
  g->setControl(col, row, new LabelControl("Lighting"));
  app->lighting = g->setControl(col+1, row, new CheckBoxControl(false, applyUI.get()));

  return vbox;
}

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
void addPlatformAndBeam(AppData* app,
                        double lat, double lon, double alt)
{
  // create the platform:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformProperties* newProps = app->ds.addPlatform(&xaction);
    app->platformId = newProps->id();
    xaction.complete(&newProps);
  }

  // now configure its preferences:
  {
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = app->ds.mutable_platformPrefs(app->platformId, &xaction);
    prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // create the beam:
  {
    simData::DataStore::Transaction xaction;
    simData::BeamProperties* props = app->ds.addBeam(&xaction);
    props->set_hostid(app->platformId);
    props->set_type(simData::BeamProperties_BeamType_ABSOLUTE_POSITION);
    app->beamId = props->id();
    xaction.complete(&props);
  }

  // apply initial settings to the antenna.
  applyAntennaPrefs(app);
}

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
    scene->getSkyNode()->setDateTime(osgEarth::Util::DateTime(2012, 0, 0, 11.0));

  AppData app;

  // install the GUI
  viewer->getMainView()->addOverlayControl(createUI(&app));

  // Set up the data:
  scene->getScenario()->bind(&app.ds);
  addPlatformAndBeam(&app, 0, 0, 10000);

  // make the sim
  simulate(app.platformId, app.ds, viewer.get());

  // zoom the camera
  viewer->getMainView()->tetherCamera(scene->getScenario()->find(app.platformId));
  viewer->getMainView()->setFocalOffsets(0, -45, 250000.);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}

