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
#include <string>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Math.h"
#include "simData/DataTable.h"
#include "simData/MemoryDataStore.h"
#include "simData/MemoryTable/DoubleBufferTimeContainer.h"
#ifdef USE_DEPRECATED_SIMDISSDK_API
#include "simData/MemoryTable/TimeContainerDeque.h"
#endif
#include "simData/MemoryTable/SubTable.h"
#include "simData/MemoryTable/TableManager.h"
#include "simUtil/DataStoreTestHelper.h"

using namespace simData;

namespace
{

/**
* Test for the ManagerObserver, checks that table identified by tableName_ or whose ownerId matches the specified owner id has been added/removed
* Usage: call setExpectedTableName before calls to DataTableManager::addDataTable or DataTableManager::deleteTable.
*        call setExpectedOwnerId before calls to DataTableManager::deleteTablesByOwner.
*/
class TestManagerObserver : public simData::DataTableManager::ManagerObserver
{
public:
  explicit TestManagerObserver(const std::string& tableName)
    :active_(true),
    numErrors_(0),
    tableName_(tableName)
  {}
  virtual void onAddTable(simData::DataTable* table)
  {
    if (active_)
      numErrors_ += SDK_ASSERT(table->tableName() == tableName_);
  }
  virtual void onPreRemoveTable(DataTable* table)
  {
    if (active_)
      numErrors_ += SDK_ASSERT(table->tableName() == tableName_ || table->ownerId() == ownerId_);
  }
  void setExpectedTableName(const std::string& tableName) {tableName_ = tableName; }
  void setExpectedOwnerId(simData::ObjectId ownerId) { ownerId_ = ownerId; }
  int numErrors() const { return numErrors_; }
  void setActive(bool active) { active_ = active; }

private:
  bool active_;
  int numErrors_;
  std::string tableName_;
  simData::ObjectId ownerId_;
};

/**
* Test for the TableObserver, checks that new column names match or new row times match
* Usage: call setExpectedColumnName before calls to DataTable::addColumn
*        call setExpectedRowTime before calls to DataTable::addRow
*/
class TestTableObserver : public simData::DataTable::TableObserver
{
public:
  explicit TestTableObserver(simData::DataTable& table)
    :active_(true),
     numErrors_(0),
     rowTime_(0.),
     removeRowTime_(0.),
     table_(table)
  {}

  virtual void onAddColumn(DataTable& table, const simData::TableColumn& column)
  {
    if (active_)
      numErrors_ += SDK_ASSERT(table_.tableId() == table.tableId() && column.name() == columnName_);
  }

  virtual void onAddRow(DataTable& table, const simData::TableRow& row)
  {
    if (active_)
      numErrors_ += SDK_ASSERT(table_.tableId() == table.tableId() && row.time() == rowTime_);
  }

  virtual void onPreRemoveColumn(DataTable& table, const simData::TableColumn& column)
  {
    if (active_)
      numErrors_ += SDK_ASSERT(table_.tableId() == table.tableId() && column.name() == columnName_);
  }

  virtual void onPreRemoveRow(DataTable& table, double rowTime)
  {
    if (active_)
    {
      numErrors_ += SDK_ASSERT(table_.tableId() == table.tableId() && rowTime == removeRowTime_);
      if (rowTime != removeRowTime_)
        std::cerr << "  -- Expected " << removeRowTime_ << " but got " << rowTime << "\n";
    }
  }

  void setActive(bool active) { active_ = active; }

  int numErrors() const { return numErrors_; }

  void clearErrors() { numErrors_ = 0; }

  void setExpectedRemoveRowTime(double rowTime) { removeRowTime_ = rowTime; }

  void setExpectedRowTime(double rowTime) { rowTime_ = rowTime; }

  void setExpectedColumnName(const std::string& columnName) { columnName_ = columnName; }

private:
  bool active_;
  int numErrors_;
  double rowTime_;
  double removeRowTime_;
  simData::DataTable& table_;
  std::string columnName_;
};

int rowTest(TableRow& row)
{
  int rv = 0;

  // Time and empty-cell checks
  row.setTime(1.0);
  rv += SDK_ASSERT(row.time() == 1.0);
  row.setTime(2.0);
  rv += SDK_ASSERT(row.time() == 2.0);
  row.clear(); // should reset time
  rv += SDK_ASSERT(row.time() != 2.0);
  rv += SDK_ASSERT(row.cellCount() == 0);
  rv += SDK_ASSERT(!row.containsCell(0));
  rv += SDK_ASSERT(!row.containsCell(1));
  int intVal = 0;
  std::string strVal;
  rv += SDK_ASSERT(row.value(0, intVal).isError());
  rv += SDK_ASSERT(row.value(0, strVal).isError());
  rv += SDK_ASSERT(row.cellCount() == 0);

  // Set some values
  row.setValue(0, 6);
  row.setValue(2, "String");
  row.setValue(1, 7.0);
  row.setValue(5, static_cast<int8_t>(11));

  rv += SDK_ASSERT(row.cellCount() == 4);
  rv += SDK_ASSERT(row.containsCell(0));
  rv += SDK_ASSERT(row.containsCell(1));
  rv += SDK_ASSERT(row.containsCell(2));
  rv += SDK_ASSERT(!row.containsCell(3));
  rv += SDK_ASSERT(row.containsCell(5));
  rv += SDK_ASSERT(!row.containsCell(6));

  // Get all the values back as ints
  rv += SDK_ASSERT(row.value(0, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 6);
  rv += SDK_ASSERT(row.value(1, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 7);
  rv += SDK_ASSERT(row.value(2, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 0);
  rv += SDK_ASSERT(row.value(3, intVal).isError());
  rv += SDK_ASSERT(row.value(5, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 11);

  // Get all the values back as strings
  rv += SDK_ASSERT(row.value(0, strVal).isSuccess());
  rv += SDK_ASSERT(strVal == "6");
  rv += SDK_ASSERT(row.value(1, strVal).isSuccess());
  rv += SDK_ASSERT(!strVal.empty() && strVal[0] == '7');
  rv += SDK_ASSERT(row.value(2, strVal).isSuccess());
  rv += SDK_ASSERT(strVal == "String");
  rv += SDK_ASSERT(row.value(3, strVal).isError());
  rv += SDK_ASSERT(row.value(5, strVal).isSuccess());
  rv += SDK_ASSERT(strVal == "11");

  // clear out and make sure everything worked
  row.clear();
  rv += SDK_ASSERT(row.cellCount() == 0);
  rv += SDK_ASSERT(!row.containsCell(0));
  rv += SDK_ASSERT(!row.containsCell(1));

  // Add some extra data in to make sure destructor clears out memory
  row.setValue(1, "Longer string");
  row.setValue(3, 9);
  rv += SDK_ASSERT(row.cellCount() == 2);
  rv += SDK_ASSERT(row.containsCell(1));
  rv += SDK_ASSERT(!row.containsCell(2));
  rv += SDK_ASSERT(row.value(3, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 9);

  // Overwrite a value
  row.setValue(3, "11");
  rv += SDK_ASSERT(row.cellCount() == 2);
  rv += SDK_ASSERT(row.value(3, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 11);
  row.setValue(3, 12);  // Overwrite with different data type
  rv += SDK_ASSERT(row.cellCount() == 2);
  rv += SDK_ASSERT(row.value(3, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 12);
  row.setValue(3, 13); // Overwrite with same data type
  rv += SDK_ASSERT(row.cellCount() == 2);
  rv += SDK_ASSERT(row.value(3, intVal).isSuccess());
  rv += SDK_ASSERT(intVal == 13);

  return rv;
}

int managerTest(DataTableManager& mgr)
{
  int rv = 0;
  // Initial check
  rv += SDK_ASSERT(mgr.getTable(1) == NULL);
  rv += SDK_ASSERT(mgr.tableCount() == 0);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) == NULL);

  // add observer to test ManagerObserver
  // NOTE, since this has been wrapped in shared ptr we don't have to delete explicitly
  TestManagerObserver* testObserver = new TestManagerObserver("Foo");
  mgr.addObserver(simData::DataTableManager::ManagerObserverPtr(testObserver));

  // Add a single table
  simData::DataTable* table10Foo = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "Foo", &table10Foo).isSuccess());
  rv += SDK_ASSERT(table10Foo != NULL);
  rv += SDK_ASSERT(mgr.findTable(10, "Foo") == table10Foo);
  rv += SDK_ASSERT(mgr.findTable(11, "Foo") == NULL);
  rv += SDK_ASSERT(mgr.tableCount() == 1);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(10)->tableCount() == 1);

  // Add 2 more tables
  testObserver->setExpectedTableName("Bar");
  simData::DataTable* table10Bar = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "Bar", &table10Bar).isSuccess());
  simData::DataTable* table10Baz = NULL;
  testObserver->setExpectedTableName("Baz");
  rv += SDK_ASSERT(mgr.addDataTable(10, "Baz", &table10Baz).isSuccess());
  rv += SDK_ASSERT(table10Bar != NULL);
  rv += SDK_ASSERT(table10Baz != NULL);
  rv += SDK_ASSERT(mgr.tableCount() == 3);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(10)->tableCount() == 3);
  // They should all be different tables ...
  rv += SDK_ASSERT(table10Foo != table10Bar);
  rv += SDK_ASSERT(table10Baz != table10Bar);
  rv += SDK_ASSERT(table10Foo != table10Baz);
  // ... and different table IDs
  rv += SDK_ASSERT(table10Foo->tableId() != table10Bar->tableId());
  rv += SDK_ASSERT(table10Baz->tableId() != table10Bar->tableId());
  rv += SDK_ASSERT(table10Foo->tableId() != table10Baz->tableId());

  // Add a duplicate table name; should return error, but also return valid pointer
  simData::DataTable* table10FooDupe = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "Foo", &table10FooDupe).isError());
  simData::DataTable* table10BarDupe = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "Bar", &table10BarDupe).isError());
  simData::DataTable* table10BazDupe = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "Baz", &table10BazDupe).isError());
  rv += SDK_ASSERT(table10FooDupe == table10Foo);
  rv += SDK_ASSERT(table10BarDupe == table10Bar);
  rv += SDK_ASSERT(table10BazDupe == table10Baz);
  rv += SDK_ASSERT(mgr.tableCount() == 3);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(10)->tableCount() == 3);

  // Add empty string
  testObserver->setExpectedTableName("");
  simData::DataTable* emptyStringDT = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(10, "", &emptyStringDT).isError());
  rv += SDK_ASSERT(emptyStringDT == NULL);
  rv += SDK_ASSERT(mgr.tableCount() == 3);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(10)->tableCount() == 3);

  // Add 2 tables to another ID
  testObserver->setExpectedTableName("Foo");
  simData::DataTable* table11Foo = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(11, "Foo", &table11Foo).isSuccess());
  testObserver->setExpectedTableName("Bar");
  simData::DataTable* table11Bar = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(11, "Bar", &table11Bar).isSuccess());
  rv += SDK_ASSERT(table11Foo != NULL);
  rv += SDK_ASSERT(table11Bar != NULL);
  rv += SDK_ASSERT(mgr.tableCount() == 5);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(10)->tableCount() == 3);
  rv += SDK_ASSERT(mgr.tablesForOwner(11) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(11)->tableCount() == 2);

  // Test getTable()
  rv += SDK_ASSERT(mgr.getTable(table10Foo->tableId()) == table10Foo);
  rv += SDK_ASSERT(mgr.getTable(table10Bar->tableId()) == table10Bar);
  rv += SDK_ASSERT(mgr.getTable(table10Baz->tableId()) == table10Baz);
  rv += SDK_ASSERT(mgr.getTable(table11Foo->tableId()) == table11Foo);
  rv += SDK_ASSERT(mgr.getTable(table11Bar->tableId()) == table11Bar);
  // Test findTable()
  rv += SDK_ASSERT(mgr.findTable(10, "Foo") == table10Foo);
  rv += SDK_ASSERT(mgr.findTable(10, "Bar") == table10Bar);
  rv += SDK_ASSERT(mgr.findTable(10, "Baz") == table10Baz);
  rv += SDK_ASSERT(mgr.findTable(11, "Foo") == table11Foo);
  rv += SDK_ASSERT(mgr.findTable(11, "Bar") == table11Bar);
  rv += SDK_ASSERT(mgr.findTable(11, "Baz") == NULL);
  // Now's a good time to test owner IDs
  rv += SDK_ASSERT(table10Foo->ownerId() == 10);
  rv += SDK_ASSERT(table10Bar->ownerId() == 10);
  rv += SDK_ASSERT(table10Baz->ownerId() == 10);
  rv += SDK_ASSERT(table11Foo->ownerId() == 11);
  rv += SDK_ASSERT(table11Bar->ownerId() == 11);

  // Test the table lists
  const TableList* table10List = mgr.tablesForOwner(10);
  const TableList* table11List = mgr.tablesForOwner(11);
  const TableList* table12List = mgr.tablesForOwner(12);
  rv += SDK_ASSERT(table10List != NULL);
  rv += SDK_ASSERT(table11List != NULL);
  rv += SDK_ASSERT(table12List == NULL);
  rv += SDK_ASSERT(table10List->findTable("Foo") == table10Foo);
  rv += SDK_ASSERT(table10List->findTable("Bar") == table10Bar);
  rv += SDK_ASSERT(table10List->findTable("Baz") == table10Baz);
  rv += SDK_ASSERT(table11List->findTable("Foo") == table11Foo);
  rv += SDK_ASSERT(table11List->findTable("Bar") == table11Bar);
  rv += SDK_ASSERT(table11List->findTable("Baz") == NULL);
  rv += SDK_ASSERT(table10List->ownerId() == 10);
  rv += SDK_ASSERT(table11List->ownerId() == 11);
  rv += SDK_ASSERT(table10List->tableCount() == 3);
  rv += SDK_ASSERT(table11List->tableCount() == 2);

  // Test deleteTable, ensure it doesn't reorder IDs
  TableId table10FooId = table10Foo->tableId();
  TableId table10BarId = table10Bar->tableId();
  TableId table10BazId = table10Baz->tableId();
  testObserver->setExpectedTableName("Bar"); // set observer to prepare for Bar delete
  rv += SDK_ASSERT(mgr.deleteTable(table10BarId).isSuccess());
  rv += SDK_ASSERT(mgr.tableCount() == 4);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(11) != NULL);
  rv += SDK_ASSERT(table10List->tableCount() == 2);
  rv += SDK_ASSERT(table11List->tableCount() == 2);
  rv += SDK_ASSERT(table10Foo->tableId() == table10FooId);
  rv += SDK_ASSERT(table10Baz->tableId() == table10BazId);
  rv += SDK_ASSERT(mgr.getTable(table10FooId) == table10Foo);
  rv += SDK_ASSERT(mgr.getTable(table10BarId) == NULL);
  rv += SDK_ASSERT(mgr.getTable(table10BazId) == table10Baz);

  // set owner id for deleteTablesByOwner test
  testObserver->setExpectedOwnerId(11);

  // Test deleteByOwner
  mgr.deleteTablesByOwner(11);
  rv += SDK_ASSERT(mgr.tableCount() == 2);
  rv += SDK_ASSERT(mgr.tablesForOwner(10) != NULL);
  rv += SDK_ASSERT(mgr.tablesForOwner(11) == NULL);
  rv += SDK_ASSERT(table10List->tableCount() == 2);
  rv += SDK_ASSERT(table10Foo->tableId() == table10FooId);
  rv += SDK_ASSERT(table10Baz->tableId() == table10BazId);
  rv += SDK_ASSERT(mgr.getTable(table10FooId) == table10Foo);
  rv += SDK_ASSERT(mgr.getTable(table10BarId) == NULL);
  rv += SDK_ASSERT(mgr.getTable(table10BazId) == table10Baz);

  // Test a double delete
  rv += SDK_ASSERT(mgr.deleteTable(table10BarId).isError());
  rv += SDK_ASSERT(mgr.tableCount() == 2);

  // see if observer had any errors
  rv += testObserver->numErrors();
  testObserver->setActive(false);

  return rv;
}

int timeContainerTest(MemoryTable::TimeContainer& times)
{
  int rv = 0;
  rv += SDK_ASSERT(times.empty());
  rv += SDK_ASSERT(times.size() == 0);
  rv += SDK_ASSERT(times.findOrAddTime(20).hasNext());
  rv += SDK_ASSERT(!times.empty());
  rv += SDK_ASSERT(times.size() == 1);
  MemoryTable::TimeContainer* clone = times.clone();
  rv += SDK_ASSERT(!clone->empty());
  rv += SDK_ASSERT(clone->size() == 1);
  rv += SDK_ASSERT(times.findOrAddTime(40).hasNext());
  rv += SDK_ASSERT(times.findOrAddTime(30).hasNext());
  rv += SDK_ASSERT(times.findOrAddTime(10).hasNext());
  rv += SDK_ASSERT(!times.empty());
  rv += SDK_ASSERT(times.size() == 4);
  rv += SDK_ASSERT(!clone->empty());
  rv += SDK_ASSERT(clone->size() == 1);

  // Test flush doesn't affect other
  clone->flush();
  rv += SDK_ASSERT(clone->empty());
  rv += SDK_ASSERT(clone->size() == 0);
  // Test deletion with a single point
  clone->findOrAddTime(15.0);
  clone->findOrAddTime(15.0); // dupe point
  rv += SDK_ASSERT(!clone->empty());
  rv += SDK_ASSERT(clone->size() == 1);
  rv += SDK_ASSERT(simCore::areEqual(clone->find(15).next(), 15.0));
  delete clone;
  rv += SDK_ASSERT(times.size() == 4);
  // Reclone
  clone = times.clone();
  rv += SDK_ASSERT(!clone->empty());
  rv += SDK_ASSERT(clone->size() == 4);

  // Test find()
  rv += SDK_ASSERT(simCore::areEqual(clone->find(10).next(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->find(40).next(), 40.0));
  rv += SDK_ASSERT(!clone->find(15).hasNext());
  rv += SDK_ASSERT(!clone->find(9).hasNext());
  rv += SDK_ASSERT(!clone->find(49).hasNext());

  // Test front/back of iterators
  MemoryTable::TimeContainer::Iterator iter = clone->end();
  rv += SDK_ASSERT(iter.hasPrevious());
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekPrevious(), 40.0));
  iter.toFront();
  rv += SDK_ASSERT(!iter.hasPrevious());
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext(), 10.0));
  iter.toBack();
  rv += SDK_ASSERT(iter.hasPrevious());
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekPrevious(), 40.0));

  // Test previous/next through a single iterator
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 10.0));
  rv += SDK_ASSERT(!simCore::areEqual(iter.previous(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.next(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.next(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.next(), 40.0));
  rv += SDK_ASSERT(!simCore::areEqual(iter.next(), 40.0));
  // Test a reversal in the middle
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.next(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(iter.previous(), 30.0));

  // Test begin()
  rv += SDK_ASSERT(clone->begin().hasNext());
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().peekNext(), 10.0));
  rv += SDK_ASSERT(!clone->begin().hasPrevious());

  // Test end()
  rv += SDK_ASSERT(clone->end().hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().peekPrevious(), 40.0));
  rv += SDK_ASSERT(!clone->end().hasNext());

  // Test lower_bound()
  rv += SDK_ASSERT(!clone->lower_bound(9.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(9.0).next(), 10.0));
  rv += SDK_ASSERT(!clone->lower_bound(10.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(10.0).next(), 10.0));
  rv += SDK_ASSERT(clone->lower_bound(11.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(11.0).previous(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(11.0).next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(40.0).previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(40.0).next(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->lower_bound(41.0).previous(), 40.0));
  rv += SDK_ASSERT(!clone->lower_bound(41.0).hasNext());

  // Test upper_bound()
  rv += SDK_ASSERT(!clone->upper_bound(9.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(9.0).next(), 10.0));
  rv += SDK_ASSERT(clone->upper_bound(10.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(10.0).previous(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(10.0).next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(11.0).previous(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(11.0).next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(39.0).previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(39.0).next(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(40.0).previous(), 40.0));
  rv += SDK_ASSERT(!clone->upper_bound(40.0).hasNext());
  rv += SDK_ASSERT(simCore::areEqual(clone->upper_bound(41.0).previous(), 40.0));
  rv += SDK_ASSERT(!clone->upper_bound(41.0).hasNext());

  // Test findTimeAtOrBeforeGivenTime()
  rv += SDK_ASSERT(clone->findTimeAtOrBeforeGivenTime(9.0).hasPrevious());
  rv += SDK_ASSERT(!clone->findTimeAtOrBeforeGivenTime(10.0).hasPrevious());
  // Note: if it's before any time in the vec, we should be returning end()
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(9.0).previous(), 40.0));
  rv += SDK_ASSERT(!clone->findTimeAtOrBeforeGivenTime(10.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(10.0).next(), 10.0));
  rv += SDK_ASSERT(!clone->findTimeAtOrBeforeGivenTime(11.0).hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(11.0).next(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(20.0).previous(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(20.0).next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(40.0).previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(40.0).next(), 40.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(41.0).previous(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->findTimeAtOrBeforeGivenTime(41.0).next(), 40.0));

  // Add more times in preparation for erase()
  clone->findOrAddTime(50.0);
  clone->findOrAddTime(60.0);
  clone->findOrAddTime(70.0);
  clone->findOrAddTime(80.0);
  clone->findOrAddTime(90.0);
  clone->findOrAddTime(100.0);
  rv += SDK_ASSERT(clone->size() == 10);
  // Erase the last item by itself
  MemoryTable::TimeContainer::Iterator lastItem = clone->end();
  rv += SDK_ASSERT(simCore::areEqual(lastItem.previous(), 100.0));
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS);
  rv += SDK_ASSERT(clone->size() == 9);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));

  // Erase the first item
  clone->erase(clone->begin(), MemoryTable::TimeContainer::ERASE_FIXOFFSETS);
  rv += SDK_ASSERT(clone->size() == 8);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));

  // Erase one that is three from the front
  lastItem = clone->begin();
  lastItem.next(); // points to 30 next
  lastItem.next(); // points to 40 next
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS); // get rid of 40; [20,30,50,60,70,80,90]
  rv += SDK_ASSERT(clone->size() == 7);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));

  // Remove 3 at the end
  lastItem = clone->end();
  lastItem.previous(); // next is 90
  lastItem.previous(); // next is 80
  lastItem.previous(); // next is 70
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS); // get rid of 70; [20,30,50,60,80,90]
  rv += SDK_ASSERT(clone->size() == 6);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));

  // Remove 2 more from the back
  lastItem = clone->end();
  lastItem.previous(); // next is 90
  lastItem.previous(); // next is 80
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS); // get rid of 80; [20,30,50,60,90]
  rv += SDK_ASSERT(clone->size() == 5);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));
  // Test that erase(end) does nothing
  lastItem = clone->end();
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS); // no-op
  rv += SDK_ASSERT(clone->size() == 5);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 90.0));
  lastItem.previous(); // next is 90
  clone->erase(lastItem, MemoryTable::TimeContainer::ERASE_FIXOFFSETS); // get rid of 90; [20,30,50,60]
  rv += SDK_ASSERT(clone->size() == 4);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 60.0));

  // Erase 2 in front
  clone->erase(clone->begin(), MemoryTable::TimeContainer::ERASE_FIXOFFSETS);
  clone->erase(clone->begin(), MemoryTable::TimeContainer::ERASE_FIXOFFSETS);
  rv += SDK_ASSERT(clone->size() == 2);
  rv += SDK_ASSERT(simCore::areEqual(clone->begin().next(), 50.0));
  rv += SDK_ASSERT(simCore::areEqual(clone->end().previous(), 60.0));

  delete clone;
  return rv;
}

int tableTest(simData::DataTable& table)
{
  int rv = 0;
  simData::TableColumn* column1 = NULL;
  simData::TableColumn* column2 = NULL;
  simData::TableColumn* column3 = NULL;

  // create test observer. NOTE: since this is wrapped in a shared ptr, don't need to delete explicitly
  TestTableObserver* testObserver = new TestTableObserver(table);
  table.addObserver(simData::DataTable::TableObserverPtr(testObserver));

  // Empty column name is an error
  rv += SDK_ASSERT(table.columnCount() == 0);
  rv += SDK_ASSERT(table.addColumn("", VT_INT32, 0, &column1).isError());
  rv += SDK_ASSERT(column1 == NULL);
  testObserver->setExpectedColumnName("1");
  rv += SDK_ASSERT(table.addColumn("1", VT_INT32, 0, &column1).isSuccess());
  rv += SDK_ASSERT(column1 != NULL);
  // Duplicate name is an error
  rv += SDK_ASSERT(table.addColumn("1", VT_INT32, 0, &column2).isError());
  rv += SDK_ASSERT(column2 == column1); // Should point to column 1, even though there's an error
  testObserver->setExpectedColumnName("2");
  rv += SDK_ASSERT(table.addColumn("2", VT_INT32, 0, &column2).isSuccess());
  // Sanity checks
  rv += SDK_ASSERT(column2 != column1);
  rv += SDK_ASSERT(column2 != NULL);
  rv += SDK_ASSERT(column1->columnId() != column2->columnId());
  rv += SDK_ASSERT(column1->name() == "1");
  rv += SDK_ASSERT(column2->name() == "2");
  rv += SDK_ASSERT(table.columnCount() == 2);

  rv += SDK_ASSERT(table.column("1") == column1);
  rv += SDK_ASSERT(table.column("2") == column2);
  rv += SDK_ASSERT(table.column("3") == NULL);
  rv += SDK_ASSERT(table.column(column1->columnId()) == column1);
  rv += SDK_ASSERT(table.column(column2->columnId()) == column2);
  rv += SDK_ASSERT(table.column(500) == NULL); // Should be unique

  // Add another column
  testObserver->setExpectedColumnName("3");
  rv += SDK_ASSERT(table.addColumn("3", VT_INT32, 0, &column3).isSuccess());
  rv += SDK_ASSERT(column3 != column1);
  rv += SDK_ASSERT(column3 != column2);
  rv += SDK_ASSERT(column3 != NULL);
  // Store the column IDs to check that they don't change over the next few tests
  simData::TableColumnId col1Id = column1->columnId();
  simData::TableColumnId col2Id = column2->columnId();
  simData::TableColumnId col3Id = column3->columnId();
  rv += SDK_ASSERT(col3Id != col1Id);
  rv += SDK_ASSERT(col3Id != col2Id);

  // Remove column 2
  testObserver->setExpectedColumnName("2");
  rv += SDK_ASSERT(table.removeColumn("2").isSuccess());
  // Verify that the column is in fact removed
  rv += SDK_ASSERT(table.column(col2Id) == NULL);
  rv += SDK_ASSERT(table.column("2") == NULL);

  // Removing a column should not affect the other columns
  rv += SDK_ASSERT(col1Id == column1->columnId());
  rv += SDK_ASSERT(col3Id == column3->columnId());
  rv += SDK_ASSERT(table.column(column1->columnId()) == column1);
  rv += SDK_ASSERT(table.column(column3->columnId()) == column3);
  rv += SDK_ASSERT(table.column("1") == column1);
  rv += SDK_ASSERT(table.column("3") == column3);
  rv += SDK_ASSERT(table.columnCount() == 2);

  // Replacing column 2
  rv += SDK_ASSERT(table.addColumn("2", VT_INT32, 0, &column2).isSuccess());
  rv += SDK_ASSERT(column2 != column1);
  rv += SDK_ASSERT(column2 != column3);
  rv += SDK_ASSERT(column2 != NULL);
  // Should not reuse the ID of removed column
  rv += SDK_ASSERT(col2Id != column2->columnId());
  col2Id = column2->columnId();
  rv += SDK_ASSERT(col2Id != col1Id);
  rv += SDK_ASSERT(col2Id != col3Id);
  // Replacing a column should not affect the other columns
  rv += SDK_ASSERT(col1Id == column1->columnId());
  rv += SDK_ASSERT(col3Id == column3->columnId());
  rv += SDK_ASSERT(table.column(column1->columnId()) == column1);
  rv += SDK_ASSERT(table.column(column3->columnId()) == column3);
  rv += SDK_ASSERT(table.column("1") == column1);
  rv += SDK_ASSERT(table.column("3") == column3);
  rv += SDK_ASSERT(table.columnCount() == 3);

  // Should be no times
  double begin;
  double end;
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) != 0);
  rv += SDK_ASSERT(begin == 0.0);
  rv += SDK_ASSERT(end == 0.0);

  // Start to add cells
  TableRow row;
  row.setTime(10.0);
  row.setValue(column1->columnId(), 1001);
  row.setValue(column2->columnId(), 1002);
  row.setValue(column3->columnId(), 1003);
  testObserver->setExpectedRowTime(10.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 10.0);
  rv += SDK_ASSERT(end == 10.0);

  row = TableRow();
  row.setTime(20.0);
  row.setValue(column1->columnId(), 2001.0);
  row.setValue(column2->columnId(), 2002.0);
  row.setValue(column3->columnId(), 2003.0);
  testObserver->setExpectedRowTime(20.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 10.0);
  rv += SDK_ASSERT(end == 20.0);

  // Adding empty row should be an error (and not leak memory)
  row.clear();
  rv += SDK_ASSERT(table.addRow(row).isError());
  // Adding row with only time should also be an error (and not leak)
  row.setTime(30.0);
  rv += SDK_ASSERT(table.addRow(row).isError());
  // Add one more good row
  row.setValue(column1->columnId(), 3001.0);
  row.setValue(column2->columnId(), 3002.0);
  row.setValue(column3->columnId(), 3003.0);
  testObserver->setExpectedRowTime(30.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());

  // Sanity check state
  rv += SDK_ASSERT(table.columnCount() == 3);

  // Check interpolate on given values
  double value = 0;
  rv += SDK_ASSERT(column1->interpolate(value, 10.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 1001.0));
  rv += SDK_ASSERT(column1->interpolate(value, 20.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2001.0));
  rv += SDK_ASSERT(column1->interpolate(value, 30.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3001.0));
  // Actually interpolate
  rv += SDK_ASSERT(column1->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2501.0));
  // Check that extrapolation fails before time, and succeeds with current value after time
  rv += SDK_ASSERT(column1->interpolate(value, 5.0, NULL).isError());
  rv += SDK_ASSERT(column1->interpolate(value, 35.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3001.0));

  // Now with column 2
  rv += SDK_ASSERT(column2->interpolate(value, 10.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 1002.0));
  rv += SDK_ASSERT(column2->interpolate(value, 20.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2002.0));
  rv += SDK_ASSERT(column2->interpolate(value, 30.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3002.0));
  // Actually interpolate
  rv += SDK_ASSERT(column2->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2502.0));
  // Check that extrapolation fails before time, and succeeds with current value after time
  rv += SDK_ASSERT(column2->interpolate(value, 5.0, NULL).isError());
  rv += SDK_ASSERT(column2->interpolate(value, 35.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3002.0));

  // Now with column 3
  rv += SDK_ASSERT(column3->interpolate(value, 10.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 1003.0));
  rv += SDK_ASSERT(column3->interpolate(value, 20.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2003.0));
  rv += SDK_ASSERT(column3->interpolate(value, 30.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3003.0));
  // Actually interpolate
  rv += SDK_ASSERT(column3->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2503.0));
  // Check that extrapolation fails before time, and succeeds with current value after time
  rv += SDK_ASSERT(column3->interpolate(value, 5.0, NULL).isError());
  rv += SDK_ASSERT(column3->interpolate(value, 35.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 3003.0));

  // Add 3 new columns
  simData::TableColumn* column4 = NULL;
  simData::TableColumn* column5 = NULL;
  simData::TableColumn* column6 = NULL;
  testObserver->setExpectedColumnName("4");
  rv += SDK_ASSERT(table.addColumn("4", VT_UINT32, 0, &column4).isSuccess());
  testObserver->setExpectedColumnName("5");
  rv += SDK_ASSERT(table.addColumn("5", VT_STRING, 0, &column5).isSuccess());
  rv += SDK_ASSERT(table.columnCount() == 5);
  testObserver->setExpectedColumnName("6");
  rv += SDK_ASSERT(table.addColumn("6", VT_INT16, 0, &column6).isSuccess());

  // Add values to 5 of the 6 columns, out of order
  row.clear();
  row.setTime(50.0);
  row.setValue(column2->columnId(), 5002.0);
  row.setValue(column3->columnId(), 5003.0);
  row.setValue(column4->columnId(), 5004.0);
  row.setValue(column5->columnId(), 5005.0);
  row.setValue(column6->columnId(), 5006.0);
  testObserver->setExpectedRowTime(50.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());
  row.clear();
  row.setTime(0.0);
  row.setValue(column2->columnId(), 0002.0);
  row.setValue(column3->columnId(), 0003.0);
  row.setValue(column4->columnId(), 0004.0);
  row.setValue(column5->columnId(), 0005.0);
  row.setValue(column6->columnId(), 0006.0);
  testObserver->setExpectedRowTime(0.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());

  // Spot check interpolation on all columns at time 25
  rv += SDK_ASSERT(column1->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2501.0));
  rv += SDK_ASSERT(column2->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2502.0));
  rv += SDK_ASSERT(column3->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2503.0));
  rv += SDK_ASSERT(column4->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2504.0));
  rv += SDK_ASSERT(column5->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2505.0));
  rv += SDK_ASSERT(column6->interpolate(value, 25.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 2506.0));

  // Store column IDs to check for changes in later steps
  simData::TableColumnId col4Id = column4->columnId();
  simData::TableColumnId col5Id = column5->columnId();
  simData::TableColumnId col6Id = column6->columnId();

  // Remove column 3
  testObserver->setExpectedColumnName("3");
  rv += SDK_ASSERT(table.removeColumn("3").isSuccess());
  // Verify that the column is in fact removed
  rv += SDK_ASSERT(table.column(col3Id) == NULL);
  rv += SDK_ASSERT(table.column("3") == NULL);

  // Check that other columns are not affected, especially columns 1 and 2 which are in the same subtable at this point
  rv += SDK_ASSERT(col1Id == column1->columnId());
  rv += SDK_ASSERT(col2Id == column2->columnId());
  rv += SDK_ASSERT(col4Id == column4->columnId());
  rv += SDK_ASSERT(col5Id == column5->columnId());
  rv += SDK_ASSERT(col6Id == column6->columnId());
  rv += SDK_ASSERT(table.column(column1->columnId()) == column1);
  rv += SDK_ASSERT(table.column(column2->columnId()) == column2);
  rv += SDK_ASSERT(table.column(column4->columnId()) == column4);
  rv += SDK_ASSERT(table.column(column5->columnId()) == column5);
  rv += SDK_ASSERT(table.column(column6->columnId()) == column6);
  rv += SDK_ASSERT(table.column("1") == column1);
  rv += SDK_ASSERT(table.column("2") == column2);
  rv += SDK_ASSERT(table.column("4") == column4);
  rv += SDK_ASSERT(table.column("5") == column5);
  rv += SDK_ASSERT(table.column("6") == column6);
  rv += SDK_ASSERT(table.columnCount() == 5);

  // Add data to the first column, then the second column, and make sure we can find
  // the data in both cases.  This validates that the split doesn't lose subtable pointers.
  row.clear();
  row.setTime(80.0);
  row.setValue(column1->columnId(), 123);
  testObserver->setExpectedRowTime(80.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());
  row.clear();
  row.setTime(85.0);
  row.setValue(column4->columnId(), 321);
  testObserver->setExpectedRowTime(85.0);
  rv += SDK_ASSERT(table.addRow(row).isSuccess());
  // Now check those values...
  rv += SDK_ASSERT(column1->interpolate(value, 80.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 123.0));
  rv += SDK_ASSERT(column4->interpolate(value, 85.0, NULL).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(value, 321.0));

  // did our observer get any errors?
  rv += testObserver->numErrors();

  return rv;
}

int flushTest(simData::DataTable& table)
{
  int rv = 0;
  rv += SDK_ASSERT(table.columnCount() != 0);

  // Define a class to figure out the amount of data in each column for the table
  class SizeCounter : public DataTable::ColumnVisitor
  {
  public:
    SizeCounter() : size_(0), numColumns_(0) {}
    virtual void visit(TableColumn* column)
    {
      numColumns_++;
      size_ += column->size();
    }
    void clear() { size_ = 0; numColumns_ = 0; }
    size_t size() const { return size_; }
    size_t numColumns() const { return numColumns_; }
  private:
    size_t size_;
    size_t numColumns_;
  };

  SizeCounter numDataValues;
  table.accept(numDataValues);
  rv += SDK_ASSERT(numDataValues.numColumns() == table.columnCount());
  rv += SDK_ASSERT(numDataValues.size() > 0);
  numDataValues.clear();

  simData::DelayedFlushContainerPtr delayedFlush = table.flush();
  table.accept(numDataValues);
  rv += SDK_ASSERT(numDataValues.numColumns() == table.columnCount());
  rv += SDK_ASSERT(numDataValues.size() == 0);
  numDataValues.clear();

  // This could fail if the container does not implement delayed flush
  rv += SDK_ASSERT(delayedFlush.get() != NULL);

  // Clear out the memory and make sure it's still all 0
  delayedFlush.reset();
  table.accept(numDataValues);
  rv += SDK_ASSERT(numDataValues.numColumns() == table.columnCount());
  rv += SDK_ASSERT(numDataValues.size() == 0);
  numDataValues.clear();
  rv += SDK_ASSERT(delayedFlush.get() == NULL);
  return rv;
}

// dataLimitingTest() misses some testing on the by-seconds testing that this function addresses; white box testing
int dataLimitSecondsTest()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  uint64_t plat1 = testHelper.addPlatform();
  ds->setDataLimiting(true);
  simData::DataStore::Transaction t;
  simData::PlatformPrefs* prefs = ds->mutable_platformPrefs(plat1, &t);
  prefs->mutable_commonprefs()->set_datalimittime(5.0); // limiting to 5 seconds
  t.commit();

  simData::DataTable* table = NULL;
  rv += SDK_ASSERT(ds->dataTableManager().addDataTable(plat1, "Data Limit Test Table", &table).isSuccess());

  // add some columns
  simData::TableColumn* column1 = NULL;
  simData::TableColumn* column2 = NULL;
  rv += SDK_ASSERT(table->addColumn("1", VT_INT32, 0, &column1).isSuccess());
  rv += SDK_ASSERT(table->addColumn("2", VT_INT64, 0, &column2).isSuccess());

  // add some rows
  simData::TableRow newRow;
  // Time: 1.0
  newRow.setTime(1.0);
  newRow.setValue(column1->columnId(), 40);
  newRow.setValue(column2->columnId(), 4000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 1);
  // Stale: empty; Fresh: 1.0

  // Time: 5.0
  newRow.setTime(5.0);
  newRow.setValue(column1->columnId(), 50);
  newRow.setValue(column2->columnId(), 5000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 2);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(1.0).next()->time() == 1.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(5.0).next()->time() == 5.0);
  // Stale: empty; Fresh: 1.0, 5.0

  // Time: 9.0
  newRow.setTime(9.0);
  newRow.setValue(column1->columnId(), 90);
  newRow.setValue(column2->columnId(), 9000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 3);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(1.0).next()->time() == 1.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(9.0).next()->time() == 9.0);
  // Stale: 1.0, 5.0, 9.0; Fresh: empty

  // Time: 13.0
  newRow.setTime(13.0);
  newRow.setValue(column1->columnId(), 130);
  newRow.setValue(column2->columnId(), 13000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 4);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(1.0).next()->time() == 1.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(13.0).next()->time() == 13.0);
  // Stale: 1.0, 5.0, 9.0; Fresh: 13.0

  // Time: 17.0
  newRow.setTime(17.0);
  newRow.setValue(column1->columnId(), 170);
  newRow.setValue(column2->columnId(), 17000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 5);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(1.0).next()->time() == 1.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(17.0).next()->time() == 17.0);
  // Stale: 1.0, 5.0, 9.0; Fresh: 13.0, 17.0

  // Time: 21.0
  newRow.setTime(21.0);
  newRow.setValue(column1->columnId(), 210);
  newRow.setValue(column2->columnId(), 21000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 3);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(13.0).next()->time() == 13.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(21.0).next()->time() == 21.0);
  // Stale: 13.0, 17.0, 21.0; Fresh: empty

  // Time: 25.0
  newRow.setTime(25.0);
  newRow.setValue(column1->columnId(), 250);
  newRow.setValue(column2->columnId(), 25000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 4);
  // Check each expected time value
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(13.0).next()->time() == 13.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(17.0).next()->time() == 17.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(21.0).next()->time() == 21.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(25.0).next()->time() == 25.0);
  // In addition to checking times, just double check that the data column values
  // also kept sync with the limiting on the time list.
  int value = 0;
  rv += SDK_ASSERT(column1->begin().next()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 130);
  rv += SDK_ASSERT(column1->end().previous()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 250);
  // Stale: 13.0, 17.0, 21.0; Fresh: 25.0

  // Insert a value before 25 to check that nothing happens bad
  newRow.setTime(23.0);
  newRow.setValue(column1->columnId(), 230);
  newRow.setValue(column2->columnId(), 23000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 5);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(13.0).next()->time() == 13.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(25.0).next()->time() == 25.0);
  // Make sure .rbegin (equivalent) points to time 25 still
  rv += SDK_ASSERT(column1->end().previous()->time() == 25.0);
  // Stale: 13.0, 17.0, 21.0; Fresh: 23.0, 25.0

  // Insert a value that will trigger a flip, EARLIER than the 25
  newRow.setTime(19.0);
  newRow.setValue(column1->columnId(), 190);
  newRow.setValue(column2->columnId(), 19000);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 3);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(19.0).next()->time() == 19.0);
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(25.0).next()->time() == 25.0);
  // Make sure .rbegin (equivalent) points to time 25 still, and .begin (equivalent) is 19
  rv += SDK_ASSERT(column1->begin().next()->time() == 19.0);
  rv += SDK_ASSERT(column1->end().previous()->time() == 25.0);
  // Stale: 19.0, 23.0, 25.0; Fresh: empty

  return rv;
}

int dataLimitingTest()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  uint64_t plat1 = testHelper.addPlatform();
  ds->setDataLimiting(true);
  simData::DataStore::Transaction t;
  simData::PlatformPrefs* prefs = ds->mutable_platformPrefs(plat1, &t);
  prefs->mutable_commonprefs()->set_datalimitpoints(3); // start out limiting to 3 points
  // Note that a data limit of 4 points would be equivalent, due to the divide-by-zero-and-round-up
  // algorithm that is employed in the double buffer time container.
  t.commit();

  simData::DataTable* table = NULL;
  rv += SDK_ASSERT(ds->dataTableManager().addDataTable(plat1, "Data Limit Test Table", &table).isSuccess());

  // create test observer. NOTE: since this is wrapped in a shared ptr, don't need to delete explicitly (but we do want to test removeObserver()
  TestTableObserver* testObserver = new TestTableObserver(*table);
  simData::DataTable::TableObserverPtr testObserverPtr(testObserver);
  table->addObserver(testObserverPtr);

  // add some columns
  simData::TableColumn* column1 = NULL;
  simData::TableColumn* column2 = NULL;
  testObserver->setExpectedColumnName("1");
  rv += SDK_ASSERT(table->addColumn("1", VT_INT32, 0, &column1).isSuccess());
  testObserver->setExpectedColumnName("2");
  rv += SDK_ASSERT(table->addColumn("2", VT_INT64, 0, &column2).isSuccess());

  // add some rows
  simData::TableRow newRow;
  newRow.setTime(1.0);
  newRow.setValue(column1->columnId(), 40);
  newRow.setValue(column2->columnId(), 4000);
  testObserver->setExpectedRowTime(1.0);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  newRow.clear();
  newRow.setTime(2.0);
  newRow.setValue(column1->columnId(), 50);
  newRow.setValue(column2->columnId(), 5000);
  testObserver->setExpectedRowTime(2.0);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // now add 3rd row, which should remove the first row, which should call the onPreRemoveRow in testObserver
  newRow.clear();
  newRow.setTime(3.0);
  newRow.setValue(column1->columnId(), 60);
  newRow.setValue(column2->columnId(), 6000);
  testObserver->setExpectedRowTime(3.0);
  testObserver->setExpectedRemoveRowTime(1.0);
  rv += SDK_ASSERT(column1->size() == 2);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  // check that we got a 1.0 (due to double buffer)
  rv += SDK_ASSERT(column1->size() == 3); // 1,2 in stale, 3 in fresh
  simData::TableColumn::Iterator iter = column1->findAtOrBeforeTime(1.0);
  rv += SDK_ASSERT(iter.hasNext() && iter.next()->time() == 1.0);
  rv += SDK_ASSERT(testObserver->numErrors() == 0);
  // Stale: 1,2; Fresh: 3

  // add another row, which goes into the fresh bin.  The stale bin (has 3 items)
  // gets emptied on the data limiting phase.  Afterwards, the "4" time will be
  // in the stale bin, and fresh bin will be empty
  newRow.clear();
  newRow.setTime(4.0);
  newRow.setValue(column1->columnId(), 70);
  newRow.setValue(column2->columnId(), 7000);
  testObserver->setExpectedRowTime(4.0);
  testObserver->setExpectedRemoveRowTime(1.0); // We really expect 1, 2, and 3 to be limited away
  std::cerr << "\nErrors here are OK (expecting to see 1 error):\n";
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  std::cerr << "----------------------------------------------\n";
  // check that we don't find time 1.0 or 2.0 in our column (they just got limited out)
  rv += SDK_ASSERT(!column1->findAtOrBeforeTime(2.0).hasNext());
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(3.0).hasNext());
  rv += SDK_ASSERT(column1->findAtOrBeforeTime(4.0).hasNext()); // But we do expect to find 4
  // We expected 2 errors (missing time 2.0); verify, and clear error log
  rv += SDK_ASSERT(testObserver->numErrors() == 1);
  testObserver->clearErrors();
  rv += SDK_ASSERT(column1->size() == 2);
  // Stale: 3,4; Fresh: empty

  // add a row whose time is earlier than all current rows, shouldn't matter because
  // it will go into the fresh bin and total will not trigger data limiting
  newRow.clear();
  newRow.setTime(2.0);
  newRow.setValue(column1->columnId(), 50);
  newRow.setValue(column2->columnId(), 5000);
  testObserver->setExpectedRowTime(2.0);
  // Note that we're at 1 item, so no rows removed
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 3);
  // Stale: 3,4;  Fresh: 2

  // now change data limiting from so time is more limiting than points
  prefs = ds->mutable_platformPrefs(plat1, &t);
  prefs->mutable_commonprefs()->set_datalimitpoints(7); // expand point limit to 7
  prefs->mutable_commonprefs()->set_datalimittime(3.0); // set data limit to 3 seconds
  t.commit();

  // add another row
  newRow.clear();
  newRow.setTime(5.0);
  newRow.setValue(column1->columnId(), 80);
  newRow.setValue(column2->columnId(), 8000);
  testObserver->setExpectedRowTime(5.0);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(testObserver->numErrors() == 0);
  rv += SDK_ASSERT(column1->size() == 4);
  // Stale: 3,4; Fresh: 2,5

  // add another row, initial row time is now 2.0, but it's in the fresh bin.  The stale
  // bin has 4, and that will be the only value limited away.
  newRow.clear();
  newRow.setTime(6.0);
  newRow.setValue(column1->columnId(), 90);
  newRow.setValue(column2->columnId(), 9000);
  testObserver->setExpectedRowTime(6.0);
  testObserver->setExpectedRemoveRowTime(4.0);
  std::cerr << "\nErrors here are OK (expecting to see 1 error):\n";
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  std::cerr << "----------------------------------------------\n";
  // Expecting error on removal of 3
  rv += SDK_ASSERT(testObserver->numErrors() == 1);
  testObserver->clearErrors();
  rv += SDK_ASSERT(column1->size() == 3);
  // check that we don't find time 4.0 in our column
  iter = column1->findAtOrBeforeTime(4.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(iter.next()->time() == 2.0); // 2 is first item before 4
  // Stale: 2, 5, 6; Fresh: empty

  // Insert a 5.5, which will go into the fresh bin.  Stale bin will get
  // NOT get cleared because data limit applies only to fresh bin
  newRow.clear();
  newRow.setTime(5.5);
  newRow.setValue(column1->columnId(), 85);
  newRow.setValue(column2->columnId(), 8500);
  testObserver->setExpectedRowTime(5.5);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(testObserver->numErrors() == 0);
  testObserver->clearErrors();
  // check that we still find time 5.5 in our column
  iter = column1->findAtOrBeforeTime(5.5);
  rv += SDK_ASSERT(iter.hasNext() && iter.peekNext()->time() == 5.5);
  rv += SDK_ASSERT(column1->size() == 4);
  // Stale: 2, 5, 6; Fresh: 5.5

  // now insert row at time 5.75
  newRow.clear();
  newRow.setTime(5.75);
  newRow.setValue(column1->columnId(), 875);
  newRow.setValue(column2->columnId(), 8750);
  testObserver->setExpectedRowTime(5.75);
  // No rows should be removed
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  rv += SDK_ASSERT(column1->size() == 5);
  rv += SDK_ASSERT(testObserver->numErrors() == 0);
  // check that we don't find anything before 5.5
  iter = column1->findAtOrBeforeTime(5.499);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(iter.next()->time() == 5.0);
  // Stale: 2, 5, 6; Fresh: 5.5, 5.75

  // add some more columns so we can cause a split
  simData::TableColumn* column3 = NULL;
  simData::TableColumn* column4 = NULL;
  testObserver->setExpectedColumnName("3");
  rv += SDK_ASSERT(table->addColumn("3", VT_INT8, 0, &column3).isSuccess());
  testObserver->setExpectedColumnName("4");
  rv += SDK_ASSERT(table->addColumn("4", VT_INT16, 0, &column4).isSuccess());

  // now add a row at 5.95, with all columns filled, which will cause a split.
  newRow.clear();
  newRow.setTime(5.95);
  newRow.setValue(column1->columnId(), 895);
  newRow.setValue(column2->columnId(), 8950);
  newRow.setValue(column3->columnId(), 8);
  newRow.setValue(column4->columnId(), 89);
  testObserver->setExpectedRowTime(5.95);
  // No removals
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // collect any errors from our test observer
  rv += testObserver->numErrors();

  // Remove it once...
  table->removeObserver(testObserverPtr);
  // Then remove it again to test a theory that double remove was causing problems
  table->removeObserver(testObserverPtr);

  return rv;
};

int getTimeRangeTest()
{
  int rv = 0;

  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  uint64_t plat1 = testHelper.addPlatform();
  ds->setDataLimiting(true);
  simData::DataStore::Transaction t;
  simData::PlatformPrefs* prefs = ds->mutable_platformPrefs(plat1, &t);
  prefs->mutable_commonprefs()->set_datalimitpoints(6); // start out limiting to 6 points
  t.commit();

  simData::DataTable* table = NULL;
  rv += SDK_ASSERT(ds->dataTableManager().addDataTable(plat1, "Data Limit Test Table", &table).isSuccess());

  // Add a column
  simData::TableColumn* column1 = NULL;
  rv += SDK_ASSERT(table->addColumn("1", VT_INT32, 0, &column1).isSuccess());

  // add some rows
  simData::TableRow newRow;
  newRow.setTime(1.0);
  newRow.setValue(column1->columnId(), 40);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  newRow.clear();
  newRow.setTime(2.0);
  newRow.setValue(column1->columnId(), 50);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // Verify expected results from getTimeRange() (all data in fresh)
  double begin;
  double end;
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.0);
  rv += SDK_ASSERT(end == 2.0);

  newRow.clear();
  newRow.setTime(3.0);
  newRow.setValue(column1->columnId(), 60);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  // Verify expected results from getTimeRange() (all data in stale)
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.0);
  rv += SDK_ASSERT(end == 3.0);

  newRow.clear();
  newRow.setTime(4.0);
  newRow.setValue(column1->columnId(), 70);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  newRow.clear();
  newRow.setTime(5.0);
  newRow.setValue(column1->columnId(), 80);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // Verify expected results from getTimeRange() (data split between fresh and stale)
  rv += SDK_ASSERT(column1->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.0);
  rv += SDK_ASSERT(end == 5.0);

  // Test again, with data being added in reverse. Creates situation where the
  // DoubleBufferTimeContainer's FRESH bin has earlier times than the STALE bin
  simData::TableColumn* column2 = NULL;
  rv += SDK_ASSERT(table->addColumn("2", VT_INT32, 0, &column2).isSuccess());

  newRow.clear();
  newRow.setTime(5.0);
  newRow.setValue(column2->columnId(), 40);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  newRow.clear();
  newRow.setTime(4.0);
  newRow.setValue(column2->columnId(), 50);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // Verify expected results from getTimeRange() (all data in fresh)
  rv += SDK_ASSERT(column2->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 4.0);
  rv += SDK_ASSERT(end == 5.0);

  newRow.clear();
  newRow.setTime(3.0);
  newRow.setValue(column2->columnId(), 60);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  // Verify expected results from getTimeRange() (all data in stale)
  rv += SDK_ASSERT(column2->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 3.0);
  rv += SDK_ASSERT(end == 5.0);

  newRow.clear();
  newRow.setTime(2.0);
  newRow.setValue(column2->columnId(), 70);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());
  newRow.clear();
  newRow.setTime(1.0);
  newRow.setValue(column2->columnId(), 80);
  rv += SDK_ASSERT(table->addRow(newRow).isSuccess());

  // Verify expected results from getTimeRange() (data split between fresh and stale)
  rv += SDK_ASSERT(column2->getTimeRange(begin, end) == 0);
  rv += SDK_ASSERT(begin == 1.0);
  rv += SDK_ASSERT(end == 5.0);

  return rv;
}

int subTableIterationTest(simData::MemoryTable::TimeContainer* newTimeContainer)
{
  // Create a mini typedef to reduce typing
  typedef simData::MemoryTable::SubTable SubTable;
  SubTable subTable(newTimeContainer, 0);
  int rv = 0;

  // Create iterator on empty subtable and validate it
  SubTable::Iterator invalidIter = subTable.begin();
  rv += SDK_ASSERT(!invalidIter.hasNext());
  rv += SDK_ASSERT(!invalidIter.hasPrevious());
  invalidIter = subTable.end();
  rv += SDK_ASSERT(!invalidIter.hasNext());
  rv += SDK_ASSERT(!invalidIter.hasPrevious());

  // Add 5 rows of data for 3 columns; start column numbering at 4 (random)
  rv += SDK_ASSERT(subTable.addColumn("C4", 4, simData::VT_INT32, 0, NULL).isSuccess());
  rv += SDK_ASSERT(subTable.addColumn("C5", 5, simData::VT_STRING, 0, NULL).isSuccess());
  rv += SDK_ASSERT(subTable.addColumn("C6", 6, simData::VT_FLOAT, 0, NULL).isSuccess());
  // Create a helper class to ensure we don't split
  class NoSplit : public SubTable::SplitObserver
  {
  public:
    NoSplit() : gotSplit(false) {}
    virtual void notifySplit(SubTable* originalTable, SubTable* newTable, const std::vector<TableColumnId>& splitColumns)
    {
      gotSplit = true;
    }
    bool gotSplit;
  };
  NoSplit* noSplit = new NoSplit();  // Cleaned up by smart pointer
  SubTable::SplitObserverPtr splitObserver(static_cast<SubTable::SplitObserver*>(noSplit));
  for (int k = 1; k <= 5; ++k)
  {
    SubTable::AddRowTransactionPtr txn = subTable.addRow(k * 10, splitObserver);
    txn->setCellValue(4, 40 + k); // 41, 42, 43, 44, 45
    txn->setCellValue(6, 60 + k); // 61, 62, 63, 64, 65
    txn->setCellValue(5, 50 + k); // 51, 52, 53, 54, 55
    // I noticed a bug where setting a cell value more than once caused a split.  This tests that.
    txn->setCellValue(4, 40 + k); // 41, 42, 43, 44, 45
    // Auto-commit on destruction
  }
  // We shouldn't have gotten a split
  rv += SDK_ASSERT(!noSplit->gotSplit);
  rv += SDK_ASSERT(subTable.columnCount() == 3);
  rv += SDK_ASSERT(subTable.rowCount() == 5);

  // Now test various iteration methods; start with begin(), next, peekNext.
  SubTable::Iterator iter = subTable.begin();
  rv += SDK_ASSERT(!iter.hasPrevious());
  int lastTime = 0;
  while (iter.hasNext())
  {
    lastTime += 10;
    rv += SDK_ASSERT(iter.peekNext().time() == lastTime);
    TableRow row;
    row.setTime(lastTime - 1);
    iter.next().fillRow(row);
    // Assertion validates that the row time doesn't get set by fillRow()
    rv += SDK_ASSERT(row.time() == (lastTime - 1));
    for (simData::TableColumnId colId = 4; colId <= 6; ++colId)
    {
      rv += SDK_ASSERT(row.containsCell(colId));
      int64_t value;
      rv += SDK_ASSERT(row.value(colId, value).isSuccess());
      rv += SDK_ASSERT(value == (colId * 10 + lastTime / 10));
    }
  }
  rv += SDK_ASSERT(lastTime == 50);

  // Now iterate backwards
  SubTable::Iterator riter = subTable.end();
  rv += SDK_ASSERT(!riter.hasNext());
  lastTime = 60;
  while (iter.hasPrevious())
  {
    lastTime -= 10;
    rv += SDK_ASSERT(iter.peekPrevious().time() == lastTime);
    TableRow row;
    row.setTime(lastTime - 1);
    iter.previous().fillRow(row);
    // Assertion validates that the row time doesn't get set by fillRow()
    rv += SDK_ASSERT(row.time() == (lastTime - 1));
    for (simData::TableColumnId colId = 4; colId <= 6; ++colId)
    {
      rv += SDK_ASSERT(row.containsCell(colId));
      int64_t value;
      rv += SDK_ASSERT(row.value(colId, value).isSuccess());
      rv += SDK_ASSERT(value == (colId * 10 + lastTime / 10));
    }
  }
  rv += SDK_ASSERT(lastTime == 10);

  // Assume at this point that since iteration works, upper/lower bound tests can be relatively minimal
  iter = subTable.lower_bound(9);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 10.0);
  iter = subTable.lower_bound(10);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 10.0);
  iter = subTable.lower_bound(11);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 20.0);
  iter = subTable.lower_bound(49);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 50.0);
  iter = subTable.lower_bound(50);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 50.0);
  iter = subTable.lower_bound(51);
  rv += SDK_ASSERT(!iter.hasNext());
  // Upper bound
  iter = subTable.upper_bound(9);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 10.0);
  iter = subTable.upper_bound(10);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 20.0);
  iter = subTable.upper_bound(11);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 20.0);
  iter = subTable.upper_bound(49);
  rv += SDK_ASSERT(iter.hasNext() && iter.next().time() == 50.0);
  iter = subTable.upper_bound(50);
  rv += SDK_ASSERT(!iter.hasNext());
  iter = subTable.upper_bound(51);
  rv += SDK_ASSERT(!iter.hasNext());

  return rv;
}

class CheckDataVisitor : public DataTable::RowVisitor
{
public:
  CheckDataVisitor()
    : numErrors_(0),
      stopAtTime_(0.0),
      allowStops_(false)
  {
  }
  explicit CheckDataVisitor(double stopAtTime)
    : numErrors_(0),
      stopAtTime_(stopAtTime),
      allowStops_(true)
  {
  }
  virtual VisitReturn visit(const TableRow& row)
  {
    numErrors_ += SDK_ASSERT(!hasTime(row.time()));
    timesVisited_.insert(row.time());

    // Check the data values
    //   10, --, 21, 31
    //   20, 12, --, 32
    //   30, 13, 23, --
    //   35, 40, 50, 40
    //   40, --, 24, 34
    //   50, 15, --, 35
    //   60, 16, 26, --
    int value;
    if (row.time() == 10)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(!row.containsCell(0));
      numErrors_ += SDK_ASSERT(row.value(1, value).isSuccess() && value == 21);
      numErrors_ += SDK_ASSERT(row.value(2, value).isSuccess() && value == 31);
    }
    else if (row.time() == 20)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(row.value(0, value).isSuccess() && value == 12);
      numErrors_ += SDK_ASSERT(!row.containsCell(1));
      numErrors_ += SDK_ASSERT(row.value(2, value).isSuccess() && value == 32);
    }
    else if (row.time() == 30)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(row.value(0, value).isSuccess() && value == 13);
      numErrors_ += SDK_ASSERT(row.value(1, value).isSuccess() && value == 23);
      numErrors_ += SDK_ASSERT(!row.containsCell(2));
    }
    else if (row.time() == 35)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 3);
      numErrors_ += SDK_ASSERT(row.value(0, value).isSuccess() && value == 40);
      numErrors_ += SDK_ASSERT(row.value(1, value).isSuccess() && value == 50);
      numErrors_ += SDK_ASSERT(row.value(2, value).isSuccess() && value == 40);
    }
    else if (row.time() == 40)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(!row.containsCell(0));
      numErrors_ += SDK_ASSERT(row.value(1, value).isSuccess() && value == 24);
      numErrors_ += SDK_ASSERT(row.value(2, value).isSuccess() && value == 34);
    }
    else if (row.time() == 50)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(row.value(0, value).isSuccess() && value == 15);
      numErrors_ += SDK_ASSERT(!row.containsCell(1));
      numErrors_ += SDK_ASSERT(row.value(2, value).isSuccess() && value == 35);
    }
    else if (row.time() == 60)
    {
      numErrors_ += SDK_ASSERT(row.cellCount() == 2);
      numErrors_ += SDK_ASSERT(row.value(0, value).isSuccess() && value == 16);
      numErrors_ += SDK_ASSERT(row.value(1, value).isSuccess() && value == 26);
      numErrors_ += SDK_ASSERT(!row.containsCell(2));
    }
    else
    {
      // Unexpected time
      numErrors_++;
    }

    if (allowStops_)
      return simCore::areEqual(row.time(), stopAtTime_) ? VISIT_STOP : VISIT_CONTINUE;
    return VISIT_CONTINUE;
  }
  void clear()
  {
    timesVisited_.clear();
    numErrors_ = 0;
  }
  int numErrors() const
  {
    return numErrors_;
  }
  bool hasTime(double t) const
  {
    return timesVisited_.find(t) != timesVisited_.end();
  }
  size_t numTimes() const
  {
    return timesVisited_.size();
  }
private:
  /// Contains all the time values we hit
  std::set<double> timesVisited_;
  int numErrors_;
  double stopAtTime_;
  bool allowStops_;
};

int testRowIteration(simData::DataTable& table)
{
  // Table should look something like:
  // Time, C1, C2, C3
  //   10, --, 21, 31
  //   20, 12, --, 32
  //   30, 13, 23, --
  //   35, 40, 50, 40
  //   40, --, 24, 34
  //   50, 15, --, 35
  //   60, 16, 26, --
  int rv = 0;
  CheckDataVisitor checkData;

  // Should match everything
  table.accept(0.0, 70.0, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(checkData.hasTime(10.0));
  rv += SDK_ASSERT(checkData.hasTime(60.0));
  rv += SDK_ASSERT(checkData.numTimes() == 7);
  checkData.clear();

  // Should be same as last one (10 is inclusive)
  table.accept(10.0, 70.0, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(checkData.hasTime(10.0));
  rv += SDK_ASSERT(checkData.hasTime(60.0));
  rv += SDK_ASSERT(checkData.numTimes() == 7);
  checkData.clear();

  // This next one shouldn't include time 60
  table.accept(10.0, 60.0, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(checkData.hasTime(10.0));
  rv += SDK_ASSERT(checkData.hasTime(50.0));
  rv += SDK_ASSERT(!checkData.hasTime(60.0));
  rv += SDK_ASSERT(checkData.numTimes() == 6);
  checkData.clear();

  // should only include 30 and 35
  table.accept(22, 37, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(checkData.hasTime(30.0));
  rv += SDK_ASSERT(checkData.hasTime(35.0));
  rv += SDK_ASSERT(checkData.numTimes() == 2);
  checkData.clear();

  // should only include 30
  table.accept(30, 35, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(checkData.hasTime(30.0));
  rv += SDK_ASSERT(checkData.numTimes() == 1);
  checkData.clear();

  // Should not include anything
  table.accept(30.5, 35, checkData);
  rv += SDK_ASSERT(checkData.numErrors() == 0);
  rv += SDK_ASSERT(!checkData.hasTime(30.0));
  rv += SDK_ASSERT(!checkData.hasTime(35.0));
  rv += SDK_ASSERT(checkData.numTimes() == 0);
  checkData.clear();

  // Next make sure the visitor stops when we need it to
  CheckDataVisitor stopAt30(30.0);
  table.accept(0, 100, stopAt30);
  rv += SDK_ASSERT(stopAt30.numErrors() == 0);
  rv += SDK_ASSERT(stopAt30.hasTime(30.0));
  rv += SDK_ASSERT(!stopAt30.hasTime(35.0));
  rv += SDK_ASSERT(stopAt30.numTimes() == 3);
  return rv;
}

int testPeekPrevSetGetValues(TableColumn::Iterator iter)
{
  int rv = 0;
  // Check the value in various data formats too while we're here
  int8_t i8 = 0;
  uint8_t ui8 = 0;
  int16_t i16 = 0;
  uint16_t ui16 = 0;
  int32_t i32 = 0;
  uint32_t ui32 = 0;
  int64_t i64 = 0;
  uint64_t ui64 = 0;
  float fl = 0.f;
  double doub = 0.0;
  std::string str = "";
  // If this first line fails, we're not pointing to the right element.  Previous() should be 12 when this is called
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(i8).isSuccess());
  rv += SDK_ASSERT(i8 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(ui8).isSuccess());
  rv += SDK_ASSERT(ui8 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(i16).isSuccess());
  rv += SDK_ASSERT(i16 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(ui16).isSuccess());
  rv += SDK_ASSERT(ui16 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(i32).isSuccess());
  rv += SDK_ASSERT(i32 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(ui32).isSuccess());
  rv += SDK_ASSERT(ui32 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(i64).isSuccess());
  rv += SDK_ASSERT(i64 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(ui64).isSuccess());
  rv += SDK_ASSERT(ui64 == 12);
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(fl).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(fl, 12.f));
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(doub).isSuccess());
  rv += SDK_ASSERT(simCore::areEqual(doub, 12.0));
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(str).isSuccess());
  rv += SDK_ASSERT(str == "12");


  // Test setting an iterator's value using different data formats
  int64_t value = -1;
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<int8_t>(0)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 0);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<uint8_t>(1)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 1);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<int16_t>(2)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 2);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<uint16_t>(3)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 3);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<int32_t>(4)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 4);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<uint32_t>(5)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 5);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<int64_t>(6)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 6);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(static_cast<uint64_t>(7)).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 7);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(8.f).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 8);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(9.0).isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 9);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue("10.0").isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 10);
  rv += SDK_ASSERT(iter.peekPrevious()->setValue("11").isSuccess());
  rv += SDK_ASSERT(iter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 11);
  // Reset it to the original 12
  rv += SDK_ASSERT(iter.peekPrevious()->setValue(12).isSuccess());

  // While we're here, test out toFront and toBack too
  TableColumn::Iterator newIter = iter;
  newIter.toFront();
  rv += SDK_ASSERT(!newIter.hasPrevious());
  rv += SDK_ASSERT(newIter.hasNext());
  rv += SDK_ASSERT(newIter.peekNext()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 12);
  newIter.toBack();
  rv += SDK_ASSERT(newIter.hasPrevious());
  rv += SDK_ASSERT(!newIter.hasNext());
  rv += SDK_ASSERT(newIter.peekPrevious()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 16);

  return rv;
}

int testColumnIteration(simData::DataTable& table)
{
  int rv = 0;

  // Create a data table with some null values
  rv += SDK_ASSERT(table.columnCount() == 0);
  simData::TableColumn* c1 = NULL;
  simData::TableColumn* c2 = NULL;
  simData::TableColumn* c3 = NULL;
  rv += SDK_ASSERT(table.addColumn("C1", simData::VT_UINT32, 0, &c1).isSuccess());
  rv += SDK_ASSERT(table.addColumn("C2", simData::VT_INT32, 0, &c2).isSuccess());
  rv += SDK_ASSERT(table.addColumn("C3", simData::VT_UINT64, 0, &c3).isSuccess());
  rv += SDK_ASSERT(c1->empty());
  rv += SDK_ASSERT(c2->empty());
  rv += SDK_ASSERT(c3->empty());
  // Create several time values, matching the layout:
  // Time, C1, C2, C3
  //   10, --, 21, 31
  //   20, 12, --, 32
  //   30, 13, 23, --
  //   35, 40, 40, 40    <-- sentinel row (see below -- becomes 40,50,40)
  //   40, --, 24, 34
  //   50, 15, --, 35
  //   60, 16, 26, --
  for (int addTime = 1; addTime <= 6; ++addTime)
  {
    simData::TableRow newRow;
    newRow.setTime(addTime * 10);
    // Add to C1 if addTime % 3 != 1; C2 if != 2; C3 if != 0
    if (addTime % 3 != 1)
      newRow.setValue(c1->columnId(), 10 + addTime);
    if (addTime % 3 != 2)
      newRow.setValue(c2->columnId(), 20 + addTime);
    if (addTime % 3 != 0)
      newRow.setValue(c3->columnId(), 30 + addTime);
    rv += SDK_ASSERT(newRow.cellCount() == 2);
    rv += SDK_ASSERT(table.addRow(newRow).isSuccess());
  }
  rv += SDK_ASSERT(!c1->empty());
  rv += SDK_ASSERT(!c2->empty());
  rv += SDK_ASSERT(!c3->empty());
  rv += SDK_ASSERT(c1->size() == c2->size());
  rv += SDK_ASSERT(c3->size() == c1->size());

  // Let's add a sentinel value so the table includes time 35, values 40,40,40
  simData::TableRow newRow;
  newRow.setTime(35.0);
  newRow.setValue(c1->columnId(), 40.0);
  newRow.setValue(c2->columnId(), 40.0);
  newRow.setValue(c3->columnId(), 40.0);
  rv += SDK_ASSERT(newRow.cellCount() == 3);
  rv += SDK_ASSERT(table.addRow(newRow).isSuccess());

  // Now iterate through and make sure the values match up what we expect, even with the out-of-order add
  TableColumn::Iterator iter = c1->begin();
  rv += SDK_ASSERT(!iter.hasPrevious());
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 20.0)); // c1 skips time 10
  int64_t value = 0;
  rv += SDK_ASSERT(iter.next()->getValue(value).isSuccess());
  rv += SDK_ASSERT(value == 12);

  // Since we're here, test this "12" value with get/set of different formats
  rv += testPeekPrevSetGetValues(iter);

  // Spot check next value on time only
  rv += SDK_ASSERT(iter.hasNext()); // time 30/13 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 30.0));
  rv += SDK_ASSERT(iter.hasNext()); // time 35/40 next...
  TableColumn::IteratorDataPtr iterData;
  iterData = iter.next();
  rv += SDK_ASSERT(simCore::areEqual(iterData->time(), 35.0));
  iterData->getValue(value);
  rv += SDK_ASSERT(value == 40);
  // Spot check 50/15
  rv += SDK_ASSERT(iter.hasNext()); // time 50/15 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 50.0));
  // Spot check 60/16
  rv += SDK_ASSERT(iter.hasNext()); // time 60/16 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 60.0));
  // Should be at the end
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(iter.hasPrevious());
  iter.previous()->getValue(value);
  rv += SDK_ASSERT(value == 16);
  // At this point, column 1 checks out

  // Before we check out column 2, use the "set value" to change the sentinel value from 40 to 50
  rv += SDK_ASSERT(c2->size() == 5);
  iter = c2->lower_bound(35.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 35.0));
  iter.next()->getValue(value);
  rv += SDK_ASSERT(value == 40);
  iter.previous()->setValue(50.0);
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 35.0));
  iter.next()->getValue(value);
  rv += SDK_ASSERT(value == 50);
  // Should still be 5 values
  rv += SDK_ASSERT(c2->size() == 5);
  rv += SDK_ASSERT(!c2->empty());

  // Similar set of iteration on column 2
  iter = c2->begin();
  rv += SDK_ASSERT(!iter.hasPrevious());
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 10.0));
  iter.next()->getValue(value);
  rv += SDK_ASSERT(value == 21);
  // Spot check next value on time only
  rv += SDK_ASSERT(iter.hasNext()); // time 30/23 next -- time 20 got skipped
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 30.0));
  rv += SDK_ASSERT(iter.hasNext()); // time 35/40 next...
  iterData = iter.next();
  rv += SDK_ASSERT(simCore::areEqual(iterData->time(), 35.0));
  iterData->getValue(value);
  rv += SDK_ASSERT(value == 50); // Note that we changed sentinel above
  // Spot check 40/24
  rv += SDK_ASSERT(iter.hasNext()); // time 40/24 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 40.0));
  // Note that we skip time 50, as per table above
  // Spot check 60/26
  rv += SDK_ASSERT(iter.hasNext()); // time 60/26 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 60.0));
  // Should be at the end
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(iter.hasPrevious());
  iter.previous()->getValue(value);
  rv += SDK_ASSERT(value == 26);

  // Finally go through column 3
  iter = c3->begin();
  rv += SDK_ASSERT(!iter.hasPrevious());
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 10.0));
  iter.next()->getValue(value);
  rv += SDK_ASSERT(value == 31);
  // Spot check next value on time only
  rv += SDK_ASSERT(iter.hasNext()); // time 20/32 next -- time 30 will get skipped
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 20.0));
  rv += SDK_ASSERT(iter.hasNext()); // time 35/40 next...
  iterData = iter.next();
  rv += SDK_ASSERT(simCore::areEqual(iterData->time(), 35.0));
  iterData->getValue(value);
  rv += SDK_ASSERT(value == 40); // Sentinel value added earlier
  rv += SDK_ASSERT(iter.hasNext()); // time 40/34 next...
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 40.0));
  // Spot check 60/36
  rv += SDK_ASSERT(iter.hasNext()); // time 50/35 next... (which is the last value)
  rv += SDK_ASSERT(simCore::areEqual(iter.next()->time(), 50.0));
  // Should be at the end
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(iter.hasPrevious());
  iter.previous()->getValue(value);
  rv += SDK_ASSERT(value == 35);

  // Quickly spot check a few lower_bound and upper_bound values
  rv += SDK_ASSERT(simCore::areEqual(c1->lower_bound(2.0).next()->time(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(c1->lower_bound(20.0).next()->time(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(c1->lower_bound(60.0).next()->time(), 60.0));
  rv += SDK_ASSERT(!c1->lower_bound(60.1).hasNext());
  rv += SDK_ASSERT(simCore::areEqual(c1->upper_bound(2.0).next()->time(), 20.0));
  rv += SDK_ASSERT(simCore::areEqual(c1->upper_bound(20.0).next()->time(), 30.0));
  rv += SDK_ASSERT(simCore::areEqual(c1->upper_bound(59.0).next()->time(), 60.0));
  rv += SDK_ASSERT(!c1->upper_bound(60).hasNext());
  rv += SDK_ASSERT(!c2->end().hasNext());
  rv += SDK_ASSERT(simCore::areEqual(c2->end().previous()->time(), 60.0));
  rv += SDK_ASSERT(!c3->end().hasNext());
  rv += SDK_ASSERT(simCore::areEqual(c3->end().previous()->time(), 50.0));

  // Check findAtOrBeforeTime()
  iter = c1->findAtOrBeforeTime(19.0);
  rv += SDK_ASSERT(!iter.hasNext());
  rv += SDK_ASSERT(iter.hasPrevious());
  rv += SDK_ASSERT(simCore::areEqual(iter.previous()->time(), 60.0));
  iter = c1->findAtOrBeforeTime(20.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 20.0));
  iter = c1->findAtOrBeforeTime(34.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 30.0));
  iter = c1->findAtOrBeforeTime(59.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 50.0));
  iter = c1->findAtOrBeforeTime(60.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 60.0));
  iter = c1->findAtOrBeforeTime(61.0);
  rv += SDK_ASSERT(iter.hasNext());
  rv += SDK_ASSERT(simCore::areEqual(iter.peekNext()->time(), 60.0));

  // we've got a well defined table, let's test row visitation with it
  rv += testRowIteration(table);

  return rv;
}

int rowTest()
{
  TableRow row;
  int rv = 0;
  rv += SDK_ASSERT(rowTest(row) == 0);
  return rv;
}

int managerTest()
{
  simData::MemoryDataStore ds;
  int rv = 0;
  rv += SDK_ASSERT(managerTest(ds.dataTableManager()) == 0);
  return rv;
}

int timeContainerTest()
{
  MemoryTable::DoubleBufferTimeContainer dbContainer;
  int rv = 0;
  rv += SDK_ASSERT(timeContainerTest(dbContainer) == 0);
#ifdef USE_DEPRECATED_SIMDISSDK_API
  MemoryTable::TimeContainerDeque sbContainer;
  rv += SDK_ASSERT(timeContainerTest(sbContainer) == 0);
#endif
  return rv;
}

int tableTest()
{
  simData::MemoryDataStore ds;
  simData::DataTableManager& mgr = ds.dataTableManager();
  simData::DataTable* table = NULL;
  int rv = 0;
  rv += SDK_ASSERT(mgr.addDataTable(1, "Test Table", &table).isSuccess());
  rv += SDK_ASSERT(table != NULL);
  rv += SDK_ASSERT(tableTest(*table) == 0);
  rv += SDK_ASSERT(flushTest(*table) == 0);
  return rv;
}

int removeEntityTest()
{
  simUtil::DataStoreTestHelper testHelper;
  int rv = 0;
  uint64_t plat1 = testHelper.addPlatform();
  uint64_t plat2 = testHelper.addPlatform();
  simData::DataTableManager& mgr = testHelper.dataStore()->dataTableManager();

  // add observer to test ManagerObserver
  // NOTE, since this has been wrapped in shared ptr we don't have to delete explicitly
  TestManagerObserver* testObserver = new TestManagerObserver("Foo");
  mgr.addObserver(simData::DataTableManager::ManagerObserverPtr(testObserver));

  testObserver->setExpectedTableName("Plat1Table1");
  rv += SDK_ASSERT(mgr.addDataTable(plat1, "Plat1Table1", NULL).isSuccess());
  testObserver->setExpectedTableName("Plat1Table2");
  rv += SDK_ASSERT(mgr.addDataTable(plat1, "Plat1Table2", NULL).isSuccess());
  testObserver->setExpectedTableName("Plat2Table1");
  rv += SDK_ASSERT(mgr.addDataTable(plat2, "Plat2Table1", NULL).isSuccess());
  testObserver->setExpectedTableName("Plat2Table2");
  rv += SDK_ASSERT(mgr.addDataTable(plat2, "Plat2Table2", NULL).isSuccess());
  rv += SDK_ASSERT(mgr.tableCount() == 4);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table1") != NULL);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table2") != NULL);
  rv += SDK_ASSERT(mgr.findTable(plat2, "Plat1Table1") == NULL); // random sanity check

  // Removing the entity should automatically remove its tables
  testObserver->setExpectedOwnerId(plat1);
  testHelper.dataStore()->removeEntity(plat1);
  rv += SDK_ASSERT(mgr.tableCount() == 2);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table1") == NULL);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table2") == NULL);
  rv += SDK_ASSERT(mgr.findTable(plat2, "Plat1Table1") == NULL);

  // Re-add the platform to make sure there's no funny business
  plat1 = testHelper.addPlatform();
  rv += SDK_ASSERT(mgr.tableCount() == 2);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table1") == NULL);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table2") == NULL);
  rv += SDK_ASSERT(mgr.findTable(plat2, "Plat1Table1") == NULL);
  testObserver->setExpectedTableName("Plat1Table3");
  rv += SDK_ASSERT(mgr.addDataTable(plat1, "Plat1Table3", NULL).isSuccess());
  rv += SDK_ASSERT(mgr.tableCount() == 3);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table1") == NULL);
  rv += SDK_ASSERT(mgr.findTable(plat1, "Plat1Table3") != NULL);

  // Try it with a non-platform entity
  uint64_t beam4 = testHelper.addBeam(plat1);
  uint64_t gate5 = testHelper.addGate(beam4);
  testObserver->setExpectedTableName("Gate5Table1");
  rv += SDK_ASSERT(mgr.addDataTable(gate5, "Gate5Table1", NULL).isSuccess());
  rv += SDK_ASSERT(mgr.tableCount() == 4);
  rv += SDK_ASSERT(mgr.findTable(gate5, "Gate5Table1") != NULL);
  testObserver->setExpectedOwnerId(gate5);
  testHelper.dataStore()->removeEntity(beam4); // should recursively kill gate5
  rv += SDK_ASSERT(mgr.tableCount() == 3);
  rv += SDK_ASSERT(mgr.findTable(gate5, "Gate5Table1") == NULL);

  rv += testObserver->numErrors();
  testObserver->setActive(false);

  return rv;
}

int testColumnIteration()
{
  int rv = 0;
  simData::MemoryTable::TableManager mgr(NULL);
  simData::DataTable* memoryTable = NULL;
  rv += SDK_ASSERT(mgr.addDataTable(0, "Table", &memoryTable).isSuccess());
  rv += SDK_ASSERT(testColumnIteration(*memoryTable) == 0);
  return rv;
}

int doubleBufferTimeContainerTest()
{
  int rv = 0;

  // Add a few time values, in time order
  simData::MemoryTable::DoubleBufferTimeContainer tc;
  rv += SDK_ASSERT(!tc.begin().hasNext());
  tc.findOrAddTime(10);
  rv += SDK_ASSERT(tc.begin().hasNext());
  tc.findOrAddTime(20);
  tc.findOrAddTime(30);
  tc.findOrAddTime(40);
  rv += SDK_ASSERT(tc.lower_bound(5).next().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(15).next().index() == 1);
  rv += SDK_ASSERT(tc.lower_bound(25).next().index() == 2);
  rv += SDK_ASSERT(tc.lower_bound(35).next().index() == 3);
  rv += SDK_ASSERT(tc.lower_bound(5).next().isFreshBin());

  rv += SDK_ASSERT(tc.lower_bound(5).hasPrevious() == false);
  rv += SDK_ASSERT(tc.lower_bound(5).previous().time() == std::numeric_limits<double>::max());
  rv += SDK_ASSERT(tc.lower_bound(15).hasPrevious() == true);
  rv += SDK_ASSERT(tc.lower_bound(15).previous().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(25).hasPrevious() == true);
  rv += SDK_ASSERT(tc.lower_bound(25).previous().index() == 1);
  rv += SDK_ASSERT(tc.lower_bound(35).hasPrevious() == true);
  rv += SDK_ASSERT(tc.lower_bound(35).previous().index() == 2);
  rv += SDK_ASSERT(tc.lower_bound(45).hasPrevious() == true);
  rv += SDK_ASSERT(tc.lower_bound(45).previous().index() == 3);
  rv += SDK_ASSERT(tc.lower_bound(5).previous().isFreshBin());

  rv += SDK_ASSERT(tc.upper_bound(5).next().index() == 0);
  rv += SDK_ASSERT(tc.upper_bound(15).next().index() == 1);
  rv += SDK_ASSERT(tc.upper_bound(25).next().index() == 2);
  rv += SDK_ASSERT(tc.upper_bound(35).next().index() == 3);
  rv += SDK_ASSERT(tc.upper_bound(45).next().time() == std::numeric_limits<double>::max());

  rv += SDK_ASSERT(tc.upper_bound(5).hasPrevious() == false);
  rv += SDK_ASSERT(tc.upper_bound(5).previous().time() == std::numeric_limits<double>::max());
  rv += SDK_ASSERT(tc.lower_bound(15).hasPrevious() == true);
  rv += SDK_ASSERT(tc.upper_bound(15).previous().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(25).hasPrevious() == true);
  rv += SDK_ASSERT(tc.upper_bound(25).previous().index() == 1);
  rv += SDK_ASSERT(tc.lower_bound(35).hasPrevious() == true);
  rv += SDK_ASSERT(tc.upper_bound(35).previous().index() == 2);
  rv += SDK_ASSERT(tc.lower_bound(45).hasPrevious() == true);
  rv += SDK_ASSERT(tc.upper_bound(45).previous().index() == 3);

  // Do a swap, and redo the searches
  std::vector<DataTable::TableObserverPtr> noObservers;
  tc.swapFreshStaleData(NULL, noObservers);
  rv += SDK_ASSERT(tc.begin().hasNext());
  rv += SDK_ASSERT(tc.begin().next().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(5).next().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(15).next().index() == 1);
  rv += SDK_ASSERT(tc.lower_bound(25).next().index() == 2);
  rv += SDK_ASSERT(tc.lower_bound(35).next().index() == 3);
  rv += SDK_ASSERT(!tc.lower_bound(5).next().isFreshBin());

  rv += SDK_ASSERT(tc.lower_bound(5).previous().time() == std::numeric_limits<double>::max());
  rv += SDK_ASSERT(tc.lower_bound(15).previous().index() == 0);
  rv += SDK_ASSERT(tc.lower_bound(25).previous().index() == 1);
  rv += SDK_ASSERT(tc.lower_bound(35).previous().index() == 2);
  rv += SDK_ASSERT(tc.lower_bound(45).previous().index() == 3);
  rv += SDK_ASSERT(tc.lower_bound(5).previous().isFreshBin());

  rv += SDK_ASSERT(tc.upper_bound(5).next().index() == 0);
  rv += SDK_ASSERT(tc.upper_bound(15).next().index() == 1);
  rv += SDK_ASSERT(tc.upper_bound(25).next().index() == 2);
  rv += SDK_ASSERT(tc.upper_bound(35).next().index() == 3);
  rv += SDK_ASSERT(tc.upper_bound(45).next().time() == std::numeric_limits<double>::max());

  rv += SDK_ASSERT(tc.upper_bound(5).previous().time() == std::numeric_limits<double>::max());
  rv += SDK_ASSERT(tc.upper_bound(15).previous().index() == 0);
  rv += SDK_ASSERT(tc.upper_bound(25).previous().index() == 1);
  rv += SDK_ASSERT(tc.upper_bound(35).previous().index() == 2);
  rv += SDK_ASSERT(tc.upper_bound(45).previous().index() == 3);

  // Add some times in the middle, should go into the fresh bin
  tc.findOrAddTime(15);
  tc.findOrAddTime(25);
  MemoryTable::TimeContainer::Iterator iter = tc.begin();
  MemoryTable::TimeContainer::IteratorData value = iter.next();
  rv += SDK_ASSERT(value.time() == 10.0);
  rv += SDK_ASSERT(value.index() == 0);
  rv += SDK_ASSERT(!value.isFreshBin());
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 15.0);
  rv += SDK_ASSERT(value.index() == 0);
  rv += SDK_ASSERT(value.isFreshBin());
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 20.0);
  rv += SDK_ASSERT(value.index() == 1);
  rv += SDK_ASSERT(!value.isFreshBin());
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 25.0);
  rv += SDK_ASSERT(value.index() == 1);
  rv += SDK_ASSERT(value.isFreshBin());
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 30.0);
  rv += SDK_ASSERT(value.index() == 2);
  rv += SDK_ASSERT(!value.isFreshBin());
  value = iter.previous(); // Goes back to 30 (which was passed with the iter.next())
  rv += SDK_ASSERT(value.time() == 30.0);
  rv += SDK_ASSERT(value.index() == 2);
  rv += SDK_ASSERT(!value.isFreshBin());
  value = iter.previous(); // move to 25
  rv += SDK_ASSERT(value.time() == 25.0);
  rv += SDK_ASSERT(value.index() == 1);
  rv += SDK_ASSERT(value.isFreshBin());
  value = iter.previous(); // move to 20
  rv += SDK_ASSERT(value.time() == 20.0);
  rv += SDK_ASSERT(value.index() == 1);
  rv += SDK_ASSERT(!value.isFreshBin());
  value = iter.previous(); // move to 15
  rv += SDK_ASSERT(value.time() == 15.0);
  rv += SDK_ASSERT(value.index() == 0);
  rv += SDK_ASSERT(value.isFreshBin());
  value = iter.previous();
  rv += SDK_ASSERT(value.time() == 10.0);
  rv += SDK_ASSERT(value.index() == 0);
  rv += SDK_ASSERT(!value.isFreshBin());

  rv += SDK_ASSERT(tc.lower_bound(5).next().time() == 10);
  rv += SDK_ASSERT(tc.lower_bound(15).next().time() == 15);
  rv += SDK_ASSERT(tc.lower_bound(20).next().time() == 20);
  rv += SDK_ASSERT(tc.lower_bound(25).next().time() == 25);
  rv += SDK_ASSERT(tc.lower_bound(30).next().time() == 30);
  rv += SDK_ASSERT(tc.lower_bound(35).next().time() == 40);
  rv += SDK_ASSERT(tc.lower_bound(40).next().time() == 40);
  rv += SDK_ASSERT(tc.lower_bound(45).next().time() == std::numeric_limits<double>::max());

  rv += SDK_ASSERT(tc.upper_bound(5).next().time() == 10);
  rv += SDK_ASSERT(tc.upper_bound(15).next().time() == 20);
  rv += SDK_ASSERT(tc.upper_bound(20).next().time() == 25);
  rv += SDK_ASSERT(tc.upper_bound(25).next().time() == 30);
  rv += SDK_ASSERT(tc.upper_bound(30).next().time() == 40);
  rv += SDK_ASSERT(tc.upper_bound(35).next().time() == 40);
  rv += SDK_ASSERT(tc.lower_bound(45).next().time() == std::numeric_limits<double>::max());

  // Swap, we should only have two times (15 and 25), because others were swapped+cleared
  rv += SDK_ASSERT(tc.size() == 6);
  tc.swapFreshStaleData(NULL, noObservers);
  rv += SDK_ASSERT(tc.size() == 2);
  iter = tc.begin();
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 15.0);
  rv += SDK_ASSERT(value.index() == 0);
  rv += SDK_ASSERT(!value.isFreshBin());
  rv += SDK_ASSERT(iter.hasNext());
  value = iter.next();
  rv += SDK_ASSERT(value.time() == 25.0);
  rv += SDK_ASSERT(value.index() == 1);
  rv += SDK_ASSERT(!value.isFreshBin());

  return rv;
}

int testPartialFlush()
{
  simData::MemoryDataStore ds;
  simData::DataTableManager& mgr = ds.dataTableManager();
  simData::DataTable* table = NULL;
  int rv = 0;
  rv += SDK_ASSERT(mgr.addDataTable(1, "Test Table", &table).isSuccess());
  // Create two columns and add data to both
  simData::TableColumn* column1;
  simData::TableColumn* column2;
  table->addColumn("Test Column 1", simData::VT_DOUBLE, 0, &column1);
  table->addColumn("Test Column 2", simData::VT_DOUBLE, 0, &column2);
  rv += SDK_ASSERT(table->columnCount() == 2);
  rv += SDK_ASSERT(column1->empty());
  rv += SDK_ASSERT(column2->empty());
  for (double i = 0; i < 10; ++i)
  {
    simData::TableRow row;
    row.setTime(i);
    row.setValue(column1->columnId(), i);
    row.setValue(column2->columnId(), i);
    table->addRow(row);
  }
  rv += SDK_ASSERT(column1->size() == 10);
  rv += SDK_ASSERT(column2->size() == 10);

  // Flush only the first column
  table->flush(column1->columnId());
  rv += SDK_ASSERT(column1->size() == 0);
  rv += SDK_ASSERT(column2->size() == 10);
  rv += SDK_ASSERT(table->columnCount() == 2);

  return rv;
}

}

int MemoryDataTableTest(int argc, char* argv[])
{
  int rv = 0;
  rv += rowTest();
  rv += managerTest();
  rv += timeContainerTest();
  rv += tableTest();
  rv += removeEntityTest();
  rv += dataLimitingTest();
  rv += dataLimitSecondsTest();
#ifdef USE_DEPRECATED_SIMDISSDK_API
  rv += subTableIterationTest(new simData::MemoryTable::TimeContainerDeque());
#endif
  rv += subTableIterationTest(new simData::MemoryTable::DoubleBufferTimeContainer());
  rv += testColumnIteration();
  rv += doubleBufferTimeContainerTest();
  rv += testPartialFlush();
  rv += getTimeRangeTest();
  return rv;
}
