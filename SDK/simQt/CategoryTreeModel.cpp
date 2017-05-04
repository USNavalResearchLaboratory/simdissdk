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
#include <QString>

#include "simData/DataStoreHelpers.h"
#include "simQt/CategoryTreeModel.h"

namespace simQt {

static const QString ALL_CATEGORIES = "All Categories";

/// Monitors for category data changes
class CategoryTreeModel::CategoryFilterListener : public simData::CategoryNameManager::Listener
{
public:
  /// Constructor
  explicit CategoryFilterListener(CategoryTreeModel* parent) : parent_(parent)
  {
  }

  virtual ~CategoryFilterListener()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex)
  {
    parent_->addCategoryName_(categoryIndex);
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex)
  {
    parent_->addCategoryValue_(categoryIndex, valueIndex, true);
  }

  /// Invoked when all data is cleared
  virtual void onClear()
  {
    parent_->clear_();
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing()
  {
    // noop
  }

private:
  CategoryTreeModel* parent_;
};

//----------------------------------------------------------------------------
CategoryTreeItem::CategoryTreeItem(const QString& text, const QString& sortText, int categoryIndex, CategoryTreeItem *parent)
  : text_(text),
    sortText_(sortText),
    categoryIndex_(categoryIndex),
    parentItem_(parent),
    state_(Qt::Checked),
    numCheckedChildren_(0)
{
}

CategoryTreeItem::~CategoryTreeItem()
{
  qDeleteAll(childItems_);
}

QString CategoryTreeItem::text() const
{
  return text_;
}

QString CategoryTreeItem::sortText() const
{
  return sortText_;
}

int CategoryTreeItem::categoryIndex() const
{
  return categoryIndex_;
}

Qt::CheckState CategoryTreeItem::state() const
{
  return state_;
}

void CategoryTreeItem::setState(Qt::CheckState value)
{
  // don't bother to update if no change
  if (state_ == value)
    return;
  state_ = value;
  // let the parent know about our updated state
  if (parentItem_ != NULL)
    parentItem_->updateNumCheckedChildren_(value);
}

void CategoryTreeItem::updateNumCheckedChildren_(Qt::CheckState value)
{
  // increment if a child has been checked
  if (value == Qt::Checked)
  {
    // moving from unchecked to checked state, since one of our children is now checked
    if (numCheckedChildren_ == 0)
      setState(Qt::Checked);
    ++numCheckedChildren_;
  }
  // decrement down to 0 if a child has been unchecked
  else if (numCheckedChildren_ > 0)
  {
    --numCheckedChildren_;
    // moving from checked to unchecked, since none of our children are now checked
    if (numCheckedChildren_ == 0)
      setState(Qt::Unchecked);
  }
}

void CategoryTreeItem::appendChild(CategoryTreeItem *item)
{
  childItems_.append(item);
  updateNumCheckedChildren_(item->state());
}

void CategoryTreeItem::removeChild(CategoryTreeItem *item)
{
  childItems_.removeOne(item);
  delete item;
  updateNumCheckedChildren_(Qt::Unchecked);
}

CategoryTreeItem* CategoryTreeItem::child(int row)
{
  return childItems_.value(row);
}

int CategoryTreeItem::childCount() const
{
  return childItems_.count();
}

CategoryTreeItem* CategoryTreeItem::parent()
{
  return parentItem_;
}

int CategoryTreeItem::row() const
{
  if (parentItem_)
    return parentItem_->childItems_.indexOf(const_cast<CategoryTreeItem*>(this));

  return 0;
}

//-----------------------------------------------------------------------------------------

CategoryProxyModel::CategoryProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
}

CategoryProxyModel::~CategoryProxyModel()
{
}

void CategoryProxyModel::resetFilter()
{
  invalidateFilter();
}

void CategoryProxyModel::setFilterText(const QString& filter)
{
  if (filter_ == filter)
    return;

  filter_ = filter;
  invalidateFilter();
}

bool CategoryProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  if (filter_.isEmpty())
    return true;

  // Always accept top level "All Categories" item
  if (!sourceParent.isValid())
    return true;

  QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());

  // include items that pass the filter
  if (item->text().contains(filter_, Qt::CaseInsensitive))
    return true;

  // include items whose parent passes the filter, but not if parent is root "All Categories" item
  if (item->parent() != NULL && item->parent()->text() != ALL_CATEGORIES && item->parent()->text().contains(filter_, Qt::CaseInsensitive))
    return true;

  // include items with any children that pass the filter
  for (int ii = 0; ii < item->childCount(); ++ii)
  {
    if (item->child(ii)->text().contains(filter_, Qt::CaseInsensitive))
      return true;
  }

  return false;
}

//-----------------------------------------------------------------------------------------

CategoryTreeModel::CategoryTreeModel(QObject *parent, simData::DataStore* dataStore, simData::CategoryFilter* categoryFilter)
  : QAbstractItemModel(parent),
    rootItem_(NULL),
    dataStore_(NULL),
    categoryFilter_(NULL)
{
  // create observers/listeners
  listenerPtr_ = simData::CategoryNameManager::ListenerPtr(new CategoryFilterListener(this));

  // setting the data store will register our observer and listener
  setProviders(dataStore, categoryFilter);
}

CategoryTreeModel::~CategoryTreeModel()
{
  setProviders(NULL, NULL);
  delete rootItem_;
}

void CategoryTreeModel::setProviders(simData::DataStore* dataStore, simData::CategoryFilter* categoryFilter)
{
  if (dataStore == dataStore_)
    return;

  // Remove the prefs observers on the data store
  if (dataStore_)
    dataStore_->categoryNameManager().removeListener(listenerPtr_);

  // Update the pointers
  dataStore_ = dataStore;
  categoryFilter_ = categoryFilter;

  // re-add the prefs observers
  if (dataStore_)
    dataStore->categoryNameManager().addListener(listenerPtr_);

  // fill the tree model
  forceRefresh();
}

//nameIndex is a unique index representing the name, managed by the Category Name Manager.
CategoryTreeItem* CategoryTreeModel::findCategoryName_(int nameIndex)
{
  for (int ii = 0; ii < allCategoriesItem_->childCount(); ii++)
  {
    CategoryTreeItem* child = allCategoriesItem_->child(ii);
    if (child->categoryIndex() == nameIndex)
      return child;
  }

  return NULL;
}

//valueIndex is a unique index representing the value, managed by the Category Name Manager.
CategoryTreeItem* CategoryTreeModel::findCategoryValue_(CategoryTreeItem* parent, int valueIndex)
{
  if (parent == NULL)
    return NULL;

  for (int ii = 0; ii < parent->childCount(); ii++)
  {
    CategoryTreeItem* child = parent->child(ii);
    if (child->categoryIndex() == valueIndex)
      return child;
  }

  return NULL;
}

void CategoryTreeModel::addCategoryName_(int nameIndex)
{
  // must call setProviders first
  assert(categoryFilter_ != NULL);

  // Prevent duplication
  if (findCategoryName_(nameIndex) != NULL)
    return;

  const simData::CategoryFilter::CategoryCheck& categoryCheck = categoryFilter_->getCategoryFilter();
  simData::CategoryFilter::CategoryCheck::const_iterator iter = categoryCheck.find(nameIndex);
  assert(iter != categoryCheck.end());
  if (iter != categoryCheck.end())
  {
    QString name = QString::fromStdString(dataStore_->categoryNameManager().nameIntToString(iter->first));
    CategoryTreeItem* catNameItem = new CategoryTreeItem(name, name, iter->first, allCategoriesItem_);
    beginInsertRows(createIndex(allCategoriesItem_->row(), 0, allCategoriesItem_), allCategoriesItem_->childCount(), allCategoriesItem_->childCount());
    allCategoriesItem_->appendChild(catNameItem);
    endInsertRows();
    for (simData::CategoryFilter::ValuesCheck::const_iterator valIter = iter->second.second.begin(); valIter != iter->second.second.end(); ++valIter)
      addCategoryValue_(iter->first, valIter->first, false);
  }
}

void CategoryTreeModel::addCategoryValue_(int nameIndex, int valueIndex, bool rebuild)
{
  // Find Category Name in GUI tree
  CategoryTreeItem* categoryName = findCategoryName_(nameIndex);
  if (categoryName == NULL)
  {
    // Add Category name to GUI tree
    addCategoryName_(nameIndex);
    categoryName = findCategoryName_(nameIndex);
  }

  // Prevent duplicates
  if (findCategoryValue_(categoryName, valueIndex))
  {
    // The CategoryTreeModel is out of sync
    assert(false);
    return;
  }

  const simData::CategoryFilter::CategoryCheck& categoryCheck = categoryFilter_->getCategoryFilter();
  simData::CategoryFilter::CategoryCheck::const_iterator iter = categoryCheck.find(nameIndex);
  assert(iter != categoryCheck.end());
  if (iter != categoryCheck.end())
  {
    // Found category so look for value
    simData::CategoryFilter::ValuesCheck::const_iterator valIter = iter->second.second.find(valueIndex);
    assert(valIter != iter->second.second.end());
    if (valIter != iter->second.second.end())
    {
      QString value = QString::fromStdString(dataStore_->categoryNameManager().valueIntToString(valIter->first));
      CategoryTreeItem* catValItem = new CategoryTreeItem(value, valueSortName_(value), valIter->first, categoryName);
      catValItem->setState(iter->second.first ? Qt::Checked : Qt::Unchecked);
      beginInsertRows(createIndex(categoryName->row(), 0, categoryName), categoryName->childCount(), categoryName->childCount());
      categoryName->appendChild(catValItem);
      endInsertRows();
    }
  }
}

void CategoryTreeModel::clear_()
{
  // must call setProviders first
  assert(categoryFilter_ != NULL);
  buildTree_();
}

void CategoryTreeModel::forceRefresh()
{
  if (dataStore_ != NULL)
    buildTree_();
}

int CategoryTreeModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

QVariant CategoryTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if ((role == Qt::DisplayRole) && (index.column() == 0))
  {
    CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());
    return item->text();
  }

  if ((role == CategoryTreeModel::SortRole) && (index.column() == 0))
  {
    CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());
    return item->sortText();
  }

  if ((role == Qt::CheckStateRole) && (index.column() == 0))
  {
    CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());
    return item->state();
  }

  if ((role == Qt::ToolTipRole) && (index.column() == 0))
  {
    CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());
    if (item->childCount() > 0)
    {
      return QString(tr("# of Values: %1")).arg(item->childCount());
    }
  }

  return QVariant();
}

int CategoryTreeModel::setState_(CategoryTreeItem* item, Qt::CheckState state)
{
  int rv = (item->state() == state) ? 0 : 1;
  item->setState(state);
  for (int ii = 0; ii < item->childCount(); ++ii)
    rv += setState_(item->child(ii), state);
  return rv;
}

bool CategoryTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if ((index.column() == 0) && (role == Qt::CheckStateRole))
  {
    CategoryTreeItem *item = static_cast<CategoryTreeItem*>(index.internalPointer());
    Qt::CheckState state = value.toBool() ? Qt::Checked : Qt::Unchecked;
    if (setState_(item, state) > 0)
    {
      emit dataChanged(index, index);
      // need to update the category filter
      if (item->parent() == rootItem_)
        categoryFilter_->updateAll(value.toBool());
      else if (item->parent() == allCategoriesItem_)
        categoryFilter_->updateCategoryFilterName(item->categoryIndex(), value.toBool());
      else
        categoryFilter_->updateCategoryFilterValue(item->parent()->categoryIndex(), item->categoryIndex(), value.toBool());
      emit(categoryFilterChanged(*categoryFilter_));
    }
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags CategoryTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  if (index.column() == 0)
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  CategoryTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<CategoryTreeItem*>(parent.internalPointer());

  CategoryTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

QString CategoryTreeModel::text(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  CategoryTreeItem *childItem = static_cast<CategoryTreeItem*>(index.internalPointer());
  if (childItem == NULL)
    return "";

  return childItem->text();
}

QModelIndex CategoryTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  CategoryTreeItem *childItem = static_cast<CategoryTreeItem*>(index.internalPointer());
  if (childItem == NULL)
    return QModelIndex();

  CategoryTreeItem *parentItem = childItem->parent();

  if (parentItem == NULL)
    return QModelIndex();

  if (parentItem == rootItem_)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int CategoryTreeModel::rowCount(const QModelIndex &parent) const
{
  if (rootItem_ == NULL)
    return 0;

  CategoryTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<CategoryTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

void CategoryTreeModel::buildTree_()
{
  beginResetModel();
  delete rootItem_;
  rootItem_ = new CategoryTreeItem("Root", "Root", 0, NULL);
  allCategoriesItem_ = new CategoryTreeItem(ALL_CATEGORIES, ALL_CATEGORIES , - 1, rootItem_);
  rootItem_->appendChild(allCategoriesItem_);

  if (categoryFilter_ != NULL)
  {
    const simData::CategoryFilter::CategoryCheck& categoryCheck = categoryFilter_->getCategoryFilter();
    for (simData::CategoryFilter::CategoryCheck::const_iterator iter = categoryCheck.begin(); iter != categoryCheck.end(); ++iter)
    {
      QString name = QString::fromStdString(dataStore_->categoryNameManager().nameIntToString(iter->first));
      CategoryTreeItem* catNameItem = new CategoryTreeItem(name, name, iter->first, allCategoriesItem_);
      allCategoriesItem_->appendChild(catNameItem);
      addAllValues_(iter->second.second, catNameItem, dataStore_->categoryNameManager());
    }
  }

  endResetModel();
}

void CategoryTreeModel::addAllValues_(const simData::CategoryFilter::ValuesCheck& iter, CategoryTreeItem* nameItem, const simData::CategoryNameManager& categoryNameManager)
{
  for (simData::CategoryFilter::ValuesCheck::const_iterator valIter = iter.begin(); valIter != iter.end(); ++valIter)
  {
    QString value = QString::fromStdString(categoryNameManager.valueIntToString(valIter->first));
    CategoryTreeItem* catValItem = new CategoryTreeItem(value, valueSortName_(value), valIter->first, nameItem);
    nameItem->appendChild(catValItem);
  }
}

QString CategoryTreeModel::valueSortName_(const QString& value) const
{
  if (value == "Unlisted Value")
    return " ";

  if (value == "No Value")
    return "  ";

  return value;
}

/**
  * The passed in categoryFilter may have a subset of the category information.
  * If categoryFilter is missing a category name the GUI should show checked for everything
  * If categoryFilter is missing a category value the GUI should show the "Unlisted" state
  */
void CategoryTreeModel::setFilter(const simData::CategoryFilter& categoryFilter)
{
  bool globalAnyChecked = false;

  // set all category values to true
  categoryFilter_->updateAll(true);
  setState_(allCategoriesItem_, Qt::Checked);

  // now update all found values to those in the passed in filter
  const simData::CategoryFilter::CategoryCheck& categoryCheck = categoryFilter.getCategoryFilter();
  for (simData::CategoryFilter::CategoryCheck::const_iterator iter = categoryCheck.begin(); iter != categoryCheck.end(); ++iter)
  {
    bool localAnyChecked = false;
    CategoryTreeItem* catItem = findCategoryName_(iter->first);

    // set everything to unlisted then update with actual values
    auto value = iter->second.second.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
    if (value != iter->second.second.end())
      localAnyChecked = value->second;

    for (int ii = 0; ii < catItem->childCount(); ++ii)
      catItem->child(ii)->setState(localAnyChecked ? Qt::Checked : Qt::Unchecked);

    // Update all values for the name
    for (simData::CategoryFilter::ValuesCheck::const_iterator valIter = iter->second.second.begin(); valIter != iter->second.second.end(); ++valIter)
    {
      CategoryTreeItem* valItem = findCategoryValue_(catItem, valIter->first);
      if (valItem != NULL)
        valItem->setState(valIter->second ? Qt::Checked : Qt::Unchecked);
      categoryFilter_->updateCategoryFilterValue(iter->first, valIter->first, valIter->second);
      if (valIter->second)
      {
        globalAnyChecked = true;
        localAnyChecked = true;
      }
    }

    if (localAnyChecked == false)
      catItem->setState(Qt::Unchecked);
  }

  if (globalAnyChecked == false)
    allCategoriesItem_->setState(Qt::Unchecked);

  emit dataChanged(createIndex(0, 0), createIndex(allCategoriesItem_->childCount(), 0));
}

}
