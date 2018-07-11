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
#include <algorithm>
#include <limits>
#include <cassert>
#include "simData/MemoryTable/DataColumn.h"
#include "simData/MemoryTable/TimeContainerDeque.h"

namespace simData { namespace MemoryTable {

namespace
{

/**
 * Comparison operator for lower_bound() and upper_bound operations.  Extra operations
 * added to permit pair-to-pair, double-to-pair, pair-to-double, and double-to-double
 * comparisons.  The alternative is to do the upper_bound/lower_bound with respect to
 * a std::pair<> rather than a time value, which is less intuitive and requires temporary values.
 */
struct LessThan
{
  bool operator()(const std::pair<double, size_t>& lhs, const std::pair<double, size_t>& rhs) const
  {
    return lhs.first < rhs.first;
  }
  bool operator()(double lhs, double rhs) const
  {
    return lhs < rhs;
  }
  bool operator()(const std::pair<double, size_t>& lhs, double rhs) const
  {
    return lhs.first < rhs;
  }
  bool operator()(double lhs, const std::pair<double, size_t>& rhs) const
  {
    return lhs < rhs.first;
  }
};
}

/////////////////////////////////////////////////////////////////////////////

/// Implements delayed flush by doing a container swap
class TimeContainerDeque::FlushContainer : public DelayedFlushContainer
{
public:
  /** Constructs a new FlushContainer on the times provided */
  explicit FlushContainer(TimeIndexDeque& times)
    : times_(NULL)
  {
    // Optimize the case where vectors are empty
    if (!times.empty())
    { // Note that deque::swap() is constant time
      times_ = new TimeIndexDeque();
      times.swap(*times_);
    }
  }
  virtual ~FlushContainer()
  {
    delete times_;
    times_ = NULL;
  }
private:
  TimeIndexDeque* times_;
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Implementation of the TimeContainer's iterator impl interface.  Allows a
 * user to step through the double buffered time container in time-sorted order.
 */
class TimeContainerDeque::SingleBufferIterator : public TimeContainer::IteratorImpl
{
public:
  /** Sentinel value for invalid values */
  static const IteratorData INVALID_VALUE;

  /** Copy constructor */
  SingleBufferIterator(const SingleBufferIterator& copyConstructor)
    : binIndex_(0),
      ownerBin_(copyConstructor.ownerBin_),
      iter_(copyConstructor.iter_)
  {
  }

  /** Constructor that takes in a time index deque */
  SingleBufferIterator(TimeContainerDeque::TimeIndexDeque& deque,
    const TimeContainerDeque::TimeIndexDeque::iterator& iter)
    : binIndex_(0),
      ownerBin_(deque),
      iter_(iter)
  {
  }

  virtual ~SingleBufferIterator()
  {
  }

  /** Retrieves next element and increments iterator to position after that element
  * @return Element after original iterator position,
  * or INVALID_VALUE if no such element
  */
  virtual const IteratorData next()
  {
    if (iter_ == ownerBin_.end())
    {
      // Iterator points to end, invalid next()
      assert(peekNext() == INVALID_VALUE);
      return INVALID_VALUE;
    }
    IteratorData returnValue(*iter_, true);

    // Move the iterator, it definitely needs to change.
    ++iter_;
    return returnValue;
  }

  /** Retrieves next element and does not change iterator position */
  virtual const IteratorData peekNext() const
  {
    // Primary bin is reflected by binIndex_.  If it doesn't have a next(), then there
    // is a failure somewhere.  So we only need to inspect its value
    if (iter_ == ownerBin_.end())
    {
      return INVALID_VALUE;
    }
    return IteratorData(*iter_, true);
  }

  /** Retrieves previous element and decrements iterator to position before that element
  * @return Element before original iterator position,
  * or INVALID_VALUE if no such element
  */
  virtual const IteratorData previous()
  {
    // Don't make copies (unlike peekPrevious)
    return previous_(iter_);
  }

  /** Retrieves previous element and does not change iterator position */
  virtual const IteratorData peekPrevious() const
  {
    // Make a copy of iterators to prevent change
    TimeIndexDeque::iterator iter = iter_;
    return previous_(iter);
  }

  /** Resets the iterator to the front of the data structure, before the first element */
  virtual void toFront()
  {
    iter_ = ownerBin_.begin();
  }
  /** Sets the iterator to the end of the data structure, after the last element */
  virtual void toBack()
  {
    iter_ = ownerBin_.end();
  }

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  virtual bool hasNext() const
  {
    // Are we at the end() of primary and secondary?
    return iter_ != ownerBin_.end();
  }
  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  virtual bool hasPrevious() const
  {
    return iter_ != ownerBin_.begin();
  }

  /** Create a copy of the actual implementation */
  virtual GenericIteratorImpl<IteratorData>* clone() const
  {
    return new SingleBufferIterator(*this);
  }

  /** Removes the iterator from the parent container */
  void erase(TimeContainer::EraseBehavior eraseBehavior)
  {
    if (iter_ != ownerBin_.end())
    {
      // Decrement the contents of the bin
      if (eraseBehavior == TimeContainer::ERASE_FIXOFFSETS)
        decreaseAllIndices_(iter_->second);

      // Performance optimization (SIMSDK-260): pop_front() when able
      if (iter_ == ownerBin_.begin())
        ownerBin_.pop_front();
      else
        ownerBin_.erase(iter_);
      // Reset the iterators to keep the state consistent.
      iter_ = ownerBin_.end();
    }
  }

private:
  size_t binIndex_;
  /// Points to the bins in the owning container; soft copy, not owned
  TimeContainerDeque::TimeIndexDeque& ownerBin_;
  TimeContainerDeque::TimeIndexDeque::iterator iter_;

  /// Helper function that adjusts indices in case of erase()
  void decreaseAllIndices_(size_t greaterThan) const
  {
    // Assert precondition
    for (TimeContainerDeque::TimeIndexDeque::iterator i = ownerBin_.begin(); i != ownerBin_.end(); ++i)
    {
      if (i->second > greaterThan)
        --(i->second);
    }
  }

  /** Generic previous function will modify the incoming iterators */
  IteratorData previous_(TimeIndexDeque::iterator& iter) const
  {
    // The precondition is that binIndex_ points to the next item, but doesn't mean the
    // previous item is in binIndex_.  We must check both queues, unlike next()

    // At the beginning, cannot decrement
    if (iter == ownerBin_.begin())
      return INVALID_VALUE;
    --iter;
    return IteratorData(*iter, true);
  }
};
const TimeContainer::IteratorData TimeContainerDeque::SingleBufferIterator::INVALID_VALUE(std::numeric_limits<double>::max(), 0, true);

/////////////////////////////////////////////////////////////////////////////

TimeContainerDeque::TimeContainerDeque()
{
}

TimeContainerDeque::TimeContainerDeque(const TimeContainerDeque& copyFrom)
  : times_(copyFrom.times_)
{
}

TimeContainerDeque::~TimeContainerDeque()
{
}

TimeContainer* TimeContainerDeque::clone() const
{
  return new TimeContainerDeque(*this);
}

size_t TimeContainerDeque::size() const
{
  return times_.size();
}

bool TimeContainerDeque::empty() const
{
  return times_.empty();
}

TimeContainer::Iterator TimeContainerDeque::begin()
{
  return newIterator_(times_.begin());
}

TimeContainer::Iterator TimeContainerDeque::end()
{
  return newIterator_(times_.end());
}

TimeContainer::Iterator TimeContainerDeque::lower_bound(double timeValue)
{
  return newIterator_(lowerBound_(timeValue));
}

TimeContainer::Iterator TimeContainerDeque::upper_bound(double timeValue)
{
  return newIterator_(upperBound_(timeValue));
}

TimeContainer::Iterator TimeContainerDeque::findTimeAtOrBeforeGivenTime(double timeValue)
{
  Iterator rv = lower_bound(timeValue);
  if (rv.hasNext() && rv.peekNext() == timeValue)
    return rv;
  if (!rv.hasPrevious())
    return end();
  rv.previous();
  assert(rv.peekNext() <= timeValue);
  return rv;
}

TimeContainer::Iterator TimeContainerDeque::find(double timeValue)
{
  bool exactMatch = false;
  TimeIndexDeque::iterator iter = lowerBound_(timeValue, &exactMatch);
  if (exactMatch)
    return newIterator_(iter);
  return end();
}

TimeContainer::Iterator TimeContainerDeque::findOrAddTime(double timeValue, bool* exactMatch)
{
  // Look for the value in the container
  bool tmpExactMatch = false;
  TimeIndexDeque::iterator iter = lowerBound_(timeValue, &tmpExactMatch);
  if (exactMatch)
    *exactMatch = tmpExactMatch;
  if (tmpExactMatch)
  { // Time was found in container
    return newIterator_(iter);
  }

  // Wasn't found, add it to the bin
  RowTimeToIndex itemToInsert(timeValue, times_.size());
  return newIterator_(times_.insert(iter, itemToInsert));
}

void TimeContainerDeque::erase(TimeContainer::Iterator iter, TimeContainer::EraseBehavior eraseBehavior)
{
  SingleBufferIterator* dbIter = dynamic_cast<SingleBufferIterator*>(iter.impl());
  if (dbIter != NULL)
    dbIter->erase(eraseBehavior);
}

simData::DelayedFlushContainerPtr TimeContainerDeque::flush()
{
  // Optimize for case where both are empty (no memory allocation)
  if (times_.empty())
    return DelayedFlushContainerPtr();
  return DelayedFlushContainerPtr(new FlushContainer(times_));
}

// TODO: This was commented out.  It was supposed to be findTimeT(), but that
// role is better filled by lowerBound_() based on findTimeT's current impl.
//TimeContainerDeque::TimeIndexDeque::iterator TimeContainerDeque::findTime_(
//  TimeContainerDeque::TimeIndexDeque& deq, double timeValue) const
//{
//  TimeIndexDeque::iterator i = lowerBound_(deq, timeValue);
//  if (i == deq.end())
//    return i;
//  // NOTE: Should simCore::areEqual() be used here?
//  return ((*i).first == timeValue) ? i : deq.end();
//}

TimeContainerDeque::TimeIndexDeque::iterator TimeContainerDeque::lowerBound_(double timeValue, bool* exactMatch)
{
  LessThan lessThan;
  TimeIndexDeque::iterator i = std::lower_bound(times_.begin(), times_.end(), timeValue, lessThan);
  if (exactMatch != NULL)
    *exactMatch = ((i != times_.end()) && (i->first == timeValue));
  return i;
}

TimeContainerDeque::TimeIndexDeque::iterator TimeContainerDeque::upperBound_(double timeValue)
{
  LessThan lessThan;
  TimeIndexDeque::iterator i = std::upper_bound(times_.begin(), times_.end(), timeValue, lessThan);
  return i;
}

TimeContainer::Iterator TimeContainerDeque::newIterator_(TimeContainerDeque::TimeIndexDeque::iterator iter)
{
  return Iterator(new SingleBufferIterator(times_, iter));
}

TableStatus TimeContainerDeque::deleteRow_(TimeContainer::Iterator timeIter, const std::vector<DataColumn*>& columns,
  DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers)
{
  if (!timeIter.hasNext())
    return TableStatus::Error("Unable to obtain state at time to delete row.");
  const TimeContainer::IteratorData nextData = timeIter.peekNext();
  const size_t idxToDelete = nextData.index();

  // notify listeners before delete
  if (table)
  {
    for (std::vector<DataTable::TableObserverPtr>::const_iterator iter = observers.begin(); iter != observers.end(); ++iter)
      (*iter)->onPreRemoveRow(*table, nextData.time());
  }

  erase(timeIter, TimeContainer::ERASE_FIXOFFSETS);
  for (std::vector<DataColumn*>::const_iterator columnIter = columns.begin(); columnIter != columns.end(); ++columnIter)
  {
    // Assertion failure means we got a NULL table column somehow
    assert(*columnIter != NULL);
    (*columnIter)->erase(nextData.isFreshBin(), idxToDelete);
  }
  return TableStatus::Success();
}

void TimeContainerDeque::limitData(size_t maxPoints, double latestInvalidTime,
  const std::vector<DataColumn*>& columns,
  DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers)
{
  // Apply by-points limiting
  if (maxPoints > 0)
  {
    while (size() > maxPoints)
    {
      deleteRow_(begin(), columns, table, observers);
    }
  }

  // Apply by-time limiting
  while (!empty())
  {
    const double firstTime = begin().peekNext().time();
    if (firstTime > latestInvalidTime)
      break;
    deleteRow_(begin(), columns, table, observers);
  }
}

int TimeContainerDeque::getTimeRange(double& begin, double& end) const
{
  // Not implemented
  assert(0);
  return 1;
}

} }
