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
#include <float.h>
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/GeoFence.h"

namespace {

int testConcaveEcefIntersect()
{
  int rv = 0;

  simCore::GeoFence fence;

  // First fence is a rectangle at height 100, except that the "top" of the rectangle
  // dips in to form a "v" 25% of the way down. So:
  //
  // --\     /--
  // |  --v--  |
  // |         |
  // |         |
  // -----------
  //
  // ... almost like house with an inverted roof.
  fence.set({
    { -100, 100, 100 },
    { 0, 50, 100 },
    { 100, 100, 100 },
    { 100, -100, 100 },
    { -100, -100, 100 },
    }, simCore::COORD_SYS_ECEF);

  rv += SDK_ASSERT(fence.contains({ 1,0,50 }));
  rv += SDK_ASSERT(fence.contains({ 1,0,150 }));
  rv += SDK_ASSERT(fence.contains({ 1,0,100 }));

  // These tests are particularly useful because when the ray angle defaults to 45
  // degrees (e.g. in GeoFence::contains() if XOFF YOFF and ZOFF are the
  // same value), then the ray that gets cast will intersect with the EXACT corner
  // between side 1 and side 2 (0-based). This causes the intersection code to, by
  // default, intersect BOTH triangles. This is a white box test to make sure that does
  // not happen, and if this fails it's an algorithmic edge case.
  rv += SDK_ASSERT(fence.contains({ 0,0,50 }));
  rv += SDK_ASSERT(fence.contains({ 0,0,150 }));
  rv += SDK_ASSERT(fence.contains({ 0,0,100 }));

  rv += SDK_ASSERT(!fence.contains({ 0,60,100 }));
  rv += SDK_ASSERT(fence.contains({ 0,40,100 }));
  rv += SDK_ASSERT(fence.contains({ 0,-40,100 }));

  rv += SDK_ASSERT(!fence.contains({ 0,120,100 }));
  rv += SDK_ASSERT(!fence.contains({ 0,-120,100 }));
  rv += SDK_ASSERT(!fence.contains({ 90,-120,100 }));
  rv += SDK_ASSERT(!fence.contains({ -90,-120,100 }));
  rv += SDK_ASSERT(!fence.contains({ 0,120,100 }));
  rv += SDK_ASSERT(!fence.contains({ 90,120,100 }));
  rv += SDK_ASSERT(!fence.contains({ -90,120,100 }));

  rv += SDK_ASSERT(fence.contains({ 90,90,100 }));
  rv += SDK_ASSERT(fence.contains({ 90,-90,100 }));
  rv += SDK_ASSERT(fence.contains({ -90,90,100 }));
  rv += SDK_ASSERT(fence.contains({ -90,-90,100 }));

  // Set up a fence near the north pole
  fence.set({
    { -100, -100, simCore::WGS_A },
    { -100, 100, simCore::WGS_A },
    { 100, 100, simCore::WGS_A },
    { 100, -100, simCore::WGS_A }
    }, simCore::COORD_SYS_ECEF);
  // This next test has the same two-triangles-edge issue as above
  rv += SDK_ASSERT(fence.contains({ 0, 0, simCore::WGS_A }));
  rv += SDK_ASSERT(fence.contains({ 0, 1, simCore::WGS_A }));
  rv += SDK_ASSERT(fence.contains({ 99, 98, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 98, 101, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 101, 98, simCore::WGS_A }));

  // Now set up points along the south pole to test
  rv += SDK_ASSERT(!fence.contains({ 0, 0, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 0, 1, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 99, 98, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 98, 101, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 101, 98, -simCore::WGS_A }));

  // Now try a variety of points around the equator every 10 degrees
  for (int angle = 0; angle < 36; ++angle)
  {
    const double pct = (angle / 36.);
    // All points should be outside the cone
    const bool inside = fence.contains({ simCore::WGS_A * cos(pct * M_TWOPI), simCore::WGS_A * sin(pct * M_TWOPI), 0. });
    if (inside)
    {
      ++rv;
      std::cerr << "Failed north pole fence.contains() on equator at angle #" << angle << "\n";
    }
  }

  // Flip the fence so it's on the south pole, and repeat
  fence.set({
    { -100, -100, -simCore::WGS_A },
    { -100, 100, -simCore::WGS_A },
    { 100, 100, -simCore::WGS_A },
    { 100, -100, -simCore::WGS_A }
    }, simCore::COORD_SYS_ECEF);
  // South pole tests
  rv += SDK_ASSERT(fence.contains({ 0, 1, -simCore::WGS_A }));
  rv += SDK_ASSERT(fence.contains({ 99, 98, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 98, 101, -simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 101, 98, -simCore::WGS_A }));

  // North pole fails
  rv += SDK_ASSERT(!fence.contains({ 0, 0, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 0, 1, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 99, 98, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 98, 101, simCore::WGS_A }));
  rv += SDK_ASSERT(!fence.contains({ 101, 98, simCore::WGS_A }));

  // Equator tests
  for (int angle = 0; angle < 36; ++angle)
  {
    const double pct = (angle / 36.);
    // All points should be outside the cone
    const bool inside = fence.contains({ simCore::WGS_A * cos(pct * M_TWOPI), simCore::WGS_A * sin(pct * M_TWOPI), 0. });
    if (inside)
    {
      ++rv;
      std::cerr << "Failed south pole fence.contains() on equator at angle #" << angle << "\n";
    }
  }

  return rv;
}

int cornerTestIntersect()
{
  int rv = 0;

  // This started as a white-box test for contains() that was intended to catch
  // corner intersections, which originally caused some significant issues in earlier
  // versions of the algorithm. The test remains because it's still valid, but it
  // no longer is representative of any internal edge case possible bug.
  constexpr double XOFF = 1009.;
  constexpr double YOFF = 1013.;
  constexpr double ZOFF = 1019.;

  simCore::GeoFence fence;
  // This is a convex fence, but will have the same problems as the concave
  // with regards to corner testing.
  fence.set({
    { -XOFF, YOFF, ZOFF },
    { XOFF, YOFF, ZOFF },
    { XOFF, -YOFF, ZOFF },
    { -XOFF, -YOFF, ZOFF },
    }, simCore::COORD_SYS_ECEF);

  // Previous testing demonstrated that point height was a cause for failure. This
  // is still tested here. The algorithm normalizes the input test point against the
  // spherical earth model's surface, which means the looping should have no impact.
  for (int heightMult = 0; heightMult < 100; ++heightMult)
  {
    const double height = 50. * (heightMult + 1.);
    if (!fence.contains({ 0,0,height }))
    {
      ++rv;
      std::cerr << "cornerTestIntersect() Fail Line " << __LINE__ << " with height " << height << "\n";
    }
  }

  // Repeat the test with shifted points to check for issues on the first/last
  fence.set({
    { XOFF, YOFF, ZOFF },
    { XOFF, -YOFF, ZOFF },
    { -XOFF, -YOFF, ZOFF },
    { -XOFF, YOFF, ZOFF },
    }, simCore::COORD_SYS_ECEF);
  for (int heightMult = 0; heightMult < 100; ++heightMult)
  {
    const double height = 50. * (heightMult + 1.);
    if (!fence.contains({ 0,0,height }))
    {
      ++rv;
      std::cerr << "cornerTestIntersect() Fail Line " << __LINE__ << " with height " << height << "\n";
    }
  }

  fence.set({
    { XOFF, -YOFF, ZOFF },
    { -XOFF, -YOFF, ZOFF },
    { -XOFF, YOFF, ZOFF },
    { XOFF, YOFF, ZOFF },
    }, simCore::COORD_SYS_ECEF);
  for (int heightMult = 0; heightMult < 100; ++heightMult)
  {
    const double height = 50. * (heightMult + 1.);
    if (!fence.contains({ 0,0,height }))
    {
      ++rv;
      std::cerr << "cornerTestIntersect() Fail Line " << __LINE__ << " with height " << height << "\n";
    }
  }

  fence.set({
    { -XOFF, -YOFF, ZOFF },
    { -XOFF, YOFF, ZOFF },
    { XOFF, YOFF, ZOFF },
    { XOFF, -YOFF, ZOFF },
    }, simCore::COORD_SYS_ECEF);
  for (int heightMult = 0; heightMult < 100; ++heightMult)
  {
    const double height = 50. * (heightMult + 1.);
    if (!fence.contains({ 0,0,height }))
    {
      ++rv;
      std::cerr << "cornerTestIntersect() Fail Line " << __LINE__ << " with height " << height << "\n";
    }
  }

  return rv;
}

int testGeoFence2DPolygon()
{
  int rv = 0;
  // data from SDK-57
  const double origin[2] = { 25.241743624 * simCore::DEG2RAD, 55.7572044591 * simCore::DEG2RAD };
  const double pnt[4][2] = {
    { 33.6088966401 * simCore::DEG2RAD, 40.9353048334 * simCore::DEG2RAD},
    { 15.5538308169 * simCore::DEG2RAD, 40.9353048334 * simCore::DEG2RAD},
    { 15.5538308169 * simCore::DEG2RAD, 68.5410128577 * simCore::DEG2RAD},
    { 33.6088966401 * simCore::DEG2RAD, 68.5410128577 * simCore::DEG2RAD}
  };

  /*  relative orientation of points:
        0            3

                 or


        1            2
  */

  {
    simCore::GeoFence geoFence;
    // empty filter should fail validate
    rv += SDK_ASSERT(!geoFence.valid());

    // only one vertex, should fail
    std::vector<simCore::Vec3> vertices;
    vertices.push_back(simCore::Vec3(origin[0], origin[1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);
    // not enough vertices
    rv += SDK_ASSERT(!geoFence.valid());
  }

  // test polygon validity, only two vertices
  {
    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    for (size_t i = 0; i < 2; ++i)
      vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);
    // not enough vertices
    rv += SDK_ASSERT(!geoFence.valid());
  }

  // the full polygon
  {
    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    for (size_t i = 0; i < 4; ++i)
      vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);
    rv += SDK_ASSERT(geoFence.valid());
  }

  // create various convex and concave polygons using origin as first vertex with three other vertices
  // polygon: or/0/1/2
  {
    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    vertices.push_back(simCore::Vec3(origin[0], origin[1], 0.0));
    for (size_t i = 0; i < 3; ++i)
      vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
    vertices.push_back(simCore::Vec3(origin[0], origin[1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);

    // passes convex test
    rv += SDK_ASSERT(geoFence.valid());
    // Test just left of origin, rather than origin exactly, due to on-edge cases
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0] - FLT_EPSILON, origin[1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0] + FLT_EPSILON, origin[1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0], origin[1] + FLT_EPSILON, 0.0))));
  }

  // different windings:
  // polygon: or/3/0/1
  {
    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    vertices.push_back(simCore::Vec3(origin[0], origin[1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[3][0], pnt[3][1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[1][0], pnt[1][1], 0.0));
    vertices.push_back(simCore::Vec3(origin[0], origin[1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);

    // passes convex test
    rv += SDK_ASSERT(geoFence.valid());
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0], origin[1] - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0] - FLT_EPSILON, origin[1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0], origin[1] + FLT_EPSILON, 0.0))));
  }

  return rv;
}


// test large area that crosses 0deg lines
int testGeoFilter2DPolygonZeroDeg()
{
  int rv = 0;
  const double pnt[4][2] = {
    { 40.0 * simCore::DEG2RAD, -10.0 * simCore::DEG2RAD},
    { -20.0 * simCore::DEG2RAD, -10.0 * simCore::DEG2RAD},
    { -20.0 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD},
    { 40.0 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD}
  };

  /*  relative orientation of points:
        0            3

                 or


        1            2
  */
  // validate the polygon
  simCore::GeoFence geoFence;
  std::vector<simCore::Vec3> vertices;

  for (size_t i = 0; i < 4; ++i)
    vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
  vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
  geoFence.set(vertices, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(geoFence.valid());

  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0], pnt[0][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] + FLT_EPSILON, pnt[0][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0], pnt[0][1] - FLT_EPSILON, 0.0))));
  }
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0], pnt[1][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] - FLT_EPSILON, pnt[1][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0], pnt[1][1] - FLT_EPSILON, 0.0))));
  }

  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] + FLT_EPSILON, pnt[2][1] - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] - FLT_EPSILON, pnt[2][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0], pnt[2][1] + FLT_EPSILON, 0.0))));
  }
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] - FLT_EPSILON, pnt[3][1] - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] + FLT_EPSILON, pnt[3][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0], pnt[3][1] + FLT_EPSILON, 0.0))));
  }

  // large rectangles are not simple LLA rectangles
  // northern latitudinal edge of rectangle, edge is in region, and have to exceed latitude by large amount to go out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(40.0 * simCore::DEG2RAD, 0.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(40.0 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(40.0 * simCore::DEG2RAD, -0.01 * simCore::DEG2RAD, 0.0))));

    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(40.5 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(41.0 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
  }

  // southern latitudinal edge of rectangle, edge is in region, and have to exceed latitude by large amount to go out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-20.0 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-20.5 * simCore::DEG2RAD, 0.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-20.5 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-20.5 * simCore::DEG2RAD, -0.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-21.0 * simCore::DEG2RAD, 0.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-21.0 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-21.0 * simCore::DEG2RAD, -0.01 * simCore::DEG2RAD, 0.0))));
  }

  // eastern longitudinal edge of rectangle, edge is in region, and have to exceed latitude by large amount to go out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 20.1 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 20.1 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 20.1 * simCore::DEG2RAD, 0.0))));
  }

  // western longitudinal edge of rectangle, edge is not in region and apparently more pinching of edge here than on eastern edge
  {
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, -20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 0.1 - 20.0 * simCore::DEG2RAD, 0.0))));

    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 0.2 - 20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 0.2 - 20.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 0.2 - 20.0 * simCore::DEG2RAD, 0.0))));
  }

  return rv;
}

// test a small region that crosses the dateline
int testGeoFilter2DPolygonDateline()
{
  int rv = 0;
  double pnt[4][2] = {
    { 20.0 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD},
    { -40.0 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD},
    { -40.0 * simCore::DEG2RAD, 200.0 * simCore::DEG2RAD},
    { 20.0 * simCore::DEG2RAD, 200.0 * simCore::DEG2RAD} };

  /*  relative orientation of points:
        0            3

           or


        1            2
  */
  // validate the polygon
  simCore::GeoFence geoFence;
  std::vector<simCore::Vec3> vertices;
  for (size_t i = 0; i < 4; ++i)
    vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
  vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
  geoFence.set(vertices, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(geoFence.valid());

  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] - FLT_EPSILON, pnt[0][1] + FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] + FLT_EPSILON, pnt[0][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0], pnt[0][1] - FLT_EPSILON, 0.0))));
  }
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] + FLT_EPSILON, pnt[1][1] + FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] - FLT_EPSILON, pnt[1][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0], pnt[1][1] - FLT_EPSILON, 0.0))));
  }
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] + FLT_EPSILON, pnt[2][1] - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] - FLT_EPSILON, pnt[2][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0], pnt[2][1] + FLT_EPSILON, 0.0))));
  }
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] - FLT_EPSILON, pnt[3][1] - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] + FLT_EPSILON, pnt[3][1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0], pnt[3][1] + FLT_EPSILON, 0.0))));
  }

  // northern latitudinal edge of rectangle, edge is in region, and have to exceed latitude by large amount to go out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(20.5 * simCore::DEG2RAD, 179.9 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(20.5 * simCore::DEG2RAD, 180.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(20.5 * simCore::DEG2RAD, 180.1 * simCore::DEG2RAD, 0.0))));

    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(21.0 * simCore::DEG2RAD, 180.1 * simCore::DEG2RAD, 0.0))));

    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(20.0 * simCore::DEG2RAD, -180.0 * simCore::DEG2RAD, 0.0))));
  }
  // southern latitudinal edge of rectangle, edge is in region, and have to exceed latitude by large amount to go out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-40.0 * simCore::DEG2RAD, 179.9 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-40.5 * simCore::DEG2RAD, 179.9 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-40.5 * simCore::DEG2RAD, 180.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-40.5 * simCore::DEG2RAD, 180.1 * simCore::DEG2RAD, 0.0))));

    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-41.0 * simCore::DEG2RAD, 179.9 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-41.0 * simCore::DEG2RAD, 180.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-41.0 * simCore::DEG2RAD, 180.1 * simCore::DEG2RAD, 0.0))));
  }

  // eastern longitudinal edge of rectangle, edge is in region, small delta takes it out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 200.0 * simCore::DEG2RAD - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 200.0 * simCore::DEG2RAD - FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 200.0 * simCore::DEG2RAD - FLT_EPSILON, 0.0))));

    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 200.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 200.01 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 200.01 * simCore::DEG2RAD, 0.0))));
  }

  // western longitudinal edge of rectangle, edge is in region, small delta takes it out of region
  {
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD + FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD + FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD + FLT_EPSILON, 0.0))));
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 170.0 * simCore::DEG2RAD + FLT_EPSILON, 0.0))));

    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-0.01 * simCore::DEG2RAD, 169.99 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.0 * simCore::DEG2RAD, 169.99 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(0.01 * simCore::DEG2RAD, 169.99 * simCore::DEG2RAD, 0.0))));
  }

  return rv;
}


// test a small region that wraps the n pole
int testGeoFilter2DPolygonNPole()
{
  int rv = 0;

  /*  relative orientation of points:
        0            3

           or


        1            2
  */
  // polygon with 89.99 as max lat
  {
    const double origin[2] = { 80.0 * simCore::DEG2RAD, 20.0 * simCore::DEG2RAD };
    const double pnt[4][2] = {
      { 89.99 * simCore::DEG2RAD, 10.0 * simCore::DEG2RAD},
      { 70.0 * simCore::DEG2RAD, 10.0 * simCore::DEG2RAD},
      { 70.0 * simCore::DEG2RAD, 140.0 * simCore::DEG2RAD},
      { 89.99 * simCore::DEG2RAD, 140.0 * simCore::DEG2RAD}
    };

    // validate the polygon
    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    for (size_t i = 0; i < 4; ++i)
      vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);
    rv += SDK_ASSERT(geoFence.valid());

    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] - FLT_EPSILON, pnt[0][1] + FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] + FLT_EPSILON, pnt[0][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0], pnt[0][1] - FLT_EPSILON, 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] + FLT_EPSILON, pnt[1][1] + FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] - FLT_EPSILON, pnt[1][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0], pnt[1][1] - FLT_EPSILON, 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] + FLT_EPSILON, pnt[2][1] - FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] - FLT_EPSILON, pnt[2][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0], pnt[2][1] + FLT_EPSILON, 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] - FLT_EPSILON, pnt[3][1] - FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] + FLT_EPSILON, pnt[3][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0], pnt[3][1] + FLT_EPSILON, 0.0))));
    }

    // test exclusions
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 141.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 9.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 145.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 5.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 150 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, -175.0 * simCore::DEG2RAD, 0.0))));

    // origin point should be in region
    rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0], origin[1], 0.0))));

    // other side of the globe
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-origin[0], -origin[1], 0.0))));

    // various points that should be excluded
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-origin[0], origin[1], 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(origin[0], -origin[1], 0.0))));

    for (size_t i = 0; i < 4; ++i)
    {
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-pnt[i][0], -pnt[i][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-pnt[i][0], pnt[i][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[i][0], -pnt[i][1], 0.0))));
    }
  }

  // same polygon, but using 90 as max lat;
  {
    const double pnt[4][2] = {
      { 90.0 * simCore::DEG2RAD, 10.0 * simCore::DEG2RAD},
      { 70.0 * simCore::DEG2RAD, 10.0 * simCore::DEG2RAD},
      { 70.0 * simCore::DEG2RAD, 140.0 * simCore::DEG2RAD},
      { 90.0 * simCore::DEG2RAD, 140.0 * simCore::DEG2RAD}
    };

    simCore::GeoFence geoFence;
    std::vector<simCore::Vec3> vertices;
    for (size_t i = 0; i < 4; ++i)
      vertices.push_back(simCore::Vec3(pnt[i][0], pnt[i][1], 0.0));
    vertices.push_back(simCore::Vec3(pnt[0][0], pnt[0][1], 0.0));
    geoFence.set(vertices, simCore::COORD_SYS_LLA);
    rv += SDK_ASSERT(geoFence.valid());

    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] - FLT_EPSILON, pnt[0][1] + FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[0][0] + FLT_EPSILON, pnt[0][1], 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] + FLT_EPSILON, pnt[1][1] + FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0] - FLT_EPSILON, pnt[1][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[1][0], pnt[1][1] - FLT_EPSILON, 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] + FLT_EPSILON, pnt[2][1] - FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0] - FLT_EPSILON, pnt[2][1], 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[2][0], pnt[2][1] + FLT_EPSILON, 0.0))));
    }
    {
      rv += SDK_ASSERT(geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] - FLT_EPSILON, pnt[3][1] - FLT_EPSILON, 0.0))));
      rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(pnt[3][0] + FLT_EPSILON, pnt[3][1], 0.0))));
    }

    // test exclusions
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 145.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 5.0 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 150 * simCore::DEG2RAD, 0.0))));
    rv += SDK_ASSERT(!geoFence.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(89.9 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 0.0))));
  }

  return rv;
}

}

int GeoFenceTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testConcaveEcefIntersect() == 0);
  rv += SDK_ASSERT(cornerTestIntersect() == 0);

  rv += SDK_ASSERT(testGeoFence2DPolygon() == 0);
  rv += SDK_ASSERT(testGeoFilter2DPolygonZeroDeg() == 0);
  rv += SDK_ASSERT(testGeoFilter2DPolygonDateline() == 0);
  rv += SDK_ASSERT(testGeoFilter2DPolygonNPole() == 0);

  std::cout << "GeoFenceTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";
  return rv;
}

