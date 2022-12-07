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
#include "simCore/Calc/Units.h"
#include "simCore/String/Angle.h"
#include "simCore/String/Format.h"
#include "simQt/UnitsComboBox.h"
#include "UnitsComboBoxTest.h"
#include "ui_UnitsComboBoxTest.h"


UnitsComboBoxTest::UnitsComboBoxTest(QWidget* parent)
  : QWidget(parent),
  ui_(new Ui_UnitsComboBoxTest)
{
  ui_->setupUi(this);
  simCore::UnitsRegistry reg;
  reg.registerDefaultUnits();

  simQt::UnitsComboBox::addTimeFormats(*ui_->timeCombo);
  simQt::UnitsComboBox::addDistanceUnits(*ui_->distanceCombo);
  simQt::UnitsComboBox::addAltitudeUnits(*ui_->altitudeCombo);
  simQt::UnitsComboBox::addSpeedUnits(*ui_->speedCombo);
  simQt::UnitsComboBox::addCoordinateSystems(*ui_->coordCombo);
  simQt::UnitsComboBox::addVerticalData(*ui_->vertDatumCombo);
  simQt::UnitsComboBox::addMagneticVariances(*ui_->magVarCombo);
  // Orientation angle units
  simQt::UnitsComboBox::addUnits(*ui_->angleCombo, simCore::ANGLE_FAMILY, reg);
  // Geodetic angle units
  simQt::UnitsComboBox::addGeodeticFormats(*ui_->geodeticCombo);

  updateUnitConverter_();

  connect(ui_->distanceCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUnitConverter_()));
  connect(ui_->inputValueSpin, SIGNAL(valueChanged(double)), this, SLOT(updateUnitConverter_()));
}

UnitsComboBoxTest::~UnitsComboBoxTest()
{
  delete ui_;
}

void UnitsComboBoxTest::updateUnitConverter_()
{
  const QVariant currentData = ui_->distanceCombo->itemData(ui_->distanceCombo->currentIndex());
  simCore::Units currentUnits = currentData.value<simCore::Units>();
  const double metersValue = ui_->inputValueSpin->value();
  double convertedValue = simCore::Units::METERS.convertTo(currentUnits, metersValue);

  ui_->lineEdit_2->setText(simCore::buildString("", convertedValue, 8, 6, (" " + currentUnits.abbreviation())).c_str());
}

// Example demonstration of the Units Combo Boxes
int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  UnitsComboBoxTest mainWindow;
  mainWindow.show();

  return app.exec();
}

