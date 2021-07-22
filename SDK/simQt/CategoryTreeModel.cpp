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
#include <QAbstractItemView>
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/DataStore.h"
#include "simQt/CategoryFilterCounter.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/RegExpImpl.h"
#include "simQt/Settings.h"
#include "simQt/CategoryTreeModel.h"

namespace simQt {

/** Lighter than lightGray, matches QPalette::Midlight */
static const QColor MIDLIGHT_BG_COLOR(227, 227, 227);
/** Breadcrumb's default fill color, used here for background brush on filter items that contribute to filter. */
static const QColor CONTRIBUTING_BG_COLOR(195, 225, 240); // Light gray with a hint of blue
/** Locked settings keys */
static const QString LOCKED_SETTING = "LockedCategories";
/** Locked settings meta data to define it as private */
static const simQt::Settings::MetaData LOCKED_SETTING_METADATA(Settings::STRING_LIST, "", "", Settings::PRIVATE);


/////////////////////////////////////////////////////////////////////////

template <typename T>
IndexedPointerContainer<T>::IndexedPointerContainer()
{
}

template <typename T>
IndexedPointerContainer<T>::~IndexedPointerContainer()
{
  deleteAll();
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
void IndexedPointerContainer<T>::deleteAll()
{
  for (auto i = vec_.begin(); i != vec_.end(); ++i)
    delete *i;
  vec_.clear();
  itemToIndex_.clear();
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
class CategoryTreeModel::CategoryItem : public TreeItem
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
  /** Tracks whether this category item is locked */
  bool locked_;
};

/////////////////////////////////////////////////////////////////////////

/** Represents a leaf node in tree, showing a category value. */
class CategoryTreeModel::ValueItem : public TreeItem
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
  : parent_(nullptr)
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
  if (parent_ == nullptr)
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
  assert(item != nullptr);
  // Assertion failure means that item is inserted more than once.
  assert(item->parent() == nullptr);

  // Set the parent and save the item in our children vector.
  item->parent_ = this;
  children_.push_back(item);
}

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel::CategoryItem::CategoryItem(const simData::CategoryNameManager& nameManager, int nameInt)
  : categoryName_(QString::fromStdString(nameManager.nameIntToString(nameInt))),
    nameInt_(nameInt),
    unlistedValue_(false),
    contributesToFilter_(false),
    font_(nullptr),
    locked_(false)
{
}

bool CategoryTreeModel::CategoryItem::isUnlistedValueChecked() const
{
  return unlistedValue_;
}

bool CategoryTreeModel::CategoryItem::isRegExpApplied() const
{
  return !regExpString_.isEmpty();
}

int CategoryTreeModel::CategoryItem::nameInt() const
{
  return nameInt_;
}

QString CategoryTreeModel::CategoryItem::categoryName() const
{
  return categoryName_;
}

Qt::ItemFlags CategoryTreeModel::CategoryItem::flags() const
{
  return Qt::ItemIsEnabled;
}

QVariant CategoryTreeModel::CategoryItem::data(int role) const
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
  case ROLE_LOCKED_STATE:
    return locked_;
  case Qt::BackgroundRole:
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

bool CategoryTreeModel::CategoryItem::setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged)
{
  if (role == ROLE_EXCLUDE)
    return setExcludeData_(value, filter, filterChanged);
  else if (role == ROLE_REGEXP_STRING)
    return setRegExpStringData_(value, filter, filterChanged);
  else if (role == ROLE_LOCKED_STATE && locked_ != value.toBool())
  {
    locked_ = value.toBool();
    filterChanged = true;
    return true;
  }
  filterChanged = false;
  return false;
}

bool CategoryTreeModel::CategoryItem::setExcludeData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
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

bool CategoryTreeModel::CategoryItem::setRegExpStringData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
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

bool CategoryTreeModel::CategoryItem::recalcContributionTo(const simData::CategoryFilter& filter)
{
  // First check the regular expression.  If there's a regexp, then this category definitely contributes
  const bool newValue = filter.nameContributesToFilter(nameInt_);
  if (newValue == contributesToFilter_)
    return false;
  contributesToFilter_ = newValue;
  return true;
}

void CategoryTreeModel::CategoryItem::setFont(QFont* font)
{
  font_ = font;
}

bool CategoryTreeModel::CategoryItem::setChildChecks_(const simData::RegExpFilter* reFilter)
{
  bool hasChange = false;
  const int count = childCount();
  for (int k = 0; k < count; ++k)
  {
    // Test the EditRole, which is used because it omits the # count (e.g. "Friendly (1)")
    ValueItem* valueItem = static_cast<ValueItem*>(child(k));
    const bool matches = reFilter != nullptr && reFilter->match(valueItem->valueString().toStdString());
    if (matches != valueItem->isChecked())
    {
      valueItem->setChecked(matches);
      hasChange = true;
    }
  }
  return hasChange;
}

int CategoryTreeModel::CategoryItem::updateTo(const simData::CategoryFilter& filter)
{
  // Update the category if it has a RegExp
  const QString oldRegExp = regExpString_;
  const auto* regExpObject = filter.getRegExp(nameInt_);
  regExpString_ = (regExpObject != nullptr ? QString::fromStdString(filter.getRegExpPattern(nameInt_)) : "");
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

void CategoryTreeModel::CategoryItem::updateFilter_(const ValueItem& valueItem, simData::CategoryFilter& filter) const
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

int CategoryTreeModel::CategoryItem::updateValueItem_(ValueItem& valueItem, const simData::CategoryFilter::ValuesCheck& checks) const
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

bool CategoryTreeModel::CategoryItem::updateCounts(const std::map<int, size_t>& valueToCountMap) const
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

CategoryTreeModel::ValueItem::ValueItem(const simData::CategoryNameManager& nameManager, int nameInt, int valueInt)
  : nameInt_(nameInt),
    valueInt_(valueInt),
    numMatches_(-1),
    checked_(Qt::Unchecked),
    valueString_(QString::fromStdString(nameManager.valueIntToString(valueInt)))
{
}

bool CategoryTreeModel::ValueItem::isUnlistedValueChecked() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return false;
  return parent()->isUnlistedValueChecked();
}

bool CategoryTreeModel::ValueItem::isRegExpApplied() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return false;
  return parent()->isRegExpApplied();
}

int CategoryTreeModel::ValueItem::nameInt() const
{
  return nameInt_;
}

QString CategoryTreeModel::ValueItem::categoryName() const
{
  // Assertion failure means we have orphan value items
  assert(parent());
  if (!parent())
    return "";
  return parent()->data(ROLE_CATEGORY_NAME).toString();
}

int CategoryTreeModel::ValueItem::valueInt() const
{
  return valueInt_;
}

QString CategoryTreeModel::ValueItem::valueString() const
{
  // "No Value" should return empty string here, not user-facing string
  if (valueInt_ == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
    return "";
  return valueString_;
}

Qt::ItemFlags CategoryTreeModel::ValueItem::flags() const
{
  if (isRegExpApplied())
    return Qt::NoItemFlags;
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

QVariant CategoryTreeModel::ValueItem::data(int role) const
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
    // Append the numeric count if specified -- only if in include mode, and NOT in exclude mode
    if (numMatches_ >= 0 && !isUnlistedValueChecked())
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

  case ROLE_LOCKED_STATE:
    // Parent node holds the lock state
    if (parent())
      return parent()->data(ROLE_LOCKED_STATE);
    break;

  default:
    break;
  }
  return QVariant();
}

bool CategoryTreeModel::ValueItem::setData(const QVariant& value, int role, simData::CategoryFilter& filter, bool& filterChanged)
{
  // Internally handle check/uncheck value.  For ROLE_REGEXP and ROLE_LOCKED_STATE, rely on category parent
  if (role == Qt::CheckStateRole)
    return setCheckStateData_(value, filter, filterChanged);
  else if (role == ROLE_REGEXP_STRING && parent() != nullptr)
    return parent()->setData(value, role, filter, filterChanged);
  else if (role == ROLE_LOCKED_STATE && parent() != nullptr)
    return parent()->setData(value, role, filter, filterChanged);
  filterChanged = false;
  return false;
}

bool CategoryTreeModel::ValueItem::setCheckStateData_(const QVariant& value, simData::CategoryFilter& filter, bool& filterChanged)
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
  if (parentTree)
    parentTree->recalcContributionTo(filter);

  filterChanged = true;
  return true;
}

void CategoryTreeModel::ValueItem::setChecked(bool value)
{
  checked_ = (value ? Qt::Checked : Qt::Unchecked);
}

bool CategoryTreeModel::ValueItem::isChecked() const
{
  return checked_ == Qt::Checked;
}

void CategoryTreeModel::ValueItem::setNumMatches(int matches)
{
  numMatches_ = matches;
}

int CategoryTreeModel::ValueItem::numMatches() const
{
  return numMatches_;
}

/////////////////////////////////////////////////////////////////////////

/// Monitors for category data changes, calling methods in CategoryTreeModel.
class CategoryTreeModel::CategoryFilterListener : public simData::CategoryNameManager::Listener
{
public:
  /// Constructor
  explicit CategoryFilterListener(CategoryTreeModel& parent)
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
  CategoryTreeModel& parent_;
};

/////////////////////////////////////////////////////////////////////////

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

  const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  const QString itemText = index.data(Qt::DisplayRole).toString();

  // include items that pass the filter
  if (itemText.contains(filter_, Qt::CaseInsensitive))
    return true;

  // include items whose parent passes the filter, but not if parent is root "All Categories" item
  if (sourceParent.isValid())
  {
    const QString parentText = sourceParent.data(Qt::DisplayRole).toString();

    if (parentText.contains(filter_, Qt::CaseInsensitive))
        return true;
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

/////////////////////////////////////////////////////////////////////////

CategoryTreeModel::CategoryTreeModel(QObject* parent)
  : QAbstractItemModel(parent),
    dataStore_(nullptr),
    filter_(new simData::CategoryFilter(nullptr)),
    categoryFont_(new QFont),
    settings_(nullptr)
{
  listener_.reset(new CategoryFilterListener(*this));

  // Increase the point size on the category
  categoryFont_->setPointSize(categoryFont_->pointSize() + 4);
  categoryFont_->setBold(true);
}

CategoryTreeModel::~CategoryTreeModel()
{
  categories_.deleteAll();
  categoryIntToItem_.clear();
  delete categoryFont_;
  categoryFont_ = nullptr;
  delete filter_;
  filter_ = nullptr;
  if (dataStore_)
    dataStore_->categoryNameManager().removeListener(listener_);
}

QModelIndex CategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  // Category items have no parent in the model
  if (!parent.isValid())
    return createIndex(row, column, categories_[row]);
  // Has a parent: must be a value item
  TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
  // Item was not made correctly, check index()
  assert(parentItem != nullptr);
  return createIndex(row, column, parentItem->child(row));
}

QModelIndex CategoryTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid() || !child.internalPointer())
    return QModelIndex();

  // Child could be a category (no parent) or a value (category parent)
  const TreeItem* childItem = static_cast<TreeItem*>(child.internalPointer());
  TreeItem* parentItem = childItem->parent();
  if (parentItem == nullptr) // child is a category; no parent
    return QModelIndex();
  return createIndex(categories_.indexOf(static_cast<CategoryItem*>(parentItem)), 0, parentItem);
}

int CategoryTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    if (parent.column() != 0)
      return 0;
    TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
    return (parentItem == nullptr) ? 0 : parentItem->childCount();
  }
  return categories_.size();
}

int CategoryTreeModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

QVariant CategoryTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid() || !index.internalPointer())
    return QVariant();
  const TreeItem* treeItem = static_cast<TreeItem*>(index.internalPointer());
  return treeItem->data(role);
}

QVariant CategoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
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

Qt::ItemFlags CategoryTreeModel::flags(const QModelIndex& index) const
{
  if (!index.isValid() || !index.internalPointer())
    return Qt::NoItemFlags;
  TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
  return item->flags();
}

bool CategoryTreeModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  // Ensure we have a valid index with a valid TreeItem pointer
  if (!idx.isValid() || !idx.internalPointer())
    return QAbstractItemModel::setData(idx, value, role);

  // nullptr filter means the tree should be empty, so we shouldn't get setData()...
  TreeItem* item = static_cast<TreeItem*>(idx.internalPointer());
  assert(filter_ && item);
  bool wasEdited = false;
  const bool rv = item->setData(value, role, *filter_, wasEdited);

  // update locked setting for this category if it is a category item and this is a locked state update
  if (settings_ && item->childCount() > 0 && role == ROLE_LOCKED_STATE)
  {
    QStringList lockedCategories = settings_->value(settingsKey_, LOCKED_SETTING_METADATA).toStringList();
    lockedCategories.removeOne(item->categoryName());
    if (value.toBool())
      lockedCategories.push_back(item->categoryName());
    settings_->setValue(settingsKey_, lockedCategories);
  }

  // Logic below needs to change if this assert triggers.  Basically, GUI may
  // update without the filter updating, but not vice versa.
  assert(rv || !wasEdited);
  if (rv)
  {
    // Update the GUI
    emit dataChanged(idx, idx);

    // Alert users who are listening
    if (wasEdited)
    {
      // Parent index, if it exists, is a category and might have updated its color data()
      const QModelIndex parentIndex = idx.parent();
      if (parentIndex.isValid())
        emit dataChanged(parentIndex, parentIndex);
      emitChildrenDataChanged_(idx);

      emit filterChanged(*filter_);
      emit filterEdited(*filter_);
    }
    else
    {
      // Should only happen in cases where EXCLUDE got changed, but no filter was edited
      assert(!idx.parent().isValid());
      emitChildrenDataChanged_(idx);
      emit excludeEdited(item->nameInt(), item->isUnlistedValueChecked());
    }
  }
  return rv;
}

void CategoryTreeModel::setFilter(const simData::CategoryFilter& filter)
{
  // Check the data store; if it's set in filter and different from ours, update
  if (filter.getDataStore() && filter.getDataStore() != dataStore_)
    setDataStore(filter.getDataStore());

  // Avoid no-op
  simData::CategoryFilter simplified(filter);
  simplified.simplify();
  if (filter_ != nullptr && simplified == *filter_)
    return;

  // Do a two step assignment so that we don't automatically get auto-update
  if (filter_ == nullptr)
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

const simData::CategoryFilter& CategoryTreeModel::categoryFilter() const
{
  // Precondition of this method is that data store was set; filter must be non-nullptr
  assert(filter_);
  return *filter_;
}

void CategoryTreeModel::setDataStore(simData::DataStore* dataStore)
{
  if (dataStore_ == dataStore)
    return;

  // Update the listeners on name manager as we change it
  if (dataStore_ != nullptr)
    dataStore_->categoryNameManager().removeListener(listener_);
  dataStore_ = dataStore;
  if (dataStore_ != nullptr)
    dataStore_->categoryNameManager().addListener(listener_);

  beginResetModel();

  // Clear out the internal storage on the tree
  categories_.deleteAll();
  categoryIntToItem_.clear();

  // Clear out the internal filter object
  const bool hadFilter = (filter_ != nullptr && !filter_->isEmpty());
  delete filter_;
  filter_ = nullptr;
  if (dataStore_)
  {
    filter_ = new simData::CategoryFilter(dataStore_);
    const simData::CategoryNameManager& nameManager = dataStore_->categoryNameManager();

    // Populate the GUI
    std::vector<int> nameInts;
    nameManager.allCategoryNameInts(nameInts);

    QString settingsKey;
    QStringList lockedCategories;
    if (settings_)
      lockedCategories = settings_->value(settingsKey_, LOCKED_SETTING_METADATA).toStringList();

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

      // check settings to determine if newly added categories should be locked
      if (settings_)
        updateLockedState_(lockedCategories, *category);
    }
  }

  // Model reset is done
  endResetModel();

  // Alert listeners if we have a new filter
  if (hadFilter && filter_)
    emit filterChanged(*filter_);
}

void CategoryTreeModel::setSettings(Settings* settings, const QString& settingsKeyPrefix)
{
  settings_ = settings;
  settingsKey_ = settingsKeyPrefix + "/" + LOCKED_SETTING;

  if (!settings_)
    return;

  // check settings to determine if newly added categories should be locked
  QStringList lockedCategories = settings_->value(settingsKey_, LOCKED_SETTING_METADATA).toStringList();
  for (int i = 0; i < categories_.size(); ++i)
  {
    updateLockedState_(lockedCategories, *categories_[i]);
  }
}

void CategoryTreeModel::clearTree_()
{
  beginResetModel();
  categories_.deleteAll();
  categoryIntToItem_.clear();
  // need to manually clear the filter_ since auto update was turned off
  filter_->clear();
  endResetModel();
}

void CategoryTreeModel::addName_(int nameInt)
{
  assert(dataStore_ != nullptr);

  // Create the tree item for the category
  const auto& nameManager = dataStore_->categoryNameManager();
  CategoryItem* category = new CategoryItem(nameManager, nameInt);
  category->setFont(categoryFont_);
  // check settings to determine if newly added categories should be locked
  if (settings_)
  {
    QStringList lockedCategories = settings_->value(settingsKey_, LOCKED_SETTING_METADATA).toStringList();
    updateLockedState_(lockedCategories, *category);
  }
  // Debug mode: Validate that there are no values in that category yet.  If this section
  // of code fails, then we'll need to add ValueItem entries for the category on creation.
#ifndef NDEBUG
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

CategoryTreeModel::CategoryItem* CategoryTreeModel::findNameTree_(int nameInt) const
{
  auto i = categoryIntToItem_.find(nameInt);
  return (i == categoryIntToItem_.end()) ? nullptr : i->second;
}

void CategoryTreeModel::updateLockedState_(const QStringList& lockedCategories, CategoryItem& category)
{
  if (!lockedCategories.contains(category.categoryName()))
    return;
  bool wasChanged = false;
  category.setData(true, CategoryTreeModel::ROLE_LOCKED_STATE, *filter_, wasChanged);
}

void CategoryTreeModel::addValue_(int nameInt, int valueInt)
{
  // Find the parent item
  TreeItem* nameItem = findNameTree_(nameInt);
  // Means we got a category that we don't know about; shouldn't happen.
  assert(nameItem);
  if (nameItem == nullptr)
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

void CategoryTreeModel::processCategoryCounts(const simQt::CategoryCountResults& results)
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

void CategoryTreeModel::emitChildrenDataChanged_(const QModelIndex& parent)
{
  // Change all children
  const int numRows = rowCount(parent);
  const int numCols = columnCount(parent);
  if (numRows == 0 || numCols == 0)
    return;
  emit dataChanged(index(0, 0, parent), index(numRows - 1, numCols - 1, parent));
}

}
