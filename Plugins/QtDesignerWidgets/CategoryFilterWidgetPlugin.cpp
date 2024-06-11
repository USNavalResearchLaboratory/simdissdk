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
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/MemoryDataStore.h"
#include "simQt/CategoryFilterWidget.h"
#include "simQt/CategoryTreeModel.h"
#include "CategoryFilterWidgetPlugin.h"

CategoryFilterWidgetPlugin::CategoryFilterWidgetPlugin(QObject *parent)
  : QObject(parent),
    dataStore_(nullptr)
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
  CategoryFilterWidgetPlugin::createDefaultCategories(*dataStore_);
}

bool CategoryFilterWidgetPlugin::isInitialized() const
{
  return dataStore_ != nullptr;
}

QWidget *CategoryFilterWidgetPlugin::createWidget(QWidget *parent)
{
  simQt::CategoryFilterWidget* rv = new simQt::CategoryFilterWidget(parent);
  // Create the data store, adding default categories
  initialize(nullptr);
  rv->setDataStore(dataStore_);

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
  return "simQt/CategoryTreeModel.h";
}

void CategoryFilterWidgetPlugin::createDefaultCategories(simData::DataStore& dataStore)
{
  // Add some useful category names for display purposes
  simData::CategoryNameManager& nameManager = dataStore.categoryNameManager();
  const int affinity = nameManager.addCategoryName("Affinity");
  nameManager.addCategoryValue(affinity, "Friendly");
  nameManager.addCategoryValue(affinity, "Hostile");
  nameManager.addCategoryValue(affinity, "Neutral");
  const int platformType = nameManager.addCategoryName("Platform Type");
  nameManager.addCategoryValue(platformType, "Unknown");
  nameManager.addCategoryValue(platformType, "Surface Ship");
  nameManager.addCategoryValue(platformType, "Submarine");
  nameManager.addCategoryValue(platformType, "Aircraft");
  nameManager.addCategoryValue(platformType, "Satellite");
  nameManager.addCategoryValue(platformType, "Helicopter");
  nameManager.addCategoryValue(platformType, "Missile");
  nameManager.addCategoryValue(platformType, "Decoy");
  nameManager.addCategoryValue(platformType, "Buoy");
  nameManager.addCategoryValue(platformType, "Reference Site");
  nameManager.addCategoryValue(platformType, "Land Site");
  nameManager.addCategoryValue(platformType, "Torpedo");
  nameManager.addCategoryValue(platformType, "Contact");
}
