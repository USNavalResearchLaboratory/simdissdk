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
#ifndef SIMVIS_LOCATOR_H
#define SIMVIS_LOCATOR_H

#include <limits>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "osg/MatrixTransform"
#include "osgEarth/Revisioning"
#include "osgEarth/SpatialReference"

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
class SDKVIS_EXPORT Locator : public osg::Referenced,
  public osgEarth::Revisioned
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
  * @param mapSRS Map spatial reference system under which this locator operates
  */
  Locator(const osgEarth::SpatialReference* mapSRS);

  /**
  * Construct a derived locator.
  * @param parent Locator from which to inherit components
  * @param compsToInherit Mask of components to inherit
  */
  Locator(Locator* parent, unsigned int compsToInherit = COMP_ALL);

  /**
   * @deprecated No longer has any function. Rotation order is always HPR.
   */
  void setRotationOrder(const RotationOrder& order, bool notify = true);

  /**
  * Notifies any listeners that properties of this Locator have changed.
  * Normally this happens automatically, but if you call any of the set*
  * methods with notify=false, you will need to call this when you are
  * finished. Doing do allows you to make multiple changes without multiple
  * notifications.
  */
  void endUpdate();

  /**
  * @deprecated Interface changed to support ECI coordinates. Use other interface for setCoordinate instead.
  * Sets the world position, orientation, and velocity vector all at once.
  * @param[in ] coord  World coordinates (must be ECEF, LLA, or ECI)
  * @param[in ] notify Whether to immediately notify listeners
  */
  void setCoordinate(const simCore::Coordinate& coord, bool notify = true);

  /**
  * Sets the world position, orientation, and velocity vector all at once. To support
  * conversion to/from an ECI coordinate, the ECI reference time must either be provided or
  * have already been set; otherwise a reference time of 0 will be used. The ECI reference
  * time can only be set by the top-level parent.
  * @param[in ] coord World coordinates (must be ECEF, LLA, or ECI)
  * @param[in ] timestamp Updates the locator with this timestamp
  * @param[in ] eciRefTime Reference time at which ECI and ECEF are equal
  * @param[in ] notify Whether to immediately notify listeners
  */
  void setCoordinate(const simCore::Coordinate& coord, double timestamp,
    double eciRefTime = std::numeric_limits<double>::max(), bool notify = true);

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
   * @deprecated No longer has any function.
   */
  void resetToLocalTangentPlane(bool notify = true);

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
  * Gets a positioning matrix that combines aggregate position, local orientation, and
  * offset position.
  */
  bool getLocatorMatrix(osg::Matrixd& output_mat, unsigned int components = COMP_ALL) const;

  /**
  * Gets the world position reflected by this Locator. This is just a convenience
  * function that extracts the Position information (not rotation) from the
  * locator matrix.
  *
  * @param[out] out_position If not NULL, resulting position stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return True if the output parameter is populated successfully
  */
  bool getLocatorPosition(simCore::Vec3* out_position,
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
  bool getLocatorPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
    const simCore::CoordinateSystem& coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Gets the matrix that xforms from the local ENU tangent plane to world coordinates.
  * (This returns the same value as getPositionMatrix)
  * @param out_matrix Result goes here upon success
  * @return True upon success, false on failure
  */
  virtual bool getLocalTangentPlaneToWorldMatrix(osg::Matrixd& out_matrix) const;

  /**
  * Set timestamp associated with the locator. If converting to or from ECI, the
  * timestamp's offset from the ECI reference time (see setEciRefTime()) will be
  * used as the elapsed ECI time.
  */
  void setTime(double timestamp, bool notify = true);

  /**
   * Set the ECI reference time for the chain of locators. All locators use the same reference time.
   * Will only be set if called by the top-level parent.
   * @param[in] eciRefTime ECI reference time which is subtracted from the locator timestamp to find the elapsed ECI time
   * @return True if the ECI reference time is set properly.
   */
  bool setEciRefTime(double eciRefTime);

  /**
   * Returns the most recent timestamp on this locator or its parents.
   */
  double getTime() const;

  /** Returns the ECI reference time for this locator (is the same for all locators in the chain). */
  double getEciRefTime() const;

  /**
   * Returns the elapsed ECI time for this locator. If no timestamp has been set prior to calling
   * this method, the time returned will be 0.
   */
  double getElapsedEciTime() const;

  /**
  * Set locator for this to follow in some way
  *
  * The optional parent locator.
  * If a Locator has a parent, it inherits position and orientation from that
  * parent as prescribed by the Components flags. Otherwise, the Locator
  * is absolute.
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
  * Get the spatial reference system associated with this locator
  * @return an SRS
  */
  const osgEarth::SpatialReference* getSRS() const { return mapSRS_.get(); }

  /**
  * Whether the Locator contains a valid position/orientation.
  * @return true if the locator's valid, else false
  */
  bool isEmpty() const;

  /**
  * Whether the Locator contains a valid position/orientation.
  * @return true if the locator's valid, else false
  */
  bool isValid() const { return !isEmpty(); }

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
   * Replaces the map SRS in this locator.
   * @param[in ] srs New map SRS.
   */
  void setMapSRS(const osgEarth::SpatialReference* srs);

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
  * Returns the timestamp on this locator. If a valid timestamp is not found, it will attempt to
  * find a valid timestamp in one of the parent locators. Used for ECI time functions.
  */
  double getTime_() const;

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

private: // methods
  void notifyListeners_();

  bool inherits_(unsigned int mask) const;

private: // data
  osg::ref_ptr<const osgEarth::SpatialReference> mapSRS_;
  osg::ref_ptr<Locator> parentLoc_;
  unsigned int componentsToInherit_; // Locator::Components mask
  RotationOrder rotOrder_;
  bool isEmpty_;                // if false, this locator has some data, though possibly only a timestamp
  std::set< osg::observer_ptr<Locator> > children_;
  std::vector< osg::ref_ptr<LocatorCallback> > callbacks_;

  simCore::Coordinate ecefCoord_;
  bool                ecefCoordIsSet_;

  simCore::Vec3       offsetPos_;
  simCore::Vec3       offsetOri_;
  bool                offsetsAreSet_;

  double timestamp_;
  double eciRefTime_;
  // cache frequently used LLA position and orientation
  mutable simCore::Vec3 llaPositionCache_;
  mutable osgEarth::Revision llaPositionCacheRevision_;
  mutable simCore::Vec3 llaOrientationCache_;
  mutable osgEarth::Revision llaOrientationCacheRevision_;
};


/**
* ResolvedPositionOrientationLocator is a locator that generates a position-with-orientation from its parents,
* based on the specified inheritance components, but which is treated thereafter (for subsequent inheritance) as a base coordinate position.
* Though it can apply its own offsets and be used like a normal locator, its primary function is to provide this resolved coordinate to inheriting locators.
* Subsequent locators that inherit the resolving locator get the same "resolved position with orientation" regardless of the orientation components they inherit.
* The inherited position matrix will represent a local tangent plane at the final position.
* Base orientation information is not stripped away as in COMP_RESOLVED_POSITION; it is maintained and available for inheritance.
*/
class SDKVIS_EXPORT ResolvedPositionOrientationLocator : public Locator
{
public:
  /** Constructor; @see Locator(const osgEarth::SpatialReference*) */
  ResolvedPositionOrientationLocator(const osgEarth::SpatialReference* mapSRS);
  /** Constructor; @see Locator(Locator*, unsigned int) */
  ResolvedPositionOrientationLocator(Locator* parentLoc, unsigned int inheritMask);
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
  /** Constructor; @see Locator(const osgEarth::SpatialReference*) */
  ResolvedPositionLocator(const osgEarth::SpatialReference* mapSRS);
  /** Constructor; @see Locator(Locator*, unsigned int) */
  ResolvedPositionLocator(Locator* parentLoc, unsigned int inheritMask);
private:
  /** @copydoc Locator::getOrientation_() */
  virtual bool getOrientation_(osg::Matrixd& ori, unsigned int comps) const;
};

//----------------------------------------------------------------------------
/// Interface for an object that can create a new Locator
class LocatorFactory
{
public:
  virtual ~LocatorFactory() {}

  /// create a new locator
  virtual Locator* createLocator() const = 0;
};

}

#endif // SIMVIS_LOCATOR_H
