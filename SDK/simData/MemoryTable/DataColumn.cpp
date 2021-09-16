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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <deque>
#include <cassert>
#include "simCore/Calc/Interpolation.h"
#include "simData/DataTable.h"
#include "simData/TableCellTranslator.h"
#include "simData/MemoryTable/DataColumn.h"

namespace simData { namespace MemoryTable {

/**
 * Implementation of the TableColumn::IteratorData interface.  This interface provides
 * access to get and set values inside the data container so that a user can iterate
 * through a data table and view/change its contents.
 */
class DataColumn::IteratorDataImpl : public simData::TableColumn::IteratorData
{
public:
  /** Constructs a new IteratorDataImpl */
  IteratorDataImpl(DataContainer* data, size_t position, double time)
    : data_(data),
      position_(position),
      time_(time)
  {
  }
  virtual ~IteratorDataImpl()
  {
  }

  virtual double time() const
  {
    return time_;
  }

  virtual TableStatus getValue(uint8_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(int8_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(uint16_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(int16_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(uint32_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(int32_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(uint64_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(int64_t& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(float& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(double& value) const { return data_->getValue(position_, value); }
  virtual TableStatus getValue(std::string& value) const { return data_->getValue(position_, value); }

  virtual TableStatus setValue(uint8_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(int8_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(uint16_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(int16_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(uint32_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(int32_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(uint64_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(int64_t value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(float value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(double value) { return data_->replace(position_, value); }
  virtual TableStatus setValue(const std::string& value) { return data_->replace(position_, value); }

private:
  DataContainer* data_;
  size_t position_;
  double time_;
};

/////////////////////////////////////////////////////////////////

/**
 * IteratorImpl is a simple wrapper around TimeContainer::Iterator, but abides
 * by GenericIteratorImpl's interface.
 */
class DataColumn::ColumnIteratorImpl : public TableColumn::IteratorImpl
{
public:
  /** Constructs a new ColumnIteratorImpl */
  ColumnIteratorImpl(DataContainer* freshData, DataContainer* staleData, TimeContainer::Iterator timeIter)
    : freshData_(freshData),
      staleData_(staleData),
      timeIter_(timeIter)
  {
  }

  virtual const IteratorDataPtr next()
  {
    return IteratorDataPtr(newIteratorDataImpl_(timeIter_.next()));
  }

  virtual const IteratorDataPtr peekNext() const
  {
    return IteratorDataPtr(newIteratorDataImpl_(timeIter_.peekNext()));
  }

  virtual const IteratorDataPtr previous()
  {
    return IteratorDataPtr(newIteratorDataImpl_(timeIter_.previous()));
  }

  virtual const IteratorDataPtr peekPrevious() const
  {
    return IteratorDataPtr(newIteratorDataImpl_(timeIter_.peekPrevious()));
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

  virtual GenericIteratorImpl<IteratorDataPtr>* clone() const
  {
    return new ColumnIteratorImpl(freshData_, staleData_, timeIter_);
  }

private:
  DataContainer* freshData_;
  DataContainer* staleData_;
  TimeContainer::Iterator timeIter_;

  /** Helper method to return an IteratorDataImpl for the given time container iterator data */
  IteratorDataImpl* newIteratorDataImpl_(TimeContainer::IteratorData data) const
  {
    if (data.isFreshBin())
      return new IteratorDataImpl(freshData_, data.index(), data.time());
    return new IteratorDataImpl(staleData_, data.index(), data.time());
  }
};

/////////////////////////////////////////////////////////////////

/**
 * Template implementation of a Data Container implements all methods for the
 * given template type.
 */
template <typename T>
class DataContainerT : public DataContainer
{
public:
  DataContainerT() {}
  virtual ~DataContainerT() {}

  // Insert item into the data container at the given position
  virtual void insert(size_t position, uint8_t value) { insert_(position, value); }
  virtual void insert(size_t position, int8_t value) { insert_(position, value); }
  virtual void insert(size_t position, uint16_t value) { insert_(position, value); }
  virtual void insert(size_t position, int16_t value) { insert_(position, value); }
  virtual void insert(size_t position, uint32_t value) { insert_(position, value); }
  virtual void insert(size_t position, int32_t value) { insert_(position, value); }
  virtual void insert(size_t position, uint64_t value) { insert_(position, value); }
  virtual void insert(size_t position, int64_t value) { insert_(position, value); }
  virtual void insert(size_t position, float value) { insert_(position, value); }
  virtual void insert(size_t position, double value) { insert_(position, value); }
  virtual void insert(size_t position, const std::string& value) { insert_(position, value); }

  // Replace item in the data container at the given position
  virtual TableStatus replace(size_t position, uint8_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, int8_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, uint16_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, int16_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, uint32_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, int32_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, uint64_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, int64_t value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, float value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, double value) { return replace_(position, value); }
  virtual TableStatus replace(size_t position, const std::string& value) { return replace_(position, value); }

  // Retrieve the item in the data container at the given position
  virtual TableStatus getValue(size_t position, uint8_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, int8_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, uint16_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, int16_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, uint32_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, int32_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, uint64_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, int64_t& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, float& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, double& value) const { return getValue_(position, value); }
  virtual TableStatus getValue(size_t position, std::string& value) const { return getValue_(position, value); }

  ///Sets the row's cell to our value
  virtual TableStatus copyToRowCell(TableRow& row, simData::TableColumnId whichCell, size_t position) const
  {
    T value;
    // Note: uses std::string::operator=(char), else POD.operator=.  Cannot use constructor.  Avoids warnings.
    value = static_cast<char>(0);
    TableStatus rv = getValue_(position, value);
    if (rv.isError())
      return rv;
    row.setValue(whichCell, value);
    return TableStatus::Success();
  }

  /** Removes the entries starting at the given index */
  virtual void erase(size_t position, size_t number = 1)
  {
    if (position < size())
    {
      // Performance optimization (SIMSDK-260): pop_front() when able
      if ((position == 0) && (number == 1))
        data_.pop_front();
      else
        data_.erase(data_.begin() + position, data_.begin() + position + number);
    }
  }
  /** Total size of the data structure */
  virtual size_t size() const { return data_.size(); }
  /** True if the structure is empty */
  virtual bool empty() const { return data_.empty(); }
  /** Removes all items from container */
  virtual void clear() { data_.clear(); }

private:
  /**
   * All data is stored in a deque.  Deque chosen for faster insertion
   * (due to no need to double size on add, relative to vector), and faster
   * removal (due to no need to shift all items every time).
   */
  typename std::deque<T> data_;

  /// Template implementation of insertion at position
  template <typename DataType>
  void insert_(size_t position, const DataType& value)
  {
    typename std::deque<T>::iterator pos = data_.end();
    if (position < size())
      pos = data_.begin() + position;
    T localValue;
    TableCellTranslator::cast(value, localValue);
    data_.insert(pos, localValue);
  }

  /// Template implementation of replacement at position
  template <typename DataType>
  TableStatus replace_(size_t position, const DataType& value)
  {
    if (position >= size())
      return TableStatus::Error("Column replacement: invalid index.");
    TableCellTranslator::cast(value, *(data_.begin() + position));
    return TableStatus::Success();
  }

  /// Template implementation of get-value at position
  template <typename DataType>
  TableStatus getValue_(size_t position, DataType& value) const
  {
    if (position >= size())
      return TableStatus::Error("Column getValue: invalid index.");
    TableCellTranslator::cast(*(data_.begin() + position), value);
    return TableStatus::Success();
  }
};

/////////////////////////////////////////////////////////////////

/** Instantiates a new data column. */
DataColumn::DataColumn(TimeContainer* timeContainer, const std::string& columnName, TableId tableId, TableColumnId columnId, VariableType storageType, UnitType unitType)
  : timeContainer_(timeContainer),
    freshData_(nullptr),
    staleData_(nullptr),
    name_(columnName),
    tableId_(tableId),
    id_(columnId),
    variableType_(storageType),
    unitType_(unitType)
{
  // Assertion failure means invalid precondition
  assert(timeContainer_ != nullptr);
  freshData_ = newDataContainer_(variableType_);
  staleData_ = newDataContainer_(variableType_);
  // Assertion means we didn't make a data container, which would only happen if
  // someone added a new variable type.  Note this would show up as a Linux
  // compile time warning as well, as long as switch() doesn't have a default case.
  assert(freshData_ != nullptr);
  assert(staleData_ != nullptr);
}

/** Columns contain no dynamic memory */
DataColumn::~DataColumn()
{
  delete staleData_;
  delete freshData_;
}

void DataColumn::erase(bool freshContainer, size_t position, size_t number)
{
  dataContainer_(freshContainer)->erase(position, number);
}

size_t DataColumn::size() const
{
  return freshData_->size() + staleData_->size();
}

bool DataColumn::empty() const
{
  return freshData_->empty() && staleData_->empty();
}

/// Permits flushing in a thread
class DataColumn::FlushContainer : public simData::DelayedFlushContainer
{
public:
  /** Constructor */
  explicit FlushContainer(DataColumn& column)
    : freshData_(column.freshData_),
      staleData_(column.staleData_)
  {
    column.freshData_ = column.newDataContainer_(column.variableType());
    column.staleData_ = column.newDataContainer_(column.variableType());
  }
  virtual ~FlushContainer()
  {
    delete freshData_;
    delete staleData_;
    freshData_ = nullptr;
    staleData_ = nullptr;
  }
private:
  DataContainer* freshData_;
  DataContainer* staleData_;
};

DelayedFlushContainerPtr DataColumn::flush()
{
  // Optimize for case where both are empty (no memory allocation)
  if (freshData_->empty() && staleData_->empty())
    return DelayedFlushContainerPtr();
  return DelayedFlushContainerPtr(new FlushContainer(*this));
}

TableId DataColumn::tableId() const
{
  return tableId_;
}

TableColumnId DataColumn::columnId() const
{
  return id_;
}

std::string DataColumn::name() const
{
  return name_;
}

VariableType DataColumn::variableType() const
{
  return variableType_;
}

UnitType DataColumn::unitType() const
{
  return unitType_;
}

void DataColumn::setUnitType(UnitType units)
{
  unitType_ = units;
}

TableStatus DataColumn::interpolate(double& value, double time, const Interpolator* interpolator) const
{
  if (timeContainer_->empty())
    return TableStatus::Error("No data.");
  TimeContainer::Iterator lowerBound = timeContainer_->lower_bound(time);
  if (!lowerBound.hasNext()) // Past the end?
  {
    // Assertion failure means either time container empty() (returned above), or logic error in time container
    assert(lowerBound.hasPrevious());
    TimeContainer::IteratorData prev = lowerBound.previous();
    return getValue(prev.isFreshBin(), prev.index(), value);
  }

  // Does the time match exactly?
  TimeContainer::IteratorData atOrAfter(lowerBound.peekNext());
  if (atOrAfter.time() == time)
  {
    // Exact match
    return getValue(atOrAfter.isFreshBin(), atOrAfter.index(), value);
  }
  // Assertion failure means this algorithm has a logic error in lower_bound usage
  assert(atOrAfter.time() > time);

  if (!lowerBound.hasPrevious())
  {
    // "time" is before the start time of the container; no extrapolation
    getValue(atOrAfter.isFreshBin(), atOrAfter.index(), value); // still return a valid value though
    return TableStatus::Error("Requested time before start of container.");
  }
  TimeContainer::IteratorData before(lowerBound.previous());
  // Assertion failure means iterators are accessing out of order, or internal logic error
  assert(before.time() < atOrAfter.time());

  // Pull out the high/low values
  double lowValue = 0.0;
  double highValue = 0.0;
  TableStatus rv = getValue(before.isFreshBin(), before.index(), lowValue);
  if (!rv.isSuccess())
    return rv;
  rv = getValue(atOrAfter.isFreshBin(), atOrAfter.index(), highValue);
  if (!rv.isSuccess())
    return rv;

  // Either interpolate linearly if interpolator is NULL, or use custom interpolation
  if (interpolator)
    value = interpolator->interpolate(this, lowValue, highValue, before.time(), time, atOrAfter.time());
  else
    value = simCore::linearInterpolate(lowValue, highValue, before.time(), time, atOrAfter.time());
  return TableStatus::Success();
}

/// Fixes the time container, i.e. after a split.  Responsibility of SubTable to keep up to date
void DataColumn::replaceTimeContainer(TimeContainer* newTimes)
{
  // Assertion failure means invalid precondition
  assert(newTimes != nullptr);
  timeContainer_ = newTimes;
}

/// Copies data from the data container into the row specified
void DataColumn::fillRow(const TimeContainer::IteratorData& timeIdxData, TableRow& row) const
{
  if (timeIdxData.isFreshBin())
    freshData_->copyToRowCell(row, id_, timeIdxData.index());
  else
    staleData_->copyToRowCell(row, id_, timeIdxData.index());
}

// Start iteration at the beginning of the container (smallest time).
TableColumn::Iterator DataColumn::begin() const
{
  return Iterator(new ColumnIteratorImpl(freshData_, staleData_, timeContainer_->begin()));
}

// Iterator representing the back of the container (largest time).
TableColumn::Iterator DataColumn::end() const
{
  return Iterator(new ColumnIteratorImpl(freshData_, staleData_, timeContainer_->end()));
}

// Returns lower_bound() iterator into container
TableColumn::Iterator DataColumn::lower_bound(double timeValue) const
{
  return Iterator(new ColumnIteratorImpl(freshData_, staleData_, timeContainer_->lower_bound(timeValue)));
}

// Returns upper_bound() iterator into container
TableColumn::Iterator DataColumn::upper_bound(double timeValue) const
{
  return Iterator(new ColumnIteratorImpl(freshData_, staleData_, timeContainer_->upper_bound(timeValue)));
}

TableColumn::Iterator DataColumn::findAtOrBeforeTime(double timeValue) const
{
  return Iterator(new ColumnIteratorImpl(freshData_, staleData_, timeContainer_->findTimeAtOrBeforeGivenTime(timeValue)));
}

DataContainer* DataColumn::newDataContainer_(simData::VariableType variableType) const
{
  switch (variableType)
  {
  case VT_UINT8: return new DataContainerT<uint8_t>();
  case VT_INT8: return new DataContainerT<int8_t>();
  case VT_UINT16: return new DataContainerT<uint16_t>();
  case VT_INT16: return new DataContainerT<int16_t>();
  case VT_UINT32: return new DataContainerT<uint32_t>();
  case VT_INT32: return new DataContainerT<int32_t>();
  case VT_UINT64: return new DataContainerT<uint64_t>();
  case VT_INT64: return new DataContainerT<int64_t>();
  case VT_FLOAT: return new DataContainerT<float>();
  case VT_DOUBLE: return new DataContainerT<double>();
  case VT_STRING: return new DataContainerT<std::string>();
  }
  return nullptr;
}

void DataColumn::swapFreshStaleData()
{
  // Clear out the old stale data, which will soon become new
  staleData_->clear();
  DataContainer* tmpNew = freshData_;
  freshData_ = staleData_;
  staleData_ = tmpNew;
}

DataContainer* DataColumn::dataContainer_(bool freshContainer) const
{
  return freshContainer ? freshData_ : staleData_;
}

int DataColumn::getTimeRange(double& begin, double& end) const
{
  return timeContainer_->getTimeRange(begin, end);
}
} }
