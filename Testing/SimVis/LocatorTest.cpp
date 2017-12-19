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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Common/Version.h"
#include "simVis/Locator.h"

namespace
{

int testOnePositionOrientation(simVis::Locator* loc, const simCore::Vec3& pos, const simCore::Vec3& ori, const simCore::Vec3& oriOffset,
  double timestamp, simCore::CoordinateSystem coordsys)
{
  const int eciRefTime = 100000;
  int rv = 0;

  // Create locator with coordinate system given by coordsys
  simCore::Coordinate coord(
      coordsys,
      pos,
      ori,
      timestamp - eciRefTime);

  // Test both the setCoordinate and setLocalOffsets methods
  loc->setCoordinate(coord, timestamp, eciRefTime);
  loc->setLocalOffsets(simCore::Vec3(), oriOffset);

  simCore::Coordinate coordLla;
  // Convert the input coordinate to LLA
  if (coordsys != simCore::COORD_SYS_LLA)
  {
    // Since oriOffset was added to the locator as a local offset, we should do the same for the converted coordinate
    coord.setOrientation(ori.yaw() + oriOffset.yaw(), ori.pitch() + oriOffset.pitch(), ori.roll() + oriOffset.roll());

    simCore::CoordinateConverter cc;
    cc.convert(coord, coordLla, simCore::COORD_SYS_LLA);
  }
  else
  {
    coordLla.setPosition(pos);
    coordLla.setOrientation(ori.yaw() + oriOffset.yaw(), ori.pitch() + oriOffset.pitch(), ori.roll() + oriOffset.roll());
  }

  // Retrieve the output coordinate in LLA
  simCore::Vec3 outPosition;
  simCore::Vec3 outOrientation;
  loc->getLocatorPositionOrientation(&outPosition, &outOrientation, simCore::COORD_SYS_LLA);
  double outTime = loc->getElapsedEciTime();

  // Check the position
  rv += SDK_ASSERT(simCore::areAnglesEqual(outPosition.lat(), coordLla.lat(), 0.00001));
  rv += SDK_ASSERT(simCore::areAnglesEqual(outPosition.lon(), coordLla.lon(), 0.00001));
  rv += SDK_ASSERT(simCore::areEqual(outPosition.alt(), coordLla.alt(), 0.01));

  // Check the orientation
  rv += SDK_ASSERT(simCore::areAnglesEqual(outOrientation.yaw(), coordLla.yaw(), 0.00001));
  rv += SDK_ASSERT(simCore::areAnglesEqual(outOrientation.pitch(), coordLla.pitch(), 0.00001));
  rv += SDK_ASSERT(simCore::areAnglesEqual(outOrientation.roll(), coordLla.roll(), 0.00001));

  // Check the elapsed ECI time
  rv += SDK_ASSERT(simCore::areEqual(eciRefTime + outTime, timestamp));

  return rv;
}

int testOrientation(simVis::Locator* loc, const simCore::Vec3& pos, simCore::CoordinateSystem coordsys)
{
  using namespace simCore;
  int rv = 0;

  // Test extremes
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(180.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-180.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(0.0 * DEG2RAD, 90.0 * DEG2RAD, 0.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(0.0 * DEG2RAD, -90.0 * DEG2RAD, 0.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, 180.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, -180.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);

  // Test local offsets
  // We can have orientations and offsets in the same axes, but anything else will fail as we're only adding them together instead of
  // using rotation matrices.
  rv += testOnePositionOrientation(loc, pos, Vec3(), simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(30.0 * DEG2RAD, 0.0, 0.0), simCore::Vec3(180.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(60.0 * DEG2RAD, 0.0, 0.0), simCore::Vec3(-180.0 * DEG2RAD, 0.0 * DEG2RAD, 0.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(0.0, -30.0 * DEG2RAD, 0.0), simCore::Vec3(0.0 * DEG2RAD, 90.0 * DEG2RAD, 0.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(0.0, 60.0 * DEG2RAD, 0.0), simCore::Vec3(0.0 * DEG2RAD, -90.0 * DEG2RAD, 0.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(0.0, 0.0, 30.0 * DEG2RAD), simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, 180.0 * DEG2RAD), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, Vec3(0.0, 0.0, 60.0 * DEG2RAD), simCore::Vec3(0.0 * DEG2RAD, 0.0 * DEG2RAD, -180.0 * DEG2RAD), 145000, coordsys);

  // Test random orientation
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(15.0 * DEG2RAD, 30.0 * DEG2RAD, 45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(15.0 * DEG2RAD, 30.0 * DEG2RAD, -45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(15.0 * DEG2RAD, -30.0 * DEG2RAD, 45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(15.0 * DEG2RAD, -30.0 * DEG2RAD, -45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-15.0 * DEG2RAD, 30.0 * DEG2RAD, 45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-15.0 * DEG2RAD, 30.0 * DEG2RAD, -45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-15.0 * DEG2RAD, -30.0 * DEG2RAD, 45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-15.0 * DEG2RAD, -30.0 * DEG2RAD, -45.0 * DEG2RAD), simCore::Vec3(), 145000, coordsys);
  // Test an elapsedEciTime of 0
  rv += testOnePositionOrientation(loc, pos, simCore::Vec3(-15.0 * DEG2RAD, -30.0 * DEG2RAD, -45.0 * DEG2RAD), simCore::Vec3(), 100000, coordsys);

  return rv;
}

int testGetLocatorPositionOrientation(simVis::Locator* loc)
{
  using namespace simCore;
  int rv = 0;

  // Test extremes
  rv += testOrientation(loc, simCore::Vec3(0.0 * DEG2RAD,    0.0 * DEG2RAD,   0), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(90.0 * DEG2RAD, 0.0 * DEG2RAD, 0), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(-90.0 * DEG2RAD, 0.0 * DEG2RAD, 0), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(0.0 * DEG2RAD, 180.0 * DEG2RAD, 0), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(0.0 * DEG2RAD, -180.0 * DEG2RAD, 0), simCore::COORD_SYS_LLA);

  // Test random position
  rv += testOrientation(loc, simCore::Vec3(22.0 * DEG2RAD, 123.0 * DEG2RAD, 200), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(-22.0 * DEG2RAD, 123.0 * DEG2RAD, 200), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(-22.0 * DEG2RAD, -123.0 * DEG2RAD, 200), simCore::COORD_SYS_LLA);
  rv += testOrientation(loc, simCore::Vec3(22.0 * DEG2RAD, -123.0 * DEG2RAD, 200), simCore::COORD_SYS_LLA);

  // Test a few ECI coordinates
  rv += testOrientation(loc, simCore::Vec3(5646775.942, 1959614.906, 2223992.894), simCore::COORD_SYS_ECI);
  rv += testOrientation(loc, simCore::Vec3(5645872.327, 1962696.334, 2223571.379), simCore::COORD_SYS_ECI);
  rv += testOrientation(loc, simCore::Vec3(3148721.910, 4176471.627, 3637866.9093), simCore::COORD_SYS_ECI);
  rv += testOrientation(loc, simCore::Vec3(-2535761.250, -4574634.391, 3637866.914), simCore::COORD_SYS_ECI);

  return rv;
}

}

int LocatorTest(int argc, char* argv[])
{
  int rv = 0;

  // Check the SIMDIS SDK version
  simCore::checkVersionThrow();

  // Create a SRS on WGS84
  osg::ref_ptr<osgEarth::SpatialReference> srs = osgEarth::SpatialReference::create("wgs84");
  // Allocate a Locator for testing
  osg::ref_ptr<simVis::Locator> loc = new simVis::Locator(srs.get());

  // Run tests
  rv += testGetLocatorPositionOrientation(loc.get());

  return rv;
}
