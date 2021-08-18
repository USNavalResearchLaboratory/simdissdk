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
#ifndef SIMDATA_MEMORYTABLE_DOUBLEBUFFERTIMECONTAINER_H
#define SIMDATA_MEMORYTABLE_DOUBLEBUFFERTIMECONTAINER_H

#include <deque>
#include <utility>
#include "simCore/Common/Common.h"

// DataTable.h required for the observer on removing rows
#include "simData/DataTable.h"
#include "simData/MemoryTable/TimeContainer.h"

namespace simData { namespace MemoryTable {

/**
 * A time container is responsible for keeping track of positions of inserts and
 * replacements, based on time values.  One concern with time containers (and data
 * containers) is data limiting.  Erase from a container can be an expensive O(n)
 * operation in worst case, as indices need to be updated for all successive entries.
 * The double buffer case optimizes the data limiting in a sense by storing the
 * most recent data points in a (time sorted) deque, then when it's time to data limit,
 * it swaps out all the contents at once.
 *
 * TODO: Implement data limiting
 */
class SDKDATA_EXPORT DoubleBufferTimeContainer : public TimeContainer
{
public:
  /** Default constructor */
  DoubleBufferTimeContainer();
  /** Copy constructor */
  DoubleBufferTimeContainer(const DoubleBufferTimeContainer& copyFrom);
  virtual ~DoubleBufferTimeContainer();

  virtual TimeContainer* clone() const;
  virtual size_t size() const;
  virtual bool empty() const;
  virtual TimeContainer::Iterator begin();
  virtual TimeContainer::Iterator end();
  virtual TimeContainer::Iterator lower_bound(double timeValue);
  virtual TimeContainer::Iterator upper_bound(double timeValue);
  virtual TimeContainer::Iterator findTimeAtOrBeforeGivenTime(double timeValue);
  virtual TimeContainer::Iterator find(double timeValue);
  virtual TimeContainer::Iterator findOrAddTime(double timeValue, bool* exactMatch=nullptr);
  virtual void erase(Iterator iter, EraseBehavior eraseBehavior);
  virtual DelayedFlushContainerPtr flush();
  virtual void flush(const std::vector<DataColumn*>& columns, double startTime, double endTime);

  /// @copydoc TimeContainer::limitData()
  virtual void limitData(size_t maxPoints, double latestInvalidTime, const std::vector<DataColumn*>& columns,
    DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers);

  /**
   * Returns the begin and end time of the column
   * @param begin Returns the begin time
   * @param end Returns the end time
   * @returns 0 if begin and end are set
   */
  virtual int getTimeRange(double& begin, double& end) const;

  /** Swaps the fresh to stale, stale to fresh, and clears out the fresh vector; announces all items removed */
  void swapFreshStaleData(DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers);

private:
  typedef std::pair<double, size_t> RowTimeToIndex; // pTimeIndex
  typedef std::deque<RowTimeToIndex> TimeIndexDeque;

  void flush_(TimeIndexDeque& deq, bool fresh, const std::vector<DataColumn*>& columns, double startTime, double endTime);
  TimeIndexDeque::iterator lowerBound_(TimeIndexDeque& deq, double timeValue, bool* exactMatch=nullptr) const;
  TimeIndexDeque::iterator upperBound_(TimeIndexDeque& deq, double timeValue) const;
  TimeContainer::Iterator newIterator_(size_t whichBin, TimeIndexDeque::iterator staleIter, TimeIndexDeque::iterator freshIter);

  TimeIndexDeque& freshTimes_() const;
  TimeIndexDeque& staleTimes_() const;

  TimeIndexDeque timesA_;
  TimeIndexDeque timesB_;
  TimeIndexDeque* times_[2];
  class DoubleBufferIterator;
  class FlushContainer;
};

} }

#endif /* SIMDATA_MEMORYTABLE_DOUBLEBUFFERTIMECONTAINER_H */
