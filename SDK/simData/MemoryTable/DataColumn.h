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
#ifndef SIMDATA_MEMORYTABLE_DATACOLUMN_H
#define SIMDATA_MEMORYTABLE_DATACOLUMN_H

#include <string>
#include "simData/DataTable.h"
#include "simData/MemoryTable/DataContainer.h"
#include "simData/MemoryTable/TimeContainer.h"

namespace simData { namespace MemoryTable {

/**
 * Implementation of the table column.  Private inside the .cpp to prevent others from
 * accessing the internal public functions that aren't in the virtual interface.
 * This implementation holds onto data in a deque and lets the time container dictate
 * where values ought to be placed inside the deque.
 */
class DataColumn : public simData::TableColumn
{
public:
  /** Instantiates a new data column. */
  DataColumn(TimeContainer* timeContainer, const std::string& columnName, TableColumnId columnId, VariableType storageType, UnitType unitType);
  /** Columns contain no dynamic memory */
  virtual ~DataColumn();

  /** Inserts a template value into a data container */
  template <typename DataType>
  void insert(bool freshContainer, size_t position, const DataType& value)
  {
    dataContainer_(freshContainer)->insert(position, value);
  }

  /** Replaces a value in a data container with a template value */
  template <typename DataType>
  TableStatus replace(bool freshContainer, size_t position, const DataType& value)
  {
    return dataContainer_(freshContainer)->replace(position, value);
  }

  /** Retrieves the value of a data cell using templates */
  template <typename DataType>
  TableStatus getValue(bool freshContainer, size_t position, DataType& value) const
  {
    return dataContainer_(freshContainer)->getValue(position, value);
  }

  /** Removes a single entry from the data column based on position */
  void erase(bool freshContainer, size_t position);
  /** Clears out the contents of the data container */
  simData::DelayedFlushContainerPtr flush();
  /** Retrieves the number of entries in the data column */
  virtual size_t size() const;
  /** Returns true if there is no data in the data column */
  virtual bool empty() const;

  /** Retrieves the designated column ID for this data column. */
  virtual TableColumnId columnId() const;
  /** Retrieves the name of this column. */
  virtual std::string name() const;
  /** Retrieves the variable type associated with this column's data. */
  virtual VariableType variableType() const;
  /** Retrieves the unit type associated with the values in the column. */
  virtual UnitType unitType() const;
  /** Changes the unit type for the column data. */
  virtual void setUnitType(UnitType units);

  /** Interpolates a double value at a given time, using a custom interpolator. */
  virtual TableStatus interpolate(double& value, double time, const Interpolator* interpolator) const;

  /** Start iteration at the beginning of the container (smallest time). */
  virtual Iterator begin();
  /** Iterator representing the back of the container (largest time). */
  virtual Iterator end();
  /**
   * Returns lower_bound() iterator into container; see DataSlice::lower_bound()
   * for detailed examples and description of lower_bound() functionality.
   */
  virtual Iterator lower_bound(double timeValue);
  /**
   * Returns upper_bound() iterator into container; see DataSlice::upper_bound()
   * for detailed examples and description of upper_bound() functionality.
   */
  virtual Iterator upper_bound(double timeValue);
  /**
   * Retrieves an iterator such that next() is the time at or immediately before
   * the current time.  If there is no value at or before the current time, then
   * next() will be invalid (hasNext() is false).
   * @param timeValue Time value to facilitate the find-at-or-before search
   * @return Iterator whose next() is at or before the given time, or end() if
   *   there is no such iterator.
   */
  virtual Iterator findAtOrBeforeTime(double timeValue) const;

  /// Fixes the time container, i.e. after a split.  Responsibility of SubTable to keep up to date
  void replaceTimeContainer(TimeContainer* newTimes);

  /// Copies data from the data container into the row specified
  void fillRow(const TimeContainer::IteratorData& timeIdxData, TableRow& row) const;

  /// Swaps the contents of the fresh and stale data, flushing out the stale
  void swapFreshStaleData();

  /**
   * Returns the begin and end time of the column
   * @param begin Returns the begin time
   * @param end Returns the end time
   * @returns 0 if begin and end are set
   */
  virtual int getTimeRange(double& begin, double& end) const;

private:
  /// Allocates a new data container based on the data storage type
  DataContainer* newDataContainer_(simData::VariableType variableType) const;
  /// Retrieves the data container, fresh or stale, as requested
  DataContainer* dataContainer_(bool freshContainer) const;

  TimeContainer* timeContainer_;
  DataContainer* freshData_;
  DataContainer* staleData_;

  std::string name_;
  TableColumnId id_;
  VariableType variableType_;
  UnitType unitType_;

  // Implement the iteration mechanisms from TableColumn::IteratorData
  class IteratorDataImpl;
  // Implements the TableColumn::IteratorImpl interface
  class ColumnIteratorImpl;
  // Implements DelayedFlushContainer to thread out deletion
  class FlushContainer;
};

}}

#endif /* SIMDATA_MEMORYTABLE_DATACOLUMN_H */
