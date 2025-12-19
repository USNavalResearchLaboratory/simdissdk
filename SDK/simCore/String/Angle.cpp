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

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>
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
  size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir, char negativeDir, bool allowRollover)
{
  AngleFormatter formatter;
  formatter.setFormat(format);
  formatter.setAllNumerics(allNumerics);
  formatter.setPrecision(precision);
  formatter.setSymbol(degSymbol);
  formatter.setDir(positiveDir, negativeDir);
  formatter.setAllowRollover(allowRollover);
  return formatter.format(radianAngle);
}

/// convert lat (in radians) to string in the form: ##.####### N/S
std::string printLatitude(double latRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir, char negativeDir)
{
  latRadians = simCore::angWrapPI2(latRadians);
  return getAngleString(latRadians, format, allNumerics, precision, degSymbol, positiveDir, negativeDir);
}

/// convert lon (in radians) to string in the form: ###.####### E/W
std::string printLongitude(double lonRadians, GeodeticFormat format, bool allNumerics,
    size_t precision, simCore::DegreeSymbolFormat degSymbol, char positiveDir, char negativeDir)
{
  lonRadians = simCore::angFixPI(lonRadians);
  return getAngleString(lonRadians, format, allNumerics, precision, degSymbol, positiveDir, negativeDir);
}

//---------------------------------------------------------------------------------------------------------------

AngleFormatter::AngleFormatter()
{
  printNegativeSign_ = getPrintNegativeSign_();
  setSymbols_();
  calculateScales_();
}

AngleFormatter::~AngleFormatter()
{
}

void AngleFormatter::setFormat(GeodeticFormat format)
{
  format_ = format;
}

void AngleFormatter::setAllNumerics(bool allNumerics)
{
  if (allNumerics_ == allNumerics)
    return;

  allNumerics_ = allNumerics;
  printNegativeSign_ = getPrintNegativeSign_();

  setSymbols_();
}

void AngleFormatter::setPrecision(size_t precision)
{
  const size_t limitedPrecision = simCore::sdkMin(precision, static_cast<size_t>(16));
  if (limitedPrecision == precision_)
    return;

  precision_ = limitedPrecision;
  calculateScales_();
}

void AngleFormatter::setSymbol(DegreeSymbolFormat degSymbol)
{
  if (degSymbol_ == degSymbol)
    return;

  degSymbol_ = degSymbol;
  if (!allNumerics_)
    degreeSymbolString_ = simCore::getDegreeSymbol(degSymbol_);
}

void AngleFormatter::setDir(char positiveDir, char negativeDir)
{
  if (positiveDir == '\0')
    positiveDir_.clear();
  else
    positiveDir_ = positiveDir;

  if (negativeDir == '\0')
    negativeDir_.clear();
  else
    negativeDir_ = negativeDir;

  printNegativeSign_ = getPrintNegativeSign_();
}

void AngleFormatter::setAllowRollover(bool allowRollover)
{
  allowRollover_ = allowRollover;
}

std::string AngleFormatter::format(double radianAngle) const
{
  switch (format_)
  {
  case FMT_DEGREES:
    return formatDegrees_(radianAngle);

  case FMT_DEGREES_MINUTES:
    return formatDegreesMinutes_(radianAngle);

  case FMT_DEGREES_MINUTES_SECONDS:
    return formatDegreesMinutesSeconds_(radianAngle);

  case FMT_RADIANS:
    return formatRadians_(radianAngle);

  case FMT_BAM:
  case FMT_MIL:
  case FMT_MILLIRADIANS:
    return formatMiscellaneous_(radianAngle);
  }

  return "";
}

std::string AngleFormatter::formatDegrees_(double radianAngle) const
{
  const bool wasNegative = (radianAngle < 0.0);
  if (wasNegative)
    radianAngle = -radianAngle;

  if (!allowRollover_)
  {
    if (fabs(simCore::angleDifference(radianAngle, M_TWOPI)) < radiansTolerance_)
      radianAngle = 0.0;
    else
      radianAngle = simCore::angFix2PI(radianAngle);
  }

  if (printNegativeSign_ && wasNegative)
    radianAngle = -radianAngle;

  std::string angleString;
  appendDouble_(angleString, radianAngle * simCore::RAD2DEG, precision_);
  angleString += degreeSymbolString_;
  appendHemisphereDirection_(angleString, wasNegative);

  return angleString;
}

std::string AngleFormatter::formatDegreesMinutes_(double radianAngle) const
{
  const bool wasNegative = (radianAngle < 0.0);

  double degreeValue = simCore::Units::RADIANS.convertTo(simCore::Units::DEGREES, radianAngle);
  if (wasNegative)
    degreeValue = -degreeValue;

  // extract minutes as fraction of degrees
  double minuteValue = (degreeValue - floor(degreeValue)) * 60.0;

  // truncate degrees
  degreeValue = floor(degreeValue);

  // Don't permit negative seconds
  const double tolerance = 0.00000001; //< Value to avoid instance of -0
  if (fabs(minuteValue) < tolerance)
    minuteValue = 0.0;

  if ((minuteValue + degreesTolerance_ > 60.0) || simCore::areEqual(minuteValue + degreesTolerance_, 60.0))
  {
    degreeValue += 1.0;
    minuteValue = 0.0;
  }
  else
  {
    double fraction = fmod(minuteValue * scale_, 10.0);
    if (fraction > 5.0)
      minuteValue += degreesTolerance_;
    else if (simCore::areEqual(fraction, 5.0))
      minuteValue += degreesTolerance_ + 0.000001;
  }

  // degreeAngle was floor() above so just checking for 360 is OK
  if (degreeValue == 360.0)
    degreeValue = 0.0;
  degreeValue = (wasNegative && printNegativeSign_) ? -degreeValue : degreeValue;

  std::string angleString;

  // SIM-14416: force leading -, as stringstream won't handle -0 as -0
  if (wasNegative && printNegativeSign_ && degreeValue == -0.)
    angleString = "-";
  angleString += std::to_string(static_cast<int>(degreeValue));
  angleString += degreeSymbolString_;
  angleString += " ";

  appendPadded_(angleString, static_cast<int>(floor(minuteValue)), 2);
  if (precision_ > 0)
  {
    double fractionalPart = minuteValue - floor(minuteValue);
    std::string fracStr;
    appendDouble_(fracStr, fractionalPart, precision_);
    angleString += (fracStr.substr(1, precision_ + 1)); // skip '0' and take '.' + precision digits
  }
  angleString += (minuteSymbolString_);

  appendHemisphereDirection_(angleString, wasNegative);

  return angleString;
}

std::string AngleFormatter::formatDegreesMinutesSeconds_(double radianAngle) const
{
  const bool wasNegative = (radianAngle < 0.0);

  double degreeValue = simCore::Units::RADIANS.convertTo(simCore::Units::DEGREES, radianAngle);
  if (wasNegative)
    degreeValue = -degreeValue;

  // extract minutes as fraction of degrees
  double minuteValue = (degreeValue - floor(degreeValue)) * 60.0;

  // truncate degrees
  degreeValue = floor(degreeValue);

  // same for seconds on minutes, with rounding
  double secondValue = (minuteValue - floor(minuteValue)) * 60.0;
  minuteValue = floor(minuteValue);

  // Don't permit negative seconds
  const double tolerance = 0.00000001; //< Value to avoid instance of -0
  if (secondValue < tolerance)
    secondValue = 0.0;

  if ((secondValue + degreesTolerance_ > 60.0) || simCore::areEqual(secondValue + degreesTolerance_, 60.0))
  {
    secondValue = 0.0;
    minuteValue += 1.0;
    if (minuteValue >= 60.0)
    {
      minuteValue = 0.0;
      degreeValue += 1.0;
    }
  }
  else
  {
    double fraction = fmod(secondValue * scale_, 10.0);
    if (fraction > 5.0)
      secondValue += degreesTolerance_;
    else if (simCore::areEqual(fraction, 5.0))
      secondValue += degreesTolerance_ + 0.000001;
  }

  // degreeAngle was floor() above so just checking for 360 is OK
  if (degreeValue == 360.0)
    degreeValue = 0.0;
  degreeValue = (wasNegative && printNegativeSign_) ? -degreeValue : degreeValue;

  std::string angleString;

  // SIM-14416: force leading -, as stringstream won't handle -0 as -0
  if (wasNegative && printNegativeSign_ && degreeValue == -0.)
    angleString = "-";
  angleString += std::to_string(static_cast<int>(degreeValue));
  angleString += degreeSymbolString_;
  angleString += " ";

  appendPadded_(angleString, static_cast<int>(minuteValue), 2);
  angleString += minuteSymbolString_;
  angleString += " ";

  appendPadded_(angleString, static_cast<int>(floor(secondValue)), 2);
  if (precision_ > 0)
  {
    double fractionalPart = secondValue - floor(secondValue);
    std::string fracStr;
    appendDouble_(fracStr, fractionalPart, precision_);
    angleString += (fracStr.substr(1, precision_ + 1)); // skip '0' and take '.' + precision digits
  }
  angleString += (secondSymbolString_);

  appendHemisphereDirection_(angleString, wasNegative);

  return angleString;
}

std::string AngleFormatter::formatRadians_(double radianAngle) const
{
  const bool wasNegative = (radianAngle < 0.0);
  if (!printNegativeSign_ && wasNegative)
    radianAngle = -radianAngle;

  std::string angleString;
  appendDouble_(angleString, radianAngle, precision_);
  appendHemisphereDirection_(angleString, wasNegative);

  return angleString;
}

std::string AngleFormatter::formatMiscellaneous_(double radianAngle) const
{
  const bool wasNegative = (radianAngle < 0.0);
  if (!printNegativeSign_ && wasNegative)
    radianAngle = -radianAngle;

  std::string angleString;
  appendDouble_(angleString, simCore::Units::RADIANS.convertTo(formatToUnits(format_), radianAngle), precision_);
  appendHemisphereDirection_(angleString, wasNegative);

  return angleString;
}

std::string AngleFormatter::getHemisphereDirection_(bool wasNegative) const
{
  if (wasNegative)
    return (!negativeDir_.empty() && !allNumerics_) ? negativeDir_ : "";

  return (!positiveDir_.empty() && !allNumerics_) ? positiveDir_ : "";
}

bool AngleFormatter::getPrintNegativeSign_() const
{
  return (allNumerics_ || negativeDir_.empty());
}

void AngleFormatter::appendPadded_(std::string& str, int value, int width) const
{
  char buffer[16];
  auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
  if (ec == std::errc())
  {
    if (int numChars = static_cast<int>(ptr - buffer); numChars < width)
      str.append(width - numChars, '0');
    str.append(buffer, ptr - buffer);
  }
}

void AngleFormatter::appendDouble_(std::string& str, double value, size_t precision) const
{
  char buffer[64]; // Sufficient for most doubles
  auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed, precision);
  if (ec == std::errc())
  {
    str.append(buffer, ptr - buffer);
  }
}

void AngleFormatter::appendHemisphereDirection_(std::string& str, bool wasNegative) const
{
  const std::string& hemiDir = getHemisphereDirection_(wasNegative);
  if (!hemiDir.empty())
  {
    str += " ";
    str += hemiDir;
  }
}

void AngleFormatter::calculateScales_()
{
  scale_ = pow(10.0, precision_ + 1.0);
  degreesTolerance_ = 5.0 / scale_;
  radiansTolerance_ = degreesTolerance_ * DEG2RAD;
}

void AngleFormatter::setSymbols_()
{
  if (!allNumerics_)
  {
    degreeSymbolString_ = simCore::getDegreeSymbol(degSymbol_);
    minuteSymbolString_ = "'";
    secondSymbolString_ = "\"";
  }
  else
  {
    degreeSymbolString_.clear();
    minuteSymbolString_.clear();
    secondSymbolString_.clear();
  }
}

} // namespace simCore
