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
#include "simCore/Calc/Math.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/MemoryCategoryDataSlice.h"

namespace simData
{

namespace {
  const double DEFAULT_TIME = -1.0;
}

MemoryCategoryDataSlice::TimeValues::TimeValues()
  : lastPos_(0)
{
}

MemoryCategoryDataSlice::TimeValues::~TimeValues()
{
}

/* Gets the start of the deque */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::begin() const
{
  return entries.begin();
}

/* Gets the end of the deque */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::end() const
{
  return entries.end();
}

/** Gets the upper bound from the deque */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::upper_bound(double time) const
{
  lastPos_ = checkPosition_(lastPos_);
  TimeValueIterator rv = upperBound_(entries.begin(), entries.begin() + lastPos_, entries.end(), time);
  lastPos_ = rv - entries.begin();
  return rv;
}

/** Finds data from the deque */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::find(double time) const
{
  lastPos_ = checkPosition_(lastPos_);
  TimeValueIterator rv = find_(entries.begin(), entries.begin() + lastPos_, entries.end(), time);
  lastPos_ = rv - entries.begin();
  return rv;
}

/** Removes data from the deque */
void MemoryCategoryDataSlice::TimeValues::erase(TimeValueIterator it)
{
  entries.erase(it);
}

/** Retrieves size of the entries */
size_t MemoryCategoryDataSlice::TimeValues::size() const
{
  // Note that this is O(1) complexity, safe to call
  return entries.size();
}

/** Inserts data into the deque */
void MemoryCategoryDataSlice::TimeValues::insert(double time, int value)
{
  // First do initial condition
  if (entries.empty())
  {
    entries.push_back(TimeValuePair(time, value));
    return;
  }

  // Second so the common case of appending to the end
  if (entries.back().time < time)
  {
    entries.push_back(TimeValuePair(time, value));
    return;
  }

  // Not appending to the end, so need to find the location
  TimeValueIterator it = upper_bound(time);
  if (it == entries.begin())
  {
    entries.push_front(TimeValuePair(time, value));
    return;
  }

  TimeValueIterator backOne = it - 1;
  if (backOne->time == time)
  {
    // Over-write the old value
    backOne->value = value;
    return;
  }

  entries.insert(it, TimeValuePair(time, value));
}

void MemoryCategoryDataSlice::TimeValues::limitByPoints(uint32_t limitPoints)
{
  // The zero case should already be handled
  assert(limitPoints);

  if (entries.size() <= limitPoints)
    return;

  bool reinsertOldTime = false;
  int oldValue = 0;
  if (entries.front().time == DEFAULT_TIME)
  {
    oldValue = entries.front().value;
    reinsertOldTime = true;
  }

  // This algorithm is different than SIMDIS 9 in that any default value is NOT counted against limitPoints
  size_t numToRemove = entries.size() - limitPoints;
  if (reinsertOldTime)
  {
    // Break out early if only removing the -1 time
    if (numToRemove == 1)
      return;
  }

  entries.erase(entries.begin(), entries.begin() + numToRemove);

  // Re-add the -1 time value
  if (reinsertOldTime)
    entries.push_front(TimeValuePair(DEFAULT_TIME, oldValue));
}

void MemoryCategoryDataSlice::TimeValues::limitByTime(double timeLimit)
{
  // The zero case should already be handle
  assert(timeLimit > 0.0);

  if (entries.size() < 2)
    return;

  bool reinsertOldTime = false;
  int oldValue = 0;
  if (entries.front().time == DEFAULT_TIME)
  {
    oldValue = entries.front().value;
    reinsertOldTime = true;
    if (entries.size() < 3)
      return;
  }

  double lastTime = entries.rbegin()->time;
  double limitPointsBeforeTime = lastTime - simCore::sdkMax(0.0, static_cast<double>(timeLimit));
  TimeValueIterator dataIter = std::lower_bound(entries.begin(), entries.end(), TimeValuePair(limitPointsBeforeTime, 0));
  if (dataIter == entries.end())
  {
    // no element was found with a timestamp >= limitPointsBeforeTime
    // all elements have timestamps < limitPointsBeforeTime
    entries.clear();
  }
  else
  {
    // dataIter is the first element in dataContainer with a timestamp >= limitPointsBeforeTime
    // all elements before dataIter have timestamps < limitPointsBeforeTime
    if (dataIter != entries.begin())
    {
      entries.erase(entries.begin(), dataIter);
    }
  }

  // Re-add the -1 time value
  if (reinsertOldTime)
    entries.push_front(TimeValuePair(DEFAULT_TIME, oldValue));
}

/** Returns a validated position */
size_t MemoryCategoryDataSlice::TimeValues::checkPosition_(size_t pos) const
{
  if (entries.empty())
    return 0;
  return pos >= entries.size() ? (entries.size() - 1) : pos;
}

/**
 * Finds the entry with the time greater than the argument time
 *@param[in] begin specifies the start of the search range
 *@param[in] current the location of the last reference
 *@param[in] end specifies the end of the search range
 *@param[in] time specifies the time to retrieve data for
 *@return iterator to the data, if found, otherwise returns end
 */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::upperBound_(TimeValueIterator begin, TimeValueIterator current, TimeValueIterator end, double time) const
{
  if (current != end)
  {
    if (current->time <= time)
    {
      for (size_t ii = 0; ii < FastSearchWidth && current != end; ii++, ++current)
      {
        if (current->time > time)
        {
          return current;
        }
      }

      if (current == end)
        return end;
    }
    else
    {
      for (size_t ii = 0; ii < FastSearchWidth && current != begin; ii++, --current)
      {
        if (current->time <= time)
        {
          ++current;
          return current;
        }
      }
    }
  }

  return std::upper_bound(begin, end, TimeValuePair(time, 0));
}

/**
 * Finds the entry with the exact time match
 *@param[in] begin specifies the start of search range
 *@param[in] current the location of the last reference
 *@param[in] end specifies the end of the search range
 *@param[in] time specifies the time to retrieve data for
 *@return iterator to the data, if found, otherwise returns end
 */
MemoryCategoryDataSlice::TimeValueIterator MemoryCategoryDataSlice::TimeValues::find_(TimeValueIterator begin, TimeValueIterator current, TimeValueIterator end, double time) const
{
  if (current != end)
  {
    if (current->time <= time)
    {

      for (size_t ii = 0; ii < FastSearchWidth && current != end; ii++, ++current)
      {
        if (current->time == time)
        {
          return current;
        }
      }
    }
    else
    {
      for (size_t ii = 0; ii < FastSearchWidth && current != begin; ii++, --current)
      {
        if (current->time == time)
        {
          return current;
        }
      }
    }
  }

  current = std::lower_bound(begin, end, TimeValuePair(time, 0));
  if (current == end)
    return end;

  if (current->time == time)
    return current;

  return end;
}

//----------------------------------------------------------------------------
/**
 * Interposer to category data
 *
 * Category Data is a time-based name/value string pair.  In the interest of
 * performance, the strings also have numerical indexes.  CategoryNameManager is
 * responsible for the mapping between indexes and values (but this interface
 * provides either as needed).
 */
class MemoryCategoryDataSlice::MemoryCategoryDataPair : public CategoryDataPair
{
public:
  /**
   * Public constructor for MemoryCategoryDataSlice
   * @param[in] catInt key for the category name
   * @param[in] valInt key for the category value
   * @param[in] categoryNameManager Used to dereference category and value
   */
  MemoryCategoryDataPair(int catInt, int valInt, CategoryNameManager& categoryNameManager)
    : catInt_(catInt),
    valInt_(valInt),
    categoryNameManager_(categoryNameManager)
  {
  }

  ///@return the category name as a string
  virtual std::string name() const
  {
    return categoryNameManager_.nameIntToString(catInt_);
  }

  ///@return the string value for the current category
  virtual std::string value() const
  {
    return categoryNameManager_.valueIntToString(valInt_);
  }

  ///@return the integer key for the category name
  virtual int nameInt() const
  {
    return catInt_;
  }

  ///@return the integer key for the value for the current category
  virtual int valueInt() const
  {
    return valInt_;
  }

private:
  int catInt_;
  int valInt_;
  CategoryNameManager& categoryNameManager_;
};

//----------------------------------------------------------------------------
/// implementation for iterators in MemoryCategoryDataSlice
class MemoryCategoryDataSlice::Iterator : public CategoryDataSlice::IteratorImpl
{
public:
  /**
   *@param[in] parent data source
   *@param[in] catInt starting category name key
   *@param[in] time time to retrieve data for
   */
  Iterator(const MemoryCategoryDataSlice &parent, int catInt, double time)
    : parent_(parent),
    time_(time)
  {
    current_ = parent_.data_.find(catInt);
    current_ = advance_(); // advance to first good state
  }

  /** Retrieves next item and increments iterator to next element */
  virtual std::tr1::shared_ptr<CategoryDataPair> next()
  {
    if (!hasNext())
    {
      assert(false);
      return std::tr1::shared_ptr<CategoryDataPair>();
    }

    // create the data pair
    std::tr1::shared_ptr<CategoryDataPair> ret = makePair_(current_);

    // go to the next absolute category (which might not be valid for this slice time)
    ++current_;

    // advance to next good state
    current_ = advance_();

    return ret;
  }

  /** Retrieves next item and does not increment iterator to next element */
  virtual std::tr1::shared_ptr<CategoryDataPair> peekNext() const
  {
    if (!hasNext())
    {
      assert(false);
      return std::tr1::shared_ptr<CategoryDataPair>();
    }

    return makePair_(current_);
  }

  /** Retrieves previous item and decrements iterator to prev element */
  virtual std::tr1::shared_ptr<CategoryDataPair> previous()
  {
    if (!hasPrevious())
    {
      assert(false);
      return std::tr1::shared_ptr<CategoryDataPair>();
    }

    // go to previous category (which might not be valid for this slice time)
    --current_;

    // go back to the last good state
    current_ = retreat_(current_);

    return makePair_(current_);
  }

  /** Retrieves previous item and does not decrement iterator to prev element */
  virtual std::tr1::shared_ptr<CategoryDataPair> peekPrevious() const
  {
    if (!hasPrevious())
    {
      assert(false);
      return std::tr1::shared_ptr<CategoryDataPair>();
    }

    // like previous, but do not change current_
    EntityData::const_iterator prev = current_;
    --prev;
    prev = retreat_(prev);
    return makePair_(prev);
  }

  /** Resets the iterator to the front of the data structure */
  virtual void toFront()
  {
    current_ = parent_.data_.begin();
  }

  /** Sets the iterator to the end of the data structure */
  virtual void toBack()
  {
    current_ = parent_.data_.end();
  }

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  virtual bool hasNext() const
  {
    return current_ != parent_.data_.end();
  }

  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  virtual bool hasPrevious() const
  {
    if (current_ == parent_.data_.begin())
      return false; // nowhere to go

    // else, will there be somewhere valid after pre-decrement?
    EntityData::const_iterator i = current_;
    --i;
    i = retreat_(i);

    // is there category data for the given time
    return i->second.data.upper_bound(time_) != i->second.data.begin();
  }

  /** Create a copy of the actual implementation */
  virtual IteratorImpl* clone() const
  {
    return new Iterator(*this);
  }

private: // methods
  std::tr1::shared_ptr<CategoryDataPair> makePair_(EntityData::const_iterator iter) const
  {
    int catInt = CategoryNameManager::NO_CATEGORY_NAME;
    int valInt = CategoryNameManager::NO_CATEGORY_VALUE;
    if (iter != parent_.data_.end())
    {
      catInt = iter->first;

      TimeValueIterator i = iter->second.data.upper_bound(time_);
      if (i != iter->second.data.begin())
      {
        --i;
        valInt = i->value;
      }
    }

    return std::tr1::shared_ptr<CategoryDataPair>(new MemoryCategoryDataPair(catInt, valInt, *parent_.categoryNameManager_));
  }

private: // methods
  /** After moving forward, return the next valid iterator.
   *
   * We store the current iterator into the parent map.  This map is from
   * category name ints to (maps from time to category value ints).
   *
   * Incrementing the iterator advances us to the next category name int.
   *
   * However, this category might not have data for the given time.  advance_
   * ensures one of:
   * 1) The iterator is done (equal to parent_.data_.end())
   * 2) The iterator points to a category with data valid for the time of the slice
   */
  EntityData::const_iterator advance_() const
  {
    EntityData::const_iterator i(current_);

    // while not at the end, and there is no category data for the given time
    while (i != parent_.data_.end() && i->second.data.upper_bound(time_) == i->second.data.begin())
    {
      ++i; // advance
    }

    return i;
  }

  /// inverse of advance_()
  EntityData::const_iterator retreat_(EntityData::const_iterator current) const
  {
    EntityData::const_iterator i(current);
    // while not at beginning, and there is no category data for the given time
    while (i != parent_.data_.begin() && i->second.data.upper_bound(time_) == i->second.data.begin())
    {
      --i; // retreat
    }

    return i;
  }

private: // data
  const MemoryCategoryDataSlice &parent_;
  EntityData::const_iterator current_;
  double time_;
};

//----------------------------------------------------------------------------
MemoryCategoryDataSlice::MemoryCategoryDataSlice(double timeStamp)
  : lastUpdateTime_(timeStamp),
  categoryNameManager_(NULL),
  sliceSize_(0)
{
}

double MemoryCategoryDataSlice::lastUpdateTime() const
{
  return lastUpdateTime_;
}

///@return true if the category data changes
bool MemoryCategoryDataSlice::update(double time)
{
  // there is no one place which has "the current category data" that we need to update
  // (data is produced on demand).
  // Thus, we could simply update the lastUpdateTime_ and return.
  // However, for notifications, we need to look for data which has changed

  // do not exit early - all category data must be updated for time before returning
  bool ret = false; // we will return true if anything has changed

  //for each category
  for (EntityData::iterator i = data_.begin(); i != data_.end(); ++i)
  {
    TimeValueState& timeState = i->second;
    // look for value beyond update time
    TimeValueIterator j = timeState.data.upper_bound(time);
    if (j == timeState.data.begin())
    {
      if (timeState.lastUpdateTime != NO_CATEGORY_DATA)
      {
        timeState.lastUpdateTime = NO_CATEGORY_DATA;
        ret = true; // Went from category data to no category data so something has changed
      }
      continue;
    }

    --j;

    if (!simCore::areEqual(j->time, timeState.lastUpdateTime))
    {
      if (timeState.lastUpdateTime == NO_CATEGORY_DATA)
        ret = true;  // Went from no category data to category data so something changed

      timeState.lastUpdateTime = j->time;
    }

    // Just because the time changed does not mean the value actually changed, check the value
    if (j->value != timeState.lastValue)
    {
      timeState.lastValue = j->value;
      ret = true; // something has changed
    }
  }

  lastUpdateTime_ = time;
  return ret;
}

/// receive all the category data in the data slice
void MemoryCategoryDataSlice::visit(Visitor *visitor) const
{
  assert(categoryNameManager_);
  //for each category
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    //for each time
    for (TimeValueIterator j = i->second.data.begin(); j != i->second.data.end(); ++j)
    {
      CategoryData cd;
      cd.set_time(j->time);

      CategoryData::Entry *e = cd.add_entry();
      e->set_key(categoryNameManager_->nameIntToString(i->first));
      e->set_value(categoryNameManager_->valueIntToString(j->value));

      (*visitor)(&cd);
    }
  }
}

CategoryDataSlice::Iterator MemoryCategoryDataSlice::current() const
{
  return CategoryDataSlice::Iterator(this);
}

bool MemoryCategoryDataSlice::removePoint(double time, int catNameInt, int valueInt)
{
  EntityData::iterator i = data_.find(catNameInt);
  if (i == data_.end())
    return false; // no such category

  TimeValueState& timeState = i->second;
  TimeValueIterator j = timeState.data.find(time);
  if (j == timeState.data.end())
    return false; // no such time

  if (j->value != valueInt)
    return false; // value mismatch

  // successful match, remove it
  timeState.data.erase(j);
  // Assertion failure means we're about to overflow; our count is out of sync
  assert(sliceSize_ > 0);
  sliceSize_--;
  return true;
}

void MemoryCategoryDataSlice::insertOneEntry_(double time, const CategoryData::Entry &e)
{
  assert(categoryNameManager_);
  const int catInt = categoryNameManager_->addCategoryName(e.key());
  const int valInt = categoryNameManager_->addCategoryValue(catInt, e.value());

  data_[catInt].data.insert(time, valInt);
  sliceSize_++;
}

void MemoryCategoryDataSlice::insert(CategoryData *data)
{
  for (int i = 0; i < data->entry_size(); ++i)
  {
    insertOneEntry_(data->time(), data->entry(i));
  }

  delete data;
}

void MemoryCategoryDataSlice::limitByPoints_(uint32_t limitPoints)
{
  // zero is special case for "no limit"
  if (limitPoints == 0)
    return;

  sliceSize_ = 0;
  for (EntityData::iterator i = data_.begin(); i != data_.end(); ++i)
  {
    i->second.data.limitByPoints(limitPoints);
    sliceSize_ += i->second.data.size();
  }
}

void MemoryCategoryDataSlice::limitByTime_(double timeLimit)
{
  if (timeLimit <= 0.0)
    return; // nothing to do

  sliceSize_ = 0;
  for (EntityData::iterator i = data_.begin(); i != data_.end(); ++i)
  {
    i->second.data.limitByTime(timeLimit);
    sliceSize_ += i->second.data.size();
  }
}

/// apply the data limits indicated by 'prefs'
void MemoryCategoryDataSlice::limitByPrefs(const CommonPrefs &prefs)
{
  limitByPoints_(prefs.datalimitpoints());
  limitByTime_(prefs.datalimittime());
}

/// remove all data in the slice, retaining current category data and the static point
void MemoryCategoryDataSlice::flush()
{
  limitByPoints_(1);
}

CategoryDataSlice::IteratorImpl* MemoryCategoryDataSlice::iterator_() const
{
  int intVal = CategoryNameManager::NO_CATEGORY_VALUE;
  if (!data_.empty())
    intVal = data_.begin()->first;
  return new Iterator(*this, intVal, lastUpdateTime_);
}

/// retrieve all
void MemoryCategoryDataSlice::allNames(std::vector<std::string> &nameVec) const
{
  assert(categoryNameManager_);
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    nameVec.push_back(categoryNameManager_->nameIntToString(i->first));
  }
}

void MemoryCategoryDataSlice::allNameInts(std::vector<int> &nameIntVec) const
{
  // very much like allNames()
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    nameIntVec.push_back(i->first);
  }
}

void MemoryCategoryDataSlice::allValues(std::vector<std::string> &valueVec) const
{
  assert(categoryNameManager_);
  //for each category
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    // look for value beyond current time
    TimeValueIterator j = i->second.data.upper_bound(lastUpdateTime_);
    if (j == i->second.data.begin())
      continue;

    --j;

    valueVec.push_back(categoryNameManager_->valueIntToString(j->value));
  }
}

void MemoryCategoryDataSlice::allValueInts(std::vector<int> &valueIntVec) const
{
  // much like allValues()
  //for each category
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    // look for value beyond current time
    TimeValueIterator j = i->second.data.upper_bound(lastUpdateTime_);
    if (j == i->second.data.begin())
      continue;

    --j;

    valueIntVec.push_back(j->value);
  }
}

void MemoryCategoryDataSlice::allStrings(std::vector<std::pair<std::string, std::string> > &nameValueVec) const
{
  assert(categoryNameManager_);
  //for each category
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    // look for value beyond current time
    TimeValueIterator j = i->second.data.upper_bound(lastUpdateTime_);
    if (j == i->second.data.begin())
      continue;

    --j;

    nameValueVec.push_back(std::make_pair(categoryNameManager_->nameIntToString(i->first),
      categoryNameManager_->valueIntToString(j->value)));
  }
}

void MemoryCategoryDataSlice::allInts(std::vector<std::pair<int, int> > &nameValueIntVec) const
{
  // much like allStrings()
  //for each category
  for (EntityData::const_iterator i = data_.begin(); i != data_.end(); ++i)
  {
    // look for value beyond current time
    TimeValueIterator j = i->second.data.upper_bound(lastUpdateTime_);
    if (j == i->second.data.begin())
      continue;

    --j;

    nameValueIntVec.push_back(std::make_pair(i->first, j->value));
  }
}

void MemoryCategoryDataSlice::setCategoryNameManager(CategoryNameManager* categoryNameManager)
{
  assert(categoryNameManager);
  categoryNameManager_ = categoryNameManager;
}

size_t MemoryCategoryDataSlice::numItems() const
{
  return sliceSize_;
}

bool MemoryCategoryDataSlice::isDuplicateValue(double time, const std::string& catName, const std::string& value) const
{
  const int catInt = categoryNameManager_->nameToInt(catName);
  EntityData::const_iterator edi = data_.find(catInt);
  // Category name does not exist -- return early (not duplicate)
  if (edi == data_.end())
    return false;

  const TimeValues& timeValues = edi->second.data;
  TimeValueIterator tvi = timeValues.upper_bound(time);
  // If the upper_bound returns begin(), then there's no earlier value, so it's not duplicate
  if (tvi == timeValues.begin())
    return false;
  // Decrement -- now at or before the provided time
  --tvi;
  const int valueInt = categoryNameManager_->valueToInt(value);
  // Can only be duplicate if the values match
  return tvi->value == valueInt;
}

}
