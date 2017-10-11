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
#ifndef SIMDATA_DATATABLE_H
#define SIMDATA_DATATABLE_H

#include <string>
#include <vector>
#include <deque>
#include <utility>
#include "simCore/Common/Common.h"
#include "simCore/Common/Memory.h"
#include "simData/DataStore.h"
#include "simData/TableStatus.h"

namespace simData {

/// Currently undefined enumeration
typedef int UnitType;
/// Storage type for variables in data table
enum VariableType
{
  /// 8 bit unsigned integer (1 byte)
  VT_UINT8 = 0,
  /// 8 bit signed integer (1 byte)
  VT_INT8,
  /// 16 bit unsigned integer (2 bytes)
  VT_UINT16,
  /// 16 bit signed integer (2 bytes)
  VT_INT16,
  /// 32 bit unsigned integer (4 bytes)
  VT_UINT32,
  /// 32 bit signed integer (4 bytes)
  VT_INT32,
  /// 64 bit unsigned integer (8 bytes)
  VT_UINT64,
  /// 64 bit signed integer (8 bytes)
  VT_INT64,
  /// Single precision floating point value (4 bytes)
  VT_FLOAT,
  /// Double precision floating point value (8 bytes)
  VT_DOUBLE,
  /// String stored internally as a std::string (variable size)
  VT_STRING
};

class DataTable;
class TableColumn;
class TableRow;
class TableList;

/// Column IDs are 64 bit integers; TODO should this be unsigned? 32 bits?
typedef int64_t TableColumnId;
/// Table IDs are also 64 bit integers
typedef uint64_t TableId;
/// Represents an invalid column ID
static const TableColumnId INVALID_TABLECOLUMN = -1;
/// Represents an invalid table ID
static const TableId INVALID_TABLEID = 0xffffffffffffffffull;

/// SIMDIS Interal track history color data table
static const std::string INTERNAL_TRACK_HISTORY_TABLE = "__Internal_TrackHistoryTable__";
/// Track History Color column name
static const std::string INTERNAL_TRACK_HISTORY_COLOR_COLUMN = "TrackHistoryColor";

/// SIMDIS Interal LOB draw style data table
static const std::string INTERNAL_LOB_DRAWSTYLE_TABLE = "__Internal_LobDrawStyleTable__";
/// LOB stipple 1 column name
static const std::string INTERNAL_LOB_STIPPLE1_COLUMN = "Stipple1";
/// LOB stipple 2 column name
static const std::string INTERNAL_LOB_STIPPLE2_COLUMN = "Stipple2";
/// LOB color 1 column name
static const std::string INTERNAL_LOB_COLOR1_COLUMN = "Color1";
/// LOB color 2 column name
static const std::string INTERNAL_LOB_COLOR2_COLUMN = "Color2";
/// LOB line width column name
static const std::string INTERNAL_LOB_LINEWIDTH_COLUMN = "LineWidth";
/// LOB line flash column name
static const std::string INTERNAL_LOB_FLASH_COLUMN = "Flash";

/**
 * Provides routines to create, delete, find, and generally manage groups of data tables.
 * Tables can be queried by an owner identification value, which typically corresponds to
 * the entity ID of a data store entity.  Each entity can have zero or more tables associated
 * with it, but a single table may only be associated with one entity.  All tables under
 * a single entity have unique names (e.g. two entities may both have a table named "Foo"
 * but a single entity may not have two tables with the same name "Foo".)
 *
 * This class serves as a table factory.  Deleting an instance of a manager is expected to
 * delete all tables created by that instance.
 */
class SDKDATA_EXPORT DataTableManager
{
public:
  virtual ~DataTableManager() {}

  /**
   * Creates a data table with the given name under the owner provided, returning existing tables
   * if the table name already exists.
   * @param ownerId Owner ID of the table.
   * @param tableName Table name unique to the entity owner ID, must be non-empty.
   * @param newTable Pointer to a data table, or NULL if there was an error.  This table may have been
   *   created by this call, or created by a previous call if the name is not unique.  The return value
   *   will indicate isError() if the table already existed.
   * @return Status indicating success for a new table creation, or error if an existing table was
   *   returned or if the name is invalid (empty)
   */
  virtual TableStatus addDataTable(simData::ObjectId ownerId, const std::string& tableName, DataTable** newTable) = 0;

  /**
   * Deletes the table associated with the unique table ID provided.  Each table is associated
   * with a single table ID that is unique to the DataTableManager that creates the table.
   * If more than one data table manager exists, they might create tables with duplicate IDs.
   * Any pointers referencing the table will be left dangling after this call.
   * @param tableId Table to delete
   * @return Success on deletion, error if table not found or otherwise couldn't be deleted.
   */
  virtual TableStatus deleteTable(TableId tableId) = 0;

  /**
   * Deletes all tables associated with a particular entity owner ID.  Any pointers left that
   * point to any deleted instance will be left dangling after this call.  Use this function when
   * an entity is being deleted, in order to clean up its data tables.
   * @param ownerId ID whose tables will be deleted.
   */
  virtual void deleteTablesByOwner(simData::ObjectId ownerId) = 0;

  /**
   * Total number of tables managed by this instance.  Expected to only increase after calls
   * to addDataTable(), and only decrease after calls to deleteTable() or deleteTablesByOwner().
   * @return Total number of tables in this manager.
   */
  virtual size_t tableCount() const = 0;

  /**
   * Retrieves a list of all tables being managed for the entity specified.
   * @param ownerId ID value for the entity in question
   * @return List of all tables managed for this entity.
   */
  virtual const TableList* tablesForOwner(simData::ObjectId ownerId) const = 0;

  /**
   * Retrieves a table by its unique table identification number.  Table IDs are unique within
   * a single DataTableManager instance.
   */
  virtual DataTable* getTable(TableId tableId) const = 0;

  /**
   * Searches for a table with the given owner identifier and table name.  An entity may hold
   * many tables, and many entities may have the same table name, but only a single table may
   * exist for a given owner ID and table name combination.
   * @param ownerId Owner of the table being searched for
   * @param tableName Name of the table being searched for
   * @return Table value associated with the owner ID and string name, or NULL if none found.
   */
  virtual DataTable* findTable(simData::ObjectId ownerId, const std::string& tableName) const = 0;

  /**
  * Defines an observer interface to notify when tables are added or removed.
  */
  class ManagerObserver
  {
  public:
    virtual ~ManagerObserver() {}
    /**
    * Called in the DataTableManager::addDataTable, after new DataTable has been created.
    * Table will be in the TableList when this is called
    */
    virtual void onAddTable(DataTable* table) = 0;
    /**
    * Called just before a DataTable is deleted.
    * Table may no longer be in TableList when this is called.
    */
    virtual void onPreRemoveTable(DataTable* table) = 0;
  };

  /** Smart pointer to hold table manager observers */
  typedef std::shared_ptr<ManagerObserver> ManagerObserverPtr;

  /**@name Observers
   * @{
   */
  /// Add or remove an observer for notification of new or removed tables
  virtual void addObserver(ManagerObserverPtr callback) = 0;
  virtual void removeObserver(ManagerObserverPtr callback) = 0;
  ///@}

  /**
  * Retrieve the observers from this table manager
  * @param [out] observers
  */
  virtual void getObservers(std::vector<ManagerObserverPtr>& observers) = 0;

};

/**
 * Interface for a list of tables associated with a single entity.
 */
class TableList
{
public:
  virtual ~TableList() {}

  /** Provides a method to iterate over all entries following a Visitor pattern. */
  class Visitor
  {
  public:
    virtual ~Visitor() {}

    /**
     * Perform a function on the table provided, which is a member of this list.
     * @param table Table inside this list, provided by the TableList::accept() routine.
     */
    virtual void visit(DataTable* table) = 0;
  };

  /** Entity owner ID associated with the list of tables */
  virtual simData::ObjectId ownerId() const = 0;
  /** Retrieves the table (or NULL) associated with the given name. */
  virtual DataTable* findTable(const std::string& tableName) const = 0;
  /** Returns the total number of tables in this list. */
  virtual size_t tableCount() const = 0;
  /** Performs visitation of each table in this list, calling the ListVisitor provided. */
  virtual void accept(TableList::Visitor& visitor) const = 0;
};

/**
 * Flush memory cleanup can be delayed for performance reasons; this structure
 * enables this capability (when supported by implementation) by holding on to
 * dynamic memory until the caller is ready to destroy it.
 */
class DelayedFlushContainer
{
public:
  /** Virtual destructor deletes the memory held by this container */
  virtual ~DelayedFlushContainer()
  {
  }
};
/** Shared pointer for a DelayedFlushedContainer */
typedef std::shared_ptr<DelayedFlushContainer> DelayedFlushContainerPtr;

/**
 * Composite implementation of the delayed flush container holds a deque of
 * other containers.
 */
class DelayedFlushContainerComposite : public DelayedFlushContainer
{
public:
  virtual ~DelayedFlushContainerComposite()
  {
  }
  /** Saves a flush container for later deletion. */
  void push_back(DelayedFlushContainerPtr ptr)
  {
    if (ptr.get() != NULL)
      deque_.push_back(ptr);
  }
private:
  std::deque<DelayedFlushContainerPtr> deque_;
};

/**
 * Data tables can contain any time stamped data in column arrangement, and permit both out-of-
 * order addition of elements and NULL cells.  Columns can be added after table creation, and
 * can also be added even after some rows are added.  Tables should be created using the
 * DataTableManager::addDataTable() factory function.
 */
class SDKDATA_EXPORT DataTable
{
public:
  virtual ~DataTable() {}

  /** Provides a method to iterate over all row entries following a Visitor pattern. */
  class RowVisitor
  {
  public:
    virtual ~RowVisitor() {}

    /// Return value from your visit() function; use this to stop early
    enum VisitReturn
    {
      /// Requests that the visitor stop visitation
      VISIT_STOP = 0,
      /// Visitation will continue normally (unless last element visited already)
      VISIT_CONTINUE
    };

    /**
     * Perform a function on the row provided, which is a member of this table.
     * @param row Row inside this table, provided by the DataTable::accept() routine.
     * @return You may interrupt a visitation and stop the acceptance algorithm by
     *   returning a VISIT_STOP value; otherwise visitation will continue.
     */
    virtual VisitReturn visit(const TableRow& row) = 0;
  };

  /** Provides a method to iterate over all entries following a Visitor pattern. */
  class ColumnVisitor
  {
  public:
    virtual ~ColumnVisitor() {}

    /**
     * Perform a function on the column provided, which is a member of this data table.
     * @param column Column inside this data table, provided by the DataTable::accept() routine.
     */
    virtual void visit(TableColumn* column) = 0;
  };

  /**
   * Retrieves the unique table identification number for this table.  Table IDs are unique
   * within the context of a single DataTableManager.
   * @return Unique table identifier value
   */
  virtual TableId tableId() const = 0;

  /**
   * Retrieves the name of the table.  Table names may or may not be mutable after construction
   * depending on the implementation.  Table names are unique within the context of a single
   * entity's list of tables, but are not unique within the context of a DataTableManager.
   * @return Table's name
   */
  virtual std::string tableName() const = 0;

  /**
   * Retrieves the owner identification value.  Tables can only belong to a single owner.
   * @return ID of the owning entity
   */
  virtual ObjectId ownerId() const = 0;

  /**
   * Returns the total number of columns in this data table.
   * @return Number of columns in the table.
   */
  virtual size_t columnCount() const = 0;

  /**
   * Retrieves the column associated with the particular column ID provided.  Column IDs
   * are unique within the context of a single DataTable and will not change.
   * @return NULL if column not found, or pointer to the column ID requested.
   */
  virtual TableColumn* column(TableColumnId id) const = 0;

  /**
   * Retrieves the column associated with the particular column name provided.  Column names
   * are unique within the context of a single DataTable.
   * @return NULL if column not found, or pointer to the column with the name requested.
   */
  virtual TableColumn* column(const std::string& name) const = 0;

  /**
   * Adds a new column to the data table with the given name.  Table names are unique within
   * the context of a single DataTable, and may or may not be mutable after construction
   * based on the implementation of the DataTable or TableColumn.  Column will always be added
   * to the end of the table, such that the column visitor will visit the newest column last.
   * @param columnName Column to add to this data table.
   * @param storageType Variable type for container storage.  All incoming data is cast to
   *   this data type.
   * @param unitType Units associated with the column if any.
   * @param column Pointer to hold the newly created column.  This contents of this pointer
   *   could be set to an existing column if that column name already exists inside this data
   *   table.  Note that this is an error condition return.
   * @return Status describing success or failure of this function call.  Note that column
   *   may be set to a valid value and an error returned if the column name is duplicated.
   */
  virtual TableStatus addColumn(const std::string& columnName, VariableType storageType, UnitType unitType, TableColumn** column) = 0;

  /**
   * Performs visitation of each row in this list after beginTime (inclusive) until endTime
   * (exclusive), calling the RowVisitor provided.  The rows are visited in time order from
   * earliest to latest time value.
   * TODO: What about -1 times?  Should those be visited even when outside time bounds?
   * @param beginTime Time at which to start visiting rows.  This is an inclusive time, so if
   *   a row with this time value exists, it will be included in visitation.
   * @param endTime Time at which to stop visiting rows.  This is an exclusive time, so if a
   *   row with this time value exists, it will NOT be included in visitation.
   * @param visitor Visitor used to operate on the rows within the time frame.
   */
  virtual void accept(double beginTime, double endTime, RowVisitor& visitor) const = 0;

  /**
   * Performs visitation of each column in this data table.
   * @param visitor Visitor object to operate on the columns in this data table.
   */
  virtual void accept(ColumnVisitor& visitor) const = 0;

  /**
   * Adds a data table row to the table.
   * @param row Row to add to the table.
   * @return Status indicating success or failure of row addition
   */
  virtual TableStatus addRow(const TableRow& row) = 0;

  /**
   * Deletes all the data in the data table columns, leaving the columns empty.
   * @return Container to all of the dynamic memory stored in the table.  When the
   *   smart pointer falls out of scope, the data gets deleted.  This enables a
   *   delayed flush mechanism that can be used to flush in a thread for improved
   *   performance and decreased application latency.
   */
  virtual DelayedFlushContainerPtr flush() = 0;


  /**
  * Defines an observer interface to notify when rows or columns are added or removed.
  */
  class TableObserver
  {
  public:
    virtual ~TableObserver() {}

    /**
    * Called after a new TableColumn is added, in DataTable::addColumn.  New column will be
    * at the end of the table, such that the column visitor will visit the newest column last.
    * @param table  reference to the parent DataTable
    * @param column  reference to the newly added TableColumn
    */
    virtual void onAddColumn(DataTable& table, const TableColumn& column) = 0;

    /**
    * Called after a new TableRow is added, in DataTable::addRow
    * @param table  reference to the parent DataTable
    * @param row  reference to the newly added TableRow
   */
    virtual void onAddRow(DataTable& table, const TableRow& row) = 0;

    /**
    * Called just before a TableColumn is removed from the table
    * @param table  reference to the parent DataTable
    * @param column  reference to the TableColumn about to be removed
    */
    virtual void onPreRemoveColumn(DataTable& table, const TableColumn& column) = 0;

    /**
    * Called just before a TableRow is removed from the table
    * @param table  reference to the parent DataTable
    * @param rowTime  time of row that is about to be removed
    */
    virtual void onPreRemoveRow(DataTable& table, double rowTime) = 0;
  };

  /** Smart pointer to hold table observers */
  typedef std::shared_ptr<TableObserver> TableObserverPtr;

  /**@name Observers
   * @{
   */
  /// Add or remove an observer for notification of new or removed rows and columns
  virtual void addObserver(TableObserverPtr callback) = 0;
  virtual void removeObserver(TableObserverPtr callback) = 0;
  ///@}

};

/**
 * Interface that represents a single column in a data table.  A data table consists of zero
 * or more columns, and columns consist of zero or more rows.
 */
class TableColumn
{
public:
  virtual ~TableColumn() {}

  /**
   * Class returned by iterator access into the column
   */
  class IteratorData
  {
  public:
    virtual ~IteratorData() {}

    /** Retrieves the data time of the cell */
    virtual double time() const = 0;

    ///@{
    /**
     * Retrieves cell data from the iterator.
     * @param value Variable into which to place the cell value.  Cell value converted to type as necessary.
     * @return Success or failure flag in TableStatus.
     */
    virtual TableStatus getValue(uint8_t& value) const = 0;
    virtual TableStatus getValue(int8_t& value) const = 0;
    virtual TableStatus getValue(uint16_t& value) const = 0;
    virtual TableStatus getValue(int16_t& value) const = 0;
    virtual TableStatus getValue(uint32_t& value) const = 0;
    virtual TableStatus getValue(int32_t& value) const = 0;
    virtual TableStatus getValue(uint64_t& value) const = 0;
    virtual TableStatus getValue(int64_t& value) const = 0;
    virtual TableStatus getValue(float& value) const = 0;
    virtual TableStatus getValue(double& value) const = 0;
    virtual TableStatus getValue(std::string& value) const = 0;
    ///@}

    ///@{
    /**
     * Retrieves cell data from the iterator.
     * @param value Variable into which to place the cell value.  Cell value converted to type as necessary.
     * @return Success or failure flag in TableStatus.
     */
    virtual TableStatus setValue(uint8_t value) = 0;
    virtual TableStatus setValue(int8_t value) = 0;
    virtual TableStatus setValue(uint16_t value) = 0;
    virtual TableStatus setValue(int16_t value) = 0;
    virtual TableStatus setValue(uint32_t value) = 0;
    virtual TableStatus setValue(int32_t value) = 0;
    virtual TableStatus setValue(uint64_t value) = 0;
    virtual TableStatus setValue(int64_t value) = 0;
    virtual TableStatus setValue(float value) = 0;
    virtual TableStatus setValue(double value) = 0;
    virtual TableStatus setValue(const std::string& value) = 0;
    ///@}
  };

  /// Smart pointer handling of Iterator Data
  typedef std::shared_ptr<IteratorData> IteratorDataPtr;
  /// TimeContainer::IteratorImpl is defined by TimeContainer implementations
  typedef GenericIteratorImpl<IteratorDataPtr> IteratorImpl;
  /// TimeContainer::Iterator lets users iterate over time/index values in-order
  typedef GenericIterator<IteratorDataPtr> Iterator;

  /**
   * Provides an interface to interpolate between data points.
   */
  class Interpolator
  {
  public:
    virtual ~Interpolator() {}

    /** Interpolates value of column at time tVal, given lowVal at time tLow and highVal at tHigh. */
    virtual double interpolate(const TableColumn* column, double lowVal, double highVal, double tLow, double tVal, double tHigh) const = 0;
  };

  /**
   * Retrieves the unique column identifier associated with this column.  Column identifiers
   * are unique to a given data table and should not change after construction, including
   * when other columns are removed.
   * @return Unique table column identifier
   */
  virtual TableColumnId columnId() const = 0;

  /**
   * Retrieves the name of the column.
   * @return Name of the column
   */
  virtual std::string name() const = 0;

  /**
    Retrieves the data variable type associated with the column.  A column can only hold a
   * single type of data, such as integer or string data.  This typically cannot be changed
   * once a column is instantiated.
   * @return Variable type for the column.
   */
  virtual VariableType variableType() const = 0;

  /**
   * Retrieves the unit type meta data for the column, if any exists.
   * @return Unit type data associated with this column.  All values are in this particular unit.
   */
  virtual UnitType unitType() const = 0;

  /**
   * Changes the unit type associated with values in this data column.
   * @return Units for the data values in this column.
   */
  virtual void setUnitType(UnitType units) = 0;

  /**
   * Retrieves the value of the column at a given time, using the interpolator provided.
   * @param value Will contain the value interpolated at the requested time.
   * @param time Time value to seek for interpolating values.
   * @param interpolator Instance of interpolator to calculate values; if NULL, then linearly interpolate.
   * @return Status return value.  This function will not extrapolate, so
   *   searching for a value before the beginning or after the end is an error.
   */
  virtual TableStatus interpolate(double& value, double time, const Interpolator* interpolator) const = 0;

  /**
   * Retrieves the number of entries in this data column.  This should be equivalent to the
   * total number of entries that can be accessed via the Iterator interface below.
   * @return Total number of entries in the column.
   */
  virtual size_t size() const = 0;

  /**
   * Returns true if the column has no data.  This implies size() == 0.
   * @return True if the column is empty and has no data.
   */
  virtual bool empty() const = 0;

  /**@defgroup TableColumnIteration Time container iteration routines.
   * @{
   */
  /** Start iteration at the beginning of the container (smallest time). */
  virtual Iterator begin() = 0;
  /** Iterator representing the back of the container (largest time). */
  virtual Iterator end() = 0;
  /**
   * Returns lower_bound() iterator into container; see DataSlice::lower_bound()
   * for detailed examples and description of lower_bound() functionality.
   */
  virtual Iterator lower_bound(double timeValue) = 0;
  /**
   * Returns upper_bound() iterator into container; see DataSlice::upper_bound()
   * for detailed examples and description of upper_bound() functionality.
   */
  virtual Iterator upper_bound(double timeValue) = 0;
  /**
   * Retrieves an iterator such that next() is the time at or immediately before
   * the current time.  If there is no value at or before the current time, then
   * next() will be invalid (hasNext() is false).
   * @param timeValue Time value to facilitate the find-at-or-before search
   * @return Iterator whose next() is at or before the given time, or end() if
   *   there is no such iterator.
   */
  virtual Iterator findAtOrBeforeTime(double timeValue) const = 0;
  /// @}
};

/// Forward declare a cell class to be used internally by TableRow
class TableCell;

/**
 * Data in tables is stored in rows.  A row can be sparse and contain values only for
 * some columns.  Each row has a time stamp that applies to all values in the row.
 * Note that rows are more or less transient classes that are typically not stored
 * as-is inside a data table.
 */
class SDKDATA_EXPORT TableRow
{
public:
  /** Default constructor */
  TableRow();
  /** Copy constructor */
  TableRow(const TableRow& copyConstr);
  /** Assignment operator is implemented for TableRow */
  TableRow& operator=(const TableRow& copy);
  virtual ~TableRow();

  /**
   * Permits visitation to all cells inside a particular row.
   */
  class CellVisitor
  {
  public:
    virtual ~CellVisitor() {}

    /**@name Cell Visitor visit() methods
     * @{
     */
    /// Visitor pattern method that is called when visiting a value of the given data type.
    virtual void visit(TableColumnId columnId, uint8_t value) = 0;
    virtual void visit(TableColumnId columnId, int8_t value) = 0;
    virtual void visit(TableColumnId columnId, uint16_t value) = 0;
    virtual void visit(TableColumnId columnId, int16_t value) = 0;
    virtual void visit(TableColumnId columnId, uint32_t value) = 0;
    virtual void visit(TableColumnId columnId, int32_t value) = 0;
    virtual void visit(TableColumnId columnId, uint64_t value) = 0;
    virtual void visit(TableColumnId columnId, int64_t value) = 0;
    virtual void visit(TableColumnId columnId, float value) = 0;
    virtual void visit(TableColumnId columnId, double value) = 0;
    virtual void visit(TableColumnId columnId, const std::string& value) = 0;
    ///@}
  };

  /**
   * Removes all values in the row and resets the time stamp.
   */
  void clear();

  /**
   * Retrieves the time stamp associated with this row.
   * @return Time stamp associated with the row
   */
  double time() const;

  /**
   * Sets the time stamp associated with this row.
   * @param t Time stamp associated with the row.
   */
  void setTime(double t);

  /**
  * Optionally reserve the space for the expected number of values; minor performance improvement 
  * @param number Expected number of values
  */
  void reserve(size_t number);

  /** Retrieves the contents of a particular column's cell, converting as needed from native to uint8_t format */
  TableStatus value(TableColumnId columnId, uint8_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to int8_t format */
  TableStatus value(TableColumnId columnId, int8_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to uint16_t format */
  TableStatus value(TableColumnId columnId, uint16_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to int16_t format */
  TableStatus value(TableColumnId columnId, int16_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to uint32_t format */
  TableStatus value(TableColumnId columnId, uint32_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to int32_t format */
  TableStatus value(TableColumnId columnId, int32_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to uint64_t format */
  TableStatus value(TableColumnId columnId, uint64_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to int64_t format */
  TableStatus value(TableColumnId columnId, int64_t& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to float format */
  TableStatus value(TableColumnId columnId, float& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to double format */
  TableStatus value(TableColumnId columnId, double& v) const;
  /** Retrieves the contents of a particular column's cell, converting as needed from native to std::string format */
  TableStatus value(TableColumnId columnId, std::string& v) const;

  /** Sets the contents of a particular column's cell, converting from uint8_t input to native cell format */
  void setValue(TableColumnId columnId, uint8_t value);
  /** Sets the contents of a particular column's cell, converting from int8_t input to native cell format */
  void setValue(TableColumnId columnId, int8_t value);
  /** Sets the contents of a particular column's cell, converting from uint16_t input to native cell format */
  void setValue(TableColumnId columnId, uint16_t value);
  /** Sets the contents of a particular column's cell, converting from int16_t input to native cell format */
  void setValue(TableColumnId columnId, int16_t value);
  /** Sets the contents of a particular column's cell, converting from uint32_t input to native cell format */
  void setValue(TableColumnId columnId, uint32_t value);
  /** Sets the contents of a particular column's cell, converting from int32_t input to native cell format */
  void setValue(TableColumnId columnId, int32_t value);
  /** Sets the contents of a particular column's cell, converting from uint64_t input to native cell format */
  void setValue(TableColumnId columnId, uint64_t value);
  /** Sets the contents of a particular column's cell, converting from int64_t input to native cell format */
  void setValue(TableColumnId columnId, int64_t value);
  /** Sets the contents of a particular column's cell, converting from float input to native cell format */
  void setValue(TableColumnId columnId, float value);
  /** Sets the contents of a particular column's cell, converting from double input to native cell format */
  void setValue(TableColumnId columnId, double value);
  /** Sets the contents of a particular column's cell, converting from std::string input to native cell format */
  void setValue(TableColumnId columnId, const std::string& value);

  /**
   * Returns true when the row contains a value for the requested column identifier.
   * @return true if the row contains a value for the column, false otherwise.
   */
  bool containsCell(TableColumnId columnId) const;

  /**
   * Returns the total number of cells in this row, not including the time value (which
   * is not a cell itself).
   * @return Number of cells in the row.
   */
  size_t cellCount() const;

  /**
   * Returns true if there are no cells in the row.
   * @return True if row is empty, false if it contains cells.
   */
  bool empty() const;

  /**
   * Permits visitation of each cell value.  Visitation is not necessarily in column order,
   * but instead is based on the order in which cells are set.
   */
  void accept(CellVisitor& visitor) const;

private:
  typedef std::pair<TableColumnId, TableCell*> ColumnCellPair;

  double time_;
  std::vector<ColumnCellPair> cells_;

  /** Returns a cell by ID. */
  TableCell* findCell_(TableColumnId id) const;
};


}

#endif /* SIMDATA_DATATABLE_H */
