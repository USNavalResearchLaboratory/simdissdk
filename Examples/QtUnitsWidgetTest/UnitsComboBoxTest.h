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
#ifndef SDKEXAMPLE_UNITSCOMBOBOXTEST_H
#define SDKEXAMPLE_UNITSCOMBOBOXTEST_H

#include <QObject>
#include <QLineEdit>
#include "simCore/Time/Constants.h"
#include "simCore/Calc/VerticalDatum.h"
#include "simCore/Calc/MagneticVariance.h"
#include "simCore/Calc/CoordinateSystem.h"

namespace simCore {
  class Units;
}

class Ui_UnitsComboBoxTest;

class UnitsComboBoxTest : public QWidget
{
  Q_OBJECT;
public:
  explicit UnitsComboBoxTest(QWidget* parent = nullptr);
  virtual ~UnitsComboBoxTest();

private Q_SLOTS:
  void updateUnitConverter_();

private:
  Ui_UnitsComboBoxTest* ui_;
};

#endif /* SDKEXAMPLE_UNITSCOMBOBOXTEST_H */

