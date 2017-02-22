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
#ifndef SIMCORE_STRING_ANGLE_H
#define SIMCORE_STRING_ANGLE_H

#include <string>
#include "simCore/Common/Export.h"

namespace simCore
{
  /// Format to use when encoding a degree symbol
  enum DegreeSymbolFormat
  {
    DEG_SYM_NONE = 0, ///< ""; no symbol
    DEG_SYM_ASCII,    ///< "\xB0"; screen text usable
    DEG_SYM_UNICODE   ///< "\u00B0"; GUI usable
  };

  /**
  * Returns appropriate string for the degree symbol based on specified format
  * @param[in ] fmt Output format for the degree symbol
  * @return string representation of degree symbol
  */
  SDKCORE_EXPORT std::string getDegreeSymbol(DegreeSymbolFormat fmt);

  /**
  * Returns a double representing a geodetic DMD, DMS, or DD string
  * Given a string that is in degrees, degrees-minutes, or degrees-minutes-seconds, this function
  * parses that string and sets the angle value.  If the value is not a valid numeric value an error is returned.
  * @param[in ] degStr Input degree string in DMD, DMS, or DD
  * @param[in ] rads If true, return angle will be in radians, otherwise degrees
  * @param[out] ang Output angle returned
  * @return 0 on success, !0 otherwise
  */
  SDKCORE_EXPORT int getAngleFromDegreeString(const std::string& degStr, bool rads, double& ang);

  /** Display formatting for geodetic (lat/lon) angle values */
  enum GeodeticFormat
  {
    FMT_DEGREES,  ///< Also referred to as DD (degrees decimal)
    FMT_DEGREES_MINUTES,  ///< Also referred to as DMD (degrees minutes, decimal)
    FMT_DEGREES_MINUTES_SECONDS,  ///< Also referred to as DMS
    FMT_RADIANS,
    FMT_BAM,  ///< Corresponds to BAM (Binary Angle Measurement) angle measurement unit
    FMT_MIL,  ///< Corresponds to Angular Mil (NATO variant) angle measurement unit
    FMT_MILLIRADIANS
  };

  /**
   * Formats a latitude value (in radians) into a string value according to the format specification.
   * @param latRadians Latitude value to print to string, in radians.
   * @param format Geodetic output format.
   * @param allNumerics If false, then string will contain hemisphere, and the minutes symbol (') and
   *   and seconds symbol (") for DMD/DMS formats; additionally, values in southern hemisphere will not
   *   show a negative (-) sign, because the "S" is appended.  If true, then the return string will
   *   omit the hemisphere, minutes symbol, seconds symbol, and will include a negative sign if in the
   *   southern hemisphere.
   * @param precision Decimal precision to be used in formatting the value.
   * @param degSymbol Symbol to use when formatting output in DMD, DMS, or DD output
   * @return Latitude string formatted according to provided options.
   */
  SDKCORE_EXPORT std::string printLatitude(double latRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol);

  /**
   * Formats a longitude value (in radians) into a string value according to the format specification.
   * @param lonRadians Longitude value to print to string, in radians.
   * @param format Geodetic output format.
   * @param allNumerics If false, then string will contain hemisphere, and the minutes symbol (') and
   *   and seconds symbol (") for DMD/DMS formats; additionally, values in western hemisphere will not
   *   show a negative (-) sign, because the "S" is appended.  If true, then the return string will
   *   omit the hemisphere, minutes symbol, seconds symbol, and will include a negative sign if in the
   *   western hemisphere.
   * @param precision Decimal precision to be used in formatting the value.
   * @param degSymbol Symbol to use when formatting output in DMD, DMS, or DD output
   * @return Latitude string formatted according to provided options.
   */
  SDKCORE_EXPORT std::string printLongitude(double lonRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol);

} // namespace simCore

#endif /* SIMCORE_STRING_ANGLE_H */
