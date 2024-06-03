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
#include <map>
#include "simCore/Common/SDKAssert.h"
#include "simData/DataStoreProxy.h"
#include "simData/DataTable.h"
#include "simUtil/DataStoreTestHelper.h"

namespace {

/** Helper class implementation of NewUpdatesListener, used to verify that we're getting data we expect. */
class TimeCollector : public simData::DataStore::NewUpdatesListener
{
public:
  /** Record the time value of entity update */
  virtual void onEntityUpdate(simData::DataStore* source, simData::ObjectId id, double dataTime)
  {
    allData_[id].insert(dataTime);
  }

  /** Record the time value of the new row for the entity, same as onEntityUpdate(). */
  virtual void onNewRowData(simData::DataStore* source, simData::DataTable& table, simData::ObjectId id, double dataTime)
  {
    allData_[id].insert(dataTime);
  }

  /** Clear out the updates for the given entity and record the flush */
  virtual void onFlush(simData::DataStore* source, simData::ObjectId flushedId)
  {
    if (flushedId == 0)
    {
      flushedIds_.clear();
      flushedIds_.insert(0);
      allData_.clear();
    }
    else
    {
      flushedIds_.insert(flushedId);
      allData_.erase(flushedId);
    }
  }

  /** Clear out saved data */
  void clear()
  {
    allData_.clear();
    flushedIds_.clear();
  }

  /** Set of time stamps */
  typedef std::set<double> Timestamps;

  /** Get all timestamps recorded since last clear/flush for a given ID */
  Timestamps getTimes(simData::ObjectId id) const
  {
    auto i = allData_.find(id);
    if (i == allData_.end())
      return Timestamps();
    return i->second;
  }

  /** Returns true if the ID was flushed since last clear() */
  bool sawFlush(simData::ObjectId id) const
  {
    if (flushedIds_.empty()) // common case
      return false;
    else if (flushedIds_.find(0) != flushedIds_.end()) // uncommon case
      return true;
    // Least common case: flush on a single entity
    return flushedIds_.find(id) != flushedIds_.end();
  }

private:
  std::map<simData::ObjectId, Timestamps> allData_;
  std::set<simData::ObjectId> flushedIds_;
};

int testEntityCollection()
{
  simUtil::DataStoreTestHelper helper;

  simData::DataStore* ds = helper.dataStore();
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  ds->addNewUpdatesListener(timeCollector);

  simData::ObjectId plat1 = helper.addPlatform(1);
  simData::ObjectId plat2 = helper.addPlatform(2);
  simData::ObjectId plat3 = helper.addPlatform(3);
  helper.addPlatformUpdate(1.0, plat1);
  helper.addPlatformUpdate(1.0, plat2);
  helper.addPlatformUpdate(1.0, plat3);

  helper.addPlatformUpdate(1.5, plat3);

  helper.addPlatformUpdate(2.0, plat1);
  helper.addPlatformUpdate(2.0, plat2);

  helper.addPlatformUpdate(2.5, plat3);

  int rv = 0;

  auto p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 2);
  rv += SDK_ASSERT(p1Times.count(1.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.5) == 0);
  auto p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 2);
  rv += SDK_ASSERT(p2Times.count(1.0) != 0);
  rv += SDK_ASSERT(p2Times.count(2.0) != 0);
  rv += SDK_ASSERT(p2Times.count(2.5) == 0);
  auto p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 3);
  rv += SDK_ASSERT(p3Times.count(1.0) != 0);
  rv += SDK_ASSERT(p3Times.count(1.5) != 0);
  rv += SDK_ASSERT(p3Times.count(2.5) != 0);

  // Test that adding more points adds to collection
  helper.addPlatformUpdate(3.0, plat1);
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 3);
  rv += SDK_ASSERT(p1Times.count(1.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.0) != 0);
  rv += SDK_ASSERT(p1Times.count(3.0) != 0);

  // Clear out the updates, simulating what should happen per frame
  timeCollector->clear();
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).empty());
  // Make sure we still haven't seen a flush (clear() doesn't count)
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat1));

  // .. then add more points
  helper.addPlatformUpdate(3.2, plat1);
  helper.addPlatformUpdate(3.6, plat2);
  helper.addPlatformUpdate(4.0, plat2);
  helper.addPlatformUpdate(4.0, plat3);
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 1);
  rv += SDK_ASSERT(p1Times.count(3.2) != 0);
  p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 2);
  rv += SDK_ASSERT(p2Times.count(3.6) != 0);
  rv += SDK_ASSERT(p2Times.count(4.0) != 0);
  p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 1);
  rv += SDK_ASSERT(p3Times.count(4.0) != 0);

  // Do a single flush on one platform; still in the same frame as last time
  ds->flush(plat1);
  // .. so we should have one flush and one empty set of times on plat1
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).size() == 2);
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).size() == 1);
  rv += SDK_ASSERT(timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat3));

  // Do a single entity flush after a clear (simulated update frame)
  timeCollector->clear();
  helper.addPlatformUpdate(4.1, plat1);
  helper.addPlatformUpdate(4.2, plat2);
  helper.addPlatformUpdate(4.3, plat3);
  ds->flush(plat2);
  helper.addPlatformUpdate(4.5, plat1);
  helper.addPlatformUpdate(4.6, plat2);
  helper.addPlatformUpdate(4.7, plat3);

  // Verify expected output from time colletor
  p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 2);
  rv += SDK_ASSERT(p1Times.count(4.1) != 0);
  rv += SDK_ASSERT(p1Times.count(4.5) != 0);
  p2Times = timeCollector->getTimes(plat2);
  rv += SDK_ASSERT(p2Times.size() == 1);
  rv += SDK_ASSERT(p2Times.count(4.2) == 0);
  rv += SDK_ASSERT(p2Times.count(4.6) != 0);
  p3Times = timeCollector->getTimes(plat3);
  rv += SDK_ASSERT(p3Times.size() == 2);
  rv += SDK_ASSERT(p3Times.count(4.3) != 0);
  rv += SDK_ASSERT(p3Times.count(4.7) != 0);
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(!timeCollector->sawFlush(plat3));

  // New frame, flush everything
  timeCollector->clear();
  helper.addPlatformUpdate(5.3, plat3);
  ds->flush(0);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat3).empty());
  rv += SDK_ASSERT(timeCollector->sawFlush(0));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat1));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat2));
  rv += SDK_ASSERT(timeCollector->sawFlush(plat3));
  rv += SDK_ASSERT(timeCollector->sawFlush(100)); // random ID that doesn't exist, but should trigger because we flushed 0

  return rv;
}

int testDataTableCollection()
{
  int rv = 0;

  // Create data store; configure time collector
  simUtil::DataStoreTestHelper helper;
  simData::DataStore* ds = helper.dataStore();
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  ds->addNewUpdatesListener(timeCollector);

  // Create two platforms with initial data points
  simData::ObjectId plat1 = helper.addPlatform(1);
  simData::ObjectId plat2 = helper.addPlatform(2);
  helper.addPlatformUpdate(1.0, plat1);
  helper.addPlatformUpdate(1.0, plat2);
  helper.addPlatformUpdate(5.0, plat1);
  helper.addPlatformUpdate(5.0, plat2);

  // Clear out the values
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 2);
  timeCollector->clear();
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(0).empty());

  // Create three tables; one on each platform, and one on the scenario
  simData::DataTableManager& dtm = ds->dataTableManager();
  simData::DataTable* table0;
  rv += SDK_ASSERT(dtm.addDataTable(0, "Table 0", &table0).isSuccess());
  rv += SDK_ASSERT(table0 != nullptr);
  simData::DataTable* table1;
  rv += SDK_ASSERT(dtm.addDataTable(plat1, "Table 1", &table1).isSuccess());
  rv += SDK_ASSERT(table1 != nullptr);
  simData::DataTable* table2;
  rv += SDK_ASSERT(dtm.addDataTable(plat2, "Table 2", &table2).isSuccess());
  rv += SDK_ASSERT(table2 != nullptr);

  // Create the table columns; plat1 gets 2 columns, rest get 1 column
  simData::TableColumn* col0_1;
  rv += SDK_ASSERT(table0->addColumn("Column 0_1", simData::VT_DOUBLE, 0, &col0_1).isSuccess());
  simData::TableColumn* col1_1;
  rv += SDK_ASSERT(table1->addColumn("Column 1_1", simData::VT_DOUBLE, 0, &col1_1).isSuccess());
  simData::TableColumn* col1_2;
  rv += SDK_ASSERT(table1->addColumn("Column 1_2", simData::VT_DOUBLE, 0, &col1_2).isSuccess());
  simData::TableColumn* col2_1;
  rv += SDK_ASSERT(table2->addColumn("Column 2_1", simData::VT_DOUBLE, 0, &col2_1).isSuccess());

  // Verify that we still don't have any resolved times
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(0).empty());

  // Add rows to validate times
  {
    simData::TableRow row;
    row.setTime(1.1);
    row.setValue(col1_1->columnId(), 100.0);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(1.1) == 1);

    row.setTime(2.2);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 2);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(1.1) == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(2.2) == 1);

    row.setTime(1.7);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 3);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(1.1) == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(1.7) == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(2.2) == 1);
  }

  { // Add rows to the second column
    simData::TableRow row;
    row.setTime(2.8);
    row.setValue(col1_2->columnId(), 100.0);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 4);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(2.8) == 1);

    // Duplicate time from the other column
    row.setTime(2.2);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 4);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(2.2) == 1);

    // Within time bounds but not a duplicate
    row.setTime(2.0);
    rv += SDK_ASSERT(table1->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).size() == 5);
    rv += SDK_ASSERT(timeCollector->getTimes(plat1).count(2.0) == 1);
  }

  { // Add rows to the table on the scenario
    simData::TableRow row;
    row.setTime(1.2);
    row.setValue(col0_1->columnId(), 100.0);
    rv += SDK_ASSERT(table0->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(0).count(1.2) == 1);

    row.setTime(2.0);
    rv += SDK_ASSERT(table0->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 2);
    rv += SDK_ASSERT(timeCollector->getTimes(0).count(1.2) == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(0).count(2.0) == 1);
  }

  // Verify behavior of getTimeRange()
  double begin = 0.0;
  double end = 0.0;
  rv += SDK_ASSERT(col0_1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.2);
  rv += SDK_ASSERT(end == 2.0);
  rv += SDK_ASSERT(col1_1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.1);
  rv += SDK_ASSERT(end == 2.2);
  rv += SDK_ASSERT(col1_2->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 2.0);
  rv += SDK_ASSERT(end == 2.8);
  rv += SDK_ASSERT(col2_1->getTimeRange(begin, end) != 0);

  // Execute a flush on the data and make sure things are good still
  ds->flush(0);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(plat2).empty());
  rv += SDK_ASSERT(timeCollector->getTimes(0).empty());
  rv += SDK_ASSERT(col0_1->getTimeRange(begin, end) != 0);
  rv += SDK_ASSERT(col1_1->getTimeRange(begin, end) != 0);
  rv += SDK_ASSERT(col1_2->getTimeRange(begin, end) != 0);
  rv += SDK_ASSERT(col2_1->getTimeRange(begin, end) != 0);

  // Add two rows and make sure they're caught
  { // Add row to table0
    simData::TableRow row;
    row.setTime(3.5);
    row.setValue(col0_1->columnId(), 100.0);
    rv += SDK_ASSERT(table0->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(0).count(3.5) == 1);
  }
  { // Add row to table2
    simData::TableRow row;
    row.setTime(3.6);
    row.setValue(col2_1->columnId(), 100.0);
    rv += SDK_ASSERT(table2->addRow(row).isSuccess());
    rv += SDK_ASSERT(timeCollector->getTimes(plat2).size() == 1);
    rv += SDK_ASSERT(timeCollector->getTimes(plat2).count(3.6) == 1);
  }

  return rv;
}

int addTableAndTime(simData::DataStore& ds, simData::ObjectId id, double timeValue)
{
  simData::DataTableManager& dtm = ds.dataTableManager();
  simData::DataTable* table;
  if (dtm.addDataTable(id, "Table", &table).isError())
    return 1;
  simData::TableColumn* column;
  if (table->addColumn("Column", simData::VT_DOUBLE, 0, &column).isError())
    return 1;
  simData::TableRow row;
  row.setTime(timeValue);
  row.setValue(column->columnId(), 100.0);
  return table->addRow(row).isError() ? 1 : 0;
}

int testDataStoreProxy()
{
  int rv = 0;

  // Make sure that when assigning a new scenario through data store proxy, the time collector lives on
  simData::MemoryDataStore* ds1 = new simData::MemoryDataStore;
  simData::DataStoreProxy proxy(ds1);
  // Listener should not have changed

  // Add a table and row to ds1 for later testing
  rv += SDK_ASSERT(addTableAndTime(proxy, 0, 1.5) == 0);

  // Migrate to a new datastore
  simData::MemoryDataStore* ds2 = new simData::MemoryDataStore;
  proxy.reset(ds2);
  ds1 = nullptr; // reset() will delete it
  // Listener should have changed

  // Now update it to a custom one we provide, Time Collector
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  proxy.addNewUpdatesListener(timeCollector);

  // Should have no time collections on entity 0
  rv += SDK_ASSERT(timeCollector->getTimes(0).empty());

  // Make sure it counts times for new rows
  rv += SDK_ASSERT(addTableAndTime(proxy, 0, 2.5) == 0);
  rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 1);
  rv += SDK_ASSERT(timeCollector->getTimes(0).count(2.5) == 1);

  // Reset a new proxy and our Time Collector should have carried over
  simData::MemoryDataStore* ds3 = new simData::MemoryDataStore;
  proxy.reset(ds3);
  ds2 = nullptr; // reset() will delete it
  // Listener should have changed

  // Make sure it counts times for new rows still after the proxy reset
  rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 1);  // Because we never reset it
  rv += SDK_ASSERT(addTableAndTime(proxy, 0, 3.5) == 0);
  rv += SDK_ASSERT(timeCollector->getTimes(0).size() == 2);
  rv += SDK_ASSERT(timeCollector->getTimes(0).count(2.5) == 1);
  rv += SDK_ASSERT(timeCollector->getTimes(0).count(3.5) == 1);

  return rv;
}

// Ensure things like Category Data do not clutter the entity values
int testIgnoresCategoryData()
{
  simUtil::DataStoreTestHelper helper;

  simData::DataStore* ds = helper.dataStore();
  std::shared_ptr<TimeCollector> timeCollector(new TimeCollector);
  ds->addNewUpdatesListener(timeCollector);

  simData::ObjectId plat1 = helper.addPlatform(1);
  helper.addPlatformUpdate(1.0, plat1);
  helper.addPlatformUpdate(2.0, plat1);

  int rv = 0;

  // Verify initial state
  auto p1Times = timeCollector->getTimes(plat1);
  rv += SDK_ASSERT(p1Times.size() == 2);
  rv += SDK_ASSERT(p1Times.count(1.0) != 0);
  rv += SDK_ASSERT(p1Times.count(2.0) != 0);
  timeCollector->clear();
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());

  // Add category update
  helper.addCategoryData(plat1, "Key", "Value", 2.5);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());
  // And Generic update
  helper.addGenericData(plat1, "GenData", "Value", 2.8);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());

  // Add a command
  simData::PlatformCommand cmd;
  cmd.set_time(2.9);
  cmd.mutable_updateprefs()->set_axisscale(2.0);
  helper.addPlatformCommand(cmd, plat1);
  rv += SDK_ASSERT(timeCollector->getTimes(plat1).empty());

  return rv;
}

}

int TestNewUpdatesListener(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testEntityCollection() == 0);
  rv += SDK_ASSERT(testDataTableCollection() == 0);
  rv += SDK_ASSERT(testDataStoreProxy() == 0);
  rv += SDK_ASSERT(testIgnoresCategoryData() == 0);
  return rv;
}
