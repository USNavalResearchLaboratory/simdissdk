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
#ifndef SIMCORE_UNITCONTEXTFORMATTER_H
#define SIMCORE_UNITCONTEXTFORMATTER_H

#include "simCore/Common/Common.h"
#include "simCore/String/Angle.h"
#include "simCore/String/TextFormatter.h"
#include "simCore/Time/String.h"

namespace simCore
{

class UnitContext;

/**
 * Text formatter that couples together UnitContext with TextFormatter to print text
 * for use in lists, console output, or other GUI frames.
 */
class SDKCORE_EXPORT UnitContextFormatter : public TextFormatter
{
public:
  /** Construct a text formatter abiding by rules of a unit context */
  UnitContextFormatter(const UnitContext* unitsProvider);
  virtual ~UnitContextFormatter();

  virtual std::string formatLatitude(double val, DegreeSymbolFormat format=DEG_SYM_NONE, bool allNumerics=false) const;
  virtual std::string formatLongitude(double val, DegreeSymbolFormat format=DEG_SYM_NONE, bool allNumerics=false) const;
  virtual std::string formatAngle(double val, AngleExtents angleFormat=ANGLEEXTENTS_ALL, DegreeSymbolFormat degreeFormat=DEG_SYM_NONE) const;
  virtual std::string formatAzimuth(const Vec3& lla, const TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset, DegreeSymbolFormat degreeFormat=DEG_SYM_NONE) const;
  virtual std::string formatDistance(double val) const;
  virtual std::string formatAltitude(double alt) const;
  virtual std::string formatAltitude(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset, VerticalDatum outputDatum) const;
  virtual std::string formatSpeed(double val) const;
  virtual std::string formatTime(double sec) const;
  virtual std::string formatTime(const TimeStamp& timeStamp) const;
  virtual std::string formatDouble(double val) const;
  virtual std::string formatRGBA(unsigned short red, unsigned short green, unsigned short blue, unsigned short alpha) const;

  virtual double rawAzimuth(const Vec3& lla, const TimeStamp& timeStamp, double az, CoordinateSystem coordSystem, double offset) const;
  virtual double rawAltitude(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem, double offset, VerticalDatum outputDatum) const;

private:
  /** Generic formatting of a double value with given precision */
  std::string formatDouble_(double val, int precision) const;
  /** Returns the precision accounting for the geodetic format */
  unsigned int geodeticPrecision_(unsigned int precision, GeodeticFormat format) const;

  /// Reference to units provider managed and owned outside of formatter
  const UnitContext* unitsProvider_;
  /// Maintains a registry of time formatters
  simCore::TimeFormatterRegistry timeFormatters_;
};

}

#endif /* SIMCORE_UNITCONTEXTFORMATTER_H */
