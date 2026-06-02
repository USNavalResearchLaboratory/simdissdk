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
  explicit TableManager(const DataLimitsProvider* dataLimitsProvider);
  virtual ~TableManager();

  /** Adds a data table to the owner specified, with name specified. */
  TableStatus addDataTable(ObjectId ownerId, const std::string& tableName, DataTable** newTable) override;
  /** Deletes a table referenced by ID. */
  TableStatus deleteTable(TableId tableId) override;
  /** Deletes all the tables associated with a particular owner. */
  void deleteTablesByOwner(simData::ObjectId ownerId) override;

  /** Total number of tables managed. */
  size_t tableCount() const override;
  /** List of all tables associated with an entity ID. */
  const TableList* tablesForOwner(simData::ObjectId ownerId) const override;
  /** Retrieves the table with the given ID. */
  DataTable* getTable(TableId tableId) const override;
  /** Finds a table by name. */
  DataTable* findTable(ObjectId ownerId, const std::string& tableName) const override;

  /** Add an observer for notification of new or removed tables */
  void addObserver(ManagerObserverPtr callback) override;
  /** Remove an observer */
  void removeObserver(ManagerObserverPtr callback) override;
  /** Retrieve the observers for this table manager */
  void getObservers(std::vector<ManagerObserverPtr>& observers) override;

  /** Removes a table from internal lists without removing it; used by simData::MemoryTable::~Table() */
  void removeTable(Table* table);

  /// Observer interface for a class that gets notified when rows are added to a data table.
  class NewRowDataListener
  {
  public:
    virtual ~NewRowDataListener() = default;

    /// New row was processed for the entity ID provided, at the time provided.  Query the table for contents of update.
    virtual void onNewRowData(simData::DataTable& table, simData::ObjectId id, double dataTime) = 0;
  };
  /// Managed pointer for NewUpdatesListener
  typedef std::shared_ptr<NewRowDataListener> NewRowDataListenerPtr;

  /// Default implementation of NewRowDataListener is a no-op
  class DefaultNewRowDataListener : public NewRowDataListener
  {
  public:
    /// Implemented as Noop
    void onNewRowData(simData::DataTable& table, simData::ObjectId id, double dataTime) override {}
  };

  /** Sets the new-row data listener for when new table rows are added; use nullptr to remove. */
  void setNewRowDataListener(NewRowDataListenerPtr listener);

  /** Internal method for Table to use to alert on new row data */
  void fireOnNewRowData(Table& table, double dataTime);

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

  /// Pointer to tell when new rows are added
  NewRowDataListenerPtr newRowDataListener_;
};

} }

#endif /* SIMDATA_MEMORYTABLE_TABLEMANAGER_H */

