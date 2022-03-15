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
#include <QtCore/QtPlugin>
#include "simQt/EntityLineEdit.h"
#include "EntityLineEditPlugin.h"

EntityLineEditPlugin::EntityLineEditPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void EntityLineEditPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool EntityLineEditPlugin::isInitialized() const
{
  return initialized;
}

QWidget *EntityLineEditPlugin::createWidget(QWidget *parent)
{
  simQt::EntityLineEdit* rv = new simQt::EntityLineEdit(parent, nullptr);
  return rv;
}

QString EntityLineEditPlugin::name() const
{
  return "simQt::EntityLineEdit";
}

QString EntityLineEditPlugin::group() const
{
  return "simQt";
}

QIcon EntityLineEditPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Text_Selection.png");
}

QString EntityLineEditPlugin::toolTip() const
{
  return "Entity line edit with auto complete";
}

QString EntityLineEditPlugin::whatsThis() const
{
  return "Entity line edit with auto complete";
}

bool EntityLineEditPlugin::isContainer() const
{
  return false;
}

QString EntityLineEditPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Entity Name Line Edit\">"
    "<widget class=\"simQt::EntityLineEdit\" name=\"entityLineEdit\">\n"
    "</widget>\n"
    "</ui>";
}

QString EntityLineEditPlugin::includeFile() const
{
  return "simQt/EntityLineEdit.h";
}



