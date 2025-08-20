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
#include <cassert>
#include <QWidget>
#include "simQt/SettingsModel.h"
#include "simQt/SettingsProxyModel.h"

namespace simQt {

namespace {

/** Apply the reg exp filtering for settings search filter, returns true if no regexp filter is set */
#if QT_VERSION_MAJOR == 5
bool testRegExp_(const QAbstractItemModel& sourceModel, const QModelIndex& index0, const QModelIndex& index1, const QModelIndex& parentIndex, const QRegExp& filterText)
#else
bool testRegExp_(const QAbstractItemModel& sourceModel, const QModelIndex& index0, const QModelIndex& index1, const QModelIndex& parentIndex, const QRegularExpression& filterText)
#endif
{
#if QT_VERSION_MAJOR == 5
  if (filterText.isEmpty())
#else
  if (filterText.pattern().isEmpty())
#endif
    return true;
  if (sourceModel.data(index0).toString().contains(filterText) ||
    sourceModel.data(index1).toString().contains(filterText) ||
    sourceModel.data(parentIndex).toString().contains(filterText))
    return true;

  // now search lineage
  QModelIndex ancestor = sourceModel.parent(parentIndex);
  while (ancestor.isValid())
  {
    if (sourceModel.data(ancestor).toString().contains(filterText))
      return true;
    ancestor = sourceModel.parent(ancestor);
  }
  return false;
}

}

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
#if QT_VERSION_MAJOR == 5
  return testRegExp_(*this, index0, index1, sourceParent, filterRegExp());
#else
  return testRegExp_(*this, index0, index1, sourceParent, filterRegularExpression());
#endif
}

void SettingsSearchFilter::setFilterText(const QString& filterText)
{
#if QT_VERSION_MAJOR == 5
  setFilterRegExp(filterText);
#else
  setFilterRegularExpression(filterText);
#endif
  invalidateFilter();
}

QString SettingsSearchFilter::filterText() const
{
#if QT_VERSION_MAJOR == 5
  return filterRegExp().pattern();
#else
  return filterRegularExpression().pattern();
#endif
}

QModelIndexList SettingsSearchFilter::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
  const auto& actualStart = mapToSource(start);

  // Make a copy of the filter's regex to preserve case sensitivity and any other options it may have
#if QT_VERSION_MAJOR == 5
  QRegExp regex = filterRegExp();
#else
  auto regex = filterRegularExpression();
#endif

  regex.setPattern(value.toString());
  QModelIndexList hitsList;
  for (int row = actualStart.row(); row < sourceModel()->rowCount(actualStart.parent()); ++row)
  {
    const QModelIndex candidate = sourceModel()->index(row, 0, actualStart.parent());
    if (testRegExp_(*this, candidate, sourceModel()->index(row, 1, actualStart.parent()), actualStart.parent(), regex))
    {
      hitsList.push_back(mapFromSource(candidate));
      if (hits > 0 && hitsList.size() >= hits)
        return hitsList;
    }

    // Check the children of the candidate index
    for (int i = 0; i < sourceModel()->rowCount(candidate); ++i)
    {
      if (testRegExp_(*this, sourceModel()->index(i, 0, candidate), sourceModel()->index(i, 1, candidate), candidate, regex))
      {
        hitsList.push_back(mapFromSource(sourceModel()->index(i, 0, candidate)));
        if (hits > 0 && hitsList.size() >= hits)
          return hitsList;
      }
    }
  }

  return hitsList;
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
  // Chain so that dataLevel>search>noempty
  dataLevel_ = new SettingsDataLevelFilter(settingsModel, parent);
  search_ = new SettingsSearchFilter(dataLevel_, parent);
  noEmptyFolders_ = new SettingsNoEmptyFoldersFilter(search_, parent);
  setSourceModel(noEmptyFolders_);

  connect(settingsModel, SIGNAL(rowsInserted(QModelIndex, int, int)), dataLevel_, SLOT(invalidate()));
  connect(settingsModel, SIGNAL(rowsRemoved(QModelIndex, int, int)), dataLevel_, SLOT(invalidate()));
  connect(settingsModel, SIGNAL(modelReset()), dataLevel_, SLOT(invalidate()));
}

SettingsProxyModel::~SettingsProxyModel()
{
  delete noEmptyFolders_;
  delete search_;
  delete dataLevel_;
}

void SettingsProxyModel::setFilterText(const QString& filterText)
{
  if (filterText != search_->filterText())
    search_->setFilterText(filterText);
}

QModelIndexList SettingsProxyModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
{
  auto searchList = search_->match(noEmptyFolders_->mapToSource(mapToSource(start)), role, value, hits, flags);
  QModelIndexList rv;
  for (const auto& index : searchList)
    rv.push_back(mapFromSource(noEmptyFolders_->mapFromSource(index)));

  return rv;
}

void SettingsProxyModel::setShowAdvanced(bool showAdvanced)
{
  if (showAdvanced != dataLevel_->showAdvanced())
  {
    dataLevel_->setShowAdvanced(showAdvanced);
    search_->invalidate();
  }
}

void SettingsProxyModel::setShowUnknown(bool showUnknown)
{
  if (showUnknown != dataLevel_->showUnknown())
    dataLevel_->setShowUnknown(showUnknown);
}

}
