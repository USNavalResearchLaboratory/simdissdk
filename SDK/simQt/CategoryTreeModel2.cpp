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
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/DataStore.h"
#include "simQt/QtFormatting.h"
#include "simQt/CategoryFilterCounter.h"
#include "simQt/CategoryTreeModel.h"
#include "simQt/RegExpImpl.h"
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

  /** Retrieves the category name this tree item is associated with */
  virtual QString categoryName() const = 0;
  /** Returns the category name integer value for this item or its parent */
  virtual int nameInt() const = 0;
  /** Returns true if the UNLISTED VALUE item is checked (i.e. if we are in EXCLUDE mode) */
  virtual bool isUnlistedValueChecked() const = 0;
  /** Returns true if the tree item's category is influenced by a regular expression */
  virtual bool isRegExpApplied() const = 0;

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
  virtual QString categoryName() const;
  virtual int nameInt() const;
  virtual bool isUnlistedValueChecked() const;
  virtual bool isRegExpApplied() const;

  /** Recalculates the "contributes to filter" flag, returning true if it changes (like setData()) */
  bool recalcContributionTo(const simData::CategoryFilter& filter);

  /** Changes the font to use. */
  void setFont(QFont* font);
  /** Sets the state of the GUI to match the state of the filter. Returns 0 if nothing changed. */
  int updateTo(const simData::CategoryFilter& filter);

  /** Sets the ID counts for each value under this category name tree, returning true if there is a change. */
  bool updateCounts(const std::map<int, size_t>& valueToCountMap) const;

private:
  /** Checks and unchecks children based on whether they match the filter, returning true if any checks change. */
  bool setChildChecks_(const simData::RegExpFilter* reFilter);

  /** Changes the filter to match the check state of the Value Item. */
  void updateFilter_(const ValueItem& valueItem, simData::CategoryFilter& filter) const;
  /** Change the value item to match the state of the checks structure (filter).  Returns 0 on no change. */
  int updateValueItem_(ValueItem& valueItem, const simData::CategoryFilter::ValuesCheck& checks) const;

  /** setData() variant that handles the ROLE_EXCLUDE role */
  bool setExcludeData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged);
  /** setData() variant that handles ROLE_REGEXP_STRING role */
  bool setRegExpStringData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged);

  /** String representation of NAME. */
  QString categoryName_;
  /** Integer representation of NAME. */
  int nameInt_;
  /** Cache the state of the UNLISTED VALUE.  When TRUE, we're in EXCLUDE mode */
  bool unlistedValue_;
  /** Category's Regular Expression string value */
  QString regExpString_;
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
  virtual QString categoryName() const;
  virtual int nameInt() const;
  virtual bool isUnlistedValueChecked() const;
  virtual bool isRegExpApplied() const;

  /** Returns the value integer for this item */
  int valueInt() const;
  /** Returns the value string for this item; for NO_CATEGORY_VALUE_AT_TIME, empty string is returned. */
  QString valueString() const;

  /**
  * Changes the GUI state of whether this item is checked.  This does not match 1-for-1
  * with the filter state, and does not directly update any CategoryFilter instance.
  */
  void setChecked(bool value);
  /** Returns true if the GUI state is such that this item is checked. */
  bool isChecked() const;

  /** Sets the number of entities that match this value.  Use -1 to reset. */
  void setNumMatches(int numMatches);
  /** Returns number entities that match this particular value in the given filter. */
  int numMatches() const;

private:
  /** setData() that handles Qt::CheckStateRole.  Returns true if GUI state changes, and sets filterChanged if filter changes. */
  bool setCheckStateData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged);

  int nameInt_;
  int valueInt_;
  int numMatches_;
  Qt::CheckState checked_;
  QString valueString_;
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

bool CategoryTreeModel2::CategoryItem::isRegExpApplied() const
{
  return !regExpString_.isEmpty();
}

int CategoryTreeModel2::CategoryItem::nameInt() const
{
  return nameInt_;
}

QString CategoryTreeModel2::CategoryItem::categoryName() const
{
  return categoryName_;
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
  case ROLE_CATEGORY_NAME:
    return categoryName_;
  case ROLE_EXCLUDE:
    return unlistedValue_;
  case ROLE_REGEXP_STRING:
    return regExpString_;
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
  if (role == ROLE_EXCLUDE)
    return setExcludeData_(value, filter, filterChanged);
  else if (role == ROLE_REGEXP_STRING)
    return setRegExpStringData_(value, filter, filterChanged);
  filterChanged = false;
  return false;
}

bool CategoryTreeModel2::CategoryItem::setExcludeData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
{
  filterChanged = false;
  // If value does not change, or if disabled, then return early
  if (value.toBool() == unlistedValue_ || !flags().testFlag(Qt::ItemIsEnabled))
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

bool CategoryTreeModel2::CategoryItem::setRegExpStringData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
{
  // Check for easy no-op
  filterChanged = false;
  if (value.toString() == regExpString_)
    return false;

  // Update the value
  regExpString_ = value.toString();
  filterChanged = true;

  // Create/set the regular expression
  simData::RegExpFilterPtr newRegExpObject;
  if (!regExpString_.isEmpty())
  {
    // The factory could/should be passed in for maximum flexibility
    simQt::RegExpFilterFactoryImpl reFactory;
    newRegExpObject = reFactory.createRegExpFilter(regExpString_.toStdString());
  }

  // Set the RegExp, simplify, and update the internal state
  filter.setCategoryRegExp(nameInt_, newRegExpObject);
  filter.simplify(nameInt_);
  recalcContributionTo(filter);
  setChildChecks_(newRegExpObject.get());
  return true;
}

bool CategoryTreeModel2::CategoryItem::recalcContributionTo(const simData::CategoryFilter& filter)
{
  // First check the regular expression.  If there's a regexp, then this category definitely contributes
  const bool newValue = filter.nameContributesToFilter(nameInt_);
  if (newValue == contributesToFilter_)
    return false;
  contributesToFilter_ = newValue;
  return true;
}

void CategoryTreeModel2::CategoryItem::setFont(QFont* font)
{
  font_ = font;
}

bool CategoryTreeModel2::CategoryItem::setChildChecks_(const simData::RegExpFilter* reFilter)
{
  bool hasChange = false;
  const int count = childCount();
  for (int k = 0; k < count; ++k)
  {
    // Test the EditRole, which is used because it omits the # count (e.g. "Friendly (1)")
    ValueItem* valueItem = static_cast<ValueItem*>(child(k));
    const bool matches = reFilter != NULL && reFilter->match(valueItem->valueString().toStdString());
    if (matches != valueItem->isChecked())
    {
      valueItem->setChecked(matches);
      hasChange = true;
    }
  }
  return hasChange;
}

int CategoryTreeModel2::CategoryItem::updateTo(const simData::CategoryFilter& filter)
{
  // Update the category if it has a RegExp
  const QString oldRegExp = regExpString_;
  const auto* regExpObject = filter.getRegExp(nameInt_);
  regExpString_ = (regExpObject != NULL ? QString::fromStdString(filter.getRegExpPattern(nameInt_)) : "");
  // If the RegExp string is different, we definitely have some sort of change
  bool hasChange = (regExpString_ != oldRegExp);

  // Case 1: Regular Expression is not empty.  Check and uncheck values as needed
  if (!regExpString_.isEmpty())
  {
    // Synchronize the checks of the children
    if (setChildChecks_(regExpObject))
      hasChange = true;
    return hasChange ? 1 : 0;
  }

  // No RegExp -- pull out the category checks
  simData::CategoryFilter::ValuesCheck checks;
  filter.getValues(nameInt_, checks);

  // Case 2: Filter doesn't have this category.  Uncheck all children
  if (checks.empty())
  {
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

    // Fix filter on/off
    if (recalcContributionTo(filter))
      hasChange = true;
    return hasChange ? 1 : 0;
  }

  // Case 3: We are in the filter, so our unlistedValueBool matters
  auto i = checks.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  if (i != checks.end())
  {
    // Unlisted value present means it must be on
    assert(i->second);
  }

  // Detect change in Unlisted Value state
  const bool newUnlistedValue = (i != checks.end() && i->second);
  if (unlistedValue_ != newUnlistedValue)
    hasChange = true;
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

bool CategoryTreeModel2::CategoryItem::updateCounts(const std::map<int, size_t>& valueToCountMap) const
{
  const int numValues = childCount();
  bool haveChange = false;
  for (int k = 0; k < numValues; ++k)
  {
    ValueItem* valueItem = dynamic_cast<ValueItem*>(child(k));
    // All children should be ValueItems
    assert(valueItem);
    if (!valueItem)
      continue;

    // It's entirely possible (through async methods) that the incoming value count map is not
    // up to date.  This can occur if a count starts and more categories get added before the
    // count finishes, and is common.
    auto i = valueToCountMap.find(valueItem->valueInt());
    int nextMatch = -1;
    if (i != valueToCountMap.end())
      nextMatch = static_cast<int>(i->second);

    // Set the number of matches and record a change
    if (valueItem->numMatches() != nextMatch)
    {
      valueItem->setNumMatches(nextMatch);
      haveChange = true;
    }
  }

  return haveChange;
}

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel2::ValueItem::ValueItem(const simData::CategoryNameManager& nameManager, int nameInt, int valueInt)
  : nameInt_(nameInt),
    valueInt_(valueInt),
    numMatches_(-1),
    checked_(Qt::Unchecked),
    valueString_(QString::fromStdString(nameManager.valueIntToString(valueInt)))
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

bool CategoryTreeModel2::ValueItem::isRegExpApplied() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return false;
  return parent()->isRegExpApplied();
}

int CategoryTreeModel2::ValueItem::nameInt() const
{
  return nameInt_;
}

QString CategoryTreeModel2::ValueItem::categoryName() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return "";
  return parent()->data(ROLE_CATEGORY_NAME).toString();
}

int CategoryTreeModel2::ValueItem::valueInt() const
{
  return valueInt_;
}

QString CategoryTreeModel2::ValueItem::valueString() const
{
  // "No Value" should return empty string here, not user-facing string
  if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
    return "";
  return valueString_;
}

Qt::ItemFlags CategoryTreeModel2::ValueItem::flags() const
{
  if (isRegExpApplied())
    return Qt::NoItemFlags;
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

QVariant CategoryTreeModel2::ValueItem::data(int role) const
{
  switch (role)
  {
  case Qt::DisplayRole:
  case Qt::EditRole:
  {
    QString returnString;
    if (!isUnlistedValueChecked())
      returnString = valueString_;
    else if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      returnString = tr("Has Value");
    else
      returnString = tr("Not %1").arg(valueString_);
    // Append the numeric count if specified
    if (numMatches_ >= 0)
      returnString = tr("%1 (%2)").arg(returnString).arg(numMatches_);
    return returnString;
  }

  case Qt::CheckStateRole:
    return checked_;

  case ROLE_SORT_STRING:
    if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      return QString("");
    return data(Qt::DisplayRole);

  case ROLE_EXCLUDE:
    return isUnlistedValueChecked();

  case ROLE_CATEGORY_NAME:
    return categoryName();

  case ROLE_REGEXP_STRING:
    // Parent node holds the RegExp string
    if (parent())
      return parent()->data(ROLE_REGEXP_STRING);
    break;

  default:
    break;
  }
  return QVariant();
}

bool CategoryTreeModel2::ValueItem::setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged)
{
  // Internally handle check/uncheck value.  For ROLE_REGEXP, rely on category parent
  if (role == Qt::CheckStateRole)
    return setCheckStateData_(value, filter, filterChanged);
  else if (role == ROLE_REGEXP_STRING && parent() != NULL)
    return parent()->setData(value, role, filter, filterChanged);
  filterChanged = false;
  return false;
}

bool CategoryTreeModel2::ValueItem::setCheckStateData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
{
  filterChanged = false;

  // If the edit sets us to same state, or disabled, then return early
  const Qt::CheckState newChecked = static_cast<Qt::CheckState>(value.toInt());
  if (newChecked == checked_ || !flags().testFlag(Qt::ItemIsEnabled))
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

void CategoryTreeModel2::ValueItem::setNumMatches(int matches)
{
  numMatches_ = matches;
}

int CategoryTreeModel2::ValueItem::numMatches() const
{
  return numMatches_;
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
    filter_(new simData::CategoryFilter(NULL)),
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
    else
    {
      // Should only happen in cases where EXCLUDE got changed, but no filter was edited
      assert(!index.parent().isValid());
      emit excludeEdited(item->nameInt(), item->isUnlistedValueChecked());
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
  if (filter_ != NULL && simplified == *filter_)
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
  const bool hadFilter = (filter_ != NULL && !filter_->isEmpty());
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
  // Value item is unchecked, unless the parent has a regular expression
  if (nameItem->isRegExpApplied())
  {
    auto* reObject = filter_->getRegExp(nameInt);
    if (reObject)
      valueItem->setChecked(reObject->match(valueItem->valueString().toStdString()));
  }

  // Get the index for the name (parent), and add this new valueItem into the tree
  const QModelIndex nameIndex = createIndex(categories_.indexOf(static_cast<CategoryItem*>(nameItem)), 0, nameItem);
  beginInsertRows(nameIndex, nameItem->childCount(), nameItem->childCount());
  nameItem->addChild(valueItem);
  endInsertRows();
}

void CategoryTreeModel2::processCategoryCounts(const simQt::CategoryCountResults& results)
{
  const int numCategories = categories_.size();
  int firstRowChanged = -1;
  int lastRowChanged = -1;
  const auto& allCats = results.allCategories;
  for (int k = 0; k < numCategories; ++k)
  {
    CategoryItem* categoryItem = categories_[k];
    const int nameInt = categoryItem->nameInt();

    // Might have a category added between when we fired off the call and when it finished
    const auto entry = allCats.find(nameInt);
    bool haveChange = false;

    // Updates the text for the category and its child values
    if (entry == allCats.end())
      haveChange = categoryItem->updateCounts(std::map<int, size_t>());
    else
      haveChange = categoryItem->updateCounts(entry->second);

    // Record the row for data changed
    if (haveChange)
    {
      if (firstRowChanged == -1)
        firstRowChanged = k;
      lastRowChanged = k;
    }
  }

  // Emit data changed
  if (firstRowChanged != -1)
    emit dataChanged(index(firstRowChanged, 0), index(lastRowChanged, 0));
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

struct CategoryTreeItemDelegate::ChildRects
{
  QRect background;
  QRect checkbox;
  QRect branch;
  QRect text;
  QRect excludeToggle;
  QRect regExpButton;
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

  if (r.excludeToggle.isValid())
  { // Draw the toggle switch for changing EXCLUDE and INCLUDE
    StyleOptionToggleSwitch switchOpt;
    ToggleSwitchPainter switchPainter;
    switchOpt.rect = r.excludeToggle;
    switchOpt.value = index.data(CategoryTreeModel2::ROLE_EXCLUDE).toBool();
    switchPainter.paint(switchOpt, painter);
  }

  if (r.regExpButton.isValid())
  { // Draw the RegExp text box
    QStyleOptionButton buttonOpt;
    buttonOpt.rect = r.regExpButton;
    buttonOpt.text = tr("RegExp...");
    buttonOpt.state = QStyle::State_Enabled;
    if (clickedElement_ == SE_REGEXP_BUTTON && clickedIndex_ == index)
      buttonOpt.state |= QStyle::State_Sunken;
    else
      buttonOpt.state |= QStyle::State_Raised;
    style->drawControl(QStyle::CE_PushButton, &buttonOpt, painter);
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
  if (opt.state.testFlag(QStyle::State_MouseOver) && opt.state.testFlag(QStyle::State_Enabled))
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
    if (clickedElement_ == SE_REGEXP_BUTTON)
      return true;
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
        QVariant oldState = index.data(CategoryTreeModel2::ROLE_EXCLUDE);
        if (index.flags().testFlag(Qt::ItemIsEnabled))
          model->setData(index, !oldState.toBool(), CategoryTreeModel2::ROLE_EXCLUDE);
        clickedIndex_ = QModelIndex();
        return true;
      }
      else if (clickedElement_ == SE_REGEXP_BUTTON)
      {
        // Need to talk to the tree itself to do the input GUI, so pass this off as a signal
        emit editRegExpClicked(index);
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
    // Ignore double click on the toggle, branch, and RegExp buttons, so that it doesn't cause expand/contract
    if (clickedElement_ == SE_EXCLUDE_TOGGLE || clickedElement_ == SE_BRANCH || clickedElement_ == SE_REGEXP_BUTTON)
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
    if (index.flags().testFlag(Qt::ItemIsEnabled))
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
    rects.excludeToggle = QRect();
    rects.regExpButton = QRect();

    // Text takes up everything to the right of the checkbox
    rects.text = rects.background.adjusted(TREE_INDENTATION, 0, 0, 0);
  }
  else
  {
    // Branch is the > or v indicator for expanding
    rects.branch = rects.background;
    rects.branch.setRight(rects.branch.left() + rects.branch.height());

    // Calculate the width given the rectangle of height, for the toggle switch
    const bool haveRegExp = !index.data(CategoryTreeModel2::ROLE_REGEXP_STRING).toString().isEmpty();
    if (haveRegExp)
    {
      rects.excludeToggle = QRect();
      rects.regExpButton = rects.background.adjusted(0, 1, -1, -1);
      rects.regExpButton.setLeft(rects.regExpButton.right() - 70);
    }
    else
    {
      rects.excludeToggle = rects.background.adjusted(0, 1, -1, -1);
      ToggleSwitchPainter switchPainter;
      StyleOptionToggleSwitch switchOpt;
      switchOpt.rect = rects.excludeToggle;
      const QSize toggleSize = switchPainter.sizeHint(switchOpt);
      // Set the left side appropriately
      rects.excludeToggle.setLeft(rects.excludeToggle.right() - toggleSize.width());
    }

    // Text takes up everything to the right of the branch button until the exclude toggle
    rects.text = rects.background;
    rects.text.setLeft(rects.branch.right());
    if (haveRegExp)
      rects.text.setRight(rects.regExpButton.left());
    else
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
  if (r.regExpButton.isValid() && r.regExpButton.contains(pos))
    return SE_REGEXP_BUTTON;
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
    const SubElement subElement = hit_(evt->pos(), option, index);
    if (subElement == SE_EXCLUDE_TOGGLE)
    {
      QToolTip::showText(evt->globalPos(), simQt::formatTooltip(tr("Exclude"),
        tr("When on, Exclude mode will omit all entities that match your selected values.<p>When off, the filter will match all entities that have one of your checked category values.")),
        view);
      return true;
    }
    else if (subElement == SE_REGEXP_BUTTON)
    {
      QToolTip::showText(evt->globalPos(), simQt::formatTooltip(tr("Set Regular Expression"),
        tr("A regular expression has been set for this category.  Use this button to change the category's regular expression.")),
        view);
      return true;
    }
  }
  return QStyledItemDelegate::helpEvent(evt, view, option, index);
}

/////////////////////////////////////////////////////////////////////////

CategoryFilterWidget2::CategoryFilterWidget2(QWidget* parent)
  : QWidget(parent),
    activeFiltering_(false),
    showEntityCount_(false),
    counter_(NULL),
    setRegExpAction_(NULL)
{
  setWindowTitle("Category Data Filter");
  setObjectName("CategoryFilterWidget2");

  treeModel_ = new simQt::CategoryTreeModel2(this);
  proxy_ = new simQt::CategoryProxyModel(this);
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

  setRegExpAction_ = new QAction(tr("Set Regular Expression..."), this);
  connect(setRegExpAction_, SIGNAL(triggered()), this, SLOT(setRegularExpression_()));
  clearRegExpAction_ = new QAction(tr("Clear Regular Expression"), this);
  connect(clearRegExpAction_, SIGNAL(triggered()), this, SLOT(clearRegularExpression_()));

  QAction* separator1 = new QAction(this);
  separator1->setSeparator(true);

  QAction* resetAction = new QAction(tr("Reset"), this);
  connect(resetAction, SIGNAL(triggered()), this, SLOT(resetFilter_()));
  QAction* separator2 = new QAction(this);
  separator2->setSeparator(true);

  QAction* collapseAction = new QAction(tr("Collapse Values"), this);
  connect(collapseAction, SIGNAL(triggered()), treeView_, SLOT(collapseAll()));
  collapseAction->setIcon(QIcon(":/simQt/images/Collapse.png"));

  QAction* expandAction = new QAction(tr("Expand Values"), this);
  connect(expandAction, SIGNAL(triggered()), treeView_, SLOT(expandAll()));
  expandAction->setIcon(QIcon(":/simQt/images/Expand.png"));

  treeView_->setContextMenuPolicy(Qt::CustomContextMenu);
  treeView_->addAction(setRegExpAction_);
  treeView_->addAction(clearRegExpAction_);
  treeView_->addAction(separator1);
  treeView_->addAction(resetAction);
  treeView_->addAction(separator2);
  treeView_->addAction(collapseAction);
  treeView_->addAction(expandAction);

  simQt::SearchLineEdit* search = new simQt::SearchLineEdit(this);
  search->setPlaceholderText(tr("Search Category Data"));

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setObjectName("CategoryFilterWidgetVBox");
  layout->setMargin(0);
  layout->addWidget(search);
  layout->addWidget(treeView_);

  connect(treeView_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu_(QPoint)));
  connect(treeModel_, SIGNAL(filterChanged(simData::CategoryFilter)), this, SIGNAL(filterChanged(simData::CategoryFilter)));
  connect(treeModel_, SIGNAL(filterEdited(simData::CategoryFilter)), this, SIGNAL(filterEdited(simData::CategoryFilter)));
  connect(treeModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToModel_(QModelIndex, int, int)));
  connect(proxy_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToProxy_(QModelIndex, int, int)));
  connect(proxy_, SIGNAL(modelReset()), treeView_, SLOT(expandAll()));
  connect(search, SIGNAL(textChanged(QString)), this, SLOT(expandAfterFilterEdited_(QString)));
  connect(search, SIGNAL(textChanged(QString)), proxy_, SLOT(setFilterText(QString)));
  connect(itemDelegate, SIGNAL(expandClicked(QModelIndex)), this, SLOT(toggleExpanded_(QModelIndex)));
  connect(itemDelegate, SIGNAL(editRegExpClicked(QModelIndex)), this, SLOT(showRegExpEditGui_(QModelIndex)));

  // Entity filtering is on by default
  setShowEntityCount(true);
}

CategoryFilterWidget2::~CategoryFilterWidget2()
{
}

void CategoryFilterWidget2::setDataStore(simData::DataStore* dataStore)
{
  treeModel_->setDataStore(dataStore);
  counter_->setFilter(categoryFilter());
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

void CategoryFilterWidget2::processCategoryCounts(const simQt::CategoryCountResults& results)
{
  treeModel_->processCategoryCounts(results);
}

bool CategoryFilterWidget2::showEntityCount() const
{
  return showEntityCount_;
}

void CategoryFilterWidget2::setShowEntityCount(bool fl)
{
  if (fl == showEntityCount_)
    return;

  showEntityCount_ = fl;
  // Clear out the old counter
  delete counter_;
  counter_ = NULL;

  // Create a new counter and configure it
  if (showEntityCount_)
  {
    counter_ = new simQt::AsyncCategoryCounter(this);
    connect(counter_, SIGNAL(resultsReady(simQt::CategoryCountResults)), this, SLOT(processCategoryCounts(simQt::CategoryCountResults)));
    connect(treeModel_, SIGNAL(filterChanged(simData::CategoryFilter)), counter_, SLOT(setFilter(simData::CategoryFilter)));
    connect(treeModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), counter_, SLOT(asyncCountEntities()));
    counter_->setFilter(categoryFilter());
  }
  else
  {
    treeModel_->processCategoryCounts(simQt::CategoryCountResults());
  }
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

void CategoryFilterWidget2::resetFilter_()
{
  // Create a new empty filter using same data store
  const simData::CategoryFilter newFilter(treeModel_->categoryFilter().getDataStore());
  treeModel_->setFilter(newFilter);

  // Tree would have sent out a changed signal, but not an edited signal (because we are
  // doing this programmatically).  That's OK, but we need to send out an edited signal.
  emit filterEdited(treeModel_->categoryFilter());
}

void CategoryFilterWidget2::showContextMenu_(const QPoint& point)
{
  QMenu contextMenu(this);
  contextMenu.addActions(treeView_->actions());

  // Mark the Set RegExp action enabled or disabled based on what you clicked on
  const QModelIndex idx = treeView_->indexAt(point);
  setRegExpAction_->setProperty("index", idx);
  setRegExpAction_->setEnabled(idx.isValid());
  // Mark the Clear RegExp action similarly
  clearRegExpAction_->setProperty("index", idx);
  clearRegExpAction_->setEnabled(idx.isValid() && !idx.data(CategoryTreeModel2::ROLE_REGEXP_STRING).toString().isEmpty());

  // Show the menu
  contextMenu.exec(treeView_->mapToGlobal(point));

  // Clear the index property and disable
  setRegExpAction_->setProperty("index", QVariant());
  setRegExpAction_->setEnabled(false);
  clearRegExpAction_->setProperty("index", idx);
  clearRegExpAction_->setEnabled(false);
}

void CategoryFilterWidget2::setRegularExpression_()
{
  // Make sure we have a sender and can pull out the index.  If not, return
  QObject* senderObject = sender();
  if (senderObject == NULL)
    return;
  QModelIndex index = senderObject->property("index").toModelIndex();
  if (index.isValid())
    showRegExpEditGui_(index);
}

void CategoryFilterWidget2::showRegExpEditGui_(const QModelIndex& index)
{
  // Grab category name and old regexp, then ask user for new value
  const QString oldRegExp = index.data(CategoryTreeModel2::ROLE_REGEXP_STRING).toString();
  const QString categoryName = index.data(CategoryTreeModel2::ROLE_CATEGORY_NAME).toString();

  // Create an input dialog on the stack so that we can set a What's This tip for more information
  QInputDialog inputDialog(this);
  inputDialog.setWhatsThis(tr(
"Regular expressions can be applied to categories in a filter.  Categories with regular expression filters will match only the values that match the regular expression."
"<p>This popup changes the regular expression value for the category '%1'."
"<p>An empty string can be used to clear the regular expression and return to normal matching mode.").arg(categoryName));
  inputDialog.setInputMode(QInputDialog::TextInput);
  inputDialog.setTextValue(oldRegExp);
  inputDialog.setWindowTitle(tr("Set Regular Expression"));
  inputDialog.setLabelText(tr("Set '%1' value regular expression:").arg(categoryName));

  // Execute the GUI and set the regexp
  if (inputDialog.exec() == QDialog::Accepted && inputDialog.textValue() != oldRegExp)
  {
    // index.model() is const because changes to the model might invalidate indices.  Since we know this
    // and no longer use the index after this call, it is safe to use const_cast here to use setData().
    QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, inputDialog.textValue(), CategoryTreeModel2::ROLE_REGEXP_STRING);
  }
}

void CategoryFilterWidget2::clearRegularExpression_()
{
  // Make sure we have a sender and can pull out the index.  If not, return
  QObject* senderObject = sender();
  if (senderObject == NULL)
    return;
  QModelIndex index = senderObject->property("index").toModelIndex();
  if (!index.isValid())
    return;
  // index.model() is const because changes to the model might invalidate indices.  Since we know this
  // and no longer use the index after this call, it is safe to use const_cast here to use setData().
  QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
  model->setData(index, QString(""), CategoryTreeModel2::ROLE_REGEXP_STRING);
}

}
