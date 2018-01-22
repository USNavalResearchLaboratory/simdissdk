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
#include "simData/DataStore.h"
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
  state_(Qt::Unchecked),
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

Qt::CheckState CategoryTreeItem::childrenState() const
{
  // This isn't really a good state and there's no good answer if we have no children.  Assert
  // and the developer can determine if this IS a good state or not, and update appropriately.
  assert(!childItems_.empty());
  if (childItems_.empty())
    return Qt::PartiallyChecked;
  Qt::CheckState state = childItems_[0]->state();
  for (int k = 1; k < childItems_.count(); ++k)
  {
    if (state != childItems_[k]->state())
      return Qt::PartiallyChecked;
  }
  return state;
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
  // Only should updateNumCheckedChildren_() when we're checked.  Unchecked does nothing
  if (item->state() == Qt::Checked)
    updateNumCheckedChildren_(Qt::Checked);
}

void CategoryTreeItem::removeChild(CategoryTreeItem *item)
{
  const bool wasChecked = (item->state() == Qt::Checked);
  childItems_.removeOne(item);
  delete item;
  // Only decrement if this one was previously checked
  if (wasChecked)
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
  : QSortFilterProxyModel(parent),
    hasAllCategories_(true)
{
}

CategoryProxyModel::~CategoryProxyModel()
{
}

void CategoryProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  // simQt::CategoryTreeModel has a top level "All Categories" item.  This item affects
  // some of the way filtering works.  Detect whether we're using a CategoryTreeModel
  // and change our internal flag appropriately.  Note that another possible choice
  // is simQt::CategoryTreeModel2, which does not have an All Categories item.
  hasAllCategories_ = (dynamic_cast<CategoryTreeModel*>(sourceModel) != NULL);
  QSortFilterProxyModel::setSourceModel(sourceModel);
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
  if (hasAllCategories_ && !sourceParent.isValid())
    return true;

  const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  const QString itemText = index.data(Qt::DisplayRole).toString();

  // include items that pass the filter
  if (itemText.contains(filter_, Qt::CaseInsensitive))
    return true;

  // include items whose parent passes the filter, but not if parent is root "All Categories" item
  if (sourceParent.isValid())
  {
    const QString parentText = sourceParent.data(Qt::DisplayRole).toString();
    // We only care about matching "All Categories" for the old model type
    if (hasAllCategories_)
    {
      if (parentText != ALL_CATEGORIES && parentText.contains(filter_, Qt::CaseInsensitive))
        return true;
    }
    else
    {
      if (parentText.contains(filter_, Qt::CaseInsensitive))
        return true;
    }
  }

  // include items with any children that pass the filter
  const int numChildren = sourceModel()->rowCount(index);
  for (int ii = 0; ii < numChildren; ++ii)
  {
    const QModelIndex childIndex = sourceModel()->index(ii, 0, index);
    // Assertion failure means rowCount() was wrong
    assert(childIndex.isValid());
    const QString childText = childIndex.data(Qt::DisplayRole).toString();
    if (childText.contains(filter_, Qt::CaseInsensitive))
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
      catValItem->setState(valIter->second ? Qt::Checked : Qt::Unchecked);
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
  allCategoriesItem_ = new CategoryTreeItem(ALL_CATEGORIES, ALL_CATEGORIES, -1, rootItem_);
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
  // Only update with a different filter
  if (&categoryFilter == categoryFilter_ || categoryFilter_ == NULL)
    return;

  // Simplify the filter.  This will reduce the total number of operations here.
  simData::CategoryFilter simplified = categoryFilter;
  simplified.simplify();

  // Do a quick check on the current filter and make sure it doesn't match incoming filter.
  // Do it in an artificial scope to prevent temporary filter from lasting long.
  {
    simData::CategoryFilter localSimplified = *categoryFilter_;
    localSimplified.simplify();
    if (localSimplified.getCategoryFilter() == simplified.getCategoryFilter())
      return;
  }


  // If the name is in the simplified filter, that means it's got an influence on the filter state.
  // If it's absent, then it has no influence; GUI should be checked and all values set true,
  // which is the state we're starting with here.  Therefore, we only need to update the filter
  // state for items that exist in the category filter.
  std::vector<int> allNamesVec;
  simplified.getNames(allNamesVec);
  // Convert this into a set for faster find()
  std::set<int> allNamesSet(allNamesVec.begin(), allNamesVec.end());

  // First, update the GUI
  const int numCategories = allCategoriesItem_->childCount();
  for (int categoryIndex = 0; categoryIndex < numCategories; ++categoryIndex)
  {
    CategoryTreeItem* categoryItem = allCategoriesItem_->child(categoryIndex);
    // Assertion fail means failure in CategoryTreeItem::child() or childCount()
    assert(categoryItem);
    if (!categoryItem)
      continue;

    // Determine if the name is in the simplified filter.  If it is, then the item needs
    // to be checked.  If it's not, then the GUI can be fully checked or fully unchecked,
    // since both states are equivalent; don't change anything in that case.
    const bool categoryIsInSimplified = (allNamesSet.find(categoryItem->categoryIndex()) != allNamesSet.end());
    if (!categoryIsInSimplified)
    {
      const Qt::CheckState currentState = categoryItem->childrenState();
      if (currentState == Qt::PartiallyChecked)
        setState_(categoryItem, Qt::Checked);
      continue;
    }

    // Item is in the filter.  It applies in some way.
    categoryItem->setState(Qt::Checked);

    // Get all the checks for this category.  This is a simplified listing and we need to
    // rely on "Unlisted Value" to tell us entries.
    simData::CategoryFilter::ValuesCheck checks;
    simplified.getValues(categoryItem->categoryIndex(), checks);
    // Determine whether Unlisted Values are checked or unchecked
    auto unlistedIter = checks.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
    // Assertion validates that if "Unlisted Value" is present, that it's set to true (a non-default value)
    assert(unlistedIter == checks.end() || unlistedIter->second);
    const bool unlistedValues = (unlistedIter != checks.end());

    // Loop through all children, setting the values correctly
    const int numValues = categoryItem->childCount();
    for (int valueIndex = 0; valueIndex < numValues; ++valueIndex)
    {
      CategoryTreeItem* valueItem = categoryItem->child(valueIndex);
      assert(valueItem); // Implies failure in child() or childCount()

      // Find it in the filter
      const int valueInt = valueItem->categoryIndex();
      auto simpleIter = checks.find(valueInt);
      // Is it present? If so use that value
      if (simpleIter != checks.end())
        valueItem->setState(simpleIter->second ? Qt::Checked : Qt::Unchecked);
      else
      {
        // Else, use the unlisted value that we detected, unless it's the NO VALUE special case (always off by default)
        if (valueInt == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
          valueItem->setState(Qt::Unchecked);
        else
          valueItem->setState(unlistedValues ? Qt::Checked : Qt::Unchecked);
      }
    }
  }

  // Next, update the state of the internal filter to match what the GUI shows
  bool globalCheck = false;
  for (int categoryIndex = 0; categoryIndex < numCategories; ++categoryIndex)
  {
    CategoryTreeItem* categoryItem = allCategoriesItem_->child(categoryIndex);
    if (!categoryItem)
      continue;
    const int nameInt = categoryItem->categoryIndex();
    categoryFilter_->updateCategoryFilterName(nameInt, categoryItem->state() == Qt::Checked);

    // Keep track of the global flag for check/uncheck on top level
    if (categoryItem->state() == Qt::Checked)
      globalCheck = true;

    // Set each child value appropriately
    const int numValues = categoryItem->childCount();
    for (int valueIndex = 0; valueIndex < numValues; ++valueIndex)
    {
      // Check the item in the filter if the GUI item is checked
      CategoryTreeItem* valueItem = categoryItem->child(valueIndex);
      if (valueItem)
        categoryFilter_->setValue(nameInt, valueItem->categoryIndex(), valueItem->state() == Qt::Checked);
    }
  }

  // Synchronize the state on the All Items check
  allCategoriesItem_->setState(globalCheck ? Qt::Checked : Qt::Unchecked);

  // Emit dataChanged() to update the GUI
  emit dataChanged(createIndex(0, 0), createIndex(allCategoriesItem_->childCount(), 0));
}

}
