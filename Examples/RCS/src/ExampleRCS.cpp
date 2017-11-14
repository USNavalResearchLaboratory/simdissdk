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
/* -*-c++-*- */
/**
 * Demonstrates and tests the loading and display of a radar cross section (RCS) on a platform.
 */

/// the simulator provides time/space data for our platform
#include "simNotify/Notify.h"
#include "simUtil/PlatformSimulator.h"
#include "simData/MemoryDataStore.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Calc/CoordinateConverter.h"

/// include definitions for objects of interest
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Beam.h"
#include "simVis/Locator.h"
#include "simVis/Utils.h"

/// some basic components (mouse hover popups, scenario, utilities, camera controls)
#include "simVis/Viewer.h"

/// paths to models
#include "simUtil/ExampleResources.h"

using namespace osgEarth;
using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;

//----------------------------------------------------------------------------

static const std::string s_title = "RCS Example";


struct AppData
{
  simData::MemoryDataStore   ds;
  simData::ObjectId          platformId;

  osg::ref_ptr<CheckBoxControl>  draw2D;
  osg::ref_ptr<CheckBoxControl>  draw3D;
  osg::ref_ptr<HSliderControl>   polarity;
  osg::ref_ptr<HSliderControl>   frequency;
  osg::ref_ptr<HSliderControl>   elevation;
  osg::ref_ptr<HSliderControl>   detail;

  osg::ref_ptr<LabelControl>     polarityLabel;
};


void applyPrefs(AppData* app)
{
  unsigned polarityIndex = (unsigned)floor(app->polarity->getValue());

  simData::DataStore::Transaction xaction;
  simData::PlatformPrefs* prefs = app->ds.mutable_platformPrefs(app->platformId, &xaction);
  {
    prefs->set_drawrcs(app->draw2D->getValue());
    prefs->set_draw3drcs(app->draw3D->getValue());
    prefs->set_rcsdetail(app->detail->getValue());
    prefs->set_rcselevation(app->elevation->getValue());
    prefs->set_rcsfrequency(app->frequency->getValue());
    prefs->set_rcspolarity((simData::Polarity)polarityIndex);
  }
  xaction.complete(&prefs);

  app->polarityLabel->setText(simCore::polarityString((simCore::PolarityType)polarityIndex));
}


struct ApplyUI : public ControlEventHandler
{
  explicit ApplyUI(AppData* app) : app_(app) { }
  AppData* app_;
  void onValueChanged(Control* c, bool value) { applyPrefs(app_); }
  void onValueChanged(Control* c, float value) { applyPrefs(app_); }
  void onValueChanged(Control* c, double value) { onValueChanged(c, (float)value); }
};


Control* createUI(AppData* app)
{
  VBox* vbox = new VBox();
  vbox->setAbsorbEvents(true);
  vbox->setVertAlign(Control::ALIGN_TOP);
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.4);
  vbox->addControl(new LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));

  // sensor parameters
  osg::ref_ptr<ApplyUI> applyUI = new ApplyUI(app);

  osg::ref_ptr<Grid> g = vbox->addControl(new Grid());
  unsigned row=0, col=0;

  row++;
  app->draw2D = g->setControl(col, row, new CheckBoxControl(true, applyUI));
  g->setControl(col+1, row, new LabelControl("Draw 2D RCS"));

  row++;
  app->draw3D = g->setControl(col, row, new CheckBoxControl(true, applyUI));
  g->setControl(col+1, row, new LabelControl("Draw 3D RCS"));

  row++;
  g->setControl(col, row, new LabelControl("Polarity"));
  app->polarity = g->setControl(col+1, row, new HSliderControl(0.0, 9.0, 0.0, applyUI));
  app->polarity->setHorizFill(true, 250.0);
  app->polarityLabel = g->setControl(col+2, row, new LabelControl());

  row++;
  g->setControl(col, row, new LabelControl("Frequency"));
  app->frequency = g->setControl(col+1, row, new HSliderControl(0.0, 10000.0, 7000.0, applyUI));
  g->setControl(col+2, row, new LabelControl(app->frequency));
  app->frequency->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Elevation"));
  app->elevation = g->setControl(col+1, row, new HSliderControl(0.0, 90.0, 45.0, applyUI));
  g->setControl(col+2, row, new LabelControl(app->elevation));
  app->elevation->setHorizFill(true, 250.0);

  row++;
  g->setControl(col, row, new LabelControl("Detail angle"));
  app->detail = g->setControl(col+1, row, new HSliderControl(1.0, 15.0, 5.0, applyUI));
  g->setControl(col+2, row, new LabelControl(app->detail));
  app->detail->setHorizFill(true, 250.0);

  return vbox;
}

//----------------------------------------------------------------------------

// create a platform and add it to 'dataStore'
void addPlatform(AppData* app)
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
    prefs->set_rcsfile(EXAMPLE_RCS_FILE);
    prefs->set_dynamicscale(true);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    xaction.complete(&prefs);
  }

  // apply the initial configuration:
  applyPrefs(app);
}

//----------------------------------------------------------------------------

void simulate(simData::ObjectId id, simData::DataStore& ds, simVis::Viewer* viewer)
{
  // set up a simple simulation to move the platform.
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);

  sim->addWaypoint(simUtil::Waypoint(0.5, -0.5, 20000, 30.0));
  sim->addWaypoint(simUtil::Waypoint(0.5,  0.5, 20000, 30.0));

  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&ds);
  simman->addSimulator(sim);
  simman->simulate(0.0, 30.0, 30.0);

  osg::ref_ptr<simVis::SimulatorEventHandler> simHandler = new simVis::SimulatorEventHandler(simman, 0.0, 30.0);
  viewer->addEventHandler(simHandler);
}

//----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();
  viewer->setMap(simExamples::createDefaultExampleMap());
  viewer->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  AppData app;

  // install the GUI
  viewer->getMainView()->addOverlayControl(createUI(&app));

  // Create the platform:
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  scene->getScenario()->bind(&app.ds);
  addPlatform(&app);

  // make the sim
  simulate(app.platformId, app.ds, viewer);

  // zoom the camera
  viewer->getMainView()->tetherCamera(scene->getScenario()->find(app.platformId));
  viewer->getMainView()->setFocalOffsets(0, -45, 800.);

  // add some stock OSG handlers and go
  viewer->installDebugHandlers();
  return viewer->run();
}
