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
#include <limits>
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Geometry.h"

namespace simCore {

Plane::Plane()
  : Plane({ 0., 0., 1.}, 0.)
{
}

Plane::Plane(const Vec3& p1, const Vec3& p2, const Vec3& p3)
{
  Vec3 a = p2 - p1;
  Vec3 b = p3 - p2;
  Vec3 w = a.cross(b);
  Vec3 normal = w.normalize();

  v_[0] = normal[0];
  v_[1] = normal[1];
  v_[2] = normal[2];
  v_[3] = -p1.dot(normal);
}

Plane::Plane(const Vec3& abc, double d)
{
  v_[3] = d;

  const auto& length = abc.length();
  // Zero length means the plane has no orientation and distance formula will return
  // unexpected (to the user) results (always "d")
  if (length != 0.)
  {
    const auto& norm = abc / length;
    for (size_t i = 0; i < 3; ++i)
      v_[i] = norm[i];

    // Since we normalized the vector, we need to inversely scale the distance too
    v_[3] *= length;
  }
}

double Plane::distance(const Vec3& p) const
{
  return v_[0]*p.x() + v_[1]*p.y() + v_[2]*p.z() + v_[3];
}

simCore::Vec3 Plane::normal() const
{
  return simCore::Vec3(v_[0], v_[1], v_[2]);
}

double Plane::d() const
{
  return v_[3];
}


//------------------------------------------------------------------------

Polytope::Polytope()
{
}

Polytope::Polytope(const Polytope& rhs) :
planes_(rhs.planes_)
{
}

void Polytope::addPlane(const Plane& plane)
{
  planes_.push_back(plane);
}

bool Polytope::contains(const Vec3& p) const
{
  const double epsilon = 1e-5;

  for (std::vector<Plane>::const_iterator i = planes_.begin(); i != planes_.end(); ++i)
  {
    const Plane& plane = *i;
    double dist = plane.distance(p);
    if (dist + epsilon < 0.0)
      return false;
  }
  return true;
}

void Polytope::clear()
{
  planes_.clear();
}

//------------------------------------------------------------------------

IntersectResultsRT rayIntersectsTriangle(const Ray& ray, const Triangle& triangle, bool inclusiveEdges)
{
  constexpr double epsilon = std::numeric_limits<double>::epsilon();

  IntersectResultsRT rv;

  // Adapted from:
  // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
  const simCore::Vec3& edge1 = triangle.b - triangle.a;
  const simCore::Vec3& edge2 = triangle.c - triangle.a;
  const simCore::Vec3& rayCrossEdge2 = ray.direction.cross(edge2);

  const double det = edge1.dot(rayCrossEdge2);
  if (simCore::isBetween(det, -epsilon, epsilon))
    return rv; // ray is parallel to triangle

  const double inverseDet = 1.0 / det;
  const simCore::Vec3& rayOriginRelocated = ray.origin - triangle.a;
  const double u = rayOriginRelocated.dot(rayCrossEdge2) * inverseDet;
  // Fail if the horizontal barycentric coordinate is outside the triangle
  if (inclusiveEdges)
  {
    if (u < 0. || u > 1.)
      return rv;
  }
  else
  {
    if (u <= 0. || u >= 1.)
      return rv;
  }

  const simCore::Vec3& rayCrossEdge1 = rayOriginRelocated.cross(edge1);
  const double v = ray.direction.dot(rayCrossEdge1) * inverseDet;
  // Fail if the vertical barycentric coordinate is outside the triangle
  if (inclusiveEdges)
  {
    if (v < 0. || u + v > 1.)
      return rv;
  }
  else
  {
    if (v <= 0. || u + v >= 1.)
      return rv;
  }

  // Compute t to figure out where intersection is
  const double t = edge2.dot(rayCrossEdge1) * inverseDet;
  rv.u = u;
  rv.v = v;
  rv.t = t;
  rv.intersects = t > epsilon;
  return rv;
}

std::optional<double> rayIntersectsPlane(const simCore::Ray& ray, const simCore::Plane& plane)
{
  // Adapted from:
  // https://stackoverflow.com/questions/7168484/3d-line-segment-and-plane-intersection

  const auto& normal = plane.normal();
  const double normalDotRay = normal.dot(ray.direction);
  // Check for 0, which is no intersection, ray is parallel to plane
  if (normalDotRay == 0.)
  {
    // Does the ray originate on the plane? If so every point intersects
    if (plane.distance(ray.origin) == 0)
      return 0.;
    return {};
  }

  const simCore::Vec3 pointOnPlane(normal * plane.d());
  const double d = normal.dot(pointOnPlane);
  return (d - normal.dot(ray.origin)) / normalDotRay;
}

std::optional<double> rayIntersectsSphere(const simCore::Ray& ray, const simCore::Sphere& sphere)
{
  // Construct vector from ray origin to the sphere center, and get length
  const simCore::Vec3 l = sphere.center - ray.origin;
  const double lNormSquared = l.dot(l);

  // Component of l onto ray. Since the ray direction is unit length, the component
  // is the distance along the ray to the closest point to the sphere (perp.)
  const double s = l.dot(ray.direction);
  const double radiusSquared = sphere.radius * sphere.radius;
  if (s < 0. && lNormSquared > radiusSquared)
  {
    // Sphere center behind ray origin AND ray origin is outside sphere
    return {};
  }

  // Calculate the distance squared, from closest point along the ray (perpendicular)
  // to the sphere center (i.e. applying Pythagorean logic)
  const double distanceSquared = lNormSquared - (s * s);
  // Does ray pass outside the sphere?
  if (distanceSquared > radiusSquared)
    return {};

  const double q = sqrt(radiusSquared - distanceSquared);
  // If true, ray origin is outside sphere, nearest intersection is at value t = s - q
  if ((lNormSquared - radiusSquared) > std::numeric_limits<double>::epsilon())
    return s - q;
  // Ray origin is inside sphere
  return s + q;
}

/** Helper function to return an invert of the incoming vector (1 / value) */
inline
simCore::Vec3 v3Invert(const simCore::Vec3& in)
{
  return { 1. / in.x(), 1. / in.y(), 1. / in.z() };
}

/** Returns a vector where the x, y, and z components are equal input vector components multiplied. */
inline
simCore::Vec3 v3ComponentMultiply(const simCore::Vec3& a, const simCore::Vec3& b)
{
  return { a.x() * b.x(), a.y() * b.y(), a.z() * b.z() };
}

std::optional<double> rayIntersectsEllipsoid(const simCore::Ray& ray, const simCore::Ellipsoid& ellipsoid)
{
  // Avoid divide by zero
  if (ellipsoid.scale.x() == 0. || ellipsoid.scale.y() == 0. || ellipsoid.scale.z() == 0.)
    return {};

  QuadricSurface q;
  q.a = 1. / (ellipsoid.scale.x() * ellipsoid.scale.x());
  q.b = 1. / (ellipsoid.scale.y() * ellipsoid.scale.y());
  q.c = 1. / (ellipsoid.scale.z() * ellipsoid.scale.z());
  q.k = -1.;

  // Translate the ray to the center
  const simCore::Ray trRay{ ray.origin - ellipsoid.center, ray.direction };
  // Solve the quadric
  const auto& ts = simCore::rayIntersectsQuadricSurface(trRay, q);
  // Project the ray
  if (ts.empty())
    return {};
  return ts[0];
}

simCore::Vec3 reflectVector(const simCore::Vec3& vec, const simCore::Vec3& normal)
{
  return vec - (normal * 2.0 * vec.dot(normal));
}

simCore::Ray reflectRay(const simCore::Ray& ray, const simCore::Vec3& atPoint, const simCore::Vec3& normal)
{
  return Ray{ atPoint, reflectVector(ray.direction, normal) };
}

std::vector<double> rayIntersectsQuadricSurface(const Ray& ray, const QuadricSurface& q)
{
  // Sourced from several places, ultimately using: http://www.bmsc.washington.edu/people/merritt/graphics/quadrics.html
  // a*x^2 + b*y^2 + c*z^2 + d*x*y + e*x*z + f*y*z + g*x + h*y + j*z + k = 0
  const auto& dir = ray.direction;
  const auto& o = ray.origin;

  // Expand the quadric formula using the ray equation (origin + direction). Solve
  // for T, leading to two solutions that can be solved with quadratic formula
  const double aq = q.a * dir.x() * dir.x()
    + q.b * dir.y() * dir.y()
    + q.c * dir.z() * dir.z()
    + q.d * dir.x() * dir.y()
    + q.e * dir.x() * dir.z()
    + q.f * dir.y() * dir.z();
  const double bq = 2 * q.a * o.x() * dir.x()
    + 2 * q.b * o.y() * dir.y()
    + 2 * q.c * o.z() * dir.z()
    + q.d * (o.x() * dir.y() + o.y() * dir.x())
    + q.e * (o.x() * dir.z() + o.z() * dir.x())
    + q.f * (o.y() * dir.z() + dir.y() * o.z())
    + q.g * dir.x()
    + q.h * dir.y()
    + q.j * dir.z();
  const double cq = q.a * o.x() * o.x()
    + q.b * o.y() * o.y()
    + q.c * o.z() * o.z()
    + q.d * o.x() * o.y()
    + q.e * o.x() * o.z()
    + q.f * o.y() * o.z()
    + q.g * o.x()
    + q.h * o.y()
    + q.j * o.z()
    + q.k;

  // We now have two solutions, as per quadratic formula, such that:
  //    aq * t^2 + bq * t + cq == 0

  // Avoid divide-by-zero, if Aq is 0 then return -cq / bq
  if (aq == 0.)
  {
    if (bq == 0.)
      return {};
    const double rv = -cq / bq;
    if (rv < 0)
      return {};
    return { rv };
  }

  // Check discriminant of quadratic formula, if less than 0 no intersection
  const double discrim = bq * bq - 4 * aq * cq;
  if (discrim < 0.)
    return {};
  const double sqrtDiscrim = std::sqrt(discrim);

  double t0 = (-bq - sqrtDiscrim) / (2 * aq);
  double t1 = (-bq + sqrtDiscrim) / (2 * aq);
  // Sort them from closest to farthest for easy parsing on return
  if (t0 > t1)
    std::swap(t0, t1);

  // Return 0, 1, or 2 values based on whether the ray is in front
  if (t1 < 0.)
    return {};
  if (t0 < 0.)
    return { t1 };
  return { t0, t1 };
}

}
