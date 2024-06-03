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

#include <limits>

#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/MemoryCategoryDataSlice.h"
#include "simData/MemoryGenericDataSlice.h"

namespace simData
{

/// -1 time is sentinel value used for infinite expiration.
static const double INFINITE_EXPIRATION_TIME = -1.0;
/// How long to look for a value string match before giving up and making a new entry
static const int COUNT_DOWN = 5;

/// Holds all the values for one Generic Data Key
class MemoryGenericDataSlice::Key
{
public:
  /** Constructor */
  explicit Key(const std::string& key)
    : key_(key)
  {
    flush();
  }

  virtual ~Key()
  {
  }

  /// Removes all times and values
  void flush()
  {
    // No static entries (-1 time) so just clear everything
    times_.clear();
    values_.clear();
    indexOffset_ = 0;
    lastUpdateDirty_ = true;
  }

  void flush(double startTime, double endTime)
  {
    // Instead of attempting to delete entries and update the data structure,
    // just save what is needed to a temporary vector, flush the data and rebuild.

    // Keep track of time value pairs
    struct TimeValuePair
    {
      double time;
      std::string value;
      TimeValuePair(double inTime, const std::string& inValue)
        : time(inTime),
          value(inValue)
      {}
    };

    // Find the remaining time value pairs
    std::vector<TimeValuePair> remainingValues;
    for (const auto& timeIndex : times_)
    {
      /// Filter out the time value pairs in the time range
      if ((timeIndex.time >= startTime) && (timeIndex.time < endTime))
        continue;

      const auto index = timeIndex.index - indexOffset_;
      // verify that indexOffset_ is updated correctly; dev error if assert
      assert((index >= 0) && (index < static_cast<int>(values_.size())));
      const ValueIndex& cacheValue = values_[index];
      remainingValues.push_back(TimeValuePair(timeIndex.time, cacheValue.value));
    }

    // clear
    flush();

    // and rebuild
    for (const auto& pair : remainingValues)
      insert(pair.time, pair.value, false);
  }

  /// If a value is no longer referenced remove it.
  void removeOrphans_()
  {
    ValueList::iterator end;
    for (end = values_.begin(); end != values_.end(); ++end)
    {
      if (end->referenceCount != 0)
        break;
    }

    if (end != values_.begin())
    {
      indexOffset_ += static_cast<int>(std::distance(values_.begin(), end));
      values_.erase(values_.begin(), end);
    }
  }

  /// Data limiting by number of points
  bool limitByPoints_(uint32_t limitPoints)
  {
    // zero is special case for "no limit"
    if (limitPoints == 0)
      return false;

    size_t size = times_.size();

    // Impossible to have a key with 0 values since limiting always leaves a value and flush() removes key
    assert(size != 0);

    if (size <= limitPoints)
      return false;

    // The amount to remove
    const size_t amount = size - limitPoints;

    // Decrease reference count
    for (uint32_t i = 0; i < amount; ++i)
      values_[times_[i].index - indexOffset_].referenceCount--;

    // Actually remove
    times_.erase(times_.begin(), times_.begin() + amount);

    return true;
  }

  /// Data limit by time limit
  bool limitByTime_(double timeLimit)
  {
    // zero is special case for "no limit"
    if (timeLimit <= 0.0)
      return false;

    // Impossible to have a key with 0 values since limiting always leaves a value and flush() removes key
    assert(!times_.empty());
    if (times_.empty())
      return false;

    // Decrease reference count on string values about to be removed
    const double cutoff = times_.back().time - timeLimit;
    TimeList::iterator timeEnd;
    for (timeEnd = times_.begin(); timeEnd != times_.end(); ++timeEnd)
    {
      if (timeEnd->time >= cutoff)
        break;
      values_[timeEnd->index - indexOffset_].referenceCount--;
    }

    if (times_.begin() != timeEnd)
    {
      times_.erase(times_.begin(), timeEnd);
      return true;
    }

    return false;
  }

  /// Data limit by preferences
  void limitByPrefs(const CommonPrefs& prefs)
  {
    const bool pointChanged = limitByPoints_(prefs.datalimitpoints());
    const bool timeChanged = limitByTime_(prefs.datalimittime());

    if (pointChanged || timeChanged)
    {
      // Remove any orphan value index
      removeOrphans_();

      // Not sure this is needed; Can only data limit in Live mode.
      // So by definition the limiting can't affect current_
      lastUpdateDirty_ = true;
    }
  }

  /// if ignoreDuplicates is true; successive duplicate values, with different times, will not be added to times_
  void insert(double time, const std::string& value, bool ignoreDuplicates)
  {
    // Find location
    TimeList::iterator start = times_.end();
    if (!times_.empty() && (time <= times_.back().time))
      start = std::lower_bound(times_.begin(), times_.end(), TimeIndex(time), Key::lessByTime);

    // prevent duplicates at the same time; independent of the ignoreDuplicates
    for (; start != times_.end(); ++start)
    {
      if (start->time != time)
        break;

      // check values
      const auto index = start->index - indexOffset_;
      // verify that indexOffset_ is updated correctly; dev error if assert
      assert((index >= 0) && (index < static_cast<int>(values_.size())));
      const ValueIndex& cacheValue = values_[index];
      if (cacheValue.value == value)
        return; // no assert, user provided data
    }

    // If necessary ignore historical duplicates
    if ((ignoreDuplicates) && (!times_.empty()) && (start != times_.begin()))
    {
      TimeList::iterator check = start;
      --check;
      const auto index = check->index - indexOffset_;
      // verify that indexOffset_ is updated correctly; dev error if assert
      assert((index >= 0) && (index < static_cast<int>(values_.size())));
      const ValueIndex& cacheValue = values_[index];
      if (cacheValue.value == value)
        return;
    }

    // check for repeat values
    int valueIndex = -1;
    int countDown = COUNT_DOWN;  // After 5 checks give up and consider "new"
    for (ValueList::reverse_iterator it = values_.rbegin(); it != values_.rend(); ++it)
    {
      if (it->value == value)
      {
        valueIndex = static_cast<int>(std::distance(it, values_.rend())) - 1 + indexOffset_;
        it->referenceCount++;
        break;
      }
      countDown--;
      if (countDown == 0)
        break;
    }

    // If not handled added
    if (valueIndex == -1)
    {
      values_.push_back(ValueIndex(value, 1));
      valueIndex = static_cast<int>(values_.size()) - 1 + indexOffset_;
    }

    // Finally add to the times_ list
    times_.insert(start, TimeIndex(time, valueIndex));

    // lazy update requires this
    lastUpdateDirty_ = true;
  }

  /// Updates to the given time, putting results in genericData
  void update(double time, GenericData& genericData)
  {
    lastUpdateDirty_ = false;

    // Impossible to have a key with 0 values since limiting always leaves a value and flush() removes key
    assert(!times_.empty());
    if (times_.empty())
      return;

    TimeList::const_iterator it = std::upper_bound(times_.begin(), times_.end(), TimeIndex(time), Key::lessByTime);
    if (it == times_.begin())
      return;

    --it;

    simData::GenericData_Entry* newEntry = genericData.add_entry();
    newEntry->set_key(key_);
    newEntry->set_value(values_[it->index - indexOffset_].value);
  }

  /** Returns true if last update dirty */
  bool hasChanged() const
  {
    return lastUpdateDirty_;
  }

  /** Retrieves number of items */
  size_t numItems() const
  {
    return times_.size();
  }

  /** Returns the key */
  std::string name() const
  {
    return key_;
  }

  /** Retrieve the time and value at the given index, returning true on success */
  bool getItem(size_t index, double& time, std::string& value) const
  {
    // Asking for an index that does not exist
    assert(index < times_.size());
    if (index >= times_.size())
      return false;

    time = times_[index].time;
    value = values_[times_[index].index - indexOffset_].value;
    return true;
  }

private:
  /// Time with an index into the value list for the value string
  struct TimeIndex
  {
    double time;
    int index;
    TimeIndex(double inTime = 0.0, int inIndex = 0)
      : time(inTime),
        index(inIndex)
    {}
  };
  typedef std::deque<TimeIndex> TimeList;

  /// Needed for the calls to std::upper_bound
  static bool lessByTime(const TimeIndex& a, const TimeIndex& b)
  {
    return (a.time < b.time);
  }

  /// The value string with a reference counter
  struct ValueIndex
  {
    std::string value;
    int referenceCount;
    ValueIndex(const std::string& inValue="", int inReferenceCount=0)
      : value(inValue),
        referenceCount(inReferenceCount)
    {}
  };
  typedef std::deque<ValueIndex> ValueList;

  std::string key_;  ///< The key for this generic data
  TimeList times_;  ///< List of times
  ValueList values_;  /// List of values
  int indexOffset_;  ///< As the values list is trim need to offset the existing indexes in times_
  bool lastUpdateDirty_; ///< True if changes have been made since last update
};



//------------------------------------------------------------------------------------------------------------------

/// Helps collects data for the visitor pattern, by "walking" down the list of time/value pairs
class MemoryGenericDataSlice::Collector
{
public:
  /** Constructor */
  explicit Collector(MemoryGenericDataSlice::Key& key)
    : key_(key)
  {
    tag_ = key_.name();
    done_ = (key.numItems() == 0);
    index_ = 0;
    if (done_)
    {
      // List is empty, so initialize to empty
      time_ = std::numeric_limits<double>::max();
      value_ = "";
    }
    else
    {
      // Initialize the walk down with the first time/value pair
      key_.getItem(index_, time_, value_);
    }
  }

  virtual ~Collector()
  {
  }

  /// Returns true if all the time/value pairs have been processed
  bool isDone() const
  {
    return done_;
  }

  /// Returns the time of the current time/value pair or std::numeric_limits<double>::max() if no pair is available
  double time() const
  {
    return time_;
  }

  /// If the time matches the current time/value pair it is added to data and the time/value is advanced to the next pair
  void add(double time, GenericData& data)
  {
    if (time == time_)
    {
      simData::GenericData_Entry* entry = data.add_entry();
      entry->set_key(tag_);
      entry->set_value(value_);
      index_++;
      done_ = (key_.numItems() == index_);
      if (done_)
      {
        time_ = std::numeric_limits<double>::max();
        value_ = "";
      }
      else
      {
        key_.getItem(index_, time_, value_);
      }
    }
  }

private:
  MemoryGenericDataSlice::Key& key_;  ///< All the data for the given generic data key
  std::string tag_;  ///< The name of the generic data
  bool done_;  ///< True means all time/value pairs have been processed
  size_t index_;  ///< The index into the time/value pair list
  double time_; ///< The time of the current time/value pair
  std::string value_; ///< The value of the current time/value pair
};

//------------------------------------------------------------------------------------------------------------------


MemoryGenericDataSlice::MemoryGenericDataSlice()
  : lastTime_(-1.0),
    force_(false)
{
}

MemoryGenericDataSlice::~MemoryGenericDataSlice()
{
  flush();
}

void MemoryGenericDataSlice::flush()
{
  // No static entries (-1 time) so just clear everything
  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
    delete it->second;
  genericData_.clear();
  lastTime_ = -1.0;
}

void MemoryGenericDataSlice::flush(double startTime, double endTime)
{
  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
    it->second->flush(startTime, endTime);

  // force a recalculation of current_
  lastTime_ = -1.0;
}

void MemoryGenericDataSlice::limitByPrefs(const CommonPrefs& prefs)
{
  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
    it->second->limitByPrefs(prefs);
}

bool MemoryGenericDataSlice::update(double time)
{
  current_.set_duration(INFINITE_EXPIRATION_TIME);
  current_.set_time(time);

  // Since 99% of the time no one calls current(), don't calculate current_ until needed
  if (!hasChanged())
  {
    if (time == lastTime_)
      return false;
  }

  return true;
}

void MemoryGenericDataSlice::insert(GenericData* data, bool ignoreDuplicates)
{
  // Should always pass data in
  assert(data != nullptr);
  if (data == nullptr)
    return;

  for (int k = 0; k < data->entry_size(); ++k)
  {
    const std::string& key = data->entry(k).key();
    const std::string& value = data->entry(k).value();

    GenericDataMap::const_iterator it = genericData_.find(key);
    if (it == genericData_.end())
    {
      Key* newKey = new Key(key);
      newKey->insert(data->time(), value, ignoreDuplicates);
      genericData_[key] = newKey;
    }
    else
      it->second->insert(data->time(), value, ignoreDuplicates);
  }

  delete data;
}

int MemoryGenericDataSlice::removeTag(const std::string& tag)
{
  GenericDataMap::iterator it = genericData_.find(tag);
  if (it == genericData_.end())
    return 1;

  delete it->second;
  genericData_.erase(it);
  force_ = true;
  return 0;
}

bool MemoryGenericDataSlice::hasChanged() const
{
  if (force_)
    return true;

  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
  {
    if (it->second->hasChanged())
      return true;
  }

  return false;
}

bool MemoryGenericDataSlice::isDirty() const
{
  // this feature is not implemented for MemoryGenericDataSlice
  assert(0);
  return false;
}

void MemoryGenericDataSlice::visit(Visitor *visitor) const
{
  if (visitor == nullptr)
    return;

  if (genericData_.empty())
    return;

  // Make a helper class for each key
  std::deque<Collector> keys;
  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
    keys.push_back(Collector(*(it->second)));

  // Process until all keys report done
  while (true)
  {
    // See if done
    bool done = true;
    for (std::deque<Collector>::const_iterator it = keys.begin();(it != keys.end()) && (done == true); ++it)
      done = done && it->isDone();

    if (done)
      break;

    // Find earliest time
    double time = std::numeric_limits<double>::max();
    for (std::deque<Collector>::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
      if (!it->isDone() && (it->time() < time))
        time = it->time();
    }

    // Make message
    GenericData data;
    data.set_time(time);
    data.set_duration(INFINITE_EXPIRATION_TIME);
    for (std::deque<Collector>::iterator it = keys.begin(); it != keys.end(); ++it)
    {
      if (!it->isDone())
        it->add(time, data);
    }

    // Do the visit
    (*visitor)(&data);
  };
}

void MemoryGenericDataSlice::modify(Modifier* modifier)
{
  // Implement when/if needed
  assert(0);
}

const GenericData* MemoryGenericDataSlice::current() const
{
  if (!hasChanged())
  {
    if (current_.time() == lastTime_)
      return &current_;
  }

  force_ = false;

  // Reset the pairs data
  current_.clear_entry();

  if (genericData_.empty())
  {
    lastTime_ = current_.time();
    return &current_;
  }

  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
  {
    it->second->update(current_.time(), current_);
  }

  lastTime_ = current_.time();
  return &current_;
}

size_t MemoryGenericDataSlice::numItems() const
{
  size_t rv = 0;
  for (GenericDataMap::const_iterator it = genericData_.begin(); it != genericData_.end(); ++it)
    rv += it->second->numItems();

  return rv;
}

}
