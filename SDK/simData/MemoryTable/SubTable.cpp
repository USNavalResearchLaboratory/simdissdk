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
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simCore/String/Utils.h"
#include "simData/DataTable.h"
#include "simData/TableCellTranslator.h"
#include "simData/MemoryTable/DoubleBufferTimeContainer.h"
#include "simData/MemoryTable/DataColumn.h"
#include "simData/MemoryTable/SubTable.h"

namespace simData { namespace MemoryTable {

/**
 * The implementation of a transaction for adding rows provides methods to actually
 * set the values of cells in the subtable.  This class caches the deque position
 * values for performance, and then detects on destruction whether or not a split
 * was required to keep subtables null-less.
 */
class SubTable::AddRowTransactionImpl : public SubTable::AddRowTransaction
{
public:
  /// Instantiates a transaction to track splits and set cells
  AddRowTransactionImpl(SubTable& subTable, double timeStamp, SubTable::SplitObserverPtr splitObserver)
    : subTable_(subTable),
      timeStamp_(timeStamp),
      splitObserver_(splitObserver),
      rowIndex_(0),
      isFreshBin_(true),
      insertRow_(true),
      origTimeMapSize_(subTable.timeContainer_->size())
  {
    bool hadExactMatch = false;
    TimeContainer::Iterator iter = subTable.timeContainer_->findOrAddTime(timeStamp, &hadExactMatch);
    insertRow_ = !hadExactMatch;
    TimeContainer::IteratorData next = iter.next();
    rowIndex_ = next.index();
    isFreshBin_ = next.isFreshBin();
  }

  /// Destructor is responsible for detecting split
  virtual ~AddRowTransactionImpl()
  {
    splitIfNeeded_();
  }

  virtual TableStatus setCellValue(TableColumnId columnId, uint8_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, int8_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, uint16_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, int16_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, uint32_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, int32_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, uint64_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, int64_t value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, float value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, double value) { return setCellValue_(columnId, value); }
  virtual TableStatus setCellValue(TableColumnId columnId, const std::string& value) { const std::string cleanup = simCore::StringUtils::removeEscapeSlashes(value); return setCellValue_(columnId, cleanup); }

private:
  SubTable& subTable_;
  double timeStamp_;
  SubTable::SplitObserverPtr splitObserver_;
  size_t rowIndex_;
  bool isFreshBin_;
  bool insertRow_;
  /// Helps track double inserts, which can happen if setCellValue() called with same column more than once
  size_t origTimeMapSize_;

  /// Template that adds a T-type to the cell
  template <typename T>
  TableStatus setCellValue_(TableColumnId columnId, T& value)
  {
    DataColumn* column = subTable_.findColumn_(columnId);
    if (column == nullptr)
      return TableStatus::Error("Column does not exist in subtable.");
    if (!insertRow_)
      return column->replace(isFreshBin_, rowIndex_, value);
    // Here we catch the case of setting the same cell value more than once
    if (column->size() == origTimeMapSize_)
      column->insert(isFreshBin_, rowIndex_, value);
    else
    {
      assert(column->size() == origTimeMapSize_ + 1);
      column->replace(isFreshBin_, rowIndex_, value);
    }
    return TableStatus::Success();
  }

  void splitIfNeeded_()
  {
    // Not possible to split with 0 or 1 column
    if (subTable_.columns_.size() < 2)
      return;

    std::vector<DataColumn*> columnsForNewTable;
    removeNonMatchingColumns_(columnsForNewTable);
    // No split if no mismatch
    if (columnsForNewTable.empty())
      return;
    // Create a new table with the columns
    SubTable* newTable = initializeNewTable_(columnsForNewTable);
    notifyObserver_(newTable, columnsForNewTable);
  }

  void removeNonMatchingColumns_(std::vector<DataColumn*>& removedColumns)
  {
    // Get a vector of all columns where the size does not match
    size_t ourSize = subTable_.timeContainer_->size();
    removedColumns.clear();
    for (std::vector<DataColumn*>::const_iterator i = subTable_.columns_.begin();
      i != subTable_.columns_.end(); ++i)
    {
      if ((*i)->size() != ourSize)
        removedColumns.push_back(*i);
    }

    // Remove them from our subtable
    for (std::vector<DataColumn*>::const_iterator i = removedColumns.begin();
      i != removedColumns.end(); ++i)
    {
      subTable_.removeColumn_((*i)->columnId());
    }
  }

  SubTable* initializeNewTable_(const std::vector<DataColumn*>& startingColumns)
  {
    return new SubTable(*subTable_.timeContainer_, startingColumns, timeStamp_, subTable_.tableId_);
  }

  void notifyObserver_(SubTable* newTable, const std::vector<DataColumn*>& startingColumns)
  {
    if (splitObserver_.get() == nullptr)
    {
      // Indicates we have a memory leak because no one is available
      // to take ownership of the newly created subtable.
      assert(0);
      return;
    }
    std::vector<TableColumnId> idVec;
    for (std::vector<DataColumn*>::const_iterator i = startingColumns.begin(); i != startingColumns.end(); ++i)
      idVec.push_back((*i)->columnId());
    splitObserver_->notifySplit(&subTable_, newTable, idVec);
  }

};

/////////////////////////////////////////////////////////////////

/**
 * IteratorImpl is a simple wrapper around TimeContainer::Iterator, but abides
 * by GenericIteratorImpl's interface.
 */
class SubTable::IteratorImpl : public GenericIteratorImpl<SubTable::IteratorData>
{
public:
  /** Construct a new IteratorImpl */
  IteratorImpl(const SubTable* owner, TimeContainer::Iterator timeIter)
    : owner_(owner),
      timeIter_(timeIter)
  {
  }

  virtual const IteratorData next()
  {
    return IteratorData(owner_, timeIter_.next());
  }

  virtual const IteratorData peekNext() const
  {
    return IteratorData(owner_, timeIter_.peekNext());
  }

  virtual const IteratorData previous()
  {
    return IteratorData(owner_, timeIter_.previous());
  }

  virtual const IteratorData peekPrevious() const
  {
    return IteratorData(owner_, timeIter_.peekPrevious());
  }

  virtual void toFront()
  {
    timeIter_.toFront();
  }

  virtual void toBack()
  {
    timeIter_.toBack();
  }

  virtual bool hasNext() const
  {
    return timeIter_.hasNext();
  }

  virtual bool hasPrevious() const
  {
    return timeIter_.hasPrevious();
  }

  virtual GenericIteratorImpl<IteratorData>* clone() const
  {
    return new IteratorImpl(owner_, timeIter_);
  }

private:
  const SubTable* owner_;
  TimeContainer::Iterator timeIter_;
};

/////////////////////////////////////////////////////////////////

SubTable::IteratorData::IteratorData(const SubTable* subTable, const TimeContainer::IteratorData& timeIterData)
  : subTable_(subTable),
    timeIterData_(timeIterData)
{
}

SubTable::IteratorData::IteratorData(const IteratorData& copyConstructor)
  : subTable_(copyConstructor.subTable_),
    timeIterData_(copyConstructor.timeIterData_)
{
}

SubTable::IteratorData::~IteratorData()
{
}

SubTable::IteratorData& SubTable::IteratorData::operator=(const SubTable::IteratorData& copy)
{
  if (this != &copy)
  {
    subTable_ = copy.subTable_;
    timeIterData_ = copy.timeIterData_;
  }
  return *this;
}

double SubTable::IteratorData::time() const
{
  return timeIterData_.time();
}

void SubTable::IteratorData::fillRow(TableRow& row) const
{
  subTable_->fillRow_(timeIterData_, row);
}

/////////////////////////////////////////////////////////////////

SubTable::SubTable(TimeContainer* newTimeContainer, TableId tableId)
  : timeContainer_(newTimeContainer),
  tableId_(tableId)
{
  // Assertion failure means the caller gave us a bad time container to use
  assert(timeContainer_ != nullptr && timeContainer_->size() == 0);
}

SubTable::SubTable(const TimeContainer& copyTimes, const std::vector<DataColumn*>& withColumns, double withoutTimeStamp, TableId tableId)
  : timeContainer_(copyTimes.clone()),
  tableId_(tableId)
{
  TimeContainer::Iterator removeIter = timeContainer_->find(withoutTimeStamp);
  // Assertion failure means this is used for more than just split, or that split
  // occurred and somehow the "extra" time wasn't added in to the original table's
  // time container.  Either way, some logic error occurred.
  assert(removeIter.hasNext());
  timeContainer_->erase(removeIter, TimeContainer::ERASE_FIXOFFSETS);

  for (std::vector<DataColumn*>::const_iterator i = withColumns.begin(); i != withColumns.end(); ++i)
  {
    columns_.push_back(*i);
    // Assertion failure means vector had duplicate column IDs
    assert(columnMap_.find((*i)->columnId()) == columnMap_.end());
    columnMap_[(*i)->columnId()] = *i;
    // Fix time container ownership so column points to our container and not old owner
    (*i)->replaceTimeContainer(timeContainer_);
  }
}

SubTable::~SubTable()
{
  delete timeContainer_;
  for (std::vector<DataColumn*>::const_iterator i = columns_.begin(); i != columns_.end(); ++i)
    delete *i;
}

size_t SubTable::rowCount() const
{
  return timeContainer_->size();
}

size_t SubTable::columnCount() const
{
  return columns_.size();
}

bool SubTable::empty() const
{
  return timeContainer_->empty();
}

TableStatus SubTable::addColumn(const std::string& columnName, TableColumnId columnId,
                                       VariableType storageType, UnitType unitType, TableColumn** newCol)
{
  if (!empty())
    return TableStatus::Error("Attempting to add column to a non-empty subtable, violates NULL-less state.");
  DataColumn* newColumn = new DataColumn(timeContainer_, columnName, tableId_, columnId, storageType, unitType);
  columns_.push_back(newColumn);
  if (newCol != nullptr) *newCol = newColumn;

  // Assertion failure means reuse of IDs
  assert(columnMap_.find(columnId) == columnMap_.end());
  columnMap_[columnId] = newColumn;
  return TableStatus::Success();
}

TableStatus SubTable::removeColumn(TableColumnId columnId)
{
  std::map<TableColumnId, DataColumn*>::iterator mapIter = columnMap_.find(columnId);
  if (mapIter == columnMap_.end())
    return TableStatus::Error("Unrecognized column ID to remove from subtable.");

  DataColumn* column = mapIter->second;
  TableStatus rv = removeColumn_(columnId);
  if (rv.isError())
    return rv;

  delete column;

  // If that was the only column in the subtable, the table is now empty.  Clear the time container
  if (columns_.empty())
    timeContainer_->flush();

  return TableStatus::Success();
}

TableStatus SubTable::removeColumn_(TableColumnId columnId)
{
  // Remove from the map
  std::map<TableColumnId, DataColumn*>::iterator mapIter = columnMap_.find(columnId);
  if (mapIter == columnMap_.end())
    return TableStatus::Error("Unrecognized column ID to remove from subtable.");
  columnMap_.erase(mapIter);

  // Remove from the column vector
  for (std::vector<DataColumn*>::iterator i = columns_.begin(); i != columns_.end(); ++i)
  {
    if ((*i)->columnId() == columnId)
    {
      columns_.erase(i);
      // Do not delete the data, because it will be moving to another subtable
      return TableStatus::Success();
    }
  }

  // Because the map stores IDs and returns on not-found, this assert should
  // not trigger.  If it does, that means we're getting duplicate IDs or
  // the map contains things that the vector does not, or perhaps that
  // the column's internal ID changed.
  assert(0);
  return TableStatus::Error("Invalid column ID to remove from subtable.");
}

simData::DelayedFlushContainerPtr SubTable::flush(TableColumnId id, SplitObserverPtr splitObserver)
{
  DelayedFlushContainerComposite* deq = new DelayedFlushContainerComposite();
  // Simple case: Flushing all columns or flushing the only column in the sub table.  No need to split
  if (id == -1 || (columns_.size() == 1 && columns_.front()->columnId() == id))
  {
    deq->push_back(timeContainer_->flush());
    for (std::vector<DataColumn*>::const_iterator i = columns_.begin(); i != columns_.end(); ++i)
      deq->push_back((*i)->flush());

    return DelayedFlushContainerPtr(deq);
  }

  // Complex case: Flushing one column among many
  if (splitObserver.get() == nullptr)
  {
    // Split observer is required when flushing a single column.
    // Without it, no one will take ownership of the new sub table
    assert(0);
    return DelayedFlushContainerPtr(deq);
  }

  DataColumn* removedCol = nullptr;
  for (std::vector<DataColumn*>::const_iterator i = columns_.begin(); i != columns_.end(); ++i)
  {
    if ((*i)->columnId() == id)
    {
      deq->push_back((*i)->flush());
      removedCol = *i;
      break;
    }
  }

  // Didn't find the column in this sub table.  Nothing to do, return
  if (removedCol == nullptr)
    return DelayedFlushContainerPtr(deq);

  // Split off the flushed column to a new subtable
  TimeContainer* newContainer = timeContainer_->clone();
  newContainer->flush();
  SubTable* newTable = new SubTable(newContainer, tableId_);
  newTable->takeColumn_(removedCol);
  removeColumn_(id);
  std::vector<TableColumnId> idVec;
  idVec.push_back(id);
  splitObserver->notifySplit(this, newTable, idVec);
  return DelayedFlushContainerPtr(deq);
}

void SubTable::flush(double startTime, double endTime)
{
  timeContainer_->flush(columns_, startTime, endTime);
}

DataColumn* SubTable::findColumn_(TableColumnId columnId) const
{
  std::map<TableColumnId, DataColumn*>::const_iterator i = columnMap_.find(columnId);
  if (i == columnMap_.end())
    return nullptr;
  return i->second;
}

TableStatus SubTable::interpolate(TableColumnId columnId, double time, double& value, const TableColumn::Interpolator* interpolator) const
{
  TableColumn* column = findColumn_(columnId);
  if (column == nullptr)
    return TableStatus::Error("Invalid column index.");
  return column->interpolate(value, time, interpolator);
}

SubTable::AddRowTransactionPtr SubTable::addRow(double timeStamp, SplitObserverPtr splitObserver)
{
  return AddRowTransactionPtr(new AddRowTransactionImpl(*this, timeStamp, splitObserver));
}

SubTable::Iterator SubTable::begin()
{
  return Iterator(new IteratorImpl(this, timeContainer_->begin()));
}

SubTable::Iterator SubTable::end()
{
  return Iterator(new IteratorImpl(this, timeContainer_->end()));
}

SubTable::Iterator SubTable::lower_bound(double timeValue)
{
  return Iterator(new IteratorImpl(this, timeContainer_->lower_bound(timeValue)));
}

SubTable::Iterator SubTable::upper_bound(double timeValue)
{
  return Iterator(new IteratorImpl(this, timeContainer_->upper_bound(timeValue)));
}

void SubTable::fillRow_(const TimeContainer::IteratorData& timeIdxData, TableRow& row) const
{
  for (std::vector<DataColumn*>::const_iterator i = columns_.begin(); i != columns_.end(); ++i)
    (*i)->fillRow(timeIdxData, row);
}

void SubTable::limitData(size_t maxPoints, double latestInvalidTime, DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers)
{
  timeContainer_->limitData(maxPoints, latestInvalidTime, columns_, table, observers);
}

void SubTable::takeColumn_(DataColumn* column)
{
  columns_.push_back(column);
  // Assertion failure means vector had duplicate column IDs
  assert(columnMap_.find((column)->columnId()) == columnMap_.end());
  columnMap_[(column)->columnId()] = column;
  // Fix time container ownership so column points to our container and not old owner
  column->replaceTimeContainer(timeContainer_);
}

} }
