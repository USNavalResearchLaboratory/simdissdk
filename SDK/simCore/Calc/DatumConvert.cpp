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
#include "simCore/Common/Exception.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/DatumConvert.h"

namespace simCore {

class DatumConvertException : public simCore::Exception
{
public:
  DatumConvertException(const std::string& name, const std::string& desc)
    : simCore::Exception(name, desc, 0)
  {
    addName_();
  }
};

MagneticDatumConvert::MagneticDatumConvert()
  : wmm_(new WorldMagneticModel)
{
}

MagneticDatumConvert::~MagneticDatumConvert()
{
  delete wmm_;
  wmm_ = nullptr;
}

double MagneticDatumConvert::convertMagneticDatum(const Vec3& lla, const TimeStamp& timeStamp, double bearingRad,
  CoordinateSystem coordSystem, MagneticVariance inputDatum, MagneticVariance outputDatum,
  double userOffset) const
{
  if (inputDatum == outputDatum || coordSystem == COORD_SYS_ECI || coordSystem == COORD_SYS_ECEF)
    return bearingRad;

  // Get the TRUE bearing value
  double trueBearing = bearingRad;
  if (inputDatum == MAGVAR_USER)
    trueBearing -= userOffset;
  else if (inputDatum == MAGVAR_WMM)
  {
    // Although calculateTrueBearing can return an error, we can't do anything reasonable
    // with that error here, so we ignore it.
    wmm_->calculateTrueBearing(lla, timeStamp, trueBearing);
  }

  // Convert from TRUE to output format
  double outputBearing = trueBearing;
  if (outputDatum == MAGVAR_USER)
    outputBearing += userOffset;
  else if (outputDatum == MAGVAR_WMM)
  {
    // Although calculateTrueBearing can return an error, we can't do anything reasonable
    // with that error here, so we ignore it.
    wmm_->calculateMagneticBearing(lla, timeStamp, outputBearing);
  }

  // Return the angfix of the output
  return angFix2PI(outputBearing);
}

double MagneticDatumConvert::convertVerticalDatum(const Vec3& lla, const TimeStamp& timeStamp, CoordinateSystem coordSystem,
  VerticalDatum inputDatum, VerticalDatum outputDatum, double userOffset)
{
  if (inputDatum == outputDatum)
    return lla.alt();

  // Does not support MSL, throw exception in that case
  if (inputDatum == VERTDATUM_MSL || outputDatum == VERTDATUM_MSL)
    throw simCore::DatumConvertException("MagneticDatumConvert: ", "MSL is not supported");

  // Datum conversions not supported for earth centered systems
  if (coordSystem == COORD_SYS_ECEF || coordSystem == COORD_SYS_ECI)
    return lla.alt();

  // Handle the VERTDATA_USER cases...

  // Get the WGS84 height value
  double wgs84Altitude = lla.alt();
  if (inputDatum == VERTDATUM_USER)
    wgs84Altitude += userOffset;

  // Convert from TRUE to output format
  double outputAltitude = wgs84Altitude;
  if (outputDatum == VERTDATUM_USER)
    outputAltitude -= userOffset;

  return outputAltitude;
}

}
