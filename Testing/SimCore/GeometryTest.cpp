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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Geometry.h"
#include "simCore/Calc/Math.h"

namespace {

int testTriangleIntersect()
{
  int rv = 0;

  simCore::IntersectResultsRT res;
  const simCore::Triangle tri1 { {0,0,0}, {0,4,10}, {0,-4,10} };

  // Ray points directly into triangle from 1000 units away
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -1000,0,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  rv += SDK_ASSERT(simCore::areEqual(res.t, 1000.));

  // Same as previous test, but with a longer (non-normalized) direction vector; the
  // "t" value should scale down to compensate.
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -1000,0,5 }, .direction = { 10000,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  rv += SDK_ASSERT(simCore::areEqual(res.t, 0.1));

  // Pointing away from triangle from 1000 units away
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -1000,0,5 }, .direction = { -1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(!res.intersects);

  // Cover a literal corner case, where two triangles intersect and a ray passes through;
  // These first few tests confirm good intersection between near the edge.
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5,-1.999,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5, 1.999,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5,-2.001,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(!res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5, 2.001,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(!res.intersects);

  // These two are right at the left and right edge of the triangles
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5,-2.0,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5, 2.0,5 }, .direction = { 1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5,-2.0,5 }, .direction = { 1,0,0 } }, tri1, false);
  rv += SDK_ASSERT(!res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { -5, 2.0,5 }, .direction = { 1,0,0 } }, tri1, false);
  rv += SDK_ASSERT(!res.intersects);

  // Same test from other side
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { 5,-2.0,5 }, .direction = { -1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { 5, 2.0,5 }, .direction = { -1,0,0 } }, tri1, true);
  rv += SDK_ASSERT(res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { 5,-2.0,5 }, .direction = { -1,0,0 } }, tri1, false);
  rv += SDK_ASSERT(!res.intersects);
  res = simCore::rayIntersectsTriangle(simCore::Ray{ .origin = { 5, 2.0,5 }, .direction = { -1,0,0 } }, tri1, false);
  rv += SDK_ASSERT(!res.intersects);

  return rv;
}

int testPlane()
{
  int rv = 0;

  // Plane that intersects origin and normal is facing up Z axis
  const simCore::Plane xyPlane({ 0, 0, 1. }, 0.);
  std::optional<double> t;

  // On plane, pointing up
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 0. }, .direction { 0., 0., 1.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 0.));
  // under plane, pointing up
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., -1. }, .direction { 0., 0., 1.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 1.));
  // above plane, pointing up
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., 1.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, -1.));

  // under plane, pointing down
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., -1. }, .direction { 0., 0., -1.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, -1.));
  // above plane, pointing down
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., -1.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 1.));

  // above plane, pointing horizontal and not intersecting
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 1., 0.} }, xyPlane);
  rv += SDK_ASSERT(!t.has_value());
  // below plane, pointing horizontal and not intersecting
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., -1. }, .direction { 0., 1., 0.} }, xyPlane);
  rv += SDK_ASSERT(!t.has_value());
  // on plane; every point intersects the plane
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., -0. }, .direction { 0., 1., 0.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 0.));

  // Confirm ray normal scaling impacts results properly
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., -2.} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 0.5));
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., -0.5} }, xyPlane);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 2.));

  // Confirm plane normal scaling has no impact since it's just a direction
  const simCore::Plane xyPlane2({ 0, 0, 3. }, 0.);
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., -1.} }, xyPlane2);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 1.0));
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 1. }, .direction { 0., 0., -2.} }, xyPlane2);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 0.5));

  // Move the XY plane up a few points and try to shoot a ray into it, so it doesn't intersect origin
  const simCore::Plane xyPlaneAt8({ 0, 0, 1. }, 8.);
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 0. }, .direction { 0., 0., 1.} }, xyPlaneAt8);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 8.));

  // Same test, but with a scaled normal on the plane
  const simCore::Plane xyPlaneAt8_2({ 0, 0, 2. }, 4.);
  t = simCore::rayIntersectsPlane({ .origin { -100.0, 2., 0. }, .direction { 0., 0., 1.} }, xyPlaneAt8_2);
  rv += SDK_ASSERT(t.has_value());
  rv += SDK_ASSERT(simCore::areEqual(*t, 8.));

  // Test distances
  rv += SDK_ASSERT(simCore::areEqual(xyPlane.distance({ 0, 0, 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane.distance({ 10., 0, 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane.distance({ 10., 10., 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane.distance({ 10., 10., 10. }), 10.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane.distance({ 10., 10., -10. }), -10.));

  // Flip the normal on the plane
  const simCore::Plane xyPlane3({ 0., 0., -1 }, 0.);
  rv += SDK_ASSERT(simCore::areEqual(xyPlane3.distance({ 0, 0, 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane3.distance({ 10., 0, 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane3.distance({ 10., 10., 0 }), 0.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane3.distance({ 10., 10., 10. }), -10.));
  rv += SDK_ASSERT(simCore::areEqual(xyPlane3.distance({ 10., 10., -10. }), 10.));

  return rv;
}

}

int GeometryTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testTriangleIntersect() == 0);
  rv += SDK_ASSERT(testPlane() == 0);

  std::cout << "GeometryTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";
  return rv;
}

