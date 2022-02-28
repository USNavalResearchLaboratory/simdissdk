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
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/String/Utils.h"
#include "simCore/String/Constants.h"
#include "simCore/String/Angle.h"

namespace simCore
{

/// Returns appropriate string for the degree symbol based on specified format
std::string getDegreeSymbol(DegreeSymbolFormat fmt)
{
  switch (fmt)
  {
  case DEG_SYM_UNICODE:
    return STR_DEGREE_SYMBOL_UNICODE;
  case DEG_SYM_ASCII:
    return STR_DEGREE_SYMBOL_ASCII;
  case DEG_SYM_UTF8:
    return STR_DEGREE_SYMBOL_UTF8;
  case DEG_SYM_NONE:
    break;
  }
  return "";
}

/// Returns a double representing a geodetic DMD, DMS, or DD string
int getAngleFromDegreeString(const std::string& degStr, bool rads, double& ang)
{
  // detect and process all numeric values
  if (simCore::isValidNumber(degStr, ang))
  {
    if (rads)
      ang *= simCore::DEG2RAD;
    return 0;
  }

  // handle strings with either hemisphere notation and/or degree symbol
  std::vector<std::string> outputVec;
  size_t end = degStr.find_first_of("SsWw-", 0);
  double signVal = (end == degStr.npos) ? 1. : -1.;
  static const std::string wsTokens = " \t\n,:\u00B0\xC2\xB0'\"NnEeSsWw";
  simCore::stringTokenizer(outputVec, degStr, wsTokens);
  if (outputVec.empty())
  {
    // no valid tokens found
    return 1;
  }

  // process tokens found
  double deg = 0;
  if (outputVec.size() == 1)
  {
    if (!simCore::isValidNumber(outputVec[0], deg))
      return 1;
    ang = signVal * fabs(deg);
  }
  else if (outputVec.size() == 2)
  {
    double min = 0;
    if (!simCore::isValidNumber(outputVec[0], deg) || !simCore::isValidNumber(outputVec[1], min))
      return 1;
    ang = signVal * (fabs(deg) + fabs(min) / 60.);
  }
  else if (outputVec.size() >= 3)
  {
    double min = 0;
    double sec = 0;
    if (!simCore::isValidNumber(outputVec[0], deg) || !simCore::isValidNumber(outputVec[1], min) || !simCore::isValidNumber(outputVec[2], sec))
      return 1;
    ang = signVal * (fabs(deg) + fabs(min) / 60. + fabs(sec) / 3600.);
  }

  // convert to radians, if desired
  if (rads)
    ang *= simCore::DEG2RAD;

  return 0;
}


namespace
{

/// Return the simCore::Units associated with the format
const simCore::Units& formatToUnits(GeodeticFormat format)
{
  switch (format)
  {
  case FMT_DEGREES:
  case FMT_DEGREES_MINUTES:
  case FMT_DEGREES_MINUTES_SECONDS:
    return simCore::Units::DEGREES;
  case FMT_RADIANS:
    return simCore::Units::RADIANS;
  case FMT_BAM:
    return simCore::Units::BAM;
  case FMT_MIL:
    return simCore::Units::MIL;
  case FMT_MILLIRADIANS:
    return simCore::Units::MILLIRADIANS;
  }
  return simCore::Units::UNITLESS;
}

}

/// return a formatted angle string
std::string getAngleString(double radianAngle, GeodeticFormat format, bool allNumerics,
  size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir, char negativeDir)
{
  double degreeAngle = simCore::Units::RADIANS.convertTo(simCore::Units::DEGREES, radianAngle);
  precision = simCore::sdkMin(precision, static_cast<size_t>(16));

  std::string hemiDir = " ";
  bool negative = false;
  if (degreeAngle < 0.0)
  {
    if (negativeDir == '\0')
      hemiDir.clear();
    else
      hemiDir += negativeDir;
    degreeAngle = -degreeAngle;
    negative = true;
  }
  else
  {
    if (positiveDir == '\0')
      hemiDir.clear();
    else
      hemiDir += positiveDir;
  }

  std::string degreeSymbolString;
  std::string minuteSymbolString;
  std::string secondSymbolString;

  if (!allNumerics)
  {
    degreeSymbolString = simCore::getDegreeSymbol(degSymbol);
    minuteSymbolString = "'";
    secondSymbolString = "\"";
  }
  else
  {
    hemiDir.clear();
  }

  const bool printNegativeSign = (allNumerics || negativeDir == '\0');

  const double tolerance = 0.00000001; //< Value to avoid instance of -0
  double minValue = 0;  // DM min value
  std::string angleString;
  switch (format)
  {
  case FMT_DEGREES_MINUTES_SECONDS:
  {
    // extract minutes as fraction of degrees
    minValue = (degreeAngle - floor(degreeAngle)) * 60.0;

    // truncate degrees
    degreeAngle = floor(degreeAngle);

    // same for seconds on minutes, with rounding
    double secValue = (minValue - floor(minValue)) * 60.0;
    minValue = floor(minValue);

    // Don't permit negative seconds
    if (secValue < tolerance)
      secValue = 0.0;

    const double rounding = 5.0 / pow(10.0, precision + 1.0);
    if ((secValue + rounding > 60.0) || simCore::areEqual(secValue + rounding, 60.0))
    {
      secValue = 0.0;
      minValue += 1.0;
      if (minValue >= 60.0)
      {
        minValue = 0.0;
        degreeAngle += 1.0;
      }
    }
    else
    {
      double fraction = fmod(secValue * pow(10.0, precision + 1.0), 10.0);
      if ((fraction > 5.0) || simCore::areEqual(fraction, 5.0))
      {
        secValue += rounding;
      }
    }

    // degreeAngle was floor() above so just checking for 360 is OK
    if (degreeAngle == 360.0)
      degreeAngle = 0.0;
    degreeAngle = (negative && printNegativeSign) ? -degreeAngle : degreeAngle;

    std::stringstream strDeg;
    strDeg << static_cast<int>(degreeAngle) << degreeSymbolString << " ";
    std::stringstream strMin;
    strMin.setf(std::ios::fixed, std::ios::floatfield);
    strMin << std::setfill('0') << std::setw(2) << static_cast<int>(minValue) << minuteSymbolString << " ";

    std::stringstream strSec;
    strSec.setf(std::ios::fixed, std::ios::floatfield);
    size_t width = (precision == 0 ? (2) : (precision + 3));  // Account for decimal if precision is non-zero
    strSec << std::setfill('0') << std::setw(width) << std::setprecision(precision) << secValue << secondSymbolString;
    angleString = strDeg.str() + strMin.str() + strSec.str();
  }
  break;

  case FMT_DEGREES_MINUTES:
  {
    minValue = ((degreeAngle - floor(degreeAngle)) * 60.0);
    degreeAngle = floor(degreeAngle);

    if (fabs(minValue) <= tolerance)
      minValue = 0.0;

    const double rounding = 5.0 / pow(10.0, precision + 1.0);
    if (minValue + rounding >= 60.0 || simCore::areEqual(minValue + rounding, 60.0))
    {
      degreeAngle += 1.0;
      minValue = 0.0;
    }
    else
    {
      double fraction = fmod(minValue * pow(10.0, precision + 1.0), 10.0);
      if ((fraction > 5.0) || simCore::areEqual(fraction, 5.0))
      {
        minValue += rounding;
      }
    }

    // degreeAngle was floor() above so just checking for 360 is OK
    if (degreeAngle == 360.0)
      degreeAngle = 0.0;
    degreeAngle = (negative && printNegativeSign) ? -degreeAngle : degreeAngle;

    std::stringstream strDeg;
    strDeg << static_cast<int>(degreeAngle) << degreeSymbolString << " ";
    std::stringstream strMin;
    strMin.setf(std::ios::fixed, std::ios::floatfield);
    size_t width = (precision == 0 ? (2) : (precision + 3));  // Account for decimal if precision is non-zero
    strMin << std::setfill('0') << std::setw(width) << std::setprecision(precision) << minValue << minuteSymbolString;
    angleString = strDeg.str() + strMin.str();
  }
  break;

  case FMT_RADIANS:
  case FMT_BAM:
  case FMT_MIL:
  case FMT_MILLIRADIANS:
  {
    degreeAngle = (negative && printNegativeSign) ? -degreeAngle : degreeAngle;
    std::stringstream str;
    str.setf(std::ios::fixed, std::ios::floatfield);
    str << std::setprecision(precision) << simCore::Units::DEGREES.convertTo(formatToUnits(format), degreeAngle);
    angleString = str.str();
  }
  break;

  case FMT_DEGREES:
  default:
  {
    const double rounding = 5.0 / pow(10.0, precision + 1.0);
    if ((degreeAngle + rounding > 360.0) || simCore::areEqual(degreeAngle + rounding, 360.0))
      degreeAngle = 0.0;
    degreeAngle = (negative && printNegativeSign) ? -degreeAngle : degreeAngle;
    std::stringstream str;
    str.setf(std::ios::fixed, std::ios::floatfield);
    str << std::setprecision(precision) << degreeAngle << degreeSymbolString;
    angleString = str.str();
  }
  break;
  }

  return angleString + hemiDir;
}

/// convert lat (in radians) to string in the form: ##.####### N/S
std::string printLatitude(double latRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol)
{
  latRadians = simCore::angWrapPI2(latRadians);
  return getAngleString(latRadians, format, allNumerics, precision, degSymbol, 'N', 'S');
}

/// convert lon (in radians) to string in the form: ###.####### E/W
std::string printLongitude(double lonRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol)
{
  lonRadians = simCore::angFixPI(lonRadians);
  return getAngleString(lonRadians, format, allNumerics, precision, degSymbol, 'E', 'W');
}

} // namespace simCore
