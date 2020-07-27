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
#include <limits>
#include <cassert>
#include <algorithm>
#include "simCore/Calc/Math.h"
#include "simData/TableCellTranslator.h"
#include "simData/MemoryTable/DoubleBufferTimeContainer.h"
#include "simData/MemoryTable/SubTable.h"
#include "simData/MemoryTable/TableManager.h"
#include "simData/MemoryTable/DataLimitsProvider.h"
#include "simData/MemoryTable/Table.h"

namespace simData { namespace MemoryTable {

/**
 * Observer for a subtable's split operation.  Responsible for taking ownership of
 * a newly created subtable, then fix internal column-to-subtable pointers so that
 * we know which subtable has which columns.
 */
class Table::MoveColumnsToNewSubTable : public SubTable::SplitObserver
{
public:
  /** Constructs a new MoveColumnsToNewSubTable subtable observer */
  explicit MoveColumnsToNewSubTable(Table& owner)
    : owner_(owner)
  {
  }

  /// Takes ownership of pointer and updates subtable map for all columns
  virtual void notifySplit(SubTable* originalTable, SubTable* newTable, const std::vector<TableColumnId>& splitColumns)
  {
    // Take ownership
    owner_.subtables_.push_back(newTable);

    // Fix the column ID to subtable map
    for (std::vector<TableColumnId>::const_iterator vecIter = splitColumns.begin();
      vecIter != splitColumns.end(); ++vecIter)
    {
      std::map<TableColumnId, Table::TableToColumn>::iterator iter = owner_.columns_.find(*vecIter);
      // Assertion failure means we got notified about a column for which we don't know
      // about.  Perhaps an observer got messed up, or our internal structures are
      // out of sync, or something bad happened
      assert(iter != owner_.columns_.end());
      if (iter != owner_.columns_.end())
        iter->second.first = newTable;
    }
  }
private:
  Table& owner_;
};

////////////////////////////////////////////////////////////////////////

/**
 * Responsible for visiting cells in a single row to add the value of the cells to
 * the columns that they represent.  Note that adding values to a row can induce one
 * or more subtable splits.  A single subtable can only split once per row addition,
 * but multiple subtables might be affected by a row addition.
 */
class Table::TransferCellsToSubTables : public TableRow::CellVisitor
{
public:
  /** Constructs a new TransferCellsToSubTables visitor */
  TransferCellsToSubTables(Table& owner, double rowTimeStamp)
    : owner_(owner),
      rowTimeStamp_(rowTimeStamp),
      visitStatus_(TableStatus::Success())
  {
  }
  virtual void visit(TableColumnId columnId, uint8_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, int8_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, uint16_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, int16_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, uint32_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, int32_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, uint64_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, int64_t value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, float value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, double value) { visit_(columnId, value); }
  virtual void visit(TableColumnId columnId, const std::string& value) { visit_(columnId, value); }

  /**
   * Retrieves the status of row visitation.  Visitation will stop upon the first error encountered,
   * such as an invalid column ID in the row provided.
   */
  const TableStatus& visitStatus() const { return visitStatus_; }

  /** Finishes the transactions */
  void finish() { transactionMap_.clear(); }

private:

  /**
   * Template function that is responsible for placing an individual cell value into a column.
   * This is implemented by using subtable row-addition transactions, which abstract away
   * the need to monitor for splits.  Splits will occur once the transactions are deleted,
   * which occurs when this instance falls out of scope (due to smart pointer destructor).
   */
  template <typename T>
  void visit_(TableColumnId columnId, T& value)
  {
    // Do nothing if we've encountered an error
    if (visitStatus_.isError())
      return;
    // Find the appropriate subtable for the column
    SubTable* subTable = owner_.subTableForId_(columnId);
    if (subTable == NULL)
    {
      visitStatus_ = TableStatus::Error("Table column ID not found.");
      return;
    }
    // Either use existing transaction, or create a new transaction, to add the row
    SubTable::AddRowTransactionPtr xaction = findTransaction_(subTable);
    // Set the cell values
    visitStatus_ = xaction->setCellValue(columnId, value);
  }

  /**
   * Locates an existing transaction for the current row-add function.  If the
   * transaction has not yet been created, then we create a new transaction and
   * store it.  When the transaction falls out of scope, the split will occur
   * as part of the transaction's implementation (up to SubTable)
   */
  SubTable::AddRowTransactionPtr findTransaction_(SubTable* subTable)
  {
    std::map<SubTable*, SubTable::AddRowTransactionPtr>::const_iterator i = transactionMap_.find(subTable);
    if (i != transactionMap_.end())
      return i->second;
    // Table wants to be notified when/if the split occurs, to fix up pointers
    SubTable::SplitObserverPtr splitObserver(new MoveColumnsToNewSubTable(owner_));
    SubTable::AddRowTransactionPtr rv = subTable->addRow(rowTimeStamp_, splitObserver);
    transactionMap_[subTable] = rv;
    return rv;
  }

  std::map<SubTable*, SubTable::AddRowTransactionPtr> transactionMap_;
  Table& owner_;
  double rowTimeStamp_;
  TableStatus visitStatus_;
};

////////////////////////////////////////////////////////////////////////

Table::Table(TableManager& mgr, TableId tableId, const std::string& tableName, ObjectId ownerId, const DataLimitsProvider* dataLimits)
  : tableManager_(mgr),
    tableId_(tableId),
    tableName_(tableName),
    ownerId_(ownerId),
    endTime_(-1.0),
    nextId_(0),
    dataLimits_(dataLimits)
{
}

Table::~Table()
{
  tableManager_.removeTable(this);
  for (std::vector<SubTable*>::const_iterator i = subtables_.begin(); i != subtables_.end(); ++i)
    delete *i;
}

std::string Table::tableName() const
{
  return tableName_;
}

TableId Table::tableId() const
{
  return tableId_;
}

ObjectId Table::ownerId() const
{
  return ownerId_;
}

size_t Table::columnCount() const
{
  return columns_.size();
}

TableColumn* Table::column(TableColumnId id) const
{
  std::map<TableColumnId, TableToColumn>::const_iterator i = columns_.find(id);
  if (i != columns_.end())
    return i->second.second;
  return NULL;
}

TableColumn* Table::column(const std::string& name) const
{
  auto i = columnsByName_.find(name);
  if (i != columnsByName_.end())
    return i->second;

  return NULL;
}

SubTable* Table::subTableForId_(TableColumnId columnId) const
{
  std::map<TableColumnId, TableToColumn>::const_iterator i = columns_.find(columnId);
  if (i == columns_.end())
    return NULL;
  return i->second.first;
}

TableStatus Table::addColumn(const std::string& columnName, VariableType storageType, UnitType unitType, TableColumn** newColumn)
{
  if (columnName.empty())
    return TableStatus::Error("Unable to create column with empty name.");
  // Check for existing name
  TableColumn* existing = column(columnName);
  if (existing)
  {
    if (newColumn) *newColumn = existing;
    return TableStatus::Error("Column name already exists.");
  }

  // Find an empty subtable to add the new column
  SubTable* emptyTable = NULL;
  for (std::vector<SubTable*>::const_iterator i = subtables_.begin();
    i != subtables_.end() && emptyTable == NULL; ++i)
  {
    if ((*i)->empty())
      emptyTable = *i;
  }

  // Create the subtable if we have to
  TableColumn* returnColumn = NULL;
  if (emptyTable == NULL)
  {
    // Create an empty table with a new time container
    emptyTable = new SubTable(new DoubleBufferTimeContainer(), tableId_);
    // Add the column; possible to roll back this operation
    TableStatus rv = emptyTable->addColumn(columnName, nextId_, storageType, unitType, &returnColumn);
    if (rv.isError())
    {
      delete emptyTable;
      return rv;
    }
    if (newColumn) *newColumn = returnColumn;

    // TODO: Carry over data limiting values
    subtables_.push_back(emptyTable);
  }
  else
  {
    TableStatus rv = emptyTable->addColumn(columnName, nextId_, storageType, unitType, &returnColumn);
    if (newColumn) *newColumn = returnColumn;
    if (rv.isError()) return rv;
  }

  // At this point, we've used nextId_ and we have a valid column
  assert(returnColumn != NULL);
  // Save the column, and do any callbacks
  columns_[nextId_++] = std::make_pair(emptyTable, returnColumn);
  columnsByName_[columnName] = returnColumn;
  // Assertion failure means we somehow forgot to set newColumn return value
  assert(newColumn == NULL || *newColumn == returnColumn);
  // notify observers of new column
  fireOnAddColumn_(*returnColumn);
  return TableStatus::Success();
}

TableStatus Table::removeColumn(const std::string& columnName)
{
  auto colByNameIter = columnsByName_.find(columnName);
  if (colByNameIter == columnsByName_.end())
    return TableStatus::Error("Column \"" + columnName + "\" does not exist.");

  TableColumn* column = colByNameIter->second;
  if (!column)
  {
    // Shouldn't be storing a NULL column
    assert(0);
    return TableStatus::Error("Column \"" + columnName + "\" does not exist.");
  }
  auto columnsIter = columns_.find(column->columnId());
  // If it's in columnsByName_, it should be columns_
  assert(columnsIter != columns_.end());
  if (columnsIter == columns_.end())
    return TableStatus::Error("Column \"" + columnName + "\" does not exist.");

  // Fire off the observers before doing the actual remove
  fireOnPreRemoveColumn_(*column);

  auto subtable = columnsIter->second.first;
  if (!subtable)
  {
    // Shouldn't be storing a NULL subtable pointer
    assert(0);
    return TableStatus::Error("Column \"" + columnName + "\" does not exist.");
  }
  // Attempt to remove the column from the subtable
  TableStatus rv = subtable->removeColumn(column->columnId());
  if (rv.isError())
    return rv;

  // Note that subtables_ does not need to be updated even if the subtable is now empty since addColumn() can make use of empty sub tables in the table

  columns_.erase(columnsIter);
  columnsByName_.erase(colByNameIter);

  return TableStatus::Success();
}

/**
 * Collection of subtable iterators, used to implement the Table::accept()
 * method for row visitation.
 */
class IteratorCollection
{
public:
  /// Instantiates an iterator collection based on subtables, between beginTime and endTime
  IteratorCollection(const std::vector<SubTable*>& subTables, double beginTime, double endTime)
    : minimumTime_(std::numeric_limits<double>::max()),
      endTime_(endTime)
  {
    lowerBoundSubTables_(subTables, beginTime, endTime, subTables_, minimumTime_);
  }

  /// Processes visitors using the iterators.
  void accept(DataTable::RowVisitor& visitor)
  {
    // Loop through until we break out of endTime, or until all iterators are exhausted
    while (minimumTime_ < endTime_)
    {
      TableRow row;
      row.setTime(minimumTime_);
      double nextMinTime = fillRowsAtTime_(row, minimumTime_);

      // Alert our visitor, and break out early if it tells us to do so
      if (visitor.visit(row) == DataTable::RowVisitor::VISIT_STOP)
        return;

      // Assertion failure means the minimumTime_ was wrong, or fillRowsAtTime_ incorrectly
      // calculated the next time value after incrementing iterators
      assert(nextMinTime != minimumTime_);
      minimumTime_ = nextMinTime;
    }
  }
private:
  /// Minimum time among all active iterators
  double minimumTime_;
  /// End time of iteration
  double endTime_;
  /// Current view into the requested subtables, using iterators
  std::vector<SubTable::Iterator> subTables_;

  /// Responsible for initializing the iterators using lower_bound()
  void lowerBoundSubTables_(const std::vector<SubTable*>& subTables, double beginTime, double endTime,
                            std::vector<SubTable::Iterator>& vec, double& minimumTime) const
  {
    // Fill out a vector of iterators using lower_bound()
    double minTime = std::numeric_limits<double>::max();
    for (std::vector<SubTable*>::const_iterator i = subTables.begin(); i != subTables.end(); ++i)
    {
      SubTable::Iterator lowerBound = (*i)->lower_bound(beginTime);
      // Only save the iterators that may have data after them
      if (lowerBound.hasNext())
      {
        double nextTime = lowerBound.peekNext().time();
        if (nextTime < endTime)
        { // Save the iterator, update the minimum time found
          vec.push_back(lowerBound);
          minTime = simCore::sdkMin(minTime, nextTime);
        }
      }
    }
    // Record the output of the minimum time
    minimumTime = minTime;
  }

  /**
   * Fills out the row with values from all subtables at the time specified.  Returns the new lowest
   * minimum time for the collection of iterators.  Assumes that all iterators are at-or-after minimum time.
   * @param row Row to fill out
   * @param atTime Time value to fill out
   * @return New minimum time after iterators visited
   */
  double fillRowsAtTime_(TableRow& row, double atTime)
  {
    double nextMinTime = std::numeric_limits<double>::max();

    // Fill out a row with the contents of all subtables that have a time at atTime
    for (std::vector<SubTable::Iterator>::iterator i = subTables_.begin(); i != subTables_.end(); ++i)
    {
      // Continue the loop to next subtable if there is no "next" value for the iterator
      if (!i->hasNext())
        continue;

      double iterTime = i->peekNext().time();
      // Assertion failure means we didn't increment an iterator properly or atTime is wrong
      // ("wrong" means that it's after the minimum time amongst iterators)
      assert(iterTime >= atTime);

      if (iterTime == atTime)
      { // Save row and increment
        SubTable::IteratorData values = i->next();
        values.fillRow(row);
        // Save the next iterator's time
        iterTime = i->hasNext() ? i->peekNext().time() : std::numeric_limits<double>::max();
      }

      // At this point, iterTime should either be max() (got used and at end), or contains this iter's next time
      // Assertion failure means logic failure in this while() loop that needs full re-examination
      assert(iterTime == std::numeric_limits<double>::max() || iterTime == i->peekNext().time());

      // Update the minimum time for this loop
      nextMinTime = simCore::sdkMin(nextMinTime, iterTime);
    }

    // NOTE: Potential optimization: If we reached the end of an iterator, we could remove
    // it from the vector of iterators (subTables_).  This could improve performance when
    //  we have tables that have lots of sparse data inside the time bounds, but could hurt
    // performance with tables that are not very sparse inside the time bounds.

    // Assertion failure means that we didn't hit any valid iterators, but we're still looping.
    // Because of use of numeric limit's max(), this should never occur
    assert(nextMinTime != atTime);
    return nextMinTime;
  }
};

void Table::accept(double beginTime, double endTime, DataTable::RowVisitor& visitor) const
{
  IteratorCollection iterators(subtables_, beginTime, endTime);
  iterators.accept(visitor);
}

void Table::accept(DataTable::ColumnVisitor& visitor) const
{
  for (std::map<TableColumnId, TableToColumn>::const_iterator i = columns_.begin(); i != columns_.end(); ++i)
    visitor.visit(i->second.second);
}

TableStatus Table::addRow(const TableRow& row)
{
  if (row.empty())
    return TableStatus::Error("Cannot add empty row.");
  // Move cells into subtables.  Note that table split should occur after the visitor
  // falls out of scope (once its smart pointers are destroyed)
  TransferCellsToSubTables transferCells(*this, row.time());
  row.accept(transferCells);
  TableStatus rv = transferCells.visitStatus();
  transferCells.finish();

  // keep track of our latest time. NOTE: data limiting will always leave at least one row, so latest end time will always remain
  if (row.time() > endTime_)
    endTime_ = row.time();

  // Alert the Data Store that we have new time values on this entity.  This is internal for
  // Memory Table and Memory Data Store to keep the Data Store's NewUpdatesListener correct.
  tableManager_.fireOnNewRowData(*this, row.time());

  // notify observers of new row. NOTE: do this before data limiting check, as data limiting may remove this row if it is inserted prior to the last row
  fireOnAddRow_(row);

  // Do data limiting when rows are added
  // TODO: This could feasibly be optimized across the data store with a parallel for-each
  //   that does data limiting at preset intervals
  if (dataLimits_ != NULL)
  {
    size_t pointsLimit = 0;
    double secondsLimit = 0.0;
    if (dataLimits_->getLimits(*this, pointsLimit, secondsLimit).isSuccess())
    {
      limitData_(pointsLimit, secondsLimit);
    }
  }
  return rv;
}

void Table::limitData_(size_t numToKeep, double timeWindow)
{
  // Break out early if no limiting
  if (numToKeep == 0 && timeWindow <= 0.0)
    return;

  const double latestInvalidTime = (timeWindow <= 0) ? (-std::numeric_limits<double>::max()) : (endTime_ - timeWindow);
  for (std::vector<SubTable*>::const_iterator i = subtables_.begin(); i != subtables_.end(); ++i)
  {
    (*i)->limitData(numToKeep, latestInvalidTime, this, observers_);
  }
}

simData::DelayedFlushContainerPtr Table::flush(TableColumnId id)
{
  DelayedFlushContainerComposite* deq = new DelayedFlushContainerComposite();
  // If flushing all columns, iterate through all subtables
  if (id == -1)
  {
    for (std::vector<SubTable*>::const_iterator i = subtables_.begin(); i != subtables_.end(); ++i)
      deq->push_back((*i)->flush());
  }
  // If flushing only one column, send the flush only to the subtable containing that column.
  // Note that the flush can't go straight to the column because the subtable may need to split
  else
  {
    SubTable::SplitObserverPtr splitObserver(new MoveColumnsToNewSubTable(*this));
    auto toFlush = columns_.find(id);
    if (toFlush != columns_.end())
      toFlush->second.first->flush(id, splitObserver);
  }
  return DelayedFlushContainerPtr(deq);
}

void Table::addObserver(TableObserverPtr callback)
{
  observers_.push_back(callback);
}

void Table::removeObserver(TableObserverPtr callback)
{
  TableObserverList::iterator i = std::find(observers_.begin(), observers_.end(), callback);
  if (i != observers_.end())
    observers_.erase(i);
}

void Table::fireOnAddColumn_(const TableColumn& column) const
{
  for (TableObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onAddColumn(*const_cast<Table*>(this), column);
  }
}

void Table::fireOnAddRow_(const TableRow& row) const
{
  for (TableObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onAddRow(*const_cast<Table*>(this), row);
  }
}

// TODO make calls to these when rows/columns are removed, which is not currently implemented
void Table::fireOnPreRemoveColumn_(const TableColumn& column) const
{
  for (TableObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onPreRemoveColumn(*const_cast<Table*>(this), column);
  }
}

void Table::fireOnPreRemoveRow_(const TableRow& row) const
{
  for (TableObserverList::const_iterator i = observers_.begin(); i != observers_.end(); ++i)
  {
    (*i)->onPreRemoveRow(*const_cast<Table*>(this), row.time());
  }
}


} }
