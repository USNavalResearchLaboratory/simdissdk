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
#ifndef SIMCORE_CALC_GEOMETRY_H
#define SIMCORE_CALC_GEOMETRY_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Math.h"
#include <vector>

namespace simCore
{
  /// Vector of simCore::Vec3
  typedef std::vector<Vec3> Vec3String;

  /// Geometric plane in 3D space.
  class SDKCORE_EXPORT Plane
  {
  public:
    /**
    * Construct a new 3D plane from 3 points. The plane's normal vector will
    * be (p2-p1) X (p3-p2), where X denotes a the cross product. A point on the
    * same side of the plane as the positive normal vector is considered to be
    * "above" or "inside" the plane and will yield a positive "distance" from
    * the plane's surface.
    *
    * @param[in ] p1 First point
    * @param[in ] p2 Second point
    * @param[in ] p3 Third point
    */
    Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3);

    /// copy ctor
    Plane(const Plane& rhs);

    /// dtor
    virtual ~Plane() { }

    /**
    * Shortest distance from a point to the plane. A positive number means
    * the point is "above" or "inside" the plane; zero means the point lies exactly
    * on the plane; negative means the point is "below" or "outside" the plane.
    * @param[in ] point Point to test.
    */
    double distance(const Vec3& point) const;

  protected:
    /** Vector representing the plane */
    double v_[4];
  };

  /// Collection of 3D planes acting as a (possibly open) convex bounding volume.
  /// The polytope is said to "contain" a point if that point lies "above"
  /// all planes comprising the polytope. An empty polytope (zero planes)
  /// contains all points.
  class SDKCORE_EXPORT Polytope
  {
  public:
    /**
    * Construct a new empty polytope
    */
    Polytope();

    /// copy ctor
    Polytope(const Polytope& rhs);

    /// dtor
    virtual ~Polytope() { }

    /**
    * Adds a bounding plane to the polytope. The "inside" of the plane
    * is the side with the positive normal vector.
    * @param[in ] plane Bounding plane to add
    */
    void addPlane(const Plane& plane);

    /**
    * True is the point is bounded by the polytope. An empty polytope (no
    * planes) contains all points. A point is contained if it falls on the
    * positive-normal side of all planes.
    * @param[in ] point Point to test.
    */
    bool contains(const Vec3& point) const;

    /**
    * Resets the polytope by removing all planes.
    */
    void clear();

  protected:
    /** Vector of all planes that, together, represent the polytope */
    std::vector<Plane> planes_;
  };

  /// Geographic, convex bounding region formed from a line string boundary.
  /// Each pair of points forms a segment of the fence. If the last point in the
  /// line string is the same as the first, the fence will bound a closed region.
  /// A valid polygon must have its vertices specified in CCW order.
  class SDKCORE_EXPORT GeoFence
  {
  public:
    /**
    * Constructs an empty fence. An empty fence contains everything.
    */
    GeoFence();

    /// copy ctor
    GeoFence(const GeoFence& rhs);

    /**
    * Construct a new geofence with the bounding coordinates.
    *
    * @param[in ] points Bounding points (i.e. "fence posts"). The fence will be
    *                    "open" unless you repeat the start point as the end point.
    *                    Each segment (consecutive point pairs) must be less than
    *                    180 degrees apart, otherwise the fence will be invalid.
    *                    The closed fence must be convex.
    * @param[in ] cs     Coordinate system of [points], must be LLA or ECEF.
    */
    GeoFence(const Vec3String& points, const CoordinateSystem& cs);

    /**
    * Sets the boundary points of the fence.
    *
    * @param[in ] points Bounding points (i.e. "fence posts"). The fence will be
    *                    "open" unless you repeat the start point as the end point.
    *                    Each segment (consecutive point pairs) must be less than
    *                    180 degrees apart, otherwise the fence will be invalid.
    *                    The closed fence must be convex.
    * @param[in ] cs     Coordinate system of [points], must be LLA or ECEF.
    */
    void set(const Vec3String& points, const CoordinateSystem& cs);

    /**
    * True if the fence is valid (forms a convex region, with at least 3 vertices)
    */
    bool valid() const { return valid_; }

    /**
    * True if the point is on the inside of the fence.
    * @param[in ] ecef Point to test; must be ECEF.
    */
    bool contains(const Vec3& ecef) const;

    /**
    * True if the fence contains the coordinate.
    * @param[in ] coord Coord to test; must be LLA or ECEF, otherwise the method will
    *                   return false
    */
    bool contains(const Coordinate& coord) const;

    /** dtor */
    virtual ~GeoFence() { }

  protected:

    /** data points in the fence */
    Vec3String points_;
    /** Polytope representing the fence shape */
    Polytope   tope_;
    /** True when the shape is valid */
    bool       valid_;

    /// call this after set
    bool verifyConvexity_(const Vec3String& v) const;
  };

} // namespace simCore

#endif /* SIMCORE_CALC_GEOMETRY_H */
