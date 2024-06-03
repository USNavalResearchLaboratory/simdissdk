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
#include "simQt/DirectorySelectorWidget.h"
#include "DirectorySelectorWidgetPlugin.h"

DirectorySelectorWidgetPlugin::DirectorySelectorWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void DirectorySelectorWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool DirectorySelectorWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *DirectorySelectorWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::DirectorySelectorWidget(parent);
}

QString DirectorySelectorWidgetPlugin::name() const
{
  return "simQt::DirectorySelectorWidget";
}

QString DirectorySelectorWidgetPlugin::group() const
{
  return "simQt";
}

QIcon DirectorySelectorWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Folder 1.png");
}

QString DirectorySelectorWidgetPlugin::toolTip() const
{
  return "A combination text field and file browser button for selecting a directory";
}

QString DirectorySelectorWidgetPlugin::whatsThis() const
{
  return "A combination text field and file browser button for selecting a directory";
}

bool DirectorySelectorWidgetPlugin::isContainer() const
{
  return false;
}

QString DirectorySelectorWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Directory Selector\">"
    "<widget class=\"simQt::DirectorySelectorWidget\" name=\"directorySelectorWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString DirectorySelectorWidgetPlugin::includeFile() const
{
  return "simQt/DirectorySelectorWidget.h";
}

