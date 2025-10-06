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
#ifndef SIMCORE_CALC_GEOMETRY_H
#define SIMCORE_CALC_GEOMETRY_H

#include <optional>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"

namespace simCore
{
class MultiFrameCoordinate;

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

/** Defines a mathematical sphere. It initializes as a unit sphere with a radius of 1. */
struct Sphere
{
  simCore::Vec3 center;
  double radius = 1.;
};

/**
 * Defines a simple ellipsoid, as a set of radius values in X Y and Z dimension.
 * This uses the general equation for an ellipsoid, which is:
 *
 *   x^2 / a^2 + y^2 / b^2 + z^2 / c^2 = 1
 *
 * The scale value represents the a, b, and c values in this equation.
 */
struct Ellipsoid
{
  simCore::Vec3 center;
  /** Scale relative to a unit sphere. This is equivalent to the radius values in each dimension. */
  simCore::Vec3 scale = simCore::Vec3(1., 1., 1.);
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
 * Generic description of a quadric surface, three dimensional surfaces with traces
 * composed of conic sections. Every quadric surface can be expressed by the formula:
 *
 * a*x^2 + b*y^2 + c*z^2 + d*x*y + e*x*z + f*y*z + g*x + h*y + j*z + k = 0
 *
 * In most quadrics, most values are 0.
 */
struct QuadricSurface
{
  double a = 0.; ///< Factor for x^2
  double b = 0.; ///< Factor for y^2
  double c = 0.; ///< Factor for z^2
  double d = 0.; ///< Factor for x*y
  double e = 0.; ///< Factor for x*z
  double f = 0.; ///< Factor for y*z
  double g = 0.; ///< Factor for x
  double h = 0.; ///< Factor for y
  double j = 0.; ///< Factor for z
  double k = 0.; ///< Constant factor
};

/**
 * Solves a Quadric Surface equation with a ray returning 0, 1, or 2 intersections. Note
 * that some shapes (e.g. sphere) have faster implementations; this is a generic solution.
 * @param ray Intersection ray to test; the direction need not be of unit length.
 * @param q Quadric surface definition
 * @return Intersection points along the ray such that (ray.origin + t * ray.direction) is
 *   an intersection. This might be 0, 1, or 2 points.
 */
SDKCORE_EXPORT std::vector<double> rayIntersectsQuadricSurface(const Ray& ray, const QuadricSurface& q);

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
 * Returns the distance along the ray where it intersects with the sphere. If the ray does
 * not intersect the sphere, this returns an empty optional. The origin is a valid intersection
 * point and would return 0. The ray may originate inside, outside, or on the sphere. A ray
 * that originates inside the sphere will return an intersection. A ray that originates on the
 * sphere will return a 0. A ray that originates outside the sphere will return an
 * intersection only if the ray passes through the sphere and the sphere is in front of the ray.
 * That is, the ray must point towards the sphere. The ray's direction must be of unit length.
 * @param ray Arbitrary ray in 3D space to test. The direction must be of unit length.
 * @param sphere Arbitrary sphere in 3D space to test.
 * @return Empty value if the ray does not intersect the sphere, else positive or zero value
 *   indicating the distance along the ray that the intersection occurs. This value is only
 *   trustworthy if the ray.direction is of unit length, which this function will not forcibly
 *   do, due to performance reasons.
 */
SDKCORE_EXPORT std::optional<double> rayIntersectsSphere(const simCore::Ray& ray, const simCore::Sphere& sphere);

/**
 * Returns the distance along the ray where it intersects with the ellipsoid. If the ray does
 * not intersect the ellipsoid, this returns an empty optional. The origin is a valid intersection
 * point and would return 0. The ray may originate inside, outside, or on the ellipsoid. A ray
 * that originates inside the ellipsoid will return an intersection. A ray that originates on the
 * ellipsoid will return a 0. A ray that originates outside the ellipsoid will return an
 * intersection only if the ray passes through the ellipsoid and the ellipsoid is in front of the ray.
 * That is, the ray must point towards the ellipsoid.
 * @param ray Arbitrary ray in 3D space to test. The direction need not be of unit length.
 * @param ellipsoid Arbitrary ellipsoid in 3D space to test
 * @return Empty value if the ray does not intersect the ellipsoid, else a positive or zero value.
 */
SDKCORE_EXPORT std::optional<double> rayIntersectsEllipsoid(const simCore::Ray& ray, const simCore::Ellipsoid& ellipsoid);

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

/**
 * Calculates the normal of the ellipsoid at a given intersection point.
 * @param ellipsoid Ellipsoid to test.
 * @param isectPt Intersection point on the ellipsoid surface.
 * @return Normal of the surface at the intersection point.
 */
SDKCORE_EXPORT simCore::Vec3 ellipsoidNormalAtIntersection(const simCore::Ellipsoid& ellipsoid, const simCore::Vec3& isectPt);

/**
 * Returns true if the line segment defined by p1 and p2 intersects a sphere of radius.
 * @param ecef1 The first location in ECEF coordinates
 * @param ecef2 The second location in ECEF coordinates
 * @param radius The radius of the sphere in meters
 * @return Returns true if the line segment defined by p1 and p2 intersects a sphere of radius.
 */
SDKCORE_EXPORT bool doesLineIntersectSphere(const simCore::Vec3& ecef1, const simCore::Vec3& ecef2, double radius);

} // namespace simCore

#endif /* SIMCORE_CALC_GEOMETRY_H */
