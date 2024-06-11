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
#include "simQt/FileSelectorWidget.h"
#include "FileSelectorWidgetPlugin.h"

FileSelectorWidgetPlugin::FileSelectorWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void FileSelectorWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool FileSelectorWidgetPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *FileSelectorWidgetPlugin::createWidget(QWidget *parent)
{
  return new simQt::FileSelectorWidget(parent);
}

QString FileSelectorWidgetPlugin::name() const
{
  return "simQt::FileSelectorWidget";
}

QString FileSelectorWidgetPlugin::group() const
{
  return "simQt";
}

QIcon FileSelectorWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Folder 1.png");
}

QString FileSelectorWidgetPlugin::toolTip() const
{
  return "A combination text field and file browser button";
}

QString FileSelectorWidgetPlugin::whatsThis() const
{
  return "A combination text field and file browser button";
}

bool FileSelectorWidgetPlugin::isContainer() const
{
  return false;
}

QString FileSelectorWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"File Selector\">"
    "<widget class=\"simQt::FileSelectorWidget\" name=\"fileSelectorWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString FileSelectorWidgetPlugin::includeFile() const
{
  return "simQt/FileSelectorWidget.h";
}

