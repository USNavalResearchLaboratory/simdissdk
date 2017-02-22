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
* License for source code at https://simdis.nrl.navy.mil/License.aspx
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
*
*/
#include <QtCore/QtPlugin>
#include "simQt/EntityFilterLineEdit.h"
#include "EntityFilterLineEditPlugin.h"

EntityFilterLineEditPlugin::EntityFilterLineEditPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void EntityFilterLineEditPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool EntityFilterLineEditPlugin::isInitialized() const
{
  return initialized;
}

QWidget *EntityFilterLineEditPlugin::createWidget(QWidget *parent)
{
  simQt::EntityFilterLineEdit* rv = new simQt::EntityFilterLineEdit(parent);
  return rv;
}

QString EntityFilterLineEditPlugin::name() const
{
  return "simQt::EntityFilterLineEdit";
}

QString EntityFilterLineEditPlugin::group() const
{
  return "simQt";
}

QIcon EntityFilterLineEditPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Text_Box.png");
}

QString EntityFilterLineEditPlugin::toolTip() const
{
  return "Filter entities by regular expression";
}

QString EntityFilterLineEditPlugin::whatsThis() const
{
  return "Filter entities by regular expression";
}

bool EntityFilterLineEditPlugin::isContainer() const
{
  return false;
}

QString EntityFilterLineEditPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Entity Filter Line Edit\">"
    "<widget class=\"simQt::EntityFilterLineEdit\" name=\"entityFilterLineEdit\">\n"
    "</widget>\n"
    "</ui>";
}

QString EntityFilterLineEditPlugin::includeFile() const
{
  return "simQt/EntityFilterLineEdit.h";
}
