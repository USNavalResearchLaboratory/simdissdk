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

#include "osg/BoundingSphere"
#include "osg/LineSegment"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Vec3.h"

// There is a corresponding testing in simVk that should match

bool doesLineIntersectSphere(const simCore::Vec3& p1, simCore::Vec3& p2, double radius)
{
  osg::BoundingSphere earthSphere(osg::Vec3(), radius);

  osg::ref_ptr<osg::LineSegment> lineSeg = new osg::LineSegment(
    osg::Vec3(p1.x(), p1.y(), p1.z()),
    osg::Vec3(p2.x(), p2.y(), p2.z()));

  // Test against sphere
  return lineSeg->intersect(earthSphere);
}

int DoesLineIntersectSphereTest(int argc, char* argv[])
{
  int rv = 0;

  // Earth's approximate radius (average, in meters) for ECEF
  const double EARTH_RADIUS_M = 6371000.0;

  // Test Cases:

  // 1. Segment entirely outside, misses sphere
  simCore::Vec3 p1OutsideMissStart(10000000.0, 0.0, 0.0);
  simCore::Vec3 p1OutsideMissEnd(11000000.0, 0.0, 0.0);
  rv += SDK_ASSERT(!doesLineIntersectSphere(p1OutsideMissStart, p1OutsideMissEnd, EARTH_RADIUS_M));

  // 2. Segment starts outside, passes through sphere
  simCore::Vec3 p2ThroughStart(7000000.0, 0.0, 0.0);
  simCore::Vec3 p2ThroughEnd(-7000000.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p2ThroughStart, p2ThroughEnd, EARTH_RADIUS_M));

  // 3. Segment starts inside, exits sphere
  simCore::Vec3 p3StartsInsideStart(100000.0, 0.0, 0.0);
  simCore::Vec3 p3StartsInsideEnd(7000000.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p3StartsInsideStart, p3StartsInsideEnd, EARTH_RADIUS_M));

  // 4. Segment entirely inside sphere
  simCore::Vec3 p4EntirelyInsideStart(100000.0, 0.0, 0.0);
  simCore::Vec3 p4EntirelyInsideEnd(200000.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p4EntirelyInsideStart, p4EntirelyInsideEnd, EARTH_RADIUS_M));

  // 5. Segment tangent to sphere (just touches)
  simCore::Vec3 p5TangentStart(EARTH_RADIUS_M, EARTH_RADIUS_M, 0.0);
  simCore::Vec3 p5TangentEnd(EARTH_RADIUS_M, -EARTH_RADIUS_M, 0.0);
  // This segment passes through (EARTH_RADIUS_M, 0, 0) which is on the sphere
  rv += SDK_ASSERT(doesLineIntersectSphere(p5TangentStart, p5TangentEnd, EARTH_RADIUS_M));

  // 6. Segment exactly on the surface (from one point on surface to another)
  simCore::Vec3 p6OnSurfaceStart(EARTH_RADIUS_M, 0.0, 0.0);
  simCore::Vec3 p6OnSurfaceEnd(0.0, EARTH_RADIUS_M, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p6OnSurfaceStart, p6OnSurfaceEnd, EARTH_RADIUS_M));

  // 7. Segment ends exactly at origin (sphere center)
  simCore::Vec3 p7ToOriginStart(7000000.0, 0.0, 0.0);
  simCore::Vec3 p7ToOriginEnd(0.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p7ToOriginStart, p7ToOriginEnd, EARTH_RADIUS_M));

  // 8. Segment starts exactly at origin
  simCore::Vec3 p8FromOriginStart(0.0, 0.0, 0.0);
  simCore::Vec3 p8FromOriginEnd(7000000.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p8FromOriginStart, p8FromOriginEnd, EARTH_RADIUS_M));

  // 9. Tiny segment far away
  simCore::Vec3 p9TinyFarStart(1e9, 1e9, 1e9);
  simCore::Vec3 p9TinyFarEnd(1e9 + 100, 1e9, 1e9);
  rv += SDK_ASSERT(!doesLineIntersectSphere(p9TinyFarStart, p9TinyFarEnd, EARTH_RADIUS_M));

  // 10. Segment goes from inside to outside, but backwards (p2 is inside, p1 is outside)
  simCore::Vec3 p10BackwardsStart(7000000.0, 0.0, 0.0);
  simCore::Vec3 p10BackwardsEnd(100000.0, 0.0, 0.0);
  rv += SDK_ASSERT(doesLineIntersectSphere(p10BackwardsStart, p10BackwardsEnd, EARTH_RADIUS_M));

  return rv;
}
