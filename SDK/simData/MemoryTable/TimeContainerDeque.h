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
#ifndef SIMDATA_MEMORYTABLE_TIMECONTAINERDEQUE_H
#define SIMDATA_MEMORYTABLE_TIMECONTAINERDEQUE_H
#ifdef USE_DEPRECATED_SIMDISSDK_API

#include <deque>
#include <utility>
#include "simCore/Common/Common.h"
#include "simData/MemoryTable/TimeContainer.h"

namespace simData { namespace MemoryTable {

/**
 * A time container is responsible for keeping track of positions of inserts and
 * replacements, based on time values.  One concern with time containers (and data
 * containers) is data limiting.  Erase from a container can be an expensive O(n)
 * operation in worst case, as indices need to be updated for all successive entries.
 *
 * TODO: Implement data limiting
 */
class SDKDATA_EXPORT TimeContainerDeque : public TimeContainer
{
public:
  /** Default constructor */
  TimeContainerDeque();
  /** Copy constructor */
  TimeContainerDeque(const TimeContainerDeque& copyFrom);
  virtual ~TimeContainerDeque();

  virtual TimeContainer* clone() const;
  virtual size_t size() const;
  virtual bool empty() const;
  virtual TimeContainer::Iterator begin();
  virtual TimeContainer::Iterator end();
  virtual TimeContainer::Iterator lower_bound(double timeValue);
  virtual TimeContainer::Iterator upper_bound(double timeValue);
  virtual TimeContainer::Iterator findTimeAtOrBeforeGivenTime(double timeValue);
  virtual TimeContainer::Iterator find(double timeValue);
  virtual TimeContainer::Iterator findOrAddTime(double timeValue, bool* exactMatch=NULL);
  virtual void erase(Iterator iter, EraseBehavior eraseBehavior);
  virtual DelayedFlushContainerPtr flush();

  /// @copydoc TimeContainer::limitData()
  virtual void limitData(size_t maxPoints, double latestInvalidTime, const std::vector<DataColumn*>& columns,
    DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers);

  /**
   * Not Implemented
   * @param begin Returns the begin time
   * @param end Returns the end time
   * @returns 0 if begin and end are set
   */
  virtual int getTimeRange(double& begin, double& end) const;

private:
  typedef std::pair<double, size_t> RowTimeToIndex; // pTimeIndex
  typedef std::deque<RowTimeToIndex> TimeIndexDeque;

  TimeIndexDeque::iterator lowerBound_(double timeValue, bool* exactMatch=NULL);
  TimeIndexDeque::iterator upperBound_(double timeValue);
  TimeContainer::Iterator newIterator_(TimeIndexDeque::iterator iter);
  TableStatus deleteRow_(TimeContainer::Iterator timeIter, const std::vector<DataColumn*>& columns,
    DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers);

  TimeIndexDeque times_;
  class SingleBufferIterator;
  class FlushContainer;
};

} }

#endif /* USE_DEPRECATED_SIMDISSDK_API */
#endif /* SIMDATA_MEMORYTABLE_TIMECONTAINERDEQUE_H */
