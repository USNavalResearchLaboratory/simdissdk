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
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#include <cassert>
#include <set>
#include "simCore/String/Format.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/UnitsComboBox.h"

namespace simQt {

namespace
{
template <typename T>
void addToList(QComboBox& comboBox, const QString& text, T value)
{
  QVariant var;
  // Have to pass an enum in as a QVariant, otherwise addItem() will cast it as an int
  var.setValue(value);
  comboBox.addItem(text, var);
}

/** Provides sorting for simCore::Units using operator< logic */
class UnitsLessThan
{
public:
  /** Returns true if lhs < rhs */
  bool operator()(const simCore::Units& lhs, const simCore::Units& rhs) const
  {
    // Prefer base scalar
    if (lhs.toBaseScalar() < rhs.toBaseScalar())
      return true;
    if (lhs.toBaseScalar() > rhs.toBaseScalar())
      return false;
    // Return simply based on name
    return simCore::upperCase(lhs.name()) < simCore::upperCase(rhs.name());
  }
};

/** Create an easy-to-use name for the set that is ordered by to-base scalar */
typedef std::set<simCore::Units, UnitsLessThan> SortedUnitsSet;

}

////////////////////////////////////////////////////

void UnitsComboBox::addUnits(QComboBox& comboBox, const std::string& unitFamily, const simCore::UnitsRegistry& reg)
{
  simCore::UnitsRegistry::UnitsVector units = reg.units(unitFamily);

  // Establish a list of candidate priority units based on type.
  SortedUnitsSet candidatePriority;
  if (unitFamily == simCore::Units::ANGLE_FAMILY)
  {
    candidatePriority.insert(simCore::Units::DEGREES);
    candidatePriority.insert(simCore::Units::RADIANS);
  }
  else if (unitFamily == simCore::Units::LENGTH_FAMILY)
  {
    candidatePriority.insert(simCore::Units::FEET);
    candidatePriority.insert(simCore::Units::YARDS);
    candidatePriority.insert(simCore::Units::METERS);
    candidatePriority.insert(simCore::Units::KILOFEET);
    candidatePriority.insert(simCore::Units::KILOYARDS);
    candidatePriority.insert(simCore::Units::KILOMETERS);
    candidatePriority.insert(simCore::Units::NAUTICAL_MILES);
  }
  else if (unitFamily == simCore::Units::SPEED_FAMILY)
  {
    candidatePriority.insert(simCore::Units::MILES_PER_HOUR);
    candidatePriority.insert(simCore::Units::KNOTS);
    candidatePriority.insert(simCore::Units::METERS_PER_SECOND);
  }
  // Other families do not have default priority units

  // Next, sort the list into priority and normal units.  Note that the candidate
  // priorities are NOT instantly added.
  SortedUnitsSet priorityUnits;
  SortedUnitsSet remainingUnits;
  for (auto i = units.begin(); i != units.end(); ++i)
  {
    if (candidatePriority.find(*i) != candidatePriority.end())
      priorityUnits.insert(*i);
    else
      remainingUnits.insert(*i);
  }

  // Add each priority unit, along with a separator
  if (!priorityUnits.empty())
  {
    for (auto i = priorityUnits.begin(); i != priorityUnits.end(); ++i)
      addUnitsItem_(comboBox, *i);
    comboBox.insertSeparator(comboBox.count());
  }
  // Add the remaining, non-priority units
  for (auto i = remainingUnits.begin(); i != remainingUnits.end(); ++i)
    addUnitsItem_(comboBox, *i);
}

void UnitsComboBox::addAltitudeUnits(QComboBox& comboBox)
{
  addUnitsItem_(comboBox, simCore::Units::FEET);
  addUnitsItem_(comboBox, simCore::Units::YARDS);
  addUnitsItem_(comboBox, simCore::Units::METERS);
  addUnitsItem_(comboBox, simCore::Units::KILOFEET);
  comboBox.insertSeparator(comboBox.count());
  addUnitsItem_(comboBox, simCore::Units::MILLIMETERS);
  addUnitsItem_(comboBox, simCore::Units::CENTIMETERS);
  addUnitsItem_(comboBox, simCore::Units::INCHES);
  addUnitsItem_(comboBox, simCore::Units::FATHOMS);
  addUnitsItem_(comboBox, simCore::Units::KILOYARDS);
  addUnitsItem_(comboBox, simCore::Units::KILOMETERS);
  addUnitsItem_(comboBox, simCore::Units::MILES);
  addUnitsItem_(comboBox, simCore::Units::DATA_MILES);
  addUnitsItem_(comboBox, simCore::Units::NAUTICAL_MILES);
}

void UnitsComboBox::addDistanceUnits(QComboBox& comboBox)
{
  addUnitsItem_(comboBox, simCore::Units::YARDS);
  addUnitsItem_(comboBox, simCore::Units::METERS);
  addUnitsItem_(comboBox, simCore::Units::KILOYARDS);
  addUnitsItem_(comboBox, simCore::Units::KILOMETERS);
  addUnitsItem_(comboBox, simCore::Units::MILES);
  addUnitsItem_(comboBox, simCore::Units::NAUTICAL_MILES);
  comboBox.insertSeparator(comboBox.count());
  addUnitsItem_(comboBox, simCore::Units::MILLIMETERS);
  addUnitsItem_(comboBox, simCore::Units::CENTIMETERS);
  addUnitsItem_(comboBox, simCore::Units::INCHES);
  addUnitsItem_(comboBox, simCore::Units::FEET);
  addUnitsItem_(comboBox, simCore::Units::FATHOMS);
  addUnitsItem_(comboBox, simCore::Units::KILOFEET);
  addUnitsItem_(comboBox, simCore::Units::DATA_MILES);
}

void UnitsComboBox::addSpeedUnits(QComboBox& comboBox)
{
  addUnitsItem_(comboBox, simCore::Units::METERS_PER_SECOND);
  addUnitsItem_(comboBox, simCore::Units::MILES_PER_HOUR);
  addUnitsItem_(comboBox, simCore::Units::KNOTS);
  comboBox.insertSeparator(comboBox.count());
  addUnitsItem_(comboBox, simCore::Units::FEET_PER_SECOND);
  addUnitsItem_(comboBox, simCore::Units::YARDS_PER_SECOND);
  addUnitsItem_(comboBox, simCore::Units::KILOMETERS_PER_SECOND);
  addUnitsItem_(comboBox, simCore::Units::KILOMETERS_PER_HOUR);
  addUnitsItem_(comboBox, simCore::Units::DATA_MILES_PER_HOUR);
}

void UnitsComboBox::addAngleUnits(QComboBox& comboBox)
{
  addUnitsItem_(comboBox, simCore::Units::DEGREES);
  addUnitsItem_(comboBox, simCore::Units::RADIANS);
  addUnitsItem_(comboBox, simCore::Units::MILLIRADIANS);
  addUnitsItem_(comboBox, simCore::Units::BAM);
  addUnitsItem_(comboBox, simCore::Units::MIL);
}

void UnitsComboBox::addTimeFormats(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("Seconds"), simCore::TIMEFORMAT_SECONDS);
  addToList(comboBox, QObject::tr("Minutes"), simCore::TIMEFORMAT_MINUTES);
  addToList(comboBox, QObject::tr("Hours"), simCore::TIMEFORMAT_HOURS);
  addToList(comboBox, QObject::tr("Ordinal"), simCore::TIMEFORMAT_ORDINAL);
  addToList(comboBox, QObject::tr("Month Day Year"), simCore::TIMEFORMAT_MONTHDAY);
  addToList(comboBox, QObject::tr("Date Time Group"), simCore::TIMEFORMAT_DTG);
}

void UnitsComboBox::addDurationFormats(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("Seconds"), simCore::TIMEFORMAT_SECONDS);
  addToList(comboBox, QObject::tr("Minutes"), simCore::TIMEFORMAT_MINUTES);
  addToList(comboBox, QObject::tr("Hours"), simCore::TIMEFORMAT_HOURS);
}

void UnitsComboBox::addCoordinateSystems(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("LLA"), simCore::COORD_SYS_LLA);
  addToList(comboBox, QObject::tr("ECEF"), simCore::COORD_SYS_ECEF);
  addToList(comboBox, QObject::tr("X-East"), simCore::COORD_SYS_XEAST);
  comboBox.insertSeparator(comboBox.count());
  addToList(comboBox, QObject::tr("ENU"), simCore::COORD_SYS_ENU);
  addToList(comboBox, QObject::tr("NED"), simCore::COORD_SYS_NED);
  addToList(comboBox, QObject::tr("NWU"), simCore::COORD_SYS_NWU);
  addToList(comboBox, QObject::tr("Generic"), simCore::COORD_SYS_GTP);
  addToList(comboBox, QObject::tr("ECI"), simCore::COORD_SYS_ECI);
}

void UnitsComboBox::addVerticalData(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("Height Above Ellipsoid"), simCore::VERTDATUM_WGS84);
  addToList(comboBox, QObject::tr("Mean Sea Level"), simCore::VERTDATUM_MSL);
  addToList(comboBox, QObject::tr("User-Defined"), simCore::VERTDATUM_USER);
}

void UnitsComboBox::addMagneticVariances(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("True Angles"), simCore::MAGVAR_TRUE);
  addToList(comboBox, QObject::tr("Magnetic Angles (WMM)"), simCore::MAGVAR_WMM);
  addToList(comboBox, QObject::tr("User-Defined"), simCore::MAGVAR_USER);
}

void UnitsComboBox::addGeodeticFormats(QComboBox& comboBox)
{
  addToList(comboBox, QObject::tr("Degrees"), simCore::FMT_DEGREES);
  addToList(comboBox, QObject::tr("Degrees Minutes"), simCore::FMT_DEGREES_MINUTES);
  addToList(comboBox, QObject::tr("Degrees Minutes Seconds"), simCore::FMT_DEGREES_MINUTES_SECONDS);
}

void UnitsComboBox::addUnitsItem_(QComboBox& comboBox, const simCore::Units& units)
{
  QVariant unitVar;
  unitVar.setValue(units);

  // Capitalize the first letter of each word
  QStringList parts = QString::fromStdString(units.name()).split(' ', QString::SkipEmptyParts);
  for (int k = 0; k < parts.size(); ++k)
    parts[k][0] = parts[k][0].toUpper();

  comboBox.addItem(parts.join(' '), unitVar);
}

////////////////////////////////////////////////////

UnitsSelectorComboBox::UnitsSelectorComboBox(QWidget* parent)
  : QComboBox(parent),
    registry_(NULL),
    registryOwned_(true)
{
  setUnitsRegistry(NULL);
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(emitUnitsOnCurrentChange_(int)));
}

void UnitsSelectorComboBox::setUnitsRegistry(const simCore::UnitsRegistry* registry)
{
  if (registry != NULL && registry == registry_)
    return;

  if (registryOwned_)
    delete registry_;
  registry_ = NULL;

  if (registry != NULL)
  {
    registry_ = registry;
    registryOwned_ = false;
    return;
  }

  // Always have a non-NULL units registry
  simCore::UnitsRegistry* nonConst = new simCore::UnitsRegistry;
  nonConst->registerDefaultUnits();
  registry_ = nonConst;
  registryOwned_ = true;
}

UnitsSelectorComboBox::~UnitsSelectorComboBox()
{
  if (registryOwned_)
    delete registry_;
}

const simCore::Units& UnitsSelectorComboBox::units() const
{
  return units_;
}

void UnitsSelectorComboBox::setUnits(const simCore::Units& units)
{
  if (units_ == units)
    return;

  {
    // Install a blocker so that signals don't go out for clearing, updating units,
    // or setting current value.  All of those signals are dealt with internally.
    // This prevents aliasing in a programmatic call to setUnits().
    simQt::ScopedSignalBlocker blocker(*this);

    // Reset the family
    if (units_.family() != units.family())
    {
      clear();
      simQt::UnitsComboBox::addUnits(*this, units.family(), *registry_);
    }

    // Assign units, the update the GUI.  Because signals are blocked,
    // emitUnitsOnCurrentChange_() will not be called automatically
    units_ = units;
    simQt::UnitsComboBox::setCurrentValue(*this, units_);
  }

  // Tell listeners that the units changed.
  emit unitsChanged(units_);
}

void UnitsSelectorComboBox::emitUnitsOnCurrentChange_(int newCurrent)
{
  simQt::UnitsComboBox::getCurrentValue<simCore::Units>(*this, units_);
  emit unitsChanged(units_);
}

}
