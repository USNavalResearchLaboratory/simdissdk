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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QToolTip>
#include <QTreeView>
#include <QVBoxLayout>
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/DataStore.h"
#include "simQt/QtFormatting.h"
#include "simQt/CategoryFilterCounter.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/SearchLineEdit.h"
#include "simQt/Settings.h"
#include "simQt/CategoryTreeModel.h"
#include "simQt/CategoryFilterWidget.h"

namespace simQt {

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
  /** Locked state gives the toggle a disabled look */
  bool locked;

  /** Describes On|Off|Lock styles */
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
  /** Style to use for OFF state */
  StateStyle off;
  /** Style to use for LOCK state */
  StateStyle lock;

  /** Initialize to default options */
  StyleOptionToggleSwitch()
    : trackMargin(0),
    value(false),
    locked(false)
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

    // Disabled-looking grey track and thumb
    lock.track = QColor(100, 100, 100);
    lock.thumb = lock.track.color().lighter();
    lock.text = QObject::tr("Locked");
    lock.textColor = Qt::black;
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

  // Priority goes to the locked state style over on/off
  const StyleOptionToggleSwitch::StateStyle& valueStyle = (option.locked ? option.lock : (option.value ? option.on : option.off));

  // Draw the track
  painter->setPen(Qt::NoPen);
  painter->setBrush(valueStyle.track);
  painter->setOpacity(0.45);
  painter->setRenderHint(QPainter::Antialiasing, true);
  // Newer Qt with newer MSVC renders the rounded rect poorly if the rounding
  // pixels argument is half of pixel height or greater; reduce to 0.49
  const double halfHeight = r.track.height() * 0.49;
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
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    const int onWidth = fontMetrics.width(option.on.text);
    const int offWidth = fontMetrics.width(option.off.text);
    const int lockWidth = fontMetrics.width(option.lock.text);
#else
    const int onWidth = fontMetrics.horizontalAdvance(option.on.text);
    const int offWidth = fontMetrics.horizontalAdvance(option.off.text);
    const int lockWidth = fontMetrics.horizontalAdvance(option.lock.text);
#endif
    textWidth = qMax(onWidth, offWidth);
    textWidth = qMax(lockWidth, textWidth);
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
  // Move thumb to the right if on and if category isn't locked
  if (option.value && !option.locked)
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
  : QStyledItemDelegate(parent),
  clickedElement_(SE_NONE)
{
}

CategoryTreeItemDelegate::~CategoryTreeItemDelegate()
{
}

void CategoryTreeItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& inOption, const QModelIndex& index) const
{
  // Initialize a new option struct that has data from the QModelIndex
  QStyleOptionViewItem opt(inOption);
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
    QStyleOptionViewItem branchOpt(opt);
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
    switchOpt.locked = index.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool();
    switchOpt.value = (switchOpt.locked ? false : index.data(CategoryTreeModel::ROLE_EXCLUDE).toBool());
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
    // Ignore event if category is locked
    if (index.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool())
    {
      clickedIndex_ = QModelIndex();
      return true;
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
    // Ignore event if category is locked
    if (index.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool())
    {
      clickedIndex_ = QModelIndex();
      return true;
    }
    // Clicking on toggle should save the index to detect release on the toggle
    const auto newHit = hit_(me->pos(), option, index);
    // Must match button, index, and element clicked
    if (me->button() == Qt::LeftButton && clickedIndex_ == index && newHit == clickedElement_)
    {
      // Toggle button should, well, toggle
      if (clickedElement_ == SE_EXCLUDE_TOGGLE)
      {
        QVariant oldState = index.data(CategoryTreeModel::ROLE_EXCLUDE);
        if (index.flags().testFlag(Qt::ItemIsEnabled))
          model->setData(index, !oldState.toBool(), CategoryTreeModel::ROLE_EXCLUDE);
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
    // Ignore event if category is locked
    if (index.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool())
    {
      clickedIndex_ = QModelIndex();
      return true;
    }

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
    const bool haveRegExp = !index.data(CategoryTreeModel::ROLE_REGEXP_STRING).toString().isEmpty();
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
        tr("When on, Exclude mode will omit all entities that match your selected values.<p>When off, the filter will match all entities that have one of your checked category values.<p>Exclude mode does not show entity counts.")),
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
/**
* Class that listens for entity events in the DataStore, and
* informs the parent when they happen.
*/
class CategoryFilterWidget::DataStoreListener : public simData::DataStore::Listener
{
public:
  explicit DataStoreListener(CategoryFilterWidget& parent)
    : parent_(parent)
  {};

  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    parent_.setEntityCountDirty();
  }
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    parent_.setEntityCountDirty();
  }
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    parent_.setEntityCountDirty();
  }

  // Fulfill the interface
  virtual void onPostRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot) {}
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId) {}
  virtual void onScenarioDelete(simData::DataStore* source) {}
  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id) {}
  virtual void onPropertiesChange(simData::DataStore *source, simData::ObjectId id) {}
  virtual void onChange(simData::DataStore *source) {}
  virtual void onFlush(simData::DataStore* source, simData::ObjectId id) {}

private:
  CategoryFilterWidget& parent_;
};

/////////////////////////////////////////////////////////////////////////

CategoryFilterWidget::CategoryFilterWidget(QWidget* parent)
  : QWidget(parent),
    activeFiltering_(false),
    showEntityCount_(false),
    counter_(nullptr),
    counterObjectTypes_(simData::ALL),
    setRegExpAction_(nullptr),
    countDirty_(true)
{
  setWindowTitle("Category Data Filter");
  setObjectName("CategoryFilterWidget");

  treeModel_ = new simQt::CategoryTreeModel(this);
  proxy_ = new simQt::CategoryProxyModel(this);
  proxy_->setSourceModel(treeModel_);
  proxy_->setSortRole(simQt::CategoryTreeModel::ROLE_SORT_STRING);
  proxy_->sort(0);

  treeView_ = new QTreeView(this);
  treeView_->setObjectName("CategoryFilterTree");
  treeView_->setFocusPolicy(Qt::NoFocus);
  treeView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  treeView_->setIndentation(0);
  treeView_->setAllColumnsShowFocus(true);
  treeView_->setHeaderHidden(true);
  treeView_->setModel(proxy_);
  treeView_->setMouseTracking(true);

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

  toggleLockCategoryAction_ = new QAction(tr("Lock Category"), this);
  connect(toggleLockCategoryAction_, SIGNAL(triggered()), this, SLOT(toggleLockCategory_()));

  QAction* separator3 = new QAction(this);
  separator3->setSeparator(true);

  QAction* collapseAction = new QAction(tr("Collapse Values"), this);
  connect(collapseAction, SIGNAL(triggered()), treeView_, SLOT(collapseAll()));
  collapseAction->setIcon(QIcon(":/simQt/images/Collapse.png"));

  QAction* expandAction = new QAction(tr("Expand Values"), this);
  connect(expandAction, SIGNAL(triggered()), this, SLOT(expandUnlockedCategories_()));
  expandAction->setIcon(QIcon(":/simQt/images/Expand.png"));

  treeView_->setContextMenuPolicy(Qt::CustomContextMenu);
  treeView_->addAction(setRegExpAction_);
  treeView_->addAction(clearRegExpAction_);
  treeView_->addAction(separator1);
  treeView_->addAction(resetAction);
  treeView_->addAction(separator2);
  treeView_->addAction(toggleLockCategoryAction_);
  treeView_->addAction(separator3);
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
  connect(proxy_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToProxy_(QModelIndex, int, int)));
  connect(search, SIGNAL(textChanged(QString)), this, SLOT(expandAfterFilterEdited_(QString)));
  connect(search, SIGNAL(textChanged(QString)), proxy_, SLOT(setFilterText(QString)));
  connect(itemDelegate, SIGNAL(expandClicked(QModelIndex)), this, SLOT(toggleExpanded_(QModelIndex)));
  connect(itemDelegate, SIGNAL(editRegExpClicked(QModelIndex)), this, SLOT(showRegExpEditGui_(QModelIndex)));

  // timer is connected by setShowEntityCount below; it must be constructed before setShowEntityCount
  auto recountTimer = new QTimer(this);
  recountTimer->setSingleShot(false);
  recountTimer->setInterval(3000);
  connect(recountTimer, SIGNAL(timeout()), this, SLOT(recountCategories_()));
  recountTimer->start();

  // Entity filtering is on by default
  setShowEntityCount(true);

  dsListener_.reset(new CategoryFilterWidget::DataStoreListener(*this));
}

CategoryFilterWidget::~CategoryFilterWidget()
{
  if (categoryFilter().getDataStore())
    categoryFilter().getDataStore()->removeListener(dsListener_);
}

void CategoryFilterWidget::setDataStore(simData::DataStore* dataStore)
{
  simData::DataStore* prevDataStore = categoryFilter().getDataStore();
  if (prevDataStore == dataStore)
    return;

  if (prevDataStore)
    prevDataStore->removeListener(dsListener_);

  treeModel_->setDataStore(dataStore);
  counter_->setFilter(categoryFilter());

  if (dataStore)
    dataStore->addListener(dsListener_);
}

void CategoryFilterWidget::setSettings(Settings* settings, const QString& settingsKeyPrefix)
{
  treeModel_->setSettings(settings, settingsKeyPrefix);
}

const simData::CategoryFilter& CategoryFilterWidget::categoryFilter() const
{
  return treeModel_->categoryFilter();
}

void CategoryFilterWidget::setFilter(const simData::CategoryFilter& categoryFilter)
{
  treeModel_->setFilter(categoryFilter);
}

void CategoryFilterWidget::processCategoryCounts(const simQt::CategoryCountResults& results)
{
  treeModel_->processCategoryCounts(results);
}

bool CategoryFilterWidget::showEntityCount() const
{
  return showEntityCount_;
}

void CategoryFilterWidget::setShowEntityCount(bool fl)
{
  if (fl == showEntityCount_)
    return;
  showEntityCount_ = fl;

  // Clear out the old counter
  delete counter_;
  counter_ = nullptr;

  // Create a new counter and configure it
  if (showEntityCount_)
  {
    counter_ = new simQt::AsyncCategoryCounter(this);
    connect(counter_, SIGNAL(resultsReady(simQt::CategoryCountResults)), this, SLOT(processCategoryCounts(simQt::CategoryCountResults)));
    connect(treeModel_, SIGNAL(filterChanged(simData::CategoryFilter)), counter_, SLOT(setFilter(simData::CategoryFilter)));
    connect(treeModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), counter_, SLOT(asyncCountEntities()));
    counter_->setFilter(categoryFilter());
    counter_->setObjectTypes(counterObjectTypes_);
  }
  else
  {
    treeModel_->processCategoryCounts(simQt::CategoryCountResults());
  }
}

void CategoryFilterWidget::setEntityCountObjectTypes(simData::ObjectType counterObjectTypes)
{
  if (counterObjectTypes_ == counterObjectTypes)
    return;
  counterObjectTypes_ = counterObjectTypes;
  if (counter_)
    counter_->setObjectTypes(counterObjectTypes_);
}

void CategoryFilterWidget::setEntityCountDirty()
{
  countDirty_ = true;
}

void CategoryFilterWidget::expandAfterFilterEdited_(const QString& filterText)
{
  if (filterText.isEmpty())
  {
    // Just removed the last character of a search so collapse all to hide everything
    if (activeFiltering_)
      treeView_->collapseAll();

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

void CategoryFilterWidget::expandDueToProxy_(const QModelIndex& parentIndex, int to, int from)
{
  // Only expand when we're actively filtering, because we want
  // to see rows that match the active filter as they show up
  if (!activeFiltering_)
    return;

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

void CategoryFilterWidget::toggleExpanded_(const QModelIndex& proxyIndex)
{
  treeView_->setExpanded(proxyIndex, !treeView_->isExpanded(proxyIndex));
}

void CategoryFilterWidget::resetFilter_()
{
  // Create a new empty filter using same data store
  const simData::CategoryFilter newFilter(treeModel_->categoryFilter().getDataStore());
  treeModel_->setFilter(newFilter);

  // Tree would have sent out a changed signal, but not an edited signal (because we are
  // doing this programmatically).  That's OK, but we need to send out an edited signal.
  emit filterEdited(treeModel_->categoryFilter());
}

void CategoryFilterWidget::showContextMenu_(const QPoint& point)
{
  QMenu contextMenu(this);
  contextMenu.addActions(treeView_->actions());

  // Mark the RegExp and Lock actions enabled or disabled based on current state
  const QModelIndex idx = treeView_->indexAt(point);
  const bool emptyRegExp = idx.data(CategoryTreeModel::ROLE_REGEXP_STRING).toString().isEmpty();
  const bool locked = idx.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool();
  if (locked && !emptyRegExp)
    assert(0); // Should not be possible to have a RegExp set on a locked category
  setRegExpAction_->setProperty("index", idx);
  setRegExpAction_->setEnabled(idx.isValid() && !locked); // RegExp is disabled while locked
  // Mark the Clear RegExp action similarly
  clearRegExpAction_->setProperty("index", idx);
  clearRegExpAction_->setEnabled(idx.isValid() && !emptyRegExp && !locked); // RegExp is disabled while locked

  // Store the index in the Toggle Lock Category action
  toggleLockCategoryAction_->setProperty("index", idx);
  toggleLockCategoryAction_->setEnabled(idx.isValid() && emptyRegExp); // Locking is disabled while locked
  // Update the text based on the current lock state
  toggleLockCategoryAction_->setText(locked ? tr("Unlock Category") : tr("Lock Category"));

  // Show the menu
  contextMenu.exec(treeView_->mapToGlobal(point));

  // Clear the index property and disable
  setRegExpAction_->setProperty("index", QVariant());
  setRegExpAction_->setEnabled(false);
  clearRegExpAction_->setProperty("index", idx);
  clearRegExpAction_->setEnabled(false);
  toggleLockCategoryAction_->setProperty("index", QVariant());
}

void CategoryFilterWidget::setRegularExpression_()
{
  // Make sure we have a sender and can pull out the index.  If not, return
  QObject* senderObject = sender();
  if (senderObject == nullptr)
    return;
  QModelIndex index = senderObject->property("index").toModelIndex();
  if (index.isValid())
    showRegExpEditGui_(index);
}

void CategoryFilterWidget::showRegExpEditGui_(const QModelIndex& index)
{
  // Grab category name and old regexp, then ask user for new value
  const QString oldRegExp = index.data(CategoryTreeModel::ROLE_REGEXP_STRING).toString();
  const QString categoryName = index.data(CategoryTreeModel::ROLE_CATEGORY_NAME).toString();

  // pop up dialog with a entity filter line edit that supports formatting regexp
  QDialog optionsDialog(this);
  optionsDialog.setWindowTitle(tr("Set Regular Expression"));
  optionsDialog.setWindowFlags(optionsDialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QLayout* layout = new QVBoxLayout(&optionsDialog);
  QLabel* label = new QLabel(tr("Set '%1' value regular expression:").arg(categoryName), &optionsDialog);
  layout->addWidget(label);
  EntityFilterLineEdit* lineEdit = new EntityFilterLineEdit(&optionsDialog);
  lineEdit->setRegexOnly(true);
  lineEdit->setText(oldRegExp);
  lineEdit->setToolTip(
    tr("Regular expressions can be applied to categories in a filter.  Categories with regular expression filters will match only the values that match the regular expression."
    "<p>This popup changes the regular expression value for the category '%1'."
    "<p>An empty string can be used to clear the regular expression and return to normal matching mode.").arg(categoryName));
  layout->addWidget(lineEdit);
  QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &optionsDialog);
  connect(lineEdit, SIGNAL(isValidChanged(bool)), buttons.button(QDialogButtonBox::Ok), SLOT(setEnabled(bool)));
  connect(&buttons, SIGNAL(accepted()), &optionsDialog, SLOT(accept()));
  connect(&buttons, SIGNAL(rejected()), &optionsDialog, SLOT(reject()));
  layout->addWidget(&buttons);
  optionsDialog.setLayout(layout);
  if (optionsDialog.exec() == QDialog::Accepted && lineEdit->text() != oldRegExp)
  {
    // index.model() is const because changes to the model might invalidate indices.  Since we know this
    // and no longer use the index after this call, it is safe to use const_cast here to use setData().
    QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
    model->setData(index, lineEdit->text(), CategoryTreeModel::ROLE_REGEXP_STRING);
  }
}

void CategoryFilterWidget::clearRegularExpression_()
{
  // Make sure we have a sender and can pull out the index.  If not, return
  QObject* senderObject = sender();
  if (senderObject == nullptr)
    return;
  QModelIndex index = senderObject->property("index").toModelIndex();
  if (!index.isValid())
    return;
  // index.model() is const because changes to the model might invalidate indices.  Since we know this
  // and no longer use the index after this call, it is safe to use const_cast here to use setData().
  QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
  model->setData(index, QString(""), CategoryTreeModel::ROLE_REGEXP_STRING);
}

void CategoryFilterWidget::toggleLockCategory_()
{
  // Make sure we have a sender and can pull out the index.  If not, return
  QObject* senderObject = sender();
  if (senderObject == nullptr)
    return;
  QModelIndex index = senderObject->property("index").toModelIndex();
  if (!index.isValid())
    return;

  const bool locked = index.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool();

  if (!locked)
  {
    // If index is a value, get its category parent
    if (index.parent().isValid())
      index = index.parent();
    if (!index.isValid())
    {
      assert(0); // value index should have a valid parent
      return;
    }

    // Collapse the category
    treeView_->setExpanded(index, false);
  }

  // index.model() is const because changes to the model might invalidate indices.  Since we know this
  // and no longer use the index after this call, it is safe to use const_cast here to use setData().
  QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
  // Unlock the category
  model->setData(index, !locked, CategoryTreeModel::ROLE_LOCKED_STATE);
}

void CategoryFilterWidget::expandUnlockedCategories_()
{
  // Expand each category if it isn't locked
  for (int i = 0; i < proxy_->rowCount(); ++i)
  {
    const QModelIndex& idx = proxy_->index(i, 0);
    if (!idx.data(CategoryTreeModel::ROLE_LOCKED_STATE).toBool())
      treeView_->setExpanded(idx, true);
  }
}

void CategoryFilterWidget::recountCategories_()
{
  if (countDirty_)
  {
    if (showEntityCount_ && counter_)
      counter_->asyncCountEntities();
    countDirty_ = false;
  }
}

}
