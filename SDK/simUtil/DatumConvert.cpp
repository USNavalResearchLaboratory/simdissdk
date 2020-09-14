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
#include "osgEarth/VerticalDatum"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Time/TimeClass.h"
#include "simUtil/DatumConvert.h"

namespace simUtil {

DatumConvert::DatumConvert()
  : wmm_(new simCore::WorldMagneticModel),
    loaded84_(false),
    loaded96_(false),
    loaded2008_(false)
{
}

DatumConvert::~DatumConvert()
{
  delete wmm_;
  wmm_ = nullptr;
}

double DatumConvert::convertMagneticDatum(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, double bearingRad,
  simCore::CoordinateSystem coordSystem, simCore::MagneticVariance inputDatum, simCore::MagneticVariance outputDatum,
  double userOffset) const
{
  if (inputDatum == outputDatum || coordSystem == simCore::COORD_SYS_ECI || coordSystem == simCore::COORD_SYS_ECEF)
    return bearingRad;

  // Get the TRUE bearing value
  double trueBearing = bearingRad;
  if (inputDatum == simCore::MAGVAR_USER)
    trueBearing -= userOffset;
  else if (inputDatum == simCore::MAGVAR_WMM)
  {
    // Although calculateTrueBearing can return an error, we can't do anything reasonable
    // with that error here, so we ignore it.
    wmm_->calculateTrueBearing(lla, timeStamp, trueBearing);
  }

  // Convert from TRUE to output format
  double outputBearing = trueBearing;
  if (outputDatum == simCore::MAGVAR_USER)
    outputBearing += userOffset;
  else if (outputDatum == simCore::MAGVAR_WMM)
  {
    // Although calculateTrueBearing can return an error, we can't do anything reasonable
    // with that error here, so we ignore it.
    wmm_->calculateMagneticBearing(lla, timeStamp, outputBearing);
  }

  // Return the angfix of the output
  return simCore::angFix2PI(outputBearing);
}

double DatumConvert::convertVerticalDatum(const simCore::Vec3& lla, const simCore::TimeStamp& timeStamp, simCore::CoordinateSystem coordSystem,
  simCore::VerticalDatum inputDatum, simCore::VerticalDatum outputDatum, double userOffset)
{
  if (inputDatum == outputDatum)
    return lla.alt();

  // Cannot convert into or out of MSL from flat earth
  const bool isFlatEarth = (coordSystem == simCore::COORD_SYS_NED || coordSystem == simCore::COORD_SYS_ENU ||
    coordSystem == simCore::COORD_SYS_NWU || coordSystem == simCore::COORD_SYS_XEAST || coordSystem == simCore::COORD_SYS_GTP);
  if (isFlatEarth && (inputDatum == simCore::VERTDATUM_MSL || outputDatum == simCore::VERTDATUM_MSL))
    return lla.alt();

  // Datum conversions not supported for earth centered systems
  if (coordSystem == simCore::COORD_SYS_ECEF || coordSystem == simCore::COORD_SYS_ECI)
    return lla.alt();

  // Get an MSL converter based on the year, if needed
  osg::ref_ptr<osgEarth::VerticalDatum> msl;
  if (inputDatum == simCore::VERTDATUM_MSL || outputDatum == simCore::VERTDATUM_MSL)
  {
    const int year = timeStamp.referenceYear();
    if (year < 1996)
    {
      load84_();
      msl = egm84_;
    }
    else if (year < 2008)
    {
      load96_();
      msl = egm96_;
    }
    else
    {
      load2008_();
      msl = egm2008_;
    }
  }

  // Convert to WGS84 height

  // Get the WGS84 height value
  double wgs84Altitude = lla.alt();
  if (inputDatum == simCore::VERTDATUM_USER)
    wgs84Altitude += userOffset;
  else if (inputDatum == simCore::VERTDATUM_MSL && msl.valid())
  {
    // Convert the meters value to the MSL value
    double altitude = osgEarth::Units::METERS.convertTo(msl->getUnits(), wgs84Altitude);
    // Save the transformed coordinate to wgs84Altitude
    if (osgEarth::VerticalDatum::transform(msl.get(), nullptr, lla.lat() * simCore::RAD2DEG, lla.lon() * simCore::RAD2DEG, altitude))
    {
      wgs84Altitude = altitude;
    }
  }

  // Convert from TRUE to output format
  double outputAltitude = wgs84Altitude;
  if (outputDatum == simCore::VERTDATUM_USER)
    outputAltitude -= userOffset;
  else if (outputDatum == simCore::VERTDATUM_MSL && msl.valid())
  {
    // Convert the wgs84 altitude (meters) to MSL (MSL units)
    if (osgEarth::VerticalDatum::transform(nullptr, msl.get(), lla.lat() * simCore::RAD2DEG, lla.lon() * simCore::RAD2DEG, outputAltitude))
    {
      // Convert back from MSL units to meters
      outputAltitude = msl->getUnits().convertTo(osgEarth::Units::METERS, outputAltitude);
    }
  }

  return outputAltitude;
}

int DatumConvert::preloadVerticalDatum()
{
  int rv = 0;
  if (load84_() != 0)
    ++rv;
  if (load96_() != 0)
    ++rv;
  if (load2008_() != 0)
    ++rv;
  return rv;
}

int DatumConvert::load84_()
{
  if (egm84_.valid())
    return 0;
  if (loaded84_) // Attempted to load, but load failed; don't try again
    return 1;
  loaded84_ = true;
  egm84_ = osgEarth::VerticalDatum::get("egm84");
  return egm84_.valid() ? 0 : 1;
}

int DatumConvert::load96_()
{
  if (egm96_.valid())
    return 0;
  if (loaded96_) // Attempted to load, but load failed; don't try again
    return 1;
  loaded96_ = true;
  egm96_ = osgEarth::VerticalDatum::get("egm96");
  return egm96_.valid() ? 0 : 1;
}

int DatumConvert::load2008_()
{
  if (egm2008_.valid())
    return 0;
  if (loaded2008_) // Attempted to load, but load failed; don't try again
    return 1;
  loaded2008_ = true;
  egm2008_ = osgEarth::VerticalDatum::get("egm2008");
  return egm2008_.valid() ? 0 : 1;
}

}
