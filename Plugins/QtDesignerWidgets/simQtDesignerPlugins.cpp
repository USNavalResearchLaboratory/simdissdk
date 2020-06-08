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
#include "ColorButtonPlugin.h"
#ifdef HAVE_OSG
#include "ColorGradientWidgetPlugin.h"
#endif
#include "ColorWidgetPlugin.h"
#include "CategoryDataBreadcrumbsPlugin.h"
#include "CategoryFilterWidgetPlugin.h"
#include "DataTableComboBoxPlugin.h"
#include "DirectorySelectorWidgetPlugin.h"
#include "DockWidgetPlugin.h"
#ifdef HAVE_SIMVIS
#include "EntityLineEditPlugin.h"
#endif
#include "EntityTreeCompositePlugin.h"
#include "EntityFilterLineEditPlugin.h"
#include "EntityTypeFilterWidgetPlugin.h"
#include "FileSelectorWidgetPlugin.h"
#include "FontWidgetPlugin.h"
#include "GanttChartViewPlugin.h"
#include "SearchLineEditPlugin.h"
#include "TimeWidgetPlugin.h"
#include "TimeButtonsPlugin.h"
#include "UnitsSelectorComboBoxPlugin.h"
#include "simQtDesignerPlugins.h"

simQtDesignerPlugins::simQtDesignerPlugins(QObject* parent) : QObject(parent)
{
  // Add all plug-in widgets here
  widgetFactories_.append(new CategoryDataBreadcrumbsPlugin(this));
  widgetFactories_.append(new CategoryFilterWidgetPlugin(this));
  widgetFactories_.append(new ColorButtonPlugin(this));
#ifdef HAVE_OSG
  widgetFactories_.append(new ColorGradientWidgetPlugin(this));
#endif
  widgetFactories_.append(new ColorWidgetPlugin(this));
  widgetFactories_.append(new DataTableComboBoxPlugin(this));
  widgetFactories_.append(new DirectorySelectorWidgetPlugin(this));
  widgetFactories_.append(new DockWidgetPlugin(this));
  widgetFactories_.append(new EntityFilterLineEditPlugin(this));
#ifdef HAVE_SIMVIS
  widgetFactories_.append(new EntityLineEditPlugin(this));
#endif
  widgetFactories_.append(new EntityTreeCompositePlugin(this));
  widgetFactories_.append(new EntityTypeFilterWidgetPlugin(this));
  widgetFactories_.append(new FileSelectorWidgetPlugin(this));
  widgetFactories_.append(new FontWidgetPlugin(this));
  widgetFactories_.append(new GanttChartViewPlugin(this));
  widgetFactories_.append(new SearchLineEditPlugin(this));
  widgetFactories_.append(new TimeButtonsPlugin(this));
  widgetFactories_.append(new TimeWidgetPlugin(this));
  widgetFactories_.append(new UnitsSelectorComboBoxPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> simQtDesignerPlugins::customWidgets() const
{
  return widgetFactories_;
}

#if(QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
Q_EXPORT_PLUGIN2(mydesignerplugin, simQtDesignerPlugins)
#endif
