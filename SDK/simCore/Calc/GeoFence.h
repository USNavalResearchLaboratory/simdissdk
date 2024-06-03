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

/**
 * Representation of a geo-fence in ECEF format. The geo-fence is defined by three or
 * more ECEF points. The fence works by doing a 3-D version of the ray casting algorithm.
 * To create a fence, the software uses a similar approach to simCore::GeoFence and
 * simCore::Polytope. It drives lines from each point from that point to the center of
 * the earth. Two successive points and (0,0,0) form a triangle. The entire series of
 * points together form a closed hull shape, almost like a coffee filter.
 *
 * The algorithm casts rays from the point towards up to six predetermined points, using
 * the even/odd intersection rules to determine whether the ray originates in the given
 * shape. Edges of the fence are extruded up slightly to account for tests of points
 * above the earth surface. Winding of the fence makes no difference.
 */
class SDKCORE_EXPORT GeoFence
{
public:
  /** Initializes an empty fence. */
  GeoFence();
  /** Initializes the fence with the given points in given coordinate system. */
  GeoFence(const std::vector<simCore::Vec3>& pts, simCore::CoordinateSystem coordSys);

  /**
   * Initializes the fence with the given geographic data points; will convert to ECEF as needed.
   * Winding of the fence makes no difference.
   * @param pts Points in a given coordinate system, such as ECEF.
   * @param coordSys Coordinate system; simCore::COORD_SYS_ECEF is most efficient, simCore::COORD_SYS_LLA also permitted.
   */
  void set(const std::vector<simCore::Vec3>& pts, simCore::CoordinateSystem coordSys);

  /** Returns true if the given coordinate is inside the fence; accepts LLA and ECEF points */
  bool contains(const simCore::Coordinate& coord) const;

  /** Returns true if the given ECEF XYZ is inside the fence. */
  bool contains(const simCore::Vec3& ecef) const;
  /** Returns true if the given ECEF XYZ is inside the fence, returning the rays tested e.g. for demo/testing purposes */
  bool contains(const simCore::Vec3& ecef, std::vector<Ray>& rays) const;

  /** Returns all triangles that represent the hull or "coffee filter" shape */
  std::vector<Triangle> triangles() const;
  /** Returns all the ECEF points representing the fence */
  std::vector<simCore::Vec3> points() const;

  /** Returns true if the fence is valid. Valid fences have 3 or more points. */
  bool valid() const;

private:
  /** Sets the points in ECEF coordinates */
  void setPointsEcef_(const std::vector<simCore::Vec3>& ptsEcef);

  /**
   * Given a series of connected points, construct a polytope hull made of triangles
   * such that each connected point of the fence forms the top of a given triangle in
   * the return vector, with the third point being (0,0,0). This creates a hull made
   * out of triangles that can be used for ray casting tests. Visually, this looks
   * like a cone with (points.size() - 1) flat sides.
   */
  std::vector<Triangle> calculatePolytopeHull_(const std::vector<simCore::Vec3>& pts) const;

  /**
   * Given a ray and series of triangles, returns the number of triangles that the
   * ray intersects.
   */
  int countIntersections_(const Ray& ray, const std::vector<Triangle>& triangles) const;

  /**
   * Returns true if the given ray intersects the configured triangles vector. Note that
   * this function can return a false positive or false negative in extreme edge cases,
   * where the ray directly and precisely intersects with a corner, either from the
   * inside or outside of the shape.
   * @param ray Testing ray against the internal triangles hull.
   * @return true if the ray tests to originate inside the shape, false if the ray originates outside.
   */
  bool rayOriginatesInShape_(const Ray& ray) const;

  /** Calculates the backface plane given all the data points */
  simCore::Plane calculateBackfacePlane_(const std::vector<simCore::Vec3>& pts) const;

  std::vector<simCore::Vec3> points_;
  std::vector<Triangle> triangles_;

  /** The plane helps detect/reject erroneous intersections through the earth. */
  simCore::Plane backfacePlane_;
};

} // namespace simCore

#endif /* SIMCORE_CALC_GEOFENCE_H */
