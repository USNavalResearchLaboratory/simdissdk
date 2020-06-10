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
#include <QtPlugin>
#include "simCore/Calc/Units.h"
#include "simQt/UnitsComboBox.h"
#include "UnitsSelectorComboBoxPlugin.h"

UnitsSelectorComboBoxPlugin::UnitsSelectorComboBoxPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void UnitsSelectorComboBoxPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool UnitsSelectorComboBoxPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *UnitsSelectorComboBoxPlugin::createWidget(QWidget *parent)
{
  simQt::UnitsSelectorComboBox* rv = new simQt::UnitsSelectorComboBox(parent);
  rv->setUnits(simCore::Units::DEGREES);
  return rv;
}

QString UnitsSelectorComboBoxPlugin::name() const
{
  return "simQt::UnitsSelectorComboBox";
}

QString UnitsSelectorComboBoxPlugin::group() const
{
  return "simQt";
}

QIcon UnitsSelectorComboBoxPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Subject Science.png");
}

QString UnitsSelectorComboBoxPlugin::toolTip() const
{
  return "Select between any type of units.";
}

QString UnitsSelectorComboBoxPlugin::whatsThis() const
{
  return "Select between any type of units.";
}

bool UnitsSelectorComboBoxPlugin::isContainer() const
{
  return false;
}

QString UnitsSelectorComboBoxPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Units Selector ComboBox\">"
    "<widget class=\"simQt::UnitsSelectorComboBox\" name=\"UnitsSelectorComboBox\">\n"
    "</widget>\n"
    "</ui>";
}

QString UnitsSelectorComboBoxPlugin::includeFile() const
{
  return "simQt/UnitsComboBox.h";
}

