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
#ifndef SIMDATA_DATASTOREHELPERS_H
#define SIMDATA_DATASTOREHELPERS_H

#include <string>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"

namespace simData {

/** Methods for getting entity information given an entity ID */
class SDKDATA_EXPORT DataStoreHelpers
{
public:
  /// Get the name of the entity given by objectId
  static std::string nameFromId(const ObjectId& objectId, const simData::DataStore* dataStore);
  /// Get the alias of the entity given by objectId
  static std::string aliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore);
  /// Get the name or alias based on the entity's preference for the given objectId
  static std::string nameOrAliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore, bool allowBlankAlias=false);
  /// Get one character (PBGLD) for the given entity type
  static std::string typeToString(simData::DataStore::ObjectType entityType);
  /// Get the object type from the given entity type character (PBGLD)
  static simData::DataStore::ObjectType typeFromChar(char entityTypeChar);
  /// Get one character (PBGLD) for the type of the entity given by objectId
  static std::string typeFromId(ObjectId objectId, const simData::DataStore* dataStore);
  /// Get a user-friendly name for the given entity type
  static std::string fullTypeToString(simData::DataStore::ObjectType entityType);
  /// Get the user-friendly type name of the entity given by objectId
  static std::string fullTypeFromId(ObjectId objectId, const simData::DataStore* dataStore);
  /// Get the original id of the entity given by object id
  static uint64_t originalIdFromId(ObjectId objectId, const simData::DataStore* dataStore);
  /**
   * Get the first existing object id if one exists. Warning: object names are NOT unique,
   * this function ONLY returns the first id available. Returns 0 on error.
   */
  static ObjectId idByName(const std::string& objectName, const simData::DataStore* dataStore);
  /// Get the Unique ID of the host platform; will return itself if a platform; return 0 on error
  static ObjectId getPlatformHostId(ObjectId objectId, const simData::DataStore* dataStore);
  /// Get the scenario source description
  static std::string description(const simData::DataStore* dataStore);
  /// Adds a media file if not already in the dataStore, the argument fileName must be full path. Returns zero on success.
  static int addMediaFile(const std::string& fileName, simData::DataStore* dataStore);

  /// create a protobuf message for the prefs of the given entity type
  static google::protobuf::Message* makeMessage(simData::DataStore::ObjectType entityType);
};

}

#endif /* SIMDATA_DATASTOREHELPERS_H */
