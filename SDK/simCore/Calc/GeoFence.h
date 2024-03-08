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
#ifndef SIMCORE_CALC_GEOFENCE_H
#define SIMCORE_CALC_GEOFENCE_H

#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/Vec3.h"

namespace simCore
{

class Coordinate;

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
  GeoFence(const std::vector<simCore::Vec3>& points, const CoordinateSystem& cs);

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
  void set(const std::vector<simCore::Vec3>& points, const CoordinateSystem& cs);

  /** True if the fence is valid (forms a convex region, with at least 3 vertices) */
  bool valid() const { return valid_; }

  /**
   * True if the point is on the inside of the fence.
   * @param[in ] ecef Point to test; must be ECEF.
   */
  bool contains(const Vec3& ecef) const;

  /**
   * True if the fence contains the coordinate.
   * @param[in ] coord Coord to test; must be LLA or ECEF, otherwise the method will return false
   */
  bool contains(const Coordinate& coord) const;

  /** dtor */
  virtual ~GeoFence() { }

private:
  /** data points in the fence */
  std::vector<simCore::Vec3> points_;
  /** Polytope representing the fence shape */
  Polytope tope_;
  /** True when the shape is valid */
  bool valid_;

  /// call this after set
  bool verifyConvexity_(const std::vector<simCore::Vec3>& v) const;
};

} // namespace simCore

#endif /* SIMCORE_CALC_GEOFENCE_H */
