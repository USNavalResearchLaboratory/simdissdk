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
#include <sstream>
#include <iomanip>
#include <limits>
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/UnitContext.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/String/Format.h"
#include "simCore/String/UnitContextFormatter.h"

namespace simCore {

UnitContextFormatter::UnitContextFormatter(const UnitContext* unitsProvider)
  : unitsProvider_(unitsProvider)
{
  // If units provider is nullptr, nothing in this class works
  assert(unitsProvider_);
}

UnitContextFormatter::~UnitContextFormatter()
{
}

unsigned int UnitContextFormatter::geodeticPrecision_(unsigned int precision, GeodeticFormat format) const
{
  int rv = static_cast<int>(precision);
  if (format == FMT_DEGREES_MINUTES)
    rv -= 2;
  else if (format == FMT_DEGREES_MINUTES_SECONDS)
    rv -= 4;

  return rv < 0 ? 0 : static_cast<unsigned int>(rv);
}

std::string UnitContextFormatter::formatLatitude(double val, simCore::DegreeSymbolFormat format, bool allNumerics) const
{
  const unsigned int precision = geodeticPrecision_(unitsProvider_->geodeticPrecision(), unitsProvider_->geodeticFormat());
  return printLatitude(val, unitsProvider_->geodeticFormat(), allNumerics, precision, format);
}

std::string UnitContextFormatter::formatLongitude(double val, simCore::DegreeSymbolFormat format, bool allNumerics) const
{
  const unsigned int precision = geodeticPrecision_(unitsProvider_->geodeticPrecision(), unitsProvider_->geodeticFormat());
  return printLongitude(val, unitsProvider_->geodeticFormat(), allNumerics, precision, format);
}

std::string UnitContextFormatter::formatAngle(double val, AngleExtents angleFormat, simCore::DegreeSymbolFormat degreeFormat) const
{
  val = angFix(val, angleFormat);
  // prevent showing 360.0
  if (angleFormat == ANGLEEXTENTS_TWOPI && simCore::areEqual(val, M_TWOPI))
    val = 0.0;

  std::string degreeSymbol;
  if (unitsProvider_->angleUnits() == Units::DEGREES)
    degreeSymbol = simCore::getDegreeSymbol(degreeFormat);

  const double newVal = Units::RADIANS.convertTo(unitsProvider_->angleUnits(), val);
  return formatDouble_(newVal, unitsProvider_->anglePrecision()) + degreeSymbol;
}

double UnitContextFormatter::rawAzimuth(const Vec3& lla, const simCore::TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset) const
{
  if (unitsProvider_->datumConvert() != nullptr)
    return unitsProvider_->datumConvert()->convertMagneticDatum(lla, timeStamp, az, coordSystem, MAGVAR_TRUE, unitsProvider_->magneticVariance(), offset);
  return az;
}

std::string UnitContextFormatter::formatAzimuth(const Vec3& lla, const simCore::TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset, simCore::DegreeSymbolFormat degreeFormat) const
{
  az = rawAzimuth(lla, timeStamp, az, coordSystem, offset);
  return formatAngle(az, ANGLEEXTENTS_TWOPI, degreeFormat);
}

std::string UnitContextFormatter::formatDistance(double val) const
{
  const double newVal = Units::METERS.convertTo(unitsProvider_->distanceUnits(), val);
  return formatDouble_(newVal, unitsProvider_->distancePrecision());
}

std::string UnitContextFormatter::formatAltitude(double alt) const
{
  const double newVal = Units::METERS.convertTo(unitsProvider_->altitudeUnits(), alt);
  return formatDouble_(newVal, unitsProvider_->altitudePrecision());
}

double UnitContextFormatter::rawAltitude(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset) const
{
  if (unitsProvider_->datumConvert() != nullptr)
    return unitsProvider_->datumConvert()->convertVerticalDatum(lla, timeStamp, coordSystem, VERTDATUM_WGS84, unitsProvider_->verticalDatum(), offset);
  return lla.alt();
}

std::string UnitContextFormatter::formatAltitude(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset) const
{
  return formatAltitude(rawAltitude(lla, timeStamp, coordSystem, offset));
}

std::string UnitContextFormatter::formatSpeed(double val) const
{
  const double newVal = Units::METERS_PER_SECOND.convertTo(unitsProvider_->speedUnits(), val);
  return formatDouble_(newVal, unitsProvider_->speedPrecision());
}

std::string UnitContextFormatter::formatTime(double sec) const
{
  // Do a check on static time, which could throw an exception in printTimeString()
  if (sec < 0)
    return "Static";
  return formatTime(simCore::TimeStamp(unitsProvider_->referenceYear(), sec));
}

std::string UnitContextFormatter::formatTime(const simCore::TimeStamp& timeStamp) const
{
  return timeFormatters_.toString(unitsProvider_->timeFormat(), timeStamp, unitsProvider_->referenceYear(), unitsProvider_->timePrecision());
}

std::string UnitContextFormatter::formatDouble(double value) const
{
  return formatDouble_(value, unitsProvider_->genericPrecision());
}

std::string UnitContextFormatter::formatDouble_(double val, int precision) const
{
  // set the limits so small values are NOT represented by scientific notation.
  const std::string value = buildString("", val, 0, static_cast<size_t>(precision), "", false, 1e+15, std::numeric_limits<double>::min());
  // Replace -0.0 with 0.0 where the number of zeros after the decimal point can vary.
  const size_t minusSign = value.find("-0");
  if ((minusSign != std::string::npos) && (value.find_first_not_of("0", minusSign+3) == std::string::npos))
    return value.substr(0, minusSign) + value.substr(minusSign+1);
  return value;
}

std::string UnitContextFormatter::formatRGBA(unsigned short red, unsigned short green, unsigned short blue, unsigned short alpha) const
{
  std::stringstream ss;
  ss << "0x" << std::hex << std::setfill('0')
     << std::setw(2) << simCore::sdkMin<unsigned short>(255, red)
     << std::setw(2) << simCore::sdkMin<unsigned short>(255, green)
     << std::setw(2) << simCore::sdkMin<unsigned short>(255, blue)
     << std::setw(2) << simCore::sdkMin<unsigned short>(255, alpha);
  return ss.str();
}

}
