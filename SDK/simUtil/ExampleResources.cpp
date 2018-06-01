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
#include <cstdlib>

#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "osgEarth/ImageLayer"
#include "osgEarthDrivers/cache_filesystem/FileSystemCache"
#include "osgEarthDrivers/mbtiles/MBTilesOptions"
#include "osgEarthDrivers/tms/TMSOptions"
#include "osgEarthDrivers/gdal/GDALOptions"
#include "osgEarthDrivers/sky_simple/SimpleSkyOptions"

#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simCore/Time/ClockImpl.h"
#include "simData/DataStore.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/DBOptions.h"
#include "simVis/Registry.h"
#include "simVis/SceneManager.h"
#include "simVis/Viewer.h"
#include "simUtil/NullSkyModel.h"
#include "simUtil/ExampleResources.h"


#ifdef WIN32
const char PATH_SEP = '\\';
#else
const char PATH_SEP = '/';
#endif

using namespace osgEarth::Drivers;

namespace {

// Stub functions to replace deprecated add__Layer methods in Map
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,6,0)
void addLayer(osgEarth::Map* map, osgEarth::Layer* layer)
{
  map->addLayer(layer);
}
#else
void addLayer(osgEarth::Map* map, osgEarth::ImageLayer* layer)
{
  map->addImageLayer(layer);
}
void addLayer(osgEarth::Map* map, osgEarth::ElevationLayer* layer)
{
  map->addElevationLayer(layer);
}
#endif

}

/// true if argv contains the pattern string.
bool simExamples::hasArg(const std::string& pattern, int argc, char** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    std::string token(argv[i]);
    if (pattern.compare(token) == 0)
      return true;
  }
  return false;
}

bool simExamples::readArg(const std::string& pattern, int argc, char** argv, std::string& out)
{
  for (int i = 1; i < argc; ++i)
  {
    std::string token(argv[i]);
    if (pattern.compare(token) == 0)
    {
      if (i < argc - 1)
      {
        out = argv[i + 1];
        return true;
      }
    }
  }
  return false;
}

// Uncomment to use remote map data
//#define USE_REMOTE_MAP_DATA

Map* simExamples::createDefaultExampleMap()
{
#ifdef USE_REMOTE_MAP_DATA
  return createRemoteWorldMap();
#else
  //return createHawaiiTMSMap();
  return createHawaiiMap();
#endif

}

Map* simExamples::createRemoteWorldMap()
{
  MapOptions mapOptions;
  mapOptions.cachePolicy() = CachePolicy::NO_CACHE;

  Map* map = new Map(mapOptions);

  // worldwide imagery layer:
  {
    TMSOptions options;
    options.url() = EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS;
    addLayer(map, new ImageLayer("simdis.imagery", options));
  }

  // global elevation layer
  {
    TMSOptions options;
    options.url() = EXAMPLE_ELEVATION_LAYER_TMS;
    addLayer(map, new ElevationLayer("simdis.elevation", options));
  }

  return map;
}

Map* simExamples::createWorldMapWithFlatOcean()
{
  MapOptions mapOptions;
  mapOptions.cachePolicy() = CachePolicy::NO_CACHE;

  Map* map = new Map(mapOptions);

  // worldwide imagery layer:
  {
    TMSOptions options;
    options.url() = EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS;
    addLayer(map, new ImageLayer("simdis.imagery", options));
  }

  // global elevation layer (with no bathymetry)
  {
    TMSOptions options;
    options.url() = EXAMPLE_ELEVATION_LAYER_TMS;
#if SDK_OSGEARTH_VERSION_LESS_OR_EQUAL(1,6,0)
    options.noDataMinValue() = -1.0;
#endif
    addLayer(map, new ElevationLayer("simdis.elevation.nobathy", options));
  }

  return map;
}

Map* simExamples::createHawaiiTMSMap()
{
  osg::ref_ptr<Map> map;
  std::string path = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HAWAII_TMS_EARTH;
  osg::ref_ptr<osg::Node> node = URI(path).getNode();
  if (node.valid() && dynamic_cast<MapNode*>(node.get()))
    map = dynamic_cast<MapNode*>(node.get())->getMap();

  node = NULL;
  return map.release();
}

// A sample map that demonstrates SIMDIS .db format support (Hi-res Hawaii inset)

Map* simExamples::createHawaiiMap()
{
  // configure an EGM96 MSL globe.
  ProfileOptions profileOptions;
  profileOptions.vsrsString() = "egm96-meters";

  MapOptions mapOptions;
  mapOptions.profile() = profileOptions;
  mapOptions.cachePolicy() = CachePolicy::NO_CACHE;

  Map* map = new Map(mapOptions);

  // the SIMDIS etopo2 default imagery:
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_GLOBAL_IMAGERY_LAYER_DB;
    addLayer(map, new ImageLayer("Earth", sourceOptions));
  }

  // the PDC Hawaii hi-res inset:
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HIRES_INSET_LAYER_DB;

    ImageLayerOptions layerOptions("Kauai Niihau", sourceOptions);
    layerOptions.minLevel() = 3;

    addLayer(map, new ImageLayer(layerOptions));
  }

#if 0
  // debug
  ImageLayerOptions debugOptions("debug");
  debugOptions.driver()->setDriver("debug");
  debugOptions.maxLevel() = 9;
  addLayer(map, new ImageLayer(debugOptions));
#endif

  // the USGS elevation data inset for Kauai
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_ELEVATION_LAYER_DB;

    ElevationLayerOptions layerOptions("Kauai Elevation", sourceOptions);
    //layerOptions.minLevel() = 7;

    addLayer(map, new ElevationLayer(layerOptions));
  }

  return map;
}

// A sample map that uses SIMDIS db and local bathymetric GeoTIFF
Map* simExamples::createHawaiiMapLocalWithBathymetry()
{
  // configure an EGM96 MSL globe.
  ProfileOptions profileOptions;
  profileOptions.vsrsString() = "egm96";

  MapOptions mapOptions;
  mapOptions.profile() = profileOptions;
  mapOptions.cachePolicy() = CachePolicy::NO_CACHE;

  Map* map = new Map(mapOptions);

  // the SIMDIS etopo2 default imagery:
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_GLOBAL_IMAGERY_LAYER_DB;
    addLayer(map, new ImageLayer("simdis.imagery.topo2", sourceOptions));
  }

  // the PDC Hawaii hi-res inset:
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HIRES_INSET_LAYER_DB;

    ImageLayerOptions layerOptions("simdis.imagery.pdc", sourceOptions);
    //layerOptions.minLevel() = 3;

    addLayer(map, new ImageLayer(layerOptions));
  }

  // An elevation map for the Hawaii area
  {
    osgEarth::Drivers::GDALOptions sourceOptions;
    sourceOptions.url() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HAWAII_LOCAL_BATHYMETRY;

    ElevationLayerOptions layerOptions("simdis.elevation.hawaii-srtm30plus-bathy", sourceOptions);

    addLayer(map, new ElevationLayer(layerOptions));
  }

  // the USGS elevation data inset for Kauai
  {
    osgEarth::Drivers::MBTilesTileSourceOptions sourceOptions;
    sourceOptions.filename() = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_ELEVATION_LAYER_DB;

    ElevationLayerOptions layerOptions("simdis.elevation.usgs-elevation", sourceOptions);
    //layerOptions.minLevel() = 7;

    addLayer(map, new ElevationLayer(layerOptions));
  }

  return map;
}

void simExamples::configureSearchPaths()
{
  const std::string folders[] = {
    "aqm-37c",
    "as-17_krypton",
    "decoys",
    "dragon_eye",
    "EKV",
    "mm-38_exocet",
    "mm-40_exocet",
    "nulka",
    "OBV",
    "OSP",
    "SRALT",
    "STARS",
    "UGV",
    "USSV",
    "imageIcons",
    ""  // Placeholder last item
  };

  std::string basePath = getSampleDataPath();
  std::string modelPath = basePath + PATH_SEP + "models";

  simVis::Registry* simVisRegistry = simVis::Registry::instance();
  simVis::FilePathList pathList;
  simVisRegistry->getModelSearchPaths(pathList);

  // Add variables from SIMDIS_SDK_FILE_PATH
  pathList.push_back(basePath);
  pathList.push_back(basePath + PATH_SEP + "textures");
  pathList.push_back(modelPath);

  // Add all of the directories for SIMDIS_SDK_FILE_PATH models
  for (size_t k = 0; !folders[k].empty(); ++k)
    pathList.push_back(modelPath + PATH_SEP + folders[k]);

  // Add SIMDIS_DIR variables
  const std::string SIMDIS_DIR = simCore::getEnvVar("SIMDIS_DIR");
  if (!SIMDIS_DIR.empty())
  {
    const std::string dataDir = SIMDIS_DIR + PATH_SEP + "data";
    const std::string simdisDirModelFolders[] = {
      "aircraft",
      "decoy",
      "equipment",
      "imageIcons",
      std::string("imageIcons") + PATH_SEP + "NTDS",
      std::string("imageIcons") + PATH_SEP + "NTDS" + PATH_SEP + "jreap",
      std::string("imageIcons") + PATH_SEP + "NTDS" + PATH_SEP + "large",
      std::string("imageIcons") + PATH_SEP + "NTDS" + PATH_SEP + "small",
      std::string("imageIcons") + PATH_SEP + "SCORE",
      "missiles",
      "other",
      "satellite",
      "ships",
      "sites",
      "vehicles",
      ""  // Placeholder last item
    };
    // Iterate over the folders, adding them all to the search path
    for (size_t k = 0; !simdisDirModelFolders[k].empty(); ++k)
      pathList.push_back(dataDir + PATH_SEP + "models" + PATH_SEP + simdisDirModelFolders[k]);
    // Add textures directory to the search path
    pathList.push_back(dataDir + PATH_SEP + "textures" + PATH_SEP + "models");
  }
  // Save model path
  simVis::FilePathList modelPathList(pathList.begin(), pathList.end());

  // Next, configure the data file path list, which is separate from the model path list
  pathList = osgDB::getDataFilePathList();
  if (!SIMDIS_DIR.empty())
  {
    // osgEarth textures, including the moon
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "osgEarth");
    // SIMDIS SDK shaders are placed here
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "shaders");
    // SIMDIS model textures
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "modelsFull");
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "models");
    // SIMDIS textures
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "app");
    // osgText looks under data directory for fonts/fontname.ttf -- add data for data/fonts folder
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data");

    // GOG files under data/GOG
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "GOG");
  }
  pathList.push_back(basePath + PATH_SEP + "textures");

  // Add the user dir to the end of the path lists to be searched last
  const std::string simdisUserDir = simCore::getEnvVar("SIMDIS_USER_DIR");
  if (!simdisUserDir.empty())
  {
    modelPathList.push_back(simdisUserDir);
    pathList.push_back(simdisUserDir);
  }

  // Set the model and data path lists
  simVisRegistry->setModelSearchPaths(modelPathList);
  osgDB::setDataFilePathList(pathList);

  osgEarth::Registry::instance()->setDefaultFont(simVisRegistry->getOrCreateFont("arial.ttf"));

#ifndef WIN32
  // On Linux, add a search path for libraries relative to executable path
  osgDB::FilePathList libPaths = osgDB::getLibraryFilePathList();
  // lib/amd64-linux is used by SIMDIS applications distributed by NRL; lib is used by SDK build defaults
  libPaths.push_back("../lib/amd64-linux");
  libPaths.push_back("../lib");
  osgDB::setLibraryFilePathList(libPaths);
#endif

  // Configure OSG to search for the right GL version.  By default, GL3 builds use "1.0" as the version,
  // which creates a compatibility context at the highest level.  That creates problems with GL core
  // profile on some drivers and cards that do not support compatibility mode.  As a result, we end up
  // getting a GL 1.4 context that only support GLSL 1.2.
#ifdef OSG_GL3_AVAILABLE
  osg::DisplaySettings* instance = osg::DisplaySettings::instance().get();
  if (instance->getGLContextVersion() == "1.0")
    instance->setGLContextVersion("3.3");
#ifdef __linux__
  // To compound the problem, certain MESA drivers on Linux have an additional requirement of setting
  // the MESA_GL_VERSION_OVERRIDE environment variable, else we get a bad version.
  if (getenv("MESA_GL_VERSION_OVERRIDE") == NULL)
    setenv("MESA_GL_VERSION_OVERRIDE", instance->getGLContextVersion().c_str(), 1);
#endif
#endif
}

std::string simExamples::getSampleDataPath()
{
  const std::string env = simCore::getEnvVar(EXAMPLE_FILE_PATH_VAR);
  if (env.empty())
  {
    SIM_WARN << "The " << EXAMPLE_FILE_PATH_VAR << " environment variable has not been set. " <<
      "Searching for data in " << EXAMPLE_DEFAULT_DATA_PATH << ".\n";
    return std::string(EXAMPLE_DEFAULT_DATA_PATH);
  }
  else
  {
    return env;
  }
}

std::string simExamples::getTritonResourcesPath()
{
  // Defaults to ${SIMDIS_DIR}/data/Triton/
  const std::string SIMDIS_DIR = simCore::getEnvVar("SIMDIS_DIR");
  if (!SIMDIS_DIR.empty())
  {
    const std::string dir = SIMDIS_DIR + "/data/Triton";
    if (osgDB::fileExists(dir))
      return dir;
  }
  const std::string TRITON_PATH = simCore::getEnvVar("TRITON_PATH");
  if (!TRITON_PATH.empty())
  {
    return TRITON_PATH + "/Resources"; // note upper case r
  }
  return "";
}

std::string simExamples::getSilverLiningResourcesPath()
{
  // Defaults to ${SIMDIS_DIR}/data/SilverLining/
  const std::string SIMDIS_DIR = simCore::getEnvVar("SIMDIS_DIR");
  if (!SIMDIS_DIR.empty())
  {
    const std::string dir = SIMDIS_DIR + "/data/SilverLining";
    if (osgDB::fileExists(dir))
      return dir;
  }
  const std::string SILVERLINING_PATH = simCore::getEnvVar("SILVERLINING_PATH");
  if (!SILVERLINING_PATH.empty())
  {
    return SILVERLINING_PATH + "/resources"; // note lower case r
  }
  return "";
}

void simExamples::addDefaultSkyNode(simVis::Viewer* viewer)
{
  osg::ref_ptr<simVis::SceneManager> scene = viewer->getSceneManager();
  simExamples::addDefaultSkyNode(scene.get());
  // Refresh scene manager on sky node change to force correct reattachment of manipulators
  std::vector<simVis::View*> views;
  viewer->getViews(views);
  // Simply set the scene manager, which does the sky reattachment properly
  for (std::vector<simVis::View*>::iterator iter = views.begin(); iter != views.end(); ++iter)
  {
    // Only reset the scene manager if it matches what is currently in the viewer. We aren't trying to change the scene manager, just attach the sky node
    if ((*iter)->getSceneManager() == scene)
      (*iter)->setSceneManager(scene.get());
  }
}

void simExamples::addDefaultSkyNode(simVis::SceneManager* sceneMan)
{
  // Only install simple sky if the osgEarth capabilities permit it
  if (osgEarth::Registry::capabilities().getGLSLVersionInt() >= 330)
  {
    osgEarth::Drivers::SimpleSky::SimpleSkyOptions skyOptions;
    skyOptions.atmosphericLighting() = false;
    skyOptions.ambient() = 0.5f;
    skyOptions.exposure() = 2.0f;
    sceneMan->setSkyNode(osgEarth::Util::SkyNode::create(osgEarth::ConfigOptions(skyOptions), sceneMan->getMapNode()));
  }
  else
    sceneMan->setSkyNode(new simUtil::NullSkyModel);
}

////////////////////////////////////////////////

simExamples::SkyNodeTimeUpdater::SkyNodeTimeUpdater(simVis::SceneManager* mgr)
  : sceneManager_(mgr),
    lastTime_(simCore::INFINITE_TIME_STAMP),
    hoursOffset_(0.0)
{
}

void simExamples::SkyNodeTimeUpdater::setSceneManager(simVis::SceneManager* mgr)
{
  sceneManager_ = mgr;
}

void simExamples::SkyNodeTimeUpdater::onSetTime(const simCore::TimeStamp &t, bool isJump)
{
  lastTime_ = t;
  if (sceneManager_.valid() && sceneManager_->getSkyNode() != NULL)
  {
    sceneManager_->getSkyNode()->setDateTime(osgEarth::Util::DateTime(t.secondsSinceRefYear(1970) + simCore::Seconds(hoursOffset_ * 3600)));
  }
}

void simExamples::SkyNodeTimeUpdater::onTimeLoop()
{
  // noop
}

void simExamples::SkyNodeTimeUpdater::adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
{
  // no-op
}

void simExamples::SkyNodeTimeUpdater::setHoursOffset(double hours)
{
  if (hours == hoursOffset_)
    return;
  hoursOffset_ = hours;
  // Update the model
  if (lastTime_ != simCore::INFINITE_TIME_STAMP)
    onSetTime(lastTime_, false);
}

double simExamples::SkyNodeTimeUpdater::hoursOffset() const
{
  return hoursOffset_;
}

////////////////////////////////////////////////

simExamples::IdleClockCallback::IdleClockCallback(simCore::ClockImpl& clock, simData::DataStore& dataStore)
  : clock_(clock),
    dataStore_(dataStore)
{
}

void simExamples::IdleClockCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
  clock_.idle();
  const double nowTime = clock_.currentTime().secondsSinceRefYear(dataStore_.referenceYear());
  dataStore_.update(nowTime);
  traverse(node, nv);
}
