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

Plane::Plane(const Plane& rhs)
{
  for (int i = 0; i < 4; ++i)
    v_[i] = rhs.v_[i];
}

double Plane::distance(const Vec3& p) const
{
  return v_[0]*p.x() + v_[1]*p.y() + v_[2]*p.z() + v_[3];
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

}
