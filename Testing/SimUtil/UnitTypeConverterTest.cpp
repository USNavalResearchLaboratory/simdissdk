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
#include "osgEarth/Units"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Units.h"
#include "simUtil/UnitTypeConverter.h"

namespace {

int testFromOsgEarth()
{
  int rv = 0;
  simUtil::UnitTypeConverter conv;

  // Distance to core
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::CENTIMETERS) == simCore::Units::CENTIMETERS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::DATA_MILES) == simCore::Units::DATA_MILES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::FATHOMS) == simCore::Units::FATHOMS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::FEET) == simCore::Units::FEET);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::FEET_US_SURVEY) == simCore::Units::UNITLESS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::INCHES) == simCore::Units::INCHES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KILOFEET) == simCore::Units::KILOFEET);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KILOMETERS) == simCore::Units::KILOMETERS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KILOYARDS) == simCore::Units::KILOYARDS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::METERS) == simCore::Units::METERS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MILES) == simCore::Units::MILES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MILLIMETERS) == simCore::Units::MILLIMETERS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::NAUTICAL_MILES) == simCore::Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::YARDS) == simCore::Units::YARDS);

  // Angular to core
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::BAM) == simCore::Units::BAM);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::DEGREES) == simCore::Units::DEGREES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::NATO_MILS) == simCore::Units::MIL);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::RADIANS) == simCore::Units::RADIANS);
  // Decimal Hours has same name as "hours", elapsed time; skip the comparison
  //rv += SDK_ASSERT(conv.toCore(osgEarth::Units::DECIMAL_HOURS) == simCore::Units::UNITLESS);

  // temporal to core
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::DAYS) == simCore::Units::DAYS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::HOURS) == simCore::Units::HOURS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MICROSECONDS) == simCore::Units::MICROSECONDS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MILLISECONDS) == simCore::Units::MILLISECONDS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MINUTES) == simCore::Units::MINUTES);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::SECONDS) == simCore::Units::SECONDS);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::WEEKS) == simCore::Units::UNITLESS);

  // speed to core
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::FEET_PER_SECOND) == simCore::Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::YARDS_PER_SECOND) == simCore::Units::YARDS_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::METERS_PER_SECOND) == simCore::Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KILOMETERS_PER_SECOND) == simCore::Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KILOMETERS_PER_HOUR) == simCore::Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::MILES_PER_HOUR) == simCore::Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::DATA_MILES_PER_HOUR) == simCore::Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::KNOTS) == simCore::Units::KNOTS);

  // screen to core
  rv += SDK_ASSERT(conv.toCore(osgEarth::Units::PIXELS) == simCore::Units::UNITLESS);

  // Distance to data
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::CENTIMETERS) == simData::UNITS_CENTIMETERS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::DATA_MILES) == simData::UNITS_DATAMILES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::FATHOMS) == simData::UNITS_FATHOMS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::FEET) == simData::UNITS_FEET);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::FEET_US_SURVEY) == 0);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::INCHES) == simData::UNITS_INCHES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KILOFEET) == simData::UNITS_KILOFEET);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KILOMETERS) == simData::UNITS_KILOMETERS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KILOYARDS) == simData::UNITS_KILOYARDS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::METERS) == simData::UNITS_METERS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MILES) == simData::UNITS_MILES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MILLIMETERS) == simData::UNITS_MILLIMETERS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::NAUTICAL_MILES) == simData::UNITS_NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::YARDS) == simData::UNITS_YARDS);

  // Angular to data
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::BAM) == simData::UNITS_BAM);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::DEGREES) == simData::UNITS_DEGREES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::NATO_MILS) == simData::UNITS_MIL);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::RADIANS) == simData::UNITS_RADIANS);
  // Decimal Hours has same name as "hours", elapsed time; skip the comparison
  //rv += SDK_ASSERT(conv.toData(osgEarth::Units::DECIMAL_HOURS) == 0);

  // temporal to data
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::DAYS) == 0);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::HOURS) == simData::ELAPSED_HOURS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MICROSECONDS) == 0);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MILLISECONDS) == 0);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MINUTES) == simData::ELAPSED_MINUTES);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::SECONDS) == simData::ELAPSED_SECONDS);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::WEEKS) == 0);

  // speed to data
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::FEET_PER_SECOND) == simData::UNITS_FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::YARDS_PER_SECOND) == simData::UNITS_YARDS_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::METERS_PER_SECOND) == simData::UNITS_METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KILOMETERS_PER_SECOND) == simData::UNITS_KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KILOMETERS_PER_HOUR) == simData::UNITS_KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::MILES_PER_HOUR) == simData::UNITS_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::DATA_MILES_PER_HOUR) == simData::UNITS_DATAMILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::KNOTS) == simData::UNITS_KNOTS);

  // screen to data
  rv += SDK_ASSERT(conv.toData(osgEarth::Units::PIXELS) == 0);

  return rv;
}

int testFromData()
{
  int rv = 0;
  const osgEarth::Units OSGEARTH_NONE;
  simUtil::UnitTypeConverter conv;

  // time to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simData::ELAPSED_SECONDS) == osgEarth::Units::SECONDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::ELAPSED_MINUTES) == osgEarth::Units::MINUTES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::ELAPSED_HOURS) == osgEarth::Units::HOURS);

  // angle to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_RADIANS) == osgEarth::Units::RADIANS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_DEGREES) == osgEarth::Units::DEGREES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_DEGREES_MINUTES) == osgEarth::Units::DEGREES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_DEGREES_MINUTES_SECONDS) == osgEarth::Units::DEGREES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_UTM) == OSGEARTH_NONE);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_BAM) == osgEarth::Units::BAM);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_MIL) == osgEarth::Units::NATO_MILS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_MILLIRADIANS) == OSGEARTH_NONE);

  // distance to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_METERS) == osgEarth::Units::METERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KILOMETERS) == osgEarth::Units::KILOMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_YARDS) == osgEarth::Units::YARDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_MILES) == osgEarth::Units::MILES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_FEET) == osgEarth::Units::FEET);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_INCHES) == osgEarth::Units::INCHES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_NAUTICAL_MILES) == osgEarth::Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_CENTIMETERS) == osgEarth::Units::CENTIMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_MILLIMETERS) == osgEarth::Units::MILLIMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KILOYARDS) == osgEarth::Units::KILOYARDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_DATAMILES) == osgEarth::Units::DATA_MILES);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_FATHOMS) == osgEarth::Units::FATHOMS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KILOFEET) == osgEarth::Units::KILOFEET);

  // speed to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_METERS_PER_SECOND) == osgEarth::Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KILOMETERS_PER_HOUR) == osgEarth::Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KNOTS) == osgEarth::Units::KNOTS);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_MILES_PER_HOUR) == osgEarth::Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_FEET_PER_SECOND) == osgEarth::Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_KILOMETERS_PER_SECOND) == osgEarth::Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_DATAMILES_PER_HOUR) == osgEarth::Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simData::UNITS_YARDS_PER_SECOND) == osgEarth::Units::YARDS_PER_SECOND);

  // time to simCore
  rv += SDK_ASSERT(conv.toCore(simData::ELAPSED_SECONDS) == simCore::Units::SECONDS);
  rv += SDK_ASSERT(conv.toCore(simData::ELAPSED_MINUTES) == simCore::Units::MINUTES);
  rv += SDK_ASSERT(conv.toCore(simData::ELAPSED_HOURS) == simCore::Units::HOURS);

  // angle to simCore
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_RADIANS) == simCore::Units::RADIANS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_DEGREES) == simCore::Units::DEGREES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_DEGREES_MINUTES) == simCore::Units::DEGREES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_DEGREES_MINUTES_SECONDS) == simCore::Units::DEGREES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_UTM) == simCore::Units::UNITLESS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_BAM) == simCore::Units::BAM);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_MIL) == simCore::Units::MIL);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_MILLIRADIANS) == simCore::Units::MILLIRADIANS);

  // distance to simCore
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_METERS) == simCore::Units::METERS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KILOMETERS) == simCore::Units::KILOMETERS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_YARDS) == simCore::Units::YARDS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_MILES) == simCore::Units::MILES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_FEET) == simCore::Units::FEET);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_INCHES) == simCore::Units::INCHES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_NAUTICAL_MILES) == simCore::Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_CENTIMETERS) == simCore::Units::CENTIMETERS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_MILLIMETERS) == simCore::Units::MILLIMETERS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KILOYARDS) == simCore::Units::KILOYARDS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_DATAMILES) == simCore::Units::DATA_MILES);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_FATHOMS) == simCore::Units::FATHOMS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KILOFEET) == simCore::Units::KILOFEET);

  // speed to simCore
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_METERS_PER_SECOND) == simCore::Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KILOMETERS_PER_HOUR) == simCore::Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KNOTS) == simCore::Units::KNOTS);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_MILES_PER_HOUR) == simCore::Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_FEET_PER_SECOND) == simCore::Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_KILOMETERS_PER_SECOND) == simCore::Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_DATAMILES_PER_HOUR) == simCore::Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toCore(simData::UNITS_YARDS_PER_SECOND) == simCore::Units::YARDS_PER_SECOND);

  return rv;
}

int testFromCore()
{
  int rv = 0;
  const osgEarth::Units OSGEARTH_NONE;
  simUtil::UnitTypeConverter conv;

  // Distance to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::METERS) == osgEarth::Units::METERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KILOMETERS) == osgEarth::Units::KILOMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::YARDS) == osgEarth::Units::YARDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MILES) == osgEarth::Units::MILES);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::FEET) == osgEarth::Units::FEET);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::INCHES) == osgEarth::Units::INCHES);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::NAUTICAL_MILES) == osgEarth::Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::CENTIMETERS) == osgEarth::Units::CENTIMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MILLIMETERS) == osgEarth::Units::MILLIMETERS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KILOYARDS) == osgEarth::Units::KILOYARDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::FATHOMS) == osgEarth::Units::FATHOMS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KILOFEET) == osgEarth::Units::KILOFEET);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::DATA_MILES) == osgEarth::Units::DATA_MILES);

  // Angular to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::RADIANS) == osgEarth::Units::RADIANS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::DEGREES) == osgEarth::Units::DEGREES);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MILLIRADIANS) == OSGEARTH_NONE);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::BAM) == osgEarth::Units::BAM);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MIL) == osgEarth::Units::NATO_MILS);

  // temporal to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::SECONDS) == osgEarth::Units::SECONDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MILLISECONDS) == osgEarth::Units::MILLISECONDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MICROSECONDS) == osgEarth::Units::MICROSECONDS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MINUTES) == osgEarth::Units::MINUTES);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::HOURS) == osgEarth::Units::HOURS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::DAYS) == osgEarth::Units::DAYS);

  // speed to osgEarth
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::METERS_PER_SECOND) == osgEarth::Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KILOMETERS_PER_HOUR) == osgEarth::Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KNOTS) == osgEarth::Units::KNOTS);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::MILES_PER_HOUR) == osgEarth::Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::FEET_PER_SECOND) == osgEarth::Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::KILOMETERS_PER_SECOND) == osgEarth::Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::DATA_MILES_PER_HOUR) == osgEarth::Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toOsgEarth(simCore::Units::YARDS_PER_SECOND) == osgEarth::Units::YARDS_PER_SECOND);

  // acceleration is not supported in osgEarth, don't bother testing
  // frequency is not supported in osgEarth, don't bother testing

  // Distance to simData
  rv += SDK_ASSERT(conv.toData(simCore::Units::METERS) == simData::UNITS_METERS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KILOMETERS) == simData::UNITS_KILOMETERS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::YARDS) == simData::UNITS_YARDS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MILES) == simData::UNITS_MILES);
  rv += SDK_ASSERT(conv.toData(simCore::Units::FEET) == simData::UNITS_FEET);
  rv += SDK_ASSERT(conv.toData(simCore::Units::INCHES) == simData::UNITS_INCHES);
  rv += SDK_ASSERT(conv.toData(simCore::Units::NAUTICAL_MILES) == simData::UNITS_NAUTICAL_MILES);
  rv += SDK_ASSERT(conv.toData(simCore::Units::CENTIMETERS) == simData::UNITS_CENTIMETERS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MILLIMETERS) == simData::UNITS_MILLIMETERS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KILOYARDS) == simData::UNITS_KILOYARDS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::FATHOMS) == simData::UNITS_FATHOMS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KILOFEET) == simData::UNITS_KILOFEET);
  rv += SDK_ASSERT(conv.toData(simCore::Units::DATA_MILES) == simData::UNITS_DATAMILES);

  // Angular to simData
  rv += SDK_ASSERT(conv.toData(simCore::Units::RADIANS) == simData::UNITS_RADIANS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::DEGREES) == simData::UNITS_DEGREES);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MILLIRADIANS) == simData::UNITS_MILLIRADIANS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::BAM) == simData::UNITS_BAM);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MIL) == simData::UNITS_MIL);

  // temporal to simData
  rv += SDK_ASSERT(conv.toData(simCore::Units::SECONDS) == simData::ELAPSED_SECONDS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MILLISECONDS) == 0);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MICROSECONDS) == 0);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MINUTES) == simData::ELAPSED_MINUTES);
  rv += SDK_ASSERT(conv.toData(simCore::Units::HOURS) == simData::ELAPSED_HOURS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::DAYS) == 0);

  // speed to simData
  rv += SDK_ASSERT(conv.toData(simCore::Units::METERS_PER_SECOND) == simData::UNITS_METERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KILOMETERS_PER_HOUR) == simData::UNITS_KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KNOTS) == simData::UNITS_KNOTS);
  rv += SDK_ASSERT(conv.toData(simCore::Units::MILES_PER_HOUR) == simData::UNITS_MILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(simCore::Units::FEET_PER_SECOND) == simData::UNITS_FEET_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(simCore::Units::KILOMETERS_PER_SECOND) == simData::UNITS_KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(conv.toData(simCore::Units::DATA_MILES_PER_HOUR) == simData::UNITS_DATAMILES_PER_HOUR);
  rv += SDK_ASSERT(conv.toData(simCore::Units::YARDS_PER_SECOND) == simData::UNITS_YARDS_PER_SECOND);

  // acceleration is not supported in simData, don't bother testing
  // frequency is not supported in simData, don't bother testing

  return rv;
}

int testAddMapping()
{
  int rv = 0;

  simUtil::UnitTypeConverter conv;
  const simCore::Units coreDbsm("dBsm", "dBsm", 1.0, "dBsm");
  const simCore::Units corePercent("percent", "%", 1.0, "percent");
  const osgEarth::Units OSGEARTH_NONE;

  // Should not recognize the unit ahead of time
  rv += SDK_ASSERT(!conv.isRegistered(coreDbsm));
  rv += SDK_ASSERT(!conv.isRegistered(corePercent));
  rv += SDK_ASSERT(conv.toData(coreDbsm) == 0);
  rv += SDK_ASSERT(conv.toData(corePercent) == 0);
  rv += SDK_ASSERT(conv.toOsgEarth(coreDbsm) == OSGEARTH_NONE);
  rv += SDK_ASSERT(conv.toOsgEarth(corePercent) == OSGEARTH_NONE);

  // Will recognize the unit after registration
  conv.addMapping(OSGEARTH_NONE, coreDbsm, 0);
  conv.addMapping(OSGEARTH_NONE, corePercent, 5000); // use an artificial value
  conv.addMapping(simCore::Units("5001", "5k1", 1.0, "5001"), 5001); // use an artificial value

  // Should now recognize both dBsm and percent, and our temporary value
  rv += SDK_ASSERT(conv.isRegistered(coreDbsm));
  rv += SDK_ASSERT(conv.isRegistered(corePercent));
  rv += SDK_ASSERT(conv.toData(coreDbsm) == 0);
  rv += SDK_ASSERT(conv.toData(corePercent) == 5000);
  rv += SDK_ASSERT(conv.toOsgEarth(coreDbsm) == OSGEARTH_NONE);
  rv += SDK_ASSERT(conv.toOsgEarth(corePercent) == OSGEARTH_NONE);
  rv += SDK_ASSERT(!conv.toCoreFromData(0).isValid());
  rv += SDK_ASSERT(conv.toCoreFromData(5000).isValid());
  rv += SDK_ASSERT(conv.toCoreFromData(5001).isValid());
  rv += SDK_ASSERT(conv.toCoreFromData(5000).name() == "percent");
  rv += SDK_ASSERT(conv.toCoreFromData(5001).name() == "5001");
  rv += SDK_ASSERT(conv.toCoreFromData(5001).abbreviation() == "5k1");

  rv += SDK_ASSERT(conv.toOsgEarthFromData(6001).getName().empty());
  return rv;
}

int testHasAllDefaultUnits()
{
  int rv = 0;

  simUtil::UnitTypeConverter conv;
  simCore::UnitsRegistry registry;
  registry.registerDefaultUnits();
  std::vector<std::string> fams = registry.families();
  for (auto f = fams.begin(); f != fams.end(); ++f)
  {
    const std::vector<simCore::Units>& units = registry.units(*f);
    for (auto u = units.begin(); u != units.end(); ++u)
    {
      // Failure here means the UnitTypeConverter is missing units
      const bool isRegistered = conv.isRegistered(*u);
      rv += SDK_ASSERT(isRegistered);
      if (!isRegistered)
      {
        // Print a message to help those debugging the failure
        std::cout << "Not registered: " << (*u).name() << "\n";
      }
    }
  }
  return rv;
}

}

int UnitTypeConverterTest(int argc, char* argv[])
{
  int rv = 0;
  rv += testFromOsgEarth();
  rv += testFromData();
  rv += testFromCore();
  rv += testAddMapping();
  rv += testHasAllDefaultUnits();
  return rv;
}
