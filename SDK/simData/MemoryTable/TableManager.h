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
#ifndef SIMDATA_MEMORYTABLE_TABLEMANAGER_H
#define SIMDATA_MEMORYTABLE_TABLEMANAGER_H

#include <vector>
#include <map>
#include "simData/DataTable.h"

namespace simData { namespace MemoryTable {

class Table;
class DataLimitsProvider;

/**
 * In-memory solution for the table manager.  Owns data tables in a map from ID to
 * table, and also owns lists that contain soft references to the tables.
 */
class SDKDATA_EXPORT TableManager : public simData::DataTableManager
{
public:
  /** Constructs a new table manager using data limits provided by the dataLimitsProvider. */
  TableManager(const DataLimitsProvider* dataLimitsProvider);
  virtual ~TableManager();

  /** Adds a data table to the owner specified, with name specified. */
  virtual TableStatus addDataTable(ObjectId ownerId, const std::string& tableName, DataTable** newTable);
  /** Deletes a table referenced by ID. */
  virtual TableStatus deleteTable(TableId tableId);
  /** Deletes all the tables associated with a particular owner. */
  virtual void deleteTablesByOwner(simData::ObjectId ownerId);

  /** Total number of tables managed. */
  virtual size_t tableCount() const;
  /** List of all tables associated with an entity ID. */
  virtual const TableList* tablesForOwner(simData::ObjectId ownerId) const;
  /** Retrieves the table with the given ID. */
  virtual DataTable* getTable(TableId tableId) const;
  /** Finds a table by name. */
  virtual DataTable* findTable(ObjectId ownerId, const std::string& tableName) const;

  /** Add an observer for notification of new or removed tables */
  virtual void addObserver(ManagerObserverPtr callback);
  /** Remove an observer */
  virtual void removeObserver(ManagerObserverPtr callback);
  /** Retrieve the observers for this table manager */
  virtual void getObservers(std::vector<ManagerObserverPtr>& observers);

  /** Removes a table from internal lists without removing it; used by simData::MemoryTable::~Table() */
  void removeTable(Table* table);

private:
  typedef std::vector<ManagerObserverPtr> ManagerObserverList;
  class MemTableList;

  /// Notify observers of added table
  void fireOnAddTable_(DataTable* table) const;
  /// Notify observers of table about to be removed
  void fireOnPreRemoveTable_(DataTable* table) const;

  /// Monotonically increase ID value for the next created table
  TableId nextId_;
  /// Lists of data tables, sorted by owner
  std::map<ObjectId, MemTableList*> listsByOwner_;
  /// All tables, sorted by table ID
  std::map<TableId, DataTable*> tablesById_;
  /// List of observers for add/remove table notification
  ManagerObserverList observers_;
  /// Provides data limits for tables
  const DataLimitsProvider* dataLimitsProvider_;
};

} }

#endif /* SIMDATA_MEMORYTABLE_TABLEMANAGER_H */

