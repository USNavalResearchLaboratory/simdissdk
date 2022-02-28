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
#ifndef SIMCORE_CALC_UNITCONTEXT_H
#define SIMCORE_CALC_UNITCONTEXT_H

#include <memory>
#include "simCore/Common/Common.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/DatumConvert.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/Calc/Units.h"
#include "simCore/String/Angle.h"
#include "simCore/Time/Constants.h"

namespace simCore {

class Units;

/**
 * Abstract interface that maintains the fields required to properly format unit values
 * into text.
 */
class SDKCORE_EXPORT UnitContext
{
public:
  /** Inherit a virtual destructor */
  virtual ~UnitContext() {}

  ///@{
  /// Getters for various unit types and formats
  virtual TimeFormat timeFormat() const = 0;
  virtual unsigned int timePrecision() const = 0;
  virtual GeodeticFormat geodeticFormat() const = 0;
  virtual unsigned int geodeticPrecision() const = 0;
  virtual const Units& distanceUnits() const = 0;
  virtual unsigned int distancePrecision() const = 0;
  virtual const Units& altitudeUnits() const = 0;
  virtual unsigned int altitudePrecision() const = 0;
  virtual const Units& angleUnits() const = 0;
  virtual unsigned int anglePrecision() const = 0;
  virtual const Units& speedUnits() const = 0;
  virtual unsigned int speedPrecision() const = 0;
  virtual unsigned int genericPrecision() const = 0;
  virtual CoordinateSystem coordinateSystem() const = 0;
  virtual MagneticVariance magneticVariance() const = 0;
  virtual VerticalDatum verticalDatum() const = 0;
  virtual int referenceYear() const = 0;
  /** Note that return value may be nullptr */
  virtual simCore::DatumConvertPtr datumConvert() const = 0;
  ///@}

  ///@{
  /// Setters for various unit types and formats
  virtual void setTimeFormat(TimeFormat unit) = 0;
  virtual void setTimePrecision(unsigned int prec) = 0;
  virtual void setGeodeticFormat(GeodeticFormat unit) = 0;
  virtual void setGeodeticPrecision(unsigned int prec) = 0;
  virtual void setDistanceUnits(const Units& unit) = 0;
  virtual void setDistancePrecision(unsigned int prec) = 0;
  virtual void setAltitudeUnits(const Units& unit) = 0;
  virtual void setAltitudePrecision(unsigned int prec) = 0;
  virtual void setAngleUnits(const Units& unit) = 0;
  virtual void setAnglePrecision(unsigned int prec) = 0;
  virtual void setSpeedUnits(const Units& unit) = 0;
  virtual void setSpeedPrecision(unsigned int prec) = 0;
  virtual void setGenericPrecision(unsigned int prec) = 0;
  virtual void setCoordinateSystem(CoordinateSystem coordSys) = 0;
  virtual void setMagneticVariance(MagneticVariance mv) = 0;
  virtual void setVerticalDatum(VerticalDatum vd) = 0;
  virtual void setReferenceYear(int refYear) = 0;
  virtual void setDatumConvert(simCore::DatumConvertPtr convert) = 0;
  ///@}
};

/**
 * Adapter class to the UnitContext that provides simple gets and sets for each field.
 */
class SDKCORE_EXPORT UnitContextAdapter : public simCore::UnitContext
{
public:
  /** Construct a new Unit Context Adapter */
  UnitContextAdapter();
  virtual ~UnitContextAdapter();

  ///@{
  /// Getters for various unit types and formats
  virtual TimeFormat timeFormat() const;
  virtual unsigned int timePrecision() const;
  virtual GeodeticFormat geodeticFormat() const;
  virtual unsigned int geodeticPrecision() const;
  virtual const Units& distanceUnits() const;
  virtual unsigned int distancePrecision() const;
  virtual const Units& altitudeUnits() const;
  virtual unsigned int altitudePrecision() const;
  virtual const Units& angleUnits() const;
  virtual unsigned int anglePrecision() const;
  virtual const Units& speedUnits() const;
  virtual unsigned int speedPrecision() const;
  virtual unsigned int genericPrecision() const;
  virtual CoordinateSystem coordinateSystem() const;
  virtual MagneticVariance magneticVariance() const;
  virtual VerticalDatum verticalDatum() const;
  virtual int referenceYear() const;
  virtual DatumConvertPtr datumConvert() const;
  ///@}

  ///@{
  /// Setters for various unit types and formats
  virtual void setTimeFormat(TimeFormat unit);
  virtual void setTimePrecision(unsigned int prec);
  virtual void setGeodeticFormat(GeodeticFormat unit);
  virtual void setGeodeticPrecision(unsigned int prec);
  virtual void setDistanceUnits(const Units& unit);
  virtual void setDistancePrecision(unsigned int prec);
  virtual void setAltitudeUnits(const Units& unit);
  virtual void setAltitudePrecision(unsigned int prec);
  virtual void setAngleUnits(const Units& unit);
  virtual void setAnglePrecision(unsigned int prec);
  virtual void setSpeedUnits(const Units& unit);
  virtual void setSpeedPrecision(unsigned int prec);
  virtual void setGenericPrecision(unsigned int prec);
  virtual void setCoordinateSystem(CoordinateSystem coordSys);
  virtual void setMagneticVariance(MagneticVariance mv);
  virtual void setVerticalDatum(VerticalDatum vd);
  virtual void setReferenceYear(int refYear);
  virtual void setDatumConvert(DatumConvertPtr convert);
  ///@}

private:
  TimeFormat timeFormat_;
  unsigned int timePrecision_;
  GeodeticFormat geodeticFormat_;
  unsigned int geodeticPrecision_;
  Units distanceUnits_;
  unsigned int distancePrecision_;
  Units altitudeUnits_;
  unsigned int altitudePrecision_;
  Units angleUnits_;
  unsigned int anglePrecision_;
  Units speedUnits_;
  unsigned int speedPrecision_;
  unsigned int genericPrecision_;
  CoordinateSystem coordinateSystem_;
  MagneticVariance magneticVariance_;
  VerticalDatum verticalDatum_;
  int referenceYear_;
  DatumConvertPtr datumConvert_;
};

}

#endif /* SIMCORE_CALC_UNITCONTEXT_H */
