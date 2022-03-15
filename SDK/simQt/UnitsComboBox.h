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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_UNITSCOMBOBOX_H
#define SIMQT_UNITSCOMBOBOX_H

#include <QComboBox>
#include <QMetaType>
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/Time/Constants.h"
#include "simCore/Calc/Units.h"
#include "simCore/Common/Common.h"
#include "simCore/String/Angle.h"

class QComboBox;

namespace simQt {

/**
 * Utility class that provides functions to add items to combo boxes and get values easily from combo boxes.
 * get/set methods work with both simCore::Units and enumerations such as simCore::CoordinateSystem.
 *
 * Example usage:
 *
 *  QComboBox* comboBox = new QComboBox(frame);
 *
 *  // Initialization
 *  simQt::UnitsComboBox::addUnits(comboBox, simCore::LENGTH_FAMILY);
 *
 *  // Data retrieval, explicit cast
 *  simCore::Units unit;
 *  simQt::UnitsComboBox::getCurrentValue(comboBox, unit);
 *
 *  // Setting the current value
 *  simQt::UnitsComboBox::setCurrentValue(comboBox, unit);
 *
 * The Units/enumeration value is stored in the "getItemData" of the QComboBox entry's item.
 */
class SDKQT_EXPORT UnitsComboBox
{
public:
  /** Populates a combo box with simCore::Units values; generic, same order as the unit vector */
  static void addUnits(QComboBox& comboBox, const std::string& unitFamily, const simCore::UnitsRegistry& reg);

  /** Populates a combo box with altitude units in an altitude-friendly order */
  static void addAltitudeUnits(QComboBox& comboBox);
  /** Populates a combo box with distance units in a distance-friendly order */
  static void addDistanceUnits(QComboBox& comboBox);
  /** Populates a combo box with speed units in a speed-friendly order */
  static void addSpeedUnits(QComboBox& comboBox);
  /** Populates a combo box with angle units in a friendly order */
  static void addAngleUnits(QComboBox& comboBox);
  /** Populates a combo box with simCore::TimeFormat values */
  static void addTimeFormats(QComboBox& comboBox);
  /** Populates a combo box with simCore::CoordinateSystem values */
  static void addCoordinateSystems(QComboBox& comboBox);
  /** Populates a combo box with simCore::VerticalDatum values */
  static void addVerticalData(QComboBox& comboBox);
  /** Populates a combo box with simCore::MagneticVariance values */
  static void addMagneticVariances(QComboBox& comboBox);
  /** Populates a combo box with simCore::GeodeticFormat values */
  static void addGeodeticFormats(QComboBox& comboBox);
  /** Populates a combo box with simCore::TimeFormat values useful for duration */
  static void addDurationFormats(QComboBox& comboBox);

  /** Retrieves the current value.  Returns 0 on success, non-zero otherwise */
  template <typename T>
  static int getCurrentValue(QComboBox& comboBox, T& value)
  {
    const QVariant currentVal = comboBox.itemData(comboBox.currentIndex());
    if (!currentVal.canConvert<T>())
      return 1;
    value = qvariant_cast<T>(currentVal);
    return 0;
  }

  /** Sets the index to the given item if it is found. Returns 0 on success; non-zero otherwise */
  template <typename T>
  static int setCurrentValue(QComboBox& comboBox, T value)
  {
    const QVariant currentData = comboBox.itemData(comboBox.currentIndex());
    if (currentData.canConvert<T>() && currentData.value<T>() != value)
    {
      for (int k = 0; k < comboBox.count(); ++k)
      {
        QVariant val = comboBox.itemData(k);
        if (val.canConvert<T>() && val.value<T>() == value)
        {
          comboBox.setCurrentIndex(k);
          return 0; // Success
        }
      }
      // Error: Iterated through and didn't find a matching value
      return 1;
    }
    // Success, it's already set to this value
    return 0;
  }

private:
  static void addUnitsItem_(QComboBox& comboBox, const simCore::Units& units);
};

/**
 * Easy-to-use single combo box for unit selection.  Starts out empty.  Once you set
 * units on it, the available unit types will expand.
 */
class SDKQT_EXPORT UnitsSelectorComboBox : public QComboBox
{
  Q_OBJECT;
  /** Define the property for the currently displayed unit */
  Q_PROPERTY(simCore::Units units READ units WRITE setUnits)

public:
  explicit UnitsSelectorComboBox(QWidget* parent=nullptr);
  virtual ~UnitsSelectorComboBox();

  /** Change the registry.  Call this before setting units.  Registry should outlive this class. */
  void setUnitsRegistry(const simCore::UnitsRegistry* registry);

  /** Retrieves the currently displayed unit. */
  const simCore::Units& units() const;

public slots:
  /** Change the currently displayed unit.  Will fill out the family of units if changing families. */
  void setUnits(const simCore::Units& unit);

signals:
  /** Currently selected units have changed. */
  void unitsChanged(const simCore::Units& unit) const;

private slots:
  /** Intercept the currentChanged() signal and re-emit in Units format. */
  void emitUnitsOnCurrentChange_(int newCurrent);

private:
  const simCore::UnitsRegistry* registry_;
  bool registryOwned_;
  simCore::Units units_;
};

}

Q_DECLARE_METATYPE(simCore::Units);
Q_DECLARE_METATYPE(simCore::TimeFormat);
Q_DECLARE_METATYPE(simCore::CoordinateSystem);
Q_DECLARE_METATYPE(simCore::VerticalDatum);
Q_DECLARE_METATYPE(simCore::MagneticVariance);
Q_DECLARE_METATYPE(simCore::GeodeticFormat);

#endif /* SIMQT_UNITSCOMBOBOX_H */
