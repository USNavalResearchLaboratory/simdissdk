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
#ifndef SIMVIS_LOCATOR_H
#define SIMVIS_LOCATOR_H

#include <limits>
#include "osg/Referenced"
#include "osg/Matrix"
#include "osg/Vec3"
#include "osgEarth/Revisioning"
#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"

/// Container for classes relating to visualization
namespace simVis
{

//----------------------------------------------------------------------------
/**
 * Callback to use when you want notification of locator changes.
 */
struct LocatorCallback : public osg::Referenced
{
  /**
  * Receive notification of a locator change.
  * @param locator Locator that changed
  */
  virtual void operator()(const class Locator* locator) = 0;

protected:
  /// osg::Referenced-derived
  virtual ~LocatorCallback() {}
};

/** Function pointer to a LocatorCallback method that takes a Locator parameter (e.g. LocatorCallback::operator()) */
typedef void(LocatorCallback::*LocatorCallbackMethodPtr)(const class Locator* locator);

/**
* Internal - convenience adapter that calls syncWithLocator() in response to
* a locator notification
*/
template<typename T>
struct SyncLocatorCallback : public LocatorCallback
{
  /** Constructor */
  SyncLocatorCallback(T *node) : node_(node) {}

  /** Calls syncWithLocator() on the node */
  void operator()(const class Locator* locator) { node_->syncWithLocator(); }

protected:
  /// osg::Referenced-derived
  virtual ~SyncLocatorCallback() {}

public: // data
  /** Node pointer */
  T* node_;
};

//----------------------------------------------------------------------------

/**
 * Generates a positional matrix for an object.
 */
class SDKVIS_EXPORT Locator : public osg::Referenced, public osgEarth::Util::Revisioned
{
public:
  /**
   * Component mask for inheriting partial Locator information.
   * Inheritance components specify the components that will be obtained from the locators in the locator graph to compose the inheriting locator.
   * For example, COMP_ALL specifies that all components of the inherited locators will be combined to compose the inheriting locator.
   * COMP_POSITION specifies that only the position components of the inherited locators will be combined to compose the inheriting locator.
   * All subsequent inheriting locators can specify inheritance components in the same way.
   */
  enum Components
  {
    COMP_NONE        = 0,
    COMP_POSITION    = 1 << 0,
    COMP_HEADING     = 1 << 1,
    COMP_PITCH       = 1 << 2,
    COMP_ROLL        = 1 << 3,
    COMP_ORIENTATION = COMP_HEADING | COMP_PITCH | COMP_ROLL,
    COMP_ALL         = COMP_POSITION | COMP_ORIENTATION,
  };

  /** Indicates whether rotation order is heading first or last */
  enum RotationOrder
  {
    HPR,
    RPH
  };

public:
  /**
  * Construct a locator.
  */
  Locator();

  /**
  * Construct a derived locator.
  * @param parent Locator from which to inherit components
  * @param compsToInherit Mask of components to inherit
  */
  explicit Locator(Locator* parent, unsigned int compsToInherit = COMP_ALL);

  /**
  * Notifies any listeners that properties of this Locator have changed.
  * Normally this happens automatically, but if you call any of the set*
  * methods with notify=false, you will need to call this when you are
  * finished. Doing do allows you to make multiple changes without multiple
  * notifications.
  */
  void endUpdate();

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
  * @deprecated Interface changed to support ECI coordinates. Use other interface for setCoordinate instead.
  * Sets the world position, orientation, and velocity vector all at once.
  * @param[in ] coord  World coordinates (must be ECEF, LLA, or ECI)
  * @param[in ] notify Whether to immediately notify listeners
  */
  SDK_DEPRECATE(void setCoordinate(const simCore::Coordinate& coord, bool notify = true), "Method will be removed in a future SDK release.");
#endif

  /**
  * Sets the world position, orientation, and velocity vector all at once. To support
  * conversion to/from an ECI coordinate, the ECI reference time must either be provided or
  * have already been set; otherwise a reference time of 0 will be used. The ECI reference
  * time can only be set by the top-level parent.
  * @param coord World coordinates (must be ECEF, LLA, or ECI)
  * @param timestamp Updates the locator with this timestamp
  * @param eciRefTime Reference time at which ECI and ECEF are equal
  * @param notify Whether to immediately notify listeners
  */
  void setCoordinate(const simCore::Coordinate& coord, double timestamp,
    double eciRefTime = std::numeric_limits<double>::max(), bool notify = true);

  /**
  * Sets the ECI rotation for this locator, using time as the measure of rotation.
  * @param[in ] rotationTime  time in seconds of earth rotation
  * @param[in ] timestamp Updates the locator with this timestamp/scenario time
  * @param[in ] notify Whether to immediately notify listeners
  */
  void setEciRotationTime(double rotationTime, double timestamp, bool notify = true);

  /**
   * Sets the local offset position and orientation of this locator, relative to a
   * world position in a parent locator.
   * @param[in ] pos Local offset coords, meters.
   * @param[in ] ori Local offset angles, radians.
   * @param[in ] timestamp Updates the locator with this timestamp
   * @param[in ] notify Whether to immediately notify listeners
   */
  void setLocalOffsets(const simCore::Vec3& pos, const simCore::Vec3& ori,
    double timestamp = std::numeric_limits<double>::max(), bool notify = true);

  /**
  * Gets the world coordinate that was set by a setCoordinate() operation for this locator
  *  or else for the nearest parent that had its coordinate set via setCoordinate()
  * @param[out] out_coord Result goes here upon success
  * @param[in ] coordsys Coordinate system of the output value
  * @return True upon success.
  */
  bool getCoordinate(simCore::Coordinate* out_coord, const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;

  /**
   * Gets the local offset coordinate (offset from parent locator)
   * @param[out] out_pos Will contain the local offset position in XYZ after this call
   * @param[out] out_ori will contain the local YPR orientation after this call
   * @return True upon success.
   */
  bool getLocalOffsets(simCore::Vec3& out_pos, simCore::Vec3& out_ori) const;

  /**
   * Convenience; returns a reference to the absolute coordinate in its default
   * internal representation
   */
  const simCore::Coordinate& getCoordinate() const { return ecefCoord_; }

  /**
  * Gets a positioning matrix that combines aggregate rotation, position, local orientation, and
  * offset position.
  */
  bool getLocatorMatrix(osg::Matrixd& output_mat, unsigned int components = COMP_ALL) const;

  /**
  * Gets a positioning matrix that combines aggregate rotation, position, local orientation, and
  * offset position.
  */
  osg::Matrixd getLocatorMatrix(unsigned int components = COMP_ALL) const;

  /**
  * Gets the world position reflected by this Locator. This is just a convenience
  * function that extracts the Position information (not rotation) from the
  * locator matrix.
  *
  * @param[out] out_position If not NULL, resulting position stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return True if the output parameter is populated successfully
  */
  virtual bool getLocatorPosition(simCore::Vec3* out_position,
    const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Gets the world position reflected by this Locator. This is just a convenience
  * function that extracts the Position information and rotation from the
  * locator matrix.
  *
  * @param[out] out_position If not NULL, resulting position stored here
  * @param[out] out_orientation If not NULL, resulting orientation stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return True if the output parameter is populated successfully
  */
  virtual bool getLocatorPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
    const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Set timestamp associated with the locator. If converting to or from ECI, the
  * timestamp's offset from the ECI reference time (see setEciRefTime()) will be
  * used as the elapsed ECI time.
  * @param timestamp time value to set; can be negative, zero or positive.
  * @param[in ] notify Whether to immediately notify listeners
  */
  void setTime(double timestamp, bool notify = true);

  /**
   * Set the ECI reference time for a locator.
   * The ECI reference time is subtracted from the locator timestamp to find the elapsed ECI time.
   * It is expected that only one locator in an inheritance chain will specify an ECI reference time;
   * and expected that that value applies to all locators in the chain.
   * Nevertheless, any locator can set an ECI reference time.
   * The ECI reference time for a locator is the first non-default value set for itself or by its parents.
   * @param eciRefTime ECI reference time.
   * @return True.
   */
  bool setEciRefTime(double eciRefTime);

  /**
   * Returns the most recent timestamp on this locator or its parents.
   */
  double getTime() const;

  /**
   * Returns the ECI reference time for this locator.
   * If not set by this locator, it will be retrieved from next parent locator that has a non-default value.
   * @return the ECI reference time if found, 0. if the locator and all parents have the default/not-set value.
   */
  double getEciRefTime() const;

  /**
   * Returns the elapsed ECI time for this locator.
   * The elapsed ECI time of a locator is the difference of :
   * the most recent timestamp of the locator that provides that ECI Reference time for this locator,
   * and the ECI Reference time of this locator (see getEciRefTime()).
   * If no timestamp has been set prior to calling this method, the time returned will be 0.
   */
  double getElapsedEciTime() const;

  /**
  * Set locator for this to follow in some way
  *
  * The optional parent locator.
  * If a Locator has a parent, it inherits rotation, position and orientation from that
  * parent as prescribed by the Components flags. Otherwise, the Locator is absolute.
  * @param parent Parent locator to set
  * @param componentsToInherit Mask of components to inherit from parent
  * @param notify If true, notifies when the compMask changes
  */
  void setParentLocator(Locator* parent, unsigned int componentsToInherit = COMP_ALL, bool notify = true);

  /**
  * Get the parent locator.
  * @return Locator, or NULL if no parent exists
  */
  Locator* getParentLocator() { return parentLoc_.get(); }

  /**
  * Get the parent locator (tail const version)
  * @return Locator, or NULL if no parent exists
  */
  const Locator* getParentLocator() const { return parentLoc_.get(); }

  /**
  * Set the policy for inheriting parent locator's components
  * @param compMask Mask of components to inherit from parent
  * @param notify If true, notifies when the compMask changes
  */
  void setComponentsToInherit(unsigned int compMask, bool notify = true);

  /**
  * Get the components to inherit
  * @return Mask of components to inherit from parent locator
  */
  unsigned int getComponentsToInherit() const { return componentsToInherit_; }

  /**
  * Whether the Locator or any of its parents contains a valid position, orientation or rotation.
  * @return true if the locator chain has position, orientation or rotation, else false
  */
  bool isEmpty() const;

  /**
  * Whether the Locator or any of its parents contains a valid position, orientation or rotation.
  * @return true if the locator chain has position, orientation or rotation, else false
  */
  bool isValid() const { return !isEmpty(); }

  /**
  * Whether the Locator supports ECI positioning.
  * @return true if the locator supports ECI positioning, else false
  */
  bool isEci() const;

  /**
  * Adds a callback to this locator.
  * @param callback Callback to add.
  */
  void addCallback(LocatorCallback* callback);

  /**
   * Removes a callback to this locator.
   * @param[in ] callback Callback to remove.
   */
  void removeCallback(LocatorCallback* callback);

 /**
  * Gets the total ECI rotation time for this locator (including parents), where time is the measure of earth rotation.
  * @return  time in seconds of earth rotation
  */
  double getEciRotationTime() const;


protected:
  /// osg::Referenced-derived
  virtual ~Locator() {}

  /**
  * Returns the base position of this locator after specified inheritance components are applied to it
  * @param[out] pos vector containing the position information
  * @param[in ] comps inheritance components to use
  * @return true if there is a non-trivial position to return
  */
  virtual bool getPosition_(osg::Vec3d& pos, unsigned int comps) const;

  /**
  * Returns the base orientation of this locator after specified inheritance components are applied to it
  * @param[out] ori matrix containing the orientation information
  * @param[in ] comps inheritance components to use
  * @return true if there is a non-trivial orientation to return
  */
  virtual bool getOrientation_(osg::Matrixd& ori, unsigned int comps) const;

  /**
  * Returns the input locator matrix with all local offsets (including those of parents) applied, as filtered by the specified inheritance components
  * @param[in,out] output matrix containing the new locator matrix with offsets applied
  * @param[in    ] comps inheritance components to use
  */
  virtual void applyOffsets_(osg::Matrixd& output, unsigned int comps) const;

  /**
  * Returns the input locator matrix with only this locator's local offsets applied, as filtered by the specified inheritance components
  * @param[in,out] output matrix containing the new locator matrix with offsets applied
  * @param[in    ] comps inheritance components to use
  */
  void applyLocalOffsets_(osg::Matrixd& output, unsigned int comps) const;

private:
  /**
  * Notifies all children and callbacks of a change to this locator
  */
  void notifyListeners_();

#ifdef USE_DEPRECATED_SIMDISSDK_API
  SDK_DEPRECATE(bool inherits_(unsigned int mask) const, "Method will be removed in future SDK release.");
#endif

  /**
  * Returns the base rotation of this locator
  * @param[out] rotation matrix with rotation
  * @return true if there is a non-trivial rotation returned
  */
  bool getRotation_(osg::Matrixd& rotation) const;

  /**
  * Returns an ENU local tangent plane at the specified position
  * simCore equivalent of osg::computeLocalToWorldTransformFromXYZ()
  * using simCore methods avoids dependency on SRS, and uses a more accurate ecef->lla conversion
  * @param ecefPos specified position
  * @param local2world ENU matrix at specified position
  */
  void computeLocalToWorldTransformFromXYZ_(const osg::Vec3d& ecefPos, osg::Matrixd& local2world) const;

  osg::observer_ptr<Locator> parentLoc_;
  unsigned int componentsToInherit_; // Locator::Components mask
  RotationOrder rotOrder_;
  std::set< osg::observer_ptr<Locator> > children_;
  std::vector< osg::ref_ptr<LocatorCallback> > callbacks_;
  simCore::Coordinate ecefCoord_; ///< the base position & orientation of this locator, possibly unset
  simCore::Vec3 offsetPos_; ///< the local position offset of this locator, possibly unset
  simCore::Vec3 offsetOri_; ///< the local orientation offset of this locator, possibly unset
  bool isEmpty_;            ///< if false, this locator has some data, though possibly only a timestamp
  bool ecefCoordIsSet_;     ///< indicates if this locator has position and/or orientation
  bool hasRotation_;        ///< indicates if this locator has a rotation
  bool offsetsAreSet_;      ///< indicates if this locator has local offsets
  double timestamp_;        ///< the most recent sim time when this locator was updated
  double eciRefTime_;       ///< the rotation offset for ECI/ECEF conversion
  double eciRotationTime_;  ///< the local earth rotation time offset specified for this locator
};

/**
* CachingLocator is a Locator class that caches lla position and orientation to improve performance for locators that often are requested to provide lla information
*/
class SDKVIS_EXPORT CachingLocator : public Locator
{
public:
  /** Constructor; @see Locator() */
  CachingLocator();
  /** Constructor; @see Locator(Locator*, unsigned int) */
  explicit CachingLocator(Locator* parentLoc, unsigned int inheritMask = COMP_ALL);

  /**
  * Gets the world position reflected by this Locator. This is just a convenience
  * function that extracts the Position information (not rotation) from the
  * locator matrix.
  *
  * @param[out] out_position If not NULL, resulting position stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return True if the output parameter is populated successfully
  */
  virtual bool getLocatorPosition(simCore::Vec3* out_position,
    const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Gets the world position reflected by this Locator. This is just a convenience
  * function that extracts the Position information and rotation from the
  * locator matrix.
  *
  * @param[out] out_position If not NULL, resulting position stored here
  * @param[out] out_orientation If not NULL, resulting orientation stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return True if the output parameter is populated successfully
  */
  virtual bool getLocatorPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
    const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;
protected:
  /// osg::Referenced-derived
  virtual ~CachingLocator() {}

private:
  // cache frequently used LLA position and orientation
  mutable simCore::Vec3 llaPositionCache_;
  mutable osgEarth::Util::Revision llaPositionCacheRevision_;
  mutable simCore::Vec3 llaOrientationCache_;
  mutable osgEarth::Util::Revision llaOrientationCacheRevision_;
};

/**
* ResolvedPositionOrientationLocator is a locator that generates a position-with-(base)-orientation from its parents,
* based on the specified inheritance components, but which is treated thereafter (for subsequent inheritance) as a base coordinate position.
*
* Base coordinate orientation information is not stripped away as in COMP_RESOLVED_POSITION; it is maintained and available for inheritance.
* But all orientation offsets are stripped away - only the orientation set in the base coordinate (as set by setCoordinate()) is maintained.
*
* Though it can apply its own offsets and be used like a normal locator, its primary function is to provide this resolved coordinate to inheriting locators.
* Subsequent locators that inherit the resolving locator get the same "resolved position with orientation" regardless of the orientation components they inherit.
* The inherited position matrix will represent a local tangent plane at the final position.
*/
class SDKVIS_EXPORT ResolvedPositionOrientationLocator : public Locator
{
public:
  /** Constructor; @see Locator() */
  ResolvedPositionOrientationLocator();
  /** Constructor; @see Locator(Locator*, unsigned int) */
  ResolvedPositionOrientationLocator(Locator* parentLoc, unsigned int inheritMask);
protected:
  /// osg::Referenced-derived
  virtual ~ResolvedPositionOrientationLocator() {}
private:
  /** @copydoc Locator::getPosition_() */
  virtual bool getPosition_(osg::Vec3d& pos, unsigned int comps) const;
  /** @copydoc Locator::applyOffsets_() */
  virtual void applyOffsets_(osg::Matrixd& output, unsigned int comps) const;
};

/**
* ResolvedPositionLocator is a locator that generates a position-with-identity-orientation from its parents,
* based on the specified inheritance components, but which is treated thereafter (for subsequent inheritance) as a base coordinate position.
* Though it can apply its own offsets and be used like a normal locator, its primary function is to provide this resolved position to inheriting locators.
* Subsequent locators that inherit the resolving locator get the same position regardless of the orientation components they inherit.
* The inherited position matrix will represent a local tangent plane at the final position.
*/
class SDKVIS_EXPORT ResolvedPositionLocator : public ResolvedPositionOrientationLocator
{
public:
  /** Constructor; @see Locator() */
  ResolvedPositionLocator();
  /** Constructor; @see Locator(Locator*, unsigned int) */
  ResolvedPositionLocator(Locator* parentLoc, unsigned int inheritMask);
protected:
  /// osg::Referenced-derived
  virtual ~ResolvedPositionLocator() {}
private:
  /** @copydoc Locator::getOrientation_() */
  virtual bool getOrientation_(osg::Matrixd& ori, unsigned int comps) const;
};
}

#endif // SIMVIS_LOCATOR_H
