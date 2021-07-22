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
#include <QDialog>
#include <osgGA/GUIEventHandler>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Time/ClockImpl.h"
#include "simData/LinearInterpolator.h"
#include "simData/MemoryDataStore.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simQt/TimeButtons.h"
#include "simQt/ViewWidget.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/PlatformSimulator.h"
#include "MainWindow.h"

class DataStoreTimeUpdate : public simCore::Clock::TimeObserver
{
public:
  explicit DataStoreTimeUpdate(simData::DataStore &dataStore)
  : dataStore_(dataStore)
  {
  }

  /// time has been changed
  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    dataStore_.update(t.secondsSinceRefYear());
  }

  /// time has looped
  virtual void onTimeLoop()
  {
  }

  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  {
  }

protected:
  simData::DataStore &dataStore_;
};

MainWindow::MainWindow()
{
  // create a world map
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();

  // anchor point for the scene graph
  osg::ref_ptr<simVis::SceneManager> sceneMan = new simVis::SceneManager();
  sceneMan->setMap(map.get());

  // add sky node
  simExamples::addDefaultSkyNode(sceneMan.get());

  // view of the world
  osg::ref_ptr<simVis::View> view = new simVis::View();
  view->setSceneManager(sceneMan.get());
  view->setNavigationMode(simVis::NAVMODE_ROTATEPAN);

  viewMan_ = new simVis::ViewManager(); // Note that the log depth buffer is not installed
  viewMan_->addView(view.get());

  // data source which will provide positions for the platform
  // based on the simulation time.
  dataStore_ = new simData::MemoryDataStore;
  dataStore_->setInterpolator(new simData::LinearInterpolator());
  dataStore_->enableInterpolation(true);
  sceneMan->getScenario()->bind(dataStore_);

  // wrap the view in a Qt widget
  simQt::ViewWidget* viewWidget = new simQt::ViewWidget(view.get());
  setCentralWidget(viewWidget);

  // clock will manage simulation time
  clock_ = new simCore::ClockImpl;
  dataStore_->bindToClock(clock_);
  clock_->registerTimeCallback(simCore::Clock::TimeObserverPtr(new DataStoreTimeUpdate(*dataStore_)));

  // add platform data to the data store
  setupSimulatedPlatform_();

  // create buttons to control time
  QDialog *buttonDialog = new QDialog;
  simQt::TimeButtons *timeButtons = new simQt::TimeButtons(buttonDialog);
  simQt::ButtonActions *timeButtonActions = new simQt::ButtonActions(buttonDialog);
  timeButtonActions->setClockManager(clock_);
  timeButtons->bindToActions(timeButtonActions);
  buttonDialog->show();

  // timer to drive updates
  connect(&updateTimer_, SIGNAL(timeout()), this, SLOT(notifyFrameUpdate_()));
  updateTimer_.start(33); // 33 ms -> 30 Hz
}

void MainWindow::notifyFrameUpdate_()
{
  // let the clock update time (if playing)
  clock_->idle();

  // update the data store with the new time
  dataStore_->update(dataStore_->updateTime());

  // refresh the view
  viewMan_->frame();
}

simData::ObjectId MainWindow::addPlatform_(simData::DataStore &dataStore)
{
  // all DataStore operations require a transaction (to avoid races)
  simData::DataStore::Transaction transaction;

  // create the platform, and get the properties for it
  simData::PlatformProperties *newProps = dataStore.addPlatform(&transaction);

  // save the platform id for our return value
  simData::ObjectId result = newProps->id();

  // done
  transaction.complete(&newProps);


  // configure some basic prefs
  simData::PlatformPrefs *prefs = dataStore.mutable_platformPrefs(result, &transaction);
  prefs->mutable_commonprefs()->set_name("Demo Platform");
  prefs->set_icon(EXAMPLE_AIRPLANE_ICON);
  prefs->set_scale(3.0f);
  prefs->set_dynamicscale(true);
  prefs->mutable_commonprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  prefs->mutable_commonprefs()->mutable_labelprefs()->set_overlayfontpointsize(14);
  prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_LINE);
  transaction.complete(&prefs);

  return result;
}

void MainWindow::populateDataStore_(simData::DataStore &dataStore, simUtil::PlatformSimulator &sim, double endTimeS, double dataRateHz)
{
  simData::DataStore::Transaction transaction;

  for (double t = 0; t < endTimeS; t += 1/dataRateHz)
  {
    simData::PlatformUpdate *u = dataStore_->addPlatformUpdate(sim.getPlatformId(), &transaction);
    sim.updatePlatform(t, u);

    transaction.complete(&u);
  }
}

void MainWindow::setupSimulatedPlatform_()
{
  simData::ObjectId platId = addPlatform_(*dataStore_);

  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platId);
  sim->addWaypoint(simUtil::Waypoint(51.5,   0.0, 30000, 200.0)); // London
  sim->addWaypoint(simUtil::Waypoint(38.8, -77.0, 30000, 200.0)); // DC
  sim->addWaypoint(simUtil::Waypoint(-33.4, -70.8, 30000, 200.0)); // Santiago
  sim->addWaypoint(simUtil::Waypoint(-34.0,  18.5, 30000, 200.0)); // Capetown
  sim->setSimulateRoll(true);
  sim->setSimulatePitch(false);

  populateDataStore_(*dataStore_, *sim, 800, 10);

  clock_->setEndTime(simCore::TimeStamp(1970, 800.));
}

