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
#ifndef SIMDATA_MEMORYGENERICDATASLICE_H
#define SIMDATA_MEMORYGENERICDATASLICE_H

#include <deque>
#include <functional>
#include <string>

#include "simCore/Common/Common.h"
#include "simData/DataSlice.h"

namespace simData
{
/**
 * Memory Data Store's Generic Data slice implementation.  For the sake of simplicity and performance,
 * non-infinite generic data is not respected in MemoryGenericDataSlice.  Instead, the non-infinite
 * expiration time is converted to an infinite expiration time.
 *
 * The algorithm attempts to balance two opposite use cases.  The first use case is Generic Data with
 * repeating values and the second use case is Generic Data with non-repeating values.  The algorithm
 * places strings in a std::deque and uses indexes to represent the strings.   When a string is inserted,
 * the algorithm checks the std::deque to see if the string is a repeat.  If the string is a repeat,
 * the algorithm reuses the index.  If there is a lot of non-repeating values the std::deque could get
 * quite large and checking all the values could take a lot of time.  The algorithm kicks out after searching
 * the 5 nearest neighbors.  The early kick out has two advantages.  First, having many non-repeating values
 * does not drag the CPU down in a futile search for a match.   Second, it allows for data limiting when both
 * repeating and non-repeating values are intermingled.  After 5 non-repeating values are received the repeating
 * value will get a new index in the queue.  The older repeating value can be data limited out without adversely
 * affecting the indexes.   Without the kick out it would theoretically be possible to stall the data limiting
 * of the std::deque and have it grow without bound.
 */
class SDKDATA_EXPORT MemoryGenericDataSlice : public GenericDataSlice
{
public:
  MemoryGenericDataSlice();
  virtual ~MemoryGenericDataSlice();

  /// remove all data in the slice
  void flush();

  /// remove data for the given time range; up to but not including endTime
  void flush(double startTime, double endTime);

  /// apply the data limits indicated by 'prefs'
  void limitByPrefs(const CommonPrefs &prefs);

  /// Returns true if the generic data changes
  bool update(double time);

  /// Calling update can be expensive so instead install a time get function that can be called only when needed
  void setTimeGetter(const std::function<double()>& fn);

  /**
   * Insert data into the slice.
   * Note that normal DataSlice behavior is to take ownership of the data;
   * since that is not how we store data, this method will delete the pointer.
   * @param data User-generated (via "new") pointer to a GenericData to insert into this slice.
   * @param ignoreDuplicates If true, the data is not inserted if it has the same key/value as earlier.
   *   Even in this case, the data is deleted.
   */
  void insert(GenericData *data, bool ignoreDuplicates);

  /// Removes all the values associated with tag
  int removeTag(const std::string& tag);

  /// Returns true if the slice was modified during last DataStore::update
  virtual bool hasChanged() const;

  /// this feature is not implemented for MemoryGenericDataSlice
  virtual bool isDirty() const;

  /// Process update range
  virtual void visit(Visitor *visitor) const;

  /// @copydoc simData::VisitableDataSlice::modify
  virtual void modify(Modifier* modifier);

  /// Gets the active generic data at current time.
  virtual const GenericData* current() const;

  /// Retrieve total number of items in the data slice
  virtual size_t numItems() const;

private:
  /// Holds the data for individual generic data keys
  class Key;
  /// Collects data for the visitor pattern
  class Collector;

  // All the member variables are mutable so that the calculation of current_ can be delayed until the call to current()
  // 99% of the time no code calls current() after a call to update(), so don't calculate current_ until needed.

  /// Maintains the current state
  mutable GenericData current_;

  /// Used to detect changes requiring an update to current_
  mutable double lastTime_;

  /// If set used to grab the current scenario time from the data store
  std::function<double()> fn_;

  // All the generic data keyed by generic data key
  typedef std::map<std::string, Key*> GenericDataMap;
  mutable GenericDataMap genericData_;

  /// force a re-calculation of current_
  mutable bool force_;
};

}

#endif /* SIMDATA_MEMORYGENERICDATASLICE_H */
