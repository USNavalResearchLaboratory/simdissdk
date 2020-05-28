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
#include <iomanip>
#include <sstream>
#include "osgDB/FileUtils"
#include "osgDB/FileNameUtils"
#include "osgEarth/ImageLayer"
#include "osgEarth/Map"
#include "osgEarth/TerrainOptions"

#include "simNotify/Notify.h"
#include "simCore/Common/Exception.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simVis/AlphaColorFilter.h"
#include "simVis/DBFormat.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simUtil/DbConfigurationFile.h"

namespace simUtil {

static const int SCRIPT_MAX_RECURSIVE_DEPTH = 8;

// token keyword defines
static const std::string version_keyword = "version";
static const std::string sphere_keyword = "sphere";
static const std::string earth_surface_keyword = "EarthSurface";
static const std::string cloud_layer_keyword = "CloudLayer";
static const std::string ocean_surface_keyword = "oceanSurface";
static const std::string texture_set_keyword = "textureSet";
static const std::string texture_set_timestamped_keyword = "timeStampedTextureSet";
static const std::string altitude_set_keyword = "altitudeSet";
static const std::string altitude_set_timestamped_keyword = "timeStampedAltitudeSet";
static const std::string cloud_opaque_keyword = "cloudThresholdOpaque";
static const std::string cloud_clear_keyword = "cloudThresholdClear";
static const std::string dbFile_keyword = "dbFile";
static const std::string active_keyword = "active";
static const std::string opacity_keyword = "opacity";
static const std::string transparency_keyword = "transparency";
static const std::string shallowest_keyword = "shallowestLevel";
static const std::string deepest_keyword = "deepestLevel";
//static const std::string fadeShallow_keyword = "fadeShallow"; // TODO: SIM-5129 implement layer fading
static const std::string noDataValue_keyword = "noDataValue";
static const std::string imageLayerName = "ImageLayer_";
static const std::string elevationLayerName = "ElevationLayer_";

static const std::string earthFileSuffix = ".earth";

const std::string DbConfigurationFile::TERRAIN_CONFIG_FILE_SETTING = "Private/TerrainConfigurationFile";

int DbConfigurationFile::load(osg::ref_ptr<osgEarth::MapNode>& mapNode, const std::string& configFile, bool quiet)
{
  // try to find the file
  std::string adjustedConfigFile = configFile;
  if (simUtil::DbConfigurationFile::resolveFilePath(adjustedConfigFile) != 0)
  {
    if (!quiet)
    {
      SIM_ERROR << "Could not resolve filename " << configFile << "\n";
    }
    mapNode = NULL;
    return 1;
  }

  // Load the map, wrapping in a ref_ptr to get rid of the memory when done copying
  // is this a .earth file?
  if (osgDB::getFileExtensionIncludingDot(configFile) == earthFileSuffix)
  {
    mapNode = NULL;
    SAFETRYBEGIN;

    // Load the map, wrap Node in a ref_ptr to get rid of the memory when done, after switching to Viewer::setMap method
    osg::ref_ptr<osg::Node> loadedModel = DbConfigurationFile::readEarthFile(adjustedConfigFile);
    if (loadedModel.valid())
    {
      // Find the MapNode
      mapNode = osgEarth::MapNode::findMapNode(loadedModel.get());
    }
    SAFETRYEND((std::string("osgEarth processing of file ") + configFile));
  }
  else // probably a SIMDIS 9 config file
  {
    mapNode = NULL;
    SAFETRYBEGIN;
    osg::ref_ptr<osgEarth::Map> map = simUtil::DbConfigurationFile::loadLegacyConfigFile(adjustedConfigFile, quiet);
    if (map.valid())
    {
      mapNode = new osgEarth::MapNode(map.get());
      simVis::SceneManager::initializeTerrainOptions(mapNode.get());
    }

    SAFETRYEND((std::string("legacy SIMDIS 9 .txt processing of file ") + configFile));
  }

  // NULL check on the node pointer
  if (!mapNode.valid())
  {
    if (!quiet)
    {
      SIM_ERROR << "Unable to load valid Map from " << adjustedConfigFile << "\n";
    }
    return 1;
  }

  // set the map's name
  if (mapNode->getMap() != NULL)
    mapNode->getMap()->setMapName(osgDB::getSimpleFileName(adjustedConfigFile));
  return 0;
}

osgEarth::Map* DbConfigurationFile::loadLegacyConfigFile(const std::string& filename, bool quiet)
{
  std::string configFilename = simCore::backslashToFrontslash(filename);

  // NOTE: std::ifstream here will cause linker errors on VC10 due to osgDB's inheritance
  // issues with osgDB::fstream.
  // For more information see:
  //  http://forum.openscenegraph.org/viewtopic.php?t=8099
  //  http://forum.osgearth.org/SVN-Build-errors-using-VS10-Win32-td6847842.html
  osgDB::ifstream infile(configFilename.c_str());

  // checks if the file didn't open
  if (!infile || !osgDB::fileExists(configFilename))
  {
    if (!quiet)
    {
      SIM_ERROR << "Unable to open file (" << filename << ").\n";
    }
    return NULL;
  }

  std::string sdTerrainDirStr;
  // Since full path is passed in for file, get directory from there
  if (configFilename.find("/") != std::string::npos)
    sdTerrainDirStr = configFilename.substr(0, configFilename.rfind("/") + 1);

  // Configure a NULL map at first
  osgEarth::Map* map = NULL;

  // set up names for the loaded layers
  int imageLayerCount = 1;
  int elevationLayerCount = 1;

  std::string st;
  std::vector<std::string> tokens;
  size_t currentLineNumber = 0;
  bool gotValidFirstLine = false;

  // steps through each line of the file
  while (simCore::getStrippedLine(infile, st))
  {
    simCore::quoteCommentTokenizer(st, tokens);

    // increment the line counter
    ++currentLineNumber;
    simCore::removeQuotes(tokens);
    if (tokens.empty())
      continue;

    // do some error checking on the parsed line, handle 'version', see that 'sphere' is there
    if (DbConfigurationFile::parseCommonTokens_(tokens, currentLineNumber) != 0)
    {
      // If this is the first line and it was invalid, stop parsing the file and return an error.
      // Covers the case where a binary or non-useful file is passed in
      if (!gotValidFirstLine)
      {
        // Programming error if assert fires; indicates memory leak
        assert(map == NULL);
        return NULL;
      }
      continue;
    }

    // Create an empty map if we got the first valid line read (means we'll have a return value)
    gotValidFirstLine = true;
    if (!map)
    {
      map = new osgEarth::Map();
      map->beginUpdate();
    }

    // NOW handle all the keywords
    DbConfigurationFile::parseLayers_(tokens, map, imageLayerCount, elevationLayerCount, sdTerrainDirStr);

  } // end while loop through each line of the file

  if (map)
    map->endUpdate();
  return map;
}

void DbConfigurationFile::processToken(std::string& token)
{
  token = simCore::removeQuotes(token);
  if (simCore::hasEnv(token))
  {
    std::string rv;
    int depth = SCRIPT_MAX_RECURSIVE_DEPTH;
    do
    {
      rv = token;
      token = simCore::expandEnv(rv);
    } while ((rv != token) && (--depth > 0));
  }
  token = simCore::backslashToFrontslash(token);
}

int DbConfigurationFile::resolveFilePath(std::string& fileName)
{
  fileName = simCore::backslashToFrontslash(fileName);
  if (osgDB::fileExists(fileName))
    return 0;
  // now see if some envars have been passed in with the file path
  DbConfigurationFile::processToken(fileName);
  if (osgDB::fileExists(fileName))
    return 0;
  // still no success, see if the file is in SIMDIS_TERRAIN dir
  else
  {
    const std::string sdTerrainDir = simCore::getEnvVar("SIMDIS_TERRAIN");
    if (!sdTerrainDir.empty())
    {
      const std::string filePath = simCore::backslashToFrontslash(sdTerrainDir);
      const std::string tempFileName = filePath + "/" + fileName;
      if (osgDB::fileExists(tempFileName))
      {
        fileName = tempFileName;
        return 0;
      }
    }
  }
  return 1;
}

int DbConfigurationFile::parseCommonTokens_(const std::vector<std::string>& tokens, int currentLineNumber)
{
  if (tokens.empty())
    return 1;

  // checks for a config file version line
  if (simCore::caseCompare(tokens[0], version_keyword) == 0)
  {
    if (tokens.size() < 2)
    {
      SIM_ERROR << "Line (" << currentLineNumber << ") will be skipped.  Not enough tokens.\n";
      return 1;
    }
    return 0;
  }
  // make sure they all have the 'sphere' keyword at the front
  if (simCore::caseCompare(tokens[0], sphere_keyword) != 0)
  {
    SIM_ERROR << "Line (" << currentLineNumber << ") contains an unrecognized token (" << tokens[0] << ").\n";
    return 1;
  }

  // skip if too few tokens on the line
  if (tokens.size() < 4)
  {
    SIM_ERROR << "Line (" << currentLineNumber << ") will be skipped.  Not enough tokens.\n";
    return 1;
  }
  return 0;
}

unsigned int DbConfigurationFile::getOsgEarthLevel_(unsigned int qsLevel)
{
  if (qsLevel > 0)
  {
    qsLevel += 2;
    if (qsLevel > 32)
      qsLevel = 32;
  }
  return qsLevel;
}


void DbConfigurationFile::parseLayers_(const std::vector<std::string>& tokens, osgEarth::Map* map, int& imageCount, int& elevationCount, const std::string& filePath)
{
  // handle EarthSurface layers
  if (tokens[1] == earth_surface_keyword)
  {
    bool textureSet = false;
    bool altitudeSet = false;
    if ((tokens[2] == texture_set_keyword) ||
      (tokens[2] == texture_set_timestamped_keyword))
      textureSet = true;
    else if ((tokens[2] == altitude_set_keyword) ||
      (tokens[2] == altitude_set_timestamped_keyword))
      altitudeSet = true;

    if (textureSet || altitudeSet)
    {
      // handle the db file, currently the only format this parser supports
      const std::string fullDbFileName = DbConfigurationFile::getDbFile_(tokens, filePath);
      if (!fullDbFileName.empty())
      {
        double opacity = 1.0; // default to max opacity
        bool active = true; // default to active

        // check if it is an ocean surface layer

        // TODO: Implement fadeShallow, fadeDeep, shallowestLevel, deepestLevel
        // For fading, see SDK-46:
        // #include <osgEarth/Util/LODBlending>
        // ...
        // osgEarth::Util::LODBlending* blending = new osgEarth::Util::LODBlending();
        // viewer->getSceneManager()->getMapNode()->getTerrainEngine()->addEffect( blending );


        const std::string oceanSurface = DbConfigurationFile::getTokenValue_(tokens, 4, ocean_surface_keyword);
        if (!oceanSurface.empty())
        {
          // for now, don't handle ocean surface, or move to bottom eventually?
          return;
        }
        // look for transparency first, then opacity
        std::string transparencyStr = DbConfigurationFile::findTokenValue_(tokens, transparency_keyword);
        if (transparencyStr.empty())
          transparencyStr = DbConfigurationFile::findTokenValue_(tokens, opacity_keyword);
        if (!transparencyStr.empty())
        {
          std::istringstream iStr(transparencyStr);
          iStr >> opacity;
        }

        const std::string activeStr = DbConfigurationFile::findTokenValue_(tokens, active_keyword);
        if (!activeStr.empty())
        {
          std::istringstream iStr(activeStr);
          iStr >> active;
        }

        // use file name for layer name
        const std::string layerName = osgDB::getStrippedName(fullDbFileName);

        if (textureSet)
        {
          // Pull out the deepest level.  It is not the same as maximum level from the layer options.
          const std::string deepestLevelStr = DbConfigurationFile::findTokenValue_(tokens, deepest_keyword);
          unsigned int deepestLevel;
          bool deepestLevelValid =
            (!deepestLevelStr.empty() && simCore::isValidNumber(deepestLevelStr, deepestLevel));

          // process min and max level
          const std::string shallowLevelStr = DbConfigurationFile::findTokenValue_(tokens, shallowest_keyword);
          unsigned int shallowestLevel;
          bool minLevelValid = (!shallowLevelStr.empty() && simCore::isValidNumber(shallowLevelStr, shallowestLevel));

          simVis::DBImageLayer* imageLayer = new simVis::DBImageLayer();
          imageLayer->setURL(fullDbFileName);
          if (deepestLevelValid)
            imageLayer->setDeepestLevel(deepestLevel);
          if (minLevelValid)
            imageLayer->setMinLevel(getOsgEarthLevel_(shallowestLevel));

          imageLayer->setOpacity(opacity);
          imageLayer->setVisible(active);
          imageLayer->setEnabled(active);
          imageLayer->setName(layerName);
          map->addLayer(imageLayer);
        }
        if (altitudeSet)
        {
          simVis::DBElevationLayer* newLayer = new simVis::DBElevationLayer();
          newLayer->setURL(fullDbFileName);

          // add a no data value to elevation layers, default is 0
          float noDataValue = 0.0f;
          std::string noDataValueStr = DbConfigurationFile::findTokenValue_(tokens, noDataValue_keyword);
          if (!noDataValueStr.empty())
          {
            std::istringstream iStr(noDataValueStr);
            iStr >> noDataValue;
          }
          newLayer->setNoDataValue(noDataValue);
          newLayer->setName(layerName);
        }
      }
    }
  } // end if earth_surface_keyword
  // handle the cloud layers
  else if (tokens[1] == cloud_layer_keyword)
  {
    DbConfigurationFile::parseCloudLayers_(tokens, map, imageCount, elevationCount, filePath);
  }
}

void DbConfigurationFile::parseCloudLayers_(const std::vector<std::string>& tokens, osgEarth::Map* map, int& imageCount, int& elevationCount, const std::string& filePath)
{
  // ignoring all but the textureSet line
  if ((tokens[2] == texture_set_keyword) ||
    (tokens[2] == texture_set_timestamped_keyword))
  {
    const std::string fullDbFileName = DbConfigurationFile::getDbFile_(tokens, filePath);
    if (!fullDbFileName.empty())
    {
      simVis::DBImageLayer* imageLayer = new simVis::DBImageLayer();
      imageLayer->setURL(fullDbFileName);
      const std::string layerName = osgDB::getStrippedName(fullDbFileName);
      imageLayer->setName(layerName);

      imageLayer->setVisible(false);
      imageLayer->setEnabled(false);
      map->addLayer(imageLayer);

      // process the cloud processing thresholds
      const std::string opaqueStr = DbConfigurationFile::findTokenValue_(tokens, cloud_opaque_keyword);
      float opaqueVal = 1.0f;
      if (!opaqueStr.empty() && simCore::isValidNumber(opaqueStr, opaqueVal))
        opaqueVal = osg::clampBetween(opaqueVal/255.0f, 0.0f, 1.0f);

      const std::string clearStr = DbConfigurationFile::findTokenValue_(tokens, cloud_clear_keyword);
      float clearVal = 0.0f;
      if (!clearStr.empty() && simCore::isValidNumber(clearStr, clearVal))
        clearVal = osg::clampBetween(clearVal/255.0f, 0.0f, 1.0f);

      // only add the AlphaColorFilter if necessary
      if (clearVal < opaqueVal && (opaqueVal != 1.0f || clearVal != 0.0f) && simVis::AlphaColorFilter::isSupported())
      {
        simVis::AlphaColorFilter* filter = new simVis::AlphaColorFilter();
        imageLayer->addColorFilter(filter);
        osg::Vec2f values(clearVal, opaqueVal);
        filter->setAlphaOffset(values);
      }
      else
        imageLayer->setOpacity(opaqueVal);
    }
  }
}

std::string DbConfigurationFile::getDbFile_(const std::vector<std::string>& tokens, const std::string& filePath)
{
  std::string value = DbConfigurationFile::getTokenValue_(tokens, 3, dbFile_keyword);
  if (!value.empty())
  {
    // get the file name and add the full path to it, since standard is to just have the file name relative to the config file
    DbConfigurationFile::processToken(value);
    const std::string dbFilename = value;
    std::string fullDbFileName = dbFilename;
    // first, check to see if the full path name was in the token
    if (!osgDB::fileExists(fullDbFileName))
      fullDbFileName = filePath + dbFilename;

    // Need to use getRealPath to expand relative paths here, else certain relative
    // paths (e.g. cwd) will load, but will fail to save out in osgEarth.
    fullDbFileName = osgDB::getRealPath(fullDbFileName);

    // safety check here to make sure file is valid
    if (osgDB::fileType(fullDbFileName.c_str()) == osgDB::REGULAR_FILE)
      return fullDbFileName;
    else
    {
      // Print out the original text (easier for end user)
      SIM_ERROR << "Could not load referenced file: " << dbFilename << "\n";
    }
  }
  return std::string();
}

std::string DbConfigurationFile::getTokenValue_(const std::vector<std::string>& tokens, int index, const std::string& keyword)
{
  if (tokens.size() > static_cast<size_t>(index) && tokens[index].substr(0, tokens[index].find("=")) == keyword)
  {
    return simCore::StringUtils::after(tokens[index], '=');
  }
  return std::string();
}

std::string DbConfigurationFile::findTokenValue_(const std::vector<std::string>& tokens, const std::string& keyword)
{
  for (std::vector<std::string>::const_iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
  {
    if ((*iter).substr(0, (*iter).find("=")) == keyword)
      return simCore::StringUtils::after(*iter, '=');
  }
  return std::string();
}

osg::Node* DbConfigurationFile::readEarthFile(std::istream& istream, const std::string& relativeTo)
{
  osg::ref_ptr<osgDB::ReaderWriter> readWrite = osgDB::Registry::instance()->getReaderWriterForExtension("earth");
  if (!readWrite.valid())
    return NULL;

  osg::ref_ptr<osgDB::Options> dbOptions = new osgDB::Options();
  dbOptions->setDatabasePath(relativeTo);
  dbOptions->setPluginStringData("osgEarth::URIContext::referrer", relativeTo);

  osgDB::ReaderWriter::ReadResult result = readWrite->readNode(istream, dbOptions.get());

  if (result.success())
  {
    osgEarth::MapNode* mapNode = osgEarth::MapNode::get(result.getNode());
    if (mapNode)
    {
      simVis::SceneManager::initializeTerrainOptions(mapNode);
    }
  }
  return result.takeNode();
}

osg::Node* DbConfigurationFile::readEarthFile(const std::string& filename)
{
  std::fstream istream(filename.c_str(), std::ios::in);
  if (!istream)
    return NULL;
  return DbConfigurationFile::readEarthFile(istream, filename);
}

int DbConfigurationFile::appendEarthFile(std::istream& istream, const std::string& relativeTo, osgEarth::Map& toMap)
{
  osg::ref_ptr<osgDB::ReaderWriter> readWrite = osgDB::Registry::instance()->getReaderWriterForExtension("earth");
  if (!readWrite.valid())
    return 1;

  osg::ref_ptr<osgDB::Options> dbOptions = new osgDB::Options();
  dbOptions->setDatabasePath(relativeTo);
  dbOptions->setPluginStringData("osgEarth::URIContext::referrer", relativeTo);
  osgDB::ReaderWriter::ReadResult result = readWrite->readNode(istream, dbOptions.get());

  if (!result.success())
    return 1;

  osgEarth::MapNode* mapNode = osgEarth::MapNode::get(result.getNode());
  if (!mapNode)
    return 1;
  osgEarth::Map* map = mapNode->getMap();
  if (!map)
    return 1;

  std::vector<osg::ref_ptr<osgEarth::Layer> > layers;
  map->getLayers(layers);
  for (auto iter = layers.begin(); iter != layers.end(); ++iter)
    toMap.addLayer((*iter).get());
  return 0;
}

int DbConfigurationFile::appendEarthFile(const std::string& filename, osgEarth::Map& toMap)
{
  std::fstream ifs(filename.c_str(), std::ios::in);
  if (!ifs)
    return 1;
  return DbConfigurationFile::appendEarthFile(ifs, filename, toMap);
}

}
