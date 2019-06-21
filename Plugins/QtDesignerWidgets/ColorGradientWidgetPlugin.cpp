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
#include "simQt/ColorGradientWidget.h"
#include "ColorGradientWidgetPlugin.h"

ColorGradientWidgetPlugin::ColorGradientWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void ColorGradientWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool ColorGradientWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *ColorGradientWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::ColorGradientWidget(parent);
}

QString ColorGradientWidgetPlugin::name() const
{
  return "simQt::ColorGradientWidget";
}

QString ColorGradientWidgetPlugin::group() const
{
  return "simQt";
}

QIcon ColorGradientWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Color_Scales.png");
}

QString ColorGradientWidgetPlugin::toolTip() const
{
  return "Qt widget that enables customization of a multi-stop color gradient.";
}

QString ColorGradientWidgetPlugin::whatsThis() const
{
  return "Qt widget that enables customization of a multi-stop color gradient.";
}

bool ColorGradientWidgetPlugin::isContainer() const
{
  return false;
}

QString ColorGradientWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Color Gradient Widget\">"
    "<widget class=\"simQt::ColorGradientWidget\" name=\"colorGradientWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString ColorGradientWidgetPlugin::includeFile() const
{
  return "simQt/ColorGradientWidget.h";
}

