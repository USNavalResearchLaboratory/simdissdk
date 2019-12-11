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
#ifdef USE_DEPRECATED_SIMDISSDK_API

#include <QtCore/QtPlugin>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/MemoryDataStore.h"
#include "simQt/CategoryFilterWidget.h"
#include "CategoryFilterWidget2Plugin.h"
#include "CategoryFilterWidgetPlugin.h"

CategoryFilterWidgetPlugin::CategoryFilterWidgetPlugin(QObject *parent)
  : QObject(parent),
    dataStore_(NULL)
{
}

CategoryFilterWidgetPlugin::~CategoryFilterWidgetPlugin()
{
  delete dataStore_;
}

void CategoryFilterWidgetPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (dataStore_)
    return;
  dataStore_ = new simData::MemoryDataStore;
  CategoryFilterWidget2Plugin::createDefaultCategories(*dataStore_);
}

bool CategoryFilterWidgetPlugin::isInitialized() const
{
  return dataStore_ != NULL;
}

QWidget *CategoryFilterWidgetPlugin::createWidget(QWidget *parent)
{
  simQt::CategoryFilterWidget* rv = new simQt::CategoryFilterWidget(parent);

  // Create the data store, adding default categories
  initialize(NULL);
  rv->setProviders(dataStore_);

  // Create a filter for user to see
  simData::CategoryNameManager& nameManager = dataStore_->categoryNameManager();
  simData::CategoryFilter filter(dataStore_);

  // Affinity: Friendly entities only
  const int affinityName = nameManager.addCategoryName("Affinity");
  filter.setValue(affinityName, nameManager.addCategoryValue(affinityName, "Friendly"), true);
  // Platform Type: Unlisted values on; ignore Ship and Submarine
  const int platformTypeName = nameManager.addCategoryName("Platform Type");
  filter.setValue(platformTypeName, nameManager.addCategoryValue(platformTypeName, "Submarine"), false);
  filter.setValue(platformTypeName, nameManager.addCategoryValue(platformTypeName, "Surface Ship"), false);
  filter.setValue(platformTypeName, simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE, true);
  rv->setFilter(filter);

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
  return toolTip();
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
#endif // USE_DEPRECATED_SIMDISSDK_API
