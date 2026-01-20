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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cstdlib>

#include "osgEarth/Capabilities"
#include "osgEarth/GDAL"
#include "osgEarth/ImageLayer"
#include "osgEarth/MBTiles"
#include "osgEarth/Registry"
#include "osgEarth/TMS"
#include "osgEarthDrivers/cache_filesystem/FileSystemCache"
#include "osgEarthDrivers/sky_simple/SimpleSkyOptions"

#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simCore/Time/ClockImpl.h"
#include "simData/DataStore.h"
#include "simVis/Gl3Utils.h"
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
  Map* map = new Map();
  map->setCachePolicy(CachePolicy::NO_CACHE);

  TMSImageLayer* imagery = new TMSImageLayer();
  imagery->setName("simdis.imagery");
  imagery->setURL(EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS);
  map->addLayer(imagery);

  TMSElevationLayer* elevation = new TMSElevationLayer();
  elevation->setName("simdis.elevation");
  elevation->setURL(EXAMPLE_ELEVATION_LAYER_TMS);
  map->addLayer(elevation);

  return map;
}

Map* simExamples::createWorldMapWithFlatOcean()
{
  Map* map = new Map();
  map->setCachePolicy(CachePolicy::NO_CACHE);

  TMSImageLayer* imagery = new TMSImageLayer();
  imagery->setName("simdis.imagery");
  imagery->setURL(EXAMPLE_GLOBAL_IMAGERY_LAYER_TMS);
  map->addLayer(imagery);

  TMSElevationLayer* elevation = new TMSElevationLayer();
  elevation->setName("simdis.elevation");
  elevation->setURL(EXAMPLE_ELEVATION_LAYER_TMS);
  elevation->setMinValidValue(-1.0);
  map->addLayer(elevation);

  return map;
}

Map* simExamples::createHawaiiTMSMap()
{
  osg::ref_ptr<Map> map;
  std::string path = getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HAWAII_TMS_EARTH;
  osg::ref_ptr<osg::Node> node = URI(path).getNode();
  if (node.valid() && dynamic_cast<MapNode*>(node.get()))
    map = dynamic_cast<MapNode*>(node.get())->getMap();

  node = nullptr;
  return map.release();
}

// A sample map that demonstrates SIMDIS .db format support (Hi-res Hawaii inset)
Map* simExamples::createHawaiiMap()
{
  // configure an EGM96 MSL globe.
  const Profile* profile = Profile::create("wgs84", "egm96");

  Map* map = new Map();
  map->setProfile(profile);
  map->setCachePolicy(CachePolicy::NO_CACHE);

  // the SIMDIS etopo2 default imagery:
  MBTilesImageLayer* baseLayer = new MBTilesImageLayer();
  baseLayer->setName("Whole Earth");
  baseLayer->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_GLOBAL_IMAGERY_LAYER_DB);
  map->addLayer(baseLayer);

  // the PDC Hawaii hi-res inset:
  MBTilesImageLayer* inset = new MBTilesImageLayer();
  inset->setName("Kauai Niihau");
  inset->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HIRES_INSET_LAYER_DB);
  inset->setMinLevel(3u);
  map->addLayer(inset);

  // the USGS elevation data inset for Kauai
  MBTilesElevationLayer* elev = new MBTilesElevationLayer();
  elev->setName("Kauai Elevation");
  elev->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_ELEVATION_LAYER_DB);
  // SIM-13914 - setting min_level on elevation layers causing artifacts
  //elev->setMinLevel(7u);
  map->addLayer(elev);

  return map;
}

// A sample map that uses SIMDIS db and local bathymetric GeoTIFF
Map* simExamples::createHawaiiMapLocalWithBathymetry()
{
  // configure an EGM96 MSL globe.
  const Profile* profile = Profile::create("wgs84", "egm96");

  Map* map = new Map();
  map->setProfile(profile);
  map->setCachePolicy(CachePolicy::NO_CACHE);

  // the SIMDIS etopo2 default imagery:
  MBTilesImageLayer* baseLayer = new MBTilesImageLayer();
  baseLayer->setName("simdis.imagery.topo2");
  baseLayer->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_GLOBAL_IMAGERY_LAYER_DB);
  map->addLayer(baseLayer);

  // the PDC Hawaii hi-res inset:
  MBTilesImageLayer* pdc = new MBTilesImageLayer();
  pdc->setName("simdis.imagery.pdc");
  pdc->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HIRES_INSET_LAYER_DB);
  map->addLayer(pdc);

  // An elevation map for the Hawaii area
  GDALElevationLayer* baseElev = new GDALElevationLayer();
  baseElev->setName("simdis.elevation.hawaii-srtm30plus-bathy");
  baseElev->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_HAWAII_LOCAL_BATHYMETRY);
  map->addLayer(baseElev);

  // the USGS elevation data inset for Kauai
  MBTilesElevationLayer* insetElev = new MBTilesElevationLayer();
  insetElev->setName("simdis.elevation.usgs-elevation");
  insetElev->setURL(getSampleDataPath() + PATH_SEP + "terrain" + PATH_SEP + EXAMPLE_ELEVATION_LAYER_DB);
  map->addLayer(insetElev);

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

  const std::string& basePath = getSampleDataPath();
  const std::string modelPath = basePath + PATH_SEP + "models";

#ifndef WIN32
  {
    // On Linux, add a search path for libraries relative to executable path (installDir/bin)
    osgDB::FilePathList libPaths = osgDB::getLibraryFilePathList();
    // SDK examples from an SDK build need ../lib in the libpath
    libPaths.push_back("../lib");
    osgDB::setLibraryFilePathList(libPaths);
  }
#endif

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
    // Iterate over the folders, adding them all to the search path (legacy)
    for (size_t k = 0; !simdisDirModelFolders[k].empty(); ++k)
      pathList.push_back(dataDir + PATH_SEP + "models" + PATH_SEP + simdisDirModelFolders[k]);
    // Iterate over the folders, adding them all to the search path (public)
    for (size_t k = 0; !simdisDirModelFolders[k].empty(); ++k)
      pathList.push_back(dataDir + PATH_SEP + "models" + PATH_SEP + "public" + PATH_SEP + simdisDirModelFolders[k]);
    // Iterate over the folders, adding them all to the search path (restricted)
    for (size_t k = 0; !simdisDirModelFolders[k].empty(); ++k)
      pathList.push_back(dataDir + PATH_SEP + "models" + PATH_SEP + "restricted" + PATH_SEP + simdisDirModelFolders[k]);
    // Add textures directory to the search path
    pathList.push_back(dataDir + PATH_SEP + "textures" + PATH_SEP + "models");
    pathList.push_back(dataDir + PATH_SEP + "textures" + PATH_SEP + "models" + PATH_SEP + "public");
    pathList.push_back(dataDir + PATH_SEP + "textures" + PATH_SEP + "models" + PATH_SEP + "restricted");
  }
  // Save model path
  simVis::FilePathList modelPathList(pathList.begin(), pathList.end());

  // Next, configure the data file path list, which is separate from the model path list
  pathList = osgDB::getDataFilePathList();
  if (!SIMDIS_DIR.empty())
  {
    // SIMDIS SDK shaders are placed here; SIMDIS shaders override data/osgEarth shaders
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "shaders");
    // osgEarth textures, including the moon
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "osgEarth");
    // SIMDIS model textures
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "modelsFull");
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "models");
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "models" + PATH_SEP + "public");
    pathList.push_back(SIMDIS_DIR + PATH_SEP + "data" + PATH_SEP + "textures" + PATH_SEP + "models" + PATH_SEP + "restricted");
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

  // Set the environment variable for proj4 if it's not set already
  const std::string PROJ_LIB = simCore::getEnvVar("PROJ_LIB");
  if (PROJ_LIB.empty() || !osgDB::fileExists(PROJ_LIB + "/proj/proj.db"))
  {
    // First try to use the SIMDIS_DIR, then fall back to SIMDIS_SDK_DATA_PATH
    if (!SIMDIS_DIR.empty() && osgDB::fileExists(SIMDIS_DIR + "/data/proj/proj.db"))
      simCore::setEnvVar("PROJ_LIB", SIMDIS_DIR + "/data/proj", true);
    else if (!basePath.empty() && osgDB::fileExists(basePath + "/proj/proj.db"))
      simCore::setEnvVar("PROJ_LIB", basePath + "/proj", true);
  }

  // Fix the GL3 version
  simVis::applyMesaGlVersionOverride();
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
  const auto& caps = osgEarth::Registry::capabilities();
  if (caps.getGLSLVersionInt() >= 330)
  {
    osgEarth::Drivers::SimpleSky::SimpleSkyOptions skyOptions;
    skyOptions.atmosphericLighting() = false;
    skyOptions.ambient() = 0.5f;
    skyOptions.exposure() = 2.0f;

    // Stars normally look good except on most newer Mesa drivers. They appear blocky
    // and not rounded. This appears to be a driver bug, but can be worked around
    // by forcing the star size to 1.5 when Mesa drivers are detected.
    const auto& glVersionString = caps.getVersion();
    const auto mesaPos = glVersionString.find("Mesa ");
    if (mesaPos != std::string::npos)
      skyOptions.starSize() = 1.5;

    sceneMan->setSkyNode(osgEarth::SkyNode::create(skyOptions));
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
  if (sceneManager_.valid() && sceneManager_->getSkyNode() != nullptr)
  {
    sceneManager_->getSkyNode()->setDateTime(osgEarth::DateTime(t.secondsSinceRefYear(1970) + simCore::Seconds(hoursOffset_ * simCore::SECPERHOUR)));
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
