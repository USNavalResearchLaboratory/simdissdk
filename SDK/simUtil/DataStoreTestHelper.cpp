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
#include "simCore/Common/SDKAssert.h"
#include "simData/DataTable.h"
#include "simUtil/DefaultDataStoreValues.h"
#include "simUtil/DataStoreTestHelper.h"

namespace simUtil
{

DataStoreTestHelper::DataStoreTestHelper(simData::DataStore* dataStore)
  : dataStore_(dataStore),
    ownDataStore_(false),
    tableId_(0)
{
  if (!dataStore_)
  {
    dataStore_ = new simData::MemoryDataStore();
    simUtil::DefaultEntityPrefs::initializeDataStorePrefs(*dataStore_);
    ownDataStore_ = true;
  }
}

DataStoreTestHelper::~DataStoreTestHelper()
{
  if (ownDataStore_)
    delete dataStore_;
}

simData::DataStore* DataStoreTestHelper::dataStore()
{
  return dataStore_;
}

uint64_t DataStoreTestHelper::addPlatform()
{
  simData::DataStore::Transaction t;
  simData::PlatformProperties *p = dataStore_->addPlatform(&t);
  t.commit();
  simData::PlatformPrefs *pp = dataStore_->mutable_platformPrefs(p->id(), &t);
  std::ostringstream platName;
  platName << "platform" << p->id();
  pp->mutable_commonprefs()->set_name(platName.str());
  pp->set_icon("icon1");
  t.commit();
  return p->id();
}

uint64_t DataStoreTestHelper::addBeam(uint64_t hostId)
{
  simData::DataStore::Transaction t;
  simData::BeamProperties* beamProps = dataStore_->addBeam(&t);
  beamProps->set_hostid(hostId);
  t.commit();
  simData::BeamPrefs* beamPrefs = dataStore_->mutable_beamPrefs(beamProps->id(), &t);
  std::ostringstream beamName;
  beamName << "beam" << beamProps->id() << "_" << hostId;
  beamPrefs->mutable_commonprefs()->set_name(beamName.str());
  t.commit();
  return beamProps->id();
}

uint64_t DataStoreTestHelper::addGate(uint64_t hostId)
{
  simData::DataStore::Transaction t;
  simData::GateProperties* gateProps = dataStore_->addGate(&t);
  gateProps->set_hostid(hostId);
  t.commit();
  simData::GatePrefs* gatePrefs = dataStore_->mutable_gatePrefs(gateProps->id(), &t);
  std::ostringstream gateName;
  gateName << "gate" << gateProps->id() << "_" << hostId;
  gatePrefs->mutable_commonprefs()->set_name(gateName.str());
  t.commit();
  return gateProps->id();
}

uint64_t DataStoreTestHelper::addLaser(uint64_t hostId)
{
  simData::DataStore::Transaction t;
  simData::LaserProperties* laserProps = dataStore_->addLaser(&t);
  laserProps->set_hostid(hostId);
  t.commit();
  simData::LaserPrefs* laserPrefs = dataStore_->mutable_laserPrefs(laserProps->id(), &t);
  std::ostringstream laserName;
  laserName << "laser" << laserProps->id() << "_" << hostId;
  laserPrefs->mutable_commonprefs()->set_name(laserName.str());
  t.commit();
  return laserProps->id();
}

uint64_t DataStoreTestHelper::addLOB(uint64_t hostId)
{
  simData::DataStore::Transaction t;
  simData::LobGroupProperties* lobProps = dataStore_->addLobGroup(&t);
  lobProps->set_hostid(hostId);
  t.commit();
  simData::LobGroupPrefs* lobPrefs = dataStore_->mutable_lobGroupPrefs(lobProps->id(), &t);
  std::ostringstream lobName;
  lobName << "lob" << lobProps->id() << "_" << hostId;
  lobPrefs->mutable_commonprefs()->set_name(lobName.str());
  t.commit();
  return lobProps->id();
}

uint64_t DataStoreTestHelper::addProjector(uint64_t hostId)
{
  simData::DataStore::Transaction t;
  simData::ProjectorProperties* projProps = dataStore_->addProjector(&t);
  projProps->set_hostid(hostId);
  t.commit();
  simData::ProjectorPrefs* projPrefs = dataStore_->mutable_projectorPrefs(projProps->id(), &t);
  std::ostringstream projName;
  projName << "projector" << projProps->id() << "_" << hostId;
  projPrefs->mutable_commonprefs()->set_name(projName.str());
  t.commit();
  return projProps->id();
}

void DataStoreTestHelper::updatePlatformPrefs(const simData::PlatformPrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::PlatformPrefs* p = dataStore_->mutable_platformPrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::updateBeamPrefs(const simData::BeamPrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::BeamPrefs* p = dataStore_->mutable_beamPrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::updateGatePrefs(const simData::GatePrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::GatePrefs* p = dataStore_->mutable_gatePrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::updateLaserPrefs(const simData::LaserPrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LaserPrefs* p = dataStore_->mutable_laserPrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::updateLOBPrefs(const simData::LobGroupPrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LobGroupPrefs* p = dataStore_->mutable_lobGroupPrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::updateProjectorPrefs(const simData::ProjectorPrefs& prefs, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::ProjectorPrefs* p = dataStore_->mutable_projectorPrefs(id, &t);
  SDK_ASSERT(p != NULL);
  p->MergeFrom(prefs);
  t.commit();
}

void DataStoreTestHelper::addPlatformUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::PlatformUpdate *u = dataStore_->addPlatformUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  u->set_x(0.0 + time);
  u->set_y(1.0 + time);
  u->set_z(2.0 + time);
  t.commit();
}

void DataStoreTestHelper::addBeamUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::BeamUpdate *u = dataStore_->addBeamUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  u->set_azimuth(0.0 + time);
  u->set_elevation(1.0 + time);
  u->set_range(2.0 + time);
  t.commit();
}

void DataStoreTestHelper::addGateUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::GateUpdate *u = dataStore_->addGateUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  u->set_azimuth(0.0 + time);
  u->set_elevation(1.0 + time);
  u->set_width(2.0 + time);
  t.commit();
}

void DataStoreTestHelper::addLaserUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LaserUpdate *u = dataStore_->addLaserUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  u->mutable_orientation()->set_yaw(0.0 + time);
  u->mutable_orientation()->set_pitch(1.0 + time);
  u->mutable_orientation()->set_roll(2.0 + time);
  t.commit();
}

void DataStoreTestHelper::addLOBUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LobGroupUpdate *u = dataStore_->addLobGroupUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  simData::LobGroupUpdatePoint *up = u->mutable_datapoints()->Add();
  up->set_time(time);
  up->set_azimuth(1.0 + time);
  up->set_elevation(10.0 + time);
  up->set_range(1000.0);
  simData::LobGroupUpdatePoint *up2 = u->mutable_datapoints()->Add();
  up2->set_time(time);
  up2->set_azimuth(20.0 + time);
  up2->set_elevation(5.0 + time);
  up2->set_range(1000.0);
  t.commit();
}

void DataStoreTestHelper::addProjectorUpdate(double time, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::ProjectorUpdate *u = dataStore_->addProjectorUpdate(id, &t);
  SDK_ASSERT(u != NULL);
  u->set_time(time);
  u->set_fov(1.0 + time);
  t.commit();
}

void DataStoreTestHelper::addPlatformCommand(const simData::PlatformCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::PlatformCommand *c = dataStore_->addPlatformCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addBeamCommand(const simData::BeamCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::BeamCommand *c = dataStore_->addBeamCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addGateCommand(const simData::GateCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::GateCommand *c = dataStore_->addGateCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addLaserCommand(const simData::LaserCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LaserCommand *c = dataStore_->addLaserCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addLOBCommand(const simData::LobGroupCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::LobGroupCommand *c = dataStore_->addLobGroupCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addProjectorCommand(const simData::ProjectorCommand& command, uint64_t id)
{
  simData::DataStore::Transaction t;
  simData::ProjectorCommand *c = dataStore_->addProjectorCommand(id, &t);
  SDK_ASSERT(c != NULL);
  c->MergeFrom(command);
  t.commit();
}

void DataStoreTestHelper::addCategoryData(uint64_t id, const std::string& key, const std::string& value, double startTime)
{
  simData::DataStore::Transaction transaction;
  simData::CategoryData* catData = dataStore_->addCategoryData(id, &transaction);
  SDK_ASSERT(catData != NULL);
  if (catData == NULL)
    return;

  catData->set_time(startTime);

  simData::CategoryData_Entry* entry = catData->add_entry();
  entry->set_key(key);
  entry->set_value(value);

  transaction.complete(&catData);
}

void DataStoreTestHelper::addGenericData(uint64_t id, const std::string& key, const std::string& value, double startTime, bool ignoreDuplicates)
{
  simData::DataStore::Transaction transaction;
  simData::GenericData* genData = dataStore_->addGenericData(id, &transaction);
  SDK_ASSERT(genData != NULL);
  if (genData == NULL)
    return;

  genData->set_time(static_cast<double>(startTime));
  genData->set_duration(-1);

  simData::GenericData_Entry* entry = genData->mutable_entry()->Add();
  entry->set_key(key);
  entry->set_value(value);

  transaction.complete(&genData);
}

void DataStoreTestHelper::addDataTable(uint64_t entityId, int numRows, const std::string& tableName)
{
  std::string name = tableName;
  // auto generate a unique name for the table if no name passed in
  if (tableName.empty())
  {
    ++tableId_;
    std::ostringstream os;
    os << "DataTable" << tableId_;
    name = os.str();
  }
  simData::DataTable* newTable;
  if (dataStore_->dataTableManager().addDataTable(entityId, name, &newTable).isError())
    return;
  addDataTableRows_(newTable, numRows, tableId_);
}

void DataStoreTestHelper::addDataTableRows_(simData::DataTable* table, int numRows, uint64_t id)
{
  // now add 4 columns
  simData::TableColumn* column1;
  simData::TableColumn* column2;
  simData::TableColumn* column3;
  simData::TableColumn* column4;
  for (int i = 0; i < 4; i++)
  {
    std::ostringstream os;
    os << "Col" << i;
    // create different storage types
    switch (i)
    {
    case 0:
      table->addColumn(os.str(), simData::VT_INT16, 0, &column1);
      break;
    case 1:
      table->addColumn(os.str(), simData::VT_DOUBLE, 0, &column2);
      break;
    case 2:
      table->addColumn(os.str(), simData::VT_INT32, 0, &column3);
      break;
    case 3:
      table->addColumn(os.str(), simData::VT_INT8, 0, &column4);
      break;
    }
  }

  // now add some rows
  for (int i = 0; i < numRows; i++)
  {
    simData::TableRow row;
    row.setTime(i + 1.0);
    row.setValue(column1->columnId(), 345);
    row.setValue(column2->columnId(), 685454);
    if (id % 2)
      row.setValue(column3->columnId(), 458685);
    if (id % 3)
      row.setValue(column4->columnId(), 45);
    table->addRow(row);
  }

}

}
