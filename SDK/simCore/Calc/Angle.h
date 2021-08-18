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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_ANGLE_H
#define SIMCORE_CALC_ANGLE_H

#include <cmath>
#include "simCore/Common/Export.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/MathConstants.h"

namespace simCore
{
  ///Radian to degree conversion factor
  static const double RAD2DEG = 180.0 / M_PI;
  ///Degree to radian conversion factor
  static const double DEG2RAD = M_PI / 180.0;

  //--------------------------------------------------------------------------
  //---general functions

  /**
  * Adjusts incoming angle to fit the range [0, 2PI)
  * @param[in ] in Input angle (rad)
  * @return equivalent angle between 0 and 2PI (rad)
  */
  inline double angFix2PI(double in)
  {
    // According to the Intel HotSpot, the call to fmod is expensive, so only make the call if necessary
    if ((in < 0.0) || (in >= M_TWOPI))
    {
      in = fmod(in, M_TWOPI);

      if (fabs(in) < 1e-10) // If really close to zero than return zero instead of M_TWOPI
        in = 0.0;
      else if (in < 0.0)
        in += M_TWOPI;
    }
    return in;
  }

  /**
  * Adjusts incoming angle to fit the range [-M_PI, M_PI]
  * @param[in ] in Input angle (rad)
  * @return equivalent angle between -M_PI and M_PI (rad)
  */
  inline double angFixPI(double in)
  {
    if (fabs(in) <= M_PI) return in;

    in = angFix2PI(in);

    return in > M_PI ? in - M_TWOPI : in;
  }

  /**
  * Clamps incoming angle to fit the range [-PI_2, PI_2]
  * This is intended for use with latitude or elevation angle values that are already known to be valid.
  * This routine does not ensure that inputs can be validly converted to [-PI_2, PI_2],
  * and may have unintended outcomes if input is not validated before this is called.
  * @param[in ] in Input angle (rad)
  * @return equivalent angle between -PI_2 and PI_2 (rad)
  */
  inline double angFixPI2(double in)
  {
    in = angFixPI(in);
    if (in > M_PI_2)
      return M_PI_2;

    else if (in < -M_PI_2)
      in = -M_PI_2;

    return in;
  }

  /**
  * Wraps an angle between -PI_2 and PI_2
  * @param[in ] in Input angle (rad)
  * @return angle between -PI_2 and PI_2 (rad)
  */
  inline double angWrapPI2(double in)
  {
    in = angFixPI(in);
    if (in > M_PI_2)
      in = M_PI - in;
    else if (in < -M_PI_2)
      in = -M_PI - in;
    return in;
  }

  /**
  * Adjusts incoming angle to fit the range [0, 360)
  * @param[in ] in angle (deg)
  * @return equivalent angle between 0 and 360 (deg)
  */
  inline double angFix360(double in)
  {
    // According to the Intel HotSpot, the call to fmod is expensive, so only make the call if necessary
    if ((in < 0.0) || (in >= 360.0))
    {
      in = fmod(in, 360.0);

      if (in < 0.0)
        in += 360.0;
    }
    return in;
  }

  /**
  * Adjusts incoming angle to fit the range [-180, 180]
  * @param[in ] in angle (deg)
  * @return equivalent angle between -180 and 180 (deg)
  */
  inline double angFix180(double in)
  {
    if (fabs(in) <= 180.) return in;
    in = angFix360(in);
    return in > 180. ? in - 360. : in;
  }

  /**
  * Clamps incoming angle to fit the range [-90, 90]
  * This is intended for use with latitude or elevation angle values that are already known to be valid.
  * This routine does not ensure that inputs can be validly converted to [-90, 90],
  * and may have unintended outcomes if input is not validated before this is called.
  * @param[in ] in angle (deg)
  * @return equivalent angle between -90 and 90 (deg)
  */
  inline double angFix90(double in)
  {
    in = angFix180(in);
    if (in > 90.)
      in = 90.;
    else if (in < -90.)
      in = -90.;
    return in;
  }

  /**
  * Wraps incoming angle to fit the range [-90, 90]
  * @param[in ] in angle (deg)
  * @return equivalent angle between -90 and 90 (deg)
  */
  inline double angWrap90(double in)
  {
    in = angFix180(in);
    if (in > 90.)
      in = 180. - in;
    else if (in < -90.)
      in = -180. - in;
    return in;
  }

  /**
  * Returns the inverse cosine with allowance for accumulated error
  * @param[in ] in value between -1 and 1
  * @return equivalent angle between 0 and PI (rad)
  */
  SDKCORE_EXPORT double inverseCosine(double in);

  /**
  * Returns the inverse sine with allowance for accumulated error
  * @param[in ] in value between -1 and 1
  * @return equivalent angle between -PI/2 and PI/2 (rad)
  */
  SDKCORE_EXPORT double inverseSine(double in);

  /**
  * Compares two angle vectors for equality within the specified tolerance
  * @param[in ] u input vector to compare
  * @param[in ] v input vector to compare
  * @param[in ] t comparison tolerance
  * @return true if u is equal to v (within tolerance 't')
  */
  SDKCORE_EXPORT bool v3AreAnglesEqual(const Vec3 &u, const Vec3 &v, double const t = 1.0e-6);

  /**
  * Checks the equality of two angles based on a tolerance
  * @param[in ] angle1 First value to compare (rad)
  * @param[in ] angle2 Second value to compare (rad)
  * @param[in ] t Comparison tolerance
  * @return the equality of two values based on a tolerance
  */
  SDKCORE_EXPORT bool areAnglesEqual(double angle1, double angle2, double t=1.0e-6);

  /**
   * Rotates an angle about another angle.  Given a starting angle (startAngle), a rotation
   * (rotateBy) is applied, and the result is returned.  This can be useful for converting
   * body angles to true angles.
   * Example: startAngle represents a platform orientation of 15,5,-90.  The rotateBy represents
   *  a body angle of 0,15,0.  The resulting rotation would be something close to 0,5,-90, because
   *  the platform's roll would cause the elevation shift to move the angle in azimuth closer to 0.
   * @param startAngle Starting angle; in body angle cases, this is typically the platform orientation  (rad)
   * @param rotateBy Rotational angle; in body angle cases, this is typically the azimuth/elevation (rad)
   * @return Rotated angle; in body angle cases, this is the true angle (rad)
   */
  SDKCORE_EXPORT simCore::Vec3 rotateEulerAngle(const simCore::Vec3& startAngle, const simCore::Vec3& rotateBy);

  /** Enumerates extents of angle values in radians */
  enum AngleExtents
  {
    /** Angle is valid from [-inf,+inf] */
    ANGLEEXTENTS_ALL,

    /** Angle is valid from [0,M_TWOPI); @see angFix2PI() */
    ANGLEEXTENTS_TWOPI,
    /** Angle is valid from [-M_PI,+M_PI]; @see angFixPI() */
    ANGLEEXTENTS_PI,
    /** Angle is valid from [-M_PI_2,+M_PI_2]; @see angFixPI2() */
    ANGLEEXTENTS_PI_2
  };

  /**
   * Wraps (modulates) the radian angle according to the requested angle extents.
   * @param radianAngle Angle value on which to operate, in radians.
   * @param extents Indicates modulation mode for the angle.
   * @return Modulated angle in radians based on requested output extents.
   */
  SDKCORE_EXPORT double angFix(double radianAngle, AngleExtents extents);

  /**
   * Wraps (modules) the degree angle according to the degrees equivalent of the
   * requested angle extents.  For example, ANGLEEXTENTS_TWOPI will wrap between [0,360).
   * @param degreeAngle Angle value on which to operate, in degrees.
   * @param extents Indicates modulation mode for the angle.  The degree-based
   *    equivalent will be used, e.g. ANGLEEXTENTS_PI will wrap [-180,+180].
   * @return Moduled angle in degrees based on requested output extents.
   */
  SDKCORE_EXPORT double angFixDegrees(double degreeAngle, AngleExtents extents);

  /**
   * Given two angles on a circle, calculates the angle difference between them.  The
   * input values are projected onto a circle (i.e. wrapped), then compared.  The
   * output is angle delta applied on "fromRad" to reach the equivalent angle "toRad",
   * and will always be in the range (-PI,PI].  This method expects values in radians.
   * @param fromRad From angle, in radians
   * @param toRad To angle, in radians
   * @return Radians value from (-PI,PI] that when added to fromRad will be an equivalent
   *   angle to toRad.  For example, angleDifference(0.4, 0.1) == -0.3.
   */
  SDKCORE_EXPORT double angleDifference(double fromRad, double toRad);

  /**
   * Degrees-based convenience wrapper for angleDifference.  Calculates difference
   * between two degree angles.
   * @see simCore::angleDifference()
   * @param fromDeg From angle, in degrees
   * @param toDeg To angle, in degrees
   * @return Degrees value from (-180,180] that when added to fromDeg will be an
   *    equivalent angle to toDeg.  For example, angleDifferenceDeg(4.0, 1.0) == -3.0.
   */
  SDKCORE_EXPORT double angleDifferenceDeg(double fromDeg, double toDeg);

} // End of namespace simCore

#endif /* SIMCORE_CALC_ANGLE_H */
