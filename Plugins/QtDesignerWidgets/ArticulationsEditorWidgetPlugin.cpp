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
#include "simQt/ArticulationsEditorWidget.h"
#include "ArticulationsEditorWidgetPlugin.h"

ArticulationsEditorWidgetPlugin::ArticulationsEditorWidgetPlugin(QObject* parent)
  : QObject(parent)
{
}

ArticulationsEditorWidgetPlugin::~ArticulationsEditorWidgetPlugin()
{
}

void ArticulationsEditorWidgetPlugin::initialize(QDesignerFormEditorInterface*)
{
  if (!initialized_)
    initialized_ = true;
}

bool ArticulationsEditorWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget* ArticulationsEditorWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::ArticulationsEditorWidget(parent);
}

QString ArticulationsEditorWidgetPlugin::name() const
{
  return "simQt::ArticulationsEditorWidget";
}

QString ArticulationsEditorWidgetPlugin::group() const
{
  return "simQt";
}

QIcon ArticulationsEditorWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Pivot_Table.png");
}

QString ArticulationsEditorWidgetPlugin::toolTip() const
{
  return "Controls for changing model articulations";
}

QString ArticulationsEditorWidgetPlugin::whatsThis() const
{
  return toolTip();
}

bool ArticulationsEditorWidgetPlugin::isContainer() const
{
  return false;
}

QString ArticulationsEditorWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Articulations Editor Widget\">"
    "<widget class=\"simQt::ArticulationsEditorWidget\" name=\"articulationsEditorWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString ArticulationsEditorWidgetPlugin::includeFile() const
{
  return "simQt/ArticulationsEditorWidget.h";
}
