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
#ifndef SIMDATA_MEMORYDATASLICE_INL_H
#define SIMDATA_MEMORYDATASLICE_INL_H

#include <algorithm>
#include <limits>
#include "simData/DataStore.h"
#include "simData/MessageVisitor/Message.h"
#include "simData/MessageVisitor/MessageVisitor.h"
#include "simData/MessageVisitor/protobuf.h"

namespace simData
{

namespace MemorySliceHelper
{
template<typename T>
SafeDequeIterator<T>::SafeDequeIterator()
: deque_(nullptr),
  val_(0)
{
}

template<typename T>
SafeDequeIterator<T>::SafeDequeIterator(typename std::deque<T>* deque, typename std::deque<T>::iterator i)
: deque_(deque)
{
  val_ = i - deque->begin();
}

template<typename T>
void SafeDequeIterator<T>::invalidate()
{
  if (deque_ != nullptr)
    val_ = static_cast<typename std::deque<T>::difference_type>(deque_->size());
  else
    val_ = 0;
}

template<typename T>
typename std::deque<T>::iterator SafeDequeIterator<T>::get() const
{
  if (deque_ == nullptr)
    return typename std::deque<T>::iterator();

  if (val_ > static_cast<typename std::deque<T>::difference_type>(deque_->size()))
    return deque_->end();

  return deque_->begin() + val_;
}

//----------------------------------------------------------------------------
template<typename T>
int limitByTime(std::deque<T*> &updates, double timeLimit)
{
  if (updates.empty() || timeLimit < 0.0)
    return -1; // nothing to do

  // get an iterator to the first point after the limit
  typename std::deque<T*>::iterator newFirstPt = std::upper_bound(updates.begin(), updates.end(), timeLimit, UpdateComp<T>());

  // always leave one point
  if (newFirstPt == updates.end())
    --newFirstPt;

  if (newFirstPt == updates.begin())
    return -1; // nothing to do

  // reclaim memory for the points which will be removed
  for (typename std::deque<T*>::iterator j = updates.begin(); j != newFirstPt; ++j)
    delete *j;

  // do the removal
  updates.erase(updates.begin(), newFirstPt);
  return 0;
}

template<typename T>
int limitByPoints(std::deque<T*> &updates, uint32_t limitPoints)
{
  // zero is special case for "no limit"
  if (limitPoints == 0)
    return -1;

  const size_t curPoints = updates.size();
  if (curPoints == 0 || curPoints <= limitPoints)  // Need "<=" instead of "<", else code below does nothing
    return -1; // nothing to do

  // set end point for deletion (only 'limitPoints' will remain at end)
  typename std::deque<T*>::iterator newFirstPt = updates.begin() + (curPoints - limitPoints);

  for (typename std::deque<T*>::iterator j = updates.begin(); j != newFirstPt; ++j)
    delete *j;

  updates.erase(updates.begin(), newFirstPt);
  return 0;
}

template<typename T>
int flush(std::deque<T*> &updates, bool keepStatic)
{
  // don't flush static entities
  if (keepStatic && updates.size() == 1 && (**updates.begin()).time() == -1.0)
    return 1;

  for (typename std::deque<T*>::iterator j = updates.begin(); j != updates.end(); ++j)
    delete *j;

  updates.clear();
  return 0;
}

template<typename T>
int flush(std::deque<T*> &updates, double startTime, double endTime)
{
  auto start = std::lower_bound(updates.begin(), updates.end(), startTime, UpdateComp<T>());
  if ((start == updates.end()) || ((*start)->time() >= endTime))
    return 1;

  // endTime is non-inclusive
  auto end = std::lower_bound(start, updates.end(), endTime, UpdateComp<T>());

  for (auto it = start; it != end; ++it)
    delete *it;

  updates.erase(start, end);
  return 0;
}

} // namespace MemorySliceHelper

template <class T>
VectorIterator<T>::VectorIterator(const std::deque<T *>* vec)
  : vec_(vec),
    nextIndex_(0)
{
  assert(vec_);
}

template <class T>
const T* const VectorIterator<T>::next()
{
  if (!hasNext())
    return nullptr;

  return (*vec_)[nextIndex_++];
}

template <class T>
const T* const VectorIterator<T>::peekNext() const
{
  if (!hasNext())
    return nullptr;

  return (*vec_)[nextIndex_];
}

template <class T>
const T* const VectorIterator<T>::previous()
{
  if (!hasPrevious())
    return nullptr;

  return (*vec_)[--nextIndex_];
}

template <class T>
const T* const VectorIterator<T>::peekPrevious() const
{
  if (!hasPrevious())
    return nullptr;
  return (*vec_)[(nextIndex_ - 1)];
}

template <class T>
void VectorIterator<T>::toFront()
{
  nextIndex_ = 0;
}

template <class T>
void VectorIterator<T>::toBack()
{
  nextIndex_ = vec_->size();
}

template <class T>
bool VectorIterator<T>::hasNext() const
{
  return nextIndex_ < vec_->size();
}

template <class T>
bool VectorIterator<T>::hasPrevious() const
{
  return nextIndex_ > 0 && nextIndex_ <= vec_->size();
}

template <class T>
typename DataSlice<T>::IteratorImpl* VectorIterator<T>::clone() const
{
  VectorIterator* rv = new VectorIterator(vec_);
  rv->nextIndex_ = nextIndex_;
  return rv;
}

template <class T>
void VectorIterator<T>::set(size_t idx)
{
  nextIndex_ = idx;
}

//----------------------------------------------------------------------------
template<typename T>
MemoryDataSlice<T>::MemoryDataSlice()
: mdsHasChanged_(false),
  dirty_(false),
  current_(nullptr),
  interpolated_(false),
  bounds_(static_cast<T*>(nullptr), static_cast<T*>(nullptr)),
  fastUpdate_(&updates_, updates_.end())
{
}

template<typename T>
MemoryDataSlice<T>::~MemoryDataSlice()
{
  MemorySliceHelper::flush(updates_, false);
}

template<typename T>
void MemoryDataSlice<T>::flush(bool keepStatic)
{
  if (MemorySliceHelper::flush(updates_, keepStatic) == 0)
    current_ = nullptr;
  dirty_ = true;
}

template<typename T>
void MemoryDataSlice<T>::flush(double startTime, double endTime)
{
  if (MemorySliceHelper::flush(updates_, startTime, endTime) == 0)
    current_ = nullptr;
  dirty_ = true;
}

template<typename T>
typename DataSlice<T>::Iterator MemoryDataSlice<T>::lower_bound(double timeValue) const
{
  VectorIterator<T>* rv = new VectorIterator<T>(&updates_);
  typename std::deque<T*>::const_iterator iter =  computeLowerBound<typename std::deque<T*>::const_iterator, T>(updates_.begin(), fastUpdate_.get(), updates_.end(), timeValue);
  rv->set(iter - updates_.begin());
  return typename DataSlice<T>::Iterator(rv);
}

template<typename T>
typename DataSlice<T>::Iterator MemoryDataSlice<T>::upper_bound(double timeValue) const
{
  VectorIterator<T>* rv = new VectorIterator<T>(&updates_);
  typename std::deque<T*>::const_iterator iter = computeUpperBound<typename std::deque<T*>::const_iterator, T>(updates_.begin(), fastUpdate_.get(), updates_.end(), timeValue);
  rv->set(iter - updates_.begin());
  return typename DataSlice<T>::Iterator(rv);
}

template<typename T>
size_t MemoryDataSlice<T>::numItems() const
{
  return updates_.size();
}

template<typename T>
bool MemoryDataSlice<T>::hasChanged() const
{
  return mdsHasChanged_;
}

template<typename T>
bool MemoryDataSlice<T>::isDirty() const
{
  return dirty_;
}

template<typename T>
const T* MemoryDataSlice<T>::current() const
{
  return current_;
}

template<typename T>
void MemoryDataSlice<T>::visit(typename DataSlice<T>::Visitor *visitor) const
{
  for (typename std::deque<T *>::const_iterator i = updates_.begin(); i != updates_.end(); ++i)
  {
    (*visitor)(*i);
  }
}

template<typename T>
void MemoryDataSlice<T>::modify(typename DataSlice<T>::Modifier *visitor)
{
   // Implement when/if needed
  assert(0);
}

template<typename T>
bool MemoryDataSlice<T>::isInterpolated() const
{
  return interpolated_;
}

template<typename T>
typename DataSlice<T>::Bounds MemoryDataSlice<T>::interpolationBounds() const
{
  return bounds_;
}

template<typename T>
void MemoryDataSlice<T>::clearChanged()
{
  mdsHasChanged_ = false;
}

template<typename T>
void MemoryDataSlice<T>::setChanged()
{
  mdsHasChanged_ = true;
}

template<typename T>
void MemoryDataSlice<T>::setCurrent(T* current)
{
  // this is a pointer comparison
  // if slice is interpolating:
  //   it detects a change from non-interpolated update to interpolated updated (or vice versa)
  //   it does not detect a new interpolated update from a previous interpolated update
  // if slice is not interpolating:
  //   it detects change from one update to another
  //   it correctly filters out the case when same update is returned
  if (current_ != current)
  {
    mdsHasChanged_ = true;
    current_ = current;
  }
}

template<typename T>
void MemoryDataSlice<T>::setInterpolated(bool interpolated, const typename DataSlice<T>::Bounds& bounds)
{
  interpolated_ = interpolated;
  bounds_ = bounds;
  // if new update is interpolated, it is a change. this handles the case where previous update was also interpolated (which setCurrent does not handle)
  if (interpolated)
    mdsHasChanged_ = true;
}

template<typename T>
void MemoryDataSlice<T>::update(double time)
{
  // start by marking as unchanged, new hasChanged status is outcome of this update
  clearChanged();

  // early out when there are no changes to this slice
  if (!dirty_ && (current_ != nullptr) && ((current_->time() == time) || (current_->time() == -1.0)))
    return;

  dirty_ = false;

  interpolated_ = false;
  fastUpdate_ = MemorySliceHelper::SafeDequeIterator<T*>(&updates_, computeTimeUpdate<typename std::deque<T*>::iterator, T>(updates_.begin(), fastUpdate_.get(), updates_.end(), time));
  if (fastUpdate_.get() != updates_.end())
    setCurrent(*fastUpdate_.get());
  else
    setCurrent(nullptr);
}

template<typename T>
void MemoryDataSlice<T>::update(double time, Interpolator *interpolator)
{
  // start by marking as unchanged, new hasChanged status is outcome of this update
  clearChanged();

  // early out when there are no changes to this slice
  if (!dirty_ && (current_ != nullptr) && ((current_->time() == time) || (current_->time() == -1.0)))
    return;

  // update is processing the changes to the slice, clear the flag
  dirty_ = false;

  typename DataSlice<T>::Bounds bounds;
  bool isBounded = false;
  typename std::deque<T*>::iterator it = fastUpdate_.get();

  // note that computeTimeUpdate can return a ptr to a real update, or pointer to currentInterpolated_
  setCurrent(computeTimeUpdate<typename std::deque<T*>::iterator, T, typename DataSlice<T>::Bounds>(updates_.begin(), it, updates_.end(), time, interpolator, &isBounded, &currentInterpolated_, &bounds));
  fastUpdate_ =  MemorySliceHelper::SafeDequeIterator<T*>(&updates_, it);
  setInterpolated(isBounded, bounds);
}

template<typename T>
void MemoryDataSlice<T>::insert(T *data)
{
  typename std::deque<T*>::iterator iter = updates_.end();
  if (!updates_.empty())
  {
    if (updates_.back()->time() >= data->time())
    {
      iter = std::lower_bound(updates_.begin(), updates_.end(), data, UpdateComp<T>());
      if ((*iter)->time() == data->time())
      {
        // null the current ptr, if we are replacing the update it aliases; current will become valid upon update
        if (current_ == *iter)
          setCurrent(nullptr);

        delete *iter;
        *iter = data;
        dirty_ = true;
        return;
      }
    }
  }
  updates_.insert(iter, data);
  fastUpdate_.invalidate();
  dirty_ = true;
}

template<typename T>
void MemoryDataSlice<T>::limitByTime(double timeWindow)
{
  if (timeWindow >= 0)
  {
    if (MemorySliceHelper::limitByTime(updates_, lastTime() - timeWindow) == 0)
      fastUpdate_.invalidate();
  }
}

template<typename T>
void MemoryDataSlice<T>::limitByPoints(uint32_t limitPoints)
{
  if (MemorySliceHelper::limitByPoints(updates_, limitPoints) == 0)
    fastUpdate_.invalidate();
}

template<typename T>
void MemoryDataSlice<T>::limitByPrefs(const CommonPrefs &prefs)
{
  limitByPoints(prefs.datalimitpoints());
  limitByTime(prefs.datalimittime());
}

template<typename T>
double MemoryDataSlice<T>::firstTime() const
{
  if (updates_.empty())
    return std::numeric_limits<double>::max();

  return (*updates_.begin())->time();
}

template<typename T>
double MemoryDataSlice<T>::lastTime() const
{
  if (updates_.empty())
    return -std::numeric_limits<double>::max();

  return (*updates_.rbegin())->time();
}

template<typename T>
double MemoryDataSlice<T>::deltaTime(double time) const
{
  if (updates_.empty() || (time < 0.0))
    return -1.0;

  typename std::deque<T*>::const_iterator it = computeLowerBound<typename std::deque<T*>::const_iterator, T>(updates_.begin(), fastUpdate_.get(), updates_.end(), time);

  if (it != updates_.end())
  {
    if ((*it)->time() == time)
      return 0.0;

    if (it == updates_.begin())
      return -1.0;
  }

  --it;

  // Check for static point
  if ((*it)->time() < 0.0)
    return -1.0;

  return time - (*it)->time();
}

template<typename T>
T* MemoryDataSlice<T>::currentInterpolated()
{
  return &currentInterpolated_;
}

template<typename T>
typename DataSlice<T>::IteratorImpl* MemoryDataSlice<T>::iterator_() const
{
  return new VectorIterator<T>(&updates_);
}

//----------------------------------------------------------------------------
template<class CommandType, class PrefType>
MemoryCommandSlice<CommandType, PrefType>::MemoryCommandSlice()
: lastUpdateTime_(-std::numeric_limits<double>::max()),
  hasChanged_(false),
  earliestInsert_(std::numeric_limits<double>::max())
{
}

template<class CommandType, class PrefType>
MemoryCommandSlice<CommandType, PrefType>::~MemoryCommandSlice()
{
  MemorySliceHelper::flush(updates_, false);
}

template<class CommandType, class PrefType>
const CommandType* MemoryCommandSlice<CommandType, PrefType>::current() const
{
  typename std::deque<CommandType*>::const_iterator i =
    std::upper_bound(updates_.begin(), updates_.end(), lastUpdateTime_, UpdateComp<CommandType>());

  return (i != updates_.begin()) ? *(i-1) : nullptr;
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::visit(typename DataSlice<CommandType>::Visitor *visitor) const
{
  for (typename std::deque<CommandType*>::const_iterator i = updates_.begin(); i != updates_.end(); ++i)
  {
    (*visitor)(*i);
  }
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::modify(typename DataSlice<CommandType>::Modifier *modifier)
{
  size_t size = updates_.size();
  size_t index = 0;
  while (index < size)
  {
    if (modifier->modify(*(updates_[index])) < 0)
    {
      auto it = updates_.begin() + index;
      delete *it;
      updates_.erase(it);
      --size;
    }
    else
      ++index;
  }
  // force a recalculation of commandPrefsCache_; less than optional solution
  // when necessary a future solution should reset the individual field
  reset_();
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::flush()
{
  MemorySliceHelper::flush(updates_);
  earliestInsert_ = std::numeric_limits<double>::max();
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::flush(double startTime, double endTime)
{
  MemorySliceHelper::flush(updates_, startTime, endTime);
  earliestInsert_ = std::numeric_limits<double>::max();
}

template<class CommandType, class PrefType>
bool MemoryCommandSlice<CommandType, PrefType>::hasChanged() const
{
  return hasChanged_;
}

template<class CommandType, class PrefType>
bool MemoryCommandSlice<CommandType, PrefType>::isDirty() const
{
  // this feature is not implemented for MemoryCommandSlice
  assert(0);
  return false;
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::insert(CommandType *data)
{
  typename std::deque<CommandType*>::iterator iter = std::lower_bound(updates_.begin(), updates_.end(), data, UpdateComp<CommandType>());
  if (data->time() < earliestInsert_)
    earliestInsert_ = data->time();
  if ((iter == updates_.end()) || (*iter)->time() != data->time())
  {
    // the transaction owns the data item, transfers ownership to the deque here
    updates_.insert(iter, data);
  }
  else
  {
    // Must clear out the shared fields in target, that are repeated and non-empty
    conditionalClearRepeatedFields_((*iter)->mutable_updateprefs(), data->mutable_updateprefs());
    // merge into existing command at same time
    (*iter)->MergeFrom(*data);
    // in this case, deque does not take ownership of the (committed) data item; we need to delete it.
    delete data;
  }
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::clearChanged()
{
  hasChanged_ = false;
}

namespace {
  void getPreference(DataStore *ds, ObjectId id, PlatformPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_platformPrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, BeamPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_beamPrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, GatePrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_gatePrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, LaserPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_laserPrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, LobGroupPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_lobGroupPrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, ProjectorPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_projectorPrefs(id, t);
  }

  void getPreference(DataStore *ds, ObjectId id, CustomRenderingPrefs** prefs, DataStore::Transaction* t)
  {
    *prefs = ds->mutable_customRenderingPrefs(id, t);
  }
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::update(DataStore *ds, ObjectId id, double time)
{
  clearChanged();

  if (updates_.empty() || (time < updates_.front()->time()))
  {
    reset_();
    return;
  }

  // process all command updates in one prefs transaction
  DataStore::Transaction t;
  PrefType* prefs = nullptr;
  simData::getPreference(ds, id, &prefs, &t);
  if (prefs == nullptr)
    return;

  const CommandType *lastCommand = current();
  if ((!lastCommand || time >= lastCommand->time()) && (earliestInsert_ > lastUpdateTime_))
  {
    // time moved forward: execute all commands from lastUpdateTime_ to new current time
    hasChanged_ = advance_(prefs, lastUpdateTime_, time);

    // Check for repeated scalars in the command, forcing complete replacement instead of add-value
    conditionalClearRepeatedFields_(prefs, &commandPrefsCache_);

    // apply the current command state at every update, even if no change in command state occurred with this update; commands override prefs settings
    prefs->MergeFrom(commandPrefsCache_);

    t.complete(&prefs);
  }
  else
  {
    // time moved backwards: reset and execute all commands from start to new current time
    // reset lastUpdateTime_
    reset_();

    // advance time forward, execute all commands from 0.0 (use -1.0 since we need a time before 0.0) to new current time
    advance_(prefs, -1.0, time);
    conditionalClearRepeatedFields_(prefs, &commandPrefsCache_);

    hasChanged_ = true;

    prefs->MergeFrom(commandPrefsCache_);
    t.complete(&prefs);
  }

  // reset to no inserted commands
  earliestInsert_ = std::numeric_limits<double>::max();
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::limitByTime(double timeWindow)
{
  if (timeWindow >= 0)
    MemorySliceHelper::limitByTime(updates_, lastTime() - timeWindow);
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::limitByPoints(uint32_t limitPoints)
{
  MemorySliceHelper::limitByPoints(updates_, limitPoints);
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::limitByPrefs(const CommonPrefs &prefs)
{
  limitByPoints(prefs.datalimitpoints());
  limitByTime(prefs.datalimittime());
}

template<class CommandType, class PrefType>
typename DataSlice<CommandType>::Iterator MemoryCommandSlice<CommandType, PrefType>::lower_bound(double timeValue) const
{
  VectorIterator<CommandType>* rv = new VectorIterator<CommandType>(&updates_);
  typename std::deque<CommandType*>::const_iterator iter = std::lower_bound(updates_.begin(), updates_.end(), timeValue, UpdateComp<CommandType>());
  rv->set(iter - updates_.begin());
  return typename DataSlice<CommandType>::Iterator(rv);
}

template<class CommandType, class PrefType>
typename DataSlice<CommandType>::Iterator MemoryCommandSlice<CommandType, PrefType>::upper_bound(double timeValue) const
{
  VectorIterator<CommandType>* rv = new VectorIterator<CommandType>(&updates_);
  typename std::deque<CommandType*>::const_iterator iter = std::upper_bound(updates_.begin(), updates_.end(), timeValue, UpdateComp<CommandType>());
  rv->set(iter - updates_.begin());
  return typename DataSlice<CommandType>::Iterator(rv);
}

template<class CommandType, class PrefType>
size_t MemoryCommandSlice<CommandType, PrefType>::numItems() const
{
  return updates_.size();
}

template<class CommandType, class PrefType>
double MemoryCommandSlice<CommandType, PrefType>::firstTime() const
{
  if (updates_.empty())
    return std::numeric_limits<double>::max();

  return (**updates_.begin()).time();
}

template<class CommandType, class PrefType>
double MemoryCommandSlice<CommandType, PrefType>::lastTime() const
{
  if (updates_.empty())
    return -std::numeric_limits<double>::max();

  return (**updates_.rbegin()).time();
}

template<class CommandType, class PrefType>
double MemoryCommandSlice<CommandType, PrefType>::deltaTime(double time) const
{
  return -1;
}

template<class CommandType, class PrefType>
bool MemoryCommandSlice<CommandType, PrefType>::advance_(PrefType* prefs, double startTime, double time)
{
  if (time < startTime)
    return false;

  // NOTE: this uses the request time as the upper bound, i.e. this finds the first value that is > than the requested time
  typename std::deque<CommandType*>::const_iterator i = std::upper_bound(updates_.begin(), updates_.end(), startTime, UpdateComp<CommandType>());
  typename std::deque<CommandType*>::const_iterator requested = std::upper_bound(updates_.begin(), updates_.end(), time, UpdateComp<CommandType>());

  bool prefsWereUpdated = false;
  for (; i != requested; ++i)
  {
    const CommandType* cmd = *i;
    if (cmd->has_updateprefs())
    {
      if (cmd->isclearcommand())
      {
        // clear the command (fields that are set in updateprefs) from both prefs and commandPrefsCache_
        clearCommand_(prefs, cmd->updateprefs());
      }
      else
      {
        // Check for repeated scalars in the command, forcing complete replacement instead of add-value
        conditionalClearRepeatedFields_(&commandPrefsCache_, &cmd->updateprefs());
        // execute the command
        commandPrefsCache_.MergeFrom(cmd->updateprefs());
      }
      // a command was executed, which may or may not be an actual change in prefs.
      prefsWereUpdated = true;
      lastUpdateTime_ = cmd->time();
    }
  }
  return prefsWereUpdated;
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::reset_()
{
  hasChanged_ = true;
  commandPrefsCache_.Clear();
  lastUpdateTime_ = -std::numeric_limits<double>::max();
  earliestInsert_ = std::numeric_limits<double>::max();
}

template<class CommandType, class PrefType>
bool MemoryCommandSlice<CommandType, PrefType>::hasRepeatedFields_(const PrefType* prefs) const
{
  return prefs->commonprefs().acceptprojectorids_size() != 0;
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::clearRepeatedFields_(PrefType* prefs) const
{
  prefs->mutable_commonprefs()->mutable_acceptprojectorids()->Clear();
}

template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::conditionalClearRepeatedFields_(PrefType* prefs, const PrefType* condition) const
{
  if (hasRepeatedFields_(condition))
    clearRepeatedFields_(prefs);
}

namespace {
/// Implementation of the Visitor interface that finds only fields that are set, adding them to the specified fieldList
class FindSetFieldsVisitor : public simData::protobuf::MessageVisitor::Visitor
{
public:
  FindSetFieldsVisitor(std::vector<std::string>& fieldList)
    : fieldList_(fieldList)
  {
  }

  /**
  * Visits the field 'descriptor' within the message 'message', and if the field is set, adds that field to the fieldList
  * @param message Message that contains the field descriptor.  Is a message definition only, and can be
  *  used to pull out siblings, reflection, and message/class names
  * @param descriptor Descriptor for the current field.  This is essentially a description of the current
  *  variable, and contains the variable name and class name for the field.
  * @param variableName Will contain the fully qualified name of the variable based on variable names (as
  *  opposed to class names).  This is different from descriptor.full_name(), which uses class names.  For
  *  example, variableName might be "commonPrefs.offset.x", while descriptor.full_name() might return
  *  something like "Position.x".  In other words, variableName is cognizant of scope.
  */
  virtual void visit(const google::protobuf::Message& message, const google::protobuf::FieldDescriptor& descriptor, const std::string& variableName)
  {
    const google::protobuf::Reflection& reflection = *message.GetReflection();
    if (descriptor.is_repeated())
    {
      if (reflection.FieldSize(message, &descriptor) > 0)
        fieldList_.push_back(variableName);
    }
    else if (reflection.HasField(message, &descriptor))
      fieldList_.push_back(variableName);
  }

private:
  std::vector<std::string>& fieldList_;
};
}


template<class CommandType, class PrefType>
void MemoryCommandSlice<CommandType, PrefType>::clearCommand_(PrefType* prefs, const PrefType& commandPref)
{
  std::vector<std::string> fieldList;
  FindSetFieldsVisitor findSetFieldsVisitor(fieldList);
  simData::protobuf::MessageVisitor::visit(commandPref, findSetFieldsVisitor);
  // locate the fields that are set in the commandPref, and clear the corresponding fields from the commandPrefsCache_
  for (std::vector<std::string>::const_iterator iter = fieldList.begin(); iter != fieldList.end(); ++iter)
  {
    // clear set field value(s) from the commandPrefsCache_
    simData::protobuf::clearField(commandPrefsCache_, *iter);
    simData::protobuf::clearField(*prefs, *iter);
  }
}

template<class CommandType, class PrefType>
typename DataSlice<CommandType>::IteratorImpl* MemoryCommandSlice<CommandType, PrefType>::iterator_() const
{
  return new VectorIterator<CommandType>(&updates_);
}


}

#endif
