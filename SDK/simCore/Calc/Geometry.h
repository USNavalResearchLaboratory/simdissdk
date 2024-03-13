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

#include <optional>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"

namespace simCore
{

/// Vector of simCore::Vec3
typedef std::vector<Vec3> Vec3String;

/** A triangle is defined by three points in space. */
struct Triangle
{
  simCore::Vec3 a;
  simCore::Vec3 b;
  simCore::Vec3 c;
};

/** A ray is defined by a 3-D origin and an orientation. */
struct Ray
{
  simCore::Vec3 origin;
  simCore::Vec3 direction;
};

/**
 * Geometric plane in 3D space. Planes are defined by the formula:
 *   ax + by + cz + d = 0
 * The plane is defined by values a, b, c, and d.
 */
class SDKCORE_EXPORT Plane
{
public:
  /** Construct a plane with a normal (0,0,1) with d of 0 (i.e. the X/Y plane intersecting origin) */
  Plane();

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

  /**
   * Construct a new 3D plane from an orientation vector and distance. This is
   * equivalent to providing the plane formula, where abc.x = a, abc.y = b,
   * abc.z = c, and d = d, where the plane is defined as:
   *   ax + by + cz + d = 0
   * @param abc Plane formula values a, b, and c, in the formula above. Represents
   *   the plane's normal vector.
   * @param d Distance or d value in the formula above
   */
  Plane(const Vec3& abc, double d);

  /**
   * Shortest distance from a point to the plane. A positive number means
   * the point is "above" or "inside" the plane; zero means the point lies exactly
   * on the plane; negative means the point is "below" or "outside" the plane.
   * @param[in ] point Point to test.
   * @return Distance between point and plane
   */
  double distance(const Vec3& point) const;

  /** Returns the unit vector, or normalized orientation of plane (plane's normal vector); (a,b,c) */
  simCore::Vec3 normal() const;
  /** Returns the distance from 0,0,0 to the closest point on plane's surface */
  double d() const;

private:
  /** Vector representing the plane. Elements 0 1 2 represent a,b,c and are normalized. */
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

/** Results of an intersection test between a ray and triangle */
struct IntersectResultsRT
{
  // Barycentric coordinates of the intersection in the triangle
  double u = 0.;
  double v = 0.;

  /** Intersection point is ray.origin + t * ray.direction */
  double t = 0.;

  /** True when the ray intersects the triangle. */
  bool intersects = false;
};

/**
 * Performs an intersection test of a ray against a triangle. Returns whether
 * the ray intersects, the (u,v) of the intersection on the triangle, and the
 * distance "t" along the ray where the triangle intersects. Winding of the
 * triangle makes no difference. By default, this function returns a true
 * intersection when the ray obliquely intersects with the exact edge of the
 * triangle. The `inclusiveEdge` value can change that behavior.
 * @param ray Describes the ray (origin and direction) to test against triangle.
 * @param triangle Describes the 3-D triangle shape to test against ray.
 * @param inclusiveEdges If true, a ray obliquely hitting the exact edge of the
 *   triangle returns a hit on the triangle. If false, it does not. This is
 *   helpful to try to distinguish between hits and misses e.g. on the corner
 *   of a shape making up a hull.
 * @return Intersection results, including barycentric coordinate of the hit if
 *   an intersection occurred, and the distance along the ray for the intersection.
 */
SDKCORE_EXPORT IntersectResultsRT rayIntersectsTriangle(const Ray& ray, const Triangle& triangle, bool inclusiveEdges);

/**
 * Returns the intersection point along the ray where it intersects the plane. If the
 * ray does not intersect the plane due to it being on a parallel plane, this returns
 * an empty optional. Otherwise it returns a scale that is applied to the ray as to
 * where it intersects the plane. A negative value means the ray points away from the
 * plane, a positive value indicates the ray points into the plane, and a 0 values
 * indicates the ray starts on the plane. The intersection point can be determined
 * by calculating ray.origin + ray.direction * t, where we return the value t.
 * @param ray Arbitrary ray in 3D space
 * @param plane Arbitrary plane in 3D space
 * @return "t" value such that ray.origin + ray.direction * t intersects the plane.
 *   0 or positive value indicates intersection. Negative value indicates the
 *   intersection occurs behind the ray's origin (in wrong direction). Empty value
 *   indicates the ray is parallel to the plane.
 */
SDKCORE_EXPORT std::optional<double> rayIntersectsPlane(const simCore::Ray& ray, const simCore::Plane& plane);

/**
 * Reflects a pointing vector about a normal.
 * @param vec Input direction vector containing the pointing direction. Note the direction vector
 *   need not be normalized, but will scale the resulting reflection.
 * @param normal Surface normal about which to reflect the vec. This is expected to be normalized,
 *   but if not the results will be invalid.
 * @return Reflected direction vector
 */
SDKCORE_EXPORT simCore::Vec3 reflectVector(const simCore::Vec3& vec, const simCore::Vec3& normal);

/**
 * Reflects a ray against the normal, generating a new ray with the new orientation
 * and provided intersection point for the new ray's origin.
 * @param ray Input ray to reflect; note the direction vector need not be normalized but
 *   will scale the resulting ray's direction.
 * @param atPoint Surface intersection point, serves as origin of the return ray.
 * @param normal Surface normal at the intersection point, used to calculate reflection angle.
 *   This is expected to be normalized, but if not the results will be invalid.
 * @return Ray reflected about the normal originating at the given point.
 */
SDKCORE_EXPORT simCore::Ray reflectRay(const simCore::Ray& ray, const simCore::Vec3& atPoint, const simCore::Vec3& normal);

} // namespace simCore

#endif /* SIMCORE_CALC_GEOMETRY_H */
