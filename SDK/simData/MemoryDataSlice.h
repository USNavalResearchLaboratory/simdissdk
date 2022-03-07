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
#ifndef SIMDATA_MEMORYDATASLICE_H
#define SIMDATA_MEMORYDATASLICE_H

#include <deque>
#include "simData/DataTypes.h"
#include "simData/DataSlice.h"
#include "simData/DataSliceUpdaters.h"
#include "simData/Interpolator.h"
#include "simData/ObjectId.h"
#include "simData/UpdateComp.h"

namespace simData
{
class DataStore;

namespace MemorySliceHelper
{

/// a deque iterator that does not fault
template<typename T>
class SafeDequeIterator
{
public:
  SafeDequeIterator();

  /// construct from a given container and current position
  SafeDequeIterator(typename std::deque<T>* deque, typename std::deque<T>::iterator i);

  /// make this point to something invalid
  void invalidate();

  ///@return an iterator representing the current offset
  typename std::deque<T>::iterator get() const;

private:
  typename std::deque<T>* deque_;
  typename std::deque<T>::difference_type val_;
};


/**
 * Reduce the data store to only have points within the given 'timeWindow'
 * @param updates Deque of updates on which to apply data limit
 * @param timeLimit earliest time to keep
 * @return 0 if at least one item is removed.
 */
template<typename T>
int limitByTime(std::deque<T*> &updates, double timeLimit);

/**
 * Reduce the data store to only have 'limitPoints' points
 * @param updates Deque of updates on which to apply data limit
 * @param limitPoints number of points to keep (0 is no limit)
 * @return 0 if at least one item is removed.
 */
template<typename T>
int limitByPoints(std::deque<T*> &updates, uint32_t limitPoints);

/// remove all points, unless keeping a static (time = -1) point; returns non-zero if flush did not occur due to static case
template<typename T>
int flush(std::deque<T*> &updates, bool keepStatic = true);

/// remove points in the given time range; up to but not including endTime
template<typename T>
int flush(std::deque<T*> &updates, double startTime, double endTime);
} // namespace MemorySliceHelper

/** Iterator for DataSlice vector */
template <class T>
class VectorIterator : public DataSlice<T>::IteratorImpl
{
public:
  /**
   * VectorIterator Constructor
   * @param vec deque to iterate through
   */
  VectorIterator(const std::deque<T *>* vec);

  /** Retrieves next item and increments iterator to next element */
  virtual const T* const next();
  /** Retrieves next item and does not increment iterator to next element */
  virtual const T* const peekNext() const;
  /** Retrieves previous item and increments iterator to next element */
  virtual const T* const previous();
  /** Retrieves previous item and does not increment iterator to next element */
  virtual const T* const peekPrevious() const;

  /** Resets the iterator to the front of the data structure */
  virtual void toFront();
  /** Sets the iterator to the end of the data structure */
  virtual void toBack();

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  virtual bool hasNext() const;
  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  virtual bool hasPrevious() const;

  /** Create a copy of the iterator */
  virtual typename DataSlice<T>::IteratorImpl* clone() const;

  /** Access to the pointer to data, for use by implementers */
  void set(size_t idx);

private:
  const std::deque<T *>* vec_;
  size_t nextIndex_;
};

/// Generic Implementation of the DataSlice types for the MemoryDataStore
/// Assumes ownership of all data it contains, deleting it in the destructor
template<typename T>
class MemoryDataSlice : public DataSlice<T>
{
public:
  MemoryDataSlice();
  virtual ~MemoryDataSlice();

  /// remove all data in the slice
  virtual void flush(bool keepStatic = true);

  /// remove points in the given time range; up to but not including endTime
  virtual void flush(double startTime, double endTime);

  /**
   * Returns an iterator pointing to the first update whose timestamp is
   * greater than or equal to (>=) the timeValue
   * @param timeValue
   * @return Iterator
   */
  virtual typename DataSlice<T>::Iterator lower_bound(double timeValue) const;

  /**
   * Returns an iterator pointing to the last update whose timestamp is
   * less than (<) the timeValue
   * @param timeValue
   * @return Iterator
   */
  virtual typename DataSlice<T>::Iterator upper_bound(double timeValue) const;

  /**
   * Total number of items in this data slice
   * @return size_t number of items
   */
  virtual size_t numItems() const;

  /**
   *modified during last DataStore::update
   * @return bool
   */
  virtual bool hasChanged() const;

  ///@return true if the slice has been modified since last DataStore::update
  virtual bool isDirty() const;

  /**
   * Retrieve the current update
   * @return data update
   */
  virtual const T *current() const;

  /// Process update range
  virtual void visit(typename DataSlice<T>::Visitor *visitor) const;

  /// @copydoc simData::VisitableDataSlice::modify
  virtual void modify(typename DataSlice<T>::Modifier *modifier);

  /**
   * Determine if current update is an actual data value or if
   * it was interpolated from actual data values
   */
  virtual bool isInterpolated() const;

  /**
   * Retrieve the bounds used to compute the interpolated value
   * The bounds are represented as a std::pair containing const pointers
   * If the value is not interpolated, the values in the pair could be NULL
   */
  virtual typename DataSlice<T>::Bounds interpolationBounds() const;

  // these next two functions should not be public
  // it would be nice if these two functions could have access limited to this class and MemoryDataStore
  /**
  * Clear the marker that indicates if the "current" update contains new data
  */
  void clearChanged();

  /**
  * Set the marker that indicates if the "current" update contains new data
  */
  void setChanged();

  /**
   * set current data slice
   * @param current the current data slice
   */
  void setCurrent(T* current);

  /**
   * set interpolated flag and bounds
   * @param interpolated
   * @param bounds
   */
  void setInterpolated(bool interpolated, const typename DataSlice<T>::Bounds& bounds);

  /**
   * Perform a time update, finding the state data whose time is matches or is the lower bound of the specified time
   * @param time
   */
  virtual void update(double time);

  /**
   * Perform a time update, finding the state data whose time is an exact match for the time specified, or interpolating
   * to compute state data for the time specified from its bounding points
   * @param time
   * @param interpolator
   */
  void update(double time, Interpolator *interpolator);

  /**
   * Insert the specified data within the MemoryDataSlice in time-based sorted order
   * @param data
   */
  virtual void insert(T *data);

  /// reduce the data store to only have points within the given 'timeWindow'
  /// @param timeWindow amount of time to keep in window (negative for no limit)
  void limitByTime(double timeWindow);

  /// reduce the data store to only have 'limitPoints' points
  /// @param limitPoints number of points to keep (0 is no limit)
  void limitByPoints(uint32_t limitPoints);

  /** Performs both point and time limiting based on the settings in prefs */
  virtual void limitByPrefs(const CommonPrefs &prefs);

  /** Retrieves the earliest time stored in this slice */
  virtual double firstTime() const;

  /** Retrieves the latest time stored in this slice */
  virtual double lastTime() const;

  /** The time delta between the given time and the data point before the given time; return -1 if no previous point */
  virtual double deltaTime(double time) const;

  /** Retrieves the current interpolated T, or nullptr if none */
  T* currentInterpolated();

protected:
  /// Helper function to return an iterator to first index
  virtual typename DataSlice<T>::IteratorImpl* iterator_() const;

protected:
  /// used to mark if time update or changes to the slice have resulted in a change to the current update
  bool mdsHasChanged_;
  /// used to mark if this slice needs to be updated (i.e. the updates_ have been modified)
  bool dirty_;
  /// list of state updates
  std::deque<T*> updates_;
  /// the current state, can either point to a real state, or a "virtual" interpolated state
  T *current_;
  /// a cache of the interpolated state for the current time
  T currentInterpolated_;
  /// specifies if the interpolated cache value is valid
  bool interpolated_;
  /// specifies the interpolation bounds; the bounds will be NULL if no interpolation is specified
  typename DataSlice<T>::Bounds bounds_;
  /// Used to optimize updates by looking at data near the last update
  typename MemorySliceHelper::SafeDequeIterator<T*> fastUpdate_;
};

//----------------------------------------------------------------------------
/// Implementation of the DataSlice types for the MemoryDataStore
/// Cache entries are std::strings
/// Implements an update slice for sparse data sets of Commands
/// Assumes ownership of all data it contains, deleting it in the destructor
template<class CommandType, class PrefType>
class MemoryCommandSlice : public DataSlice<CommandType>
{
public:
  MemoryCommandSlice();
  virtual ~MemoryCommandSlice();

  //--- from DataSlice
  /**
   * Retrieve the current update
   * @return CommandType
   */
  virtual const CommandType* current() const;

  /**
   * Process update range
   * @param visitor
   */
  virtual void visit(typename DataSlice<CommandType>::Visitor *visitor) const;

  /// @copydoc simData::VisitableDataSlice::modify
  virtual void modify(typename DataSlice<CommandType>::Modifier *modifier);

  /// remove all data in the slice
  void flush();

  /// remove points in the given time range; up to but not including endTime
  void flush(double startTime, double endTime);

  //--- from DataSliceBase
  /**
   * modified during last DataStore::update
   * @return bool
   */
  virtual bool hasChanged() const;

  /// this feature is not implemented for MemoryCommandSlice
  virtual bool isDirty() const;

  //--- effectively required interface
  /**
   * Insert the specified data within the MemoryDataSlice in time-based sorted order
   * Ownership of the data item is transferred (from the transaction) to the datastore
   * @param data update
   */
  void insert(CommandType *data);

  //--- interface
  /**
   * Clear the marker that indicates if the "current" pointer has changed.
   */
  void clearChanged();

  /**
   * Perform a time update on the state data with the specified id
   * in the DataStore
   * @param ds DataStore reference
   * @param id DataStore id
   * @param time
   */
  virtual void update(DataStore *ds, ObjectId id, double time);

  /// reduce the data store to only have points within the given 'timeWindow'
  /// @param timeWindow amount of time to keep in window (negative for no limit)
  void limitByTime(double timeWindow);

  /// reduce the data store to only have 'limitPoints' points
  /// @param limitPoints number of points to keep (0 is no limit)
  void limitByPoints(uint32_t limitPoints);

  /** Performs both point and time limiting based on the settings in prefs */
  virtual void limitByPrefs(const CommonPrefs &prefs);

  /**
   * Returns the first iterator at or after the time value
   * @param timeValue
   * @return Iterator
   */
  virtual typename DataSlice<CommandType>::Iterator lower_bound(double timeValue) const;

  /**
   * Returns the first iterator after the time value
   * @param timeValue
   * @return data slice iterator
   */
  virtual typename DataSlice<CommandType>::Iterator upper_bound(double timeValue) const;

  /**
   * Total number of items in this data slice
   * @return size_t number of items
   */
  virtual size_t numItems() const;

  /**
   * Get first time value in data slice
   * @return double first time
   */
  virtual double firstTime() const;

  /**
   * Get last time value in data slice
   * @return double last time
   */
  virtual double lastTime() const;

  /// Not Implemented; always returns -1;
  virtual double deltaTime(double time) const;

protected: // methods
  /**
   * Move "current" to specified time.
   * Since there is a sparse representation (i.e. key:value pairs are only updated when changing, they are not updated every time step)
   *   this accumulates entries from the last "current" until the specified time.
   * @param startTime the time AFTER for which to execute commands
   * @param time current time
   * @return True if a prefs was updated
   */
  bool advance_(double startTime, double time);

  /// Set values to default
  void reset_();

  /**
   * Repeated fields for command processing have unique requirements with respect to updating.
   * If a repeated field of the update command has values, then the current preferences
   * should be updated.  If the repeated field of the update command is empty, the current
   * preferences should not be updated.   The routines below implement the unique
   * requirements.  Currently the only repeated field that is part of command processing
   * is acceptprojectorids().  If a new repeated field command is added then the routines
   * hasRepeatedFields_() and clearRepeatedFields_() require changes.
   */
  /// Returns true if any repeated field has at least one value
  bool hasRepeatedFields_(const PrefType* prefs) const;
  /// Clears all repeated fields
  void clearRepeatedFields_(PrefType* prefs) const;
  /// Clears the repeated fields in pref if the corresponding repeated field in condition has at least one value
  void conditionalClearRepeatedFields_(PrefType* prefs, const PrefType* condition) const;

  /**
  * Clear a command from the command cache
  * The affected preference fields in the commandPrefsCache_ will be clear()'ed.
  * @param commandPref a prefs message in which the fields that are set represent the command that is to be cleared
  */
  void clearCommand_(const PrefType& commandPref);

  /// Helper function to return an iterator to first index
  virtual typename DataSlice<CommandType>::IteratorImpl* iterator_() const;

protected: // data
  /// list of state updates
  std::deque<CommandType*> updates_;
  /// caches the current command pref state
  PrefType commandPrefsCache_;
  /// Cached value of last update() time
  double lastUpdateTime_;
  /// Flags changes
  bool hasChanged_;
  /// Keeps track of the earliest command time insert since the last update(), to efficiently process command updates
  double earliestInsert_;
};

/**
 * Beam Specific Implementation of the MemoryCommandSlice types for the MemoryDataStore
 * Resets beams to default command state when time moves backward.
 * Processes all command updates in a single prefs transaction.
 */
class BeamMemoryCommandSlice : public MemoryCommandSlice<BeamCommand, BeamPrefs>
{
public:
  /**
  * Perform a time update on the state data with the specified id
  * in the DataStore by calling MemoryCommandSlice update
  * @param ds DataStore reference
  * @param id DataStore id
  * @param time the time up to (and including) which to execute commands
  */
  virtual void update(DataStore *ds, ObjectId id, double time);
};

/**
* Gate-specific implementation of the MemoryCommandSlice types for the MemoryDataStore
* Resets gates to default command state when time moves backward.
* Processes all command updates in a single prefs transaction.
*/
class GateMemoryCommandSlice : public MemoryCommandSlice<GateCommand, GatePrefs>
{
public:
  /**
  * Perform a time update on the state data with the specified id
  * in the DataStore by calling MemoryCommandSlice update
  * @param ds DataStore reference
  * @param id DataStore id
  * @param time the time up to (and including) which to execute commands
  */
  virtual void update(DataStore *ds, ObjectId id, double time);
};

/**
 * LobGroup Specific Implementation of the MemoryDataSlice used by the MemoryDataStore
 * Assumes ownership of all data it contains, deleting it in the destructor
 * This implements the handling of data that is specific to the LobGroup, as
 * the current data slice can have multiple points based on the LobGroup settings
 * NOTE: the maxDataPoints_ and maxDataSeconds_ are updated whenever prefs are updated
 * since any prefs transaction results in a datastore update
*/
class LobGroupMemoryDataSlice : public MemoryDataSlice<LobGroupUpdate>
{
public:
  LobGroupMemoryDataSlice();
  virtual ~LobGroupMemoryDataSlice();

  /**
  * Overrides the MemoryDataSlice update, to set the current data slice
  * to have all the data points that fit within the maxDataPoints and
  * maxDataSeconds values, based on the new current time passed in
  * @param time the new current time
  */
  virtual void update(double time);

  /**
  * Overrides the MemoryDataSlice method.  Since LobGroup can have multiple data points
  * at the same time, merges the LobGroupUpdatePoints into the LobGroupUpdate record with
  * the same time.  Ensures that all points in the data record have the same time as data
  * @param data the new update data
  */
  virtual void insert(LobGroupUpdate *data);

  /// remove all data in the slice
  virtual void flush(bool keepStatic = true);

  /// remove points in the given time range; up to but not including endTime
  virtual void flush(double startTime, double endTime);

  /**
  * Set the maximum number of data points times for current data slice, recalculates the current data slice if changed
  * matches the value in the LobGroupPrefs
  * @param maxDataPoints
  */
  void setMaxDataPoints(size_t maxDataPoints);

  /**
  * Set the max age for data points in the current data slice, recalculates the current data slice if changed
  * matches the value in the LobGroupPrefs
  * @param maxDataSeconds
  */
  void setMaxDataSeconds(double maxDataSeconds);

private:
  /// defines the max number of data points in the data slice
  size_t maxDataPoints_;
  /// defines the max age for data points in the data slice
  double  maxDataSeconds_;
  /// cache last update time
  double currentTime_;
};

} // End of namespace simData

// implementation of inline functions
#include "simData/MemoryDataSlice-inl.h"

#endif // SIMDATA_MEMORYDATASLICE_H

