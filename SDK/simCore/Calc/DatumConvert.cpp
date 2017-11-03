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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/DatumConvert.h"

namespace simCore {

MagneticDatumConvert::MagneticDatumConvert()
  : wmm_(new WorldMagneticModel)
{
}

MagneticDatumConvert::~MagneticDatumConvert()
{
  delete wmm_;
  wmm_ = NULL;
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

  // Cannot convert into or out of MSL from flat earth
  const bool isFlatEarth = (coordSystem == COORD_SYS_NED || coordSystem == COORD_SYS_ENU ||
    coordSystem == COORD_SYS_NWU || coordSystem == COORD_SYS_XEAST || coordSystem == COORD_SYS_GTP);
  if (isFlatEarth && (inputDatum == VERTDATUM_MSL || outputDatum == VERTDATUM_MSL))
    return lla.alt();

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
