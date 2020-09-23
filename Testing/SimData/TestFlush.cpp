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

#include "simCore/Common/SDKAssert.h"
#include "simData/DataSlice.h"
#include "simData/DataTable.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

/// Test an empty data store
int testEmpty()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();

  auto keepStaticPlatformFlush = static_cast<simData::DataStore::FlushFields>(simData::DataStore::FLUSH_UPDATES | simData::DataStore::FLUSH_EXCLUDE_MINUS_ONE);
  auto historicCategoryFlush = static_cast<simData::DataStore::FlushFields>(simData::DataStore::FLUSH_CATEGORY_DATA | simData::DataStore::FLUSH_EXCLUDE_MINUS_ONE);

  // Flushing an empty scenario should be OK
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, keepStaticPlatformFlush) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_UPDATES) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_COMMANDS) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, historicCategoryFlush) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) == 0);
  rv += SDK_ASSERT(ds->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);

  // Flushing an non-existing entity should fail
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, keepStaticPlatformFlush) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_UPDATES) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_COMMANDS) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, historicCategoryFlush) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) != 0);
  rv += SDK_ASSERT(ds->flush(1, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) != 0);

  return rv;
}

/// Count the number of category entries
class CategoryCounter : public simData::CategoryDataSlice::Visitor
{
public:
  CategoryCounter() : counter_(0) {}
  virtual ~CategoryCounter() {}
  virtual void operator()(const simData::CategoryData *update)
  {
    ++counter_;
  }
  int counter() const { return counter_; }

private:
  int counter_;
};

/// Return the number of category entries
int categoryDataEntries(simData::DataStore* ds, simData::ObjectId id)
{
  auto slice = ds->categoryDataSlice(id);
  if (slice == nullptr)
    return -1;

  CategoryCounter counter;
  slice->visit(&counter);

  return counter.counter();
}

/// Test category data
int testCategoryData(simUtil::DataStoreTestHelper& helper, simData::ObjectId id)
{
  int rv = 0;

  // Should start off empty
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 0);

  helper.addCategoryData(id, "Category", "Value", 0.0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 1);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) == 0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 0);

  helper.addCategoryData(id, "Category", "Value", 0.0);
  helper.addCategoryData(id, "Category", "Value2", 1.0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 2);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) == 0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 0);

  // Historic flush keeps the default entry and the last entry
  auto historicCategoryFlush = static_cast<simData::DataStore::FlushFields>(simData::DataStore::FLUSH_CATEGORY_DATA | simData::DataStore::FLUSH_EXCLUDE_MINUS_ONE);

  helper.addCategoryData(id, "Category", "Value", -1.0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 1);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) == 0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 1);

  helper.addCategoryData(id, "Category", "Value2", 1.0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 2);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) == 0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 2);

  helper.addCategoryData(id, "Category", "Value3", 2.0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 3);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) == 0);
  rv += SDK_ASSERT(categoryDataEntries(helper.dataStore(), id) == 2);

  return rv;
}

// Returns the number of generic data entries
size_t genericDataEntries(simData::DataStore* ds, simData::ObjectId id)
{
  auto slice = ds->genericDataSlice(id);
  if (slice == nullptr)
    return -1;

  return slice->numItems();
}

// Test generic data
int testGenericData(simUtil::DataStoreTestHelper& helper, simData::ObjectId id)
{
  int rv = 0;

  rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), id) == 0);

  helper.addGenericData(id, "Key", "Value", 0.0);
  rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), id) == 1);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) == 0);
  rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), id) == 0);

  helper.addGenericData(id, "Key", "Value", 0.0);
  helper.addGenericData(id, "Key", "Value2", 1.0);
  rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), id) == 2);

  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
  rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), id) == 0);

  return rv;
}

// Counts the number of rows in a data table
class DataTableCounter : public simData::DataTable::RowVisitor
{
public:
  DataTableCounter() : counter_(0) {}
  virtual ~DataTableCounter() {}

  virtual VisitReturn visit(const simData::TableRow& row)
  {
    ++counter_;
    return VISIT_CONTINUE;
  }

  int counter() const { return counter_; }

private:
  int counter_;
};

/// Returns the number of rows in a data table
int dataTableEntries(simData::DataStore* ds, uint64_t tableId)
{
  auto table = ds->dataTableManager().getTable(tableId);
  if (table == NULL)
    return -1;

  DataTableCounter counter;
  table->accept(0.0, std::numeric_limits<double>::max(), counter);
  return counter.counter();
}

int testDataTable(simUtil::DataStoreTestHelper& helper, simData::ObjectId id)
{
  int rv = 0;

  auto tableId = helper.addDataTable(id, 1, "Table 1");
  rv += SDK_ASSERT(dataTableEntries(helper.dataStore(), tableId) == 1);
  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) == 0);
  rv += SDK_ASSERT(dataTableEntries(helper.dataStore(), tableId) == 0);

  // flushing an empty table should be OK
  rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) == 0);
  rv += SDK_ASSERT(dataTableEntries(helper.dataStore(), tableId) == 0);

  return rv;
}

int testScenario()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;

  rv += testGenericData(testHelper, 0);
  rv += testDataTable(testHelper, 0);

  return rv;
}

simData::ObjectId makePlatform(simUtil::DataStoreTestHelper& testHelper, uint64_t& tableId)
{
  simData::ObjectId id = testHelper.addPlatform();

  // Add update
  testHelper.addPlatformUpdate(0.0, id);

  // Add Command
  simData::PlatformCommand command;
  command.set_time(0.0);
  command.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addPlatformCommand(command, id);

  // Add category data
  testHelper.addCategoryData(id, "Category", "Default", -1.0);
  testHelper.addCategoryData(id, "Category", "Value1", 1.0);
  testHelper.addCategoryData(id, "Category", "Value2", 2.0);

  // Add generic data
  testHelper.addGenericData(id, "Key", "Value", 0.0);

  // Add data table
  tableId = testHelper.addDataTable(id, 1, "Table 1");

  return id;
}

int validatePlatform(simData::DataStore* ds, simData::ObjectId id, uint64_t tableId, int updates, int commands, int categoryData, int genericData, int dataTable)
{
  int rv = 0;

  rv += SDK_ASSERT(ds->platformUpdateSlice(id)->numItems() == static_cast<size_t>(updates));
  rv += SDK_ASSERT(ds->platformCommandSlice(id)->numItems() == static_cast<size_t>(commands));
  rv += SDK_ASSERT(categoryDataEntries(ds, id) == categoryData);
  rv += SDK_ASSERT(genericDataEntries(ds, id) == static_cast<size_t>(genericData));
  rv += SDK_ASSERT(dataTableEntries(ds, id) == dataTable);

  return 0;
}

int testFields()
{
  int rv = 0;

  {
    // Test FLUSH_ALL
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 0, 0, 0, 0, 0) == 0);
  }

  {
    // Test FLUSH_UPDATES
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 0, 1, 3, 1, 1) == 0);
  }

  {
    // Test FLUSH_COMMANDS
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 0, 3, 1, 1) == 0);
  }

  {
    // Test FLUSH_CATEGORY_DATA
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_CATEGORY_DATA) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 0, 1, 1) == 0);
  }

  {
    // Test FLUSH_CATEGORY_DATA with FLUSH_EXCLUDE_MINUS_ONE
    auto historicCategoryFlush = static_cast<simData::DataStore::FlushFields>(simData::DataStore::FLUSH_CATEGORY_DATA | simData::DataStore::FLUSH_EXCLUDE_MINUS_ONE);
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, historicCategoryFlush) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 2, 1, 1) == 0);
  }

  {
    // Test FLUSH_GENERIC_DATA
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 0, 1) == 0);
  }

  {
    // Test FLUSH_DATA_TABLES
    simUtil::DataStoreTestHelper testHelper;
    uint64_t tableId;
    simData::ObjectId id = makePlatform(testHelper, tableId);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_DATA_TABLES) == 0);
    rv += SDK_ASSERT(validatePlatform(testHelper.dataStore(), id, tableId, 1, 1, 3, 1, 0) == 0);
  }

  {
    // Test static platform
    simUtil::DataStoreTestHelper testHelper;
    simData::DataStore* ds = testHelper.dataStore();

    auto staticId = testHelper.addPlatform();
    // Add data
    testHelper.addPlatformUpdate(-1, staticId);
    rv += SDK_ASSERT(ds->platformUpdateSlice(staticId)->numItems() == 1);

    // Should not clear out the static platform
    auto keepStaticPlatformFlush = static_cast<simData::DataStore::FlushFields>(simData::DataStore::FLUSH_UPDATES | simData::DataStore::FLUSH_EXCLUDE_MINUS_ONE);
    rv += SDK_ASSERT(ds->flush(staticId, simData::DataStore::FLUSH_NONRECURSIVE, keepStaticPlatformFlush) == 0);
    rv += SDK_ASSERT(ds->platformUpdateSlice(staticId)->numItems() == 1);

    // Clear out the static platform
    rv += SDK_ASSERT(ds->flush(staticId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES) == 0);
    rv += SDK_ASSERT(ds->platformUpdateSlice(staticId)->numItems() == 0);
  }

  return rv;
}

// Keep track of ids in a scenario
struct Ids
{
  simData::ObjectId platformId;
  simData::ObjectId beamId;
  simData::ObjectId gateId;
  simData::ObjectId beamProjectorId;
  simData::ObjectId platformProjectorId;
  simData::ObjectId laserId;
  simData::ObjectId lobId;
  simData::ObjectId customRenderingId;
  simData::ObjectId platformCustomRenderingId;
  Ids()
    : platformId(0),
      beamId(0),
      gateId(0),
      beamProjectorId(0),
      platformProjectorId(0),
      laserId(0),
      lobId(0),
      customRenderingId(0),
      platformCustomRenderingId(0)
  {}
};

// Make a scenario to test recursion
void makeScenario(simUtil::DataStoreTestHelper& testHelper, Ids& ids)
{
  ids.platformId = testHelper.addPlatform();
  simData::PlatformCommand platformCommand;
  platformCommand.set_time(0.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addPlatformCommand(platformCommand, ids.platformId);

  ids.beamId = testHelper.addBeam(ids.platformId);
  simData::BeamCommand beamCommand;
  beamCommand.set_time(0.0);
  beamCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addBeamCommand(beamCommand, ids.beamId);

  ids.gateId = testHelper.addGate(ids.beamId);
  simData::GateCommand gateCommand;
  gateCommand.set_time(0.0);
  gateCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addGateCommand(gateCommand, ids.gateId);

  ids.beamProjectorId = testHelper.addProjector(ids.beamId);
  simData::ProjectorCommand projectorCommand;
  projectorCommand.set_time(0.0);
  projectorCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addProjectorCommand(projectorCommand, ids.beamProjectorId);

  ids.laserId = testHelper.addLaser(ids.platformId);
  simData::LaserCommand laserCommand;
  laserCommand.set_time(0.0);
  laserCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addLaserCommand(laserCommand, ids.laserId);

  ids.lobId = testHelper.addLOB(ids.platformId);
  simData::LobGroupCommand lobCommand;
  lobCommand.set_time(0.0);
  lobCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addLOBCommand(lobCommand, ids.lobId);

  ids.platformProjectorId = testHelper.addProjector(ids.platformId);
  testHelper.addProjectorCommand(projectorCommand, ids.platformProjectorId);

  ids.platformCustomRenderingId = testHelper.addCustomRendering(ids.platformId);
  simData::CustomRenderingCommand customRenderingCommand;
  customRenderingCommand.set_time(0.0);
  customRenderingCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addCustomRenderingCommand(customRenderingCommand, ids.platformCustomRenderingId);

  ids.customRenderingId = testHelper.addCustomRendering(0);
  testHelper.addCustomRenderingCommand(customRenderingCommand, ids.customRenderingId);
}

// Make sure each entity type has the correct number of commands
int validateCommands(simData::DataStore* ds, Ids& ids, size_t platform, size_t beam, size_t gate, size_t beamProjector, size_t laser, size_t lob, size_t platformProjector, size_t platformCr, size_t cr)
{
  int rv = 0;

  rv += SDK_ASSERT(ds->platformCommandSlice(ids.platformId)->numItems() == platform);
  rv += SDK_ASSERT(ds->beamCommandSlice(ids.beamId)->numItems() == beam);
  rv += SDK_ASSERT(ds->gateCommandSlice(ids.gateId)->numItems() == gate);
  rv += SDK_ASSERT(ds->projectorCommandSlice(ids.beamProjectorId)->numItems() == beamProjector);
  rv += SDK_ASSERT(ds->laserCommandSlice(ids.laserId)->numItems() == laser);
  rv += SDK_ASSERT(ds->lobGroupCommandSlice(ids.lobId)->numItems() == lob);
  rv += SDK_ASSERT(ds->projectorCommandSlice(ids.platformProjectorId)->numItems() == platformProjector);
  rv += SDK_ASSERT(ds->customRenderingCommandSlice(ids.platformCustomRenderingId)->numItems() == platformCr);
  rv += SDK_ASSERT(ds->customRenderingCommandSlice(ids.customRenderingId)->numItems() == cr);

  return rv;
}

int testRecursion()
{
  int rv = 0;

  {
    // Test scenario, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(0, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 0, 0, 0, 0, 0, 0, 0, 0, 0) == 0);
  }

  {
    // Test scenario, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test platform, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 0, 0, 0, 0, 0, 0, 0, 0, 1) == 0);
  }

  {
    // Test platform, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 0, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test beam, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.beamId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 0, 0, 0, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test beam, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.beamId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 0, 1, 1, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test gate, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.gateId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 0, 1, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test gate, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.gateId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 0, 1, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test beam projector, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.beamProjectorId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 0, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test beam projector, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.beamProjectorId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 0, 1, 1, 1, 1, 1) == 0);
  }

  {
    // Test laser, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.laserId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 0, 1, 1, 1, 1) == 0);
  }

  {
    // Test laser, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.laserId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 0, 1, 1, 1, 1) == 0);
  }

  {
    // Test lob, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.lobId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 0, 1, 1, 1) == 0);
  }

  {
    // Test lob, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.lobId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 0, 1, 1, 1) == 0);
  }

  {
    // Test platform projector, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformProjectorId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 0, 1, 1) == 0);
  }

  {
    // Test platform projector, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformProjectorId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 0, 1, 1) == 0);
  }

  {
    // Test platform custom rendering, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformCustomRenderingId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 0, 1) == 0);
  }

  {
    // Test platform custom rendering, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.platformCustomRenderingId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 0, 1) == 0);
  }

  {
    // Test custom rendering, recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.customRenderingId, simData::DataStore::FLUSH_RECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 0) == 0);
  }

  {
    // Test custom rendering, non-recursive
    simUtil::DataStoreTestHelper testHelper;
    Ids ids;
    makeScenario(testHelper, ids);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 1) == 0);
    rv += SDK_ASSERT(testHelper.dataStore()->flush(ids.customRenderingId, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_ALL) == 0);
    rv += SDK_ASSERT(validateCommands(testHelper.dataStore(), ids, 1, 1, 1, 1, 1, 1, 1, 1, 0) == 0);
  }

  return rv;
}

// Make a series of platform updates
simData::ObjectId makePlatformUpdateSeries(simUtil::DataStoreTestHelper& testHelper)
{
  auto id = testHelper.addPlatform();
  testHelper.addPlatformUpdate(0, id);
  testHelper.addPlatformUpdate(1, id);
  testHelper.addPlatformUpdate(2, id);
  testHelper.addPlatformUpdate(3, id);
  testHelper.addPlatformUpdate(4, id);
  return id;
}

// Visitor to validate platform updates
class ValidatePlatformUpdate : public simData::VisitableDataSlice<simData::PlatformUpdate>::Visitor
{
public:
  ValidatePlatformUpdate(const std::vector<double>& times)
    : times_(times),
    index_(0),
    errors_(0)
  {
  }

  virtual void operator()(const simData::PlatformUpdate* update)
  {
    if (update->time() != times_[index_])
      ++errors_;
    ++index_;
  }

  int errors() const { return errors_; }

private:
  const std::vector<double>& times_;
  size_t index_;
  int errors_;
};

// Validate the platform updates against the given times
int validatePlatformUpdateSeries(simUtil::DataStoreTestHelper& helper, simData::ObjectId id, const std::vector<double>& times)
{
  int rv = 0;
  auto ds = helper.dataStore();
  auto slice = ds->platformUpdateSlice(id);
  rv += SDK_ASSERT(slice->numItems() == times.size());
  if (rv != 0)
    return rv;

  ValidatePlatformUpdate validate(times);
  slice->visit(&validate);
  return validate.errors();
}

// Make a series of platform updates
simData::ObjectId makePlatformCommandSeries(simUtil::DataStoreTestHelper& testHelper)
{
  auto id = testHelper.addPlatform();
  simData::PlatformCommand platformCommand;
  platformCommand.set_time(0.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x0);
  testHelper.addPlatformCommand(platformCommand, id);
  platformCommand.set_time(1.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x1);
  testHelper.addPlatformCommand(platformCommand, id);
  platformCommand.set_time(2.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x2);
  testHelper.addPlatformCommand(platformCommand, id);
  platformCommand.set_time(3.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x3);
  testHelper.addPlatformCommand(platformCommand, id);
  platformCommand.set_time(4.0);
  platformCommand.mutable_updateprefs()->mutable_commonprefs()->set_color(0x4);
  testHelper.addPlatformCommand(platformCommand, id);

  return id;
}

// Visitor to validate platform updates
class ValidatePlatformCommand : public simData::VisitableDataSlice<simData::PlatformCommand>::Visitor
{
public:
  ValidatePlatformCommand(const std::vector<double>& times)
    : times_(times),
    index_(0),
    errors_(0)
  {
  }

  virtual void operator()(const simData::PlatformCommand* update)
  {
    if (update->time() != times_[index_])
      ++errors_;
    ++index_;
  }

  int errors() const { return errors_; }

private:
  const std::vector<double>& times_;
  size_t index_;
  int errors_;
};

// Validate the platform updates against the given times
int validatePlatformCommandSeries(simUtil::DataStoreTestHelper& helper, simData::ObjectId id, const std::vector<double>& times)
{
  int rv = 0;
  auto ds = helper.dataStore();
  auto slice = ds->platformCommandSlice(id);
  rv += SDK_ASSERT(slice->numItems() == times.size());
  if (rv != 0)
    return rv;

  ValidatePlatformCommand validate(times);
  slice->visit(&validate);
  return validate.errors();
}

int testPlatformTimeRange()
{
  int rv = 0;

  // Test updates
  {
    // flush all
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformUpdateSeries(helper);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 0.0, 10.0) == 0);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { }) == 0);
  }

  {
    // flush start
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformUpdateSeries(helper);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 0.0, 2.0) == 0);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformUpdateSeries(helper);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 1.0, 2.0) == 0);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush more from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformUpdateSeries(helper);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 1.0, 4.0) == 0);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 4.0 }) == 0);
  }

  {
    // flush the end
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformUpdateSeries(helper);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 3.0, 5.0) == 0);
    rv += SDK_ASSERT(validatePlatformUpdateSeries(helper, id, { 0.0, 1.0, 2.0 }) == 0);
  }

  // Test commands
  {
    // flush all
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformCommandSeries(helper);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS, 0.0, 10.0) == 0);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { }) == 0);
  }

  {
    // flush start
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformCommandSeries(helper);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS, 0.0, 2.0) == 0);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformCommandSeries(helper);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS, 1.0, 2.0) == 0);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush more from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformCommandSeries(helper);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS, 1.0, 4.0) == 0);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 4.0 }) == 0);
  }

  {
    // flush the end
    simUtil::DataStoreTestHelper helper;
    auto id = makePlatformCommandSeries(helper);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_COMMANDS, 3.0, 5.0) == 0);
    rv += SDK_ASSERT(validatePlatformCommandSeries(helper, id, { 0.0, 1.0, 2.0 }) == 0);
  }

  return rv;
}

// Make a series of LOB updates
simData::ObjectId makeLobUpdateSeries(simUtil::DataStoreTestHelper& testHelper)
{
  auto platformId = testHelper.addPlatform();
  auto id = testHelper.addLOB(platformId);
  testHelper.addLOBUpdate(0, id);
  testHelper.addLOBUpdate(1, id);
  testHelper.addLOBUpdate(2, id);
  testHelper.addLOBUpdate(3, id);
  testHelper.addLOBUpdate(4, id);
  return id;
}

// Visitor to validate LOB updates
class ValidateLobUpdate : public simData::VisitableDataSlice<simData::LobGroupUpdate>::Visitor
{
public:
  ValidateLobUpdate(const std::vector<double>& times)
    : times_(times),
    index_(0),
    errors_(0)
  {
  }

  virtual void operator()(const simData::LobGroupUpdate* update)
  {
    if (update->time() != times_[index_])
      ++errors_;
    ++index_;
  }

  int errors() const { return errors_; }

private:
  const std::vector<double>& times_;
  size_t index_;
  int errors_;
};

// Validate the LOB updates against the given times
int validateLobUpdateSeries(simUtil::DataStoreTestHelper& helper, simData::ObjectId id, const std::vector<double>& times)
{
  int rv = 0;
  auto ds = helper.dataStore();
  auto slice = ds->lobGroupUpdateSlice(id);
  rv += SDK_ASSERT(slice->numItems() == times.size());
  if (rv != 0)
    return rv;

  ValidateLobUpdate validate(times);
  slice->visit(&validate);
  return validate.errors();
}

// Test LOB time range flush
int testLobTimeRange()
{
  int rv = 0;

  {
    // flush all
    simUtil::DataStoreTestHelper helper;
    auto id = makeLobUpdateSeries(helper);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 0.0, 10.0) == 0);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { }) == 0);
  }

  {
    // flush start
    simUtil::DataStoreTestHelper helper;
    auto id = makeLobUpdateSeries(helper);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 0.0, 2.0) == 0);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makeLobUpdateSeries(helper);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 1.0, 2.0) == 0);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 2.0, 3.0, 4.0 }) == 0);
  }

  {
    // flush more from the middle
    simUtil::DataStoreTestHelper helper;
    auto id = makeLobUpdateSeries(helper);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 1.0, 4.0) == 0);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 4.0 }) == 0);
  }

  {
    // flush the end
    simUtil::DataStoreTestHelper helper;
    auto id = makeLobUpdateSeries(helper);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0, 3.0, 4.0 }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(id, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_UPDATES, 3.0, 5.0) == 0);
    rv += SDK_ASSERT(validateLobUpdateSeries(helper, id, { 0.0, 1.0, 2.0 }) == 0);
  }

  return rv;
}

void makeGenericDataSeries(simUtil::DataStoreTestHelper& helper)
{
  helper.addGenericData(0, "Key", "0", 0.0);
  helper.addGenericData(0, "Key", "1", 1.0);
  helper.addGenericData(0, "Key", "2", 2.0);
  helper.addGenericData(0, "Key", "3", 3.0);
  helper.addGenericData(0, "Key", "4", 4.0);
}

// Keep track of time value pairs
struct TimeValuePair
{
  double time;
  std::string value;
  TimeValuePair(double inTime, const std::string& inValue)
    : time(inTime),
    value(inValue)
  {}
};

// Visitor to validate generic data
class ValidateGenericData : public simData::VisitableDataSlice<simData::GenericData>::Visitor
{
public:
  ValidateGenericData(const std::vector<TimeValuePair>& pairs)
    : pairs_(pairs),
      index_(0),
      errors_(0)
  {
  }

  virtual void operator()(const simData::GenericData* update)
  {
    if (update->time() != pairs_[index_].time)
      ++errors_;

    for (int ii = 0; ii < update->entry_size(); ++ii)
    {
      auto entry = update->entry(ii);
      if (entry.value() != pairs_[index_].value)
        ++errors_;
      ++index_;
    }
  }

  int errors() const { return errors_; }

private:
  const std::vector<TimeValuePair>& pairs_;
  size_t index_;
  int errors_;
};

// Validate generic data against the given pairs
int validateGenericDataSeries(simUtil::DataStoreTestHelper& helper, const std::vector<TimeValuePair>& pairs)
{
  auto ds = helper.dataStore();
  auto slice = ds->genericDataSlice(0);
  ValidateGenericData validate(pairs);
  slice->visit(&validate);
  return validate.errors();
}

// Test Generic data time range flush
int testGenericDataTimeRange()
{
  int rv = 0;

  {
    // flush all
    simUtil::DataStoreTestHelper helper;
    rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), 0) == 0);
    makeGenericDataSeries(helper);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA, 0.0, 10.0) == 0);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { }) == 0);
  }

  {
    // flush start
    simUtil::DataStoreTestHelper helper;
    rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), 0) == 0);
    makeGenericDataSeries(helper);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA, 0.0, 2.0) == 0);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
  }

  {
    // flush from the middle
    simUtil::DataStoreTestHelper helper;
    rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), 0) == 0);
    makeGenericDataSeries(helper);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA, 1.0, 2.0) == 0);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
  }

  {
    // flush more from the middle
    simUtil::DataStoreTestHelper helper;
    rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), 0) == 0);
    makeGenericDataSeries(helper);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA, 1.0, 4.0) == 0);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {4.0, "4"} }) == 0);
  }

  {
    // flush the end
    simUtil::DataStoreTestHelper helper;
    rv += SDK_ASSERT(genericDataEntries(helper.dataStore(), 0) == 0);
    makeGenericDataSeries(helper);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"}, {3.0, "3"}, {4.0, "4"} }) == 0);
    rv += SDK_ASSERT(helper.dataStore()->flush(0, simData::DataStore::FLUSH_NONRECURSIVE, simData::DataStore::FLUSH_GENERIC_DATA, 3.0, 5.0) == 0);
    rv += SDK_ASSERT(validateGenericDataSeries(helper, { {0.0, "0"}, {1.0, "1"}, {2.0, "2"} }) == 0);
  }

  return rv;
}

}

int TestFlush(int argc, char* argv[])
{
  int rv = 0;

  rv += testEmpty();
  rv += testScenario();
  rv += testFields();
  rv += testRecursion();

  rv += testPlatformTimeRange();
  rv += testLobTimeRange();
  rv += testGenericDataTimeRange();

  return rv;
}
