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
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/MemoryDataStore.h"
#include "simQt/CategoryTreeModel2.h"
#include "CategoryFilterWidget2Plugin.h"

CategoryFilterWidget2Plugin::CategoryFilterWidget2Plugin(QObject *parent)
  : QObject(parent),
    dataStore_(NULL)
{
}

CategoryFilterWidget2Plugin::~CategoryFilterWidget2Plugin()
{
  delete dataStore_;
}

void CategoryFilterWidget2Plugin::initialize(QDesignerFormEditorInterface *)
{
  if (dataStore_)
    return;
  dataStore_ = new simData::MemoryDataStore;
  CategoryFilterWidget2Plugin::createDefaultCategories(*dataStore_);
}

bool CategoryFilterWidget2Plugin::isInitialized() const
{
  return dataStore_ != NULL;
}

QWidget *CategoryFilterWidget2Plugin::createWidget(QWidget *parent)
{
  simQt::CategoryFilterWidget2* rv = new simQt::CategoryFilterWidget2(parent);
  // Create the data store, adding default categories
  initialize(NULL);
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

QString CategoryFilterWidget2Plugin::name() const
{
  return "simQt::CategoryFilterWidget2";
}

QString CategoryFilterWidget2Plugin::group() const
{
  return "simQt";
}

QIcon CategoryFilterWidget2Plugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Categorize.png");
}

QString CategoryFilterWidget2Plugin::toolTip() const
{
  return "Filter entities by category";
}

QString CategoryFilterWidget2Plugin::whatsThis() const
{
  return toolTip();
}

bool CategoryFilterWidget2Plugin::isContainer() const
{
  return false;
}

QString CategoryFilterWidget2Plugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Category Filter Widget 2\">"
    "<widget class=\"simQt::CategoryFilterWidget2\" name=\"categoryFilterWidget\">\n"
    "</widget>\n"
    "</ui>";
}

QString CategoryFilterWidget2Plugin::includeFile() const
{
  return "simQt/CategoryTreeModel2.h";
}

void CategoryFilterWidget2Plugin::createDefaultCategories(simData::DataStore& dataStore)
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
