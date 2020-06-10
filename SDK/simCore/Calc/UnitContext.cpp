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
#include "simCore/Calc/UnitContext.h"

namespace simCore {

UnitContextAdapter::UnitContextAdapter()
  : timeFormat_(TIMEFORMAT_ORDINAL),
    timePrecision_(3),
    geodeticFormat_(FMT_DEGREES_MINUTES),
    geodeticPrecision_(6),
    distanceUnits_(Units::METERS),
    distancePrecision_(3),
    altitudeUnits_(Units::METERS),
    altitudePrecision_(3),
    angleUnits_(Units::DEGREES),
    anglePrecision_(3),
    speedUnits_(Units::METERS_PER_SECOND),
    speedPrecision_(3),
    genericPrecision_(3),
    coordinateSystem_(COORD_SYS_LLA),
    magneticVariance_(MAGVAR_TRUE),
    verticalDatum_(VERTDATUM_WGS84),
    referenceYear_(1970),
    datumConvert_(new MagneticDatumConvert)
{
}

UnitContextAdapter::~UnitContextAdapter()
{
}

// Getters for various unit types
TimeFormat UnitContextAdapter::timeFormat() const
{
  return timeFormat_;
}

unsigned int UnitContextAdapter::timePrecision() const
{
  return timePrecision_;
}

CoordinateSystem UnitContextAdapter::coordinateSystem() const
{
  return coordinateSystem_;
}

GeodeticFormat UnitContextAdapter::geodeticFormat() const
{
  return geodeticFormat_;
}

unsigned int UnitContextAdapter::geodeticPrecision() const
{
  return geodeticPrecision_;
}

const Units& UnitContextAdapter::distanceUnits() const
{
  return distanceUnits_;
}

unsigned int UnitContextAdapter::distancePrecision() const
{
  return distancePrecision_;
}

const Units& UnitContextAdapter::altitudeUnits() const
{
  return altitudeUnits_;
}

unsigned int UnitContextAdapter::altitudePrecision() const
{
  return altitudePrecision_;
}

const Units& UnitContextAdapter::angleUnits() const
{
  return angleUnits_;
}

unsigned int UnitContextAdapter::anglePrecision() const
{
  return anglePrecision_;
}

const Units& UnitContextAdapter::speedUnits() const
{
  return speedUnits_;
}

unsigned int UnitContextAdapter::speedPrecision() const
{
  return speedPrecision_;
}

unsigned int UnitContextAdapter::genericPrecision() const
{
  return genericPrecision_;
}

MagneticVariance UnitContextAdapter::magneticVariance() const
{
  return magneticVariance_;
}

VerticalDatum UnitContextAdapter::verticalDatum() const
{
  return verticalDatum_;
}

int UnitContextAdapter::referenceYear() const
{
  return referenceYear_;
}

simCore::DatumConvertPtr UnitContextAdapter::datumConvert() const
{
  return datumConvert_;
}

void UnitContextAdapter::setTimeFormat(TimeFormat unit)
{
  timeFormat_ = unit;
}

void UnitContextAdapter::setTimePrecision(unsigned int prec)
{
  timePrecision_ = prec;
}

void UnitContextAdapter::setCoordinateSystem(CoordinateSystem coordSys)
{
  coordinateSystem_ = coordSys;
}

void UnitContextAdapter::setGeodeticFormat(GeodeticFormat format)
{
  geodeticFormat_ = format;
}

void UnitContextAdapter::setGeodeticPrecision(unsigned int prec)
{
  geodeticPrecision_ = prec;
}

void UnitContextAdapter::setDistanceUnits(const Units& unit)
{
  distanceUnits_ = unit;
}

void UnitContextAdapter::setDistancePrecision(unsigned int prec)
{
  distancePrecision_ = prec;
}

void UnitContextAdapter::setAltitudeUnits(const Units& unit)
{
  altitudeUnits_ = unit;
}

void UnitContextAdapter::setAltitudePrecision(unsigned int prec)
{
  altitudePrecision_ = prec;
}

void UnitContextAdapter::setAngleUnits(const Units& unit)
{
  angleUnits_ = unit;
}

void UnitContextAdapter::setAnglePrecision(unsigned int prec)
{
  anglePrecision_ = prec;
}

void UnitContextAdapter::setSpeedUnits(const Units& unit)
{
  speedUnits_ = unit;
}

void UnitContextAdapter::setSpeedPrecision(unsigned int prec)
{
  speedPrecision_ = prec;
}

void UnitContextAdapter::setGenericPrecision(unsigned int prec)
{
  genericPrecision_ = prec;
}

void UnitContextAdapter::setMagneticVariance(MagneticVariance mv)
{
  magneticVariance_ = mv;
}

void UnitContextAdapter::setVerticalDatum(VerticalDatum vd)
{
  verticalDatum_ = vd;
}

void UnitContextAdapter::setReferenceYear(int refYear)
{
  referenceYear_ = refYear;
}

void UnitContextAdapter::setDatumConvert(simCore::DatumConvertPtr datumConvert)
{
  if (datumConvert == NULL)
    datumConvert_.reset(new MagneticDatumConvert);
  else
    datumConvert_ = datumConvert;
}

}
