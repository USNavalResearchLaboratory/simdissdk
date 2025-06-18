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
#include <cassert>
#include "osgEarth/Units"
#include "simCore/Calc/Units.h"
#include "simUtil/UnitTypeConverter.h"

namespace simUtil {

static const osgEarth::UnitsType OSGEARTH_NONE;

/**
 * Helper class to UnitTypeConverter that stores the mappings from each
 * unit type to the other.  Handles gracefully when a mapping exists only
 * in one or two of the three systems.
 */
class UnitTypeConverter::LookupHelper
{
public:
  LookupHelper()
  {
  }

  /** Adds a mapping. */
  void add(const osgEarth::UnitsType& osg, const simCore::Units& core, int data)
  {
    // No way to add new families in osgEarth, so we can't test getType()
    const bool osgValid = !osg.getName().empty();
    const bool coreValid = core.isValid();
    // Data of 0 (Plug-in API CU_UNKNOWN) might sometimes map to simCore::UNITLESS, so cover that case
    const bool dataValid = (data != 0 || (!osgValid && coreValid));

    // Core <=> OSG, and Core <=> Data
    if (coreValid)
    {
      if (!core.name().empty())
        knownCore_.insert(core.name());
      // Core <=> OSG
      if (osgValid)
      {
        osgEarthToCore_[osg.getName()] = core;
        coreToOsgEarth_[core.name()] = osg;
      }
      // Core <=> Data
      if (dataValid)
      {
        dataToCore_[data] = core;
        coreToData_[core.name()] = data;
      }
    }

    // OSG <=> Data
    if (osgValid && dataValid)
    {
      osgEarthToData_[osg.getName()] = data;
      dataToOsgEarth_[data] = osg;
    }
  }

  /** Retrieve the simCore units */
  const simCore::Units& toCore(const osgEarth::UnitsType& osg) const
  {
    auto i = osgEarthToCore_.find(osg.getName());
    if (i == osgEarthToCore_.end())
      return simCoreInvalid_;
    return i->second;
  }

  /** Retrieve the simCore units */
  const simCore::Units& toCore(int data) const
  {
    auto i = dataToCore_.find(data);
    if (i == dataToCore_.end())
      return simCoreInvalid_;
    return i->second;
  }

  /** Retrieve the simData units */
  int toData(const osgEarth::UnitsType& osg) const
  {
    auto i = osgEarthToData_.find(osg.getName());
    if (i == osgEarthToData_.end())
      return 0;
    return i->second;
  }

  /** Retrieve the simData units */
  int toData(const simCore::Units& core) const
  {
    auto i = coreToData_.find(core.name());
    if (i == coreToData_.end())
      return 0;
    return i->second;
  }

  /** Retrieve the osgEarth units */
  const osgEarth::UnitsType& toOsgEarth(const simCore::Units& core) const
  {
    auto i = coreToOsgEarth_.find(core.name());
    if (i == coreToOsgEarth_.end())
      return osgEarthInvalid_;
    return i->second;
  }

  /** Retrieve the osgEarth units */
  const osgEarth::UnitsType& toOsgEarth(int data) const
  {
    auto i = dataToOsgEarth_.find(data);
    if (i == dataToOsgEarth_.end())
      return osgEarthInvalid_;
    return i->second;
  }

  /** Returns true if the given unit type was ever registered, even against invalid simData/osgEarth. */
  bool isRegistered(const simCore::Units& core) const
  {
    return knownCore_.find(core.name()) != knownCore_.end();
  }

private:
  std::set<std::string> knownCore_;

  std::map<std::string, simCore::Units> osgEarthToCore_;
  std::map<int, simCore::Units> dataToCore_;

  std::map<std::string, int> osgEarthToData_;
  std::map<std::string, int> coreToData_;

  std::map<std::string, osgEarth::UnitsType> coreToOsgEarth_;
  std::map<int, osgEarth::UnitsType> dataToOsgEarth_;
  const osgEarth::UnitsType osgEarthInvalid_;
  const simCore::Units simCoreInvalid_;
};

//////////////////////////////////////////////////////////////

UnitTypeConverter::UnitTypeConverter()
  : helper_(new LookupHelper)
{
  // Linear
  helper_->add(osgEarth::Units::CENTIMETERS, simCore::Units::CENTIMETERS, static_cast<int>(simData::DistanceUnits::UNITS_CENTIMETERS));
  helper_->add(osgEarth::Units::DATA_MILES, simCore::Units::DATA_MILES, static_cast<int>(simData::DistanceUnits::UNITS_DATAMILES));
  helper_->add(osgEarth::Units::FATHOMS, simCore::Units::FATHOMS, static_cast<int>(simData::DistanceUnits::UNITS_FATHOMS));
  helper_->add(osgEarth::Units::FEET, simCore::Units::FEET, static_cast<int>(simData::DistanceUnits::UNITS_FEET));
  helper_->add(osgEarth::Units::FEET_US_SURVEY, simCore::Units::UNITLESS, 0);
  helper_->add(osgEarth::Units::INCHES, simCore::Units::INCHES, static_cast<int>(simData::DistanceUnits::UNITS_INCHES));
  helper_->add(osgEarth::Units::KILOFEET, simCore::Units::KILOFEET, static_cast<int>(simData::DistanceUnits::UNITS_KILOFEET));
  helper_->add(osgEarth::Units::KILOMETERS, simCore::Units::KILOMETERS, static_cast<int>(simData::DistanceUnits::UNITS_KILOMETERS));
  helper_->add(osgEarth::Units::KILOYARDS, simCore::Units::KILOYARDS, static_cast<int>(simData::DistanceUnits::UNITS_KILOYARDS));
  helper_->add(osgEarth::Units::METERS, simCore::Units::METERS, static_cast<int>(simData::DistanceUnits::UNITS_METERS));
  helper_->add(osgEarth::Units::MILES, simCore::Units::MILES, static_cast<int>(simData::DistanceUnits::UNITS_MILES));
  helper_->add(osgEarth::Units::MILLIMETERS, simCore::Units::MILLIMETERS, static_cast<int>(simData::DistanceUnits::UNITS_MILLIMETERS));
  helper_->add(osgEarth::Units::NAUTICAL_MILES, simCore::Units::NAUTICAL_MILES, static_cast<int>(simData::DistanceUnits::UNITS_NAUTICAL_MILES));
  helper_->add(osgEarth::Units::YARDS, simCore::Units::YARDS, static_cast<int>(simData::DistanceUnits::UNITS_YARDS));

  // Angular
  helper_->add(osgEarth::Units::BAM, simCore::Units::BAM, static_cast<int>(simData::AngleUnits::UNITS_BAM));
  helper_->add(osgEarth::Units::DEGREES, simCore::Units::DEGREES, static_cast<int>(simData::AngleUnits::UNITS_DEGREES_MINUTES_SECONDS));
  helper_->add(osgEarth::Units::DEGREES, simCore::Units::DEGREES, static_cast<int>(simData::AngleUnits::UNITS_DEGREES_MINUTES));
  helper_->add(osgEarth::Units::DEGREES, simCore::Units::DEGREES, static_cast<int>(simData::AngleUnits::UNITS_DEGREES));
  // Last one (DEGREES) overrides previous (DEGREES_MINUTES, DEGREES_MINUTES_SECONDS)
  assert(helper_->toData(simCore::Units::DEGREES) == static_cast<int>(simData::AngleUnits::UNITS_DEGREES));
  helper_->add(osgEarth::Units::NATO_MILS, simCore::Units::MIL, static_cast<int>(simData::AngleUnits::UNITS_MIL));
  helper_->add(osgEarth::Units::RADIANS, simCore::Units::RADIANS, static_cast<int>(simData::AngleUnits::UNITS_RADIANS));
  // Decimal Hours has same name as "hours", elapsed time; skip the comparison
  //helper_->add(osgEarth::Units::DECIMAL_HOURS, simCore::Units::UNITLESS, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MILLIRADIANS, static_cast<int>(simData::AngleUnits::UNITS_MILLIRADIANS));
  helper_->add(OSGEARTH_NONE, simCore::Units::UNITLESS, static_cast<int>(simData::AngleUnits::UNITS_UTM));

  // Elapsed time
  helper_->add(osgEarth::Units::DAYS, simCore::Units::DAYS, 0);
  helper_->add(osgEarth::Units::HOURS, simCore::Units::HOURS, static_cast<int>(simData::ElapsedTimeFormat::ELAPSED_HOURS));
  helper_->add(osgEarth::Units::MICROSECONDS, simCore::Units::MICROSECONDS, 0);
  helper_->add(osgEarth::Units::MILLISECONDS, simCore::Units::MILLISECONDS, 0);
  helper_->add(osgEarth::Units::SECONDS, simCore::Units::SECONDS, static_cast<int>(simData::ElapsedTimeFormat::ELAPSED_SECONDS));
  helper_->add(osgEarth::Units::MINUTES, simCore::Units::MINUTES, static_cast<int>(simData::ElapsedTimeFormat::ELAPSED_MINUTES));
  helper_->add(osgEarth::Units::WEEKS, simCore::Units::UNITLESS, 0);

  // Speed
  helper_->add(osgEarth::Units::FEET_PER_SECOND, simCore::Units::FEET_PER_SECOND, static_cast<int>(simData::SpeedUnits::UNITS_FEET_PER_SECOND));
  helper_->add(osgEarth::Units::YARDS_PER_SECOND, simCore::Units::YARDS_PER_SECOND, static_cast<int>(simData::SpeedUnits::UNITS_YARDS_PER_SECOND));
  helper_->add(osgEarth::Units::METERS_PER_SECOND, simCore::Units::METERS_PER_SECOND, static_cast<int>(simData::SpeedUnits::UNITS_METERS_PER_SECOND));
  helper_->add(osgEarth::Units::KILOMETERS_PER_SECOND, simCore::Units::KILOMETERS_PER_SECOND, static_cast<int>(simData::SpeedUnits::UNITS_KILOMETERS_PER_SECOND));
  helper_->add(osgEarth::Units::KILOMETERS_PER_HOUR, simCore::Units::KILOMETERS_PER_HOUR, static_cast<int>(simData::SpeedUnits::UNITS_KILOMETERS_PER_HOUR));
  helper_->add(osgEarth::Units::MILES_PER_HOUR, simCore::Units::MILES_PER_HOUR, static_cast<int>(simData::SpeedUnits::UNITS_MILES_PER_HOUR));
  helper_->add(osgEarth::Units::DATA_MILES_PER_HOUR, simCore::Units::DATA_MILES_PER_HOUR, static_cast<int>(simData::SpeedUnits::UNITS_DATAMILES_PER_HOUR));
  helper_->add(osgEarth::Units::KNOTS, simCore::Units::KNOTS, static_cast<int>(simData::SpeedUnits::UNITS_KNOTS));

  // Screen
  helper_->add(osgEarth::Units::PIXELS, simCore::Units::UNITLESS, 0);

  // Acceleration
  helper_->add(OSGEARTH_NONE, simCore::Units::METERS_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::KILOMETERS_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::YARDS_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MILES_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::FEET_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::INCHES_PER_SECOND_SQUARED, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::NAUTICAL_MILES_PER_SECOND_SQUARED, 0);

  // Temperature
  helper_->add(OSGEARTH_NONE, simCore::Units::CELSIUS, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::FAHRENHEIT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::KELVIN, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::RANKINE, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::REAUMUR, 0);

  // Frequency
  helper_->add(OSGEARTH_NONE, simCore::Units::HERTZ, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::REVOLUTIONS_PER_MINUTE, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::RADIANS_PER_SECOND, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::DEGREES_PER_SECOND, 0);

  // Volume
  helper_->add(OSGEARTH_NONE, simCore::Units::LITER, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MILLILITER, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::FLUID_OUNCE, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::CUP, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::PINT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::QUART, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::GALLON, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::TEASPOON, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::TABLESPOON, 0);

  // Pressure
  helper_->add(OSGEARTH_NONE, simCore::Units::MILLIBAR, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::BAR, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::POUNDS_PER_SQUARE_INCH, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::ATMOSPHERE, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::TORR, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::PASCALS, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::KILOPASCAL, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MEGAPASCAL, 0);

  // Potential
  helper_->add(OSGEARTH_NONE, simCore::Units::VOLT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MILLIVOLT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MICROVOLT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::KILOVOLT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::MEGAVOLT, 0);
  helper_->add(OSGEARTH_NONE, simCore::Units::GIGAVOLT, 0);
}

UnitTypeConverter::~UnitTypeConverter()
{
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarth(const simCore::Units& core) const
{
  return helper_->toOsgEarth(core);
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarth(simData::ElapsedTimeFormat data) const
{
  return helper_->toOsgEarth(static_cast<int>(data));
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarth(simData::AngleUnits data) const
{
  return helper_->toOsgEarth(static_cast<int>(data));
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarth(simData::DistanceUnits data) const
{
  return helper_->toOsgEarth(static_cast<int>(data));
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarth(simData::SpeedUnits data) const
{
  return helper_->toOsgEarth(static_cast<int>(data));
}

const osgEarth::UnitsType& UnitTypeConverter::toOsgEarthFromData(int data) const
{
  return helper_->toOsgEarth(data);
}

const simCore::Units& UnitTypeConverter::toCore(const osgEarth::UnitsType& osg) const
{
  return helper_->toCore(osg);
}

const simCore::Units& UnitTypeConverter::toCore(simData::ElapsedTimeFormat data) const
{
  return helper_->toCore(static_cast<int>(data));
}

const simCore::Units& UnitTypeConverter::toCore(simData::AngleUnits data) const
{
  return helper_->toCore(static_cast<int>(data));
}

const simCore::Units& UnitTypeConverter::toCore(simData::DistanceUnits data) const
{
  return helper_->toCore(static_cast<int>(data));
}

const simCore::Units& UnitTypeConverter::toCore(simData::SpeedUnits data) const
{
  return helper_->toCore(static_cast<int>(data));
}

const simCore::Units& UnitTypeConverter::toCoreFromData(int data) const
{
  return helper_->toCore(data);
}

int UnitTypeConverter::toData(const osgEarth::UnitsType& osg) const
{
  return helper_->toData(osg);
}

int UnitTypeConverter::toData(const simCore::Units& core) const
{
  return helper_->toData(core);
}

bool UnitTypeConverter::isRegistered(const simCore::Units& units) const
{
  return helper_->isRegistered(units);
}

void UnitTypeConverter::addMapping(const osgEarth::UnitsType& osg, const simCore::Units& core, int data)
{
  helper_->add(osg, core, data);
}

void UnitTypeConverter::addMapping(const osgEarth::UnitsType& osg, const simCore::Units& core)
{
  addMapping(osg, core, 0);
}

void UnitTypeConverter::addMapping(const simCore::Units& core, int data)
{
  addMapping(OSGEARTH_NONE, core, data);
}

void UnitTypeConverter::addMapping(const osgEarth::UnitsType& osg, int data)
{
  addMapping(osg, simCore::Units::UNITLESS, data);
}

}
