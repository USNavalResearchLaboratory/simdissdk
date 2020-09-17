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
#ifndef SIMDATA_MEMORYTABLE_TIMECONTAINER_H
#define SIMDATA_MEMORYTABLE_TIMECONTAINER_H

#include <utility>
#include "simCore/Common/Common.h"
#include "simData/GenericIterator.h"
#include "simData/DataTable.h"

namespace simData { namespace MemoryTable {

class DataColumn;

/**
 * Generic pure-virtual interface that describes a container of time data.  Time
 * containers are held by subtables.  Because subtables are null-less, each column
 * in a subtable can/should/will use the same time container.  Rather than each
 * column having its own time container, it is managed by the SubTable class, who
 * then delegates the "position" assignments from the time container to the data
 * containers.
 *
 * TimeContainer is an interface that is only used by In-Memory data tables.
 */
class SDKDATA_EXPORT TimeContainer
{
public:

  /**
   * The time container associates times to indices.  The IteratorData is the
   * type used for the GenericIterator template, allowing those using iteration
   * on this container to retrieve the appropriate index for the time value.
   */
  class IteratorData
  {
  public:
    /// Instantiates with a specific time and index.
    IteratorData(double t, size_t idx, bool isFreshBin)
      : time_(t),
        index_(idx),
        isFreshBin_(isFreshBin)
    {
    }
    /// Copies a time and index
    IteratorData(const IteratorData& copy)
      : time_(copy.time_),
        index_(copy.index_),
        isFreshBin_(copy.isFreshBin_)
    {
    }
    /// Converts a pair of time/size to an IteratorData
    explicit IteratorData(const std::pair<double, size_t>& p, bool isFreshBin)
      : time_(p.first),
        index_(p.second),
        isFreshBin_(isFreshBin)
    {
    }
    /// Assignment operator
    IteratorData& operator=(const IteratorData& copy)
    {
      if (this != &copy)
      {
        time_ = copy.time_;
        index_ = copy.index_;
        isFreshBin_ = copy.isFreshBin_;
      }
      return *this;
    }
    /// Automatic conversion to a double value (for time)
    operator double() const
    {
      return time();
    }
    /// Time value associated with this value
    double time() const
    {
      return time_;
    }
    /// Index value associated with this value
    size_t index() const
    {
      return index_;
    }
    /// True if the referring to the 'fresh' bin, or false for 'stale' bin
    bool isFreshBin() const
    {
      return isFreshBin_;
    }

  private:
    double time_;
    size_t index_;
    bool isFreshBin_;
  };

  /// TimeContainer::IteratorImpl is defined by TimeContainer implementations
  typedef GenericIteratorImpl<IteratorData> IteratorImpl;
  /// TimeContainer::Iterator lets users iterate over time/index values in-order
  typedef GenericIterator<IteratorData> Iterator;

  virtual ~TimeContainer() {}

  /** Copies the time container and its internal data. */
  virtual TimeContainer* clone() const = 0;
  /** Returns number of time entries in the container */
  virtual size_t size() const = 0;
  /** Returns true only if there are no time entries in the container. */
  virtual bool empty() const = 0;
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
   * next() will be invalid (hasNext() is false)
   */
  virtual Iterator findTimeAtOrBeforeGivenTime(double timeValue) = 0;
  /**
   * Retrieves the iterator with the given time value.  If not found, hasNext()
   * will be false.
   */
  virtual Iterator find(double timeValue) = 0;
  /**
   * Either returns the iterator representing the time value, or adds the time
   * value and returns an iterator such that its next() is the new time value.
   * @param timeValue Time to find, or add if it didn't exist
   * @param exactMatch If non-nullptr, will be set to false if added row, or true if found row
   */
  virtual Iterator findOrAddTime(double timeValue, bool* exactMatch=nullptr) = 0;

  /**
   * Performs data limiting for the container and associated columns
   * @param maxPoints Maximum number of points; 0 for no limiting
   * @param latestInvalidTime Remove data before this time; <0 for no limiting
   * @param columns Associated data columns that also need updating to keep in sync
   * @param table Parameter to pass to observers for pre-remove of time value
   * @param observers List of observers to notify on pre-remove of time value
   */
  virtual void limitData(size_t maxPoints, double latestInvalidTime, const std::vector<DataColumn*>& columns,
    DataTable* table, const std::vector<DataTable::TableObserverPtr>& observers) = 0;

  /** Different usage of erase() function */
  enum EraseBehavior
  {
    /** Possibly O(n) behavior, adjusts all offsets for a removed entry. */
    ERASE_FIXOFFSETS = 0,
    /** O(lg n) behavior, but will not adjust offsets; dangerous if used incorrectly. */
    ERASE_QUICK
  };
  /** Removes an element from the container (iter's next() value). */
  virtual void erase(Iterator iter, EraseBehavior eraseBehavior) = 0;
  /** Removes all entries from the container. */
  virtual DelayedFlushContainerPtr flush() = 0;

  /**
   * Returns the begin and end time
   * @param begin Returns the begin time
   * @param end Returns the end time
   * @returns 0 if begin and end are set
   */
  virtual int getTimeRange(double& begin, double& end) const = 0;
};

} }

#endif  /* SIMDATA_MEMORYTABLE_TIMECONTAINER_H */
