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
#include <algorithm>
#include <cassert>
#include <limits>
#include <set>
#include "simCore/Calc/Math.h"
#include "simData/MemoryTable/DataColumn.h"
#include "simData/MemoryTable/DoubleBufferTimeContainer.h"

namespace simData { namespace MemoryTable {

namespace
{

static const size_t BIN_STALE = 0;
static const size_t BIN_FRESH = 1;
static const size_t BIN_INVALID = 2; // invalid value for bins

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
class DoubleBufferTimeContainer::FlushContainer : public DelayedFlushContainer
{
public:
  /** Construct a new FlushContainer */
  FlushContainer(TimeIndexDeque& timesA, TimeIndexDeque& timesB)
    : timesA_(nullptr),
      timesB_(nullptr)
  {
    // Optimize the case where vectors are empty
    if (!timesA.empty())
    { // Note that deque::swap() is constant time
      timesA_ = new TimeIndexDeque();
      timesA.swap(*timesA_);
    }
    if (!timesB.empty())
    {
      timesB_ = new TimeIndexDeque();
      timesB.swap(*timesB_);
    }
  }
  virtual ~FlushContainer()
  {
    delete timesA_;
    timesA_ = nullptr;
    delete timesB_;
    timesB_ = nullptr;
  }
private:
  TimeIndexDeque* timesA_;
  TimeIndexDeque* timesB_;
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Implementation of the TimeContainer's iterator impl interface.  Allows a
 * user to step through the double buffered time container in time-sorted order.
 */
class DoubleBufferTimeContainer::DoubleBufferIterator : public TimeContainer::IteratorImpl
{
public:
  /** Sentinel value for invalid values */
  static const IteratorData INVALID_VALUE;

  /** Default constructor */
  DoubleBufferIterator()
    : binIndex_(BIN_INVALID)
  {
    ownerBins_[BIN_STALE] = ownerBins_[BIN_FRESH] = nullptr;
    // Order here doesn't really matter since the bins are invalid
    iter_[BIN_STALE] = &iterA_;
    iter_[BIN_FRESH] = &iterB_;
  }

  /** Copy constructor */
  DoubleBufferIterator(const DoubleBufferIterator& copyConstructor)
    : binIndex_(copyConstructor.binIndex_),
      iterA_(copyConstructor.iterA_),
      iterB_(copyConstructor.iterB_)
  {
    ownerBins_[BIN_STALE] = copyConstructor.ownerBins_[BIN_STALE];
    ownerBins_[BIN_FRESH] = copyConstructor.ownerBins_[BIN_FRESH];

    // If copy's stale is A, then our stale should be A
    if (copyConstructor.iter_[BIN_STALE] == &copyConstructor.iterA_)
    {
      iter_[BIN_STALE] = &iterA_;
      iter_[BIN_FRESH] = &iterB_;
    }
    else
    {
      iter_[BIN_FRESH] = &iterA_;
      iter_[BIN_STALE] = &iterB_;
    }
  }

  /** Construct a new DoubleBufferIterator with time index deques */
  DoubleBufferIterator(DoubleBufferTimeContainer::TimeIndexDeque* deques[2], size_t whichBin,
    const DoubleBufferTimeContainer::TimeIndexDeque::iterator& iterA,
    const DoubleBufferTimeContainer::TimeIndexDeque::iterator& iterB,
    bool useStale)
    : binIndex_(whichBin),
      iterA_(iterA),
      iterB_(iterB)
  {
    ownerBins_[BIN_STALE] = deques[BIN_STALE];
    ownerBins_[BIN_FRESH] = deques[BIN_FRESH];
    iter_[BIN_STALE] = &iterA_;
    iter_[BIN_FRESH] = &iterB_;
  }

  virtual ~DoubleBufferIterator()
  {
  }

  /** Retrieves next element and increments iterator to position after that element
  * @return Element after original iterator position,
  * or INVALID_VALUE if no such element
  */
  virtual const IteratorData next()
  {
    if (!isValid_())
      return INVALID_VALUE;
    const TimeIndexDeque& nextDeq = *ownerBins_[binIndex_];
    TimeIndexDeque::iterator& nextIter = *iter_[binIndex_];
    size_t otherBin = otherBin_();
    if (nextIter == nextDeq.end())
    {
      // Either we're at the end, or there's a precondition violation (sync)
      assert(peekNext() == INVALID_VALUE);
      assert((*iter_[otherBin]) == ownerBins_[otherBin]->end());
      return INVALID_VALUE;
    }
    IteratorData returnValue(*nextIter, binIndex_ == BIN_FRESH);

    // Move the iterator, it definitely needs to change.
    ++nextIter;
    // Adjust the next binIndex_
    if (nextIter == nextDeq.end())
      binIndex_ = otherBin; // Must be the other bin (even if it's empty)
    else
    {
      const TimeIndexDeque& otherDeq = *ownerBins_[otherBin];
      TimeIndexDeque::iterator& otherIter = *iter_[otherBin];
      // Only need to compare if the other deque is not at end, and greater than ours
      if ((otherIter != otherDeq.end()) && (nextIter->first > otherIter->first))
      {
        binIndex_ = otherBin;
      }
    }
    return returnValue;
  }
  /** Retrieves next element and does not change iterator position */
  virtual const IteratorData peekNext() const
  {
    if (!isValid_())
      return INVALID_VALUE;
    // Primary bin is reflected by binIndex_.  If it doesn't have a next(), then there
    // is a failure somewhere.  So we only need to inspect its value
    if (*iter_[binIndex_] == ownerBins_[binIndex_]->end())
    {
      // Assertion failure means the precondition (iterators in sync, binIndex_
      //   is the primary) is not satisfied
      assert(*iter_[otherBin_()] == ownerBins_[otherBin_()]->end());
      return INVALID_VALUE;
    }
    return IteratorData(*(*iter_[binIndex_]), binIndex_ == BIN_FRESH);
  }

  /** Retrieves previous element and decrements iterator to position before that element
  * @return Element before original iterator position,
  * or INVALID_VALUE if no such element
  */
  virtual const IteratorData previous()
  {
    if (!isValid_())
      return INVALID_VALUE;
    // Don't make copies (unlike peekPrevious)
    return previous_(freshIter_(), staleIter_(), &binIndex_);
  }

  /** Retrieves previous element and does not change iterator position */
  virtual const IteratorData peekPrevious() const
  {
    if (!isValid_())
      return INVALID_VALUE;
    // Make a copy of iterators to prevent change
    TimeIndexDeque::iterator freshIter = freshIter_();
    TimeIndexDeque::iterator staleIter = staleIter_();
    return previous_(freshIter, staleIter, nullptr);
  }

  /** Resets the iterator to the front of the data structure, before the first element */
  virtual void toFront()
  {
    if (!isValid_())
      return;
    freshIter_() = freshTimes_()->begin();
    staleIter_() = staleTimes_()->begin();
  }
  /** Sets the iterator to the end of the data structure, after the last element */
  virtual void toBack()
  {
    if (!isValid_())
      return;
    freshIter_() = freshTimes_()->end();
    staleIter_() = staleTimes_()->end();
  }

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  virtual bool hasNext() const
  {
    if (!isValid_())
      return false;
    // Are we at the end() of primary and secondary?
    return freshIter_() != freshTimes_()->end() ||
      staleIter_() != staleTimes_()->end();
  }
  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  virtual bool hasPrevious() const
  {
    if (!isValid_())
      return false;
    return freshIter_() != freshTimes_()->begin() ||
      staleIter_() != staleTimes_()->begin();
  }

  /** Create a copy of the actual implementation */
  virtual GenericIteratorImpl<IteratorData>* clone() const
  {
    return new DoubleBufferIterator(*this);
  }

  /** Removes the iterator from the parent container */
  void erase(TimeContainer::EraseBehavior eraseBehavior)
  {
    if (!isValid_())
      return;
    if (*iter_[binIndex_] != ownerBins_[binIndex_]->end())
    {
      // Decrement the contents of the bin
      if (eraseBehavior == TimeContainer::ERASE_FIXOFFSETS)
        decreaseAllIndices_(*ownerBins_[binIndex_], (*iter_[binIndex_])->second);

      ownerBins_[binIndex_]->erase(*iter_[binIndex_]);
      // Reset the iterators to keep the state consistent.
      staleIter_() = staleTimes_()->end();
      freshIter_() = freshTimes_()->end();
    }
  }

private:
  size_t binIndex_;
  /// Points to the bins in the owning container; soft copy, not owned
  DoubleBufferTimeContainer::TimeIndexDeque* ownerBins_[2];
  DoubleBufferTimeContainer::TimeIndexDeque::iterator iterA_;
  DoubleBufferTimeContainer::TimeIndexDeque::iterator iterB_;
  DoubleBufferTimeContainer::TimeIndexDeque::iterator* iter_[2];

  /// Helper function that adjusts indices in case of erase()
  void decreaseAllIndices_(DoubleBufferTimeContainer::TimeIndexDeque& deq, size_t greaterThan) const
  {
    for (DoubleBufferTimeContainer::TimeIndexDeque::iterator i = deq.begin(); i != deq.end(); ++i)
    {
      if (i->second > greaterThan)
        --(i->second);
    }
  }

  bool isValid_() const
  {
    return staleTimes_() != nullptr && freshTimes_() != nullptr &&
      (binIndex_ == BIN_STALE || binIndex_ == BIN_FRESH);
  }
  size_t otherBin_() const
  {
    // Note that unconditional else() is not possible due to BIN_INVALID
    if (binIndex_ == BIN_STALE)
      return BIN_FRESH;
    else if (binIndex_ == BIN_FRESH)
      return BIN_STALE;
    return BIN_INVALID;
  }
  void swapBins_()
  {
    binIndex_ = otherBin_();
  }

  // Generic previous function will modify the incoming iterators
  IteratorData previous_(TimeIndexDeque::iterator& freshIter, TimeIndexDeque::iterator& staleIter, size_t* nextBin) const
  {
    if (!isValid_())
      return INVALID_VALUE;
    // The precondition is that binIndex_ points to the next item, but doesn't mean the
    // previous item is in binIndex_.  We must check both queues, unlike next()

    // Make a copy of iterators to avoid modification
    const TimeIndexDeque& freshDeq = *freshTimes_();
    const TimeIndexDeque& staleDeq = *staleTimes_();
    // Early out for begin()
    if (freshIter == freshDeq.begin())
    {
      if (staleIter == staleDeq.begin()) // cannot decrement anything
        return INVALID_VALUE;
      if (nextBin) *nextBin = BIN_STALE; // the "next" is the one that got decremented
      --staleIter;
      return IteratorData(*staleIter, false);
    }
    // Can definitely decrement the fresh side
    --freshIter;
    if (staleIter == staleDeq.begin()) // can't decrement stale
    {
      if (nextBin) *nextBin = BIN_FRESH; // the "next" is the one that got decremented
      return IteratorData(*freshIter, true);
    }
    --staleIter;
    // Return the fresher of the two values
    if (staleIter->first > freshIter->first)
    {
      // Reset freshIter before returning
      ++freshIter;
      if (nextBin) *nextBin = BIN_STALE; // the "next" is the one that got decremented
      return IteratorData(*staleIter, false);
    }
    // Reset staleIter before returning
    ++staleIter;
    if (nextBin) *nextBin = BIN_FRESH; // the "next" is the one that got decremented
    return IteratorData(*freshIter, true);
  }

  /// Retrieve pointer to the owner's stale container
  DoubleBufferTimeContainer::TimeIndexDeque* staleTimes_() const
  {
    return ownerBins_[BIN_STALE];
  }

  /// Retrieve pointer to the owner's fresh container
  DoubleBufferTimeContainer::TimeIndexDeque* freshTimes_() const
  {
    return ownerBins_[BIN_FRESH];
  }

  /// Iterator into the owner's stale container
  DoubleBufferTimeContainer::TimeIndexDeque::iterator& staleIter_() const
  {
    return *iter_[BIN_STALE];
  }

  /// Iterator into the owner's fresh container
  DoubleBufferTimeContainer::TimeIndexDeque::iterator& freshIter_() const
  {
    return *iter_[BIN_FRESH];
  }

};
const TimeContainer::IteratorData DoubleBufferTimeContainer::DoubleBufferIterator::INVALID_VALUE(std::numeric_limits<double>::max(), 0, true);

/////////////////////////////////////////////////////////////////////////////

DoubleBufferTimeContainer::DoubleBufferTimeContainer()
{
  times_[BIN_STALE] = &timesA_;
  times_[BIN_FRESH] = &timesB_;
}

DoubleBufferTimeContainer::DoubleBufferTimeContainer(const DoubleBufferTimeContainer& copyFrom)
  : timesA_(copyFrom.timesA_),
    timesB_(copyFrom.timesB_)
{
  // If copy's stale is A, then our stale should be A
  if (copyFrom.times_[BIN_STALE] == &copyFrom.timesA_)
  {
    times_[BIN_STALE] = &timesA_;
    times_[BIN_FRESH] = &timesB_;
  }
  else
  {
    times_[BIN_FRESH] = &timesA_;
    times_[BIN_STALE] = &timesB_;
  }
}

DoubleBufferTimeContainer::~DoubleBufferTimeContainer()
{
}

TimeContainer* DoubleBufferTimeContainer::clone() const
{
  return new DoubleBufferTimeContainer(*this);
}

size_t DoubleBufferTimeContainer::size() const
{
  return timesA_.size() + timesB_.size();
}

bool DoubleBufferTimeContainer::empty() const
{
  return timesA_.empty() && timesB_.empty();
}

TimeContainer::Iterator DoubleBufferTimeContainer::begin()
{
  if (freshTimes_().empty())
  {
    if (staleTimes_().empty())
      return end();
    else
      return newIterator_(BIN_STALE, staleTimes_().begin(), freshTimes_().begin());
  }
  else if (staleTimes_().empty())
  {
    return newIterator_(BIN_FRESH, staleTimes_().begin(), freshTimes_().begin());
  }

  // Neither is empty, compare times
  if (freshTimes_().begin()->first < staleTimes_().begin()->first)
    return newIterator_(BIN_FRESH, staleTimes_().begin(), freshTimes_().begin());
  return newIterator_(BIN_STALE, staleTimes_().begin(), freshTimes_().begin());
}

TimeContainer::Iterator DoubleBufferTimeContainer::end()
{
  return newIterator_(BIN_FRESH, staleTimes_().end(), freshTimes_().end());
}

TimeContainer::Iterator DoubleBufferTimeContainer::lower_bound(double timeValue)
{
  TimeIndexDeque& staleDeq = staleTimes_();
  TimeIndexDeque& freshDeq = freshTimes_();
  TimeIndexDeque::iterator staleIter = lowerBound_(staleDeq, timeValue);
  TimeIndexDeque::iterator freshIter = lowerBound_(freshDeq, timeValue);
  if (staleIter == staleDeq.end())
    return newIterator_(BIN_FRESH, staleIter, freshIter);
  else if (freshIter == freshDeq.end())
    return newIterator_(BIN_STALE, staleIter, freshIter);
  // Return the lower-valued iterator
  return newIterator_(staleIter->first < freshIter->first ? BIN_STALE : BIN_FRESH, staleIter, freshIter);
}

TimeContainer::Iterator DoubleBufferTimeContainer::upper_bound(double timeValue)
{
  TimeIndexDeque& staleDeq = staleTimes_();
  TimeIndexDeque& freshDeq = freshTimes_();
  TimeIndexDeque::iterator staleIter = upperBound_(staleDeq, timeValue);
  TimeIndexDeque::iterator freshIter = upperBound_(freshDeq, timeValue);
  if (staleIter == staleDeq.end())
    return newIterator_(BIN_FRESH, staleIter, freshIter);
  else if (freshIter == freshDeq.end())
    return newIterator_(BIN_STALE, staleIter, freshIter);
  // Return the lower-valued iterator
  return newIterator_(staleIter->first < freshIter->first ? BIN_STALE : BIN_FRESH, staleIter, freshIter);
}

TimeContainer::Iterator DoubleBufferTimeContainer::findTimeAtOrBeforeGivenTime(double timeValue)
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

TimeContainer::Iterator DoubleBufferTimeContainer::find(double timeValue)
{
  bool exactMatchStale = false;
  TimeIndexDeque::iterator iterStale = lowerBound_(staleTimes_(), timeValue, &exactMatchStale);
  bool exactMatchFresh = false;
  TimeIndexDeque::iterator iterFresh = lowerBound_(freshTimes_(), timeValue, &exactMatchFresh);
  if (exactMatchFresh)
    return newIterator_(BIN_FRESH, iterStale, iterFresh);
  else if (exactMatchStale)
    return newIterator_(BIN_STALE, iterStale, iterFresh);
  return end();
}

TimeContainer::Iterator DoubleBufferTimeContainer::findOrAddTime(double timeValue, bool* exactMatch)
{
  // Presume exact match; set false below if needed
  if (exactMatch)
    *exactMatch = true;

  // Look for the value in the stale container
  TimeIndexDeque& freshDeq = freshTimes_();
  TimeIndexDeque& staleDeq = staleTimes_();
  bool tmpExactMatch = false;

  TimeIndexDeque::iterator iterStale = staleDeq.end();
  // Optimize away the case where time-sorted deque has all times less than requested time
  if (!staleDeq.empty() && (staleDeq.back().first >= timeValue))
  {
    iterStale = lowerBound_(staleDeq, timeValue, &tmpExactMatch);
    if (tmpExactMatch)
    { // Time was found in stale container
      // Update contents of the second bin iterator
      TimeIndexDeque::iterator iterFresh = lowerBound_(freshDeq, timeValue);
      return newIterator_(BIN_STALE, iterStale, iterFresh);
    }
  }
  // Look in the fresh bin
  TimeIndexDeque::iterator iterFresh = freshDeq.end();
  // Optimize away the case where time-sorted deque has all times less than requested time
  if (!freshDeq.empty() && (freshDeq.back().first >= timeValue))
  {
    iterFresh = lowerBound_(freshDeq, timeValue, &tmpExactMatch);
    if (tmpExactMatch)
    {
      // It was found, return the iterator
      return newIterator_(BIN_FRESH, iterStale, iterFresh);
    }
  }

  // Not an exact match; reverses earlier operator=
  if (exactMatch)
    *exactMatch = false;

  // Wasn't found, add it to the fresh bin
  RowTimeToIndex itemToInsert(timeValue, freshDeq.size());

  // NOTE: The following section of code could be uncommented to perform worst-case
  // performance testing, and to test the validity of iterator crossing containers
  //if (size() % 2 == 0)
  //  return newIterator_(BIN_STALE, staleDeq.insert(iterStale, itemToInsert), iterFresh);
  return newIterator_(BIN_FRESH, iterStale, freshDeq.insert(iterFresh, itemToInsert));
}

void DoubleBufferTimeContainer::erase(TimeContainer::Iterator iter, TimeContainer::EraseBehavior eraseBehavior)
{
  DoubleBufferIterator* dbIter = dynamic_cast<DoubleBufferIterator*>(iter.impl());
  if (dbIter != nullptr)
    dbIter->erase(eraseBehavior);
}

simData::DelayedFlushContainerPtr DoubleBufferTimeContainer::flush()
{
  // Optimize for case where both are empty (no memory allocation)
  if (timesA_.empty() && timesB_.empty())
    return DelayedFlushContainerPtr();
  return DelayedFlushContainerPtr(new FlushContainer(timesA_, timesB_));
}

void DoubleBufferTimeContainer::flush(const std::vector<DataColumn*>& columns, double startTime, double endTime)
{
  flush_(*times_[BIN_STALE], false, columns, startTime, endTime);
  flush_(*times_[BIN_FRESH], true, columns, startTime, endTime);
}

void DoubleBufferTimeContainer::flush_(TimeIndexDeque& deq, bool fresh, const std::vector<DataColumn*>& columns, double startTime, double endTime)
{
  LessThan lessThan;
  const auto start = std::lower_bound(deq.begin(), deq.end(), startTime, lessThan);
  const auto end = std::lower_bound(deq.begin(), deq.end(), endTime, lessThan);

  // Nothing to do
  if (start == end)
    return;

  const size_t itemsToDelete = std::distance(start, end);

  std::vector<size_t> indexToRemove;
  indexToRemove.reserve(itemsToDelete);
  for (auto it = start; it != end; ++it)
    indexToRemove.push_back(it->second);
  std::sort(indexToRemove.begin(), indexToRemove.end());

  // Group continuous ranges of indexes to greatly improve performance
  std::set< std::pair<size_t, size_t> > indexes;

  auto startIndex = indexToRemove.front();
  auto endIndex = indexToRemove.back();
  if (itemsToDelete == (endIndex - startIndex + 1))
  {
    // data is in time order so just need one entry
    indexes.insert(std::make_pair(startIndex, endIndex));
  }
  else
  {
    // Find continuous ranges of indexes
    startIndex = indexToRemove.front();
    endIndex = startIndex;
    for (auto it = indexToRemove.begin() + 1; it != indexToRemove.end(); ++it)
    {
      if ((startIndex + 1) != *it)
      {
        indexes.insert(std::make_pair(startIndex, endIndex));
        startIndex = *it;
      }
      endIndex = *it;
    }

    // The last range needs to be committed
    indexes.insert(std::make_pair(startIndex, endIndex));
  }

  // Time is always in order so erase everything in one call
  deq.erase(start, end);

  // Go in reverse order so pairs in "indexes" do not need to be adjusted on removal
  for (auto it = indexes.rbegin(); it != indexes.rend(); ++it)
  {
    auto indexDelta = it->second - it->first + 1;

    // Adjust time index values in the deq
    for (auto& timeIndex : deq)
    {
      //Dev error, algorithm above is not correct
      assert((timeIndex.second < it->first) || (timeIndex.second > it->second));
      if (timeIndex.second > it->second)
        timeIndex.second -= indexDelta;
    }

    // and update the columns
    for (auto& column : columns)
      column->erase(fresh, it->first, indexDelta);
  }
}

// TODO: This was commented out.  It was supposed to be findTimeT(), but that
// role is better filled by lowerBound_() based on findTimeT's current impl.
//DoubleBufferTimeContainer::TimeIndexDeque::iterator DoubleBufferTimeContainer::findTime_(
//  DoubleBufferTimeContainer::TimeIndexDeque& deq, double timeValue) const
//{
//  TimeIndexDeque::iterator i = lowerBound_(deq, timeValue);
//  if (i == deq.end())
//    return i;
//  // NOTE: Should simCore::areEqual() be used here?
//  return ((*i).first == timeValue) ? i : deq.end();
//}

DoubleBufferTimeContainer::TimeIndexDeque::iterator DoubleBufferTimeContainer::lowerBound_(
  DoubleBufferTimeContainer::TimeIndexDeque& deq, double timeValue, bool* exactMatch) const
{
  LessThan lessThan;
  TimeIndexDeque::iterator i = std::lower_bound(deq.begin(), deq.end(), timeValue, lessThan);
  if (exactMatch != nullptr)
    *exactMatch = ((i != deq.end()) && (i->first == timeValue));
  return i;
}

DoubleBufferTimeContainer::TimeIndexDeque::iterator DoubleBufferTimeContainer::upperBound_(
  DoubleBufferTimeContainer::TimeIndexDeque& deq, double timeValue) const
{
  LessThan lessThan;
  TimeIndexDeque::iterator i = std::upper_bound(deq.begin(), deq.end(), timeValue, lessThan);
  return i;
}

TimeContainer::Iterator DoubleBufferTimeContainer::newIterator_(size_t whichBin,
                                                                DoubleBufferTimeContainer::TimeIndexDeque::iterator staleIter,
                                                                DoubleBufferTimeContainer::TimeIndexDeque::iterator freshIter)
{
  return Iterator(new DoubleBufferIterator(times_, whichBin, staleIter, freshIter, times_[BIN_STALE] == &timesA_));
}

DoubleBufferTimeContainer::TimeIndexDeque& DoubleBufferTimeContainer::freshTimes_() const
{
  return *times_[BIN_FRESH];
}

DoubleBufferTimeContainer::TimeIndexDeque& DoubleBufferTimeContainer::staleTimes_() const
{
  return *times_[BIN_STALE];
}

void DoubleBufferTimeContainer::swapFreshStaleData(DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers)
{
  // Before clearing out the stale bin, announce that all those times are being removed
  if (table != nullptr && !times_[BIN_STALE]->empty())
  {
    // Loop through each observer
    for (std::vector<DataTable::TableObserverPtr>::const_iterator oiter = observers.begin(); oiter != observers.end(); ++oiter)
    {
      // Loop through each item in the deque
      for (TimeIndexDeque::const_iterator timeIter = times_[BIN_STALE]->begin();
        timeIter != times_[BIN_STALE]->end(); ++timeIter)
      {
        // TODO: Could optimize this by setting up a vector of times and sending all at once
        (*oiter)->onPreRemoveRow(*table, (*timeIter).first);
      }
    }
  }

  TimeIndexDeque* tmp = times_[BIN_STALE];
  times_[BIN_STALE] = times_[BIN_FRESH];
  times_[BIN_FRESH] = tmp;
  times_[BIN_FRESH]->clear();
}

void DoubleBufferTimeContainer::limitData(size_t maxPoints, double latestInvalidTime,
  const std::vector<DataColumn*>& columns,
  DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers)
{
  // Break out early to avoid a potential invalid dereference of begin()
  if (empty())
    return;

  // Avoid storing (n*2) items for data limit, by limiting to half the requested size (since
  // stale buffer still exists and will have older items).
  if (maxPoints > 0)
  {
    // Add 1 to provide a little extra safety, and so that "1" limit doesn't go to "0" limit
    if (maxPoints != std::numeric_limits<size_t>::max())
      ++maxPoints;
    maxPoints /= 2;
  }

  // Break out early if we don't need to limit by points (0 == no limiting)
  if (maxPoints == 0 || times_[BIN_FRESH]->size() < maxPoints)
  {
    // Determine if the fresh buffer exceeds the time limits
    if (times_[BIN_FRESH]->empty()) // Need to have some times in the fresh bin to swap
      return;
    // Check the fresh bin times only; ignore stale bin for swapping
    const double firstTime = times_[BIN_FRESH]->front().first;
    // Break out early if we don't need to seconds limit either
    if (latestInvalidTime <= 0.0 || firstTime >= latestInvalidTime)
      return;
  }

  // Note that naive implementation here does not retain item -1

  // We definitely need to limit.  Do a swap of buffers to handle it
  swapFreshStaleData(table, observers);
  for (std::vector<DataColumn*>::const_iterator i = columns.begin(); i != columns.end(); ++i)
    (*i)->swapFreshStaleData();
}

int DoubleBufferTimeContainer::getTimeRange(double& begin, double& end) const
{
  const auto* fresh = times_[BIN_FRESH];
  const auto* stale = times_[BIN_STALE];

  if (fresh->empty())
  {
    if (stale->empty())
    {
      begin = 0.0;
      end = 0.0;
      return 1;
    }
    begin = stale->front().first;
    end = stale->back().first;
    return 0;
  }

  if (stale->empty())
  {
    begin = fresh->front().first;
    end = fresh->back().first;
    return 0;
  }

  begin = simCore::sdkMin(fresh->front().first, stale->front().first);
  end = simCore::sdkMax(fresh->back().first, stale->back().first);
  return 0;
}

} }
