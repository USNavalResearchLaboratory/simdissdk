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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */

#include <QtCore/QtPlugin>
#include "FontWidgetPlugin.h"
#include "simQt/FontWidget.h"

FontWidgetPlugin::FontWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void FontWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool FontWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *FontWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::FontWidget(parent);
}

QString FontWidgetPlugin::name() const
{
  return "simQt::FontWidget";
}

QString FontWidgetPlugin::group() const
{
  return "simQt";
}

QIcon FontWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Fonts_Select.png");
}

QString FontWidgetPlugin::toolTip() const
{
  return "A Qt selector for font files";
}

QString FontWidgetPlugin::whatsThis() const
{
  return "A Qt selector for font files";
}

bool FontWidgetPlugin::isContainer() const
{
  return false;
}

QString FontWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Font Selector\">"
    "<widget class=\"simQt::FontWidget\" name=\"fontWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString FontWidgetPlugin::includeFile() const
{
  return "simQt/FontWidget.h";
}

