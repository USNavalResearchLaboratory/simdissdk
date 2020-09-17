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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
/**
 * ECI Track History TEST
 * Test app for the various features of the ECI Track History feature.
 */
#include <osgEarth/Controls>
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

namespace ui = osgEarth::Util::Controls;

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

struct AppData
{
  osg::ref_ptr<ui::HSliderControl>  modeSlider_;
  osg::ref_ptr<ui::HSliderControl>  timeTicksSlider_;

  osg::ref_ptr<ui::LabelControl>   modeLabel_;
  osg::ref_ptr<ui::LabelControl>   sizeLabel_;
  osg::ref_ptr<ui::LabelControl>   timeTicksLabel_;

  osg::ref_ptr<ui::ButtonControl>  rewind1_;
  osg::ref_ptr<ui::ButtonControl>  rewind2_;
  osg::ref_ptr<ui::ButtonControl>  ff1_;
  osg::ref_ptr<ui::ButtonControl>  ff2_;
  osg::ref_ptr<ui::HSliderControl> timeSlider_;
  osg::ref_ptr<ui::ButtonControl>  tether_;

  osg::ref_ptr<ui::CheckBoxControl> altModeCheck_;
  osg::ref_ptr<ui::CheckBoxControl> reverseModeCheck_;

  std::vector< std::pair<simData::TrackPrefs_Mode, std::string> > modes_;
  std::vector< std::pair<simData::TimeTickPrefs_DrawStyle, std::string> > timeTickModes_;
  simData::DataStore*  ds_;
  simData::ObjectId    hostId_;
  osg::ref_ptr<simVis::View> view_;
  osg::ref_ptr<osg::Node>    platformModel_;

  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler_;

  AppData(simData::DataStore* ds, simData::ObjectId hostId)
   : modeSlider_(nullptr),
    timeTicksSlider_(nullptr),
    modeLabel_(nullptr),
    sizeLabel_(nullptr),
    timeTicksLabel_(nullptr),
    rewind1_(nullptr),
    rewind2_(nullptr),
    ff1_(nullptr),
    ff2_(nullptr),
    timeSlider_(nullptr),
    tether_(nullptr),
    ds_(ds),
    hostId_(hostId),
    view_(nullptr),
    platformModel_(nullptr)
  {
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_OFF,    "OFF"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_POINT,  "POINT"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_LINE,   "LINE"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_RIBBON, "RIBBON"));
    modes_.push_back(std::make_pair(simData::TrackPrefs_Mode_BRIDGE, "BRIDGE"));

    timeTickModes_.push_back(std::make_pair(simData::TimeTickPrefs_DrawStyle_NONE,   "OFF"));
    timeTickModes_.push_back(std::make_pair(simData::TimeTickPrefs_DrawStyle_POINT,  "POINT"));
    timeTickModes_.push_back(std::make_pair(simData::TimeTickPrefs_DrawStyle_LINE,   "LINE"));
  }

  void apply()
  {
    int modeIndex  = simCore::sdkMax(0, (int)floor(modeSlider_->getValue()));
    int timeTicksIndex  = simCore::sdkMax(0, (int)floor(timeTicksSlider_->getValue()));

    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* platformPrefs = ds_->mutable_platformPrefs(hostId_, &xaction);
    simData::TrackPrefs*    trackPrefs    = platformPrefs->mutable_trackprefs();
    simData::TimeTickPrefs* timeTickPrefs    = trackPrefs->mutable_timeticks();

    trackPrefs->set_trackdrawmode(modes_[modeIndex].first);
    trackPrefs->set_linewidth(2.0);
    trackPrefs->set_altmode(altModeCheck_->getValue());
    timeTickPrefs->set_drawstyle(timeTickModes_[timeTicksIndex].first);
    timeTickPrefs->set_interval(2.);
    timeTickPrefs->set_linelength(1000.);

    xaction.complete(&platformPrefs);

    // time direction:
    if (reverseModeCheck_->getValue() == true)
      ds_->getBoundClock()->playReverse();
    else
      ds_->getBoundClock()->playForward();

    // update labels.
    modeLabel_->setText(modes_[modeIndex].second);
    timeTicksLabel_->setText(timeTickModes_[timeTicksIndex].second);
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
};

//----------------------------------------------------------------------------

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
  top->addControl(new ui::LabelControl("ECI Track History - Test App", 22.0f, simVis::Color::Yellow));

  int c=0, r=0;
  osg::ref_ptr<ui::Grid> grid = top->addControl(new ui::Grid());
  grid->setChildSpacing(5.0f);

  grid->setControl(c, r, new ui::LabelControl("Mode"));
  app.modeSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.modes_.size(), 1, applyUI.get()));
  app.modeLabel_  = grid->setControl(c+2, r, new ui::LabelControl());
  app.modeSlider_->setHorizFill(true, 250);

  r++;
  grid->setControl(c, r, new ui::LabelControl("Alt mode"));
  app.altModeCheck_ = grid->setControl(c+1, r, new ui::CheckBoxControl(false, applyUI.get()));

  r++;
  grid->setControl(c, r, new ui::LabelControl("TimeTicks"));
  app.timeTicksSlider_ = grid->setControl(c+1, r, new ui::HSliderControl(0, app.timeTickModes_.size(), 1, applyUI.get()));
  app.timeTicksLabel_  = grid->setControl(c+2, r, new ui::LabelControl());
  app.timeTicksSlider_->setHorizFill(true, 250);

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

  return top;
}

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

  // Attach the simulation updater to OSG timer events
  AppData app(&dataStore, platformId);
  app.simHandler_ = new simUtil::SimulatorEventHandler(simMgr.get(), SIM_START, SIM_END);
  viewer->addEventHandler(app.simHandler_.get());

  // Tether camera to platform
  app.view_          = viewer->getMainView();
  app.platformModel_ = scene->getScenario()->find<simVis::PlatformNode>(platformId);
  app.view_->tetherCamera(app.platformModel_.get());

  // set the camera to look at the platform
  viewer->getMainView()->setFocalOffsets(45, -45, 2e6);

  // show the instructions overlay
  viewer->getMainView()->addOverlayControl(createUI(app));
  app.apply();

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

