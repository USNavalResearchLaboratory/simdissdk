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
#ifndef SIMCORE_CALC_UNITS_H
#define SIMCORE_CALC_UNITS_H

#include <string>
#include <vector>
#include <map>
#include "simCore/Common/Common.h"

// Units code can cause strange behavior on newer versions of g++.  Mark
// the static member values as hidden to avoid the symbol lookup problems.
#if defined(__GNUC__) && defined(simCore_LIB_EXPORT_STATIC)
#define ATTRIB_HIDDEN __attribute((visibility("hidden")))
#else
#define ATTRIB_HIDDEN
#endif

namespace simCore {

/** Definition for a single unit of measurement */
class SDKCORE_EXPORT Units
{
public:
  /** Unitless value that has no conversion factor. */
  static const Units UNITLESS ATTRIB_HIDDEN;

  /** @name ElapsedTime */
  ///@{
  /// Elapsed time measurement units
  static const Units SECONDS ATTRIB_HIDDEN;
  static const Units MILLISECONDS ATTRIB_HIDDEN;
  static const Units MICROSECONDS ATTRIB_HIDDEN;
  static const Units NANOSECONDS ATTRIB_HIDDEN;
  static const Units MINUTES ATTRIB_HIDDEN;
  static const Units HOURS ATTRIB_HIDDEN;
  static const Units DAYS ATTRIB_HIDDEN;
  ///@}

  /** @name Angles */
  ///@{
  /// Angle measurement units
  static const Units RADIANS ATTRIB_HIDDEN;
  static const Units DEGREES ATTRIB_HIDDEN;
  static const Units MILLIRADIANS ATTRIB_HIDDEN;
  /** BAM (Binary Angle Measurement) angle measurement unit */
  static const Units BAM ATTRIB_HIDDEN;
  /** Angular Mil (NATO variant) angle measurement unit */
  static const Units MIL ATTRIB_HIDDEN;
  ///@}

  /** @name Length */
  ///@{
  /// Length measurement units
  static const Units METERS ATTRIB_HIDDEN;
  static const Units KILOMETERS ATTRIB_HIDDEN;
  static const Units YARDS ATTRIB_HIDDEN;
  static const Units MILES ATTRIB_HIDDEN;
  static const Units FEET ATTRIB_HIDDEN;
  static const Units INCHES ATTRIB_HIDDEN;
  static const Units NAUTICAL_MILES ATTRIB_HIDDEN;
  static const Units CENTIMETERS ATTRIB_HIDDEN;
  static const Units MILLIMETERS ATTRIB_HIDDEN;
  static const Units KILOYARDS ATTRIB_HIDDEN;
  static const Units FATHOMS ATTRIB_HIDDEN;
  static const Units KILOFEET ATTRIB_HIDDEN;
  /** Length measurement used in radar related subjects, equal to 6000 feet */
  static const Units DATA_MILES ATTRIB_HIDDEN;
  ///@}

  /** @name Speed */
  ///@{
  /// Speed measurement units
  static const Units METERS_PER_SECOND ATTRIB_HIDDEN;
  static const Units KILOMETERS_PER_HOUR ATTRIB_HIDDEN;
  static const Units KNOTS ATTRIB_HIDDEN;
  static const Units MILES_PER_HOUR ATTRIB_HIDDEN;
  static const Units FEET_PER_SECOND ATTRIB_HIDDEN;
  static const Units KILOMETERS_PER_SECOND ATTRIB_HIDDEN;
  static const Units DATA_MILES_PER_HOUR ATTRIB_HIDDEN;
  static const Units YARDS_PER_SECOND ATTRIB_HIDDEN;
  ///@}

  /** @name Acceleration */
  ///@{
  /// Acceleration measurement units
  static const Units METERS_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units KILOMETERS_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units YARDS_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units MILES_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units FEET_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units INCHES_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  static const Units NAUTICAL_MILES_PER_SECOND_SQUARED ATTRIB_HIDDEN;
  ///@}

  /** @name Temperature */
  ///@{
  /// Temperature measurement units
  static const Units CELSIUS ATTRIB_HIDDEN;
  static const Units FAHRENHEIT ATTRIB_HIDDEN;
  static const Units KELVIN ATTRIB_HIDDEN;
  static const Units RANKINE ATTRIB_HIDDEN;
  static const Units REAUMUR ATTRIB_HIDDEN;
  ///@}

  /** @name Frequency */
  ///@{
  /// Frequency Units
  static const Units HERTZ ATTRIB_HIDDEN;
  static const Units KILOHERTZ ATTRIB_HIDDEN;
  static const Units MEGAHERTZ ATTRIB_HIDDEN;
  static const Units REVOLUTIONS_PER_MINUTE ATTRIB_HIDDEN;
  static const Units RADIANS_PER_SECOND ATTRIB_HIDDEN;
  static const Units DEGREES_PER_SECOND ATTRIB_HIDDEN;
  ///@}

  /** @name Volume */
  ///@{
  /// Volume Units
  static const Units LITER ATTRIB_HIDDEN;
  static const Units MILLILITER ATTRIB_HIDDEN;
  static const Units FLUID_OUNCE ATTRIB_HIDDEN;
  static const Units CUP ATTRIB_HIDDEN;
  static const Units PINT ATTRIB_HIDDEN;
  static const Units QUART ATTRIB_HIDDEN;
  static const Units GALLON ATTRIB_HIDDEN;
  static const Units TEASPOON ATTRIB_HIDDEN;
  static const Units TABLESPOON ATTRIB_HIDDEN;
  ///@}

  /** @name Pressure */
  ///@{
  /// Pressure Units
  static const Units MILLIBAR ATTRIB_HIDDEN;
  static const Units BAR ATTRIB_HIDDEN;
  static const Units POUNDS_PER_SQUARE_INCH ATTRIB_HIDDEN;
  static const Units ATMOSPHERE ATTRIB_HIDDEN;
  static const Units TORR ATTRIB_HIDDEN;
  /** NOTE: Use PASCALS instead of PASCAL to avoid naming conflict with macro from minwindef.h */
  static const Units PASCALS ATTRIB_HIDDEN;
  static const Units KILOPASCAL ATTRIB_HIDDEN;
  static const Units MEGAPASCAL ATTRIB_HIDDEN;
  ///@}

  /** @name Potential */
  ///@{
  /// Potential Units
  static const Units VOLT ATTRIB_HIDDEN;
  static const Units MILLIVOLT ATTRIB_HIDDEN;
  static const Units MICROVOLT ATTRIB_HIDDEN;
  static const Units KILOVOLT ATTRIB_HIDDEN;
  static const Units MEGAVOLT ATTRIB_HIDDEN;
  static const Units GIGAVOLT ATTRIB_HIDDEN;
  ///@}

  ///@{
  /// Predefined unit families
  static const std::string INVALID_FAMILY ATTRIB_HIDDEN;
  static const std::string UNITLESS_FAMILY ATTRIB_HIDDEN;
  static const std::string ELAPSED_TIME_FAMILY ATTRIB_HIDDEN;
  static const std::string ANGLE_FAMILY ATTRIB_HIDDEN;
  static const std::string LENGTH_FAMILY ATTRIB_HIDDEN;
  static const std::string SPEED_FAMILY ATTRIB_HIDDEN;
  static const std::string ACCELERATION_FAMILY ATTRIB_HIDDEN;
  static const std::string TEMPERATURE_FAMILY ATTRIB_HIDDEN;
  static const std::string FREQUENCY_FAMILY ATTRIB_HIDDEN;
  static const std::string VOLUME_FAMILY ATTRIB_HIDDEN;
  static const std::string PRESSURE_FAMILY ATTRIB_HIDDEN;
  static const std::string POTENTIAL_FAMILY ATTRIB_HIDDEN;
  ///@}

  /** Construct an invalid unit */
  Units();
  /** Construct a new unit type belonging to the given family with conversion factor */
  Units(const std::string& name, const std::string& abbrev, double toBase, const std::string& family);
  /** Virtual destructor */
  virtual ~Units();

  /** Retrieves the name of the unit */
  const std::string& name() const;
  /** Retrieves the abbreviation of the unit */
  const std::string& abbreviation() const;
  /** Retrieves the family to which this unit belongs */
  const std::string& family() const;

  /** Returns true if units can be converted between */
  bool canConvert(const Units& toUnits) const;
  /** Returns 0 on success and sets output to converted value.  Non-zero for failure, and output set to value. */
  int convertTo(const Units& toUnits, double value, double& output) const;
  /** Returns converted value, returning value on error */
  double convertTo(const Units& toUnits, double value) const;

  /** Returns true if two units are equal */
  bool operator==(const Units& other) const;
  /** Returns true if two units are unequal */
  bool operator!=(const Units& other) const;

  /** Returns true if valid unit type */
  bool isValid() const;

  /** Retrieves the to-base scalar.  This is 1.0 for base units. */
  double toBaseScalar() const;
  /** Retrieves the to-base offset.  This is typically 0.0 */
  double toBaseOffset() const;

  /**
   * Factory Method for a unit that is scaled and offset from base, such that:
   *   baseUnitValue = (unitValue + offset) * toBase.
   * This is useful for conversions like Celsius and Fahrenheit, where Fahrenheit can be defined as:
   *   Celsius = (5. / 9.) * (Fahrenheit - 32.)
   * In the example above, offset is -32.0 and toBase is (5. / 9.).  This can
   * also be used for Kelvin conversion, where toBase would be 1. and offset -273.
   */
  static Units offsetThenScaleUnit(const std::string& name, const std::string& abbrev,
    double offset, double toBase, const std::string& family);

private:
  std::string name_;
  std::string abbrev_;
  double toBaseOffset_;
  double toBase_;
  std::string family_;
};

/** Searchable registry of all unit types and families */
class SDKCORE_EXPORT UnitsRegistry
{
public:
  /** Vector of units */
  typedef std::vector<Units> UnitsVector;

  /** Construct an empty UnitsRegistry */
  UnitsRegistry();
  /** Destroys memory */
  virtual ~UnitsRegistry();

  /** Registers all built-in units */
  void registerDefaultUnits();
  /** Called by Units constructor to register a new unit type */
  int registerUnits(const Units& units);

  /** Retrieve all units belonging to given family */
  const UnitsVector& units(const std::string& family) const;
  /** Retrieve all registered families */
  std::vector<std::string> families() const;

  /** Retrieve units with given name; returns invalid if not found.  Names are not case sensitive. */
  const Units& unitsByName(const std::string& name) const;
  /** Retrieve units with given name; returns 0 if unit found, non-zero if not found.  Names are not case sensitive. */
  int unitsByName(const std::string& name, Units& outUnits) const;
  /** Retrieve units with given abbreviation; returns invalid if not found.  Abbreviations are case sensitive. */
  const Units& unitsByAbbreviation(const std::string& abbrev) const;
  /** Retrieve units with given abbreviation; returns 0 if unit found, non-zero if not found.   Abbreviations are case sensitive. */
  int unitsByAbbreviation(const std::string& abbrev, Units& outUnits) const;

private:
  Units invalidUnits_;
  UnitsVector emptyUnitsVector_;
  std::map<std::string, UnitsVector> units_;
  std::map<std::string, Units> unitsByName_;
  std::map<std::string, Units> unitsByAbbrev_;
};

}

#undef ATTRIB_HIDDEN

#endif /* SIMCORE_CALC_UNITS_H */
