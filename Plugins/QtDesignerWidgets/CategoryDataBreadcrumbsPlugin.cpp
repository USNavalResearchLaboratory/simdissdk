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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QtCore/QtPlugin>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/MemoryDataStore.h"
#include "simQt/CategoryDataBreadcrumbs.h"
#include "CategoryDataBreadcrumbsPlugin.h"

CategoryDataBreadcrumbsPlugin::CategoryDataBreadcrumbsPlugin(QObject *parent)
  : QObject(parent),
    dataStore_(nullptr)
{
}

CategoryDataBreadcrumbsPlugin::~CategoryDataBreadcrumbsPlugin()
{
  delete dataStore_;
}

void CategoryDataBreadcrumbsPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (dataStore_)
    return;
  dataStore_ = new simData::MemoryDataStore;
}

bool CategoryDataBreadcrumbsPlugin::isInitialized() const
{
  return dataStore_ != nullptr;
}

QWidget *CategoryDataBreadcrumbsPlugin::createWidget(QWidget *parent)
{
  simQt::CategoryDataBreadcrumbs* rv = new simQt::CategoryDataBreadcrumbs(parent);

  // Create a filter for user to see
  if (dataStore_ == nullptr)
    dataStore_ = new simData::MemoryDataStore;
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

QString CategoryDataBreadcrumbsPlugin::name() const
{
  return "simQt::CategoryDataBreadcrumbs";
}

QString CategoryDataBreadcrumbsPlugin::group() const
{
  return "simQt";
}

QIcon CategoryDataBreadcrumbsPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Toaster.png");
}

QString CategoryDataBreadcrumbsPlugin::toolTip() const
{
  return "Breadcrumb display for a category data filter.";
}

QString CategoryDataBreadcrumbsPlugin::whatsThis() const
{
  return toolTip();
}

bool CategoryDataBreadcrumbsPlugin::isContainer() const
{
  return false;
}

QString CategoryDataBreadcrumbsPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Category Data Breadcrumbs\">"
    "<widget class=\"simQt::CategoryDataBreadcrumbs\" name=\"breadcrumbs\">\n"
    "</widget>\n"
    "</ui>";
}

QString CategoryDataBreadcrumbsPlugin::includeFile() const
{
  return "simQt/CategoryDataBreadcrumbs.h";
}
