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
  virtual ~UnitContext() {}

signals:
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
  virtual simCore::TimeFormat timeFormat() const;
  virtual unsigned int timePrecision() const;
  virtual simCore::GeodeticFormat geodeticFormat() const;
  virtual unsigned int geodeticPrecision() const;
  virtual const simCore::Units& distanceUnits() const;
  virtual unsigned int distancePrecision() const;
  virtual const simCore::Units& altitudeUnits() const;
  virtual unsigned int altitudePrecision() const;
  virtual const simCore::Units& angleUnits() const;
  virtual unsigned int anglePrecision() const;
  virtual const simCore::Units& speedUnits() const;
  virtual unsigned int speedPrecision() const;
  virtual unsigned int genericPrecision() const;
  virtual simCore::CoordinateSystem coordinateSystem() const;
  virtual simCore::MagneticVariance magneticVariance() const;
  virtual simCore::VerticalDatum verticalDatum() const;
  virtual int referenceYear() const;
  virtual simCore::DatumConvertPtr datumConvert() const;

public slots:
  // Setters for various unit types
  virtual void setTimeFormat(simCore::TimeFormat unit);
  virtual void setTimePrecision(unsigned int prec);
  virtual void setGeodeticFormat(simCore::GeodeticFormat unit);
  virtual void setGeodeticPrecision(unsigned int prec);
  virtual void setDistanceUnits(const simCore::Units& unit);
  virtual void setDistancePrecision(unsigned int prec);
  virtual void setAltitudeUnits(const simCore::Units& unit);
  virtual void setAltitudePrecision(unsigned int prec);
  virtual void setAngleUnits(const simCore::Units& unit);
  virtual void setAnglePrecision(unsigned int prec);
  virtual void setSpeedUnits(const simCore::Units& unit);
  virtual void setSpeedPrecision(unsigned int prec);
  virtual void setGenericPrecision(unsigned int prec);
  virtual void setCoordinateSystem(simCore::CoordinateSystem coordSys);
  virtual void setMagneticVariance(simCore::MagneticVariance mv);
  virtual void setVerticalDatum(simCore::VerticalDatum vd);
  virtual void setReferenceYear(int refYear);
  virtual void setDatumConvert(simCore::DatumConvertPtr convert);

private:
  simCore::TimeFormat timeFormat_;
  unsigned int timePrecision_;
  simCore::GeodeticFormat geodeticFormat_;
  unsigned int geodeticPrecision_;
  simCore::Units distanceUnits_;
  unsigned int distancePrecision_;
  simCore::Units altitudeUnits_;
  unsigned int altitudePrecision_;
  simCore::Units angleUnits_;
  unsigned int anglePrecision_;
  simCore::Units speedUnits_;
  unsigned int speedPrecision_;
  unsigned int genericPrecision_;
  simCore::CoordinateSystem coordinateSystem_;
  simCore::MagneticVariance magneticVariance_;
  simCore::VerticalDatum verticalDatum_;
  int referenceYear_;
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
  virtual simCore::TimeFormat timeFormat() const;
  virtual unsigned int timePrecision() const;
  virtual simCore::GeodeticFormat geodeticFormat() const;
  virtual unsigned int geodeticPrecision() const;
  virtual const simCore::Units& distanceUnits() const;
  virtual unsigned int distancePrecision() const;
  virtual const simCore::Units& altitudeUnits() const;
  virtual unsigned int altitudePrecision() const;
  virtual const simCore::Units& angleUnits() const;
  virtual unsigned int anglePrecision() const;
  virtual const simCore::Units& speedUnits() const;
  virtual unsigned int speedPrecision() const;
  virtual unsigned int genericPrecision() const;
  virtual simCore::CoordinateSystem coordinateSystem() const;
  virtual simCore::MagneticVariance magneticVariance() const;
  virtual simCore::VerticalDatum verticalDatum() const;
  virtual int referenceYear() const;
  virtual simCore::DatumConvertPtr datumConvert() const;

public slots:
  // Setters for various unit types
  virtual void setTimeFormat(simCore::TimeFormat unit);
  virtual void setTimePrecision(unsigned int prec);
  virtual void setGeodeticFormat(simCore::GeodeticFormat unit);
  virtual void setGeodeticPrecision(unsigned int prec);
  virtual void setDistanceUnits(const simCore::Units& unit);
  virtual void setDistancePrecision(unsigned int prec);
  virtual void setAltitudeUnits(const simCore::Units& unit);
  virtual void setAltitudePrecision(unsigned int prec);
  virtual void setAngleUnits(const simCore::Units& unit);
  virtual void setAnglePrecision(unsigned int prec);
  virtual void setSpeedUnits(const simCore::Units& unit);
  virtual void setSpeedPrecision(unsigned int prec);
  virtual void setGenericPrecision(unsigned int prec);
  virtual void setCoordinateSystem(simCore::CoordinateSystem coordSys);
  virtual void setMagneticVariance(simCore::MagneticVariance mv);
  virtual void setVerticalDatum(simCore::VerticalDatum vd);
  virtual void setReferenceYear(int refYear);
  virtual void setDatumConvert(simCore::DatumConvertPtr convert);

signals:
  /** Emitted once the subject of the proxy has changed. */
  void subjectChanged(simQt::UnitContext* newSubject, simQt::UnitContext* oldSubject);

private slots:
  /** We intercept the subject's unitsChanged() signal and re-emit it as our own for pointer integrity */
  void reemit_();

private:
  simQt::UnitContext* subject_;
};

}

#endif /* SIMQT_UNITCONTEXT_H */
