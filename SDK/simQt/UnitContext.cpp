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
#include <cassert>
#include "simCore/String/Angle.h"
#include "simQt/UnitContext.h"

namespace simQt {

UnitContextAdapter::UnitContextAdapter(QObject* parent)
  : UnitContext(parent),
    timeFormat_(simCore::TIMEFORMAT_ORDINAL),
    timePrecision_(3),
    geodeticFormat_(simCore::FMT_DEGREES_MINUTES),
    geodeticPrecision_(6),
    distanceUnits_(simCore::Units::METERS),
    distancePrecision_(3),
    altitudeUnits_(simCore::Units::METERS),
    altitudePrecision_(3),
    angleUnits_(simCore::Units::DEGREES),
    anglePrecision_(3),
    speedUnits_(simCore::Units::METERS_PER_SECOND),
    speedPrecision_(3),
    genericPrecision_(3),
    coordinateSystem_(simCore::COORD_SYS_LLA),
    magneticVariance_(simCore::MAGVAR_TRUE),
    verticalDatum_(simCore::VERTDATUM_WGS84),
    referenceYear_(1970)
{
}

UnitContextAdapter::~UnitContextAdapter()
{
}

// Getters for various unit types
simCore::TimeFormat UnitContextAdapter::timeFormat() const
{
  return timeFormat_;
}

unsigned int UnitContextAdapter::timePrecision() const
{
  return timePrecision_;
}

simCore::CoordinateSystem UnitContextAdapter::coordinateSystem() const
{
  return coordinateSystem_;
}

simCore::GeodeticFormat UnitContextAdapter::geodeticFormat() const
{
  return geodeticFormat_;
}

unsigned int UnitContextAdapter::geodeticPrecision() const
{
  return geodeticPrecision_;
}

const simCore::Units& UnitContextAdapter::distanceUnits() const
{
  return distanceUnits_;
}

unsigned int UnitContextAdapter::distancePrecision() const
{
  return distancePrecision_;
}

const simCore::Units& UnitContextAdapter::altitudeUnits() const
{
  return altitudeUnits_;
}

unsigned int UnitContextAdapter::altitudePrecision() const
{
  return altitudePrecision_;
}

const simCore::Units& UnitContextAdapter::angleUnits() const
{
  return angleUnits_;
}

unsigned int UnitContextAdapter::anglePrecision() const
{
  return anglePrecision_;
}

const simCore::Units& UnitContextAdapter::speedUnits() const
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

simCore::MagneticVariance UnitContextAdapter::magneticVariance() const
{
  return magneticVariance_;
}

simCore::VerticalDatum UnitContextAdapter::verticalDatum() const
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

void UnitContextAdapter::setTimeFormat(simCore::TimeFormat unit)
{
  if (timeFormat_ != unit)
  {
    timeFormat_ = unit;
    Q_EMIT timeFormatChanged(unit);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setTimePrecision(unsigned int prec)
{
  if (timePrecision_ != prec)
  {
    timePrecision_ = prec;
    Q_EMIT timePrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setCoordinateSystem(simCore::CoordinateSystem coordSys)
{
  if (coordinateSystem_ != coordSys)
  {
    coordinateSystem_ = coordSys;
    Q_EMIT coordinateSystemChanged(coordSys);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setGeodeticFormat(simCore::GeodeticFormat format)
{
  if (geodeticFormat_ != format)
  {
    geodeticFormat_ = format;
    Q_EMIT geodeticFormatChanged(format);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setGeodeticPrecision(unsigned int prec)
{
  if (geodeticPrecision_ != prec)
  {
    geodeticPrecision_ = prec;
    Q_EMIT geodeticPrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setDistanceUnits(const simCore::Units& unit)
{
  if (distanceUnits_ != unit)
  {
    distanceUnits_ = unit;
    Q_EMIT distanceUnitsChanged(unit);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setDistancePrecision(unsigned int prec)
{
  if (distancePrecision_ != prec)
  {
    distancePrecision_ = prec;
    Q_EMIT distancePrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setAltitudeUnits(const simCore::Units& unit)
{
  if (altitudeUnits_ != unit)
  {
    altitudeUnits_ = unit;
    Q_EMIT altitudeUnitsChanged(unit);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setAltitudePrecision(unsigned int prec)
{
  if (altitudePrecision_ != prec)
  {
    altitudePrecision_ = prec;
    Q_EMIT altitudePrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setAngleUnits(const simCore::Units& unit)
{
  if (angleUnits_ != unit)
  {
    angleUnits_ = unit;
    Q_EMIT angleUnitsChanged(unit);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setAnglePrecision(unsigned int prec)
{
  if (anglePrecision_ != prec)
  {
    anglePrecision_ = prec;
    Q_EMIT anglePrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setSpeedUnits(const simCore::Units& unit)
{
  if (speedUnits_ != unit)
  {
    speedUnits_ = unit;
    Q_EMIT speedUnitsChanged(unit);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setSpeedPrecision(unsigned int prec)
{
  if (speedPrecision_ != prec)
  {
    speedPrecision_ = prec;
    Q_EMIT speedPrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setGenericPrecision(unsigned int prec)
{
  if (genericPrecision_ != prec)
  {
    genericPrecision_ = prec;
    Q_EMIT genericPrecisionChanged(prec);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setMagneticVariance(simCore::MagneticVariance mv)
{
  if (magneticVariance_ != mv)
  {
    magneticVariance_ = mv;
    Q_EMIT magneticVarianceChanged(mv);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setVerticalDatum(simCore::VerticalDatum vd)
{
  if (verticalDatum_ != vd)
  {
    verticalDatum_ = vd;
    Q_EMIT verticalDatumChanged(vd);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setReferenceYear(int refYear)
{
  if (referenceYear_ != refYear)
  {
    referenceYear_ = refYear;
    Q_EMIT referenceYearChanged(refYear);
    Q_EMIT unitsChanged(this);
  }
}

void UnitContextAdapter::setDatumConvert(simCore::DatumConvertPtr datumConvert)
{
  if (datumConvert_ != datumConvert)
  {
    datumConvert_ = datumConvert;
    Q_EMIT datumConvertPtrChanged(datumConvert);
    Q_EMIT unitsChanged(this);
  }
}

/////////////////////////////////////////////////////////////

UnitContextProxy::UnitContextProxy(UnitContext* subject)
  : UnitContext(),
    subject_(subject)
{
}

UnitContextProxy::~UnitContextProxy()
{
}

simQt::UnitContext* UnitContextProxy::subject() const
{
  return subject_;
}

void UnitContextProxy::setSubject(UnitContext* newSubject)
{
  // Break early if setting to same value
  if (newSubject == subject_)
    return;

  // The new context (subject) must be non-nullptr; refuse
  assert(newSubject != nullptr);

  // First disconnect all signals from the old subject
  if (subject_)
  {
    disconnect(subject_, SIGNAL(unitsChanged(simQt::UnitContext*)), this, SLOT(reemit_()));
    disconnect(subject_, SIGNAL(timeFormatChanged(simCore::TimeFormat)), this, SIGNAL(timeFormatChanged(simCore::TimeFormat)));
    disconnect(subject_, SIGNAL(timePrecisionChanged(unsigned int)), this, SIGNAL(timePrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(geodeticFormatChanged(simCore::GeodeticFormat)), this, SIGNAL(geodeticFormatChanged(simCore::GeodeticFormat)));
    disconnect(subject_, SIGNAL(geodeticPrecisionChanged(unsigned int)), this, SIGNAL(geodeticPrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(distanceUnitsChanged(simCore::Units)), this, SIGNAL(distanceUnitsChanged(simCore::Units)));
    disconnect(subject_, SIGNAL(distancePrecisionChanged(unsigned int)), this, SIGNAL(distancePrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(altitudeUnitsChanged(simCore::Units)), this, SIGNAL(altitudeUnitsChanged(simCore::Units)));
    disconnect(subject_, SIGNAL(altitudePrecisionChanged(unsigned int)), this, SIGNAL(altitudePrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(angleUnitsChanged(simCore::Units)), this, SIGNAL(angleUnitsChanged(simCore::Units)));
    disconnect(subject_, SIGNAL(anglePrecisionChanged(unsigned int)), this, SIGNAL(anglePrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(speedUnitsChanged(simCore::Units)), this, SIGNAL(speedUnitsChanged(simCore::Units)));
    disconnect(subject_, SIGNAL(speedPrecisionChanged(unsigned int)), this, SIGNAL(speedPrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(genericPrecisionChanged(unsigned int)), this, SIGNAL(genericPrecisionChanged(unsigned int)));
    disconnect(subject_, SIGNAL(coordinateSystemChanged(simCore::CoordinateSystem)), this, SIGNAL(coordinateSystemChanged(simCore::CoordinateSystem)));
    disconnect(subject_, SIGNAL(magneticVarianceChanged(simCore::MagneticVariance)), this, SIGNAL(magneticVarianceChanged(simCore::MagneticVariance)));
    disconnect(subject_, SIGNAL(verticalDatumChanged(simCore::VerticalDatum)), this, SIGNAL(verticalDatumChanged(simCore::VerticalDatum)));
    disconnect(subject_, SIGNAL(referenceYearChanged(int)), this, SIGNAL(referenceYearChanged(int)));
    disconnect(subject_, SIGNAL(datumConvertPtrChanged(simCore::DatumConvertPtr)), this, SIGNAL(datumConvertPtrChanged(simCore::DatumConvertPtr)));
  }

  UnitContext* oldSubject = subject_;
  subject_ = newSubject;

  // Try to safely deal with subject_ being nullptr by returning (assert should trigger above in debug mode)
  if (subject_ == nullptr)
    return;

  // Next, connect to the new subject (context)
  connect(subject_, SIGNAL(unitsChanged(simQt::UnitContext*)), this, SLOT(reemit_()));
  connect(subject_, SIGNAL(timeFormatChanged(simCore::TimeFormat)), this, SIGNAL(timeFormatChanged(simCore::TimeFormat)));
  connect(subject_, SIGNAL(timePrecisionChanged(unsigned int)), this, SIGNAL(timePrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(geodeticFormatChanged(const simCore::Units&)), this, SIGNAL(geodeticFormatChanged(const simCore::Units&)));
  connect(subject_, SIGNAL(geodeticPrecisionChanged(unsigned int)), this, SIGNAL(geodeticPrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(distanceUnitsChanged(const simCore::Units&)), this, SIGNAL(distanceUnitsChanged(const simCore::Units&)));
  connect(subject_, SIGNAL(distancePrecisionChanged(unsigned int)), this, SIGNAL(distancePrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(altitudeUnitsChanged(const simCore::Units&)), this, SIGNAL(altitudeUnitsChanged(const simCore::Units&)));
  connect(subject_, SIGNAL(altitudePrecisionChanged(unsigned int)), this, SIGNAL(altitudePrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(angleUnitsChanged(const simCore::Units&)), this, SIGNAL(angleUnitsChanged(const simCore::Units&)));
  connect(subject_, SIGNAL(anglePrecisionChanged(unsigned int)), this, SIGNAL(anglePrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(speedUnitsChanged(const simCore::Units&)), this, SIGNAL(speedUnitsChanged(const simCore::Units&)));
  connect(subject_, SIGNAL(speedPrecisionChanged(unsigned int)), this, SIGNAL(speedPrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(genericPrecisionChanged(unsigned int)), this, SIGNAL(genericPrecisionChanged(unsigned int)));
  connect(subject_, SIGNAL(coordinateSystemChanged(simCore::CoordinateSystem)), this, SIGNAL(coordinateSystemChanged(simCore::CoordinateSystem)));
  connect(subject_, SIGNAL(magneticVarianceChanged(simCore::MagneticVariance)), this, SIGNAL(magneticVarianceChanged(simCore::MagneticVariance)));
  connect(subject_, SIGNAL(verticalDatumChanged(simCore::VerticalDatum)), this, SIGNAL(verticalDatumChanged(simCore::VerticalDatum)));
  connect(subject_, SIGNAL(referenceYearChanged(int)), this, SIGNAL(referenceYearChanged(int)));
  connect(subject_, SIGNAL(datumConvertPtrChanged(simCore::DatumConvertPtr)), this, SIGNAL(datumConvertPtrChanged(simCore::DatumConvertPtr)));

  // Detect changes in any of the fields from the old context to the new context
  bool foundChange = false;
  if (oldSubject == nullptr || oldSubject->timeFormat() != subject_->timeFormat())
  {
    foundChange = true;
    Q_EMIT timeFormatChanged(subject_->timeFormat());
  }
  if (oldSubject == nullptr || oldSubject->timePrecision() != subject_->timePrecision())
  {
    foundChange = true;
    Q_EMIT timePrecisionChanged(subject_->timePrecision());
  }
  if (oldSubject == nullptr || oldSubject->geodeticFormat() != subject_->geodeticFormat())
  {
    foundChange = true;
    Q_EMIT geodeticFormatChanged(subject_->geodeticFormat());
  }
  if (oldSubject == nullptr || oldSubject->geodeticPrecision() != subject_->geodeticPrecision())
  {
    foundChange = true;
    Q_EMIT geodeticPrecisionChanged(subject_->geodeticPrecision());
  }
  if (oldSubject == nullptr || oldSubject->distanceUnits() != subject_->distanceUnits())
  {
    foundChange = true;
    Q_EMIT distanceUnitsChanged(subject_->distanceUnits());
  }
  if (oldSubject == nullptr || oldSubject->distancePrecision() != subject_->distancePrecision())
  {
    foundChange = true;
    Q_EMIT distancePrecisionChanged(subject_->distancePrecision());
  }
  if (oldSubject == nullptr || oldSubject->altitudeUnits() != subject_->altitudeUnits())
  {
    foundChange = true;
    Q_EMIT altitudeUnitsChanged(subject_->altitudeUnits());
  }
  if (oldSubject == nullptr || oldSubject->altitudePrecision() != subject_->altitudePrecision())
  {
    foundChange = true;
    Q_EMIT altitudePrecisionChanged(subject_->altitudePrecision());
  }
  if (oldSubject == nullptr || oldSubject->angleUnits() != subject_->angleUnits())
  {
    foundChange = true;
    Q_EMIT angleUnitsChanged(subject_->angleUnits());
  }
  if (oldSubject == nullptr || oldSubject->anglePrecision() != subject_->anglePrecision())
  {
    foundChange = true;
    Q_EMIT anglePrecisionChanged(subject_->anglePrecision());
  }
  if (oldSubject == nullptr || oldSubject->speedUnits() != subject_->speedUnits())
  {
    foundChange = true;
    Q_EMIT speedUnitsChanged(subject_->speedUnits());
  }
  if (oldSubject == nullptr || oldSubject->speedPrecision() != subject_->speedPrecision())
  {
    foundChange = true;
    Q_EMIT speedPrecisionChanged(subject_->speedPrecision());
  }
  if (oldSubject == nullptr || oldSubject->genericPrecision() != subject_->genericPrecision())
  {
    foundChange = true;
    Q_EMIT genericPrecisionChanged(subject_->genericPrecision());
  }
  if (oldSubject == nullptr || oldSubject->coordinateSystem() != subject_->coordinateSystem())
  {
    foundChange = true;
    Q_EMIT coordinateSystemChanged(subject_->coordinateSystem());
  }
  if (oldSubject == nullptr || oldSubject->magneticVariance() != subject_->magneticVariance())
  {
    foundChange = true;
    Q_EMIT magneticVarianceChanged(subject_->magneticVariance());
  }
  if (oldSubject == nullptr || oldSubject->verticalDatum() != subject_->verticalDatum())
  {
    foundChange = true;
    Q_EMIT verticalDatumChanged(subject_->verticalDatum());
  }
  if (oldSubject == nullptr || oldSubject->referenceYear() != subject_->referenceYear())
  {
    foundChange = true;
    Q_EMIT referenceYearChanged(subject_->referenceYear());
  }
  if (oldSubject == nullptr || oldSubject->datumConvert() != subject_->datumConvert())
  {
    foundChange = true;
    Q_EMIT datumConvertPtrChanged(subject_->datumConvert());
  }

  // Finally, detect any changes and send them out
  if (foundChange)
  {
    Q_EMIT unitsChanged(this);
  }

  // Tell anyone who cares that we changed our underlying subject
  Q_EMIT subjectChanged(subject_, oldSubject);
}

void UnitContextProxy::reemit_()
{
  Q_EMIT unitsChanged(this);
}

// Getters for various unit types
simCore::TimeFormat UnitContextProxy::timeFormat() const
{
  return subject_->timeFormat();
}

unsigned int UnitContextProxy::timePrecision() const
{
  return subject_->timePrecision();
}

simCore::CoordinateSystem UnitContextProxy::coordinateSystem() const
{
  return subject_->coordinateSystem();
}

simCore::GeodeticFormat UnitContextProxy::geodeticFormat() const
{
  return subject_->geodeticFormat();
}

unsigned int UnitContextProxy::geodeticPrecision() const
{
  return subject_->geodeticPrecision();
}

const simCore::Units& UnitContextProxy::distanceUnits() const
{
  return subject_->distanceUnits();
}

unsigned int UnitContextProxy::distancePrecision() const
{
  return subject_->distancePrecision();
}

const simCore::Units& UnitContextProxy::altitudeUnits() const
{
  return subject_->altitudeUnits();
}

unsigned int UnitContextProxy::altitudePrecision() const
{
  return subject_->altitudePrecision();
}

const simCore::Units& UnitContextProxy::angleUnits() const
{
  return subject_->angleUnits();
}

unsigned int UnitContextProxy::anglePrecision() const
{
  return subject_->anglePrecision();
}

const simCore::Units& UnitContextProxy::speedUnits() const
{
  return subject_->speedUnits();
}

unsigned int UnitContextProxy::speedPrecision() const
{
  return subject_->speedPrecision();
}

unsigned int UnitContextProxy::genericPrecision() const
{
  return subject_->genericPrecision();
}

simCore::MagneticVariance UnitContextProxy::magneticVariance() const
{
  return subject_->magneticVariance();
}

simCore::VerticalDatum UnitContextProxy::verticalDatum() const
{
  return subject_->verticalDatum();
}

int UnitContextProxy::referenceYear() const
{
  return subject_->referenceYear();
}

simCore::DatumConvertPtr UnitContextProxy::datumConvert() const
{
  return subject_->datumConvert();
}

void UnitContextProxy::setTimeFormat(simCore::TimeFormat unit)
{
  subject_->setTimeFormat(unit);
}

void UnitContextProxy::setTimePrecision(unsigned int prec)
{
  subject_->setTimePrecision(prec);
}

void UnitContextProxy::setCoordinateSystem(simCore::CoordinateSystem coordSys)
{
  subject_->setCoordinateSystem(coordSys);
}

void UnitContextProxy::setGeodeticFormat(simCore::GeodeticFormat unit)
{
  subject_->setGeodeticFormat(unit);
}

void UnitContextProxy::setGeodeticPrecision(unsigned int prec)
{
  subject_->setGeodeticPrecision(prec);
}

void UnitContextProxy::setDistanceUnits(const simCore::Units& unit)
{
  subject_->setDistanceUnits(unit);
}

void UnitContextProxy::setDistancePrecision(unsigned int prec)
{
  subject_->setDistancePrecision(prec);
}

void UnitContextProxy::setAltitudeUnits(const simCore::Units& unit)
{
  subject_->setAltitudeUnits(unit);
}

void UnitContextProxy::setAltitudePrecision(unsigned int prec)
{
  subject_->setAltitudePrecision(prec);
}

void UnitContextProxy::setAngleUnits(const simCore::Units& unit)
{
  subject_->setAngleUnits(unit);
}

void UnitContextProxy::setAnglePrecision(unsigned int prec)
{
  subject_->setAnglePrecision(prec);
}

void UnitContextProxy::setSpeedUnits(const simCore::Units& unit)
{
  subject_->setSpeedUnits(unit);
}

void UnitContextProxy::setSpeedPrecision(unsigned int prec)
{
  subject_->setSpeedPrecision(prec);
}

void UnitContextProxy::setGenericPrecision(unsigned int prec)
{
  subject_->setGenericPrecision(prec);
}

void UnitContextProxy::setMagneticVariance(simCore::MagneticVariance mv)
{
  subject_->setMagneticVariance(mv);
}

void UnitContextProxy::setVerticalDatum(simCore::VerticalDatum vd)
{
  subject_->setVerticalDatum(vd);
}

void UnitContextProxy::setReferenceYear(int refYear)
{
  subject_->setReferenceYear(refYear);
}

void UnitContextProxy::setDatumConvert(simCore::DatumConvertPtr datumConvert)
{
  subject_->setDatumConvert(datumConvert);
}

}
