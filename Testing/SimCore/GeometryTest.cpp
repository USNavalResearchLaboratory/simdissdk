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
#include "simCore/Calc/CoordinateSystem.h"
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

int testReflectRay()
{
  // Implicitly tests reflect vector too
  int rv = 0;

  simCore::Ray ray;

  // Surface at 0,0,0, with varying normals. Ray points straight down.
  const simCore::Ray rayDown{ { 0, 100, 0 }, { 0, -1, 0 } };
  const simCore::Vec3 origin{ 0, 0, 0 };
  // First test fires down at a flat surface, expecting it to come back up
  ray = simCore::reflectRay(rayDown, origin, { 0, 1, 0 });
  rv += SDK_ASSERT(ray.origin == origin);
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, 1, 0));

  ray = simCore::reflectRay(rayDown, { 2, 3, 4 }, {0, 1, 0});
  rv += SDK_ASSERT(ray.origin == simCore::Vec3(2, 3, 4));
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, 1, 0));

  ray = simCore::reflectRay(rayDown, origin - simCore::Vec3( 0, 3, 0), {0, 1, 0});
  rv += SDK_ASSERT(ray.origin == simCore::Vec3(0, -3, 0));
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, 1, 0));

  // Now start to change the reflection angle by adjusting the normal:

  // Inverted normal points down. We bounce off the "back" of the flat surface
  ray = simCore::reflectRay(rayDown, origin, { 0, -1, 0 });
  rv += SDK_ASSERT(ray.origin == origin);
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, 1, 0));

  // 45 degree normal, positive into the X direction. Take the surface and tilt right
  ray = simCore::reflectRay(rayDown, origin, simCore::Vec3(1, 1, 0).normalize());
  rv += SDK_ASSERT(ray.origin == origin);
  // Because it's a 45 degree angle, the ray should reflect 90 degrees over and be positive on the X axis
  rv += SDK_ASSERT(simCore::v3AreEqual(ray.direction, simCore::Vec3(1, 0, 0)));

  // Reverse that angle normal and make sure results are the same
  ray = simCore::reflectRay(rayDown, origin, -simCore::Vec3(1, 1, 0).normalize());
  rv += SDK_ASSERT(ray.origin == origin);
  rv += SDK_ASSERT(simCore::v3AreEqual(ray.direction, simCore::Vec3(1, 0, 0)));

  // Tilt left, should go down X axis negative
  ray = simCore::reflectRay(rayDown, origin, simCore::Vec3(-1, 1, 0).normalize());
  rv += SDK_ASSERT(ray.origin == origin);
  rv += SDK_ASSERT(simCore::v3AreEqual(ray.direction, simCore::Vec3(-1, 0, 0)));

  // Test with non-unit normals
  ray = simCore::reflectRay(rayDown, origin, { 0, 8, 0 });
  rv += SDK_ASSERT(ray.origin == origin);
  // The surface normal is not normalized, so the result is garbage and not (0,1,0)
  rv += SDK_ASSERT(ray.direction != simCore::Vec3(0, 1, 0));

  const simCore::Ray rayDown3{ { 0, 100, 0}, {0, -3, 0 } };
  ray = simCore::reflectRay(rayDown3, origin, { 0, 1, 0 });
  rv += SDK_ASSERT(ray.origin == origin);
  // The surface normal is OK but ray direction is scaled; it reflects back at scale
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, 3, 0));

  // Same test, but onto the right-slanted surface
  ray = simCore::reflectRay(rayDown3, origin, simCore::Vec3(1, 1, 0).normalize());
  rv += SDK_ASSERT(ray.origin == origin);
  // Because it's a 45 degree angle, the ray should reflect 90 degrees over and be positive on the X axis
  rv += SDK_ASSERT(simCore::v3AreEqual(ray.direction, simCore::Vec3(3, 0, 0)));

  // Test against a surface that is parallel to the ray, with its normal pointed down the X axis
  ray = simCore::reflectRay(rayDown, origin, { -1, 0, 0 });
  rv += SDK_ASSERT(ray.origin == origin);
  // No change in the ray's direction, it does not intersect at all and keeps going through
  rv += SDK_ASSERT(ray.direction == simCore::Vec3(0, -1, 0));

  return rv;
}

int testSphere()
{
  int rv = 0;

  const simCore::Ray rayDown{ { 0, 100, 0 }, { 0, -1, 0 } };

  std::optional<double> val;

  // Ray pointing down into unit sphere
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere());
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 99.));

  // Ray pointing down to sphere at origin, but radius of 2
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(), 2. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 98.));

  // Move the unit sphere 1 "down", so intersection is now at origin
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(0, -1, 0), 1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 100.));

  // Move unit sphere 1 "right", so it barely grazes the left side of sphere, hitting tangent
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(1.0, 0, 0), 1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 100.));

  // Same as before, but sphere moves SLIGHTLY more so the ray misses
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(1.00001, 0, 0), 1. });
  rv += SDK_ASSERT(!val.has_value());

  // Make sure ray pointing in other direction misses
  val = simCore::rayIntersectsSphere(simCore::Ray{ { 0, 100, 0 }, { 0, 1, 0 } }, simCore::Sphere());
  rv += SDK_ASSERT(!val.has_value());

  // Ray direction not unit length
  val = simCore::rayIntersectsSphere(simCore::Ray{ { 0, 100, 0 }, { 0, -20, 0 } }, simCore::Sphere{ simCore::Vec3(0, -1, 0), 1. });
  // Would typically be 100.0, but ray direction vector is scaled to a length of 20, which
  // throws off the calculations. You'd think naively the answer might be 5, but it's not.
  rv += SDK_ASSERT(val.has_value() && !simCore::areEqual(*val, 5.));

  // Test inside the sphere:
  const simCore::Ray rayInside{ { 0, 0, 0 }, { 0, -1, 0 } };

  // Ray pointing down in unit sphere
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere());
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 1.));

  // Scale up the sphere to radius of 2
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere{ simCore::Vec3(), 2. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 2.));

  // Move the unit sphere 1 "down", so intersection is now at ray origin AND
  // at -2, but we only test the first intersection.
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere{ simCore::Vec3(0, 1, 0), 1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 0.));

  // Move the unit sphere 1 "up", so intersection is now at ray origin
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere{ simCore::Vec3(0, 1, 0), 1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 0.));

  // Test that a ray "through" a 0 radius sphere still hits at origin
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(), 0. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 100.));

  // Ray starting on 0 radius sphere hits at origin
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere{ simCore::Vec3(), 0. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 0.));

  // Test that a ray through a negative radius hits as normal
  val = simCore::rayIntersectsSphere(rayDown, simCore::Sphere{ simCore::Vec3(), -1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 99.));

  // Ray inside the negative size radius sphere hits as normal
  val = simCore::rayIntersectsSphere(rayInside, simCore::Sphere{ simCore::Vec3(), -1. });
  rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 1.));

  // Comparison test against https://www.geogebra.org/m/uxv5kfum visualizer; independently verify
  {
    simCore::Ray ray{ { -0.19, 1.82, 1.0 }, simCore::Vec3(-2.0, 1.31, 0.48).normalize() };
    val = simCore::rayIntersectsSphere(ray,
      simCore::Sphere{ { -7.04, 5.16, 2.0}, 1.5 });
    // Comparison values (6.57 and (-5.58, 5.35, 2.3) extracted from website values
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 6.57, 0.01));
    const auto& intersectPoint = ray.origin + ray.direction * (*val);
    rv += SDK_ASSERT(simCore::v3AreEqual(intersectPoint, simCore::Vec3(-5.58, 5.35, 2.3), 0.01));
  }

  return rv;
}

int testEllipsoid()
{
  int rv = 0;

  const simCore::Ray rayDown{ { 0, 100, 0 }, { 0, -1, 0 } };
  const simCore::Vec3 vOne(1., 1., 1.);

  std::optional<double> val;

  { // Start with basic tests very similar to Sphere, using a spherical ellipsoid
    // Ray pointing down into unit sphere
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid());
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 99.));

    // Ray pointing down to sphere at origin, but radius of 2
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(), vOne * 2. });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 98.));

    // Move the unit sphere 1 "down", so intersection is now at origin
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(0, -1, 0), vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 100.));

    // Move unit sphere 1 "right", so it barely grazes the left side of sphere, hitting tangent
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(1.0, 0, 0), vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 100.));

    // Same as before, but sphere moves SLIGHTLY more so the ray misses
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(1.00001, 0, 0), vOne });
    rv += SDK_ASSERT(!val.has_value());

    // Make sure ray pointing in other direction misses
    val = simCore::rayIntersectsEllipsoid(simCore::Ray{ { 0, 100, 0 }, { 0, 1, 0 } }, simCore::Ellipsoid());
    rv += SDK_ASSERT(!val.has_value());

    // Ray direction not unit length
    val = simCore::rayIntersectsEllipsoid(simCore::Ray{ { 0, 100, 0 }, { 0, -20, 0 } }, simCore::Ellipsoid{ simCore::Vec3(0, -1, 0), vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 5.));

    // Test inside the sphere:
    const simCore::Ray rayInside{ { 0, 0, 0 }, { 0, -1, 0 } };

    // Ray pointing down in unit sphere
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid());
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 1.));

    // Scale up the sphere to radius of 2
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid{ simCore::Vec3(), vOne * 2 });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 2.));

    // Move the unit sphere 1 "down", so intersection is now at ray origin AND
    // at -2, but we only test the first intersection.
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid{ simCore::Vec3(0, 1, 0), vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 0.));

    // Move the unit sphere 1 "up", so intersection is now at ray origin
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid{ simCore::Vec3(0, 1, 0), vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 0.));

    // Test that a ray "through" a 0 radius sphere still hits at origin
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(), simCore::Vec3() });
    rv += SDK_ASSERT(!val.has_value());

    // Ray starting on 0 radius sphere hits at origin
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid{ simCore::Vec3(), simCore::Vec3() });
    rv += SDK_ASSERT(!val.has_value());

    // Test that a ray through a negative radius hits as normal
    val = simCore::rayIntersectsEllipsoid(rayDown, simCore::Ellipsoid{ simCore::Vec3(), -vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 99.));

    // Ray inside the negative size radius sphere hits as normal
    val = simCore::rayIntersectsEllipsoid(rayInside, simCore::Ellipsoid{ simCore::Vec3(), -vOne });
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 1.));
  }

  // Comparison test against https://www.geogebra.org/m/uxv5kfum visualizer; independently verify
  {
    simCore::Ray ray{ { -0.19, 1.82, 1.0 }, simCore::Vec3(-2.0, 1.31, 0.48).normalize() };
    val = simCore::rayIntersectsEllipsoid(ray,
      simCore::Ellipsoid{ { -7.04, 5.16, 2.0}, vOne * 1.5 });
    // Comparison values (6.57 and (-5.58, 5.35, 2.3) extracted from website values
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, 6.57, 0.01));
    const auto& intersectPoint = ray.origin + ray.direction * (*val);
    rv += SDK_ASSERT(simCore::v3AreEqual(intersectPoint, simCore::Vec3(-5.58, 5.35, 2.3), 0.01));
  }

  // More complex case with WGS-84 ellipsoid and intersections near a major city
  {
    // Area near DC, 38.9072 N, 77.0369 W, 0.0 m
    const simCore::Vec3 dcEcef{ 1099033.55, 4774463.87, 4070086.94 };
    //const simCore::Vec3 dcEcef{ 1099033.56, 4774463.95, 4070087.01 };
    const simCore::Ellipsoid ecef{ {}, { simCore::WGS_A, simCore::WGS_A, simCore::WGS_B} };

    // Form a ray from the center of earth, pointing right at DC
    const simCore::Ray rayCtoDc{ {}, dcEcef.normalize() };
    val = simCore::rayIntersectsEllipsoid(rayCtoDc, ecef);
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, dcEcef.length(), 0.01));

    // Form another ray from above the earth, pointing towards DC; position arbitrary but outside ellipsoid
    const simCore::Vec3& spaceRayOffset{ 1000., 3000., 8000. };
    const simCore::Vec3& spaceRayOrigin = dcEcef + spaceRayOffset;
    const simCore::Ray spaceRay{ spaceRayOrigin, (dcEcef - spaceRayOrigin).normalize() };
    val = simCore::rayIntersectsEllipsoid(spaceRay, ecef);
    rv += SDK_ASSERT(val.has_value() && simCore::areEqual(*val, spaceRayOffset.length(), 0.01));
  }

  return rv;
}

int testQuadricSurface()
{
  // Test various quadric surface intersections
  int rv = 0;

  // testEllipsoid() already tests this code against ellipsoidal values. This routine
  // is intended to catch other edge cases.
  {
    // Create a contrived case to hit the divide-by-zero case in the quadric solver
    simCore::QuadricSurface q;
    // Start with a hyperbola
    q.a = 1;
    q.b = -1;
    q.k = 1;
    const simCore::Ray ray{ {}, {1,1,0} };

    // aq == 0 and bq == 0
    auto tt = simCore::rayIntersectsQuadricSurface(ray, q);
    rv += SDK_ASSERT(tt.empty());

    // Shrinks the hyperbola a bit in the y dimension, moving left a bit
    // in the x dimension. Never intersects:
    // aq == 0 and (-cq / bq) is negative (behind ray)
    q.g = 1;
    tt = simCore::rayIntersectsQuadricSurface(ray, q);
    rv += SDK_ASSERT(tt.empty());

    // aq == 0 and (-cq / bq) is positive (in front of ray), intersects at
    // (1/sqrt(2)) in x/y, at ray length 1.0. This is a hyperbola shifted
    // up and right slightly.
    q.g = -1;
    tt = simCore::rayIntersectsQuadricSurface(ray, q);
    rv += SDK_ASSERT(tt.size() == 1 && tt[0] == 1.);

    // Repeat, with a longer ray direction, should shrink result
    const simCore::Ray ray2{ {}, { 2, 2, 0} };
    tt = simCore::rayIntersectsQuadricSurface(ray2, q);
    rv += SDK_ASSERT(tt.size() == 1 && tt[0] == 0.5);
  }

  return rv;
}

}

int GeometryTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testTriangleIntersect() == 0);
  rv += SDK_ASSERT(testPlane() == 0);
  rv += SDK_ASSERT(testReflectRay() == 0);
  rv += SDK_ASSERT(testSphere() == 0);
  rv += SDK_ASSERT(testEllipsoid() == 0);
  rv += SDK_ASSERT(testQuadricSurface() == 0);

  std::cout << "GeometryTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";
  return rv;
}
