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

#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"

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
   * @return Distance between point and plane
   */
  double distance(const Vec3& point) const;

private:
  /** Vector representing the plane */
  double v_[4] = { 0., 0., 0., 0. };
};

/// Collection of 3D planes acting as a (possibly open) convex bounding volume.
/// The polytope is said to "contain" a point if that point lies "above"
/// all planes comprising the polytope. An empty polytope (zero planes)
/// contains all points.
class SDKCORE_EXPORT Polytope
{
public:
  /** Construct a new empty polytope */
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

  /** Resets the polytope by removing all planes. */
  void clear();

private:
  /** Vector of all planes that, together, represent the polytope */
  std::vector<Plane> planes_;
};

} // namespace simCore

#endif /* SIMCORE_CALC_GEOMETRY_H */
