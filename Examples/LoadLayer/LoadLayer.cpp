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
 * Demonstrates loading an osgEarth .earth file at runtime, showing how you
 * can swap terrain configurations on the fly.
 */
#include "osgEarth/ImageLayer"
#include "osgEarth/ModelLayer"
#include "osgEarthUtil/Controls"
#include "simNotify/Notify.h"
#include "simCore/Common/HighPerformanceGraphics.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simVis/Viewer.h"
#include "simVis/InsetViewEventHandler.h"
#include "simVis/Platform.h"
#include "simVis/osgEarthVersion.h"
#include "simUtil/DbConfigurationFile.h"
#include "simUtil/PlatformSimulator.h"
#include "simUtil/ExampleResources.h"
#include "simUtil/LayerFactory.h"

using namespace osgEarth::Util::Controls;

namespace {

static std::string terrainFile;
static bool foundImage = true;
static bool foundElevation = false;
static osgEarth::TileSourceOptions imageDriver;
static osgEarth::TileSourceOptions elevationDriver;

//----------------------------------------------------------------------------

static std::string s_title =
  "Load Single Layer Example";

static std::string s_help =
  "Controls:\n"
  " e : load first elevation layer (if it exists)\n"
  " i : load first image layer (if it exists)\n"
  " l : reload the terrain file\n"
  " r : remove all image layers\n"
  " t : toggle first elevation layer visibility\n";


static Control* createHelp()
{
  VBox* vbox = new VBox();
  vbox->setPadding(10);
  vbox->setBackColor(0, 0, 0, 0.6);
  vbox->addControl(new LabelControl(s_title, 20, osg::Vec4f(1, 1, 0, 1)));
  vbox->addControl(new LabelControl(s_help, 14, osg::Vec4f(.8, .8, .8, 1)));
  return vbox;
}

void addImageLayer(osgEarth::Map* map)
{
  if (map == NULL)
    return;
  osgEarth::CachePolicy cachePolicy;
  osg::ref_ptr<osgEarth::ImageLayer> imageLayer = simUtil::LayerFactory::newImageLayer("ImageLayer", imageDriver, map->getProfile(), &cachePolicy);
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  map->addLayer(imageLayer);
#else
  map->addImageLayer(imageLayer);
#endif
  if (!imageLayer->getStatus().isOK())
    std::cerr << "Image layer could not be created.\n";
}

void addElevationLayer(osgEarth::Map* map)
{
  if (map == NULL)
    return;
  osgEarth::CachePolicy cachePolicy;
  osg::ref_ptr<osgEarth::ElevationLayer> elevationLayer = simUtil::LayerFactory::newElevationLayer("ElevationLayer", elevationDriver, &cachePolicy);
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  map->addLayer(elevationLayer);
#else
  map->addElevationLayer(elevationLayer);
#endif
  if (!elevationLayer->getStatus().isOK())
    std::cerr << "Elevation layer could not be created.\n";
}

void loadTerrainFile(const std::string& terrainFile, simVis::Viewer*    viewer)
{
  osg::ref_ptr<osgEarth::MapNode> newMap;
  std::string validated = terrainFile;
  if (simUtil::DbConfigurationFile::resolveFilePath(validated) != 0)
  {
    std::cerr << "Failed to load terrain file : " << validated << "\n";
    return; // error
  }

  // load the map
  if (simUtil::DbConfigurationFile::load(newMap, validated, true) != 0 || !newMap.valid())
  {
    std::cerr << "Failed to load terrain file : " << validated << "\n";
    return; // error
  }
  if (newMap)
    viewer->setMapNode(newMap);
  else
    std::cerr << "Failed to load terrain file : " << validated << "\n";
}

void removeAllLayers(osgEarth::Map* map)
{
  if (map == NULL)
    return;
  osgEarth::ImageLayerVector imageLayers;
  osgEarth::ElevationLayerVector elevationLayers;
  osgEarth::ModelLayerVector modelLayers;

#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  map->getLayers(imageLayers);
  for (osgEarth::ImageLayerVector::const_iterator iter = imageLayers.begin(); iter != imageLayers.end(); ++iter)
  {
    map->removeLayer(*iter);
  }
  map->getLayers(elevationLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = elevationLayers.begin(); iter != elevationLayers.end(); ++iter)
  {
    map->removeLayer(*iter);
  }
  map->getLayers(modelLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = modelLayers.begin(); iter != modelLayers.end(); ++iter)
  {
    map->removeLayer(*iter);
  }
#else
  map->getImageLayers(imageLayers);
  for (osgEarth::ImageLayerVector::const_iterator iter = imageLayers.begin(); iter != imageLayers.end(); ++iter)
  {
    map->removeImageLayer(*iter);
  }
  map->getElevationLayers(elevationLayers);
  for (osgEarth::ElevationLayerVector::const_iterator iter = elevationLayers.begin(); iter != elevationLayers.end(); ++iter)
  {
    map->removeElevationLayer(*iter);
  }
  map->getModelLayers(modelLayers);
  for (osgEarth::ModelLayerVector::const_iterator iter = modelLayers.begin(); iter != modelLayers.end(); ++iter)
  {
    map->removeModelLayer(*iter);
  }
#endif
}

void toggleElevationLayers(osgEarth::Map* map)
{
  osgEarth::ElevationLayerVector elevationLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  map->getLayers(elevationLayers);
#else
  map->getElevationLayers(elevationLayers);
#endif
  for (osgEarth::ElevationLayerVector::const_iterator iter = elevationLayers.begin(); iter != elevationLayers.end(); ++iter)
  {
    (*iter)->setVisible(!(*iter)->getVisible());
  }
}

/// An event handler to assist in testing the InsetViewManager / Load Earth functionality.
struct MenuHandler : public osgGA::GUIEventHandler
{
  explicit MenuHandler(simVis::Viewer* viewer)
   : viewer_(viewer)
  {
  }

  bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    bool handled = false;

    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
      switch (ea.getKey())
      {
      case 'e': // LOAD ELEVATION LAYER
        if (foundElevation)
          addElevationLayer(viewer_->getSceneManager()->getMap());
        else
          std::cerr << "No elevation layer found to load\n";
        handled = true;
        break;
      case 'i': // LOAD IMAGE LAYER
        if (foundImage)
          addImageLayer(viewer_->getSceneManager()->getMap());
        else
          std::cerr << "No image layer found to load\n";
        handled = true;
        break;
      case 'l': // RELOAD EARTH FILE
        loadTerrainFile(terrainFile, viewer_);
        handled = true;
        break;
      case 'r': // REMOVE ALL LAYERS
        removeAllLayers(viewer_->getSceneManager()->getMap());
        handled = true;
        break;

      case 't':
        toggleElevationLayers(viewer_->getSceneManager()->getMap());
        handled = true;
        break;
      }
    }
    return handled;
  }

private:
  simVis::Viewer* viewer_;
};

simData::MemoryDataStore dataStore;
simData::ObjectId platformId;
simVis::PlatformNode* platform;
simUtil::PlatformSimulatorManager* simMan;

void addSimulation(simVis::ScenarioManager* scenario, simVis::View* mainView)
{
  scenario->bind(&dataStore);
  simMan = new simUtil::PlatformSimulatorManager(&dataStore);

  { // create the platform in the database (artificial scope for transaction)
    simData::DataStore::Transaction transaction;
    simData::PlatformProperties* newProps = dataStore.addPlatform(&transaction);
    platformId = newProps->id();
    transaction.complete(&newProps);
  }

  { // Set platform prefs (artificial scope for transaction)
    simData::DataStore::Transaction xaction;
    simData::PlatformPrefs* prefs = dataStore.mutable_platformPrefs(platformId, &xaction);
    prefs->mutable_commonprefs()->set_name("HSMST");
    prefs->set_dynamicscale(true);
    prefs->set_icon(EXAMPLE_SHIP_ICON);
    prefs->mutable_commonprefs()->mutable_labelprefs()->set_draw(true);
    prefs->set_surfaceclamping(true);
    xaction.complete(&prefs);
  }

  // Run the simulator
  osg::ref_ptr<simUtil::PlatformSimulator> sim = new simUtil::PlatformSimulator(platformId);
  // Nearly stationary over Kauai
  sim->addWaypoint(simUtil::Waypoint(22.074, -159.563445, 1, 30.0));
  sim->addWaypoint(simUtil::Waypoint(22.073, -159.563445, 1, 30.0));
  sim->addWaypoint(simUtil::Waypoint(22.074, -159.563445, 1, 30.0));
  sim->addWaypoint(simUtil::Waypoint(22.073, -159.563445, 1, 30.0));

  simMan->addSimulator(sim);
  platform = scenario->find<simVis::PlatformNode>(platformId);

  simMan->simulate(0,120, 60);
  mainView->addEventHandler(new simVis::SimulatorEventHandler(simMan, 0, 120, true));
}

}


int main(int argc, char** argv)
{
  // Set up the scene:
  simCore::checkVersionThrow();
  simExamples::configureSearchPaths();

  if (argc != 2)
  {
    std::cerr << "USAGE:\n"
      << argv[0] << " <terrain.earth>\n\n"
      << "  <terrain.earth>: Terrain configuration file to load.\n\n";
    return 0;
  }
  terrainFile = argv[argc-1];

  osg::ref_ptr<simVis::Viewer> viewer = new simVis::Viewer();

  // inset view support
  simVis::View* mainView = viewer->getMainView();

  // Handles hotkeys from user
  mainView->addEventHandler(new MenuHandler(viewer));

  if (!terrainFile.empty())
  {
    loadTerrainFile(terrainFile, viewer);
  }
  else
  {
    osg::ref_ptr<osgEarth::Map> map = simExamples::createDefaultExampleMap();
    viewer->setMap(map);
  }

  // now store off the image and elevation layers
  osgEarth::ImageLayerVector imageLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  viewer->getSceneManager()->getMap()->getLayers(imageLayers);
#else
  viewer->getSceneManager()->getMap()->getImageLayers(imageLayers);
#endif
  if (!imageLayers.empty())
  {
    foundImage = true;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    imageDriver = imageLayers.front()->options().driver().get();
#else
    imageDriver = imageLayers.front()->getImageLayerOptions().driver().get();
#endif
  }
  else
    std::cerr << "Failed to find an image layer in supplied configuration.\n";

  osgEarth::ElevationLayerVector elevationLayers;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
  viewer->getSceneManager()->getMap()->getLayers(elevationLayers);
#else
  viewer->getSceneManager()->getMap()->getElevationLayers(elevationLayers);
#endif
  if (!elevationLayers.empty())
  {
    foundElevation = true;
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
    elevationDriver = elevationLayers.front()->options().driver().get();
#else
    elevationDriver = elevationLayers.front()->getElevationLayerOptions().driver().get();
#endif
  }
  else
    std::cerr << "Failed to find an elevation layer in supplied configuration.\n";

  // add sky node
  simExamples::addDefaultSkyNode(viewer);

  // Add a platform over Kauai
  addSimulation(viewer->getSceneManager()->getScenario(), mainView);

  // Center on the entity
  simVis::Viewpoint vp;
  vp.heading()->set(20, osgEarth::Units::DEGREES);
  vp.pitch()->set(-20, osgEarth::Units::DEGREES);
  vp.range()->set(90, osgEarth::Units::METERS);
  mainView->tetherCamera(platform, vp, 0);

  // show the help menu
  viewer->getMainView()->addOverlayControl(createHelp());
  viewer->installDebugHandlers();
  viewer->run();
}
