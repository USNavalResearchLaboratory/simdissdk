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
* U.S. Naval Research Laboratory.
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*
*/
#include <cassert>
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
  }

  void UnitsComboBox::addUnits(QComboBox& comboBox, const std::string& unitFamily, const simCore::UnitsRegistry& reg)
  {
    simCore::UnitsRegistry::UnitsVector units = reg.units(unitFamily);
    for (simCore::UnitsRegistry::UnitsVector::const_iterator i = units.begin(); i != units.end(); ++i)
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

  void UnitsComboBox::addTimeFormats(QComboBox& comboBox)
  {
    addToList(comboBox, QObject::tr("Seconds"), simCore::TIMEFORMAT_SECONDS);
    addToList(comboBox, QObject::tr("Minutes"), simCore::TIMEFORMAT_MINUTES);
    addToList(comboBox, QObject::tr("Hours"), simCore::TIMEFORMAT_HOURS);
    addToList(comboBox, QObject::tr("Ordinal"), simCore::TIMEFORMAT_ORDINAL);
    addToList(comboBox, QObject::tr("Month Day Year"), simCore::TIMEFORMAT_MONTHDAY);
    addToList(comboBox, QObject::tr("Date Time Group"), simCore::TIMEFORMAT_DTG);
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
    comboBox.addItem(QString::fromStdString(units.name()), unitVar);
  }

}
