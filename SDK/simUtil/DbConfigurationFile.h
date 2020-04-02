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
#ifndef SIMUTIL_DBCONFIGURATIONFILE_H
#define SIMUTIL_DBCONFIGURATIONFILE_H

#include <vector>
#include <string>
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"

namespace osgEarth
{
  class Map;
  class MapNode;
}
namespace simData { class DataStore; }
namespace simVis  { class SceneManager; }

namespace simUtil {

/** Loads parts of a legacy .txt configuration file (typically consisting of .db files) */
class SDKUTIL_EXPORT DbConfigurationFile
{
public:
  /// keyword for accessing the most recent terrain configuration file in the settings (QSettings)
  static const std::string TERRAIN_CONFIG_FILE_SETTING;

  /**
   * Loads the configuration file (.txt or .earth).  Returns a MapNode that contains
   * the map.  The caller is responsible for memory and for associating the MapNode
   * with a scene (e.g. simVis::Viewer::setMapNode).
   * @param mapNode Reference to hold the new map node.  Set to the newly allocated
   *    MapNode when the file is correctly processed (zero return value)
   * @param filename Path to the configuration file to load; can be a .earth file
   *    or a legacy SIMDIS 9 .txt configuration file.  Environment variables are
   *    expanded (e.g. $(ENV_VAR)).  SIMDIS_TERRAIN is used as a fallback location
   *    to locate the data file, as per resolveFilePath().
   * @param quiet If false, errors and warnings are not reported using SIM_NOTIFY.
   * @return 0 on success.  Non-zero on error.
   */
  static int load(osg::ref_ptr<osgEarth::MapNode>& mapNode, const std::string& filename, bool quiet=false);

  /**
   * Loads a SIMDIS 9 terrain configuration file.  Full path must be passed in.
   * Returns a map representing the configuration file with layers added appropriately.
   * Caller is responsible for creating a MapNode if desired.  This method is a subset
   * of load() that is only able to load legacy SIMDIS 9 files.
   * @param filename Full path to the configuration file to load (e.g. configDefault.txt)
   * @param quiet If false, errors and warnings are not reported using SIM_NOTIFY.
   * @return NULL on inability to load.  Else returns a newly allocated osgEarth::Map.
   *    Remember to wrap the return value in an osg::ref_ptr.
   */
  static osgEarth::Map* loadLegacyConfigFile(const std::string& filename, bool quiet=false);

  /// Removes quotes and expands environment variables
  static void processToken(std::string& token);

  /// resolves the configuration file path using defined rules for finding the file's location (e.g. searching SIMDIS_TERRAIN)
  static int resolveFilePath(std::string& fileName);

  /**
   * Helper method to load a .earth file with a default set of options appropriate
   * for the SIMDIS scene graph.  A new osg::Node is allocated and should be contained
   * in a ref_ptr for memory management.
   * @see osgDB::readNodeFile()
   * @param filename Name of the file resource to load (should end in .earth).  Expected
   *   to be fully adjusted, e.g. using DbConfigurationFile::resolveFilePath().  Will
   *   be passed as-is to osgDB::readNodeFile().
   * @return NULL on error, else newly allocated node from osgDB::readNodeFile() with default earth options
   */
  static osg::Node* readEarthFile(const std::string& filename);

  /**
   * Stream-based version of readEarthFile().  Loads the .earth file from an input stream, using
   * the provided referrer (relativeTo) to help resolve relative paths.
   * @param istream Input stream holding the .earth file contents
   * @param relativeTo Absolute path to a location used to help resolve relative paths.  Sometimes called "referrer",
   *   or the Database Path in OSG parlance.
   * @return NULL on error, else newly allocated node from osgDB::ReaderWriter for earth files
   */
  static osg::Node* readEarthFile(std::istream& istream, const std::string& relativeTo);

  /**
   * Helper method to append a .earth file to an already-existing Map.  Returns 0 on successful
   * read of the earth file.  Layers are appended to the end of the provided map.
   * @param filename Name of .earth file to append to the map
   * @param toMap Map to host the earth files
   * @return 0 on successful load of file, non-zero on error
   */
  static int appendEarthFile(const std::string& filename, osgEarth::Map& toMap);

  /**
   * Stream-based version of appendEarthFile().  Appends the .earth file from an input stream, using
   * the provided referrer (relativeTo) to help resolve relative paths.
   * @param istream Input stream holding the .earth file contents
   * @param relativeTo Absolute path to a location used to help resolve relative paths.  Sometimes called "referrer",
   *   or the Database Path in OSG parlance.
   * @param toMap Map to host the earth files
   * @return 0 on successful load of file, non-zero on error
   */
  static int appendEarthFile(std::istream& istream, const std::string& relativeTo, osgEarth::Map& toMap);

private:
  /// does some error checking on the parsed tokens, returns false on error
  static int parseCommonTokens_(const std::vector<std::string>& tokens, int currentLineNumber);
  /// parse the image and elevation layers
  static void parseLayers_(const std::vector<std::string>& tokens, osgEarth::Map* map, int& imageCount, int& elevationCount, const std::string& filePath);
  /// parse the cloud layers
  static void parseCloudLayers_(const std::vector<std::string>& tokens, osgEarth::Map* map, int& imageCount, int& elevationCount, const std::string& filePath);
  /// pulls out the db file path name
  static std::string getDbFile_(const std::vector<std::string>& tokens, const std::string& filePath);
  /// return the value of the indexed token, as long as the token's keyword matches
  static std::string getTokenValue_(const std::vector<std::string>& tokens, int index, const std::string& keyword);
  /// return the value of the token keyword if it is found anywhere in the tokens vector
  static std::string findTokenValue_(const std::vector<std::string>& tokens, const std::string& keyword);
  /// convert QuadSphere levels to osgEarth levels
  static unsigned int getOsgEarthLevel_(unsigned int QSlevel);
};

}

#endif /* SIMUTIL_DBCONFIGURATIONFILE_H */

