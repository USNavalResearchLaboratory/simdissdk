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
#include "simQt/CategoryFilterWidget.h"
#include "CategoryFilterWidgetPlugin.h"

CategoryFilterWidgetPlugin::CategoryFilterWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void CategoryFilterWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool CategoryFilterWidgetPlugin::isInitialized() const
{
  return initialized;
}

QWidget *CategoryFilterWidgetPlugin::createWidget(QWidget *parent)
{
  simQt::CategoryFilterWidget* rv = new simQt::CategoryFilterWidget(parent);
  return rv;
}

QString CategoryFilterWidgetPlugin::name() const
{
  return "simQt::CategoryFilterWidget";
}

QString CategoryFilterWidgetPlugin::group() const
{
  return "simQt";
}

QIcon CategoryFilterWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Categorize.png");
}

QString CategoryFilterWidgetPlugin::toolTip() const
{
  return "Filter entities by category";
}

QString CategoryFilterWidgetPlugin::whatsThis() const
{
  return "Filter entities by category";
}

bool CategoryFilterWidgetPlugin::isContainer() const
{
  return false;
}

QString CategoryFilterWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Category Filter Widget\">"
    "<widget class=\"simQt::CategoryFilterWidget\" name=\"categoryFilterWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString CategoryFilterWidgetPlugin::includeFile() const
{
  return "simQt/CategoryFilterWidget.h";
}


