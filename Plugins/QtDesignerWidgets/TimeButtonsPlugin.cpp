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
#include <QtCore/QtPlugin>
#include "simQt/TimeButtons.h"
#include "TimeButtonsPlugin.h"

TimeButtonsPlugin::TimeButtonsPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void TimeButtonsPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool TimeButtonsPlugin::isInitialized() const
{
  return initialized;
}

QWidget *TimeButtonsPlugin::createWidget(QWidget *parent)
{
  return new simQt::TimeButtons(parent);
}

QString TimeButtonsPlugin::name() const
{
  return "simQt::TimeButtons";
}

QString TimeButtonsPlugin::group() const
{
  return "simQt";
}

QIcon TimeButtonsPlugin::icon() const
{
  return QIcon(":/simQt/images/Navigation Blue Next.png");
}

QString TimeButtonsPlugin::toolTip() const
{
  return "Time editing buttons in a horizontal frame.";
}

QString TimeButtonsPlugin::whatsThis() const
{
  return "Time editing buttons in a horizontal frame";
}

bool TimeButtonsPlugin::isContainer() const
{
  return false;
}

QString TimeButtonsPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Time Control Buttons\">"
    "<widget class=\"simQt::TimeButtons\" name=\"timeButtons\">\n"
    "</widget>\n"
    "</ui>";
}

QString TimeButtonsPlugin::includeFile() const
{
  return "simQt/TimeButtons.h";
}

