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
#include "simQt/EntityTypeFilterWidget.h"
#include "EntityTypeFilterWidgetPlugin.h"

EntityTypeFilterWidgetPlugin::EntityTypeFilterWidgetPlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void EntityTypeFilterWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool EntityTypeFilterWidgetPlugin::isInitialized() const
{
  return initialized;
}

QWidget *EntityTypeFilterWidgetPlugin::createWidget(QWidget *parent)
{
  simQt::EntityTypeFilterWidget* rv = new simQt::EntityTypeFilterWidget(parent);
  return rv;
}

QString EntityTypeFilterWidgetPlugin::name() const
{
  return "simQt::EntityTypeFilterWidget";
}

QString EntityTypeFilterWidgetPlugin::group() const
{
  return "simQt";
}

QIcon EntityTypeFilterWidgetPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Objects.png");
}

QString EntityTypeFilterWidgetPlugin::toolTip() const
{
  return "Filter entities by entity type";
}

QString EntityTypeFilterWidgetPlugin::whatsThis() const
{
  return "Filter entities by entity type";
}

bool EntityTypeFilterWidgetPlugin::isContainer() const
{
  return false;
}

QString EntityTypeFilterWidgetPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Entity Type Filter\">"
    "<widget class=\"simQt::EntityTypeFilterWidget\" name=\"entityTypeFilterWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString EntityTypeFilterWidgetPlugin::includeFile() const
{
  return "simQt/EntityTypeFilterWidget.h";
}


