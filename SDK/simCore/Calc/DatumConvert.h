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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_DATUMCONVERT_H
#define SIMCORE_CALC_DATUMCONVERT_H

#include <memory>
#include "simCore/Common/Common.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/VerticalDatum.h"

namespace simCore {

class TimeStamp;
class Vec3;

/** Interface to a class expected to perform datum conversions for magnetic variance and vertical datum */
class SDKCORE_EXPORT DatumConvert
{
public:
  virtual ~DatumConvert() {}

  /**
   * Returns a modified bearing based on location, time, requested conversion and optional offset.
   * @param lla Position of recorded bearing origin, in radians and meters.
   * @param timeStamp Time of validity for the bearing.
   * @param bearingRad Magnetic datum bearing to be converted, in radians.
   * @param coordSystem Coordinate system of the supplied posit.
   * @param inputDatum Input type.
   * @param outputDatum Desired output type.
   * @param userOffset Offset from the supplied bearing param, in radians, for USER data.
   * @return value of converted datum in radians.
   */
  virtual double convertMagneticDatum(const Vec3& lla, const TimeStamp& timeStamp, double bearingRad,
    CoordinateSystem coordSystem, MagneticVariance inputDatum, MagneticVariance outputDatum,
    double userOffset) const = 0;

  /**
   * Returns a modified altitude based on location, time, requested conversion and optional offset.
   * Note that MSL conversions not supported for flat earth & TP systems.
   * @param lla Position of recorded altitude, in radians and meters
   * @param timeStamp Time of validity for the posit.
   * @param coordSystem Coordinate system of the supplied posit.
   * @param inputDatum Input type.
   * @param outputDatum Desired output type.
   * @param userOffset Offset from the supplied alt param, in meters.
   * @return value of converted datum in meters.
   */
  virtual double convertVerticalDatum(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem,
    VerticalDatum inputDatum, VerticalDatum outputDatum, double userOffset) = 0;
};

/** Typedef for a shared pointer to a datum convert instance */
typedef std::shared_ptr<DatumConvert> DatumConvertPtr;

/**
 * Datum convert that can convert between magnetic data, and user/WGS84 vertical data.
 * EGM96 conversion is not supported by this implementation.  This can be used as a
 * NULL-object implementation of the DatumConvert implementation.
 */
class SDKCORE_EXPORT MagneticDatumConvert : public DatumConvert
{
public:
  /** Initializes the WMM */
  MagneticDatumConvert();
  virtual ~MagneticDatumConvert();

  /// Converts Magnetic Datum
  virtual double convertMagneticDatum(const Vec3& lla, const TimeStamp& timeStamp, double bearingRad,
    CoordinateSystem coordSystem, MagneticVariance inputDatum, MagneticVariance outputDatum,
    double userOffset) const;

  /// Note: Does not support EGM96 (MSL)
  virtual double convertVerticalDatum(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem,
    VerticalDatum inputDatum, VerticalDatum outputDatum, double userOffset);

private:
  WorldMagneticModel* wmm_;
};

}

#endif /* SIMCORE_CALC_DATUMCONVERT_H */
