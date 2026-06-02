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
#ifndef SIMQT_UNITCONTEXT_H
#define SIMQT_UNITCONTEXT_H

#include <QObject>
#include "simCore/Calc/UnitContext.h"

namespace simQt {

/**
 * Abstract interface that maintains the fields required to properly format unit values
 * into text.  Includes a suite of slots and signals to change values dynamically and
 * announce when changes have occurred to the internal state.
 */
class UnitContext : public QObject, public simCore::UnitContext
{
  Q_OBJECT;
public:
  /** Inherit a virtual destructor */
  virtual ~UnitContext() = default;

Q_SIGNALS:
  /** One of the values in the provided unit context has changed.  Emitted after individual changes. */
  void unitsChanged(simQt::UnitContext* context=nullptr);

  void timeFormatChanged(simCore::TimeFormat fmt=simCore::TIMEFORMAT_ORDINAL);
  void timePrecisionChanged(unsigned int prec=3);
  void geodeticFormatChanged(simCore::GeodeticFormat format=simCore::FMT_DEGREES);
  void geodeticPrecisionChanged(unsigned int prec=3);
  void distanceUnitsChanged(const simCore::Units& unit=simCore::Units::METERS);
  void distancePrecisionChanged(unsigned int prec=3);
  void altitudeUnitsChanged(const simCore::Units& unit=simCore::Units::METERS);
  void altitudePrecisionChanged(unsigned int prec=3);
  void angleUnitsChanged(const simCore::Units& unit=simCore::Units::DEGREES);
  void anglePrecisionChanged(unsigned int prec=3);
  void speedUnitsChanged(const simCore::Units& unit=simCore::Units::METERS_PER_SECOND);
  void speedPrecisionChanged(unsigned int prec=3);
  void genericPrecisionChanged(unsigned int prec=3);
  void coordinateSystemChanged(simCore::CoordinateSystem coordSys=simCore::COORD_SYS_LLA);
  void magneticVarianceChanged(simCore::MagneticVariance mv=simCore::MAGVAR_TRUE);
  void verticalDatumChanged(simCore::VerticalDatum vd=simCore::VERTDATUM_WGS84);
  void referenceYearChanged(int refYear=1970);
  void datumConvertPtrChanged(simCore::DatumConvertPtr datum);

protected:
  UnitContext(QObject* parent=nullptr) : QObject(parent) {}
};

/** Adapter class to the UnitContext that provides simple gets and sets for each field. */
class UnitContextAdapter : public simQt::UnitContext
{
  Q_OBJECT;
public:
  UnitContextAdapter(QObject* parent=nullptr);
  virtual ~UnitContextAdapter();

  // Getters for various unit types
  simCore::TimeFormat timeFormat() const override;
  unsigned int timePrecision() const override;
  simCore::GeodeticFormat geodeticFormat() const override;
  unsigned int geodeticPrecision() const override;
  const simCore::Units& distanceUnits() const override;
  unsigned int distancePrecision() const override;
  const simCore::Units& altitudeUnits() const override;
  unsigned int altitudePrecision() const override;
  const simCore::Units& angleUnits() const override;
  unsigned int anglePrecision() const override;
  const simCore::Units& speedUnits() const override;
  unsigned int speedPrecision() const override;
  unsigned int genericPrecision() const override;
  simCore::CoordinateSystem coordinateSystem() const override;
  simCore::MagneticVariance magneticVariance() const override;
  simCore::VerticalDatum verticalDatum() const override;
  int referenceYear() const override;
  simCore::DatumConvertPtr datumConvert() const override;

public Q_SLOTS:
  // Setters for various unit types
  void setTimeFormat(simCore::TimeFormat unit) override;
  void setTimePrecision(unsigned int prec) override;
  void setGeodeticFormat(simCore::GeodeticFormat unit) override;
  void setGeodeticPrecision(unsigned int prec) override;
  void setDistanceUnits(const simCore::Units& unit) override;
  void setDistancePrecision(unsigned int prec) override;
  void setAltitudeUnits(const simCore::Units& unit) override;
  void setAltitudePrecision(unsigned int prec) override;
  void setAngleUnits(const simCore::Units& unit) override;
  void setAnglePrecision(unsigned int prec) override;
  void setSpeedUnits(const simCore::Units& unit) override;
  void setSpeedPrecision(unsigned int prec) override;
  void setGenericPrecision(unsigned int prec) override;
  void setCoordinateSystem(simCore::CoordinateSystem coordSys) override;
  void setMagneticVariance(simCore::MagneticVariance mv) override;
  void setVerticalDatum(simCore::VerticalDatum vd) override;
  void setReferenceYear(int refYear) override;
  void setDatumConvert(simCore::DatumConvertPtr convert) override;

private:
  simCore::TimeFormat timeFormat_ = simCore::TIMEFORMAT_MONTHDAY;
  unsigned int timePrecision_ = 1u;
  simCore::GeodeticFormat geodeticFormat_ = simCore::FMT_DEGREES_MINUTES;
  unsigned int geodeticPrecision_ = 6u;
  simCore::Units distanceUnits_ = simCore::Units::METERS;
  unsigned int distancePrecision_ = 3u;
  simCore::Units altitudeUnits_ = simCore::Units::METERS;
  unsigned int altitudePrecision_ = 3u;
  simCore::Units angleUnits_ = simCore::Units::DEGREES;
  unsigned int anglePrecision_ = 3u;
  simCore::Units speedUnits_ = simCore::Units::METERS_PER_SECOND;
  unsigned int speedPrecision_ = 3u;
  unsigned int genericPrecision_ = 3u;
  simCore::CoordinateSystem coordinateSystem_ = simCore::COORD_SYS_LLA;
  simCore::MagneticVariance magneticVariance_ = simCore::MAGVAR_TRUE;
  simCore::VerticalDatum verticalDatum_ = simCore::VERTDATUM_WGS84;
  int referenceYear_ = 1970;
  simCore::DatumConvertPtr datumConvert_;
};

/**
 * Proxy pattern class for UnitContext.  The proxy subject can be set to another
 * UnitContext at will.  Changes between the old proxy and new proxy are detected
 * and the proper signals are emitted on change.
 */
class UnitContextProxy : public simQt::UnitContext
{
  Q_OBJECT;
public:
  UnitContextProxy(simQt::UnitContext* subject=nullptr);
  virtual ~UnitContextProxy();

  /** Changes the underlying subject of this proxy. */
  void setSubject(simQt::UnitContext* newSubject);
  /** Retrieves the current subject of this proxy */
  simQt::UnitContext* subject() const;

  // Getters for various unit types
  simCore::TimeFormat timeFormat() const override;
  unsigned int timePrecision() const override;
  simCore::GeodeticFormat geodeticFormat() const override;
  unsigned int geodeticPrecision() const override;
  const simCore::Units& distanceUnits() const override;
  unsigned int distancePrecision() const override;
  const simCore::Units& altitudeUnits() const override;
  unsigned int altitudePrecision() const override;
  const simCore::Units& angleUnits() const override;
  unsigned int anglePrecision() const override;
  const simCore::Units& speedUnits() const override;
  unsigned int speedPrecision() const override;
  unsigned int genericPrecision() const override;
  simCore::CoordinateSystem coordinateSystem() const override;
  simCore::MagneticVariance magneticVariance() const override;
  simCore::VerticalDatum verticalDatum() const override;
  int referenceYear() const override;
  simCore::DatumConvertPtr datumConvert() const override;

public Q_SLOTS:
  // Setters for various unit types
  void setTimeFormat(simCore::TimeFormat unit) override;
  void setTimePrecision(unsigned int prec) override;
  void setGeodeticFormat(simCore::GeodeticFormat unit) override;
  void setGeodeticPrecision(unsigned int prec) override;
  void setDistanceUnits(const simCore::Units& unit) override;
  void setDistancePrecision(unsigned int prec) override;
  void setAltitudeUnits(const simCore::Units& unit) override;
  void setAltitudePrecision(unsigned int prec) override;
  void setAngleUnits(const simCore::Units& unit) override;
  void setAnglePrecision(unsigned int prec) override;
  void setSpeedUnits(const simCore::Units& unit) override;
  void setSpeedPrecision(unsigned int prec) override;
  void setGenericPrecision(unsigned int prec) override;
  void setCoordinateSystem(simCore::CoordinateSystem coordSys) override;
  void setMagneticVariance(simCore::MagneticVariance mv) override;
  void setVerticalDatum(simCore::VerticalDatum vd) override;
  void setReferenceYear(int refYear) override;
  void setDatumConvert(simCore::DatumConvertPtr convert) override;

Q_SIGNALS:
  /** Emitted once the subject of the proxy has changed. */
  void subjectChanged(simQt::UnitContext* newSubject, simQt::UnitContext* oldSubject);

private Q_SLOTS:
  /** We intercept the subject's unitsChanged() signal and re-emit it as our own for pointer integrity */
  void reemit_();

private:
  simQt::UnitContext* subject_;
};

}

#endif /* SIMQT_UNITCONTEXT_H */
