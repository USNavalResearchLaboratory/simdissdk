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
#include <cassert>
#include "simCore/String/Format.h"
#include "simData/DataStore.h"
#include "simData/DataStoreHelpers.h"

namespace simData {

std::string DataStoreHelpers::nameFromId(const ObjectId& objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return "";
  simData::DataStore::Transaction transaction;
  const simData::CommonPrefs* prefs = dataStore->commonPrefs(objectId, &transaction);
  return prefs ? prefs->name() : "";
}

std::string DataStoreHelpers::aliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return "";
  simData::DataStore::Transaction transaction;
  const simData::CommonPrefs* prefs = dataStore->commonPrefs(objectId, &transaction);
  return prefs ? prefs->alias() : "";
}

std::string DataStoreHelpers::nameOrAliasFromId(const ObjectId& objectId, const simData::DataStore* dataStore, bool allowBlankAlias)
{
  if (dataStore == NULL)
    return "";
  simData::DataStore::Transaction transaction;
  const simData::CommonPrefs* prefs = dataStore->commonPrefs(objectId, &transaction);

  if (prefs == NULL)
    return "";

  if (prefs->usealias())
  {
    if (!prefs->alias().empty() || allowBlankAlias)
      return prefs->alias();
  }

  return prefs->name();
}

simData::ObjectType DataStoreHelpers::typeFromChar(char entityTypeChar)
{
  switch (entityTypeChar)
  {
  case 'P':
  case 'p': return simData::PLATFORM;

  case 'B':
  case 'b': return simData::BEAM;

  case 'G':
  case 'g': return simData::GATE;

  case 'L':
  case 'l': return simData::LASER;

  case 'D':
  case 'd': return simData::LOB_GROUP;

  case 'R':
  case 'r': return simData::PROJECTOR;

  case simData::ALL:
  case simData::NONE:
    return simData::NONE;
  }

  return simData::NONE;
}

std::string DataStoreHelpers::typeToString(simData::ObjectType entityType)
{
  switch (entityType)
  {
  case simData::PLATFORM:
    return "P";
  case simData::BEAM:
    return "B";
  case simData::GATE:
    return "G";
  case simData::LASER:
    return "L";
  case simData::LOB_GROUP:
    return "D";
  case simData::PROJECTOR:
    return "R";
  case simData::ALL:
  case simData::NONE:
    return "";
  }
  assert(0);
  return "";
}

std::string DataStoreHelpers::typeFromId(ObjectId objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return "";
  return typeToString(dataStore->objectType(objectId));
}

std::string DataStoreHelpers::fullTypeToString(simData::ObjectType entityType)
{
  switch (entityType)
  {
  case simData::PLATFORM:
    return "Platform";
  case simData::BEAM:
    return "Beam";
  case simData::GATE:
    return "Gate";
  case simData::LASER:
    return "Laser";
  case simData::LOB_GROUP:
    return "LOB";
  case simData::PROJECTOR:
    return "Projector";
  case simData::ALL:
  case simData::NONE:
    return "";
  }
  assert(0);
  return "";
}

std::string DataStoreHelpers::fullTypeFromId(ObjectId objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return "";
  simData::DataStore::Transaction transaction;
  return fullTypeToString(dataStore->objectType(objectId));
}

uint64_t DataStoreHelpers::originalIdFromId(ObjectId objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return 0;
  simData::ObjectType objType = dataStore->objectType(objectId);
  simData::DataStore::Transaction transaction;
  switch (objType)
  {
  case simData::PLATFORM:
  {
    const simData::PlatformProperties* props = dataStore->platformProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::BEAM:
  {
    const simData::BeamProperties* props = dataStore->beamProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::GATE:
  {
    const simData::GateProperties* props = dataStore->gateProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::LASER:
  {
    const simData::LaserProperties* props = dataStore->laserProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::PROJECTOR:
  {
    const simData::ProjectorProperties* props = dataStore->projectorProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::LOB_GROUP:
  {
    const simData::LobGroupProperties* props = dataStore->lobGroupProperties(objectId, &transaction);
    assert(props);
    return props->originalid();
  }
  case simData::NONE:
  case simData::ALL:
    break;
  }

  return 0;
}

ObjectId DataStoreHelpers::idByName(const std::string& objectName, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return 0;
  simData::DataStore::IdList ids;
  dataStore->idListByName(objectName, &ids);
  if (ids.empty())
    return 0;
  return ids[0];
}

ObjectId DataStoreHelpers::getPlatformHostId(ObjectId objectId, const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return 0;

  while (dataStore->objectType(objectId) != simData::PLATFORM)
  {
    // Return an error code if an invalid entity id is encountered
    if (dataStore->objectType(objectId) == simData::NONE)
      return 0;
    objectId = dataStore->entityHostId(objectId);
  }

  return objectId;
}

std::string DataStoreHelpers::description(const simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return 0;
  simData::DataStore::Transaction transaction;
  return dataStore->scenarioProperties(&transaction)->description();
}

google::protobuf::Message* DataStoreHelpers::makeMessage(simData::ObjectType entityType)
{
  switch (entityType)
  {
  case simData::NONE:
    break; // should not see this case
  case simData::ALL: // All is used for common prefs
    return new simData::CommonPrefs();
  case simData::PLATFORM:
    return new simData::PlatformPrefs();
  case simData::BEAM:
    return new simData::BeamPrefs();
  case simData::GATE:
    return new simData::GatePrefs();
  case simData::LASER:
    return new simData::LaserPrefs();
  case simData::LOB_GROUP:
    return new simData::LobGroupPrefs();
  case simData::PROJECTOR:
    return new simData::ProjectorPrefs();
  }

  assert(false);
  return NULL;
}

int DataStoreHelpers::addMediaFile(const std::string& fileName, simData::DataStore* dataStore)
{
  if (dataStore == NULL)
    return 1;

  if (fileName.empty())
    return 1;

  simData::DataStore::Transaction transaction;
  simData::ScenarioProperties* props = dataStore->mutable_scenarioProperties(&transaction);
  if (props != NULL)
  {
    // Prevent duplicates.
    for (int ii = 0; ii < props->mediafile_size(); ++ii)
    {
#ifdef WIN32
      if (simCore::caseCompare(props->mediafile(ii), fileName) == 0)
        return 1;
#else
      if (props->mediafile(ii) == fileName)
        return 1;
#endif
    }
    props->add_mediafile(fileName);
    transaction.commit();

    return 0;
  }

  return 1;
}

simData::DataTable* DataStoreHelpers::getOrCreateDataTable(ObjectId objectId, const std::string& tableName, simData::DataStore* dataStore)
{
  if ((dataStore->objectType(objectId) == simData::NONE) || tableName.empty() || (dataStore == NULL))
    return NULL;

  simData::DataTableManager& tableManager = dataStore->dataTableManager();
  simData::DataTable* table = tableManager.findTable(objectId, tableName);

  // if failed to find the table, create the table.
  if (table == NULL)
  {
    simData::TableStatus status = tableManager.addDataTable(objectId, tableName, &table);
    if (status.isError())
      return NULL;
  }

  return table;
}


int DataStoreHelpers::getOrCreateColumn(simData::DataTable* table, const std::string& columnName, VariableType storageType, UnitType unitType, simData::DataStore* dataStore, simData::TableColumnId& id)
{
  if ((table == NULL) || columnName.empty() || (dataStore == NULL))
    return 1;

  simData::TableColumn* column = table->column(columnName);
  if (column)
  {
    id = column->columnId();
    return 0;
  }

  // if failed to find the column, create the column
  simData::TableColumn* newColumn = NULL;
  if (table->addColumn(columnName, storageType, unitType, &newColumn).isError())
    return 1;

  id = newColumn->columnId();
  return 0;
}

namespace {

  /** Helper method to determine if a platform is active */
  bool isPlatformActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
  {
    if (dataStore.dataLimiting())
    {
      simData::DataStore::Transaction txn;
      const simData::CommonPrefs* prefs = dataStore.commonPrefs(objectId, &txn);
      if (prefs != NULL)
      {
        return prefs->datadraw();
      }

      return true;
    }

    const simData::PlatformUpdateSlice* slice = dataStore.platformUpdateSlice(objectId);
    if (slice == NULL)
      return false;

    // static platforms are always active
    if (slice->firstTime() == -1.0)
      return true;

    if ((slice->firstTime() > atTime) || (slice->lastTime() < atTime))
      return false;

    return true;
  }

  /** Helper method to determine if a beam is active */
  bool isBeamActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::BeamProperties* beamProperty = dataStore.beamProperties(objectId, &propertyTrans);
    if (!isPlatformActive(dataStore, beamProperty->hostid(), atTime))
      return false;

    const simData::BeamCommandSlice* slice = dataStore.beamCommandSlice(objectId);
    if (slice == NULL)
      return false;

    // Check the draw state
    bool rv = false;
    simData::BeamCommandSlice::Iterator iter = slice->upper_bound(atTime);
    while (iter.hasPrevious())
    {
      const simData::BeamCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        rv = command->updateprefs().commonprefs().datadraw();
        break;
      }
    }

    // If false can return now
    if (!rv)
      return false;

    // Active depends on Beam Type
    if (beamProperty->type() != simData::BeamProperties::TARGET)
      return rv;

    // Verify that the target beam has a target and that the target is active
    iter = slice->upper_bound(atTime);
    while (iter.hasPrevious())
    {
      const simData::BeamCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().has_targetid())
      {
        // Verify that the target platform exists
        return isPlatformActive(dataStore, command->updateprefs().targetid(), atTime);
      }
    }

    // no previous target command exists
    return false;
  }

  /** Helper method to determine if a gate is active */
  bool isGateActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::GateProperties* gateProperty = dataStore.gateProperties(objectId, &propertyTrans);
    if (!isBeamActive(dataStore, gateProperty->hostid(), atTime))
      return false;

    const simData::GateCommandSlice* slice = dataStore.gateCommandSlice(objectId);
    if (slice == NULL)
      return false;

    // Check the draw state
    simData::GateCommandSlice::Iterator iter = slice->upper_bound(atTime);
    while (iter.hasPrevious())
    {
      const simData::GateCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        return command->updateprefs().commonprefs().datadraw();
      }
    }

    // no previous data draw command exists
    return false;
  }

  /** Helper method to determine if a laser is active */
  bool isLaserActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::LaserProperties* laserProperty = dataStore.laserProperties(objectId, &propertyTrans);
    if (!isPlatformActive(dataStore, laserProperty->hostid(), atTime))
      return false;

    const simData::LaserCommandSlice* slice = dataStore.laserCommandSlice(objectId);
    if (slice == NULL)
      return false;

    // Check the draw state
    simData::LaserCommandSlice::Iterator iter = slice->upper_bound(atTime);
    while (iter.hasPrevious())
    {
      const simData::LaserCommand* command = iter.previous();
      if (command->has_time() && command->updateprefs().commonprefs().has_datadraw())
      {
        return command->updateprefs().commonprefs().datadraw();
      }
    }

    // no previous data draw command exists
    return false;
  }

  /** Helper method to determine if a LOB Group is active */
  bool isLobGroupActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
  {
    // Host must be active
    simData::DataStore::Transaction propertyTrans;
    const simData::LobGroupProperties* lobProperty = dataStore.lobGroupProperties(objectId, &propertyTrans);
    if (!isPlatformActive(dataStore, lobProperty->hostid(), atTime))
      return false;

    // LOB do NOT have datadraw command; LOBs are always on
    return true;
  }
}

bool DataStoreHelpers::isEntityActive(const simData::DataStore& dataStore, simData::ObjectId objectId, double atTime)
{
  const simData::ObjectType type = dataStore.objectType(objectId);
  switch (type)
  {
  case simData::PLATFORM:
    return isPlatformActive(dataStore, objectId, atTime);

  case simData::BEAM:
    return isBeamActive(dataStore, objectId, atTime);

  case simData::GATE:
    return isGateActive(dataStore, objectId, atTime);

  case simData::LASER:
    return isLaserActive(dataStore, objectId, atTime);

  case simData::LOB_GROUP:
    return isLobGroupActive(dataStore, objectId, atTime);

  case simData::PROJECTOR:
    return true;

  case simData::NONE:
    // Entity does not exist
    break;

  default:
    // The switch statement needs to be updated
    assert(false);
    break;
  }
  return false;
}

}
