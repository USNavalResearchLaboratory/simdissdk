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
#include <algorithm>
#include <QApplication>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/DataStore.h"
#include "simQt/EntityCategoryFilter.h"
#include "simQt/QtFormatting.h"
#include "simQt/CategoryDataBreadcrumbs.h"

/** @file CategoryDataBreadcrumbs.cpp
 * Category data breadcrumbs widget is a QWidget that exposes many settings for configuring
 * a category filter display.  This is intended to be a quick-look, easily editable display
 * for seeing the current state of a category data filter.  Internally this wraps a QListWidget
 * with a styled item delegate to draw items with a close button and a rounded rectangle.
 *
 * The delegate is responsible both for the drawing of the items as well as the unique mouse
 * behavior with the close button.  It detects the position of the mouse relative to the
 * size of the icon button and will draw a hovered QToolButton-like rectangle when the mouse
 * is over the icon.  It also detects mouse clicks and will emit the closeClicked() signal.
 * It is the responsibility of the CategoryDataBreadcrumbs widget itself to intercept the
 * signal and actually remove the item.
 *
 * The widget was designed to fit wide-and-short in a layout.  The minimumSizeHint() and
 * sizeHint() are overridden to give a minimum height that clamps the vertical size of the
 * widget to however many rows are currently displayed.  This allows layouts to grow when
 * this widget requires an additional row, and then to shrink back into place once rows
 * are removed.  There is a minimum size of 1 row.  When an empty filter is displayed, the
 * widget reserves room for 1 row.
 */

namespace simQt {

// Unnamed enumeration of roles for data in the tree widget
enum
{
  /// String of the category name
  ROLE_CATEGORY_NAME = Qt::UserRole,
  /// Integer value for the category name
  ROLE_NAME_INT,
  /// Integer value for the value.  May be unset if the item represents a whole category
  ROLE_VALUE_INT,
  /// Indicates the current state flag for the value in the category
  ROLE_IS_CHECKED,
  /// Indicates whether the alternate fill color should be used for breadcrumbs
  ROLE_USE_ALT_FILL_COLOR,
  /// Contains a string that can be used for sorting purposes
  ROLE_SORT_STRING
};

/// Maximum number of items in the HTML list; limiting to keep size of tooltip down
static const size_t MAX_ITEMS_IN_TOOLTIP = 25;

////////////////////////////////////////////////////////////////////

/** Simple sorter on category names for use with std::stable_sort() on a vector. */
class CategoryNameSorter
{
public:
  explicit CategoryNameSorter(const simData::CategoryNameManager& nameManager)
    : nameManager_(nameManager)
  {
  }

  /** Less-than operator does a straight name compare with the category name manager */
  bool operator()(int left, int right) const
  {
    return nameManager_.nameIntToString(left) < nameManager_.nameIntToString(right);
  }

private:
  const simData::CategoryNameManager& nameManager_;
};

////////////////////////////////////////////////////////////////////

/** Simple custom QListWidgetItem that allows for sorting between items using a sort role */
class SortedListWidgetItem : public QListWidgetItem
{
public:
  explicit SortedListWidgetItem(const QString& text)
    : QListWidgetItem(text)
  {
  }

  /** Override operator<() to sort based on the unique role, presumably externally set. */
  virtual bool operator<(const QListWidgetItem& other) const
  {
    return data(ROLE_SORT_STRING).toString() < other.data(ROLE_SORT_STRING).toString();
  }
};

////////////////////////////////////////////////////////////////////

CloseableItemDelegate::Style::Style()
  : outlinePen(QColor(188, 195, 199, 255), 1.5), // Grayish
    fillColor(QColor(195, 225, 240, 255)), // Light gray with a hint of blue
    altFillColor(QColor(161, 212, 237, 255)), // Slightly darker blue
    textColor(Qt::black),
    itemMargins(2, 2, 2, 2),
    textPadding(2, 0, 2, 0),
    iconPadding(2, 2, 4, 2),
    icon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton)),
    iconSize(11, 11)
{
}

////////////////////////////////////////////////////////////////////

CloseableItemDelegate::CloseableItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

CloseableItemDelegate::~CloseableItemDelegate()
{
}

QRectF CloseableItemDelegate::calcIconRect_(const QStyleOptionViewItem& opt) const
{
  const double x = opt.rect.right() - style_.itemMargins.right() - style_.iconPadding.right() - style_.iconSize.width();
  const double y = opt.rect.y() + ((opt.rect.height() - style_.iconSize.height()) / 2.0) +
    (style_.iconPadding.top() - style_.iconPadding.bottom());
  const QRectF iconRect(x, y, style_.iconSize.width(), style_.iconSize.height());
  // Expand by 1 pixel in each direction to account for frame size
  return iconRect.adjusted(-1, -1, 1, 1);
}

void CloseableItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& inOption, const QModelIndex& index) const
{
  painter->save();

  const QSize textSize = QStyledItemDelegate::sizeHint(inOption, index);

  // Calculate rectangles for the item, text, and icon
  const QRect itemRect(inOption.rect.left() + style_.itemMargins.left(),
    inOption.rect.top() + style_.itemMargins.top(),
    inOption.rect.width() - style_.itemMargins.left() - style_.itemMargins.right(),
    inOption.rect.height() - style_.itemMargins.top() - style_.itemMargins.bottom());
  const QRect textRect(itemRect.left() + style_.textPadding.left(),
    itemRect.top() + style_.textPadding.top(),
    itemRect.width() - style_.textPadding.right() - style_.iconPadding.left() - style_.iconPadding.right() - style_.iconSize.width(),
    itemRect.height() - style_.textPadding.top() - style_.textPadding.bottom());
  const QRect iconRect = calcIconRect_(inOption).toRect();

  { // Draw a rounded rectangle
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(itemRect, style_.rectangleRadiusX, style_.rectangleRadiusY);
    painter->setPen(style_.outlinePen);
    QColor fillColor = (index.data(ROLE_USE_ALT_FILL_COLOR).toBool() ? style_.altFillColor : style_.fillColor);
    painter->fillPath(path, fillColor);
    painter->drawPath(path);
  }

  { // Draw the text for the list item
    QStyleOptionViewItem textOpt(inOption);
    initStyleOption(&textOpt, index);
    textOpt.showDecorationSelected = false;
    textOpt.palette.setBrush(QPalette::Text, QBrush(style_.textColor));
    textOpt.rect = textRect;
    textOpt.state = textOpt.state & ~QStyle::State_HasFocus;
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &textOpt, painter);
  }

  { // Draw the toolbar button for the close
    QStyleOptionToolButton tbOpt;
    tbOpt.features = QStyleOptionToolButton::None;
    tbOpt.toolButtonStyle = Qt::ToolButtonIconOnly;
    // Only show the tool button (no arrows or anything)
    tbOpt.subControls = QStyle::SC_ToolButton;
    tbOpt.rect = iconRect;

    tbOpt.icon = style_.icon;
    tbOpt.iconSize = style_.iconSize;

    // Turn off state flags that we manage ourselves
    tbOpt.state = inOption.state & ~QStyle::State_HasFocus;
    tbOpt.state = tbOpt.state & ~QStyle::State_MouseOver;
    // Turn on auto-raise
    tbOpt.state |= QStyle::State_AutoRaise;
    if (index == hoverIndex_)
    {
      tbOpt.state |= QStyle::State_MouseOver;
      // If pressed index is invalid, then we are awaiting a press; show as raised
      if (!pressedIndex_.isValid())
        tbOpt.state |= QStyle::State_Raised;
      // If pressed index matches, then we are awaiting a release; show as sunken
      else if (pressedIndex_ == index)
        tbOpt.state |= QStyle::State_Sunken;
    }
    QApplication::style()->drawComplexControl(QStyle::CC_ToolButton, &tbOpt, painter);
  }

  // Restore the painter state
  painter->restore();
}

QSize CloseableItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QSize textSize = QStyledItemDelegate::sizeHint(option, index);
  // Adjust width by the margin of the item, and the padding of the icon and text
  textSize.rwidth() += style_.iconSize.width() +
    style_.itemMargins.left() + style_.itemMargins.right() +
    style_.textPadding.left() + style_.textPadding.right() +
    style_.iconPadding.left() + style_.iconPadding.right();
  // Adjust height by the margin of the item, and the padding of the text alone.
  // Icon padding does not contribute to overall size, to simplify code.
  textSize.rheight() +=
    style_.itemMargins.top() + style_.itemMargins.bottom() +
    style_.textPadding.top() + style_.textPadding.bottom();
  return textSize;
}

bool CloseableItemDelegate::mousePressEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  // Determine whether the mouse is inside the close button
  const QRectF closeRect = QRectF(calcIconRect_(option));
  const bool insideCloseButton = closeRect.contains(evt->pos());

  if (insideCloseButton)
    pressedIndex_ = index;
  else
    pressedIndex_ = QModelIndex();

  return true;
}

bool CloseableItemDelegate::mouseReleaseEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  // Determine whether the mouse is inside the close button
  const QRectF closeRect = QRectF(calcIconRect_(option));
  const bool insideCloseButton = closeRect.contains(evt->pos());

  // Detect whether it counts as a click
  const bool click = (index == pressedIndex_ && index.isValid() && insideCloseButton && evt->button() == Qt::LeftButton);
  pressedIndex_ = QModelIndex();
  if (click)
    Q_EMIT closeClicked(index);

  return true;
}

bool CloseableItemDelegate::mouseMoveEvent_(QMouseEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  // Determine whether the mouse is inside the close button
  const QRectF closeRect = QRectF(calcIconRect_(option));
  const bool insideCloseButton = closeRect.contains(evt->pos());

  // Did hover change?  If so, return true and update the hoverIndex_
  if (insideCloseButton && hoverIndex_ != index)
  {
    hoverIndex_ = index;
    return true;
  }
  else if (!insideCloseButton && hoverIndex_.isValid())
  {
    hoverIndex_ = QModelIndex();
    return true;
  }
  return false;
}

bool CloseableItemDelegate::editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  // We only care about mouse events
  QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(evt);
  if (mouseEvent == nullptr)
    return false;

  // Farm off to helper functions
  if (mouseEvent->type() == QEvent::MouseButtonPress)
    return mousePressEvent_(mouseEvent, model, option, index);
  if (mouseEvent->type() == QEvent::MouseButtonRelease)
    return mouseReleaseEvent_(mouseEvent, model, option, index);
  if (mouseEvent->type() == QEvent::MouseMove)
    return mouseMoveEvent_(mouseEvent, model, option, index);
  return false;
}

CloseableItemDelegate::Style& CloseableItemDelegate::style()
{
  return style_;
}

const CloseableItemDelegate::Style& CloseableItemDelegate::style() const
{
  return style_;
}

////////////////////////////////////////////////////////////////////

class CategoryDataBreadcrumbs::FilterClearListener : public simData::CategoryNameManager::Listener
{
public:
  explicit FilterClearListener(CategoryDataBreadcrumbs& parent)
    : parent_(parent)
  {
  }

  ~FilterClearListener()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex)
  {
    // noop
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex)
  {
    // noop
  }

  /// Invoked when all data is cleared
  virtual void onClear()
  {
    parent_.rebuildList_();
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing()
  {
    // noop
  }

private:
  CategoryDataBreadcrumbs& parent_;
};

////////////////////////////////////////////////////////////////////

CategoryDataBreadcrumbs::CategoryDataBreadcrumbs(QWidget* parent)
  : QWidget(parent),
    emptyText_(tr("No active category filter"))
{
  listWidget_ = new QListWidget(this);
  listWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  listWidget_->setContentsMargins(0, 0, 0, 0);
  listWidget_->setFocusPolicy(Qt::NoFocus);
  // Mouse tracking is required for highlighting the item close X button
  listWidget_->setMouseTracking(true);
  // Left to right flow with wrapping to new lines
  listWidget_->setFlow(QListView::LeftToRight);
  listWidget_->setWrapping(true);
  listWidget_->setResizeMode(QListView::Adjust);
  // No selection or editing; turn off scroll bar
  listWidget_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  listWidget_->setSelectionMode(QAbstractItemView::NoSelection);
  listWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Set our own size policy
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sizePolicy.setHeightForWidth(true);
  setSizePolicy(sizePolicy);

  // See-through background with no border
  QPalette palette;
  palette.setColor(QPalette::Base, QColor(255, 255, 255, 0));
  listWidget_->setPalette(palette);
  listWidget_->setFrameShape(QFrame::NoFrame);

  // Create an item delegate that will draw the filter settings
  itemDelegate_ = new CloseableItemDelegate(this);
  listWidget_->setItemDelegate(itemDelegate_);
  connect(itemDelegate_, &CloseableItemDelegate::closeClicked, this, &CategoryDataBreadcrumbs::removeFilter_);
  // Create an item delegate that has no decorations that we can use when we don't want close button
  plainDelegate_ = new QStyledItemDelegate(this);

  // Create a layout and add the list widget to that layout
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(listWidget_);

  // Set the list contents
  rebuildList_();

  listener_.reset(new FilterClearListener(*this));
}

CategoryDataBreadcrumbs::~CategoryDataBreadcrumbs()
{
  if (filter_ && filter_->getDataStore())
    filter_->getDataStore()->categoryNameManager().removeListener(listener_);
  listener_.reset();
  delete filter_;
}

void CategoryDataBreadcrumbs::resizeEvent(QResizeEvent* evt)
{
  QWidget::resizeEvent(evt);

  validHints_ = false;
  updateGeometry();
  update(contentsRect());
}

QSize CategoryDataBreadcrumbs::minimumSizeHint() const
{
  if (validHints_)
    return minimumSizeHint_;

  ensurePolished();

  // Width is arbitrary, but matches QAbstractScrollArea::viewportSizeHint()
  QSize size(fontMetrics().height() * 6, 0);
  // Note that this is only for cases where we have drawn items with the custom item delegate
  if (listWidget_->count() && listWidget_->itemDelegate() == itemDelegate_)
  {
    // calculate the position of the bottom item
    const QRect r2 = listWidget_->visualItemRect(listWidget_->item(listWidget_->count() - 1));
    size.setHeight(r2.bottom() + 1);
  }
  else if (!hideWhenEmpty_)
  {
    // Ask for the height/width of an invalid item
    QStyleOptionViewItem opt;
    opt.fontMetrics = fontMetrics();
    opt.font = font();
    const QSize itemSize = itemDelegate_->sizeHint(opt, QModelIndex());
    // Adjust the return values slightly, based on testing
    size.setHeight(itemSize.height() + 2);
  }

  // Cache the size hints for later queries
  minimumSizeHint_ = size;
  sizeHint_ = size;
  validHints_ = true;

  return size;
}

QSize CategoryDataBreadcrumbs::sizeHint() const
{
  // Recalculate size hints
  if (!validHints_)
    minimumSizeHint();
  return sizeHint_;
}

void CategoryDataBreadcrumbs::setFilter(const simData::CategoryFilter& filter)
{
  // Avoid deleting the input
  if (filter_ == &filter)
    return;

  if (filter_ && filter_->getDataStore())
    filter_->getDataStore()->categoryNameManager().removeListener(listener_);

  // Recreate our filter
  delete filter_;
  filter_ = new simData::CategoryFilter(filter);
  filter_->simplify();
  if (filter_->getDataStore())
    filter_->getDataStore()->categoryNameManager().addListener(listener_);

  rebuildList_();
}

void CategoryDataBreadcrumbs::rebuildList_()
{
  // Clear out the list and start fresh.  This could be optimized in future passes.
  listWidget_->clear();

  // Add items for each name into the list
  if (filter_)
  {
    std::vector<int> names;
    filter_->getNames(names);

    // Sort breadcrumbs by category name; must be done here too to allow alternating rows to work
    if (filter_->getDataStore())
    {
      CategoryNameSorter nameSorter(filter_->getDataStore()->categoryNameManager());
      std::stable_sort(names.begin(), names.end(), nameSorter);
    }

    bool useAltFillColor = false;
    for (auto i = names.begin(); i != names.end(); ++i)
    {
      addNameToList_(*i, useAltFillColor);
      useAltFillColor = !useAltFillColor;
    }

    // Sort the list
    listWidget_->sortItems();
  }

  // Make sure that the "no valid item" notice is shown if needed
  addNoValidItemIfEmptyList_();

  // Invalidate the size hints because number of items changed
  validHints_ = false;
  updateGeometry();
  update();
}

void CategoryDataBreadcrumbs::addNoValidItemIfEmptyList_()
{
  // If there are no items in the list, add an item to tell the user
  if (listWidget_->count() == 0)
  {
    QListWidgetItem* emptyItem = new QListWidgetItem(emptyText_, listWidget_);
    emptyItem->setFlags(Qt::ItemIsEnabled);
    listWidget_->setItemDelegate(plainDelegate_);
  }
  else
    listWidget_->setItemDelegate(itemDelegate_);
}

QListWidgetItem* CategoryDataBreadcrumbs::addNameItem_(const QString& categoryName, int nameInt, bool useAltFillColor)
{
  QListWidgetItem* newItem = new SortedListWidgetItem(tr("[%1]").arg(categoryName));
  newItem->setData(ROLE_CATEGORY_NAME, categoryName);
  newItem->setData(ROLE_NAME_INT, nameInt);
  newItem->setData(ROLE_IS_CHECKED, true);
  newItem->setData(ROLE_USE_ALT_FILL_COLOR, useAltFillColor);
  newItem->setData(ROLE_SORT_STRING, categoryName);
  listWidget_->insertItem(listWidget_->count(), newItem);
  return newItem;
}

QListWidgetItem* CategoryDataBreadcrumbs::addValueItem_(const QString& text, const QString& name, int nameInt, int valueInt, bool isChecked, bool useAltFillColor)
{
  QListWidgetItem* newItem = new SortedListWidgetItem(text);
  newItem->setData(ROLE_CATEGORY_NAME, name);
  newItem->setData(ROLE_NAME_INT, nameInt);
  newItem->setData(ROLE_VALUE_INT, valueInt);
  newItem->setData(ROLE_IS_CHECKED, isChecked);
  newItem->setData(ROLE_USE_ALT_FILL_COLOR, useAltFillColor);
  // Show "No Value" (or "Has Value") first always.  Use spaces to separate since in ASCII they're earlier (0x20)
  if (valueInt == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
    newItem->setData(ROLE_SORT_STRING, QString("%1    %2").arg(name, text));
  else
    newItem->setData(ROLE_SORT_STRING, QString("%1  %2").arg(name, text));
  listWidget_->insertItem(listWidget_->count(), newItem);
  return newItem;
}

QString CategoryDataBreadcrumbs::buildValuesHtmlList_(const simData::CategoryNameManager& nameManager, const simData::CategoryFilter::ValuesCheck& checks, size_t maxItems) const
{
  // Avoid the degenerate case of 0 and 1 items
  if (maxItems < 2)
    maxItems = 2;

  // Add 1 because the "... and XX more values" takes up a spot on the list
  const size_t extraItemsCount = (maxItems >= checks.size() ? 0 : (1 + (checks.size() - maxItems)));
  // Documentational assert to explain that the "... and XX more values" is always 2 or more, if it's there at all
  assert(extraItemsCount == 0 || extraItemsCount >= 2);

  QString valueText;
  // Loop through each check
  size_t numItemsListed = 0;
  for (auto i = checks.begin(); i != checks.end(); ++i)
  {
    valueText += tr("<li>%1\n").arg(QString::fromStdString(nameManager.valueIntToString(i->first)));
    ++numItemsListed;

    // Break out early if we've hit our limit
    if (extraItemsCount > 0 && numItemsListed >= maxItems - 1)
    {
      valueText += tr("<li>... and %1 more values.\n").arg(extraItemsCount);
      break;
    }
  }
  return valueText;
}

void CategoryDataBreadcrumbs::addNameToList_(int nameIndex, bool useAltFillColor)
{
  // Break out to avoid nullptr problems
  if (filter_ == nullptr || filter_->getDataStore() == nullptr)
    return;

  // Initialize by getting the name manager, name, and current set of checks
  simData::CategoryNameManager& nameManager = filter_->getDataStore()->categoryNameManager();
  const QString name = QString::fromStdString(nameManager.nameIntToString(nameIndex));

  // Regular expressions show up uniquely
  const std::string regExpPattern = filter_->getRegExpPattern(nameIndex);
  if (!regExpPattern.empty())
  {
    // Form a tooltip
    const QString tipText = tr("Regular Expression filter on a variety of values in the '%1' Category, matching values with the following expression:<p><code>%2</code>")
        .arg(name, QString::fromStdString(regExpPattern));

    // Create a group item, then add a tooltip
    QListWidgetItem* newItem = addNameItem_(name, nameIndex, useAltFillColor);
    newItem->setText(tr("<%1>").arg(name));
    newItem->setToolTip(simQt::formatTooltip(tr("%1 Regular Expression").arg(name), tipText));
    return;
  }

  simData::CategoryFilter::ValuesCheck checks;
  filter_->getValues(nameIndex, checks);

  // Is unlisted values present?  If so, then it should be set to "true", and all other items
  // are "exclude ___".  If not, then all items are "include ___".  This function is essentially
  // split into two parts -- inclusive filters and exclusive filters.  The first half covers
  // the inclusive cases and the second half covers the exclusive cases.
  simData::CategoryFilter::ValuesCheck::iterator unlistedIter = checks.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  const bool inclusiveFilter = (unlistedIter == checks.end());

  // Inclusive filter means all items in the list are "Include X or Y or Z"
  if (inclusiveFilter)
  {
    // Only add up to "minimumGroupSize_" filters before we just group up the category
    if (static_cast<int>(checks.size()) > minimumGroupSize_)
    {
      // Form a list using HTML for each item being filtered
      QString valueText = buildValuesHtmlList_(nameManager, checks, MAX_ITEMS_IN_TOOLTIP);
      const QString tipText = tr("Filter a variety of values in the '%1' Category, including values:<ul>%2</ul>")
        .arg(name, valueText);

      // Create a group item, then add a tooltip
      QListWidgetItem* newItem = addNameItem_(name, nameIndex, useAltFillColor);
      newItem->setToolTip(simQt::formatTooltip(tr("%1 Category").arg(name), tipText));
      return;
    }

    // Add "<Value>" items to the tree
    for (auto i = checks.begin(); i != checks.end(); ++i)
    {
      // This is an inclusive filter; if this assert fails, then either simplify failed, or display
      // logic in this class failed.
      assert(i->second);

      const QString value = QString::fromStdString(nameManager.valueIntToString(i->first));
      // If "No Value" is checked, give a custom string (i.e. including only 'no value' items)
      QString itemText = value;
      if (i->first == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
        itemText = tr("No %1").arg(name);  // e.g. "No Affinity"

      QListWidgetItem* newItem = addValueItem_(itemText, name, nameIndex, i->first, i->second, useAltFillColor);
      newItem->setToolTip(simQt::formatTooltip(tr("%1: %2").arg(name, value),
        tr("Match value '%2' in category '%1'.").arg(name, value)));
    }
    return;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Unlisted value is present, which means "Unlisted Value" must be checked.  If not, then
  // the simplify failed or we have incorrect logic or assumptions.  Note that all the logic
  // from here down presumes that the category is 'exclusive', omitting values
  assert(unlistedIter->second);
  // Remove it from the checks structure to simplify logic below.
  checks.erase(unlistedIter);

  // Look for "No Value" because it's treated special, and its presence means that it is
  // checked, so we need a way to clear it.  The "No Value" logic is confusing in this class
  // due to the "No Value" logic in the rules for Category Filters.
  simData::CategoryFilter::ValuesCheck::iterator noValueIter = checks.find(simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
  // Determine maximum of checks.size() before we swap to a group view.  This is weird because
  // sometimes we have to add No Value, and sometimes we have to remove it.  So it's off by
  // one on both sides.
  int maxChecks = minimumGroupSize_ - 1; // Need to add "No Value"
  if (noValueIter != checks.end())
    maxChecks += 2; // Need to remove "No Value"

  // Create the group item if we exceed the number of checks
  if (static_cast<int>(checks.size()) > maxChecks)
  {
    // Form a list using HTML for each item being filtered, for the tooltip
    QString valueText;
    // Add No Value to the tooltip -- needed because of the weirdness with No Value being treated
    // special by the Category Filter rules.  We remove it here so that it doesn't get used in
    // the list of values in the tooltip.
    if (noValueIter == checks.end())
      valueText += tr("<li>%1\n").arg(tr("No Value"));
    else
      checks.erase(noValueIter);

    // Add each individual category value into the tooltip
    valueText += buildValuesHtmlList_(nameManager, checks, MAX_ITEMS_IN_TOOLTIP);
    const QString tipText = tr("Filter a variety of values in the '%1' Category, excluding values:<ul>%2</ul>")
      .arg(name, valueText);

    QListWidgetItem* newItem = addNameItem_(name, nameIndex, useAltFillColor);
    newItem->setToolTip(simQt::formatTooltip(tr("%1 Category").arg(name), tipText));
    return;
  }

  // At this point, we're omitting values, and each value needs to be listed separately.  We
  // also have taken care of all the "grouping" branches.  So each check gets a list item.

  if (noValueIter != checks.end())
  {
    // No Value would only be present here if it was checked.  If this fails, then the
    // simplify logic failed or changed, or this internal logic is wrong
    assert(noValueIter->second);

    // Remove it from checks structure to simplify logic below
    checks.erase(noValueIter);
  }
  else
  {
    QListWidgetItem* newItem = addValueItem_(tr("Has %1").arg(name), name, nameIndex, simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME, false, useAltFillColor);
    newItem->setToolTip(simQt::formatTooltip(tr("%1: Has Value").arg(name),
      tr("Match empty value in category '%1'.").arg(name)));
  }

  // Add "Not <Value>" items to the tree
  for (auto i = checks.begin(); i != checks.end(); ++i)
  {
    // This is an exclusive filter; if this assert fails, then either simplify failed, or display
    // logic in this class failed.
    assert(!i->second);

    const QString value = QString::fromStdString(nameManager.valueIntToString(i->first));
    QListWidgetItem* newItem = addValueItem_(tr("Not %1").arg(value), name, nameIndex, i->first, i->second, useAltFillColor);
    newItem->setToolTip(simQt::formatTooltip(tr("Exclude %1: %2").arg(name, value),
      tr("Match without value '%2' in category '%1'.").arg(name, value)));
  }
}

void CategoryDataBreadcrumbs::clearFilter()
{
  if (!filter_)
    return;

  // Clear out content
  delete filter_;
  filter_ = nullptr;
  listWidget_->clear();

  // Resize
  validHints_ = false;
  updateGeometry();
}

// Happens in response to clicking the close button on an entry
void CategoryDataBreadcrumbs::removeFilter_(const QModelIndex& index)
{
  if (!filter_ || !index.isValid() || index.model() != listWidget_->model())
    return;

  const int name = index.data(ROLE_NAME_INT).toInt();
  const QVariant valueVariant = index.data(ROLE_VALUE_INT);
  // Value Variant is undefined when we're using a single value to represent an entire category
  if (!valueVariant.isValid())
  {
    // Remove that whole category name from the filter and re-emit
    filter_->removeName(name);
  }
  else
  {
    // Removal requires a simplified filter to behave well, so simplify, remove the value, simplify again, and re-emit
    const int value = valueVariant.toInt();
    filter_->simplify();
    const int rv = filter_->removeValue(name, value);
    // Failure to remove value means internal configuration error
    assert(rv == 0);
    filter_->simplify();
  }

  // Delete the actual item, and notify that we've changed
  delete listWidget_->takeItem(index.row());

  // If we're out of items, add in the no-valid-item item
  addNoValidItemIfEmptyList_();

  // Redraw and update size
  validHints_ = false;
  updateGeometry();
  update(contentsRect());

  // Notify change
  Q_EMIT filterEdited(*filter_);
}

qreal CategoryDataBreadcrumbs::rectangleRadiusX() const
{
  return itemDelegate_->style().rectangleRadiusX;
}

qreal CategoryDataBreadcrumbs::rectangleRadiusY() const
{
  return itemDelegate_->style().rectangleRadiusY;
}

QColor CategoryDataBreadcrumbs::fillColor() const
{
  return itemDelegate_->style().fillColor;
}

QColor CategoryDataBreadcrumbs::altFillColor() const
{
  return itemDelegate_->style().altFillColor;
}

QColor CategoryDataBreadcrumbs::textColor() const
{
  return itemDelegate_->style().textColor;
}

QPen CategoryDataBreadcrumbs::outlinePen() const
{
  return itemDelegate_->style().outlinePen;
}

qreal CategoryDataBreadcrumbs::outlineWidth() const
{
  return itemDelegate_->style().outlinePen.widthF();
}

QColor CategoryDataBreadcrumbs::outlineColor() const
{
  return itemDelegate_->style().outlinePen.color();
}

QMargins CategoryDataBreadcrumbs::itemMargins() const
{
  return itemDelegate_->style().itemMargins;
}

QMargins CategoryDataBreadcrumbs::textPadding() const
{
  return itemDelegate_->style().textPadding;
}

QMargins CategoryDataBreadcrumbs::iconPadding() const
{
  return itemDelegate_->style().iconPadding;
}

QIcon CategoryDataBreadcrumbs::closeIcon() const
{
  return itemDelegate_->style().icon;
}

QSize CategoryDataBreadcrumbs::iconSize() const
{
  return itemDelegate_->style().iconSize;
}

int CategoryDataBreadcrumbs::minimumGroupSize() const
{
  return minimumGroupSize_;
}

bool CategoryDataBreadcrumbs::hideWhenEmpty() const
{
  return hideWhenEmpty_;
}

QString CategoryDataBreadcrumbs::emptyText() const
{
  return emptyText_;
}

void CategoryDataBreadcrumbs::setRectangleRadiusX(qreal value)
{
  if (value == itemDelegate_->style().rectangleRadiusX)
    return;

  itemDelegate_->style().rectangleRadiusX = value;
  update();
}

void CategoryDataBreadcrumbs::setRectangleRadiusY(qreal value)
{
  if (value == itemDelegate_->style().rectangleRadiusY)
    return;

  itemDelegate_->style().rectangleRadiusY = value;
  update();
}

void CategoryDataBreadcrumbs::setFillColor(const QColor& value)
{
  if (value == itemDelegate_->style().fillColor)
    return;

  itemDelegate_->style().fillColor = value;
  update();
}

void CategoryDataBreadcrumbs::setAltFillColor(const QColor& value)
{
  if (value == itemDelegate_->style().altFillColor)
    return;

  itemDelegate_->style().altFillColor = value;
  update();
}

void CategoryDataBreadcrumbs::setTextColor(const QColor& value)
{
  if (value == itemDelegate_->style().textColor)
    return;

  itemDelegate_->style().textColor = value;
  update();
}

void CategoryDataBreadcrumbs::setOutlinePen(const QPen& value)
{
  // Don't bother comparing, it's more expensive than just setting the value
  itemDelegate_->style().outlinePen = value;
  update();
}

void CategoryDataBreadcrumbs::setOutlineColor(const QColor& value)
{
  if (value == itemDelegate_->style().outlinePen.color())
    return;

  itemDelegate_->style().outlinePen.setColor(value);
  update();
}

void CategoryDataBreadcrumbs::setOutlineWidth(qreal value)
{
  if (value == itemDelegate_->style().outlinePen.widthF())
    return;

  itemDelegate_->style().outlinePen.setWidthF(value);
  update();
}

void CategoryDataBreadcrumbs::setItemMargins(const QMargins& value)
{
  if (value == itemDelegate_->style().itemMargins)
    return;

  itemDelegate_->style().itemMargins = value;
  validHints_ = false;
  updateGeometry();
  update();
}

void CategoryDataBreadcrumbs::setTextPadding(const QMargins& value)
{
  if (value == itemDelegate_->style().textPadding)
    return;

  itemDelegate_->style().textPadding = value;
  validHints_ = false;
  updateGeometry();
  update();
}

void CategoryDataBreadcrumbs::setIconPadding(const QMargins& value)
{
  if (value == itemDelegate_->style().iconPadding)
    return;

  itemDelegate_->style().iconPadding = value;
  validHints_ = false;
  updateGeometry();
  update();
}

void CategoryDataBreadcrumbs::setCloseIcon(const QIcon& value)
{
  itemDelegate_->style().icon = value;
  update();
}

void CategoryDataBreadcrumbs::setIconSize(const QSize& value)
{
  if (value == itemDelegate_->style().iconSize)
    return;

  itemDelegate_->style().iconSize = value;
  validHints_ = false;
  updateGeometry();
  update();
}

void CategoryDataBreadcrumbs::setMinimumGroupSize(int value)
{
  if (value == minimumGroupSize_)
    return;
  minimumGroupSize_ = value;
  // Rebuild the tree using setFilter()
  if (filter_)
  {
    simData::CategoryFilter newFilter(*filter_);
    setFilter(newFilter);
  }
}

void CategoryDataBreadcrumbs::setHideWhenEmpty(bool value)
{
  if (hideWhenEmpty_ == value)
    return;
  hideWhenEmpty_ = value;
  // If we're empty, then we'll need to update the geometry
  if (listWidget_->itemDelegate() == plainDelegate_ && listWidget_->count() == 1)
  {
    validHints_ = false;
    updateGeometry();
  }
}

void CategoryDataBreadcrumbs::setEmptyText(const QString& value)
{
  if (emptyText_ == value)
    return;
  emptyText_ = value;
  // Only need to change list text if we're actually showing that item; detect that by using itemDelegate()
  if (listWidget_->itemDelegate() == plainDelegate_ && listWidget_->count() == 1)
    listWidget_->item(0)->setText(emptyText_);
}

void CategoryDataBreadcrumbs::bindTo(simQt::EntityCategoryFilter* categoryFilter)
{
  if (!categoryFilter)
    return;
  // Changes to us will be reflected in the filter
  connect(this, &CategoryDataBreadcrumbs::filterEdited, categoryFilter, &EntityCategoryFilter::setCategoryFilter);
  // Changes in the filter trigger us to resynchronize.  Note that due to the way the
  // categoryFilterChanged() signal is emitted, we cannot use it directly.
  connect(categoryFilter, &EntityCategoryFilter::filterUpdated, this, &CategoryDataBreadcrumbs::synchronizeToSenderFilter_);
  // Update our current state to that of the category filter
  setFilter(categoryFilter->categoryFilter());
}

void CategoryDataBreadcrumbs::synchronizeToSenderFilter_()
{
  simQt::EntityCategoryFilter* from = dynamic_cast<simQt::EntityCategoryFilter*>(sender());
  // Assertion failure means that this method was called by something that wasn't an entity category
  // filter, which shouldn't be possible because it's a private slot.
  assert(from != nullptr);
  if (from)
    setFilter(from->categoryFilter());
}

}
