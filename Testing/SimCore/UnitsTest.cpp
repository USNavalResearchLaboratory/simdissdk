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
#include <algorithm>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"

using namespace simCore;

namespace {

int testRegistryFamilies()
{
  int rv = 0;

  UnitsRegistry reg;

  // Nothing should be in the registry until we register default units
  rv += SDK_ASSERT(reg.families().empty());
  rv += SDK_ASSERT(reg.units(Units::INVALID_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::UNITLESS_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::ELAPSED_TIME_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::ANGLE_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::LENGTH_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::SPEED_FAMILY).empty());
  rv += SDK_ASSERT(reg.units(Units::FREQUENCY_FAMILY).empty());
  rv += SDK_ASSERT(reg.units("Not a real family").empty());
  rv += SDK_ASSERT(!reg.unitsByName("").isValid()); // Units::UNITLESS
  rv += SDK_ASSERT(!reg.unitsByName("meters").isValid());
  rv += SDK_ASSERT(!reg.unitsByName("seconds").isValid());
  rv += SDK_ASSERT(!reg.unitsByName("invalid").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("km").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("d").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("inv").isValid());

  // Register defaults, then validate each family exists
  reg.registerDefaultUnits();
  std::vector<std::string> fams = reg.families();
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::UNITLESS_FAMILY) != fams.end());
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::ELAPSED_TIME_FAMILY) != fams.end());
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::ANGLE_FAMILY) != fams.end());
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::LENGTH_FAMILY) != fams.end());
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::SPEED_FAMILY) != fams.end());
  // Make sure invalid values don't exist
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), Units::INVALID_FAMILY) == fams.end());
  rv += SDK_ASSERT(std::find(fams.begin(), fams.end(), "Does Not Exist") == fams.end());

  // Spot check several families for known entries

  // Elapsed time
  const UnitsRegistry::UnitsVector& unitlessVec = reg.units(Units::UNITLESS_FAMILY);
  rv += SDK_ASSERT(std::find(unitlessVec.begin(), unitlessVec.end(), Units::UNITLESS) != unitlessVec.end());

  // Elapsed time
  const UnitsRegistry::UnitsVector& timeVec = reg.units(Units::ELAPSED_TIME_FAMILY);
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::SECONDS) != timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::MILLISECONDS) != timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::MICROSECONDS) != timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::MINUTES) != timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::HOURS) != timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::DAYS) != timeVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::RADIANS) == timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::METERS) == timeVec.end());
  rv += SDK_ASSERT(std::find(timeVec.begin(), timeVec.end(), Units::METERS_PER_SECOND) == timeVec.end());

  const UnitsRegistry::UnitsVector& angleVec = reg.units(Units::ANGLE_FAMILY);
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::RADIANS) != angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::DEGREES) != angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::BAM) != angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::MIL) != angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::MILLIRADIANS) != angleVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::SECONDS) == angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::METERS) == angleVec.end());
  rv += SDK_ASSERT(std::find(angleVec.begin(), angleVec.end(), Units::METERS_PER_SECOND) == angleVec.end());

  const UnitsRegistry::UnitsVector& lengthVec = reg.units(Units::LENGTH_FAMILY);
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::METERS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::KILOMETERS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::YARDS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::FEET) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::INCHES) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::NAUTICAL_MILES) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::CENTIMETERS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::MILLIMETERS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::KILOYARDS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::DATA_MILES) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::FATHOMS) != lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::KILOFEET) != lengthVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::SECONDS) == lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::RADIANS) == lengthVec.end());
  rv += SDK_ASSERT(std::find(lengthVec.begin(), lengthVec.end(), Units::METERS_PER_SECOND) == lengthVec.end());

  const UnitsRegistry::UnitsVector& speedVec = reg.units(Units::SPEED_FAMILY);
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::METERS_PER_SECOND) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::KILOMETERS_PER_HOUR) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::KNOTS) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::MILES_PER_HOUR) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::FEET_PER_SECOND) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::KILOMETERS_PER_SECOND) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::DATA_MILES_PER_HOUR) != speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::YARDS_PER_SECOND) != speedVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::SECONDS) == speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::RADIANS) == speedVec.end());
  rv += SDK_ASSERT(std::find(speedVec.begin(), speedVec.end(), Units::METERS) == speedVec.end());

  const UnitsRegistry::UnitsVector& accelerationVec = reg.units(Units::ACCELERATION_FAMILY);
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::METERS_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::KILOMETERS_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::YARDS_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::MILES_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::FEET_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::INCHES_PER_SECOND_SQUARED) != accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::NAUTICAL_MILES_PER_SECOND_SQUARED) != accelerationVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::SECONDS) == accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::RADIANS) == accelerationVec.end());
  rv += SDK_ASSERT(std::find(accelerationVec.begin(), accelerationVec.end(), Units::METERS_PER_SECOND) == accelerationVec.end());

  const UnitsRegistry::UnitsVector& temperatureVec = reg.units(Units::TEMPERATURE_FAMILY);
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::CELSIUS) != temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::FAHRENHEIT) != temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::KELVIN) != temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::RANKINE) != temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::REAUMUR) != temperatureVec.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::SECONDS) == temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::RADIANS) == temperatureVec.end());
  rv += SDK_ASSERT(std::find(temperatureVec.begin(), temperatureVec.end(), Units::METERS_PER_SECOND) == temperatureVec.end());

  const UnitsRegistry::UnitsVector& frequencyVeq = reg.units(Units::FREQUENCY_FAMILY);
  rv += SDK_ASSERT(std::find(frequencyVeq.begin(), frequencyVeq.end(), Units::HERTZ) != frequencyVeq.end());
  rv += SDK_ASSERT(std::find(frequencyVeq.begin(), frequencyVeq.end(), Units::REVOLUTIONS_PER_MINUTE) != frequencyVeq.end());
  // Following few tests are expected to fail
  rv += SDK_ASSERT(std::find(frequencyVeq.begin(), frequencyVeq.end(), Units::SECONDS) == frequencyVeq.end());
  rv += SDK_ASSERT(std::find(frequencyVeq.begin(), frequencyVeq.end(), Units::RADIANS) == frequencyVeq.end());
  rv += SDK_ASSERT(std::find(frequencyVeq.begin(), frequencyVeq.end(), Units::METERS) == frequencyVeq.end());

  return rv;
}

int testRegistrySearchByName()
{
  int rv = 0;

  UnitsRegistry reg;
  reg.registerDefaultUnits();

  rv += SDK_ASSERT(reg.unitsByName("") == Units::UNITLESS);

  // Capitalization shouldn't matter for by-name
  rv += SDK_ASSERT(reg.unitsByName("Seconds") == Units::SECONDS);
  rv += SDK_ASSERT(reg.unitsByName("milliseconds") == Units::MILLISECONDS);
  rv += SDK_ASSERT(reg.unitsByName("microseconds") == Units::MICROSECONDS);
  rv += SDK_ASSERT(reg.unitsByName("mINUtes") == Units::MINUTES);
  rv += SDK_ASSERT(reg.unitsByName("HOURS") == Units::HOURS);
  rv += SDK_ASSERT(reg.unitsByName("days") == Units::DAYS);

  rv += SDK_ASSERT(reg.unitsByName("radians") == Units::RADIANS);
  rv += SDK_ASSERT(reg.unitsByName("degrees") == Units::DEGREES);
  rv += SDK_ASSERT(reg.unitsByName("binary angle measurement") == Units::BAM);
  rv += SDK_ASSERT(reg.unitsByName("angular mil") == Units::MIL);
  rv += SDK_ASSERT(reg.unitsByName("milliradians") == Units::MILLIRADIANS);

  rv += SDK_ASSERT(reg.unitsByName("meters") == Units::METERS);
  rv += SDK_ASSERT(reg.unitsByName("kilometers") == Units::KILOMETERS);
  rv += SDK_ASSERT(reg.unitsByName("yards") == Units::YARDS);
  rv += SDK_ASSERT(reg.unitsByName("miles") == Units::MILES);
  rv += SDK_ASSERT(reg.unitsByName("feet") == Units::FEET);
  rv += SDK_ASSERT(reg.unitsByName("inches") == Units::INCHES);
  rv += SDK_ASSERT(reg.unitsByName("nautical miles") == Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(reg.unitsByName("centimeters") == Units::CENTIMETERS);
  rv += SDK_ASSERT(reg.unitsByName("millimeters") == Units::MILLIMETERS);
  rv += SDK_ASSERT(reg.unitsByName("kiloyards") == Units::KILOYARDS);
  rv += SDK_ASSERT(reg.unitsByName("data miles") == Units::DATA_MILES);
  rv += SDK_ASSERT(reg.unitsByName("fathoms") == Units::FATHOMS);
  rv += SDK_ASSERT(reg.unitsByName("kilofeet") == Units::KILOFEET);

  rv += SDK_ASSERT(reg.unitsByName("meters per second") == Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByName("kilometers per hour") == Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByName("knots") == Units::KNOTS);
  rv += SDK_ASSERT(reg.unitsByName("miles per hour") == Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByName("feet per second") == Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByName("kilometers per second") == Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByName("data miles per hour") == Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByName("yards per second") == Units::YARDS_PER_SECOND);

  rv += SDK_ASSERT(reg.unitsByName("meters per second squared") == Units::METERS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("kilometers per second squared") == Units::KILOMETERS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("yards per second squared") == Units::YARDS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("miles per second squared") == Units::MILES_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("feet per second squared") == Units::FEET_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("inches per second squared") == Units::INCHES_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByName("knots per second") == Units::NAUTICAL_MILES_PER_SECOND_SQUARED);

  rv += SDK_ASSERT(reg.unitsByName("celsius") == Units::CELSIUS);
  rv += SDK_ASSERT(reg.unitsByName("fahrenheit") == Units::FAHRENHEIT);
  rv += SDK_ASSERT(reg.unitsByName("kelvin") == Units::KELVIN);
  rv += SDK_ASSERT(reg.unitsByName("rankine") == Units::RANKINE);
  rv += SDK_ASSERT(reg.unitsByName("reaumur") == Units::REAUMUR);

  rv += SDK_ASSERT(reg.unitsByName("revolutions per minute") == Units::REVOLUTIONS_PER_MINUTE);
  rv += SDK_ASSERT(reg.unitsByName("cycles per second") == Units::HERTZ);

  // Search for invalid units
  const Units& inv1 = reg.unitsByName("invalid");
  rv += SDK_ASSERT(!inv1.isValid());
  const Units& inv2 = reg.unitsByName("asdf");
  rv += SDK_ASSERT(!inv2.isValid());

  Units outUnits;
  rv += SDK_ASSERT(reg.unitsByName("invalid", outUnits) != 0);
  rv += SDK_ASSERT(!outUnits.isValid());
  rv += SDK_ASSERT(reg.unitsByName("meters per second", outUnits) == 0);
  rv += SDK_ASSERT(outUnits.isValid());
  rv += SDK_ASSERT(outUnits == Units::METERS_PER_SECOND);
  // Repeat test to ensure it overwrites valid values
  rv += SDK_ASSERT(reg.unitsByName("invalid", outUnits) != 0);
  rv += SDK_ASSERT(!outUnits.isValid());

  return rv;
}

int testRegistrySearchByAbbrev()
{
  int rv = 0;

  UnitsRegistry reg;
  reg.registerDefaultUnits();

  rv += SDK_ASSERT(reg.unitsByAbbreviation("") == Units::UNITLESS);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("sec") == Units::SECONDS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("ms") == Units::MILLISECONDS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("us") == Units::MICROSECONDS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("min") == Units::MINUTES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("hr") == Units::HOURS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("d") == Units::DAYS);

  // Capitalization matters for unit searches
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("uS").isValid());
  rv += SDK_ASSERT(reg.unitsByAbbreviation("us").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("MIN").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("Hr").isValid());
  rv += SDK_ASSERT(!reg.unitsByAbbreviation("D").isValid());

  rv += SDK_ASSERT(reg.unitsByAbbreviation("rad") == Units::RADIANS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("deg") == Units::DEGREES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("bam") == Units::BAM);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("mil") == Units::MIL);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("mrad") == Units::MILLIRADIANS);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("m") == Units::METERS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("km") == Units::KILOMETERS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("yd") == Units::YARDS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("mi") == Units::MILES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("ft") == Units::FEET);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("in") == Units::INCHES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("nm") == Units::NAUTICAL_MILES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("cm") == Units::CENTIMETERS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("mm") == Units::MILLIMETERS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("kyd") == Units::KILOYARDS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("dm") == Units::DATA_MILES);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("fm") == Units::FATHOMS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("kf") == Units::KILOFEET);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("m/sec") == Units::METERS_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("km/hr") == Units::KILOMETERS_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("kts") == Units::KNOTS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("mph") == Units::MILES_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("ft/sec") == Units::FEET_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("km/sec") == Units::KILOMETERS_PER_SECOND);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("dm/hr") == Units::DATA_MILES_PER_HOUR);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("yd/sec") == Units::YARDS_PER_SECOND);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("m/(s^2)") == Units::METERS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("km/(s^2)") == Units::KILOMETERS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("yd/(s^2)") == Units::YARDS_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("sm/(s^2)") == Units::MILES_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("ft/(s^2)") == Units::FEET_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("in/(s^2)") == Units::INCHES_PER_SECOND_SQUARED);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("nm/(s^2)") == Units::NAUTICAL_MILES_PER_SECOND_SQUARED);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("C") == Units::CELSIUS);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("F") == Units::FAHRENHEIT);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("k") == Units::KELVIN);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("ra") == Units::RANKINE);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("re") == Units::REAUMUR);

  rv += SDK_ASSERT(reg.unitsByAbbreviation("Hz") == Units::HERTZ);
  rv += SDK_ASSERT(reg.unitsByAbbreviation("rpm") == Units::REVOLUTIONS_PER_MINUTE);

  // Search for invalid units
  const Units& inv1 = reg.unitsByAbbreviation("inv");
  rv += SDK_ASSERT(!inv1.isValid());
  const Units& inv2 = reg.unitsByAbbreviation("asdf");
  rv += SDK_ASSERT(!inv2.isValid());

  Units outUnits;
  rv += SDK_ASSERT(reg.unitsByAbbreviation("inv", outUnits) != 0);
  rv += SDK_ASSERT(!outUnits.isValid());
  rv += SDK_ASSERT(reg.unitsByAbbreviation("m/sec", outUnits) == 0);
  rv += SDK_ASSERT(outUnits.isValid());
  rv += SDK_ASSERT(outUnits == Units::METERS_PER_SECOND);
  // Repeat test to ensure it overwrites valid values
  rv += SDK_ASSERT(reg.unitsByAbbreviation("inv", outUnits) != 0);
  rv += SDK_ASSERT(!outUnits.isValid());

  return rv;
}

int testUnitlessConvert()
{
  int rv = 0;

  // Convert various values
  rv += SDK_ASSERT(simCore::areEqual(Units::UNITLESS.convertTo(Units::UNITLESS, 36.0), 36.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::DAYS.convertTo(Units::UNITLESS, 2.5), 2.5));
  rv += SDK_ASSERT(simCore::areEqual(Units::UNITLESS.convertTo(Units::HOURS, 3.5), 3.5));
  double value;
  rv += SDK_ASSERT(Units::UNITLESS.convertTo(Units::METERS, 5.0, value) != 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 5.0));
  rv += SDK_ASSERT(Units::UNITLESS.convertTo(Units::UNITLESS, 7.0, value) == 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 7.0));

  // Confirm canConvert()
  rv += SDK_ASSERT(Units::UNITLESS.canConvert(Units::UNITLESS));
  rv += SDK_ASSERT(!Units::DAYS.canConvert(Units::UNITLESS));
  rv += SDK_ASSERT(!Units::UNITLESS.canConvert(Units::MILES));

  return rv;
}

int testTimeConvert()
{
  int rv = 0;

  // Convert various values
  rv += SDK_ASSERT(simCore::areEqual(Units::HOURS.convertTo(Units::DAYS, 36.0), 1.5));
  rv += SDK_ASSERT(simCore::areEqual(Units::DAYS.convertTo(Units::MINUTES, 2.5), 3600.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::DAYS.convertTo(Units::HOURS, 3.5), 84));
  rv += SDK_ASSERT(simCore::areEqual(Units::MINUTES.convertTo(Units::SECONDS, 2.5), 150.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::MILLISECONDS.convertTo(Units::MICROSECONDS, 3.5), 3500.0));

  // Try an invalid conversion
  rv += SDK_ASSERT(simCore::areEqual(Units::HOURS.convertTo(Units::METERS, 5.5), 5.5));

  // Try second signature
  double value = 3.0;
  rv += SDK_ASSERT(Units::DAYS.convertTo(Units::HOURS, 1.5, value) == 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 36.0));
  rv += SDK_ASSERT(Units::HOURS.convertTo(Units::METERS, 5.5, value) != 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 5.5));

  // Confirm canConvert()
  rv += SDK_ASSERT(Units::DAYS.canConvert(Units::MICROSECONDS));
  rv += SDK_ASSERT(!Units::DAYS.canConvert(Units::KNOTS));

  return rv;
}

int testAngleConvert()
{
  int rv = 0;

  // Convert various values
  rv += SDK_ASSERT(simCore::areEqual(Units::RADIANS.convertTo(Units::DEGREES, 1.5), 85.9436692));
  rv += SDK_ASSERT(simCore::areEqual(Units::DEGREES.convertTo(Units::RADIANS, 90), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(Units::BAM.convertTo(Units::MIL, 0.5), 324.227788));
  rv += SDK_ASSERT(simCore::areEqual(Units::MIL.convertTo(Units::DEGREES, 3.0), 0.16875));
  rv += SDK_ASSERT(simCore::areEqual(Units::RADIANS.convertTo(Units::MILLIRADIANS, 3.5), 3500.0));

  // Try an invalid conversion
  rv += SDK_ASSERT(simCore::areEqual(Units::MIL.convertTo(Units::METERS, 5.5), 5.5));

  // Try second signature
  double value = 3.0;
  rv += SDK_ASSERT(Units::DEGREES.convertTo(Units::RADIANS, 14, value) == 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 0.2443461));
  rv += SDK_ASSERT(Units::MILLIRADIANS.convertTo(Units::METERS, 5.5, value) != 0);
  rv += SDK_ASSERT(simCore::areEqual(value, 5.5));

  // Confirm canConvert()
  rv += SDK_ASSERT(Units::DEGREES.canConvert(Units::MILLIRADIANS));
  rv += SDK_ASSERT(!Units::DEGREES.canConvert(Units::KNOTS));

  return rv;
}

int testLengthConvert()
{
  int rv = 0;

  // At this point, presume a-to-b and b-to-a is fully tested; just test one side
  rv += SDK_ASSERT(simCore::areEqual(Units::METERS.convertTo(Units::FEET, 1.5), 4.92126));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOMETERS.convertTo(Units::METERS, 1.5), 1500.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::YARDS.convertTo(Units::METERS, 1.5), 1.3716));
  rv += SDK_ASSERT(simCore::areEqual(Units::MILES.convertTo(Units::METERS, 1.5), 2414.016));
  rv += SDK_ASSERT(simCore::areEqual(Units::FEET.convertTo(Units::METERS, 1.5), 0.457199984));
  rv += SDK_ASSERT(simCore::areEqual(Units::INCHES.convertTo(Units::METERS, 1.5), 0.0381));
  rv += SDK_ASSERT(simCore::areEqual(Units::NAUTICAL_MILES.convertTo(Units::METERS, 1.5), 2778.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::CENTIMETERS.convertTo(Units::METERS, 1.5), 0.015));
  rv += SDK_ASSERT(simCore::areEqual(Units::MILLIMETERS.convertTo(Units::METERS, 1.5), 0.0015));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOYARDS.convertTo(Units::METERS, 1.5), 1371.599998));
  rv += SDK_ASSERT(simCore::areEqual(Units::DATA_MILES.convertTo(Units::METERS, 1.5), 2743.2002466));
  rv += SDK_ASSERT(simCore::areEqual(Units::FATHOMS.convertTo(Units::METERS, 1.5), 2.74319991));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOFEET.convertTo(Units::METERS, 1.5), 457.199984));

  return rv;
}

int testSpeedConvert()
{
  int rv = 0;

  // At this point, presume a-to-b and b-to-a is fully tested; just test one side
  rv += SDK_ASSERT(simCore::areEqual(Units::METERS_PER_SECOND.convertTo(Units::MILES_PER_HOUR, 1.5), 3.35540466));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOMETERS_PER_HOUR.convertTo(Units::METERS_PER_SECOND, 1.5), 0.416667));
  rv += SDK_ASSERT(simCore::areEqual(Units::KNOTS.convertTo(Units::METERS_PER_SECOND, 1.5), 0.771667));
  rv += SDK_ASSERT(simCore::areEqual(Units::MILES_PER_HOUR.convertTo(Units::METERS_PER_SECOND, 1.5), 0.67056));
  rv += SDK_ASSERT(simCore::areEqual(Units::FEET_PER_SECOND.convertTo(Units::METERS_PER_SECOND, 1.5), 0.4572));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOMETERS_PER_SECOND.convertTo(Units::METERS_PER_SECOND, 1.5), 1500.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::DATA_MILES_PER_HOUR.convertTo(Units::METERS_PER_SECOND, 1.5), 0.76196607));
  rv += SDK_ASSERT(simCore::areEqual(Units::YARDS_PER_SECOND.convertTo(Units::METERS_PER_SECOND, 1.5), 1.3716));

  return rv;
}

int testAccelerationConvert()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::areEqual(Units::METERS_PER_SECOND_SQUARED.convertTo(Units::FEET_PER_SECOND_SQUARED, 1.5), 4.92125984));
  rv += SDK_ASSERT(simCore::areEqual(Units::KILOMETERS_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 1500.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::YARDS_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 1.3716));
  rv += SDK_ASSERT(simCore::areEqual(Units::MILES_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 2414.016));
  rv += SDK_ASSERT(simCore::areEqual(Units::FEET_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 0.4572));
  rv += SDK_ASSERT(simCore::areEqual(Units::INCHES_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 0.0381));
  rv += SDK_ASSERT(simCore::areEqual(Units::NAUTICAL_MILES_PER_SECOND_SQUARED.convertTo(Units::METERS_PER_SECOND_SQUARED, 1.5), 2778.0));

  return rv;
}

int testTemperatureConvert()
{
  int rv = 0;

  // Convert both ways for temperature, since it's the first unit introduced that uses offsets
  rv += SDK_ASSERT(simCore::areEqual(Units::CELSIUS.convertTo(Units::FAHRENHEIT, 1.5), 34.7));
  rv += SDK_ASSERT(simCore::areEqual(Units::FAHRENHEIT.convertTo(Units::CELSIUS, 1.5), -16.944444));
  rv += SDK_ASSERT(simCore::areEqual(Units::CELSIUS.convertTo(Units::KELVIN, 1.5), 274.65));
  rv += SDK_ASSERT(simCore::areEqual(Units::KELVIN.convertTo(Units::CELSIUS, 1.5), -271.65));
  rv += SDK_ASSERT(simCore::areEqual(Units::CELSIUS.convertTo(Units::RANKINE, 1.5), 494.37));
  rv += SDK_ASSERT(simCore::areEqual(Units::RANKINE.convertTo(Units::CELSIUS, 1.5), -272.316667));
  rv += SDK_ASSERT(simCore::areEqual(Units::CELSIUS.convertTo(Units::REAUMUR, 1.5), 1.2));
  rv += SDK_ASSERT(simCore::areEqual(Units::REAUMUR.convertTo(Units::CELSIUS, 1.5), 1.875));

  return rv;
}

int testFrequencyConvert()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::areEqual(Units::REVOLUTIONS_PER_MINUTE.convertTo(Units::HERTZ, 2.5), 150.0));
  rv += SDK_ASSERT(simCore::areEqual(Units::HERTZ.convertTo(Units::REVOLUTIONS_PER_MINUTE, 600), 10.0));

  return rv;
}

int testCustomUnitsToExistingFamily()
{
  int rv = 0;

  UnitsRegistry reg;
  reg.registerDefaultUnits();

  const UnitsRegistry::UnitsVector lengthVecCopy1 = reg.units(Units::LENGTH_FAMILY);
  const UnitsRegistry::UnitsVector& lengthVecRef = reg.units(Units::LENGTH_FAMILY);
  rv += SDK_ASSERT(lengthVecCopy1 == lengthVecRef);
  rv += SDK_ASSERT(!lengthVecCopy1.empty());

  // From https://en.wikipedia.org/wiki/List_of_unusual_units_of_measurement
  const Units RACK_UNITS("rack units", "rck", 0.04445, Units::LENGTH_FAMILY);
  // Add units inside a scope to ensure they persist outside of scope
  {
    // Hands are equal to 4 inches; Intentionally do not create a constant for testing scoping
    rv += SDK_ASSERT(reg.registerUnits(Units("hands", "hnd", 0.1016, Units::LENGTH_FAMILY)) == 0);
    // Rack unit is 1.75 inches
    rv += SDK_ASSERT(reg.registerUnits(RACK_UNITS) == 0);
  }

  // Now test that we can access the units
  const Units& handByName = reg.unitsByName("hands");
  const Units& handByAbbrev = reg.unitsByAbbreviation("hnd");
  const Units& rackByName = reg.unitsByName("rack units");
  const Units& rackByAbbrev = reg.unitsByAbbreviation("rck");

  rv += SDK_ASSERT(handByName.isValid());
  rv += SDK_ASSERT(handByAbbrev.isValid());
  rv += SDK_ASSERT(rackByName.isValid());
  rv += SDK_ASSERT(rackByAbbrev.isValid());

  rv += SDK_ASSERT(handByAbbrev.family() == Units::LENGTH_FAMILY);
  rv += SDK_ASSERT(rackByName.family() == Units::LENGTH_FAMILY);

  // Make sure the show up in the family list
  const UnitsRegistry::UnitsVector lengthVecCopy2 = reg.units(Units::LENGTH_FAMILY);
  rv += SDK_ASSERT(lengthVecCopy1.size() != lengthVecCopy2.size());
  // The reference should have update though
  rv += SDK_ASSERT(lengthVecRef == lengthVecCopy2);

  // Validate some conversions
  rv += SDK_ASSERT(simCore::areEqual(handByName.convertTo(Units::INCHES, 1.0), 4.0));
  rv += SDK_ASSERT(simCore::areEqual(rackByAbbrev.convertTo(Units::INCHES, 1.0), 1.75));

  return rv;
}

static const std::string UNITS_OF_INFO_STR("units of information");

int testCustomFamily()
{
  int rv = 0;

  UnitsRegistry reg;
  reg.registerDefaultUnits();
  const size_t numFamiliesBefore = reg.families().size();

  // Create the family and its members in a scope so the values can be tested out of scope
  {
    rv += SDK_ASSERT(reg.units(UNITS_OF_INFO_STR).empty());
    const Units bytes("bytes", "B", 1.0, UNITS_OF_INFO_STR);
    const Units bits("bits", "b", 0.125, UNITS_OF_INFO_STR);
    const Units kilobytes("kilobytes", "kB", 1024.0, UNITS_OF_INFO_STR);
    const Units decKilobytes("decimal kilobytes", "KB", 1000.0, UNITS_OF_INFO_STR);
    const Units kilobits("kilobits", "kb", 125.0, UNITS_OF_INFO_STR);
    reg.registerUnits(bits);
    reg.registerUnits(bytes);
    reg.registerUnits(kilobytes);
    reg.registerUnits(decKilobytes);
    reg.registerUnits(kilobits);
    rv += SDK_ASSERT(!reg.units(UNITS_OF_INFO_STR).empty());
  }

  // Number of families should have changed
  rv += SDK_ASSERT((numFamiliesBefore + 1) == reg.families().size());

  // Validate that we can find all the entries
  const Units bytesByName = reg.unitsByName("bytes");
  const Units bitsByName = reg.unitsByName("bits");
  const Units kilobytesByName = reg.unitsByName("kilobytes");
  const Units dKilobytesByName = reg.unitsByName("decimal kilobytes");
  const Units kilobitsByName = reg.unitsByName("kilobits");
  rv += SDK_ASSERT(bytesByName.isValid());
  rv += SDK_ASSERT(bitsByName.isValid());
  rv += SDK_ASSERT(kilobytesByName.isValid());
  rv += SDK_ASSERT(dKilobytesByName.isValid());
  rv += SDK_ASSERT(kilobitsByName.isValid());

  // By abbreviation too
  const Units bytesByAbbrev = reg.unitsByAbbreviation("B");
  const Units bitsByAbbrev = reg.unitsByAbbreviation("b");
  const Units kilobytesByAbbrev = reg.unitsByAbbreviation("kB");
  const Units dKilobytesByAbbrev = reg.unitsByAbbreviation("KB");
  const Units kilobitsByAbbrev = reg.unitsByAbbreviation("kb");
  rv += SDK_ASSERT(bytesByName.isValid());
  rv += SDK_ASSERT(bitsByName.isValid());
  rv += SDK_ASSERT(kilobytesByName.isValid());
  rv += SDK_ASSERT(dKilobytesByAbbrev.isValid());
  rv += SDK_ASSERT(kilobitsByAbbrev.isValid());

  // Test the abbreviations for capitalization issues
  rv += SDK_ASSERT(bytesByAbbrev != bitsByAbbrev);
  rv += SDK_ASSERT(bytesByAbbrev.name() == "bytes");
  rv += SDK_ASSERT(bitsByAbbrev.name() == "bits");
  rv += SDK_ASSERT(kilobytesByAbbrev != dKilobytesByAbbrev);
  rv += SDK_ASSERT(kilobytesByAbbrev != kilobitsByAbbrev);
  rv += SDK_ASSERT(dKilobytesByAbbrev != kilobitsByAbbrev);
  rv += SDK_ASSERT(kilobytesByAbbrev.name() == "kilobytes");
  rv += SDK_ASSERT(dKilobytesByAbbrev.name() == "decimal kilobytes");
  rv += SDK_ASSERT(kilobitsByAbbrev.name() == "kilobits");

  // Now test conversions
  rv += SDK_ASSERT(simCore::areEqual(bytesByName.convertTo(kilobytesByAbbrev, 2048.0), 2.0));
  rv += SDK_ASSERT(simCore::areEqual(bitsByAbbrev.convertTo(kilobytesByName, 28672.0), 3.5));
  rv += SDK_ASSERT(simCore::areEqual(kilobytesByName.convertTo(bytesByName, 0.5), 512));
  rv += SDK_ASSERT(simCore::areEqual(kilobytesByName.convertTo(kilobitsByAbbrev, 0.25), 2.048));
  rv += SDK_ASSERT(simCore::areEqual(kilobitsByAbbrev.convertTo(dKilobytesByAbbrev, 100), 12.5));

  return rv;
}

}

int UnitsTest(int argc, char* argv[])
{
  int rv = 0;

  rv += testRegistryFamilies();
  rv += testRegistrySearchByName();
  rv += testRegistrySearchByAbbrev();
  rv += testUnitlessConvert();
  rv += testTimeConvert();
  rv += testAngleConvert();
  rv += testLengthConvert();
  rv += testSpeedConvert();
  rv += testAccelerationConvert();
  rv += testTemperatureConvert();
  rv += testFrequencyConvert();
  rv += testCustomUnitsToExistingFamily();
  rv += testCustomFamily();

  return rv;
}
