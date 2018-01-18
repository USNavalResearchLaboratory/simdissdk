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
#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QColor>
#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/DataStore.h"
#include "simQt/QtFormatting.h"
#include "simQt/CategoryTreeModel.h"
#include "simQt/SearchLineEdit.h"
#include "simQt/CategoryTreeModel2.h"

namespace simQt {

/** Lighter than lightGray, matches QPalette::Midlight */
static const QColor MIDLIGHT_BG_COLOR(227, 227, 227);
/** Breadcrumb's default fill color, used here for background brush on filter items that contribute to filter. */
static const QColor CONTRIBUTING_BG_COLOR(195, 225, 240); // Light gray with a hint of blue

/////////////////////////////////////////////////////////////////////////

template <typename T>
IndexedPointerContainer<T>::IndexedPointerContainer()
{
}

template <typename T>
IndexedPointerContainer<T>::~IndexedPointerContainer()
{
  clear();
}

template <typename T>
T* IndexedPointerContainer<T>::operator[](int index) const
{
  return vec_[index];
}

template <typename T>
int IndexedPointerContainer<T>::indexOf(const T* item) const
{
  // Use a const-cast to help find() to use right signature
  const auto i = itemToIndex_.find(const_cast<T*>(item));
  return (i == itemToIndex_.end() ? -1 : i->second);
}

template <typename T>
int IndexedPointerContainer<T>::size() const
{
  return static_cast<int>(vec_.size());
}

template <typename T>
void IndexedPointerContainer<T>::push_back(T* item)
{
  // Don't add the same item twice
  assert(itemToIndex_.find(item) == itemToIndex_.end());
  const int index = size();
  vec_.push_back(item);
  itemToIndex_[item] = index;
}

template <typename T>
void IndexedPointerContainer<T>::clear()
{
  vec_.clear();
  itemToIndex_.clear();
}

template <typename T>
void IndexedPointerContainer<T>::deleteAll()
{
  for (auto i = vec_.begin(); i != vec_.end(); ++i)
    delete *i;
  clear();
}

/////////////////////////////////////////////////////////////////////////

/**
* Base class for an item in the composite pattern of Category Tree Item / Value Tree Item.
* Note that child trees to this class are owned by this class (in the IndexedPointerContainer).
*/
class TreeItem
{
public:
  TreeItem();
  virtual ~TreeItem();

  /** Forward from QAbstractItemModel::data() */
  virtual QVariant data(int role) const = 0;
  /** Forward from QAbstractItemModel::flags() */
  virtual Qt::ItemFlags flags() const = 0;
  /** Returns true if the GUI changed; sets filterChanged if filter edited. */
  virtual bool setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged) = 0;

  /** Returns the category name integer value for this item or its parent */
  virtual int nameInt() const = 0;
  /** Returns true if the UNLISTED VALUE item is checked (i.e. if we are in EXCLUDE mode) */
  virtual bool isUnlistedValueChecked() const = 0;

  ///@{ Composite Tree Management Methods
  TreeItem* parent() const;
  int rowInParent() const;
  int indexOf(const TreeItem* child) const;
  TreeItem* child(int index) const;
  int childCount() const;
  void addChild(TreeItem* item);
  ///@}

private:
  TreeItem* parent_;
  simQt::IndexedPointerContainer<TreeItem> children_;
};

/////////////////////////////////////////////////////////////////////////

/** Represents a group node in tree, showing a category name and containing children values. */
class CategoryTreeModel2::CategoryItem : public TreeItem
{
public:
  CategoryItem(const simData::CategoryNameManager& nameManager, int nameInt);

  /** TreeItem Overrides */
  virtual Qt::ItemFlags flags() const;
  virtual QVariant data(int role) const;
  virtual bool setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged);
  virtual int nameInt() const;
  virtual bool isUnlistedValueChecked() const;

  /** Recalculates the "contributes to filter" flag, returning true if it changes (like setData()) */
  bool recalcContributionTo(const simData::CategoryFilter& filter);

  /** Changes the font to use. */
  void setFont(QFont* font);
  /** Sets the state of the GUI to match the state of the filter. Returns 0 if nothing changed. */
  int updateTo(const simData::CategoryFilter& filter);

private:
  /** Changes the filter to match the check state of the Value Item. */
  void updateFilter_(const ValueItem& valueItem, simData::CategoryFilter& filter) const;
  /** Change the value item to match the state of the checks structure (filter).  Returns 0 on no change. */
  int updateValueItem_(ValueItem& valueItem, const simData::CategoryFilter::ValuesCheck& checks) const;

  /** String representation of NAME. */
  QString categoryName_;
  /** Integer representation of NAME. */
  int nameInt_;
  /** Cache the state of the UNLISTED VALUE.  When TRUE, we're in EXCLUDE mode */
  bool unlistedValue_;
  /** Set to true if this category contributes to the filter. */
  bool contributesToFilter_;
  /** Font to use for FontRole (not owned) */
  QFont* font_;
};

/////////////////////////////////////////////////////////////////////////

/** Represents a leaf node in tree, showing a category value. */
class CategoryTreeModel2::ValueItem : public TreeItem
{
public:
  ValueItem(const simData::CategoryNameManager& nameManager, int nameInt, int valueInt);

  /** TreeItem Overrides */
  virtual Qt::ItemFlags flags() const;
  virtual QVariant data(int role) const;
  virtual bool setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged);
  virtual int nameInt() const;
  virtual bool isUnlistedValueChecked() const;

  /** Returns the value integer for this item */
  int valueInt() const;
  /**
  * Changes the GUI state of whether this item is checked.  This does not match 1-for-1
  * with the filter state, and does not directly update any CategoryFilter instance.
  */
  void setChecked(bool value);
  /** Returns true if the GUI state is such that this item is checked. */
  bool isChecked() const;

private:
  int nameInt_;
  int valueInt_;
  int numMatches_;
  Qt::CheckState checked_;
  QString inclusiveText_;
};

/////////////////////////////////////////////////////////////////////////

TreeItem::TreeItem()
  : parent_(NULL)
{
}

TreeItem::~TreeItem()
{
  children_.deleteAll();
}

TreeItem* TreeItem::parent() const
{
  return parent_;
}

int TreeItem::rowInParent() const
{
  if (parent_ == NULL)
  {
    // Caller is getting an invalid value
    assert(0);
    return -1;
  }
  return parent_->indexOf(this);
}

int TreeItem::indexOf(const TreeItem* child) const
{
  return children_.indexOf(child);
}

TreeItem* TreeItem::child(int index) const
{
  return children_[index];
}

int TreeItem::childCount() const
{
  return children_.size();
}

void TreeItem::addChild(TreeItem* item)
{
  // Assertion failure means developer is doing something weird.
  assert(item != NULL);
  // Assertion failure means that item is inserted more than once.
  assert(item->parent() == NULL);

  // Set the parent and save the item in our children vector.
  item->parent_ = this;
  children_.push_back(item);
}

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel2::CategoryItem::CategoryItem(const simData::CategoryNameManager& nameManager, int nameInt)
  : categoryName_(QString::fromStdString(nameManager.nameIntToString(nameInt))),
    nameInt_(nameInt),
    unlistedValue_(false),
    contributesToFilter_(false),
    font_(NULL)
{
}

bool CategoryTreeModel2::CategoryItem::isUnlistedValueChecked() const
{
  return unlistedValue_;
}

int CategoryTreeModel2::CategoryItem::nameInt() const
{
  return nameInt_;
}

Qt::ItemFlags CategoryTreeModel2::CategoryItem::flags() const
{
  return Qt::ItemIsEnabled;
}

QVariant CategoryTreeModel2::CategoryItem::data(int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
  case Qt::EditRole:
  case ROLE_SORT_STRING:
    return categoryName_;
  case ROLE_EXCLUDE:
    return unlistedValue_;
  case Qt::BackgroundColorRole:
    if (contributesToFilter_)
      return CONTRIBUTING_BG_COLOR;
    return MIDLIGHT_BG_COLOR;
  case Qt::FontRole:
    if (font_)
      return *font_;
    break;
  default:
    break;
  }
  return QVariant();
}

bool CategoryTreeModel2::CategoryItem::setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged)
{
  filterChanged = false;
  if (role != ROLE_EXCLUDE || value.toBool() == unlistedValue_)
    return false;

  // Update the value
  unlistedValue_ = value.toBool();

  // If the filter does not include our category, then we do nothing RE: filter
  auto values = filter.getCategoryFilter();
  if (values.find(nameInt_) == values.end())
    return true; // True, update our GUI -- but note that the filter did not change

  // Remove the whole name from the filter, then build it from scratch from GUI
  filterChanged = true;
  filter.removeName(nameInt_);
  filter.setValue(nameInt_, simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE, unlistedValue_);
  const int count = childCount();
  for (int k = 0; k < count; ++k)
    updateFilter_(*static_cast<ValueItem*>(child(k)), filter);
  filter.simplify(nameInt_);

  // Update the flag for contributing to the filter
  recalcContributionTo(filter);
  return true;
}

bool CategoryTreeModel2::CategoryItem::recalcContributionTo(const simData::CategoryFilter& filter)
{
  const auto& catValues = filter.getCategoryFilter();
  const bool newValue = (catValues.find(nameInt_) != catValues.end());
  if (newValue == contributesToFilter_)
    return false;
  contributesToFilter_ = newValue;
  return true;
}

void CategoryTreeModel2::CategoryItem::setFont(QFont* font)
{
  font_ = font;
}

int CategoryTreeModel2::CategoryItem::updateTo(const simData::CategoryFilter& filter)
{
  simData::CategoryFilter::ValuesCheck checks;
  filter.getValues(nameInt_, checks);

  // Case 1: Filter doesn't have this category.  Uncheck all children
  if (checks.empty())
  {
    bool hasChange = false;
    const int count = childCount();
    for (int k = 0; k < count; ++k)
    {
      ValueItem* valueItem = static_cast<ValueItem*>(child(k));
      if (valueItem->isChecked())
      {
        valueItem->setChecked(false);
        hasChange = true;
      }
    }
    return hasChange ? 1 : 0;
  }

  // Case 2: We are in the filter, so our unlistedValueBool matters
  auto i = checks.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  if (i != checks.end())
  {
    // Unlisted value present means it must be on
    assert(i->second);
  }

  // Detect change in Unlisted Value state
  const bool newUnlistedValue = (i != checks.end() && i->second);
  bool hasChange = (unlistedValue_ != newUnlistedValue);
  unlistedValue_ = newUnlistedValue;

  // Iterate through children and make sure the state matches
  const int count = childCount();
  for (int k = 0; k < count; ++k)
  {
    if (0 != updateValueItem_(*static_cast<ValueItem*>(child(k)), checks))
      hasChange = true;
  }

  // Update the flag for contributing to the filter
  if (recalcContributionTo(filter))
    hasChange = true;

  return hasChange ? 1 : 0;
}

void CategoryTreeModel2::CategoryItem::updateFilter_(const ValueItem& valueItem, simData::CategoryFilter& filter) const
{
  const bool filterValue = (valueItem.isChecked() != unlistedValue_);
  // NO_VALUE is a special case
  if (valueItem.valueInt() == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
  {
    if (filterValue)
      filter.setValue(nameInt_, valueItem.valueInt(), true);
  }
  else
  {
    if (filterValue != unlistedValue_)
      filter.setValue(nameInt_, valueItem.valueInt(), filterValue);
  }
}

int CategoryTreeModel2::CategoryItem::updateValueItem_(ValueItem& valueItem, const simData::CategoryFilter::ValuesCheck& checks) const
{
  // NO VALUE is a special case unfortunately
  const auto i = checks.find(valueItem.valueInt());
  bool nextCheckedState = false;
  if (valueItem.valueInt() == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
  {
    // Item is a NO-VALUE item.  This does not follow the rules of "unlisted value"
    // in CategoryFilter class, so it's a special case, because we DO want to follow
    // logical rules for the end user here in this GUI.
    const bool showingNoValue = (i != checks.end() && i->second);
    // If unlisted value is false, then we show the NO VALUE as checked if its check
    // is present and on.  If unlisted value is true, then we invert the display
    // so that No-Value swaps into No-No-Value, or Has-Value for short.  This all
    // simplifies into the expression "setChecked(unlisted != showing)".
    nextCheckedState = (unlistedValue_ != showingNoValue);
  }
  else if (unlistedValue_)
  {
    // "Harder" case.  Unlisted Values are checked, so GUI is showing "omit" or "not"
    // states.  If it's checked, then we're explicitly omitting that value.  So the
    // only way to omit is if there is an entry in the checks, and it's set false.
    nextCheckedState = (i != checks.end() && !i->second);
  }
  else
  {
    // "Simple" case.  Unlisted Values are unchecked, so we're matching ONLY items
    // that are in the filter, that are checked.  So to be checked in the GUI, the
    // value must have a checkmark
    nextCheckedState = (i != checks.end() && i->second);
  }

  if (nextCheckedState == valueItem.isChecked())
    return 0;
  valueItem.setChecked(nextCheckedState);
  return 1;
}

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel2::ValueItem::ValueItem(const simData::CategoryNameManager& nameManager, int nameInt, int valueInt)
  : nameInt_(nameInt),
    valueInt_(valueInt),
    numMatches_(0),
    checked_(Qt::Unchecked),
    inclusiveText_(QString::fromStdString(nameManager.valueIntToString(valueInt)))
{
}

bool CategoryTreeModel2::ValueItem::isUnlistedValueChecked() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return false;
  return parent()->isUnlistedValueChecked();
}

int CategoryTreeModel2::ValueItem::nameInt() const
{
  return nameInt_;
}

int CategoryTreeModel2::ValueItem::valueInt() const
{
  return valueInt_;
}

Qt::ItemFlags CategoryTreeModel2::ValueItem::flags() const
{
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

QVariant CategoryTreeModel2::ValueItem::data(int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
  case Qt::EditRole:
    if (!isUnlistedValueChecked())
      return inclusiveText_;
    if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      return tr("Has Value");
    return tr("Not %1").arg(inclusiveText_);

  case Qt::CheckStateRole:
    return checked_;

  case ROLE_SORT_STRING:
    if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      return QString("");
    return data(Qt::DisplayRole);

  case ROLE_EXCLUDE:
    return isUnlistedValueChecked();

  default:
    break;
  }
  return QVariant();
}

bool CategoryTreeModel2::ValueItem::setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged)
{
  filterChanged = false;
  // Only editing supported on values is check/uncheck
  if (role != Qt::CheckStateRole)
    return false;

  // If the edit sets us to same state, then return early
  const Qt::CheckState newChecked = static_cast<Qt::CheckState>(value.toInt());
  if (newChecked == checked_)
    return false;

  // Figure out how to translate the GUI state into the filter value
  checked_ = newChecked;
  const bool unlistedValue = isUnlistedValueChecked();
  const bool checkedBool = (checked_ == Qt::Checked);
  const bool filterValue = (unlistedValue != checkedBool);

  // Change the value in the filter.  NO VALUE is a special case
  if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
  {
    // If the filter value is off, then remove it from the filter; it's always off by default
    if (!filterValue)
      filter.removeValue(nameInt_, valueInt_);
    else
      filter.setValue(nameInt_, valueInt_, true);
  }
  else
  {
    // Remove items that match unlisted value.  Add items that do not.
    if (filterValue == unlistedValue)
      filter.removeValue(nameInt_, valueInt_);
    else
    {
      // If the filter was previously empty and we're setting a value, we need to
      // make sure that the "No Value" check is correctly set in some cases.
      if (!filterValue && unlistedValue)
      {
        simData::CategoryFilter::ValuesCheck checks;
        filter.getValues(nameInt_, checks);
        if (checks.empty())
          filter.setValue(nameInt_, simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME, true);
      }

      filter.setValue(nameInt_, valueInt_, filterValue);
    }
  }

  // Ensure UNLISTED VALUE is set correctly.
  if (unlistedValue)
    filter.setValue(nameInt_, simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE, true);
  else
    filter.removeValue(nameInt_, simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  // Make sure the filter is simplified
  filter.simplify(nameInt_);

  // Update the parent too, which fixes the GUI for whether it contributes
  CategoryItem* parentTree = dynamic_cast<CategoryItem*>(parent());
  parentTree->recalcContributionTo(filter);

  filterChanged = true;
  return true;
}

void CategoryTreeModel2::ValueItem::setChecked(bool value)
{
  checked_ = (value ? Qt::Checked : Qt::Unchecked);
}

bool CategoryTreeModel2::ValueItem::isChecked() const
{
  return checked_ == Qt::Checked;
}

/////////////////////////////////////////////////////////////////////////

/// Monitors for category data changes, calling methods in CategoryTreeModel2.
class CategoryTreeModel2::CategoryFilterListener : public simData::CategoryNameManager::Listener
{
public:
  /// Constructor
  explicit CategoryFilterListener(CategoryTreeModel2& parent)
    : parent_(parent)
  {
  }

  virtual ~CategoryFilterListener()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex)
  {
    parent_.addName_(categoryIndex);
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex)
  {
    parent_.addValue_(categoryIndex, valueIndex);
  }

  /// Invoked when all data is cleared
  virtual void onClear()
  {
    parent_.clearTree_();
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing()
  {
    // noop
  }

private:
  CategoryTreeModel2& parent_;
};

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel2::CategoryTreeModel2(QObject* parent)
  : QAbstractItemModel(parent),
    dataStore_(NULL),
    filter_(NULL),
    categoryFont_(new QFont)
{
  listener_.reset(new CategoryFilterListener(*this));

  // Increase the point size on the category
  categoryFont_->setPointSize(categoryFont_->pointSize() + 4);
  categoryFont_->setBold(true);
}

CategoryTreeModel2::~CategoryTreeModel2()
{
  categories_.deleteAll();
  categoryIntToItem_.clear();
  delete categoryFont_;
  categoryFont_ = NULL;
  delete filter_;
  filter_ = NULL;
  if (dataStore_)
    dataStore_->categoryNameManager().removeListener(listener_);
}

QModelIndex CategoryTreeModel2::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  // Category items have no parent in the model
  if (!parent.isValid())
    return createIndex(row, column, categories_[row]);
  // Has a parent: must be a value item
  TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
  // Item was not made correctly, check index()
  assert(parentItem != NULL);
  return createIndex(row, column, parentItem->child(row));
}

QModelIndex CategoryTreeModel2::parent(const QModelIndex &child) const
{
  if (!child.isValid() || !child.internalPointer())
    return QModelIndex();

  // Child could be a category (no parent) or a value (category parent)
  const TreeItem* childItem = static_cast<TreeItem*>(child.internalPointer());
  TreeItem* parentItem = childItem->parent();
  if (parentItem == NULL) // child is a category; no parent
    return QModelIndex();
  return createIndex(categories_.indexOf(static_cast<CategoryItem*>(parentItem)), 0, parentItem);
}

int CategoryTreeModel2::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    if (parent.column() != 0)
      return 0;
    TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
    return (parentItem == NULL) ? 0 : parentItem->childCount();
  }
  return categories_.size();
}

int CategoryTreeModel2::columnCount(const QModelIndex &parent) const
{
  return 1;
}

QVariant CategoryTreeModel2::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || !index.internalPointer())
    return QVariant();
  const TreeItem* treeItem = static_cast<TreeItem*>(index.internalPointer());
  return treeItem->data(role);
}

QVariant CategoryTreeModel2::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
  {
    if (section == 0)
      return tr("Category");

    // A column was added and this section was not updated
    assert(0);
    return QVariant();
  }

  // Isn't the bar across the top -- fall back to whatever QAIM does
  return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags CategoryTreeModel2::flags(const QModelIndex& index) const
{
  if (!index.isValid() || !index.internalPointer())
    return Qt::NoItemFlags;
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  return item->flags();
}

bool CategoryTreeModel2::setData(const QModelIndex& index, const QVariant& value, int role)
{
  // Ensure we have a valid index with a valid TreeItem pointer
  if (!index.isValid() || !index.internalPointer())
    return QAbstractItemModel::setData(index, value, role);

  // NULL filter means the tree should be empty, so we shouldn't get setData()...
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  assert(filter_ && item);
  bool wasEdited = false;
  const bool rv = item->setData(value, role, *filter_, wasEdited);
  // Logic below needs to change if this assert triggers.  Basically, GUI may
  // update without the filter updating, but not vice versa.
  assert(rv || !wasEdited);
  if (rv)
  {
    // Update the GUI
    emit dataChanged(index, index);

    // Alert users who are listening
    if (wasEdited)
    {
      // Parent index, if it exists, is a category and might have updated its color data()
      const QModelIndex parentIndex = index.parent();
      if (parentIndex.isValid())
        emit dataChanged(parentIndex, parentIndex);

      emit filterChanged(*filter_);
      emit filterEdited(*filter_);
    }
  }
  return rv;
}

void CategoryTreeModel2::setFilter(const simData::CategoryFilter& filter)
{
  // Check the data store; if it's set in filter and different from ours, update
  if (filter.getDataStore() && filter.getDataStore() != dataStore_)
    setDataStore(filter.getDataStore());

  // Avoid no-op
  simData::CategoryFilter simplified(filter);
  simplified.simplify();
  if (filter_ != NULL && simplified.getCategoryFilter() == filter_->getCategoryFilter())
    return;

  // Do a two step assignment so that we don't automatically get auto-update
  if (filter_ == NULL)
    filter_ = new simData::CategoryFilter(filter.getDataStore());
  filter_->assign(simplified, false);

  const int categoriesSize = categories_.size();
  if (categoriesSize == 0)
  {
    // This means we have a simplified filter that is DIFFERENT from our current
    // filter, AND it means we have no items in the GUI.  It means we're out of
    // sync and something is not right.  Check into it.
    assert(0);
    return;
  }

  // Update to the filter, but detect which rows changed so we can simplify dataChanged()
  // for performance reasons.  This will prevent the display from updating too much.
  int firstChangeRow = -1;
  int lastChangeRow = -1;
  for (int k = 0; k < categoriesSize; ++k)
  {
    // Detect change and record the row number
    if (categories_[k]->updateTo(*filter_) != 0)
    {
      if (firstChangeRow == -1)
        firstChangeRow = k;
      lastChangeRow = k;
    }
  }

  // This shouldn't happen because we checked the simplified filters.  If this
  // assert triggers, then we have a change in filter (detected above) but the
  // GUI didn't actually change.  Maybe filter compare failed, or updateTo()
  // is returning incorrect values.
  assert(firstChangeRow != -1 && lastChangeRow != -1);
  if (firstChangeRow != -1 && lastChangeRow != -1)
  {
    emit dataChanged(index(firstChangeRow, 0), index(lastChangeRow, 0));
  }
  emit filterChanged(*filter_);
}

const simData::CategoryFilter& CategoryTreeModel2::categoryFilter() const
{
  // Precondition of this method is that data store was set; filter must be non-NULL
  assert(filter_);
  return *filter_;
}

void CategoryTreeModel2::setDataStore(simData::DataStore* dataStore)
{
  if (dataStore_ == dataStore)
    return;

  // Update the listeners on name manager as we change it
  if (dataStore_ != NULL)
    dataStore_->categoryNameManager().removeListener(listener_);
  dataStore_ = dataStore;
  if (dataStore_ != NULL)
    dataStore_->categoryNameManager().addListener(listener_);

  beginResetModel();

  // Clear out the internal storage on the tree
  categories_.deleteAll();
  categoryIntToItem_.clear();

  // Clear out the internal filter object
  const bool hadFilter = (filter_ != NULL && !filter_->getCategoryFilter().empty());
  delete filter_;
  filter_ = NULL;
  if (dataStore_)
  {
    filter_ = new simData::CategoryFilter(dataStore_);
    const simData::CategoryNameManager& nameManager = dataStore_->categoryNameManager();

    // Populate the GUI
    std::vector<int> nameInts;
    nameManager.allCategoryNameInts(nameInts);
    for (auto i = nameInts.begin(); i != nameInts.end(); ++i)
    {
      // Save the Category item and map it into our quick-search map
      CategoryItem* category = new CategoryItem(nameManager, *i);
      category->setFont(categoryFont_);
      categories_.push_back(category);
      categoryIntToItem_[*i] = category;

      // Create an item for "NO VALUE" since it won't be in the list of values we receive
      ValueItem* noValueItem = new ValueItem(nameManager, *i, simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
      category->addChild(noValueItem);

      // Save all the category values
      std::vector<int> valueInts;
      nameManager.allValueIntsInCategory(*i, valueInts);
      for (auto vi = valueInts.begin(); vi != valueInts.end(); ++vi)
      {
        ValueItem* valueItem = new ValueItem(nameManager, *i, *vi);
        category->addChild(valueItem);
      }
    }
  }

  // Model reset is done
  endResetModel();

  // Alert listeners if we have a new filter
  if (hadFilter && filter_)
    emit filterChanged(*filter_);
}

void CategoryTreeModel2::clearTree_()
{
  beginResetModel();
  categories_.clear();
  categoryIntToItem_.clear();
  endResetModel();
}

void CategoryTreeModel2::addName_(int nameInt)
{
  assert(dataStore_ != NULL);

  // Create the tree item for the category
  const auto& nameManager = dataStore_->categoryNameManager();
  CategoryItem* category = new CategoryItem(nameManager, nameInt);
  category->setFont(categoryFont_);

  // Debug mode: Validate that there are no values in that category yet.  If this section
  // of code fails, then we'll need to add ValueItem entries for the category on creation.
#ifdef DEBUG
  std::vector<int> valuesInCategory;
  dataStore_->categoryNameManager().allValueIntsInCategory(nameInt, valuesInCategory);
  // Assertion failure means we need to update this code to add the values.
  assert(valuesInCategory.empty());
#endif

  // About to update the GUI by adding a new item at the end
  beginInsertRows(QModelIndex(), categories_.size(), categories_.size());
  categories_.push_back(category);
  categoryIntToItem_[nameInt] = category;

  // Create an item for "NO VALUE" since it won't be in the list of values we receive
  ValueItem* noValueItem = new ValueItem(nameManager, nameInt, simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
  category->addChild(noValueItem);

  endInsertRows();
}

CategoryTreeModel2::CategoryItem* CategoryTreeModel2::findNameTree_(int nameInt) const
{
  auto i = categoryIntToItem_.find(nameInt);
  return (i == categoryIntToItem_.end()) ? NULL : i->second;
}

void CategoryTreeModel2::addValue_(int nameInt, int valueInt)
{
  // Find the parent item
  TreeItem* nameItem = findNameTree_(nameInt);
  // Means we got a category that we don't know about; shouldn't happen.
  assert(nameItem);
  if (nameItem == NULL)
    return;

  // Create the value item
  ValueItem* valueItem = new ValueItem(dataStore_->categoryNameManager(), nameInt, valueInt);

  // Get the index for the name (parent), and add this new valueItem into the tree
  const QModelIndex nameIndex = createIndex(categories_.indexOf(static_cast<CategoryItem*>(nameItem)), 0, nameItem);
  beginInsertRows(nameIndex, nameItem->childCount(), nameItem->childCount());
  nameItem->addChild(valueItem);
  endInsertRows();
}

/////////////////////////////////////////////////////////////////////////

/** Style options for drawing a toggle switch */
struct StyleOptionToggleSwitch
{
  /** Rectangle to draw the switch in */
  QRect rect;
  /** Vertical space between drawn track and the rect */
  int trackMargin;
  /** Font to draw text in */
  QFont font;

  /** State: on (to the right) or off (to the left) */
  bool value;

  /** Describes On|Off styles */
  struct StateStyle {
    /** Brush for painting the track */
    QBrush track;
    /** Brush for painting the thumb */
    QBrush thumb;
    /** Text to draw in the track */
    QString text;
    /** Color of text to draw */
    QColor textColor;
  };

  /** Style to use for ON state */
  StateStyle on;
  /** Stile to use for OFF state */
  StateStyle off;

  /** Initialize to default options */
  StyleOptionToggleSwitch()
    : trackMargin(0),
    value(false)
  {
    // Teal colored track and thumb
    on.track = QColor(0, 150, 136);
    on.thumb = on.track;
    on.text = QObject::tr("Exclude");
    on.textColor = Qt::black;

    // Black and grey track and thumb
    off.track = Qt::black;
    off.thumb = QColor(200, 200, 200);
    off.text = QObject::tr("Match");
    off.textColor = Qt::white;
  }
};

/////////////////////////////////////////////////////////////////////////

/** Responsible for internal layout and painting of a Toggle Switch widget */
class ToggleSwitchPainter
{
public:
  /** Paint the widget using the given options on the painter provided. */
  virtual void paint(const StyleOptionToggleSwitch& option, QPainter* painter) const;
  /** Returns a size hint for the toggle switch.  Uses option's rectangle height. */
  virtual QSize sizeHint(const StyleOptionToggleSwitch& option) const;

private:
  /** Stores rectangle zones for sub-elements of switch. */
  struct ChildRects
  {
    QRect track;
    QRect thumb;
    QRect text;
  };

  /** Calculates the rectangles for painting for each sub-element of the toggle switch. */
  void calculateRects_(const StyleOptionToggleSwitch& option, ChildRects& rects) const;
};

void ToggleSwitchPainter::paint(const StyleOptionToggleSwitch& option, QPainter* painter) const
{
  painter->save();

  // Adapted from https://stackoverflow.com/questions/14780517

  // Figure out positions of all subelements
  ChildRects r;
  calculateRects_(option, r);

  const StyleOptionToggleSwitch::StateStyle& valueStyle = (option.value ? option.on : option.off);

  // Draw the track
  painter->setPen(Qt::NoPen);
  painter->setBrush(valueStyle.track);
  painter->setOpacity(0.45);
  painter->setRenderHint(QPainter::Antialiasing, true);
  const double halfHeight = r.track.height() * 0.5;
  painter->drawRoundedRect(r.track, halfHeight, halfHeight);

  // Draw the text next
  painter->setOpacity(1.0);
  painter->setPen(valueStyle.textColor);
  painter->setFont(option.font);
  painter->drawText(r.text, Qt::AlignHCenter | Qt::AlignVCenter, valueStyle.text);

  // Draw thumb on top of all
  painter->setPen(Qt::NoPen);
  painter->setBrush(valueStyle.thumb);
  painter->drawEllipse(r.thumb);

  painter->restore();
}

QSize ToggleSwitchPainter::sizeHint(const StyleOptionToggleSwitch& option) const
{
  // Count in the font text for width
  int textWidth = 0;
  QFontMetrics fontMetrics(option.font);
  if (!option.on.text.isEmpty() || !option.off.text.isEmpty())
  {
    const int onWidth = fontMetrics.width(option.on.text);
    const int offWidth = fontMetrics.width(option.off.text);
    textWidth = qMax(onWidth, offWidth);
  }

  // Best width depends on height
  int height = option.rect.height();
  if (height == 0)
    height = fontMetrics.height();

  const int desiredWidth = static_cast<int>(1.5 * option.rect.height()) + textWidth;
  return QSize(desiredWidth, height);
}

void ToggleSwitchPainter::calculateRects_(const StyleOptionToggleSwitch& option, ChildRects& rects) const
{
  // Track is centered about the rectangle
  rects.track = QRect(option.rect.adjusted(0, option.trackMargin, 0, -option.trackMargin));

  // Thumb should be 1 pixel shorter than the track on top and bottom
  rects.thumb = QRect(option.rect.adjusted(0, 1, 0, -1));
  rects.thumb.setWidth(rects.thumb.height());
  // Move thumb to the right
  if (option.value)
    rects.thumb.translate(rects.track.width() - rects.thumb.height(), 0);

  // Text is inside the rect, excluding the thumb area
  rects.text = QRect(option.rect);
  if (option.value)
    rects.text.setRight(rects.thumb.left());
  else
    rects.text.setLeft(rects.thumb.right());
  // Shift the text closer to center (thumb) to avoid being too close to edge
  rects.text.translate(option.value ? 1 : -1, 0);
}

/////////////////////////////////////////////////////////////////////////

/** Expected tree indentation.  Tree takes away parts of delegate for tree painting and we want to undo that. */
static const int TREE_INDENTATION = 20;
/** Role to use for editing and displaying the "EXCLUDE" button */
static const int ROLE_EXCLUDE = simQt::CategoryTreeModel2::ROLE_EXCLUDE;

struct CategoryTreeItemDelegate::ChildRects
{
  QRect background;
  QRect checkbox;
  QRect branch;
  QRect text;
  QRect excludeToggle;
};

CategoryTreeItemDelegate::CategoryTreeItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

CategoryTreeItemDelegate::~CategoryTreeItemDelegate()
{
}

void CategoryTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& inOption, const QModelIndex& index) const
{
  // Initialize a new option struct that has data from the QModelIndex
  QStyleOptionViewItemV4 opt(inOption);
  initStyleOption(&opt, index);

  // Save the painter then draw based on type of node
  painter->save();
  if (!index.parent().isValid())
    paintCategory_(painter, opt, index);
  else
    paintValue_(painter, opt, index);
  painter->restore();
}

void CategoryTreeItemDelegate::paintCategory_(QPainter* painter, QStyleOptionViewItem& opt, const QModelIndex& index) const
{
  const QStyle* style = (opt.widget ? opt.widget->style() : qApp->style());

  // Calculate the rectangles for drawing
  ChildRects r;
  calculateRects_(opt, index, r);

  { // Draw a background for the whole row
    painter->setBrush(opt.backgroundBrush);
    painter->setPen(Qt::NoPen);
    painter->drawRect(r.background);
  }

  { // Draw the expand/collapse icon on left side
    QStyleOptionViewItemV4 branchOpt(opt);
    branchOpt.rect = r.branch;
    branchOpt.state &= ~QStyle::State_MouseOver;
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOpt, painter);
  }

  { // Draw the text for the category
    opt.rect = r.text;
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
  }

  { // Draw the toggle switch for changing EXCLUDE and INCLUDE
    StyleOptionToggleSwitch switchOpt;
    ToggleSwitchPainter switchPainter;
    switchOpt.rect = r.excludeToggle;
    switchOpt.value = index.data(ROLE_EXCLUDE).toBool();
    switchPainter.paint(switchOpt, painter);
  }
}

void CategoryTreeItemDelegate::paintValue_(QPainter* painter, QStyleOptionViewItem& opt, const QModelIndex& index) const
{
  const QStyle* style = (opt.widget ? opt.widget->style() : qApp->style());
  const bool isChecked = (index.data(Qt::CheckStateRole).toInt() == Qt::Checked);

  // Calculate the rectangles for drawing
  ChildRects r;
  calculateRects_(opt, index, r);
  opt.rect = r.text;

  // Draw a checked checkbox on left side of item if the item is checked
  if (isChecked)
  {
    // Move it to left side of widget
    QStyleOption checkOpt(opt);
    checkOpt.rect = r.checkbox;
    // Check the button, then draw
    checkOpt.state |= QStyle::State_On;
    style->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkOpt, painter);

    // Checked category values also show up bold
    opt.font.setBold(true);
  }

  // Category values that are hovered are shown as underlined in link color (blue usually)
  if (opt.state.testFlag(QStyle::State_MouseOver))
  {
    opt.font.setUnderline(true);
    opt.palette.setBrush(QPalette::Text, opt.palette.color(QPalette::Link));
  }

  // Turn off the check indicator unconditionally, then draw the item
  opt.features &= ~QStyleOptionViewItem::HasCheckIndicator;
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

bool CategoryTreeItemDelegate::editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if (index.isValid() && !index.parent().isValid())
    return categoryEvent_(evt, model, option, index);
  return valueEvent_(evt, model, option, index);
}

bool CategoryTreeItemDelegate::categoryEvent_(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  // Cast may not be valid, depends on evt->type()
  const QMouseEvent* me = static_cast<const QMouseEvent*>(evt);

  switch (evt->type())
  {
  case QEvent::MouseButtonPress:
    // Only care about left presses.  All other presses are ignored.
    if (me->button() != Qt::LeftButton)
    {
      clickedIndex_ = QModelIndex();
      return false;
    }

    clickedElement_ = hit_(me->pos(), option, index);
    // Eat the branch press and don't do anything on release
    if (clickedElement_ == SE_BRANCH)
    {
      clickedIndex_ = QModelIndex();
      emit expandClicked(index);
      return true;
    }
    clickedIndex_ = index;
    break;

  case QEvent::MouseButtonRelease:
  {
    // Clicking on toggle should save the index to detect release on the toggle
    const auto newHit = hit_(me->pos(), option, index);
    // Must match button, index, and element clicked
    if (me->button() == Qt::LeftButton && clickedIndex_ == index && newHit == clickedElement_)
    {
      // Toggle button should, well, toggle
      if (clickedElement_ == SE_EXCLUDE_TOGGLE)
      {
        QVariant oldState = index.data(ROLE_EXCLUDE);
        model->setData(index, !oldState.toBool(), ROLE_EXCLUDE);
        clickedIndex_ = QModelIndex();
        return true;
      }
    }
    clickedIndex_ = QModelIndex();
    break;
  }

  case QEvent::MouseButtonDblClick:
    clickedIndex_ = QModelIndex();
    clickedElement_ = hit_(me->pos(), option, index);
    // Ignore double click on the toggle button and branch button, so that it doesn't cause expand/contract
    if (clickedElement_ == SE_EXCLUDE_TOGGLE || clickedElement_ == SE_BRANCH)
      return true;
    break;

  default: // Many potential events not handled
    break;
  }

  return false;
}

bool CategoryTreeItemDelegate::valueEvent_(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if (evt->type() != QEvent::MouseButtonPress && evt->type() != QEvent::MouseButtonRelease)
    return false;
  // At this stage it's either a press or a release
  const QMouseEvent* me = static_cast<const QMouseEvent*>(evt);
  const bool isPress = (evt->type() == QEvent::MouseButtonPress);
  const bool isRelease = !isPress;

  // Determine whether we care about the event
  bool usefulEvent = true;
  if (me->button() != Qt::LeftButton)
    usefulEvent = false;
  else if (isRelease && clickedIndex_ != index)
    usefulEvent = false;
  // Should have a check state; if not, that's weird, return out
  QVariant checkState = index.data(Qt::CheckStateRole);
  if (!checkState.isValid())
    usefulEvent = false;

  // Clear out the model index before returning
  if (!usefulEvent)
  {
    clickedIndex_ = QModelIndex();
    return false;
  }

  // If it's a press, save the index for later.  Note we don't use clickedElement_
  if (isPress)
    clickedIndex_ = index;
  else
  {
    // Invert the state and send it as an updated check
    Qt::CheckState newState = (checkState.toInt() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    model->setData(index, newState, Qt::CheckStateRole);
    clickedIndex_ = QModelIndex();
  }
  return true;
}

void CategoryTreeItemDelegate::calculateRects_(const QStyleOptionViewItem& option, const QModelIndex& index, ChildRects& rects) const
{
  rects.background = option.rect;

  const bool isValue = index.isValid() && index.parent().isValid();
  if (isValue)
  {
    rects.background.setLeft(0);
    rects.checkbox = rects.background;
    rects.checkbox.setRight(TREE_INDENTATION);

    // Text takes up everything to the right of the checkbox
    rects.text = rects.background.adjusted(TREE_INDENTATION, 0, 0, 0);
  }
  else
  {
    // Branch is the > or v indicator for expanding
    rects.branch = rects.background;
    rects.branch.setRight(rects.branch.left() + rects.branch.height());

    // Calculate the width given the rectangle of height, for the toggle switch
    rects.excludeToggle = rects.background.adjusted(0, 1, -1, -1);
    ToggleSwitchPainter switchPainter;
    StyleOptionToggleSwitch switchOpt;
    switchOpt.rect = rects.excludeToggle;
    const QSize toggleSize = switchPainter.sizeHint(switchOpt);
    // Set the left side appropriately
    rects.excludeToggle.setLeft(rects.excludeToggle.right() - toggleSize.width());

    // Text takes up everything to the right of the branch button until the exclude toggle
    rects.text = rects.background;
    rects.text.setLeft(rects.branch.right());
    rects.text.setRight(rects.excludeToggle.left());
  }
}

CategoryTreeItemDelegate::SubElement CategoryTreeItemDelegate::hit_(const QPoint& pos, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  // Calculate the various rectangles
  ChildRects r;
  calculateRects_(option, index, r);

  if (r.excludeToggle.isValid() && r.excludeToggle.contains(pos))
    return SE_EXCLUDE_TOGGLE;
  if (r.checkbox.isValid() && r.checkbox.contains(pos))
    return SE_CHECKBOX;
  if (r.branch.isValid() && r.branch.contains(pos))
    return SE_BRANCH;
  if (r.text.isValid() && r.text.contains(pos))
    return SE_TEXT;
  // Background encompasses all, so if we're not here we're in NONE
  if (r.background.isValid() && r.background.contains(pos))
    return SE_BACKGROUND;
  return SE_NONE;
}

bool CategoryTreeItemDelegate::helpEvent(QHelpEvent* evt, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if (evt->type() == QEvent::ToolTip)
  {
    // Special tooltip for the EXCLUDE filter
    if (hit_(evt->pos(), option, index) == SE_EXCLUDE_TOGGLE)
    {
      QToolTip::showText(evt->globalPos(), simQt::formatTooltip(tr("Exclude"),
        tr("When on, Exclude mode will omit all entities that match your selected values.<p>When off, the filter will match all entities that have one of your checked category values.")),
        view);
      return true;
    }
  }
  return QStyledItemDelegate::helpEvent(evt, view, option, index);
}

/////////////////////////////////////////////////////////////////////////

CategoryFilterWidget2::CategoryFilterWidget2(QWidget* parent)
  : QWidget(parent),
  activeFiltering_(false)
{
  setWindowTitle("Category Data Filter");
  setObjectName("CategoryFilterWidget2");

  treeModel_ = new simQt::CategoryTreeModel2(this);
  proxy_ = new simQt::CategoryProxyModel();
  proxy_->setSourceModel(treeModel_);
  proxy_->setSortRole(simQt::CategoryTreeModel2::ROLE_SORT_STRING);
  proxy_->sort(0);

  treeView_ = new QTreeView(this);
  treeView_->setObjectName("CategoryFilterTree");
  treeView_->setFocusPolicy(Qt::NoFocus);
  treeView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  treeView_->setIndentation(0);
  treeView_->setAllColumnsShowFocus(true);
  treeView_->setHeaderHidden(true);
  treeView_->setModel(proxy_);

  simQt::CategoryTreeItemDelegate* itemDelegate = new simQt::CategoryTreeItemDelegate(this);
  treeView_->setItemDelegate(itemDelegate);

  QAction* collapseAction = new QAction(tr("Collapse Values"), this);
  connect(collapseAction, SIGNAL(triggered()), treeView_, SLOT(collapseAll()));
  collapseAction->setIcon(QIcon(":/simQt/images/Collapse.png"));
  QAction* expandAction = new QAction(tr("Expand Values"), this);
  connect(expandAction, SIGNAL(triggered()), treeView_, SLOT(expandAll()));
  expandAction->setIcon(QIcon(":/simQt/images/Expand.png"));
  treeView_->setContextMenuPolicy(Qt::ActionsContextMenu);
  treeView_->addAction(collapseAction);
  treeView_->addAction(expandAction);

  simQt::SearchLineEdit* search = new simQt::SearchLineEdit(this);
  search->setPlaceholderText(tr("Search Category Data"));

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setObjectName("CategoryFilterWidgetVBox");
  layout->setMargin(0);
  layout->addWidget(search);
  layout->addWidget(treeView_);

  connect(treeModel_, SIGNAL(filterChanged(simData::CategoryFilter)), this, SIGNAL(filterChanged(simData::CategoryFilter)));
  connect(treeModel_, SIGNAL(filterEdited(simData::CategoryFilter)), this, SIGNAL(filterEdited(simData::CategoryFilter)));
  connect(treeModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToModel_(QModelIndex, int, int)));
  connect(proxy_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToProxy_(QModelIndex, int, int)));
  connect(proxy_, SIGNAL(modelReset()), treeView_, SLOT(expandAll()));
  connect(search, SIGNAL(textChanged(QString)), this, SLOT(expandAfterFilterEdited_(QString)));
  connect(search, SIGNAL(textChanged(QString)), proxy_, SLOT(setFilterText(QString)));
  connect(itemDelegate, SIGNAL(expandClicked(QModelIndex)), this, SLOT(toggleExpanded_(QModelIndex)));
}

CategoryFilterWidget2::~CategoryFilterWidget2()
{
}

void CategoryFilterWidget2::setDataStore(simData::DataStore* dataStore)
{
  treeModel_->setDataStore(dataStore);
  treeView_->expandAll();
}

const simData::CategoryFilter& CategoryFilterWidget2::categoryFilter() const
{
  return treeModel_->categoryFilter();
}

void CategoryFilterWidget2::setFilter(const simData::CategoryFilter& categoryFilter)
{
  treeModel_->setFilter(categoryFilter);
}

void CategoryFilterWidget2::expandAfterFilterEdited_(const QString& filterText)
{
  if (filterText.isEmpty())
  {
    // Just removed the last character of a search so expand all to make everything visible
    if (activeFiltering_)
      treeView_->expandAll();

    activeFiltering_ = false;
  }
  else
  {
    // Just started a search so expand all to make everything visible
    if (!activeFiltering_)
      treeView_->expandAll();

    activeFiltering_ = true;
  }
}

void CategoryFilterWidget2::expandDueToModel_(const QModelIndex& parentIndex, int to, int from)
{
  if (!activeFiltering_)
    return;

  bool isCategory = !parentIndex.isValid();
  if (isCategory)
    return;

  if (!treeView_->isExpanded(parentIndex))
    proxy_->resetFilter();
}

void CategoryFilterWidget2::expandDueToProxy_(const QModelIndex& parentIndex, int to, int from)
{
  bool isCategory = !parentIndex.isValid();
  if (isCategory)
  {
    // The category names are the "to" to "from" and they just showed up, so expand them
    for (int ii = to; ii <= from; ++ii)
    {
      QModelIndex catIndex = proxy_->index(ii, 0, parentIndex);
      treeView_->expand(catIndex);
    }
  }
  else
  {
    if (activeFiltering_)
    {
      // Adding a category value; make sure it is visible by expanding its parent
      if (!treeView_->isExpanded(parentIndex))
        treeView_->expand(parentIndex);
    }
  }
}

void CategoryFilterWidget2::toggleExpanded_(const QModelIndex& proxyIndex)
{
  treeView_->setExpanded(proxyIndex, !treeView_->isExpanded(proxyIndex));
}

}
