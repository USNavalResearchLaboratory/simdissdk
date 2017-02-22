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
  *  QComboBox* listBox = new QComboBox(frame);
  *
  *  // Initialization
  *  simQt::UnitsComboBox::addUnits(listBox, simCore::LENGTH_FAMILY);
  *
  *  // Data retrieval, explicit cast
  *  simCore::Units unit;
  *  simQt::UnitsComboBox::getCurrentValue(listBox, unit);
  *
  *  // Setting the current value
  *  simQt::UnitsComboBox::setCurrentValue(listBox, unit);
  *
  * The Units/enumeration value is stored in the "getItemData" of the QComboBox entry's item.
  */
  class SDKQT_EXPORT UnitsComboBox
  {
  public:
    /** Populates a list box with simCore::Units values; generic, same order as the unit vector */
    static void addUnits(QComboBox& listBox, const std::string& unitFamily, const simCore::UnitsRegistry& reg);

    /** Populates a list box with altitude units in an altitude-friendly order */
    static void addAltitudeUnits(QComboBox& listBox);
    /** Populates a list box with distance units in a distance-friendly order */
    static void addDistanceUnits(QComboBox& listBox);
    /** Populates a list box with speed units in a speed-friendly order */
    static void addSpeedUnits(QComboBox& listBox);
    /** Populates a list box with simCore::TimeFormat values */
    static void addTimeFormats(QComboBox& listBox);
    /** Populates a list box with simCore::CoordinateSystem values */
    static void addCoordinateSystems(QComboBox& listBox);
    /** Populates a list box with simCore::VerticalDatum values */
    static void addVerticalData(QComboBox& listBox);
    /** Populates a list box with simCore::MagneticVariance values */
    static void addMagneticVariances(QComboBox& listBox);
    /** Populates a list box with simCore::GeodeticFormat values */
    static void addGeodeticFormats(QComboBox& listBox);

    /** Retrieves the current value.  Returns 0 on success, non-zero otherwise */
    template <typename T>
    static int getCurrentValue(QComboBox& listBox, T& value)
    {
      const QVariant currentVal = listBox.itemData(listBox.currentIndex());
      if (!currentVal.canConvert<T>())
        return 1;
      value = qvariant_cast<T>(currentVal);
      return 0;
    }

    /** Sets the index to the given item if it is found. Returns 0 on success; non-zero otherwise */
    template <typename T>
    static int setCurrentValue(QComboBox& listBox, T value)
    {
      const QVariant currentData = listBox.itemData(listBox.currentIndex());
      if (currentData.canConvert<T>() && currentData.value<T>() != value)
      {
        for (int k = 0; k < listBox.count(); ++k)
        {
          QVariant val = listBox.itemData(k);
          if (val.canConvert<T>() && val.value<T>() == value)
          {
            listBox.setCurrentIndex(k);
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

}

Q_DECLARE_METATYPE(simCore::Units);
Q_DECLARE_METATYPE(simCore::TimeFormat);
Q_DECLARE_METATYPE(simCore::CoordinateSystem);
Q_DECLARE_METATYPE(simCore::VerticalDatum);
Q_DECLARE_METATYPE(simCore::MagneticVariance);
Q_DECLARE_METATYPE(simCore::GeodeticFormat);

#endif /* SIMQT_UNITSCOMBOBOX_H */
