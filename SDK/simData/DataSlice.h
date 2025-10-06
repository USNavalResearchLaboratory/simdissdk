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
#ifndef SIMDATA_DATASLICE_H
#define SIMDATA_DATASLICE_H

#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simData/GenericIterator.h"

namespace simData
{

/**
 * Interface to a list of updates used for drawing an object and its
 * history within the scene.
 *
 * Contains a reference to the current update and the range of updates for
 * the history trail.
 *
 * Visitor pattern is used for access to the range of updates.
 *
 * The list and its contents are immutable.
 *
 * Iterators point at positions between elements, not at elements.
 */
class SDKDATA_EXPORT DataSliceBase
{
public:
  virtual ~DataSliceBase() {}

  ///@return true if the slice was modified during last DataStore::update
  virtual bool hasChanged() const = 0;

  ///@return true if the slice has been modified since last DataStore::update
  virtual bool isDirty() const = 0;
};

/// Type-specific data slice with iterator and visitor pattern definitions
template<typename T>
class VisitableDataSlice : public DataSliceBase
{
public:
  /// Visitor for slice
  class Visitor
  {
  public:
    /// called by DataSlice::visit
    virtual void operator()(const T *update) = 0;

    virtual ~Visitor() {}
  };

  /// Visitor for modifying a slice
  class Modifier
  {
  public:
    /**
     * Called by DataSlice::modify
     * @param message  The message to modify
     * @returns 0=No change, 1=at least one field changed with no fields removed, -1=at least one field removed
     */
    virtual int modify(FieldList& message) = 0;

    virtual ~Modifier() {}
  };

public:
  /// Process update range
  virtual void visit(Visitor *visitor) const = 0;

  /// Modify the slice using the given modifier
  virtual void modify(Modifier *modifier) = 0;

}; // End of class VisitableDataSlice<T>

/// Type-specific slice with interpolation and iteration
template<typename T>
class DataSlice : public VisitableDataSlice<T>
{
public:
  /// Before and after values used to create the interpolated value
  typedef std::pair<const T*, const T*> Bounds;

  /// Typedef for the iterator implementation using the typename T
  typedef GenericIteratorImpl<const T*> IteratorImpl;

  /// Iterator into the slice for public use
  class Iterator : public GenericIterator<const T*>
  {
  public:
    /// Constructor for iterator on a data slice
    explicit Iterator(const DataSlice* slice)
      : GenericIterator<const T*>(slice->iterator_())
    {
    }
    /// Initializes from an IteratorImpl; Note: no clone here, ownership transfers to this instance via shared_ptr
    explicit Iterator(IteratorImpl* impl)
      : GenericIterator<const T*>(impl)
    {
    }
    /// Copy constructor uses clone
    Iterator(const Iterator& other)
      : GenericIterator<const T*>(other.impl_->clone())
    {
    }
  };

public:
  virtual ~DataSlice() {}

  /**
   * Returns an iterator into DataSlice, such that the iterator's next() value is
   * the first update at-or-after (>=) the time value.  Also, the previous() value
   * will be the last update before (<) the requested time value.
   *
   * For example, given
   * values [1, 3], lower_bound() will return the following:
   *  lower_bound(0): next == 1, previous == null
   *  lower_bound(1): next == 1, previous == null
   *  lower_bound(2): next == 3, previous == 1
   *  lower_bound(3): next == 3, previous == 1
   *  lower_bound(4): next == null, previous == 3
   *
   * @param timeValue Time value for which to seek a lower bound
   * @return Iterator pointing between elements such that next() is the first element
   *  at or after the timeValue.
   */
  virtual Iterator lower_bound(double timeValue) const = 0;

  /**
   * Returns an iterator into DataSlice, such that the iterator's previous() value is
   * the last update at-or-before (<=) the time value.  The next() value will be the
   * first value after (>) the requested time value.
   *
   * For example, given
   * values [1, 3], upper_bound() will return the following:
   *  upper_bound(0): next == 1, previous == null
   *  upper_bound(1): next == 3, previous == 1
   *  upper_bound(2): next == 3, previous == 1
   *  upper_bound(3): next == null, previous == 3
   *  upper_bound(4): next == null, previous == 3
   *
   * @param timeValue Time value for which to seek an upper bound
   * @return Iterator pointing between elements such that previous() is the first element
   *  at or before the timeValue.
   */
  virtual Iterator upper_bound(double timeValue) const = 0;

  /// Total number of items in this data slice
  virtual size_t numItems() const = 0;

  /**
   * If interpolation off, retrieves the most recent update whose timestamp is
   * less than or equal to datastore's current time
   * If interpolation on, retrieves an update whose time is the current datastore time,
   * and whose values are interpolated from existing updates
   */
  virtual const T* current() const = 0;

  /**
   * Determine if current update is an actual data value or if
   * it was interpolated from actual data values
   */
  virtual bool isInterpolated() const { return false; }

  /**
   * Retrieve the bounds used to compute the interpolated value
   * The bounds are represented as a std::pair containing const pointers
   * If the value is not interpolated, the values in the pair could be nullptr
   */
  virtual Bounds interpolationBounds() const { return Bounds(static_cast<T*>(nullptr), static_cast<T*>(nullptr)); }

  /// Earliest time in the update slice, or DBL_MAX if none
  virtual double firstTime() const = 0;
  /// Latest time in the update slice, or -DBL_MAX if none
  virtual double lastTime() const = 0;
  /// Returns the delta between the given time and the time of the data point before the given time; returns -1 if there is no previous point
  virtual double deltaTime(double time) const = 0;

protected:
  /// Helper function to return an iterator to first index
  virtual IteratorImpl* iterator_() const = 0;
}; // End of class DataSlice<T>

/** Defines the interface to access generic data */
class SDKDATA_EXPORT GenericDataSlice : public VisitableDataSlice<GenericData>
{
public:
  virtual ~GenericDataSlice() {}

  /// Total number of items in the entire data slice
  virtual size_t numItems() const = 0;
  /// Gets the active generic data at current time.
  virtual const GenericData* current() const = 0;
};

// Type definitions for platform, beam, gate, laser, projector, and lobGroup update lists

/// Slice of platform updates
typedef DataSlice<PlatformUpdate>  PlatformUpdateSlice;
/// Slice of Beam updates
typedef DataSlice<BeamUpdate>      BeamUpdateSlice;
/// Slice of Gate updates
typedef DataSlice<GateUpdate>      GateUpdateSlice;
/// Slice of Laser updates
typedef DataSlice<LaserUpdate>     LaserUpdateSlice;
/// Slice of Projector updates
typedef DataSlice<ProjectorUpdate> ProjectorUpdateSlice;
/// Slice of LOB Group updates
typedef DataSlice<LobGroupUpdate>  LobGroupUpdateSlice;

/// Slice of Platform commands
typedef DataSlice<PlatformCommand>  PlatformCommandSlice;
/// Slice of Beam commands
typedef DataSlice<BeamCommand>      BeamCommandSlice;
/// Slice of Gate commands
typedef DataSlice<GateCommand>      GateCommandSlice;
/// Slice of Laser commands
typedef DataSlice<LaserCommand>     LaserCommandSlice;
/// Slice of Projector commands
typedef DataSlice<ProjectorCommand> ProjectorCommandSlice;
/// Slice of LOB Group commands
typedef DataSlice<LobGroupCommand>  LobGroupCommandSlice;
/// Slice of Custom Rendering commands
typedef DataSlice<CustomRenderingCommand> CustomRenderingCommandSlice;

} // End of namespace simData

#endif // SIMDATA_DATASLICE_H

