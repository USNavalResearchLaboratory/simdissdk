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
    DEG_SYM_UNICODE,  ///< "\u00B0"; GUI usable
    DEG_SYM_UTF8      ///< "\xC2\xB0"; UTF-8 degree symbol
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
   * Underlying method used by printLatitude and printLongitude for a formatted angle string. Note that angles
   * requested in simCore::FMT_DEGREES will be clamped to 0 if the value exceeds 360 (see allowRollover parameter)
   * @param radianAngle Angle in radians to print to string
   * @param format Geodetic output format.
   * @param allNumerics If true, string omits positiveDir and negativeDir, and omits the degree, minute, and second
   *   symbol between tokens for DMS/DMD/DD formats, using a "-" sign as needed.  If false, then DD/DMD/DMS formatting
   *   is applied, and either positiveDir and negativeDir are appended, OR a "-" sign is appended if negativeDir is 0.
   * @param precision Decimal precision to be used in formatting the value.
   * @param degSymbol Symbol to use when formatting output in DMD, DMS, or DD output
   * @param positiveDir Character to append for positive angles, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for positive directions.
   * @param negativeDir Character to append for negative angles, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for negative directions, in which case a negative sign is prepended.
   * @param allowRollover If true, allows angle returns that go over 360. If false, angle returns in simCore::FMT_DEGREES
   *   will be clamped to 0 if they exceed 360.
   * @return Formatted string
   */
  SDKCORE_EXPORT std::string getAngleString(double radianAngle, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir, char negativeDir, bool allowRollover = false);

  /**
   * Formats a latitude value (in radians) into a string value according to the format specification.
   * @param latRadians Latitude value to print to string, in radians.
   * @param format Geodetic output format.
   * @param allNumerics If true, string omits positiveDir and negativeDir, and omits the degree, minute, and second
   *   symbol between tokens for DMS/DMD/DD formats, using a "-" sign as needed.  If false, then DD/DMD/DMS formatting
   *   is applied, and either positiveDir and negativeDir are appended, OR a "-" sign is appended if negativeDir is 0.
   * @param precision Decimal precision to be used in formatting the value.
   * @param degSymbol Symbol to use when formatting output in DMD, DMS, or DD output
   * @param positiveDir Character to append for latitudes above the equator, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for positive directions.
   * @param negativeDir Character to append for latitudes below the equator, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for negative directions, in which case a negative sign is prepended.
   * @return Latitude string formatted according to provided options.
   */
  SDKCORE_EXPORT std::string printLatitude(double latRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir = 'N', char negativeDir = 'S');

  /**
   * Formats a longitude value (in radians) into a string value according to the format specification.
   * @param lonRadians Longitude value to print to string, in radians.
   * @param format Geodetic output format.
   * @param allNumerics If true, string omits positiveDir and negativeDir, and omits the degree, minute, and second
   *   symbol between tokens for DMS/DMD/DD formats, using a "-" sign as needed.  If false, then DD/DMD/DMS formatting
   *   is applied, and either positiveDir and negativeDir are appended, OR a "-" sign is appended if negativeDir is 0.
   * @param precision Decimal precision to be used in formatting the value.
   * @param degSymbol Symbol to use when formatting output in DMD, DMS, or DD output
   * @param positiveDir Character to append for longitudes east of the meridian, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for positive directions.
   * @param negativeDir Character to append for longitudes west of the meridian, only when allNumerics==false.  Set this to 0 (\0 or null)
   *   to not append anything for negative directions, in which case a negative sign is prepended.
   * @return Latitude string formatted according to provided options.
   */
  SDKCORE_EXPORT std::string printLongitude(double lonRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir = 'E', char negativeDir = 'W');

  /** High performance angle formatter. Create once and use many time for best performance. */
  class SDKCORE_EXPORT AngleFormatter
  {
  public:
    AngleFormatter();
    virtual ~AngleFormatter();

    /** Set the geodetic format */
    void setFormat(GeodeticFormat format);

    /** If true only numbers */
    void setAllNumerics(bool allNumerics);

    /** Number of digits after the decimal point */
    void setPrecision(size_t precision);

    /** The degree symbol to use*/
    void setSymbol(DegreeSymbolFormat degSymbol);

    /** The optional 'N'/'S' or 'E'/'W' */
    void setDir(char positiveDir, char negativeDir);

    /** If true allow values over 360/2PI */
    void setAllowRollover(bool allowRollover);

    /** The value to format based on the options above */
    std::string format(double radianAngle) const;

  private:
    /** Format the value into degrees */
    std::string formatDegrees_(double radianAngle) const;

    /** Format the value into degrees and minutes */
    std::string formatDegreesMinutes_(double radianAngle) const;

    /** Format the value into degrees, minutes and seconds */
    std::string formatDegreesMinutesSeconds_(double radianAngle) const;

    /** Format the value into radians */
    std::string formatRadians_(double radianAngle) const;

    /** Format the value into format specified by format_ */
    std::string formatMiscellaneous_(double radianAngle) const;

    /** Returns the hemisphere direction character */
    std::string getHemisphereDirection_(bool wasNegative) const;

    /** Returns true if the negative sign should be printed */
    bool getPrintNegativeSign_() const;

    /** Appends an integer with leading zeros based on the width */
    void appendPadded_(std::string& str, int value, int width) const;

    /** Appends a double with the given precision */
    void appendDouble_(std::string& str, double value, size_t precision) const;

    /** Appends, if necessary, the hemisphere direction */
    void appendHemisphereDirection_(std::string& str, bool wasNegative) const;

    /** Set the scale member variables based on precision_ */
    void calculateScales_();

    /** Set the symbol member variables based on allNumerics_*/
    void setSymbols_();

    bool allNumerics_ = false;
    bool allowRollover_ = false;
    bool printNegativeSign_ = false;
    std::string positiveDir_;
    std::string negativeDir_;
    GeodeticFormat format_ = GeodeticFormat::FMT_DEGREES;
    DegreeSymbolFormat degSymbol_ = DegreeSymbolFormat::DEG_SYM_NONE;
    size_t precision_ = 0;
    double scale_ = 0.0;
    double radiansTolerance_ = 0.0;
    double degreesTolerance_ = 0.0;
    std::string degreeSymbolString_;
    std::string minuteSymbolString_;
    std::string secondSymbolString_;
  };

} // namespace simCore

#endif /* SIMCORE_STRING_ANGLE_H */
