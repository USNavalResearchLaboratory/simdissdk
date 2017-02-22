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
#ifndef SIMDATA_MEMORYTABLE_SUBTABLE_H
#define SIMDATA_MEMORYTABLE_SUBTABLE_H

#include <map>
#include <vector>
#include <utility>
#include "simCore/Common/Memory.h"
#include "simData/GenericIterator.h"
#include "simData/DataTable.h"
#include "simData/MemoryTable/TimeContainer.h"

namespace simData {

  class DataTable;

namespace MemoryTable {

/// In-memory implementation of the TableColumn interface
class DataColumn;

/**
 * SubTable is a null-less table contained inside an in-memory Table instance.
 * Each SubTable manages one or more columns of data.  It is a container for both
 * data columns and a time container.  The time container and data container are
 * very tightly coupled, as a performance boost for insertion, search, and data
 * limiting.  The class is modeled after the Time-Index Deque, Data Deque implementation
 * in SIMDIS 9.
 */
class SDKDATA_EXPORT SubTable
{
public:
  /// Creates a sub table using the time container strategy provided
  SubTable(TimeContainer* newTimeContainer);
  virtual ~SubTable();

  /** Number of rows in the time container of this subtable */
  size_t rowCount() const;
  /** Number of columns in this null-less subtable */
  size_t columnCount() const;
  /**
   * Returns true if there are no cells contained in this subtable.  An empty table
   * may contain columns, but those columns must also be empty.
   */
  bool empty() const;

  /** Defines an interface that lets you add rows to this subtable safely */
  class AddRowTransaction
  {
  public:
    virtual ~AddRowTransaction() {}

    /**@name SubTable AddRowTransaction setCellValue() methods
     * @{
     */
    /// Visitor pattern method that is called when visiting a value of the given data type.
    virtual TableStatus setCellValue(TableColumnId columnId, uint8_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, int8_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, uint16_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, int16_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, uint32_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, int32_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, uint64_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, int64_t value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, float value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, double value) = 0;
    virtual TableStatus setCellValue(TableColumnId columnId, const std::string& value) = 0;
    ///@}
  };
  /// Smart pointer to hold transactions
  typedef std::tr1::shared_ptr<AddRowTransaction> AddRowTransactionPtr;

  /** Defines an interface to notify when splits occur */
  class SplitObserver
  {
  public:
    virtual ~SplitObserver() {}

    /** Called when a subtable splits into a new table due to time differences (i.e. to maintain null-less subtable) */
    virtual void notifySplit(SubTable* originalTable, SubTable* newTable, const std::vector<TableColumnId>& splitColumns) = 0;
  };
  /// Smart pointer to hold split observers
  typedef std::tr1::shared_ptr<SplitObserver> SplitObserverPtr;

  /**
   * Creates a transaction to add a row to the subtable.  A single subtable should only
   * have a single transaction open at a time.  Once the transaction falls out of scope,
   * the table may split into different subtables.  When this occurs, the splitObserver
   * is told, allowing the owner to update their internal structures.
   */
  AddRowTransactionPtr addRow(double timeStamp, SplitObserverPtr splitObserver);

  /**
   * Creates a column with given parameters, returning it to the caller.  This call
   * will fail if there is any data in the subtable (i.e. if rowCount() > 0).
   * @param columnName Name of the column to create
   * @param columnId Unique identifier for the column in the context of a data table.
   * @param storageType Variable type for storage of column data.
   * @param unitType Units for the column data.
   * @param newCol Returns the table column pointer back to the caller.
   * @return Status of the operation.
   */
  TableStatus addColumn(const std::string& columnName, TableColumnId columnId,
    VariableType storageType, UnitType unitType, TableColumn** newCol);

  /**
   * Interpolates a double precision value from the column at the given time, using
   * the interpolator provided.
   */
  TableStatus interpolate(TableColumnId columnId, double time, double& value, const TableColumn::Interpolator* interpolator) const;

  /**
   * Removes all rows from the subtable.
   */
  simData::DelayedFlushContainerPtr flush();

  /** Performs data limiting */
  void limitData(size_t maxPoints, double latestInvalidTime, DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers);

  /** Represents the return value from the subtable iterators */
  class SDKDATA_EXPORT IteratorData
  {
  public:
    /** Constructor for IteratorData on a subtable, given a time iterator */
    IteratorData(const SubTable* subTable, const TimeContainer::IteratorData& timeIterData);
    /** Copy constructor */
    IteratorData(const IteratorData& copyConstructor);
    /** Default constructor */
    ~IteratorData();
    /** Assignment operator */
    IteratorData& operator=(const IteratorData& copy);

    /** Data time for the iterator if the iterator is valid. */
    double time() const;
    /** Fills out the contents of the row with values from this subtable. */
    void fillRow(TableRow& row) const;

  private:
    const SubTable* subTable_;
    TimeContainer::IteratorData timeIterData_;
  };

  /// Implementation of the actual iterator is private to SubTable
  class IteratorImpl;
  /// Iterator implementation is wrapped with Iterator typedef as normal
  typedef GenericIterator<IteratorData> Iterator;

  /** Start iteration at the beginning of the container (smallest time). */
  Iterator begin();
  /** Iterator representing the back of the container (largest time). */
  Iterator end();
  /**
   * Returns lower_bound() iterator into container; see DataSlice::lower_bound()
   * for detailed examples and description of lower_bound() functionality.
   */
  Iterator lower_bound(double timeValue);
  /**
   * Returns upper_bound() iterator into container; see DataSlice::upper_bound()
   * for detailed examples and description of upper_bound() functionality.
   */
  Iterator upper_bound(double timeValue);

private:
  /// Implementation of the AddRowTransaction interface
  class AddRowTransactionImpl;
  /// Vector of all owned columns
  std::vector<DataColumn*> columns_;
  /// Maps the column ID to owned column; indirect map, should map in size() to columns_
  std::map<TableColumnId, DataColumn*> columnMap_;
  /// Orders the time values and deque contents
  TimeContainer* timeContainer_;

  /// Initializes a subtable using the columns provided, with our time map (minus "withoutTimeStamp")
  SubTable(const TimeContainer& copyTimes, const std::vector<DataColumn*>& withColumns, double withoutTimeStamp);

  /// Finds a column based on ID, returning NULL if none
  DataColumn* findColumn_(TableColumnId columnId) const;
  /// Removes but does not delete, used in splits to maintain null-less table
  TableStatus removeColumn_(TableColumnId columnId);
  /// Fills a row with our contents (but does not set the time), at the specified time
  void fillRow_(const TimeContainer::IteratorData& timeIdxData, TableRow& row) const;
};

} }

#endif /* SIMDATA_MEMORYTABLE_SUBTABLE_H */
