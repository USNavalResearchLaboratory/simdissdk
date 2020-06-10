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
#include <cassert>
#include <algorithm>
#include "simData/MemoryTable/Table.h"
#include "simData/MemoryTable/TableManager.h"

namespace simData { namespace MemoryTable {

/**
 * Organizes the data tables into list by owner.  The lists hold soft or observer pointers
 * into the data tables.  Each owner has zero or one list.
 */
class TableManager::MemTableList : public simData::TableList
{
public:
  /** Constructs a MemTableList on the object ID provided */
  explicit MemTableList(simData::ObjectId ownerId)
    : ownerId_(ownerId)
  {
    // Memory is not owned
  }

  virtual ~MemTableList()
  {
  }

  // Inherits parent's documentation
  virtual size_t tableCount() const
  {
    return tables_.size();
  }

  // Inherits parent's documentation
  virtual simData::ObjectId ownerId() const
  {
    return ownerId_;
  }

  // Inherits parent's documentation
  virtual DataTable* findTable(const std::string& tableName) const
  {
    std::map<std::string, simData::DataTable*>::const_iterator i = tables_.find(tableName);
    return (i == tables_.end()) ? NULL : i->second;
  }

  // Inherits parent's documentation
  virtual void accept(TableList::Visitor& visitor) const
  {
    for (std::map<std::string, simData::DataTable*>::const_iterator i = tables_.begin(); i != tables_.end(); ++i)
    {
      visitor.visit(i->second);
    }
  }

  /** Assigns a data table to this list */
  int addDataTable(const std::string& tableName, DataTable* newTable)
  {
    // Assertion means the caller (TableManager) forgot to check for table name
    // existence before attempting to create a table.  While it may be "safer" to search
    // here too, that's double the performance penalty.  Since the classes are already
    // tightly coupled (this is an inner class), this is only an assertion.
    assert(tables_.find(tableName) == tables_.end());
    // Assertion failure means caller doing something funny
    assert(tableName == newTable->tableName());
    tables_[tableName] = newTable;
    return 0;
  }

  /**
   * Removes a data table from this list without deleting it (since the
   * table manager owns the memory
   */
  void removeTable(DataTable* aboutToDelete)
  {
    std::map<std::string, simData::DataTable*>::iterator i = tables_.find(aboutToDelete->tableName());
    if (i != tables_.end() && i->second == aboutToDelete)
      tables_.erase(i);
  }


private:
  // Note that the pointer is not owned
  std::map<std::string, simData::DataTable*> tables_;
  simData::ObjectId ownerId_;
};

//////////////////////////////////////////////////////////////////

TableManager::TableManager(const DataLimitsProvider* dataLimitsProvider)
  : nextId_(1),
    dataLimitsProvider_(dataLimitsProvider),
    newRowDataListener_(new DefaultNewRowDataListener)
{
}

TableManager::~TableManager()
{
  for (std::map<ObjectId, MemTableList*>::const_iterator i = listsByOwner_.begin();
    i != listsByOwner_.end(); ++i)
  {
    delete i->second;
  }
  listsByOwner_.clear();
  // Copy the tables-by-id to iterate through it, to avoid iterator invalidation
  std::map<TableId, DataTable*> tablesById;
  tablesById.swap(tablesById_);
  for (std::map<TableId, DataTable*>::const_iterator i = tablesById.begin(); i != tablesById.end(); ++i)
  {
    delete i->second;
  }
}

TableStatus TableManager::addDataTable(ObjectId ownerId, const std::string& tableName, DataTable** newTable)
{
  // Don't allow empty string
  if (tableName.empty())
  {
    if (newTable != NULL) *newTable = NULL;
    return TableStatus::Error("Empty name not permitted for new data tables.");
  }

  // Make sure we don't add duplicates
  std::map<ObjectId, MemTableList*>::const_iterator i = listsByOwner_.find(ownerId);
  MemTableList* list = NULL;
  if (i != listsByOwner_.end())
  {
    list = i->second;
    // Assertion failure means internal code forgot to erase() an iterator, shouldn't happen
    assert(list != NULL);
    DataTable* table = list->findTable(tableName);
    if (table != NULL)
    { // NULL == table already exists
      if (newTable != NULL) *newTable = table;
      return TableStatus::Error("Table with name already exists for specified entity.");
    }
  }
  else
  {
    list = new MemTableList(ownerId);
    listsByOwner_[ownerId] = list;
  }

  DataTable* table = new MemoryTable::Table(*this, nextId_++, tableName, ownerId, dataLimitsProvider_);
  int addTableRv = list->addDataTable(tableName, table);
  // Only failure possible is due to logic error before calling
  assert(addTableRv == 0);
  // Assertion failure means reuse of IDs or invalidly saved ID in constructor
  assert(tablesById_.find(table->tableId()) == tablesById_.end());
  tablesById_[table->tableId()] = table;
  if (newTable != NULL) *newTable = table;

  // notify observers table was added
  fireOnAddTable_(table);

  return TableStatus::Success();
}

TableStatus TableManager::deleteTable(TableId tableId)
{
  std::map<TableId, DataTable*>::iterator tableIter = tablesById_.find(tableId);
  if (tableIter != tablesById_.end())
  {
    delete tableIter->second;
    // Assertion failure means ~MemoryTable::Table didn't remove itself
    assert(tablesById_.find(tableId) == tablesById_.end());
    return TableStatus::Success();
  }
  return TableStatus::Error("Table not found.");
}

void TableManager::removeTable(MemoryTable::Table* table)
{
  if (table == NULL)
    return;

  // notify observers table is being removed
  fireOnPreRemoveTable_(table);

  std::map<TableId, DataTable*>::iterator tableIter = tablesById_.find(table->tableId());
  // Table not found
  if (tableIter == tablesById_.end() || tableIter->second == NULL)
    return;

  // Find the entry in the listsByOwner_
  std::map<ObjectId, MemTableList*>::iterator listIter = listsByOwner_.find(table->ownerId());
  // Assertion failure means the list got out of sync with our maps
  assert(listIter != listsByOwner_.end() && listIter->second != NULL);
  if (listIter != listsByOwner_.end() && listIter->second != NULL)
  {
    MemTableList* list = listIter->second;
    // Assertion failure means we lost sync with the list
    assert(list->findTable(table->tableName()) == table);
    // Do we delete the entry?
    if (list->tableCount() == 1)
    {
      delete list;
      listsByOwner_.erase(listIter);
    }
    else
    {
      list->removeTable(table);
    }
  }

  // Remove it from our map too
  tablesById_.erase(tableIter);
}

void TableManager::setNewRowDataListener(TableManager::NewRowDataListenerPtr listener)
{
  if (listener == NULL)
    newRowDataListener_.reset(new DefaultNewRowDataListener);
  else
    newRowDataListener_ = listener;
}

void TableManager::fireOnNewRowData(Table& table, double dataTime)
{
  newRowDataListener_->onNewRowData(table, table.ownerId(), dataTime);
}

void TableManager::addObserver(ManagerObserverPtr callback)
{
  observers_.push_back(callback);
}

void TableManager::removeObserver(ManagerObserverPtr callback)
{
  ManagerObserverList::iterator i = std::find(observers_.begin(), observers_.end(), callback);
  if (i != observers_.end())
    observers_.erase(i);
}

void TableManager::getObservers(std::vector<ManagerObserverPtr>& observers)
{
  observers = observers_;
}

/// Visitor to remove entries from by-ID list and delete tables, by owner ID
class DeleteTablesInList : public TableList::Visitor
{
public:
  /** Constructs a new DeelteTablesInList visitor */
  explicit DeleteTablesInList(std::map<TableId, DataTable*>& tablesById)
    : tablesById_(tablesById)
  {
  }
  virtual void visit(DataTable* table)
  {
    std::map<TableId, DataTable*>::iterator i = tablesById_.find(table->tableId());
    // Assertion failure indicates sync failure in list/map
    assert(i != tablesById_.end());
    tablesById_.erase(i);
    delete table;
  }
private:
  std::map<TableId, DataTable*>& tablesById_;
};

void TableManager::deleteTablesByOwner(ObjectId ownerId)
{
  std::map<ObjectId, MemTableList*>::iterator listIter = listsByOwner_.find(ownerId);
  if (listIter == listsByOwner_.end() || listIter->second == NULL)
    return; // nothing to do
  MemTableList* list = listIter->second;
  // TODO: Test this; might have iterator invalidation issues
  DeleteTablesInList deleteTablesInList(tablesById_);
  list->accept(deleteTablesInList);

  // clear out the list entry
  delete list;
  listsByOwner_.erase(listIter);
}

size_t TableManager::tableCount() const
{
  return tablesById_.size();
}

const TableList* TableManager::tablesForOwner(ObjectId ownerId) const
{
  std::map<ObjectId, MemTableList*>::const_iterator i = listsByOwner_.find(ownerId);
  return (i == listsByOwner_.end()) ? NULL : i->second;
}

DataTable* TableManager::getTable(simData::TableId tableId) const
{
  std::map<TableId, DataTable*>::const_iterator i = tablesById_.find(tableId);
  return (i == tablesById_.end() ? NULL : i->second);
}

DataTable* TableManager::findTable(ObjectId ownerId, const std::string& tableName) const
{
  const TableList* list = tablesForOwner(ownerId);
  return (list != NULL) ? list->findTable(tableName) : NULL;
}

void TableManager::fireOnAddTable_(DataTable* table) const
{
  for (ManagerObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onAddTable(table);
  }
}

void TableManager::fireOnPreRemoveTable_(DataTable* table) const
{
  for (ManagerObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onPreRemoveTable(table);
  }
}


} }
