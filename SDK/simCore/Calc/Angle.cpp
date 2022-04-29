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
#include <complex>
#include <cassert>
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Angle.h"

//------------------------------------------------------------------------

/// Returns the inverse cosine with allowance for accumulated error
double simCore::inverseCosine(double in)
{
  // Assert if off by more than an accumulated error
  assert((in > -1.0001) && (in < 1.0001));

  // Guard against accumulated math error that results in an invalid argument value
  if (in <= -1.0)
    return M_PI;

  // Guard against accumulated math error that results in an invalid argument value
  if (in >= 1.0)
    return 0.0;

  return acos(in);
}

/// Returns the inverse sine with allowance for accumulated error
double simCore::inverseSine(double in)
{
  // Assert if off by more than an accumulated error
  assert((in > -1.0001) && (in < 1.0001));

  // Guard against accumulated math error that results in an invalid argument value
  if (in <= -1.0)
    return -M_PI_2;

  // Guard against accumulated math error that results in an invalid argument value
  if (in >= 1.0)
    return M_PI_2;

  return asin(in);
}

/// Compares two angle vectors for equality within the specified tolerance
bool simCore::v3AreAnglesEqual(const Vec3 &u, const Vec3 &v, double const t)
{
  return simCore::areAnglesEqual(u[0], v[0], t) && simCore::areAnglesEqual(u[1], v[1], t) && simCore::areAnglesEqual(u[2], v[2], t);
}

/// Checks the equality of two angles based on a tolerance
bool simCore::areAnglesEqual(double angle1, double angle2, double t)
{
  if (simCore::areEqual(angle1, angle2, t))
    return true;

  // Make sure the values are in the same range
  angle1 = simCore::angFixPI(angle1);
  angle2 = simCore::angFixPI(angle2);

  if (simCore::areEqual(angle1, angle2, t))
    return true;

  // Test for 180 versus -180 which are the same
  if (simCore::areEqual(fabs(angle1), M_PI, t) && simCore::areEqual(fabs(angle2), M_PI, t))
    return true;

  return false;
}

/// Rotates one angle by another
simCore::Vec3 simCore::rotateEulerAngle(const simCore::Vec3& startAngle, const simCore::Vec3& rotateBy)
{
  // Create quaternions from the rotations
  double qStartAngle[4];
  simCore::d3EulertoQ(startAngle, qStartAngle);
  double qRotateBy[4];
  simCore::d3EulertoQ(rotateBy, qRotateBy);

  // Multiply the rotations, and convert back out to Euler
  double qFinal[4];
  simCore::dQMult(qStartAngle, qRotateBy, qFinal);
  simCore::Vec3 rv;
  simCore::d3QtoEuler(qFinal, rv);
  return rv;
}

double simCore::angFix(double radianAngle, simCore::AngleExtents extents)
{
  switch (extents)
  {
  case ANGLEEXTENTS_TWOPI:
    return simCore::angFix2PI(radianAngle);
  case ANGLEEXTENTS_PI:
    return simCore::angFixPI(radianAngle);
  case ANGLEEXTENTS_PI_2:
    return simCore::angFixPI2(radianAngle);
  case ANGLEEXTENTS_ALL:
    return radianAngle;
  }
  // Invalid enumeration, developer error
  assert(0);
  return radianAngle;
}

double simCore::angFixDegrees(double degreeAngle, simCore::AngleExtents extents)
{
  switch (extents)
  {
  case ANGLEEXTENTS_TWOPI:
    return simCore::angFix360(degreeAngle);
  case ANGLEEXTENTS_PI:
    return simCore::angFix180(degreeAngle);
  case ANGLEEXTENTS_PI_2:
    return simCore::angFix90(degreeAngle);
  case ANGLEEXTENTS_ALL:
    return degreeAngle;
  }
  // Invalid enumeration, developer error
  assert(0);
  return degreeAngle;
}

double simCore::angleDifference(double fromRad, double toRad)
{
  // Implementation drew from https://stackoverflow.com/questions/1878907

  // Fix toRad and fromRad from [-PI,PI] inclusive before subtracting
  const double subtracted = simCore::angFixPI(toRad) - simCore::angFixPI(fromRad);
  // Note that we can't rely solely on angFixPI here due to inclusiveness of -PI
  const double fixed = simCore::angFixPI(subtracted);
  return (fixed <= -M_PI) ? (fixed + M_TWOPI) : fixed;
}

double simCore::angleDifferenceDeg(double fromDeg, double toDeg)
{
  return simCore::RAD2DEG * simCore::angleDifference(simCore::DEG2RAD * fromDeg, simCore::DEG2RAD * toDeg);
}

bool simCore::isAngleBetween(double testAngle, double fromAngle, double sweep)
{
  // Reformat with a positive sweep to simplify math
  if (sweep < 0.)
  {
    fromAngle = fromAngle + sweep;
    sweep = -sweep;
  }
  const double diff1 = simCore::angFix2PI(testAngle - fromAngle);
  return diff1 <= sweep;
}

bool simCore::isAngleBetweenDeg(double testAngleDeg, double fromAngleDeg, double sweepDeg)
{
  return simCore::isAngleBetween(simCore::DEG2RAD * testAngleDeg, simCore::DEG2RAD * fromAngleDeg,
    simCore::DEG2RAD * sweepDeg);
}
