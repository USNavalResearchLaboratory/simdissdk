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
 * Massive Data Example
 *
 * Simulates a user-defined number of entities around the world, used as a stress test for performance.
 */

#include "osgGA/StateSetManipulator"
#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"

#include "osgEarth/Controls"

#include "simNotify/Notify.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"

#include "simData/DataSlice.h"
#include "simData/MemoryDataStore.h"
#include "simData/LinearInterpolator.h"

#include "simVis/Viewer.h"
#include "simVis/Platform.h"
#include "simVis/LocalGrid.h"
#include "simVis/PlatformModel.h"
#include "simVis/TrackHistory.h"
#include "simVis/Beam.h"
#include "simVis/Gate.h"
#include "simVis/Popup.h"
#include "simVis/Scenario.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"

#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"

namespace ui = osgEarth::Util::Controls;

//----------------------------------------------------------------------------

struct App
{
  osg::ref_ptr<simVis::View>                  view_;
  osg::ref_ptr<simVis::ScenarioManager>       scenario_;
  osg::ref_ptr<simUtil::SimulatorEventHandler> simHandler_;
};

namespace
{
  const std::string LC = "[MassiveData] ";

  double RAND01()
  {
    return static_cast<double>(::rand()) / static_cast<double>(RAND_MAX);
  }

  struct SetLODScale : public ui::ControlEventHandler
  {
    explicit SetLODScale(App& app) : app_(app) {}
    void onValueChanged(ui::Control*, float value)
    {
      app_.scenario_->setLODScaleFactor(value);
    }
    App& app_;
  };

  struct SetTime : public ui::ControlEventHandler
  {
    explicit SetTime(App& app) : app_(app) {}
    void onValueChanged(ui::Control*, float value)
    {
      app_.simHandler_->setTime(value);
    }
    App& app_;
  };

  struct ToggleOverhead : public ui::ControlEventHandler
  {
    explicit ToggleOverhead(App& app) : app_(app) {}
    void onValueChanged(ui::Control*, bool value)
    {
      app_.view_->enableOverheadMode(!app_.view_->isOverheadEnabled());
    }
    App& app_;
  };

#if OSGEARTH_VERSION_LESS_THAN(3,0,0)
  struct ToggleGeoGraph : public ui::ControlEventHandler
  {
    explicit ToggleGeoGraph(App& app) : app_(app) {}
    void onValueChanged(ui::Control*, bool value)
    {
      // Allocate the strategy
      simVis::ScenarioManager::AbstractEntityGraph* graph = NULL;
      if (value)
      {
        simVis::ScenarioDisplayHints hints;
        // 50 is a reasonable value for 10,000 entities
        hints.maxPerCell_ = 50;
        graph = new simVis::ScenarioManager::GeoGraphEntityGraph(hints);
      }
      else
        graph = new simVis::ScenarioManager::SimpleEntityGraph;
      // Change the strategy
      app_.scenario_->setEntityGraphStrategy(graph);
    }
    App& app_;
  };
#endif
}


ui::Control* createUI(App& app, float duration)
{
  ui::Grid* grid = new ui::Grid();
  grid->setVertAlign(ui::Control::ALIGN_BOTTOM);
  grid->setPadding(10);
  grid->setBackColor(0, 0, 0, 0.4);
  int r = 0;
  grid->setControl(0, r, new ui::LabelControl("Massive Data Example", 20, simVis::Color::Yellow));

  ++r;
  grid->setControl(0, r, new ui::LabelControl("LOD scale:"));
  grid->setControl(1, r, new ui::HSliderControl(1.0, 60.0, 1.0, new SetLODScale(app)));
  grid->getControl(1, r)->setHorizFill(true, 300);
  grid->setControl(2, r, new ui::LabelControl(grid->getControl(1, r)));

  ++r;
  grid->setControl(0, r, new ui::LabelControl("Time:"));
  grid->setControl(1, r, new ui::HSliderControl(0.0, duration, 0.0, new SetTime(app)));
  grid->getControl(1, r)->setHorizFill(true, 300);

  ++r;
  grid->setControl(0, r, new ui::LabelControl("Overhead:"));
  grid->setControl(1, r, new ui::CheckBoxControl(false, new ToggleOverhead(app)));

#if OSGEARTH_VERSION_LESS_THAN(3,0,0)
  ++r;
  // Turning on Geo Graph will improve cull for a lot of situations with lots of data
  grid->setControl(0, r, new ui::LabelControl("Geo Graph:"));
  grid->setControl(1, r, new ui::CheckBoxControl(false, new ToggleGeoGraph(app)));
#endif

  return grid;
}

int usage(int argc, char** argv)
{
  SIM_NOTICE << "USAGE: " << argv[0] << "\n"
    "<num-entities> <duration_sec> <hertz> \n"
    "   [--tracks]           : show track history trails\n"
    "   [--labels]           : show platform labels\n"
    "   [--icons]            : use icons instead of models\n"
    "   [--nodynscale]       : disable dynamic scaling\n"
    "   [--model <filename>] : 3D model to use\n";

  return 0;
}

//----------------------------------------------------------------------------

simData::ObjectId addPlatform(simData::DataStore& dataStore)
{
  // create the platform in the database:
  simData::DataStore::Transaction transaction;

  simData::PlatformProperties* newProps = dataStore.addPlatform(&transaction);
  simData::ObjectId result = newProps->id();

  transaction.complete(&newProps);
  return result;
}

void configPlatform(const simData::ObjectId&  id,
                    simData::DataStore&       ds,
                    unsigned                  number,
                    int                       argc,
                    char**                    argv)
{
  bool tracks   = simExamples::hasArg("--tracks",   argc, argv);
  bool labels   = simExamples::hasArg("--labels",   argc, argv);
  bool icons    = simExamples::hasArg("--icons",    argc, argv);
  bool nodynscale = simExamples::hasArg("--nodynscale", argc, argv);

  std::string iconFile;
  simExamples::readArg("--model", argc, argv, iconFile);

  simData::DataStore::Transaction xaction;
  simData::PlatformPrefs* prefs = ds.mutable_platformPrefs(id, &xaction);

  std::stringstream buf;
  buf << "P" << number;
  prefs->mutable_commonprefs()->set_name(buf.str());

  if (labels)
  {
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
  }

  if (tracks)
  {
    prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_BRIDGE);
    prefs->mutable_trackprefs()->set_linewidth(1);
  }
  else
  {
    prefs->mutable_trackprefs()->set_trackdrawmode(simData::TrackPrefs_Mode_OFF);
  }

  // Set the icon to either a 2D image or a 3D shape
  if (icons)
    prefs->set_icon(EXAMPLE_IMAGE_ICON);
  else
    prefs->set_icon(!iconFile.empty() ? iconFile : EXAMPLE_AIRPLANE_ICON);

  // Dynamic scale is on by default
  if (!nodynscale)
    prefs->set_dynamicscale(true);

  prefs->set_rotateicons(simData::IR_2D_YAW);
  xaction.complete(&prefs);
}

void simulatePlatform(const simData::ObjectId& id, simUtil::PlatformSimulatorManager* simman)
{
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(id);
  for (unsigned i = 0; i < 2; ++i)
  {
    const double lat = -80.0 + 160.0 * RAND01();
    const double lon = -180 + 360.0 * RAND01();
    const double alt = 15000.0 + 300000.0 * RAND01();
    sim->addWaypoint(simUtil::Waypoint(lat, lon, alt, 200.0));
  }

  simman->addSimulator(sim.get());
}

//----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  simCore::checkVersionThrow();

  unsigned numPlatforms;
  double   duration;
  double   hertz;

  if (simExamples::hasArg("--help", argc, argv))
  {
    return usage(argc, argv);
  }
  else
  {
    if (argc < 2 || !simCore::isValidNumber(argv[1], numPlatforms) || numPlatforms < 1)
      numPlatforms = 1000;

    if (argc < 3 || !simCore::isValidNumber(argv[2], duration) || duration < 1.0)
      duration = 30.0;

    if (argc < 4 || !simCore::isValidNumber(argv[3], hertz) || hertz < 1.0)
      hertz = 10.0;
  }

  SIM_NOTICE << LC << "Simulating " << numPlatforms << " platforms for "
    << duration << "s. at " << hertz << "hz." << std::endl;

  // set up the registry so the SDK can find platform models
  simExamples::configureSearchPaths();

  // world map
#ifdef USE_REMOTE_DATA
  // Retrieve imagery/terrain from Amazon EC2 server
  osg::ref_ptr<osgEarth::Map> map = simExamples::createWorldMapWithFlatOcean();
#else
  // Load imagery and terrain from standard SIMDIS DB files
  osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
#endif

  // simdis viewer to display the scene
  App app;

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  app.view_ = viewer->getMainView();

  // add sky node
  simExamples::addDefaultSkyNode(viewer.get());

  // configure the scenario manager for large-scale support
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();

  viewer->setMap(map.get());

  // data source that records the platform data.
  simData::LinearInterpolator interpolator;
  simData::MemoryDataStore dataStore;
  dataStore.setInterpolator(&interpolator);
  dataStore.enableInterpolation(true);
  app.scenario_ = scene->getScenario();
  app.scenario_->bind(&dataStore);

  // Managed all the platform sims:
  osg::ref_ptr<simUtil::PlatformSimulatorManager> simman = new simUtil::PlatformSimulatorManager(&dataStore);

  SIM_NOTICE << "Building simulation... please wait..." << std::endl;

  for (unsigned i=0; i<numPlatforms; ++i)
  {
    simData::ObjectId platformId = addPlatform(dataStore);
    configPlatform(platformId, dataStore, i, argc, argv);
    simulatePlatform(platformId, simman.get());
  }
  simman->simulate(0, duration, hertz);

  SIM_NOTICE << "...done!" << std::endl;

  app.simHandler_ = new simUtil::SimulatorEventHandler(simman.get(), 0, duration, true);
  viewer->addEventHandler(app.simHandler_.get());

  // popup handler:
  viewer->addEventHandler(new simVis::PopupHandler(scene.get()));

  // instructions:
  viewer->getMainView()->addOverlayControl(createUI(app, duration));

  // add some stock OSG handlers
  viewer->installDebugHandlers();

  return viewer->run();
}

