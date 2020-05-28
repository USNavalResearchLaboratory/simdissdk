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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#ifndef SIMCORE_STRING_TEXTFORMATTER_H
#define SIMCORE_STRING_TEXTFORMATTER_H

#include <string>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/String/Angle.h"

namespace simCore
{

class TimeStamp;
class Vec3;

/** Unified interface for a class that can convert values into strings */
class SDKCORE_EXPORT TextFormatter
{
public:
  /** Inherit a virtual destructor */
  virtual ~TextFormatter() {}

  /** Converts a latitude value into a string; input is in radians */
  virtual std::string formatLatitude(double val, DegreeSymbolFormat format=DEG_SYM_NONE, bool allNumerics=false) const = 0;
  /** Converts a longitude value into a string; input is in radians */
  virtual std::string formatLongitude(double val, DegreeSymbolFormat format=DEG_SYM_NONE, bool allNumerics=false) const = 0;
  /** Converts an angle value into a string; input is in radians */
  virtual std::string formatAngle(double val, AngleExtents angleFormat=ANGLEEXTENTS_ALL, DegreeSymbolFormat degreeFormat=DEG_SYM_NONE) const = 0;
  /** Converts a true angle value into a string, when the angle could be affected by magnetic variance; input is in radians */
  virtual std::string formatAzimuth(const Vec3& lla, const simCore::TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset, DegreeSymbolFormat degreeFormat=DEG_SYM_NONE) const = 0;
  /** Converts a distance value into a string; input is in meters */
  virtual std::string formatDistance(double val) const = 0;
  /** Converts an altitude value into a string; input is in meters */
  virtual std::string formatAltitude(double alt) const = 0;
  /** Converts an altitude value into a string after accounting for vertical datum; input is in meters, angles in radians */
  virtual std::string formatAltitude(const Vec3& lla, const simCore::TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset, VerticalDatum outputDatum) const = 0;
  /** Converts a speed value into a string; input is in meters per second */
  virtual std::string formatSpeed(double val) const = 0;
  /** Converts a time value into a string; time is always in seconds since the scenario's reference year; negative time implies static time */
  virtual std::string formatTime(double sec) const = 0;
  /** Converts a time value into a string; time is absolute and may not be 'static' */
  virtual std::string formatTime(const TimeStamp& timeStamp) const = 0;
  /** Formats a standard double precision value using appropriate precision */
  virtual std::string formatDouble(double val) const = 0;
  /** Formats a color value into a human readable output format */
  virtual std::string formatRGBA(unsigned short red, unsigned short green, unsigned short blue, unsigned short alpha) const = 0;
  /** Formats a red-major color value RRGGBBAA into a human readable output format */
  virtual std::string formatRGBA(unsigned int rgba) const;
  /** Formats an alpha-major color value AABBGGRR into a human readable output format */
  virtual std::string formatABGR(unsigned int abgr) const;

  /** Converts a true angle value, when the angle could be affected by magnetic variance; input is in radians */
  virtual double rawAzimuth(const Vec3& lla, const simCore::TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset) const = 0;
  /** Converts an altitude value  accounting for vertical datum; input is in meters, angles in radians */
  virtual double rawAltitude(const Vec3& lla, const simCore::TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset, VerticalDatum outputDatum) const = 0;
};

}

#endif /* SIMCORE_STRING_TEXTFORMATTER_H */
