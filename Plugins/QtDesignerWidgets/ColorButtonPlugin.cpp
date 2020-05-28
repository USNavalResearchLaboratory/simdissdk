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
#include "simQt/ColorButton.h"
#include "ColorButtonPlugin.h"

ColorButtonPlugin::ColorButtonPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void ColorButtonPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool ColorButtonPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *ColorButtonPlugin::createWidget(QWidget *parent)
{
  return new simQt::ColorButton(parent);
}

QString ColorButtonPlugin::name() const
{
  return "simQt::ColorButton";
}

QString ColorButtonPlugin::group() const
{
  return "simQt";
}

QIcon ColorButtonPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/ColorWell.png");
}

QString ColorButtonPlugin::toolTip() const
{
  return "Qt colored pushbutton that shows color with alpha values blended against a black and white background.";
}

QString ColorButtonPlugin::whatsThis() const
{
  return "Qt colored pushbutton that shows color with alpha values blended against a black and white background.";
}

bool ColorButtonPlugin::isContainer() const
{
  return false;
}

QString ColorButtonPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Color Push Button\">"
    "<widget class=\"simQt::ColorButton\" name=\"colorButton\">\n"
    "</widget>\n"
    "</ui>";
}

QString ColorButtonPlugin::includeFile() const
{
  return "simQt/ColorButton.h";
}

