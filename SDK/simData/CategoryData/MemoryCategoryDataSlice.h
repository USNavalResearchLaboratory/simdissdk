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
#ifndef SIMDATA_MEMORY_CATEGORY_DATASLICE_H
#define SIMDATA_MEMORY_CATEGORY_DATASLICE_H

#include <algorithm>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#ifdef HAVE_ENTT
#include "entt/container/dense_map.hpp"
#endif
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simData/CategoryData/CategoryData.h"

namespace simData {

class CategoryNameManager; // manages string to int translation

/** access to all the category data for an entity at a given time
 *
 * Any entity has a number of category data values at any time.  These values
 * might have been set recently, or might have been set a long time ago.
 *
 * Acts like an iterator, but also provides total dumps
 */
class SDKDATA_EXPORT MemoryCategoryDataSlice : public CategoryDataSlice
{
public:
  ///@param[in] timeStamp time associated with the given data
  MemoryCategoryDataSlice(double timeStamp = 0.0);

  /// insert data into the slice
  ///@note: normal DataSlice behavior is to take ownership of the data
  /// since that is not how we store data, we will delete the pointer
  void insert(CategoryData *data);

  /// Retrieves the total number of items in the slice
  size_t numItems() const;

  /// Returns true if the key/value provided would be a duplicate/repeated value at the time given
  bool isDuplicateValue(double time, const std::string& catName, const std::string& value) const;

  //--- from CategoryDataSlice
  /// remove one specific point from the category data (invalidates any iterators)
  ///@return true if point was found and removed
  virtual bool removePoint(double time, int catNameInt, int valueInt);

  ///@return last update time
  virtual double lastUpdateTime() const;

  ///@return true if the category data changes
  virtual bool update(double time);

  /**
   * Returns true if the category data changes.  Also returns the time span of the category state for the given time
   * @param time Scenario time for the category data
   * @param startTime The start time of the time range that has the same category state as time
   * @param endTime The end time of the time range that has the same category state as time
   * @return true if the category data changes
  */
  bool update(double time, std::optional<double>& startTime, std::optional<double>& endTime);

  /// A function that is called every time the list is modified
  void installNotifier(const std::function<void()>& fn);

  /// apply the data limits indicated by 'prefs'
  virtual void limitByPrefs(const CommonPrefs &prefs);

  /// receive all the category data for the last update time
  virtual void visit(Visitor *visitor) const;

  ///@return an iterator for the current data
  virtual Iterator current() const;

  /// retrieve all
  ///@{
  virtual void allNames(std::vector<std::string> &nameVec) const;
  virtual void allValues(std::vector<std::string> &valueVec) const;
  virtual void allStrings(std::vector<std::pair<std::string, std::string> > &nameValueVec) const;

  virtual void allNameInts(std::vector<int> &nameIntVec) const;
  virtual void allValueInts(std::vector<int> &valueIntVec) const;
  virtual void allInts(std::vector<std::pair<int, int> > &nameValueIntVec) const;
  virtual void allInts(std::map<int, int> &nameValueIntMap) const;

  ///@}

  /// remove all data in the slice, retaining current category data and the static point
  void flush();

  /// remove all data in the slice
  void completeFlush();

  /// remove points in the given time range; up to but not including endTime
  void flush(double startTime, double endTime);

  /// pass in the category name manager reference
  void setCategoryNameManager(CategoryNameManager* categoryNameManager);

private: // types
  class Iterator;
  class MemoryCategoryDataPair;

  // Helper struct for holding the time value pair
  struct TimeValuePair
  {
    TimeValuePair(double inTime, int inValue)
      : time(inTime),
        value(inValue)
    {
    }

    /// Less-than operator for time; used with the STL calls
    bool operator<(const TimeValuePair& rhs) const
    {
      return time < rhs.time;
    }

    double time;
    int value;
  };

public:
  typedef std::deque<TimeValuePair> TimeValuesEntries; ///< multiple time value pairs
  typedef std::deque<TimeValuePair>::iterator TimeValueIterator; ///< iterator for above

private:
  // A wrapper around a vector, providing optimized access for common-case conditions in Memory Data Store.
  class TimeValues
  {
  public:
    TimeValues() = default;
    ~TimeValues() = default;

    // Same meaning as the STL meanings, just done faster via variable pos_
    TimeValueIterator begin() const;
    TimeValueIterator end() const;
    TimeValueIterator upper_bound(double time) const;
    TimeValueIterator find(double time) const;
    void erase(TimeValueIterator it);
    void insert(double time, int value);

    /// Returns the number of data entries in the data container
    size_t size() const;

    /// Limit category data by points; return 1 if entries removed
    int limitByPoints(uint32_t limitPoints);
    /// Limit category data by time; return 1 if entries removed
    int limitByTime(double timeLimit);
    /// Remove all data in the slice
    void completeFlush();
    /// Remove points in the given time range; up to but not including endTime
    void flush(double startTime, double endTime);

  private:
    /// Verifies pos before using
    size_t checkPosition_(size_t pos) const;

    // Same meaning as the STL meanings, just done faster via variable pos_
    TimeValueIterator upperBound_(TimeValueIterator begin, TimeValueIterator current, TimeValueIterator end, double time) const;
    TimeValueIterator find_(TimeValueIterator start, TimeValueIterator current, TimeValueIterator end, double time) const;

    /// Invalid the time range around lastPos_ to force a new seach
    void invalidateLastPosTime_() const;

    const static size_t FastSearchWidth = 3; ///< Number of entries to check linearly before switching to a binary search
    mutable size_t lastPos_ = 0;  // the last location referenced so use as the start location when searching
    mutable std::optional<double> timeRangeStart_;  ///< The time of the point at or before lastPos_;
    mutable std::optional<double> timeRangeEnd_; ///< The time of the point after lastPos_
    mutable TimeValuesEntries entries_;  ///< The actual category data
  };

  /// A time to indicate no available category data
  static const int NO_CATEGORY_DATA = -1;
  /// Need to keep track of TimeValue state to keep track of changes in current category value
  struct TimeValueState
  {
    TimeValues data;
    int lastValue;
    double lastUpdateTime;
    TimeValueState()
      : lastValue(0),
        lastUpdateTime(NO_CATEGORY_DATA)
    {
    }
  };

  /// all the data for one entity (in an optimized data structure)
  // (it's a map from category name int to maps of time to category value ints)
#ifdef HAVE_ENTT
  typedef entt::dense_map<int, TimeValueState> EntityData;
#else
  typedef std::map<int, TimeValueState> EntityData;
#endif // HAVE_ENTT


private: // methods
  void insertOneEntry_(double time, const CategoryData::Entry &e);

  /**
   * Limit category data by points
   * Applies individually to each category.  Does not apply any default value.
   */
  void limitByPoints_(uint32_t limitPoints);

  /**
   * Limit category data by time
   * Applies individually to each category.  Does not apply any default value.
   * The supplied time is a delta time with respect to the last time in the category deque
   */
  void limitByTime_(double timeLimit);

  //--- from CategoryDataSlice
  virtual std::unique_ptr<IteratorImpl> iterator_() const;

private: // data
  EntityData data_;
  double lastUpdateTime_;
  CategoryNameManager* categoryNameManager_;
  size_t sliceSize_;
  std::function<void()> notifierFn_;
};

} // namespace

#endif

