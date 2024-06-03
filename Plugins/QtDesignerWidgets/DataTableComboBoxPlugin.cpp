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
#include <QtCore/QtPlugin>
#include "simQt/DataTableComboBox.h"
#include "DataTableComboBoxPlugin.h"

DataTableComboBoxPlugin::DataTableComboBoxPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void DataTableComboBoxPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool DataTableComboBoxPlugin::isInitialized() const
{
  return initialized;
}

QWidget *DataTableComboBoxPlugin::createWidget(QWidget *parent)
{
  simQt::DataTableComboBox* rv = new simQt::DataTableComboBox(parent);
  return rv;
}

QString DataTableComboBoxPlugin::name() const
{
  return "simQt::DataTableComboBox";
}

QString DataTableComboBoxPlugin::group() const
{
  return "simQt";
}

QIcon DataTableComboBoxPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Data Blue Table.png");
}

QString DataTableComboBoxPlugin::toolTip() const
{
  return "Display and entity's data tables";
}

QString DataTableComboBoxPlugin::whatsThis() const
{
  return "Display and entity's data tables";
}

bool DataTableComboBoxPlugin::isContainer() const
{
  return false;
}

QString DataTableComboBoxPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Data Table Combo Box\">"
    "<widget class=\"simQt::DataTableComboBox\" name=\"dataTableComboBox\">\n"
    "</widget>\n"
    "</ui>";
}

QString DataTableComboBoxPlugin::includeFile() const
{
  return "simQt/DataTableComboBox.h";
}

