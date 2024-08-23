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
#ifndef SIMDATA_DATASTOREHELPERS_H
#define SIMDATA_DATASTOREHELPERS_H

#include <optional>
#include <string>
#include <utility>
#include "simCore/Common/Common.h"
#include "simData/DataSlice.h"
#include "simData/DataTable.h"
#include "simData/DataTypes.h"
#include "simData/ObjectId.h"

namespace simData {

class DataStore;

/** Methods for getting/setting entity information given an entity ID */
class SDKDATA_EXPORT DataStoreHelpers
{
public:
  /// Get the name of the entity given by objectId
  static std::string nameFromId(const ObjectId& objectId, const simData::DataStore* dataStore);
  /// Get the alias of the entity given by objectId
  static std::string aliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore);
  /// Get the name or alias based on the entity's preference for the given objectId
  static std::string nameOrAliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore, bool allowBlankAlias=false);
  /// Set the name of the entity; returns 0 on success
  static int setName(const std::string& newName, const ObjectId& objectId, simData::DataStore* dataStore);
  /// Get one character (PBGLD) for the given entity type
  static std::string typeToString(simData::ObjectType entityType);
  /// Get the object type from the given entity type character (PBGLD)
  static simData::ObjectType typeFromChar(char entityTypeChar);
  /// Get one character (PBGLD) for the type of the entity given by objectId
  static std::string typeFromId(ObjectId objectId, const simData::DataStore* dataStore);
  /// Get a user-friendly name for the given entity type
  static std::string fullTypeToString(simData::ObjectType entityType);
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
  /// Gets or creates a table for the given object with the given name;  returns nullptr on error
  static simData::DataTable* getOrCreateDataTable(ObjectId objectId, const std::string& tableName, simData::DataStore* dataStore);
  /// Gets or creates a column for the given table with the given name; return 0 on success
  static int getOrCreateColumn(simData::DataTable* table, const std::string& columnName, VariableType storageType, UnitType unitType, simData::DataStore* dataStore, simData::TableColumnId& id);

  /// create a protobuf message for the prefs of the given entity type
  static google::protobuf::Message* makeMessage(simData::ObjectType entityType);

  /** Returns true if the entity is active, or false if inactive; e.g. for Super Form-like filtering. */
  static bool isEntityActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime);
  /** Returns time bounds for platform in file mode based on lifespan mode. Does not inspect data draw nor live mode. */
  static std::optional<std::pair<double, double>> getFileModePlatformTimeBounds(simData::LifespanMode lifespan, const simData::PlatformUpdateSlice& slice);
  /** Inspecting an individual platforms slice, determines whether it is active in file mode. Does not account for data draw nor live mode. */
  static bool isFileModePlatformActive(simData::LifespanMode lifespan, const simData::PlatformUpdateSlice& slice, double atTime);

  /** Returns the user vertical datum value, in meters, for the given entity. */
  static double getUserVerticalDatum(const simData::DataStore& dataStore, simData::ObjectId id);

  /** Replaces contents of repeated field with the contents of the provided vector. */
  template <typename T>
  static void vecToRepeated(typename google::protobuf::RepeatedField<T>* field, const typename std::vector<T>& vec)
  {
    if (!field)
      return;
    field->Clear();
    for (const auto& value : vec)
      field->Add(value);
  }

  /** Converts a protobuf RepeatedField into a std::vector of same type. */
  template <typename T>
  static typename std::vector<T> vecFromRepeated(const typename google::protobuf::RepeatedField<T>& field)
  {
    typename std::vector<T> rv;
    if (!field.empty())
    {
      rv.reserve(field.size());
      for (int k = 0; k < field.size(); ++k)
        rv.emplace_back(field.Get(k));
    }
    return rv;
  }
};

}

#endif /* SIMDATA_DATASTOREHELPERS_H */
