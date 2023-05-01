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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMDATA_MEMORYTABLE_TABLE_H
#define SIMDATA_MEMORYTABLE_TABLE_H

#include <map>
#include <vector>
#include <utility>
#include <string>
#include "simData/DataTable.h"

namespace simData { namespace MemoryTable {

class TableManager;
class SubTable;
class DataLimitsProvider;

/**
 * In-memory implementation of a data table.  Data tables are possibly null-full and are associated with
 * a single entity (or the scenario, if owner ID is 0).  A single entity may have multiple tables
 * (represented by a simData::TableList), but a single data table is associated with only one owner.
 *
 * Tables might have nullptr values.  For performance, the in-memory solution subdivides itself into SubTable
 * instances.  Each SubTable is null-less.  So this Table implementation is a collection of SubTable
 * instances.
 */
class SDKDATA_EXPORT Table : public simData::DataTable
{
public:
  /** Instantiates a table with a given manager and various properties. */
  Table(TableManager& mgr, TableId tableId, const std::string& tableName, ObjectId ownerId, const DataLimitsProvider* dataLimits);
  /** Virtual destructor cleans up memory and unregisters from the table manager. */
  virtual ~Table();

  /** Unique ID of the table (unique within the table manager).  Assigned by the table manager. */
  virtual TableId tableId() const;
  /** Name of the table (unique within a given entity's table list) */
  virtual std::string tableName() const;
  /** Unique ID of the owning entity. */
  virtual ObjectId ownerId() const;
  /**
   * If the table has no null values, the routine will return the number of rows in the table.
   * If the table has null values the routine will return the number of rows in the largest sub-table.
   */
  virtual size_t maxSubTableRow() const;
  /** Number of columns in this table. */
  virtual size_t columnCount() const;
  /** Retrieves column by ID */
  virtual TableColumn* column(TableColumnId id) const;
  /** Retrieves column by name. */
  virtual TableColumn* column(const std::string& name) const;
  /** Adds a new data column, returning a pointer to the table interface. */
  virtual TableStatus addColumn(const std::string& columnName, VariableType storageType, UnitType unitType, TableColumn** newColumn);
  /** Removes a data column */
  virtual TableStatus removeColumn(const std::string& columnName);
  /** Visitor pattern to access all rows in the table. */
  virtual void accept(double beginTime, double endTime, DataTable::RowVisitor& visitor) const;
  /** Visitor pattern to access all columns in the table. */
  virtual void accept(DataTable::ColumnVisitor& visitor) const;
  /** Adds a row to the table. */
  virtual TableStatus addRow(const TableRow& row);
  /** Clears data out of the given column or all columns if given -1 */
  virtual DelayedFlushContainerPtr flush(TableColumnId id = -1);
  /** Remove rows in the given time range; up to but not including endTime */
  virtual void flush(double startTime, double endTime);
  /** Add an observer for notification when rows or columns are added or removed */
  virtual void addObserver(TableObserverPtr callback);
  /** Remove an observer */
  virtual void removeObserver(TableObserverPtr callback);

private:
  /** Retrieves the subtable for the given column */
  SubTable* subTableForId_(TableColumnId columnId) const;
  /** Notify observers of new column */
  void fireOnAddColumn_(const TableColumn& column) const;
  /** Notify observers of new row */
  void fireOnAddRow_(const TableRow& row) const;
  /** Notify observers of column about to be removed */
  void fireOnPreRemoveColumn_(const TableColumn& column) const;
  /** Notify observers of row about to be removed */
  void fireOnPreRemoveRow_(const TableRow& row) const;

  /**
   * Performs data limiting on the table
   * @param numToKeep Number of points to keep in the data table
   * @param timeWindow Number of seconds of data to keep in memory
   */
  void limitData_(size_t numToKeep, double timeWindow);


  /// Pointer back to the owning table manager.
  TableManager& tableManager_;
  /// Internal table-manager-assigned ID value
  TableId tableId_;
  /// Table name
  std::string tableName_;
  /// Entity associated with the table
  ObjectId ownerId_;
  /// latest row time in the table
  double endTime_;

  /// Container of all null-less subtables
  std::vector<SubTable*> subtables_;

  typedef std::pair<SubTable*, TableColumn*> TableToColumn;
  /// Maps the table ID to subtable and column pointer
  std::map<TableColumnId, TableToColumn> columns_;
  /// Maps column name to column pointer
  std::map<std::string, TableColumn*> columnsByName_;

  /// Each table stores columns with monotonically increasing ID values
  TableColumnId nextId_;
  /// Contains the type of data limiting applied to the table
  const DataLimitsProvider* dataLimits_;

  /// Visitor for row that transfers cells to right subtables
  class TransferCellsToSubTables;
  /// When a split occurs, we fix pointers and take ownership of new table
  class MoveColumnsToNewSubTable;

  typedef std::vector<TableObserverPtr> TableObserverList;
  TableObserverList observers_; /// List of observers to be notified when columns/rows are added/removed
};

} }

#endif /* SIMDATA_MEMORYTABLE_TABLE_H */
