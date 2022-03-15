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
#include "simQt/ColorGradientWidget2.h"
#include "ColorGradientWidget2Plugin.h"

ColorGradientWidget2Plugin::ColorGradientWidget2Plugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void ColorGradientWidget2Plugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool ColorGradientWidget2Plugin::isInitialized() const
{
  return initialized_;
}

QWidget *ColorGradientWidget2Plugin::createWidget(QWidget *parent)
{
  return new simQt::ColorGradientWidget2(parent);
}

QString ColorGradientWidget2Plugin::name() const
{
  return "simQt::ColorGradientWidget2";
}

QString ColorGradientWidget2Plugin::group() const
{
  return "simQt";
}

QIcon ColorGradientWidget2Plugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Color_Scales.png");
}

QString ColorGradientWidget2Plugin::toolTip() const
{
  return "Qt widget that enables customization of a multi-stop color gradient.";
}

QString ColorGradientWidget2Plugin::whatsThis() const
{
  return "Qt widget that enables customization of a multi-stop color gradient.";
}

bool ColorGradientWidget2Plugin::isContainer() const
{
  return false;
}

QString ColorGradientWidget2Plugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Color Gradient Widget 2\">"
    "<widget class=\"simQt::ColorGradientWidget2\" name=\"colorGradientWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString ColorGradientWidget2Plugin::includeFile() const
{
  return "simQt/ColorGradientWidget2.h";
}

