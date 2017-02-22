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
#include <cassert>
#include <QWidget>
#include "simQt/SettingsModel.h"
#include "simQt/SettingsProxyModel.h"

namespace simQt {

SettingsSearchFilter::SettingsSearchFilter(QAbstractItemModel* settingsModel, QWidget* parent)
  : QSortFilterProxyModel(parent)
{
  setSourceModel(settingsModel);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool SettingsSearchFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
  // Accept all parent items
  if (sourceModel()->hasChildren(index0))
    return true;
  // Run regexp against children and parent text
  QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
  return testRegExp_(index0, index1, sourceParent);
}

bool SettingsSearchFilter::testRegExp_(const QModelIndex& index0, const QModelIndex& index1, const QModelIndex& parentIndex) const
{
  QRegExp filterText = filterRegExp();
  if (filterText.isEmpty())
    return true;
  if (sourceModel()->data(index0).toString().contains(filterText) ||
    sourceModel()->data(index1).toString().contains(filterText) ||
    sourceModel()->data(parentIndex).toString().contains(filterText))
    return true;
  // now search lineage
  QModelIndex ancestor = sourceModel()->parent(parentIndex);
  while (ancestor.isValid())
  {
    if (sourceModel()->data(ancestor).toString().contains(filterText))
      return true;
    ancestor = sourceModel()->parent(ancestor);
  }
  return false;
}

void SettingsSearchFilter::setFilterText(const QString& filterText)
{
  setFilterRegExp(filterText);
  invalidateFilter();
}

QString SettingsSearchFilter::filterText() const
{
  return filterRegExp().pattern();
}

///////////////////////////////////////////////////////////////////////

SettingsDataLevelFilter::SettingsDataLevelFilter(QAbstractItemModel* settingsModel, QWidget* parent)
  : QSortFilterProxyModel(parent),
    showAdvanced_(false),
    showUnknown_(false)
{
  setSourceModel(settingsModel);
  // insert all the invalid data types not to display in the settings model
  invalidDataTypes_.insert(QVariant::BitArray);
  invalidDataTypes_.insert(QVariant::ByteArray);
  invalidDataTypes_.insert(QVariant::Invalid);
}

bool SettingsDataLevelFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
  // Accept all parent items
  if (sourceModel()->hasChildren(index0))
    return true;
  // Check data level of children
  QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
  simQt::Settings::DataLevel dataLevel = static_cast<simQt::Settings::DataLevel>(sourceModel()->data(index1, SettingsModel::DataLevelRole).toInt());
  // test data level, as well as check that data type is valid
  return testDataLevel_(dataLevel) && invalidDataTypes_.find(sourceModel()->data(index1).type()) == invalidDataTypes_.end();
}

void SettingsDataLevelFilter::setShowAdvanced(bool showAdvanced)
{
  if (this->showAdvanced() != showAdvanced)
  {
    showAdvanced_ = showAdvanced;
    invalidateFilter();
  }
}

bool SettingsDataLevelFilter::showAdvanced() const
{
  return showAdvanced_;
}

void SettingsDataLevelFilter::setShowUnknown(bool showUnknown)
{
  if (this->showUnknown() != showUnknown)
  {
    showUnknown_ = showUnknown;
    invalidateFilter();
  }
}

bool SettingsDataLevelFilter::showUnknown() const
{
  return showUnknown_;
}

bool SettingsDataLevelFilter::testDataLevel_(simQt::Settings::DataLevel dataLevel) const
{
  switch (dataLevel)
  {
  case simQt::Settings::DEFAULT:
    return true;
  case simQt::Settings::PRIVATE:
    return false;
  case simQt::Settings::ADVANCED:
    return showAdvanced();
  case simQt::Settings::UNKNOWN:
    return showUnknown();
  }
  // Assertion failure means new data level not accounted for
  assert(0);
  return false;
}

///////////////////////////////////////////////////////////////////////

SettingsNoEmptyFoldersFilter::SettingsNoEmptyFoldersFilter(QAbstractItemModel* settingsModel, QWidget* parent)
  : QSortFilterProxyModel(parent)
{
  setSourceModel(settingsModel);
  // Any insertion or removal might change whether folder is shown
  connect(sourceModel(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this, SLOT(invalidate()));
  connect(sourceModel(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this, SLOT(invalidate()));
}

bool SettingsNoEmptyFoldersFilter::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  const QModelIndex rowIndex = sourceModel()->index(sourceRow, 0, sourceParent);
  if (!rowIndex.isValid())
    return false;

  if (!hasChildren_(rowIndex))
  {
    // No children -- must be a leaf node.  Either a folder or not a folder
    return !isFolder_(sourceRow, sourceParent);
  }

  // Has children, so:
  //   If all children are hidden, we are hidden
  //   If any one child is visible, we are visible
  const int numChildren = sourceModel()->rowCount(rowIndex);
  for (int k = 0; k < numChildren; ++k)
  {
    if (filterAcceptsRow(k, rowIndex))
      return true;
  }
  return false;
}

bool SettingsNoEmptyFoldersFilter::hasChildren_(const QModelIndex& sourceParent) const
{
  return sourceModel()->hasChildren(sourceParent);
}

bool SettingsNoEmptyFoldersFilter::isFolder_(int sourceRow, const QModelIndex& sourceParent) const
{
  // Folders are not editable in column 1
  QModelIndex idx = sourceModel()->index(sourceRow, 1, sourceParent);
  if (!idx.isValid())
    return false;
  Qt::ItemFlags flags = sourceModel()->flags(idx);
  return !((flags & Qt::ItemIsEditable) || (flags & Qt::ItemIsUserCheckable));
}

///////////////////////////////////////////////////////////////////////

SettingsProxyModel::SettingsProxyModel(QAbstractItemModel* settingsModel, QWidget* parent)
  : QSortFilterProxyModel(parent)
{
  // Chain so that search>dataLevel>noempty
  search_ = new SettingsSearchFilter(settingsModel, parent);
  dataLevel_ = new SettingsDataLevelFilter(search_, parent);
  noEmptyFolders_ = new SettingsNoEmptyFoldersFilter(dataLevel_, parent);
  setSourceModel(noEmptyFolders_);
}

SettingsProxyModel::~SettingsProxyModel()
{
  delete noEmptyFolders_;
  delete dataLevel_;
  delete search_;
}

void SettingsProxyModel::setFilterText(const QString& filterText)
{
  if (filterText != search_->filterText())
  {
    search_->setFilterText(filterText);
    noEmptyFolders_->invalidate();
    invalidateFilter();
  }
}

void SettingsProxyModel::setShowAdvanced(bool showAdvanced)
{
  if (showAdvanced != dataLevel_->showAdvanced())
  {
    dataLevel_->setShowAdvanced(showAdvanced);
    noEmptyFolders_->invalidate();
    invalidateFilter();
  }
}

void SettingsProxyModel::setShowUnknown(bool showUnknown)
{
  if (showUnknown != dataLevel_->showUnknown())
  {
    dataLevel_->setShowUnknown(showUnknown);
    noEmptyFolders_->invalidate();
    invalidateFilter();
  }
}

}
