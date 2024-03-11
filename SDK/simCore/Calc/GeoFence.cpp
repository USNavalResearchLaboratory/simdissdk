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
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/GeoFence.h"

namespace simCore {

/** WGS_A (Earth radius) divided by two */
inline constexpr double WGS_A_2 = simCore::WGS_A * 0.5;
/** List of points to use to generate rays to test intersection */
inline const std::vector<simCore::Vec3> TEST_POINTS = {
  { WGS_A_2, 0, 0 },
  { -WGS_A_2, 0, 0 },
  { 0, WGS_A_2, 0 },
  { 0, -WGS_A_2, 0 },
  { 0, 0, WGS_A_2 },
  { 0, 0, -WGS_A_2 },
};
/** Number of points in TEST_POINTS, divided by two (rounded down) */
inline constexpr size_t TEST_POINTS_SIZE_2 = 3;
/**
 * Scale of the polytope hull in terms of earth's radius. Should be greater than
 * one. This is the extrude factor for the fence heights. Higher fences will get
 * more intersections which can both be beneficial for finding in-fence for large
 * fences, but also negative for false positives.
 */
inline constexpr double POLYTOPE_HULL_SCALE = 4.;

//////////////////////////////////////////////////////////

GeoFence::GeoFence()
{
}

GeoFence::GeoFence(const std::vector<simCore::Vec3>& pts, simCore::CoordinateSystem coordSys)
{
  set(pts, coordSys);
}

void GeoFence::set(const std::vector<simCore::Vec3>& pts, simCore::CoordinateSystem coordSys)
{
  if (coordSys == simCore::COORD_SYS_ECEF)
  {
    setPointsEcef_(pts);
    return;
  }

  // Convert to ECEF
  simCore::CoordinateConverter cc;
  simCore::Coordinate output;
  std::vector<simCore::Vec3> ecef;
  for (const auto& pt : pts)
  {
    cc.convert(Coordinate(coordSys, pt), output, simCore::COORD_SYS_ECEF);
    ecef.push_back(output.position());
  }
  setPointsEcef_(ecef);
}

void GeoFence::setPointsEcef_(const std::vector<simCore::Vec3>& ptsEcef)
{
  points_ = ptsEcef;

  // Filter the points to remove successive duplicates, including front to back
  if (points_.size() > 2)
  {
    // Remove successive points where they are the same value
    for (size_t idx = 0; idx < points_.size() - 1; /* noop */)
    {
      if (points_[idx] == points_[idx + 1])
        points_.erase(points_.begin() + idx);
      else
        ++idx;
    }

    // Don't allow points to loop
    while (points_.size() > 2 && points_.front() == points_.back())
      points_.erase(points_.end() - 1);
  }
  triangles_ = calculatePolytopeHull_(points_);
  backfacePlane_ = calculateBackfacePlane_(points_);
}

bool GeoFence::contains(const simCore::Coordinate& coord) const
{
  if (coord.coordinateSystem() == simCore::COORD_SYS_ECEF)
    return contains(coord.position());
  simCore::Coordinate ecef;
  simCore::CoordinateConverter cc;
  cc.convert(coord, ecef, simCore::COORD_SYS_ECEF);
  return contains(ecef.position());
}

bool GeoFence::contains(const simCore::Vec3& ecef) const
{
  std::vector<Ray> notUsed;
  return contains(ecef, notUsed);
}

bool GeoFence::contains(const simCore::Vec3& ecef, std::vector<Ray>& raysTested) const
{
  raysTested.clear();
  // No triangles means not inside anything (fence not well defined)
  if (triangles_.empty())
    return false;

  const auto& ecefNormalized = ecef.normalize();
  const auto& onSurface = ecefNormalized * simCore::WGS_A;

  // Test a ray against the plane first. If it intersects, then we do not contain
  const simCore::Ray planeRay{ .origin = onSurface, .direction = -ecefNormalized };
  const auto& backfaceIsect = simCore::rayIntersectsPlane(planeRay, backfacePlane_);
  if (backfaceIsect.value_or(1.) >= 0.)
  {
    raysTested = { planeRay };
    return false;
  }

  size_t numInside = 0;
  size_t numOutside = 0;
  for (const auto& targetPoint : TEST_POINTS)
  {
    // Generate a point on the surface, roughly (spherical earth is fine). This is required
    // to avoid edge cases where points are very high or very low relative to earth surface,
    // which will create odd issues with intersection rays cast outside expected range.

    const simCore::Ray ray{ .origin = onSurface, .direction = (targetPoint - onSurface).normalize() };
    raysTested.push_back(ray);
    if (rayOriginatesInShape_(ray))
      ++numInside;
    else
      ++numOutside;

    // Minor optimization: If we have more than half inside or half outside, we're done
    if (numInside > TEST_POINTS_SIZE_2 || numOutside > TEST_POINTS_SIZE_2)
      break;
  }
  return numInside > numOutside;
}

bool GeoFence::valid() const
{
  return points_.size() >= 3;
}

bool GeoFence::rayOriginatesInShape_(const Ray& ray) const
{
  // https://en.wikipedia.org/wiki/Point_in_polygon
  return countIntersections_(ray, triangles_) % 2 == 1;
}

std::vector<Triangle> GeoFence::calculatePolytopeHull_(const std::vector<simCore::Vec3>& pts) const
{
  constexpr double TOTAL_SCALE = simCore::WGS_A * POLYTOPE_HULL_SCALE;

  std::vector<Triangle> rv;
  if (pts.size() < 3)
    return rv;

  // First triangle saved, is between the last point and first point in the vector
  auto behindIter = pts.begin();
  Triangle lastTriangle;
  lastTriangle.a.set(0, 0, 0);
  lastTriangle.b.set(pts.back().normalize() * TOTAL_SCALE);
  lastTriangle.c.set(behindIter->normalize() * TOTAL_SCALE);
  rv.push_back(lastTriangle);

  for (auto aheadIter = behindIter + 1; aheadIter != pts.end(); ++aheadIter, ++behindIter)
  {
    lastTriangle.b = lastTriangle.c;
    lastTriangle.c.set(aheadIter->normalize() * TOTAL_SCALE);
    rv.push_back(lastTriangle);
  }

  return rv;
}

int GeoFence::countIntersections_(const Ray& ray, const std::vector<Triangle>& triangles) const
{
  int rv = 0;
  for (const auto& triangle : triangles)
  {
    // Intersect testing, with inclusive edge testing enabled. This might mean that corners
    // get counted twice.
    const auto& results = rayIntersectsTriangle(ray, triangle, true);
    // If the resulting intersection is greater than the radius of earth, ignore it.
    // Give a little wiggle room
    if (results.intersects)
      ++rv;
  }
  return rv;
}

simCore::Plane GeoFence::calculateBackfacePlane_(const std::vector<simCore::Vec3>& pts) const
{
  // Return empty plane with empty fence
  if (pts.empty())
    return simCore::Plane();

  simCore::Vec3 minV{ std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),std::numeric_limits<double>::max(), };
  simCore::Vec3 maxV{ std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),std::numeric_limits<double>::lowest(), };
  for (const auto& v : pts)
  {
    for (size_t k = 0; k < 3; ++k)
    {
      minV[k] = std::min(minV[k], v[k]);
      maxV[k] = std::max(maxV[k], v[k]);
    }
  }
  const simCore::Vec3& centerUnitVec = ((minV + maxV) / 2.).normalize();
  return simCore::Plane(centerUnitVec, simCore::WGS_A * POLYTOPE_HULL_SCALE);
}

std::vector<Triangle> GeoFence::triangles() const
{
  return triangles_;
}

std::vector<simCore::Vec3> GeoFence::points() const
{
  return points_;
}

}
