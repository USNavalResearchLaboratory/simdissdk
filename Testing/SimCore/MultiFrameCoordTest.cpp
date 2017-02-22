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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/MultiFrameCoordinate.h"

namespace
{

using namespace simCore;

/** Position near DC */
const simCore::Coordinate DC_LLA(simCore::COORD_SYS_LLA, Vec3(38.5 * simCore::DEG2RAD, -75 * simCore::DEG2RAD, 0.0));
const simCore::Coordinate DC_ECEF(simCore::COORD_SYS_ECEF, Vec3(1293596, -4827764, 3949029));
/** Position in Australia */
const simCore::Coordinate AUS_LLA(simCore::COORD_SYS_LLA, Vec3(-37.8 * simCore::DEG2RAD, 145 * simCore::DEG2RAD, 0.0));
const simCore::Coordinate AUS_ECEF(simCore::COORD_SYS_ECEF, Vec3(-4133495, 2894304, -3887927));
/** X-East position of 0,0,0 */
const simCore::Coordinate ZERO_XEAST(simCore::COORD_SYS_XEAST, Vec3(0,0,0));
/** Invalid coordinate */
const simCore::Coordinate INVALID_COORD;

/** Precision comparison for LLA->ECEF or ECEF->LLA for distance values (meters) */
const double DISTANCE_PRECISION_THRESHOLD = 1.0; // meters
/** Precision comparison for LLA->ECEF or ECEF->LLA for lat/lon angle values (radians) */
const double LATLON_PRECISION_THRESHOLD = 0.000001; // radians

int testDefaultConstructor()
{
  int rv = 0;
  MultiFrameCoordinate mfc;
  rv += SDK_ASSERT(!mfc.isValid());
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);
  rv += SDK_ASSERT(mfc.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);
  return rv;
}

int testCoordConstructor()
{
  int rv = 0;

  // Validate a precondition assumption
  rv += SDK_ASSERT(INVALID_COORD.coordinateSystem() == simCore::COORD_SYS_NONE);

  // First pass in an invalid coordinate to make sure that fails gracefully
  MultiFrameCoordinate mfc1(INVALID_COORD);
  rv += SDK_ASSERT(!mfc1.isValid());
  rv += SDK_ASSERT(mfc1.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  // Next pass in an LLA coordinate
  MultiFrameCoordinate mfc2(DC_LLA);
  rv += SDK_ASSERT(mfc2.isValid());
  rv += SDK_ASSERT(mfc2.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfc2.llaCoordinate().position()));

  // Pass in an ECEF coordinate
  MultiFrameCoordinate mfc3(DC_ECEF);
  rv += SDK_ASSERT(mfc3.isValid());
  rv += SDK_ASSERT(mfc3.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_ECEF.position(), mfc3.ecefCoordinate().position()));

  // Pass in a tangent plane and make sure it fails properly
  MultiFrameCoordinate mfc4(ZERO_XEAST);
  rv += SDK_ASSERT(!mfc4.isValid());
  rv += SDK_ASSERT(mfc4.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);
  rv += SDK_ASSERT(mfc4.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  return rv;
}

int testCopyConstructor()
{
  int rv = 0;

  // Make sure an invalid coordinate copies the invalidity
  MultiFrameCoordinate invalid;
  rv += SDK_ASSERT(!invalid.isValid());
  MultiFrameCoordinate invalidCopy(invalid);
  rv += SDK_ASSERT(!invalidCopy.isValid());

  // Copying an LLA should give me an LLA coordinate
  MultiFrameCoordinate mfcLla(DC_LLA);
  MultiFrameCoordinate mfcLlaCopy(mfcLla);
  rv += SDK_ASSERT(mfcLlaCopy.isValid());
  rv += SDK_ASSERT(mfcLlaCopy.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfcLlaCopy.llaCoordinate().position()));

  // Copying an ECEF should give me an ECEF coordinate
  MultiFrameCoordinate mfcEcef(DC_ECEF);
  MultiFrameCoordinate mfcEcefCopy(mfcEcef);
  rv += SDK_ASSERT(mfcEcefCopy.isValid());
  rv += SDK_ASSERT(mfcEcefCopy.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_ECEF.position(), mfcEcefCopy.ecefCoordinate().position()));

  // Test the operator= because it might happen
  mfcEcef = mfcLlaCopy;
  mfcLlaCopy.setCoordinate(AUS_LLA); // Change data to anything but DC_LLA
  rv += SDK_ASSERT(mfcEcef.isValid());
  rv += SDK_ASSERT(mfcEcef.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfcEcef.llaCoordinate().position()));

  return rv;
}

int testClear()
{
  int rv = 0;

  // Make sure an invalid clears to invalid
  MultiFrameCoordinate invalid;
  rv += SDK_ASSERT(!invalid.isValid());
  invalid.clear();
  rv += SDK_ASSERT(!invalid.isValid());
  rv += SDK_ASSERT(invalid.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);
  rv += SDK_ASSERT(invalid.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  // Valid clears to invalid
  MultiFrameCoordinate mfc(DC_LLA);
  rv += SDK_ASSERT(mfc.isValid());
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfc.llaCoordinate().position()));
  mfc.clear();
  rv += SDK_ASSERT(!mfc.isValid());
  rv += SDK_ASSERT(invalid.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);
  rv += SDK_ASSERT(invalid.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  return rv;
}

int testSetCoordinate()
{
  int rv = 0;

  // Configure coordinate with an LLA value
  MultiFrameCoordinate mfc;
  rv += SDK_ASSERT(!mfc.isValid());
  rv += SDK_ASSERT(mfc.setCoordinate(DC_LLA) == 0);
  rv += SDK_ASSERT(mfc.isValid());

  // Check the coordinate value (which we just set from LLA)
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfc.llaCoordinate().position()));

  // Validate that unsetting it with an invalid position clears it out
  rv += SDK_ASSERT(mfc.setCoordinate(INVALID_COORD) != 0);
  rv += SDK_ASSERT(!mfc.isValid());
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  // Set it back to LLA and test the ECEF values...
  rv += SDK_ASSERT(mfc.setCoordinate(DC_LLA) == 0);
  rv += SDK_ASSERT(mfc.isValid());

  // Check the LLA parameter
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_LLA.position(), mfc.llaCoordinate().position()));

  // Verify that the ECEF matches (should generate an ECEF pos)
  rv += SDK_ASSERT(mfc.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_ECEF.position(), mfc.ecefCoordinate().position(), DISTANCE_PRECISION_THRESHOLD));

  // Now set it back to ECEF for another position and test it
  rv += SDK_ASSERT(mfc.setCoordinate(AUS_ECEF) == 0);
  rv += SDK_ASSERT(mfc.isValid());

  // Check the same, but using ecef functions explicitly
  rv += SDK_ASSERT(mfc.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(AUS_ECEF.position(), mfc.ecefCoordinate().position()));

  // Verify the LLA matches (should generate an LLA from the ECEF)
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::areAnglesEqual(AUS_LLA.position().lat(), mfc.llaCoordinate().position().lat(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(AUS_LLA.position().lon(), mfc.llaCoordinate().position().lon(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areEqual(AUS_LLA.position().alt(), mfc.llaCoordinate().position().alt(), DISTANCE_PRECISION_THRESHOLD));

  // Test again (with just DC) using setCoordinate(Coordinate, CoordinateConverter)
  simCore::CoordinateConverter cc;
  // First test with an invalid CC -- should still work (because passing in ECEF and LLA coords)
  rv += SDK_ASSERT(mfc.setCoordinate(DC_ECEF, cc) == 0);
  rv += SDK_ASSERT(mfc.isValid());
  // Verify that the LLA matches (should generate an LLA pos)
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::areAnglesEqual(DC_LLA.position().lat(), mfc.llaCoordinate().position().lat(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(DC_LLA.position().lon(), mfc.llaCoordinate().position().lon(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areEqual(DC_LLA.position().alt(), mfc.llaCoordinate().position().alt(), DISTANCE_PRECISION_THRESHOLD));

  // Initialize the CC and pass in an LLA coord
  cc.setReferenceOrigin(DC_LLA.position()); // Far away from the coord we're actually using
  rv += SDK_ASSERT(mfc.setCoordinate(AUS_LLA, cc) == 0);
  rv += SDK_ASSERT(mfc.isValid());
  // Verify the ECEF matches
  rv += SDK_ASSERT(mfc.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(AUS_ECEF.position(), mfc.ecefCoordinate().position(), DISTANCE_PRECISION_THRESHOLD));

  return rv;
}

int testSetXeastCoordinate()
{
  int rv = 0;

  // Validate that passing in an XEast will fail on construction
  MultiFrameCoordinate mfc(ZERO_XEAST);
  rv += SDK_ASSERT(!mfc.isValid());
  // It also won't work with the typical setCoordinate() call
  rv += SDK_ASSERT(mfc.setCoordinate(ZERO_XEAST) != 0);
  rv += SDK_ASSERT(!mfc.isValid());

  // Set up a Coord Convert that isn't initialized yet, and this should also fail
  simCore::CoordinateConverter cc;
  rv += SDK_ASSERT(mfc.setCoordinate(ZERO_XEAST, cc) != 0);
  rv += SDK_ASSERT(!mfc.isValid());
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_NONE);

  // Initialize the CC and this should work
  cc.setReferenceOrigin(DC_LLA.position());
  rv += SDK_ASSERT(mfc.setCoordinate(ZERO_XEAST, cc) == 0);
  rv += SDK_ASSERT(mfc.isValid());
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() != simCore::COORD_SYS_NONE);
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() != simCore::COORD_SYS_XEAST); // could be LLA or ECEF

  // Test the positions against the reference LLA
  rv += SDK_ASSERT(mfc.llaCoordinate().coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::areAnglesEqual(DC_LLA.position().lat(), mfc.llaCoordinate().position().lat(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(DC_LLA.position().lon(), mfc.llaCoordinate().position().lon(), LATLON_PRECISION_THRESHOLD));
  rv += SDK_ASSERT(simCore::areEqual(DC_LLA.position().alt(), mfc.llaCoordinate().position().alt(), DISTANCE_PRECISION_THRESHOLD));

  // Verify that the ECEF matches (should generate an ECEF pos)
  rv += SDK_ASSERT(mfc.ecefCoordinate().coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::v3AreEqual(DC_ECEF.position(), mfc.ecefCoordinate().position(), DISTANCE_PRECISION_THRESHOLD));

  return rv;
}

}

int MultiFrameCoordTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testDefaultConstructor() == 0);
  rv += SDK_ASSERT(testCoordConstructor() == 0);
  rv += SDK_ASSERT(testCopyConstructor() == 0);
  rv += SDK_ASSERT(testClear() == 0);
  rv += SDK_ASSERT(testSetCoordinate() == 0);
  rv += SDK_ASSERT(testSetXeastCoordinate() == 0);
  return rv;
}
