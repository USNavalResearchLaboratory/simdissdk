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
#ifdef HAVE_SIMUTIL
#include "simUtil/DatumConvert.h"
#endif /* HAVE_SIMUTIL */
#include "simCore/Time/TimeClass.h"
#include "simCore/Calc/UnitContext.h"
#include "simCore/String/UnitContextFormatter.h"
#include "simCore/Common/SDKAssert.h"

namespace
{

int tspiSetAll(simCore::UnitContext* iface,
  simCore::TimeFormat timeFormat,
  const simCore::Units& angleUnits,
  const simCore::Units& distanceUnits,
  const simCore::Units& speedUnits,
  simCore::CoordinateSystem coordinateSystem,
  simCore::VerticalDatum verticalDatum,
  simCore::MagneticVariance magneticVariance,
  unsigned int precision,
  int referenceYear)
{
  iface->setTimeFormat(timeFormat);
  iface->setAngleUnits(angleUnits);
  iface->setDistanceUnits(distanceUnits);
  iface->setSpeedUnits(speedUnits);
  iface->setCoordinateSystem(coordinateSystem);
  iface->setVerticalDatum(verticalDatum);
  iface->setMagneticVariance(magneticVariance);
  iface->setTimePrecision(precision);
  iface->setGeodeticPrecision(precision);
  iface->setDistancePrecision(precision);
  iface->setAltitudePrecision(precision);
  iface->setAnglePrecision(precision);
  iface->setSpeedPrecision(precision);
  iface->setGenericPrecision(precision);
  iface->setReferenceYear(referenceYear);
  int rv = 0;
  rv += SDK_ASSERT(iface->timeFormat() == timeFormat);
  rv += SDK_ASSERT(iface->angleUnits() == angleUnits);
  rv += SDK_ASSERT(iface->distanceUnits() == distanceUnits);
  rv += SDK_ASSERT(iface->speedUnits() == speedUnits);
  rv += SDK_ASSERT(iface->coordinateSystem() == coordinateSystem);
  rv += SDK_ASSERT(iface->verticalDatum() == verticalDatum);
  rv += SDK_ASSERT(iface->magneticVariance() == magneticVariance);
  rv += SDK_ASSERT(iface->genericPrecision() == precision);
  rv += SDK_ASSERT(iface->referenceYear() == referenceYear);
  return rv;
}

int testUnitsProviderAdapter()
{
  int rv = 0;
  simCore::UnitContextAdapter provider;
  // Simply test 2 different set() calls to ensure sets and gets match
  rv += SDK_ASSERT(0 == tspiSetAll(&provider, simCore::TIMEFORMAT_DTG, simCore::Units::RADIANS,
    simCore::Units::FEET, simCore::Units::KNOTS, simCore::COORD_SYS_ECI, simCore::VERTDATUM_MSL,
    simCore::MAGVAR_USER, 6, 2008));
  rv += SDK_ASSERT(0 == tspiSetAll(&provider, simCore::TIMEFORMAT_MONTHDAY, simCore::Units::MIL,
    simCore::Units::INCHES, simCore::Units::FEET_PER_SECOND, simCore::COORD_SYS_GTP,
    simCore::VERTDATUM_WGS84, simCore::MAGVAR_WMM, 4, 2012));
  return rv;
}

bool latLonStringEquals(const std::string& latLon, const std::string& beforeDeg, const std::string& afterDeg)
{
  return (latLon.find(beforeDeg) == 0 && latLon.find(afterDeg) == latLon.length() - afterDeg.length());
}

int testLatitudes(const simCore::TextFormatter* fmt, simCore::DegreeSymbolFormat degreeFormat, simCore::UnitContextAdapter* units)
{
  int rv = 0;
  std::string suffix = simCore::getDegreeSymbol(degreeFormat);
  units->setGeodeticFormat(simCore::FMT_DEGREES);
  units->setGeodeticPrecision(1);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(0, degreeFormat), "0.0" + suffix, " N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(simCore::DEG2RAD * 80.0, degreeFormat), "80.0" + suffix, " N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(simCore::DEG2RAD * 100.0, degreeFormat), "80.0" + suffix, " N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(simCore::DEG2RAD * -1.56, degreeFormat), "1.6" + suffix, " S"));
  units->setGeodeticFormat(simCore::FMT_DEGREES_MINUTES);
  units->setGeodeticPrecision(3);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(0, degreeFormat), "0" + suffix, " 00.0' N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(simCore::DEG2RAD * 80.0, degreeFormat), "80" + suffix, " 00.0' N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(simCore::DEG2RAD * -1.56, degreeFormat), "1" + suffix, " 33.6' S"));
  units->setGeodeticFormat(simCore::FMT_RADIANS);
  units->setGeodeticPrecision(1);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(0, degreeFormat), "0.0", " N"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(1.396, degreeFormat), "1.4", " N")); // 80 degrees
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(-0.0272, degreeFormat), "0.0", " S")); // -1.56 degrees
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLatitude(-0.2, degreeFormat), "0.2", " S")); // -1.56 degrees
  return rv;
}

int testLongitudes(const simCore::TextFormatter* fmt, simCore::DegreeSymbolFormat degreeFormat, simCore::UnitContextAdapter* units)
{
  int rv = 0;
  units->setGeodeticFormat(simCore::FMT_DEGREES);
  units->setGeodeticPrecision(1);
  std::string suffix = simCore::getDegreeSymbol(degreeFormat);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(0, degreeFormat), "0.0" + suffix, " E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * 100.0, degreeFormat), "100.0" + suffix, " E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * 170.0, degreeFormat), "170.0" + suffix, " E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * 190.0, degreeFormat), "170.0" + suffix, " W"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * -1.56, degreeFormat), "1.6" + suffix, " W"));
  units->setGeodeticFormat(simCore::FMT_DEGREES_MINUTES);
  units->setGeodeticPrecision(3);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(0, degreeFormat), "0" + suffix, " 00.0' E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * 100.0, degreeFormat), "100" + suffix, " 00.0' E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(simCore::DEG2RAD * -1.56, degreeFormat), "1" + suffix, " 33.6' W"));
  units->setGeodeticFormat(simCore::FMT_RADIANS);
  units->setGeodeticPrecision(1);
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(0, degreeFormat), "0.0", " E"));
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(1.396, degreeFormat), "1.4", " E")); // 80 degrees
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(-0.0272, degreeFormat), "0.0", " W")); // -1.56 degrees
  rv += SDK_ASSERT(latLonStringEquals(fmt->formatLongitude(-0.2, degreeFormat), "0.2", " W")); // -1.56 degrees
  return rv;
}

int testAngles(const simCore::TextFormatter* fmt, simCore::DegreeSymbolFormat degreeFormat, simCore::UnitContextAdapter* units)
{
  units->setAngleUnits(simCore::Units::DEGREES);
  int rv = 0;
  std::string suffix = simCore::getDegreeSymbol(degreeFormat);
  rv += SDK_ASSERT(fmt->formatAngle(0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "0.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 100.0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "100.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * -1.56, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "-1.6" + suffix);
  // Make sure DMD doesn't change the angles
  units->setAngleUnits(simCore::Units::DEGREES);
  rv += SDK_ASSERT(fmt->formatAngle(0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "0.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 100.0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "100.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * -1.56, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "-1.6" + suffix);
  units->setAngleUnits(simCore::Units::RADIANS);
  rv += SDK_ASSERT(fmt->formatAngle(0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "0.0");
  rv += SDK_ASSERT(fmt->formatAngle(100.0, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "100.0");
  rv += SDK_ASSERT(fmt->formatAngle(-1.56, simCore::ANGLEEXTENTS_ALL, degreeFormat) == "-1.6");

  units->setAngleUnits(simCore::Units::DEGREES);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 0.0, simCore::ANGLEEXTENTS_TWOPI, degreeFormat) == "0.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 0.0, simCore::ANGLEEXTENTS_PI, degreeFormat) == "0.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 0.0, simCore::ANGLEEXTENTS_PI_2, degreeFormat) == "0.0" + suffix);

  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 361.0, simCore::ANGLEEXTENTS_TWOPI, degreeFormat) == "1.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 361.0, simCore::ANGLEEXTENTS_PI, degreeFormat) == "1.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 361.0, simCore::ANGLEEXTENTS_PI_2, degreeFormat) == "1.0" + suffix);

  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 91.0, simCore::ANGLEEXTENTS_TWOPI, degreeFormat) == "91.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 91.0, simCore::ANGLEEXTENTS_PI, degreeFormat) == "91.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 91.0, simCore::ANGLEEXTENTS_PI_2, degreeFormat) == "90.0" + suffix);

  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 181.0, simCore::ANGLEEXTENTS_TWOPI, degreeFormat) == "181.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 181.0, simCore::ANGLEEXTENTS_PI, degreeFormat) == "-179.0" + suffix);
  rv += SDK_ASSERT(fmt->formatAngle(simCore::DEG2RAD * 181.0, simCore::ANGLEEXTENTS_PI_2, degreeFormat) == "-90.0" + suffix);

  return rv;
}

int testAzimuths(const simCore::TextFormatter* fmt, simCore::DegreeSymbolFormat degreeFormat, simCore::UnitContextAdapter* units)
{
  units->setAngleUnits(simCore::Units::DEGREES);
  units->setAnglePrecision(1);
  int rv = 0;

  // WMM values as of 10/03/13
  std::string suffix = simCore::getDegreeSymbol(degreeFormat);
  units->setMagneticVariance(simCore::MAGVAR_TRUE);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 0.0, degreeFormat) == "0.0" + suffix);
  fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 0.0, degreeFormat);
  units->setMagneticVariance(simCore::MAGVAR_USER);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 10.0 * simCore::DEG2RAD, degreeFormat) == "10.0" + suffix);
  fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 10.0 * simCore::DEG2RAD, degreeFormat);
  units->setMagneticVariance(simCore::MAGVAR_WMM);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 0.0, degreeFormat) == "5.8" + suffix);

  units->setAngleUnits(simCore::Units::RADIANS);
  units->setMagneticVariance(simCore::MAGVAR_TRUE);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 0.0, degreeFormat) == "0.0");
  units->setMagneticVariance(simCore::MAGVAR_USER);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 10.0, degreeFormat) == "3.7");  // formatAzimuth has a call to simCore::angFix2PI
  units->setMagneticVariance(simCore::MAGVAR_WMM);
  rv += SDK_ASSERT(fmt->formatAzimuth(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), 0.0, simCore::COORD_SYS_LLA, 0.0, degreeFormat) == "0.1");

  return rv;
}

int testDistances(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units)
{
  units->setDistanceUnits(simCore::Units::METERS);
  int rv = 0;
  rv += SDK_ASSERT(fmt->formatDistance(0) == "0.0");
  rv += SDK_ASSERT(fmt->formatDistance(100.0) == "100.0");
  rv += SDK_ASSERT(fmt->formatDistance(-1.56) == "-1.6");
  units->setDistanceUnits(simCore::Units::FEET);
  rv += SDK_ASSERT(fmt->formatDistance(simCore::Units::FEET.convertTo(simCore::Units::METERS, 0.0)) == "0.0");
  rv += SDK_ASSERT(fmt->formatDistance(simCore::Units::FEET.convertTo(simCore::Units::METERS, 100.0)) == "100.0");
  rv += SDK_ASSERT(fmt->formatDistance(simCore::Units::FEET.convertTo(simCore::Units::METERS, -1.56)) == "-1.6");
  return rv;
}

int testAltitudes(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units, bool testMsl)
{
  units->setAltitudeUnits(simCore::Units::METERS);
  units->setAltitudePrecision(1);
  int rv = 0;

  rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_WGS84) == "0.0");
  rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 10.0, simCore::VERTDATUM_USER) == "-10.0");
  // The following test is useful if MSL is implemented in the SDK
  if (testMsl)
  {
    // Vertical Datum values as of 7/12/16, validated against:
    //  * http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/intpt.html
    //  * http://geographiclib.sourceforge.net/cgi-bin/GeoidEval
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-17.2");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2008, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-17.2");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2007, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-17.2");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(1988, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-18.3");

    // Spot check 4 spots in EGM 1984
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1985, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-11.9");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1985, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-16.9");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1985, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-14.4");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1985, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "7.9");

    // Spot check 4 spots in EGM 1996
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1997, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-9.8");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1997, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-16.3");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1997, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-13.6");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(1997, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "8.7");

    // Spot check 4 spots in EGM 2008
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.9");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-16.6");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, 40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-13.7");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(-30 * simCore::DEG2RAD, -40 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "8.5");

    // Spot check boundary conditions on EGM 2008
    units->setAltitudePrecision(2);
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.24 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.04");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.25 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.01");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.26 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-9.99");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.12 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.45");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.125 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.43");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(30 * simCore::DEG2RAD, 40.13 * simCore::DEG2RAD, 0.0), simCore::TimeStamp(2010, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-10.41");
    units->setAltitudePrecision(1);
  }

  units->setAltitudeUnits(simCore::Units::FEET);
  rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_WGS84) == "0.0");
  rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 10.0, simCore::VERTDATUM_USER) == "-32.8");
  // The following test is useful if MSL is implemented in the SDK
  if (testMsl)
  {
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2013, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-56.5");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2008, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-56.5");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(2007, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-56.3");
    rv += SDK_ASSERT(fmt->formatAltitude(simCore::Vec3(0.0, 0.0, 0.0), simCore::TimeStamp(1988, 0.0), simCore::COORD_SYS_LLA, 0.0, simCore::VERTDATUM_MSL) == "-60.1");
  }

  units->setAltitudeUnits(simCore::Units::METERS);
  rv += SDK_ASSERT(fmt->formatAltitude(0.0) == "0.0");
  rv += SDK_ASSERT(fmt->formatAltitude(10.0) == "10.0");

  return rv;
}

int testSpeeds(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units)
{
  units->setSpeedUnits(simCore::Units::METERS_PER_SECOND);
  int rv = 0;
  rv += SDK_ASSERT(fmt->formatSpeed(0) == "0.0");
  rv += SDK_ASSERT(fmt->formatSpeed(100.0) == "100.0");
  rv += SDK_ASSERT(fmt->formatSpeed(-1.56) == "-1.6");
  units->setSpeedUnits(simCore::Units::KNOTS);
  rv += SDK_ASSERT(fmt->formatSpeed(simCore::Units::KNOTS.convertTo(simCore::Units::METERS_PER_SECOND, 0.0)) == "0.0");
  rv += SDK_ASSERT(fmt->formatSpeed(simCore::Units::KNOTS.convertTo(simCore::Units::METERS_PER_SECOND, 100.0)) == "100.0");
  rv += SDK_ASSERT(fmt->formatSpeed(simCore::Units::KNOTS.convertTo(simCore::Units::METERS_PER_SECOND, -1.56)) == "-1.6");
  return rv;
}

int testTimes(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units)
{
  units->setTimeFormat(simCore::TIMEFORMAT_SECONDS);
  units->setReferenceYear(2012);
  units->setTimePrecision(1);
  int rv = 0;
  // Note: Time formatting is NOT set to chop 0's
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(0.04) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(0.05) == "0.1");
  rv += SDK_ASSERT(fmt->formatTime(3601.22) == "3601.2");
  rv += SDK_ASSERT(fmt->formatTime(86403.56) == "86403.6");
  rv += SDK_ASSERT(fmt->formatTime(-1) == "Static");
  rv += SDK_ASSERT(fmt->formatTime(-100) == "Static");
  units->setTimeFormat(simCore::TIMEFORMAT_ORDINAL);
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "001 2012 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(0.04) == "001 2012 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(0.05) == "001 2012 00:00:00.1");
  rv += SDK_ASSERT(fmt->formatTime(3601.22) == "001 2012 01:00:01.2");
  rv += SDK_ASSERT(fmt->formatTime(86403.56) == "002 2012 00:00:03.6");
  rv += SDK_ASSERT(fmt->formatTime(-1) == "Static");
  rv += SDK_ASSERT(fmt->formatTime(-100) == "Static");

  // Test scenario roll over (see review 725)
  units->setReferenceYear(1970); // 2012 is a leap year, let's use a non-leap year
  const double ONE_YEAR = 365.0 * 24 * 60 * 60;
  // Basic tests of years in ordinal format
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "001 1970 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(ONE_YEAR) == "001 1971 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(2 * ONE_YEAR) == "001 1972 00:00:00.0");
  // Use simCore::TimeStamp in ordinal
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 0.0)) == "001 1970 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, ONE_YEAR)) == "001 1971 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, ONE_YEAR)) == "001 1972 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 2 * ONE_YEAR)) == "001 1972 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1972, 0)) == "001 1972 00:00:00.0");

  // Same tests, in seconds time format
  units->setTimeFormat(simCore::TIMEFORMAT_SECONDS);
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(ONE_YEAR) == "31536000.0");
  rv += SDK_ASSERT(fmt->formatTime(2 * ONE_YEAR) == "63072000.0");
  // Use simCore::TimeStamp in ordinal
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 0.0)) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, ONE_YEAR)) == "31536000.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, ONE_YEAR)) == "63072000.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 2 * ONE_YEAR)) == "63072000.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1972, 0)) == "63072000.0");

  // Test the invalid time stamp case (time stamp < scenario origin)
  units->setReferenceYear(1971);
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(ONE_YEAR) == "31536000.0");
  // no good way to represent this one...
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 0.0)) == "-31536000.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, ONE_YEAR)) == "0.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 2 * ONE_YEAR)) == "31536000.0");
  // Swap back to ordinal and check the same conditions
  units->setTimeFormat(simCore::TIMEFORMAT_ORDINAL);
  rv += SDK_ASSERT(fmt->formatTime(0.0) == "001 1971 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(ONE_YEAR) == "001 1972 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 0.0)) == "001 1970 00:00:00.0");  // decent representation
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, ONE_YEAR)) == "001 1971 00:00:00.0");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1970, 2 * ONE_YEAR)) == "001 1972 00:00:00.0");

  // SIM-3722: Test that rounding up works
  units->setTimePrecision(0);
  units->setGeodeticPrecision(0);
  units->setDistancePrecision(0);
  units->setAltitudePrecision(0);
  units->setAnglePrecision(0);
  units->setSpeedPrecision(0);
  units->setGenericPrecision(0);
  units->setTimeFormat(simCore::TIMEFORMAT_SECONDS);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3.9)) == "4");
  units->setTimeFormat(simCore::TIMEFORMAT_MINUTES);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3.9)) == "0:04");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 8 * 60 - 0.1)) == "8:00");
  units->setTimeFormat(simCore::TIMEFORMAT_HOURS);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 8 * 60 - 0.1)) == "0:08:00");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3600 - 0.1)) == "1:00:00");
  units->setTimeFormat(simCore::TIMEFORMAT_ORDINAL);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 8 * 60 - 0.1)) == "001 1971 00:08:00");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3600 - 0.1)) == "001 1971 01:00:00");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 24 * 3600 - 0.1)) == "002 1971 00:00:00");
  units->setTimeFormat(simCore::TIMEFORMAT_MONTHDAY);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 8 * 60 - 0.1)) == "Jan 1 1971 00:08:00");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3600 - 0.1)) == "Jan 1 1971 01:00:00");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 24 * 3600 - 0.1)) == "Jan 2 1971 00:00:00");
  units->setTimeFormat(simCore::TIMEFORMAT_DTG);
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 8 * 60 - 0.1)) == "010008:00 Z Jan71");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 3600 - 0.1)) == "010100:00 Z Jan71");
  rv += SDK_ASSERT(fmt->formatTime(simCore::TimeStamp(1971, 24 * 3600 - 0.1)) == "020000:00 Z Jan71");

  return rv;
}

int testDoubles(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units)
{
  units->setGenericPrecision(0);
  int rv = 0;
  rv += SDK_ASSERT(fmt->formatDouble(0.0) == "0");
  rv += SDK_ASSERT(fmt->formatDouble(-0.4) == "0");
  rv += SDK_ASSERT(fmt->formatDouble(-1.50000001) == "-2");
  rv += SDK_ASSERT(fmt->formatDouble(1.50000001) == "2");
  rv += SDK_ASSERT(fmt->formatDouble(1234567890123.05) == "1234567890123");
  units->setGenericPrecision(2);
  rv += SDK_ASSERT(fmt->formatDouble(0.001) == "0.00");
  rv += SDK_ASSERT(fmt->formatDouble(-0.001) == "0.00");
  rv += SDK_ASSERT(fmt->formatDouble(-0.4) == "-0.40");
  rv += SDK_ASSERT(fmt->formatDouble(-0.5) == "-0.50");
  rv += SDK_ASSERT(fmt->formatDouble(0.511) == "0.51");
  rv += SDK_ASSERT(fmt->formatDouble(1234567890123.05) == "1234567890123.05");
  return rv;
}

int testColors(const simCore::TextFormatter* fmt, simCore::UnitContextAdapter* units)
{
  int rv = 0;
  rv += SDK_ASSERT(fmt->formatRGBA(255, 0, 0, 255) == "0xff0000ff");
  rv += SDK_ASSERT(fmt->formatRGBA(0, 255, 0, 255) == "0x00ff00ff");
  rv += SDK_ASSERT(fmt->formatRGBA(0, 0, 64, 128) == "0x00004080");
  rv += SDK_ASSERT(fmt->formatRGBA(128, 64, 0, 0) == "0x80400000");
  rv += SDK_ASSERT(fmt->formatRGBA(0, 0, 0, 0) == "0x00000000");
  rv += SDK_ASSERT(fmt->formatRGBA(255, 255, 255, 255) == "0xffffffff");
  // We clamp colors to 255
  rv += SDK_ASSERT(fmt->formatRGBA(256, 300, 1200, 600) == "0xffffffff");
  // Test the RGBA(uint32) version
  rv += SDK_ASSERT(fmt->formatRGBA(0xff0000ff) == "0xff0000ff");
  rv += SDK_ASSERT(fmt->formatRGBA(0x00ff00ff) == "0x00ff00ff");
  rv += SDK_ASSERT(fmt->formatRGBA(0x00004080) == "0x00004080");
  rv += SDK_ASSERT(fmt->formatRGBA(0x80400000) == "0x80400000");
  rv += SDK_ASSERT(fmt->formatRGBA(0x00000000) == "0x00000000");
  rv += SDK_ASSERT(fmt->formatRGBA(0xffffffff) == "0xffffffff");
  rv += SDK_ASSERT(fmt->formatRGBA(0xff) == "0x000000ff");
  // Test the ABGR(uint32) version
  rv += SDK_ASSERT(fmt->formatABGR(0xff0000ff) == "0xff0000ff");
  rv += SDK_ASSERT(fmt->formatABGR(0x00ff00ff) == "0xff00ff00");
  rv += SDK_ASSERT(fmt->formatABGR(0x00004080) == "0x80400000");
  rv += SDK_ASSERT(fmt->formatABGR(0x80400000) == "0x00004080");
  rv += SDK_ASSERT(fmt->formatABGR(0x00000000) == "0x00000000");
  rv += SDK_ASSERT(fmt->formatABGR(0xffffffff) == "0xffffffff");
  rv += SDK_ASSERT(fmt->formatABGR(0xff) == "0xff000000");
  return rv;
}


int testFormatter()
{
  int rv = 0;
  simCore::UnitContextAdapter provider;
  provider.setTimePrecision(1);
  provider.setGeodeticPrecision(1);
  provider.setDistancePrecision(1);
  provider.setAltitudePrecision(1);
  provider.setAnglePrecision(1);
  provider.setSpeedPrecision(1);
  provider.setGenericPrecision(1);
  // Couple together the units to the text formatter
  simCore::UnitContextFormatter formatter(&provider);
  rv += SDK_ASSERT(0 == testLatitudes(&formatter, simCore::DEG_SYM_NONE, &provider));
  rv += SDK_ASSERT(0 == testLatitudes(&formatter, simCore::DEG_SYM_UNICODE, &provider));
  rv += SDK_ASSERT(0 == testLatitudes(&formatter, simCore::DEG_SYM_ASCII, &provider));
  rv += SDK_ASSERT(0 == testLongitudes(&formatter, simCore::DEG_SYM_NONE, &provider));
  rv += SDK_ASSERT(0 == testLongitudes(&formatter, simCore::DEG_SYM_UNICODE, &provider));
  rv += SDK_ASSERT(0 == testLongitudes(&formatter, simCore::DEG_SYM_ASCII, &provider));
  rv += SDK_ASSERT(0 == testAngles(&formatter, simCore::DEG_SYM_NONE, &provider));
  rv += SDK_ASSERT(0 == testAngles(&formatter, simCore::DEG_SYM_UNICODE, &provider));
  rv += SDK_ASSERT(0 == testAngles(&formatter, simCore::DEG_SYM_ASCII, &provider));
  rv += SDK_ASSERT(0 == testAzimuths(&formatter, simCore::DEG_SYM_NONE, &provider));
  rv += SDK_ASSERT(0 == testAzimuths(&formatter, simCore::DEG_SYM_UNICODE, &provider));
  rv += SDK_ASSERT(0 == testAzimuths(&formatter, simCore::DEG_SYM_ASCII, &provider));
  rv += SDK_ASSERT(0 == testDistances(&formatter, &provider));
  rv += SDK_ASSERT(0 == testAltitudes(&formatter, &provider, false));
  rv += SDK_ASSERT(0 == testSpeeds(&formatter, &provider));
  rv += SDK_ASSERT(0 == testTimes(&formatter, &provider));
  rv += SDK_ASSERT(0 == testDoubles(&formatter, &provider));
  rv += SDK_ASSERT(0 == testColors(&formatter, &provider));

  // Add a quick test on datum conversion with simUtil code
#ifdef HAVE_SIMUTIL
  std::shared_ptr<simUtil::DatumConvert> dc(std::make_shared<simUtil::DatumConvert>());
  provider.setDatumConvert(dc);
  // Tests WMM and Vertical Datum
  rv += SDK_ASSERT(0 == testAzimuths(&formatter, simCore::DEG_SYM_NONE, &provider));

  const bool haveVd = (dc->preloadVerticalDatum() == 0);
  rv += SDK_ASSERT(0 == testAltitudes(&formatter, &provider, haveVd));
#endif
  return rv;
}
}

int UnitsFormatter(int argc, char* argv[])
{
  int rv = 0;
  rv += testUnitsProviderAdapter();
  rv += testFormatter();
  return rv;
}
