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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_MAGNETICVARIANCE_H
#define SIMCORE_CALC_MAGNETICVARIANCE_H

#include "simCore/Common/Export.h"

namespace simCore {

class Vec3;
class TimeStamp;

/// Enumeration of magnetic variance datum type constants
enum MagneticVariance
{
  MAGVAR_TRUE=1,  /**< No variance, True Heading */
  MAGVAR_WMM,     /**< World Magnetic Model (WMM) */
  MAGVAR_USER     /**< User Defined Magnetic Variance Offset */
};

/** Provides magnetic variance from the NOAA WMM */
class SDKCORE_EXPORT WorldMagneticModel
{
public:
  /** Public constructor */
  WorldMagneticModel();
  /** Private destructor */
  virtual ~WorldMagneticModel();

  /**
   * Calculates the magnetic variance at the given lat/lon and time.
   * @param lla Geodetic position in radians and meters.
   * @param ordinalDay Ordinal day of year (e.g. 0 for January 1st, 365 for December 31 on most years)
   * @param year Year value from [1985-2020].  WMM cannot be used outside these bounds.
   * @param varianceRad Radian value of the magnetic variance for the given time at the position.
   * @return 0 on success, non-zero on error
   */
  int calculateMagneticVariance(const simCore::Vec3& lla, int ordinalDay, int year, double& varianceRad);

  /**
   * Calculates the magnetic variance at the given lat/lon and time.
   * @param lla Geodetic position in radians and meters.
   * @param timeStamp Time value between the years [1985-2020].  WMM cannot be used outside these bounds.
   * @param varianceRad Radian value of the magnetic variance for the given time at the position.
   * @return 0 on success, non-zero on error
   */
  int calculateMagneticVariance(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, double& varianceRad);

  /**
   * Converts a true bearing to a magnetic bearing at the given position and time.
   * @param lla Geodetic position in radians and meters.
   * @param timeStamp Time value between the years [1985-2020].  WMM cannot be used outside these bounds.
   * @param bearingRad On input, a true bearing value in radians.  On output, a magnetic bearing in radians.
   * @return 0 on success, non-zero on error
   */
  int calculateMagneticBearing(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, double& bearingRad);

  /**
   * Converts a magnetic bearing to a true bearing at the given position and time.
   * @param lla Geodetic position in radians and meters.
   * @param timeStamp Time value between the years [1985-2020].  WMM cannot be used outside these bounds.
   * @param bearingRad On input, a magnetic bearing value in radians.  On output, a true bearing in radians.
   * @return 0 on success, non-zero on error
   */
  int calculateTrueBearing(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, double& bearingRad);

private:
  class GeoMag;
  GeoMag* geomag_;
};

}

#endif /* SIMCORE_CALC_MAGNETICVARIANCE_H */
