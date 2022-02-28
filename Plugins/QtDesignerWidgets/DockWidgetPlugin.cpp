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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 */

#include <QtCore/QtPlugin>
#include "DockWidgetPlugin.h"
#include "simQt/DockWidget.h"

DockWidgetPlugin::DockWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void DockWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool DockWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *DockWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::DockWidget(parent);
}

QString DockWidgetPlugin::name() const
{
  return "simQt::DockWidget";
}

QString DockWidgetPlugin::group() const
{
  return "simQt";
}

QIcon DockWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Window Dock.png");
}

QString DockWidgetPlugin::toolTip() const
{
  return "A simQt replacement for the QDockWidget";
}

QString DockWidgetPlugin::whatsThis() const
{
  return "A simQt replacement for the QDockWidget";
}

bool DockWidgetPlugin::isContainer() const
{
  return false;
}

QString DockWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"simQt Dock Widget\">"
    "<widget class=\"simQt::DockWidget\" name=\"DockWidget\">\n"
    " <property name=\"geometry\">\n"
    "  <rect>\n"
    "   <x>0</x>\n"
    "   <y>0</y>\n"
    "   <width>18</width>\n"
    "   <height>18</height>\n"
    "  </rect>\n"
    " </property>\n"
    "</widget>\n"
    "</ui>";
}

QString DockWidgetPlugin::includeFile() const
{
  return "simQt/DockWidget.h";
}

