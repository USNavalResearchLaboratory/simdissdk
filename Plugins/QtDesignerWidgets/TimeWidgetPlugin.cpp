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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QtCore/QtPlugin>
#include "simQt/TimeWidget.h"
#include "TimeWidgetPlugin.h"

TimeWidgetPlugin::TimeWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void TimeWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool TimeWidgetPlugin::isInitialized() const
{
  return initialized;
}

QWidget *TimeWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::TimeWidget(parent);
}

QString TimeWidgetPlugin::name() const
{
  return "simQt::TimeWidget";
}

QString TimeWidgetPlugin::group() const
{
  return "simQt";
}

QIcon TimeWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Calendar_Appointment.png");
}

QString TimeWidgetPlugin::toolTip() const
{
  return "Time widget that supports SIMDIS SDK time formats";
}

QString TimeWidgetPlugin::whatsThis() const
{
  return "Time widget that supports SIMDIS SDK time formats";
}

bool TimeWidgetPlugin::isContainer() const
{
  return false;
}

QString TimeWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Time Edit Widget\">"
    "<widget class=\"simQt::TimeWidget\" name=\"timeWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString TimeWidgetPlugin::includeFile() const
{
  return "simQt/TimeWidget.h";
}

