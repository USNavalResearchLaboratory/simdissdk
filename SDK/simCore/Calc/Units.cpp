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
#include <memory>
#include <algorithm>
#include "simCore/String/Format.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Units.h"

namespace simCore {

const std::string Units::INVALID_FAMILY("invalid");
const std::string Units::UNITLESS_FAMILY("unitless");
const std::string Units::ELAPSED_TIME_FAMILY("elapsed time");
const std::string Units::ANGLE_FAMILY("angle");
const std::string Units::LENGTH_FAMILY("length");
const std::string Units::SPEED_FAMILY("speed");
const std::string Units::FREQUENCY_FAMILY("frequency");

const Units Units::UNITLESS("", "", 1.0, Units::UNITLESS_FAMILY);

const Units Units::SECONDS("seconds", "sec", 1.0, Units::ELAPSED_TIME_FAMILY);
const Units Units::MILLISECONDS("milliseconds", "ms", 0.001, Units::ELAPSED_TIME_FAMILY);
const Units Units::MICROSECONDS("microseconds", "us", 1e-6, Units::ELAPSED_TIME_FAMILY);
const Units Units::MINUTES("minutes", "min", 60.0, Units::ELAPSED_TIME_FAMILY);
const Units Units::HOURS("hours", "hr", 3600.0, Units::ELAPSED_TIME_FAMILY);
const Units Units::DAYS("days", "d", 86400.0, Units::ELAPSED_TIME_FAMILY);

const Units Units::RADIANS("radians", "rad", 1.0, Units::ANGLE_FAMILY);
const Units Units::DEGREES("degrees", "deg", simCore::DEG2RAD, Units::ANGLE_FAMILY);
const Units Units::MILLIRADIANS("milliradians", "mrad", 1e-3, Units::ANGLE_FAMILY);
const Units Units::BAM("binary angle measurement", "bam", M_2_PI, Units::ANGLE_FAMILY);
// Based on NATO definition of angular mils (6400 mils in a circle)
const Units Units::MIL("angular mil", "mil", 9.8174770424681038701957605727484e-4, Units::ANGLE_FAMILY);

const Units Units::METERS("meters", "m", 1.0, Units::LENGTH_FAMILY);
const Units Units::KILOMETERS("kilometers", "km", 1e3, Units::LENGTH_FAMILY);
const Units Units::YARDS("yards", "yd", 0.91439997, Units::LENGTH_FAMILY);
const Units Units::MILES("miles", "mi", 1609.3439, Units::LENGTH_FAMILY);
const Units Units::FEET("feet", "ft", 0.30479999, Units::LENGTH_FAMILY);
const Units Units::INCHES("inches", "in", 0.025399999, Units::LENGTH_FAMILY);
const Units Units::NAUTICAL_MILES("nautical miles", "nm", 1852.0, Units::LENGTH_FAMILY);
const Units Units::CENTIMETERS("centimeters", "cm", 1e-2, Units::LENGTH_FAMILY);
const Units Units::MILLIMETERS("millimeters", "mm", 1e-3, Units::LENGTH_FAMILY);
const Units Units::KILOYARDS("kiloyards", "kyd", 914.399998610, Units::LENGTH_FAMILY);
const Units Units::FATHOMS("fathoms", "fm", 1.82879994, Units::LENGTH_FAMILY);
const Units Units::KILOFEET("kilofeet", "kf", 304.79999, Units::LENGTH_FAMILY);
// Distance used in radar related subjects, equal to 6000 feet
const Units Units::DATA_MILES("data miles", "dm", 1828.800164446, Units::LENGTH_FAMILY);

const Units Units::METERS_PER_SECOND("meters per second", "m/sec", 1.0, Units::SPEED_FAMILY);
const Units Units::KILOMETERS_PER_HOUR("kilometers per hour", "km/hr", 0.27777778, Units::SPEED_FAMILY);
const Units Units::KNOTS("knots", "kts", 0.51444444, Units::SPEED_FAMILY);
const Units Units::MILES_PER_HOUR("miles per hour", "mph", 0.44703997, Units::SPEED_FAMILY);
const Units Units::FEET_PER_SECOND("feet per second", "ft/sec", 0.3047999, Units::SPEED_FAMILY);
const Units Units::KILOMETERS_PER_SECOND("kilometers per second", "km/sec", 1e3, Units::SPEED_FAMILY);
const Units Units::DATA_MILES_PER_HOUR("data miles per hour", "dm/hr", 0.50797738, Units::SPEED_FAMILY);
const Units Units::YARDS_PER_SECOND("yards per second", "yd/sec", 0.91439997, Units::SPEED_FAMILY);

// Frequency units
const Units Units::HERTZ("cycles per second", "Hz", 1.0, Units::FREQUENCY_FAMILY);
const Units Units::REVOLUTIONS_PER_MINUTE("revolutions per minute", "rpm", 60.0, Units::FREQUENCY_FAMILY);

///////////////////////////////////////////////////////

Units::Units(const std::string& name, const std::string& abbrev, double toBase, const std::string& family)
  : name_(name),
    abbrev_(abbrev),
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
    toBase_(1.0),
    family_(Units::INVALID_FAMILY)
{
}

Units::~Units()
{
}

bool Units::isValid() const
{
  return family_ != Units::INVALID_FAMILY;
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
  output = (value * toBase_) / toUnits.toBase_;
  return 0;
}

double Units::convertTo(const Units& toUnits, double value) const
{
  if (!canConvert(toUnits))
    return value;
  return (value * toBase_) / toUnits.toBase_;
}

bool Units::operator==(const Units& other) const
{
  // Ignore the name and abbreviation
  return toBase_ == other.toBase_ &&
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

  registerUnits(Units::HERTZ);
  registerUnits(Units::REVOLUTIONS_PER_MINUTE);
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
