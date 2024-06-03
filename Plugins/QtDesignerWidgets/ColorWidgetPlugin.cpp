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
#include "simQt/ColorWidget.h"
#include "ColorWidgetPlugin.h"

ColorWidgetPlugin::ColorWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void ColorWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool ColorWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *ColorWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::ColorWidget(parent);
}

QString ColorWidgetPlugin::name() const
{
  return "simQt::ColorWidget";
}

QString ColorWidgetPlugin::group() const
{
  return "simQt";
}

QIcon ColorWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/ColorWell.png");
}

QString ColorWidgetPlugin::toolTip() const
{
  return "Qt colorwell that shows color with alpha values against a black and white background.";
}

QString ColorWidgetPlugin::whatsThis() const
{
  return "Qt colorwell that shows color with alpha values against a black and white background.";
}

bool ColorWidgetPlugin::isContainer() const
{
  return false;
}

QString ColorWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Color Well\">"
    "<widget class=\"simQt::ColorWidget\" name=\"colorWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString ColorWidgetPlugin::includeFile() const
{
  return "simQt/ColorWidget.h";
}

