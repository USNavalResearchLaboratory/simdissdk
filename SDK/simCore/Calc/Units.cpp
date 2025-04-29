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
#include <memory>
#include <algorithm>
#include "simCore/String/Format.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Time/Constants.h"
#include "simCore/Calc/Units.h"

namespace simCore {

const std::string Units::INVALID_FAMILY = simCore::INVALID_FAMILY;
const std::string Units::UNITLESS_FAMILY = simCore::UNITLESS_FAMILY;
const std::string Units::ELAPSED_TIME_FAMILY = simCore::ELAPSED_TIME_FAMILY;
const std::string Units::ANGLE_FAMILY = simCore::ANGLE_FAMILY;
const std::string Units::LENGTH_FAMILY = simCore::LENGTH_FAMILY;
const std::string Units::SPEED_FAMILY = simCore::SPEED_FAMILY;
const std::string Units::ACCELERATION_FAMILY = simCore::ACCELERATION_FAMILY;
const std::string Units::TEMPERATURE_FAMILY = simCore::TEMPERATURE_FAMILY;
const std::string Units::FREQUENCY_FAMILY = simCore::FREQUENCY_FAMILY;
const std::string Units::VOLUME_FAMILY = simCore::VOLUME_FAMILY;
const std::string Units::PRESSURE_FAMILY = simCore::PRESSURE_FAMILY;
const std::string Units::POTENTIAL_FAMILY = simCore::POTENTIAL_FAMILY;

const Units Units::UNITLESS("", "", 1.0, simCore::UNITLESS_FAMILY);

const Units Units::SECONDS("seconds", "s", 1.0, simCore::ELAPSED_TIME_FAMILY);
const Units Units::MILLISECONDS("milliseconds", "ms", 0.001, simCore::ELAPSED_TIME_FAMILY);
const Units Units::MICROSECONDS("microseconds", "us", 1e-6, simCore::ELAPSED_TIME_FAMILY);
const Units Units::NANOSECONDS("nanoseconds", "ns", 1e-9, simCore::ELAPSED_TIME_FAMILY);
const Units Units::MINUTES("minutes", "min", SECPERMIN, simCore::ELAPSED_TIME_FAMILY);
const Units Units::HOURS("hours", "hr", SECPERHOUR, simCore::ELAPSED_TIME_FAMILY);
const Units Units::DAYS("days", "d", SECPERDAY, simCore::ELAPSED_TIME_FAMILY);

const Units Units::RADIANS("radians", "rad", 1.0, simCore::ANGLE_FAMILY);
const Units Units::DEGREES("degrees", "deg", simCore::DEG2RAD, simCore::ANGLE_FAMILY);
const Units Units::MILLIRADIANS("milliradians", "mrad", 1e-3, simCore::ANGLE_FAMILY);
const Units Units::BAM("binary angle measurement", "bam", M_TWOPI, simCore::ANGLE_FAMILY);
// Based on NATO definition of angular mils (6400 mils in a circle)
const Units Units::MIL("angular mil", "mil", 9.8174770424681038701957605727484e-4, simCore::ANGLE_FAMILY);

const Units Units::METERS("meters", "m", 1.0, simCore::LENGTH_FAMILY);
const Units Units::KILOMETERS("kilometers", "km", 1e3, simCore::LENGTH_FAMILY);
const Units Units::YARDS("yards", "yd", 0.9144, simCore::LENGTH_FAMILY);
const Units Units::MILES("miles", "mi", 1609.344, simCore::LENGTH_FAMILY);
const Units Units::FEET("feet", "ft", 0.3048, simCore::LENGTH_FAMILY);
const Units Units::INCHES("inches", "in", 0.0254, simCore::LENGTH_FAMILY);
const Units Units::NAUTICAL_MILES("nautical miles", "nm", 1852.0, simCore::LENGTH_FAMILY);
const Units Units::CENTIMETERS("centimeters", "cm", 1e-2, simCore::LENGTH_FAMILY);
const Units Units::MILLIMETERS("millimeters", "mm", 1e-3, simCore::LENGTH_FAMILY);
const Units Units::KILOYARDS("kiloyards", "kyd", 914.4, simCore::LENGTH_FAMILY);
const Units Units::FATHOMS("fathoms", "fm", 1.8288, simCore::LENGTH_FAMILY);
const Units Units::KILOFEET("kilofeet", "kf", 304.8, simCore::LENGTH_FAMILY);
// Distance used in radar related subjects, equal to 6000 feet
const Units Units::DATA_MILES("data miles", "dm", 1828.8, simCore::LENGTH_FAMILY);

const Units Units::METERS_PER_SECOND("meters per second", "m/sec", 1.0, simCore::SPEED_FAMILY);
const Units Units::KILOMETERS_PER_HOUR("kilometers per hour", "km/hr", Units::KILOMETERS.convertTo(Units::METERS, 1.0) / SECPERHOUR, simCore::SPEED_FAMILY);
const Units Units::KNOTS("knots", "kts", Units::NAUTICAL_MILES.convertTo(Units::METERS, 1.0) / SECPERHOUR, simCore::SPEED_FAMILY);
const Units Units::MILES_PER_HOUR("miles per hour", "mph", Units::MILES.convertTo(Units::METERS, 1.0) / SECPERHOUR, simCore::SPEED_FAMILY);
const Units Units::FEET_PER_SECOND("feet per second", "ft/sec", Units::FEET.convertTo(Units::METERS, 1.0), simCore::SPEED_FAMILY);
const Units Units::KILOMETERS_PER_SECOND("kilometers per second", "km/sec", Units::KILOMETERS.convertTo(Units::METERS, 1.0), simCore::SPEED_FAMILY);
const Units Units::DATA_MILES_PER_HOUR("data miles per hour", "dm/hr", Units::DATA_MILES.convertTo(Units::METERS, 1.0) / SECPERHOUR, simCore::SPEED_FAMILY);
const Units Units::YARDS_PER_SECOND("yards per second", "yd/sec", Units::YARDS.convertTo(Units::METERS, 1.0), simCore::SPEED_FAMILY);

const Units Units::METERS_PER_SECOND_SQUARED("meters per second squared", "m/(s^2)", 1.0, simCore::ACCELERATION_FAMILY);
const Units Units::KILOMETERS_PER_SECOND_SQUARED("kilometers per second squared", "km/(s^2)", 1e3, simCore::ACCELERATION_FAMILY);
const Units Units::YARDS_PER_SECOND_SQUARED("yards per second squared", "yd/(s^2)", 0.9144, simCore::ACCELERATION_FAMILY);
const Units Units::MILES_PER_SECOND_SQUARED("miles per second squared", "sm/(s^2)", 1609.344, simCore::ACCELERATION_FAMILY);
const Units Units::FEET_PER_SECOND_SQUARED("feet per second squared", "ft/(s^2)", 0.3048, simCore::ACCELERATION_FAMILY);
const Units Units::INCHES_PER_SECOND_SQUARED("inches per second squared", "in/(s^2)", 0.0254, simCore::ACCELERATION_FAMILY);
const Units Units::NAUTICAL_MILES_PER_SECOND_SQUARED("knots per second", "nm/(s^2)", 1852.0, simCore::ACCELERATION_FAMILY);

const Units Units::CELSIUS("celsius", "C", 1.0, simCore::TEMPERATURE_FAMILY);
const Units Units::FAHRENHEIT(Units::offsetThenScaleUnit("fahrenheit", "F", -32.0, 5./9., simCore::TEMPERATURE_FAMILY));
const Units Units::KELVIN(Units::offsetThenScaleUnit("kelvin", "k", -273.15, 1.0, simCore::TEMPERATURE_FAMILY));
const Units Units::RANKINE(Units::offsetThenScaleUnit("rankine", "ra", -491.67, 5./9., simCore::TEMPERATURE_FAMILY));
const Units Units::REAUMUR("reaumur", "re", 1.25, simCore::TEMPERATURE_FAMILY);

// Frequency units
const Units Units::HERTZ("cycles per second", "Hz", 1.0, simCore::FREQUENCY_FAMILY);
const Units Units::KILOHERTZ("1000 cycles per second", "kHz", 1000.0, simCore::FREQUENCY_FAMILY);
const Units Units::MEGAHERTZ("1000000 cycles per second", "MHz", 1000000.0, simCore::FREQUENCY_FAMILY);
const Units Units::REVOLUTIONS_PER_MINUTE("revolutions per minute", "rpm", 0.01666666666, simCore::FREQUENCY_FAMILY);
const Units Units::RADIANS_PER_SECOND("radians per second", "rad/sec", 0.15915494309, simCore::FREQUENCY_FAMILY);
const Units Units::DEGREES_PER_SECOND("degrees per second", "deg/sec", 0.00277777777, simCore::FREQUENCY_FAMILY);

// Volume units
const Units Units::LITER("liters", "l", 1.0, simCore::VOLUME_FAMILY);
const Units Units::MILLILITER("milliliters", "ml", 0.001, simCore::VOLUME_FAMILY);
const Units Units::FLUID_OUNCE("fluid ounces", "fl oz", 0.0295703125, simCore::VOLUME_FAMILY);
const Units Units::CUP("cups", "cup", 0.2365625, simCore::VOLUME_FAMILY);
const Units Units::PINT("pints", "pt", 0.473125, simCore::VOLUME_FAMILY);
const Units Units::QUART("quarts", "qt", 0.94625, simCore::VOLUME_FAMILY);
const Units Units::GALLON("gallons", "gal", 3.785, simCore::VOLUME_FAMILY);
const Units Units::TEASPOON("teaspoons", "tsp", 0.00492838542, simCore::VOLUME_FAMILY);
const Units Units::TABLESPOON("tablespoons", "tbsp", 0.01478515625, simCore::VOLUME_FAMILY);

// Pressure units
const Units Units::MILLIBAR("millibar", "mbar", 1.0, simCore::PRESSURE_FAMILY);  // Equivalent to a hectopascal
const Units Units::BAR("bars", "bar", 1000, simCore::PRESSURE_FAMILY);
const Units Units::POUNDS_PER_SQUARE_INCH("pounds per square inch", "psia", 68.94757, simCore::PRESSURE_FAMILY);
const Units Units::ATMOSPHERE("atmospheres", "atm", 1013.247139776643, simCore::PRESSURE_FAMILY);
const Units Units::TORR("torr", "torr", 1.33321992075874, simCore::PRESSURE_FAMILY);
const Units Units::PASCALS("pascals", "Pa", 0.01, simCore::PRESSURE_FAMILY);
const Units Units::KILOPASCAL("kilopascals", "kPa", 10, simCore::PRESSURE_FAMILY);
const Units Units::MEGAPASCAL("megapascals", "MPa", 1e4, simCore::PRESSURE_FAMILY);

// Potential units
const Units Units::VOLT("volts", "V", 1.0, simCore::POTENTIAL_FAMILY);
const Units Units::MILLIVOLT("millivolts", "mV", .001, simCore::POTENTIAL_FAMILY);
const Units Units::MICROVOLT("microvolts", "uV", 1e-6, simCore::POTENTIAL_FAMILY);
const Units Units::KILOVOLT("kilovolts", "kV", 1000, simCore::POTENTIAL_FAMILY);
const Units Units::MEGAVOLT("megavolts", "MV", 1e6, simCore::POTENTIAL_FAMILY);
const Units Units::GIGAVOLT("gigavolts", "GV", 1e9, simCore::POTENTIAL_FAMILY);

///////////////////////////////////////////////////////

Units::Units(const std::string& name, const std::string& abbrev, double toBase, const std::string& family)
  : name_(name),
    abbrev_(abbrev),
    toBaseOffset_(0.0),
    toBase_(toBase),
    family_(family)
{
  // Assertion failure means we need to add divide-by-zero protection, and
  // arguably is a developer error.
  assert(toBase_ != 0.0);
  // Avoid developer errors; do not reference Units::INVALID_FAMILY since order of global variables is not guarantee
  assert(family_ != "invalid");
}

Units::Units()
  : name_("Invalid"),
    abbrev_("inv"),
    toBaseOffset_(0.0),
    toBase_(1.0),
    family_(simCore::INVALID_FAMILY)
{
}

Units Units::offsetThenScaleUnit(const std::string& name, const std::string& abbrev, double offset, double toBase, const std::string& family)
{
  Units rv(name, abbrev, toBase, family);
  rv.toBaseOffset_ = offset;
  return rv;
}

Units::~Units()
{
}

bool Units::isValid() const
{
  return family_ != simCore::INVALID_FAMILY;
}

const std::string& Units::name() const
{
  return name_;
}

const std::string& Units::abbreviation() const
{
  return abbrev_;
}

const std::string& Units::family() const
{
  return family_;
}

double Units::toBaseScalar() const
{
  return toBase_;
}

double Units::toBaseOffset() const
{
  return toBaseOffset_;
}

bool Units::canConvert(const Units& toUnits) const
{
  // A to-base of 0.0 would cause a divide-by-zero later, and doesn't make sense
  assert(toUnits.toBase_ != 0.0);
  return family() == toUnits.family() && (toUnits.toBase_ != 0.0);
}

int Units::convertTo(const Units& toUnits, double value, double& output) const
{
  if (!canConvert(toUnits))
  {
    output = value;
    return 1;
  }

  // Convert the value to the base units, then to our unit format
  const double inBaseUnits = (value + toBaseOffset_) * toBase_;
  output = (inBaseUnits / toUnits.toBase_) - toUnits.toBaseOffset_;
  return 0;
}

double Units::convertTo(const Units& toUnits, double value) const
{
  if (!canConvert(toUnits))
    return value;
  const double inBaseUnits = (value + toBaseOffset_) * toBase_;
  return (inBaseUnits / toUnits.toBase_) - toUnits.toBaseOffset_;
}

bool Units::operator==(const Units& other) const
{
  // Ignore the name and abbreviation
  return toBase_ == other.toBase_ &&
    toBaseOffset_ == other.toBaseOffset_ &&
    family_ == other.family_;
}

bool Units::operator!=(const Units& other) const
{
  return !operator==(other);
}

///////////////////////////////////////////////////////

UnitsRegistry::UnitsRegistry()
{
}

UnitsRegistry::~UnitsRegistry()
{
}

void UnitsRegistry::registerDefaultUnits()
{
  registerUnits(Units::UNITLESS);

  registerUnits(Units::SECONDS);
  registerUnits(Units::MILLISECONDS);
  registerUnits(Units::MICROSECONDS);
  registerUnits(Units::MINUTES);
  registerUnits(Units::HOURS);
  registerUnits(Units::DAYS);

  registerUnits(Units::RADIANS);
  registerUnits(Units::DEGREES);
  registerUnits(Units::MILLIRADIANS);
  registerUnits(Units::BAM);
  registerUnits(Units::MIL);

  registerUnits(Units::METERS);
  registerUnits(Units::KILOMETERS);
  registerUnits(Units::YARDS);
  registerUnits(Units::MILES);
  registerUnits(Units::FEET);
  registerUnits(Units::INCHES);
  registerUnits(Units::NAUTICAL_MILES);
  registerUnits(Units::CENTIMETERS);
  registerUnits(Units::MILLIMETERS);
  registerUnits(Units::KILOYARDS);
  registerUnits(Units::FATHOMS);
  registerUnits(Units::KILOFEET);
  registerUnits(Units::DATA_MILES);

  registerUnits(Units::METERS_PER_SECOND);
  registerUnits(Units::KILOMETERS_PER_HOUR);
  registerUnits(Units::KNOTS);
  registerUnits(Units::MILES_PER_HOUR);
  registerUnits(Units::FEET_PER_SECOND);
  registerUnits(Units::KILOMETERS_PER_SECOND);
  registerUnits(Units::DATA_MILES_PER_HOUR);
  registerUnits(Units::YARDS_PER_SECOND);

  registerUnits(Units::METERS_PER_SECOND_SQUARED);
  registerUnits(Units::KILOMETERS_PER_SECOND_SQUARED);
  registerUnits(Units::YARDS_PER_SECOND_SQUARED);
  registerUnits(Units::MILES_PER_SECOND_SQUARED);
  registerUnits(Units::FEET_PER_SECOND_SQUARED);
  registerUnits(Units::INCHES_PER_SECOND_SQUARED);
  registerUnits(Units::NAUTICAL_MILES_PER_SECOND_SQUARED);

  registerUnits(Units::CELSIUS);
  registerUnits(Units::FAHRENHEIT);
  registerUnits(Units::KELVIN);
  registerUnits(Units::RANKINE);
  registerUnits(Units::REAUMUR);

  registerUnits(Units::HERTZ);
  registerUnits(Units::REVOLUTIONS_PER_MINUTE);
  registerUnits(Units::RADIANS_PER_SECOND);
  registerUnits(Units::DEGREES_PER_SECOND);

  registerUnits(Units::LITER);
  registerUnits(Units::MILLILITER);
  registerUnits(Units::FLUID_OUNCE);
  registerUnits(Units::CUP);
  registerUnits(Units::PINT);
  registerUnits(Units::QUART);
  registerUnits(Units::GALLON);
  registerUnits(Units::TEASPOON);
  registerUnits(Units::TABLESPOON);

  registerUnits(Units::MILLIBAR);
  registerUnits(Units::BAR);
  registerUnits(Units::POUNDS_PER_SQUARE_INCH);
  registerUnits(Units::ATMOSPHERE);
  registerUnits(Units::TORR);
  registerUnits(Units::PASCALS);
  registerUnits(Units::KILOPASCAL);
  registerUnits(Units::MEGAPASCAL);

  registerUnits(Units::VOLT);
  registerUnits(Units::MILLIVOLT);
  registerUnits(Units::MICROVOLT);
  registerUnits(Units::KILOVOLT);
  registerUnits(Units::MEGAVOLT);
  registerUnits(Units::GIGAVOLT);
}

int UnitsRegistry::registerUnits(const Units& units)
{
  // Avoid adding same unit more than once
  std::map<std::string, UnitsVector>::iterator fi = units_.find(units.family());
  if (fi == units_.end())
  {
    // Add the item to a new vector
    UnitsVector v;
    v.push_back(units);
    units_[units.family()] = v;
  }
  else
  {
    // Vector found for family (common case)
    UnitsVector& v = fi->second;
    // Does the unit already exist in vector? if so fail
    if (std::find(v.begin(), v.end(), units) != v.end())
    {
      // Developer failure -- adding same unit more than once.  Aliases not currently permitted
      assert(0);
      return 1;
    }
    v.push_back(units);
  }

  // Avoid adding same units more than once, or reusing names/abbreviations
  const std::string name = simCore::lowerCase(units.name());
  if (unitsByName_.find(name) == unitsByName_.end())
    unitsByName_[name] = units;
  else
    assert(0); // Developer error

  // Avoid reusing abbreviations
  if (unitsByAbbrev_.find(units.abbreviation()) == unitsByAbbrev_.end())
    unitsByAbbrev_[units.abbreviation()] = units;
  else
    assert(0); // Developer error

  return 0;
}

const UnitsRegistry::UnitsVector& UnitsRegistry::units(const std::string& family) const
{
  std::map<std::string, UnitsVector>::const_iterator i = units_.find(family);
  if (i == units_.end())
    return emptyUnitsVector_;
  return i->second;
}

std::vector<std::string> UnitsRegistry::families() const
{
  std::vector<std::string> rv;
  for (std::map<std::string, UnitsVector>::const_iterator i = units_.begin(); i != units_.end(); ++i)
  {
    rv.push_back(i->first);
  }
  return rv;
}

const Units& UnitsRegistry::unitsByName(const std::string& name) const
{
  // Search-by-name is case insensitive
  std::map<std::string, Units>::const_iterator i = unitsByName_.find(simCore::lowerCase(name));
  if (i == unitsByName_.end())
    return invalidUnits_;
  return i->second;
}

int UnitsRegistry::unitsByName(const std::string& name, Units& outUnits) const
{
  // Search-by-name is case insensitive
  std::map<std::string, Units>::const_iterator i = unitsByName_.find(simCore::lowerCase(name));
  if (i == unitsByName_.end())
  {
    outUnits = invalidUnits_;
    return 1;
  }
  outUnits = i->second;
  return 0;
}

const Units& UnitsRegistry::unitsByAbbreviation(const std::string& abbrev) const
{
  // Search-by-abbreviation is case sensitive
  std::map<std::string, Units>::const_iterator i = unitsByAbbrev_.find(abbrev);
  if (i == unitsByAbbrev_.end())
    return invalidUnits_;
  return i->second;
}

int UnitsRegistry::unitsByAbbreviation(const std::string& abbrev, Units& outUnits) const
{
  // Search-by-abbreviation is case sensitive
  std::map<std::string, Units>::const_iterator i = unitsByAbbrev_.find(abbrev);
  if (i == unitsByAbbrev_.end())
  {
    outUnits = invalidUnits_;
    return 1;
  }
  outUnits = i->second;
  return 0;
}

}
