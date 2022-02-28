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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include <cassert>
#include <set>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QTreeWidget>
#include <QTimer>
#include "simNotify/Notify.h"
#include "simCore/Time/Utils.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityProxyModel.h"

namespace simQt {

static const QString EntityTreeWidgetViewSetting = "Private/Entity Tree/Show Tree View";

/** Watch settings for changes between tree view and list view */
class EntityTreeWidget::EntitySettingsObserver : public Settings::Observer
{
public:
  /** Constructor */
  explicit EntitySettingsObserver(EntityTreeWidget* parent) : parent_(parent) {}
  virtual ~EntitySettingsObserver() {}
  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    parent_->toggleTreeView(value.toBool());
  }

private:
  EntityTreeWidget* parent_;
};


EntityTreeWidget::EntityTreeWidget(QTreeView* view)
  : QObject(view),
    view_(view),
    model_(nullptr),
    proxyModel_(nullptr),
    settings_(SettingsPtr()),
    treeView_(false),
    pendingSendNumItems_(false),
    processSelectionModelSignals_(true),
    countEntityTypes_(simData::ALL),
    lastSelectionChangedTime_(0.0)
{
  proxyModel_ = new simQt::EntityProxyModel(this);
  proxyModel_->setDynamicSortFilter(true);
  view_->setModel(proxyModel_);
  view_->setSortingEnabled(true);
  view_->sortByColumn(0, Qt::AscendingOrder);
  view_->setIndentation(4);  // The default indentation for a list view

  emitItemsSelectedTimer_ = new QTimer(this);
  emitItemsSelectedTimer_->setSingleShot(true);
  emitItemsSelectedTimer_->setInterval(0); // instant, when event loop picks up

  connect(proxyModel_, SIGNAL(modelReset()), this, SLOT(selectionCleared_()));
  connect(proxyModel_, SIGNAL(modelReset()), this, SLOT(sendNumFilteredItems_()));
  connect(proxyModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(proxyModel_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(proxyModel_, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>)), this, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>))); // Echo out the signal
  connect(view_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(selectionChanged_(const QItemSelection&, const QItemSelection&)));
  connect(view_, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked_(const QModelIndex&)));
  connect(emitItemsSelectedTimer_, SIGNAL(timeout()), this, SLOT(emitItemsSelected_()));
}

EntityTreeWidget::~EntityTreeWidget()
{
  delete proxyModel_;
  // Do not delete model_ or view_, this class does not own them
  if (settings_)
  {
    settings_->removeObserver(EntityTreeWidgetViewSetting, settingsObserver_);
    settings_->setValue(EntityTreeWidgetViewSetting, treeView_);
  }
}

void EntityTreeWidget::addEntityFilter(EntityFilter* entityFilter)
{
  proxyModel_->addEntityFilter(entityFilter);
  // Adding a filter can change the numbers so send them out
  sendNumFilteredItems_();
}

QList<QWidget*> EntityTreeWidget::filterWidgets(QWidget* newWidgetParent) const
{
  return proxyModel_->filterWidgets(newWidgetParent);
}

void EntityTreeWidget::setModel(AbstractEntityTreeModel* model)
{
  if (model_ != nullptr)
    disconnect(model_, nullptr, this, nullptr);

  model_ = model;

  connect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(delaySend_()));

  connect(model_, SIGNAL(rowsAboutToBeInserted(QModelIndex, int, int)), this, SLOT(captureVisible_()));
  connect(model_, SIGNAL(rowsAboutToBeRemoved(QModelIndex, int, int)), this, SLOT(captureVisible_()));
  connect(model_, SIGNAL(rowsAboutToBeMoved(QModelIndex, int, int, QModelIndex, int)), this, SLOT(captureVisible_()));
  /** Handle rename, since there is only one signal the slot needs to handle both capture and keep */
  connect(model_, SIGNAL(dataChanged(QModelIndex, QModelIndex, QVector<int>)), this, SLOT(captureAndKeepVisible_()));

  proxyModel_->setSourceModel(model_);

  // Need to allow the view to update before checking if the selected item is still visible
  auto keepVisibleOnTimer = [this]() { QTimer::singleShot(10, this, SLOT(keepVisible_())); };
  connect(model_, &QAbstractItemModel::rowsInserted, this, keepVisibleOnTimer);
  connect(model_, &QAbstractItemModel::rowsRemoved, this, keepVisibleOnTimer);
  connect(model_, &QAbstractItemModel::rowsMoved, this, keepVisibleOnTimer);

  // new model set, update from our settings
  if (settings_)
  {
    QVariant treeView = settings_->value(EntityTreeWidgetViewSetting, settingsObserver_);
    toggleTreeView(treeView.toBool());
  }
  sendNumFilteredItems_();

  // Set column widths here because setting the widths before setting the model resets the widths.
  view_->setColumnWidth(0, 140);
  view_->setColumnWidth(1, 35);
  view_->setColumnWidth(2, 45);
}

void EntityTreeWidget::captureAndKeepVisible_()
{
  /** There is no before or after signal for rename, just dataChanged.  Need to capture before the proxy and keep after everyone */
  captureVisible_();
  if (!setVisible_.empty())
    QTimer::singleShot(10, this, SLOT(keepVisible_()));
}

void EntityTreeWidget::captureVisible_()
{
  // Temporary structure to sort the selected items by vertical location in the list
  struct Entry
  {
    QRect rect;
    QModelIndex index;
    Entry(const QRect& inRect, const QModelIndex& inIndex)
      : rect(inRect),
        index(inIndex)
    {}

    bool operator<(const Entry& a) const
    {
      return rect.top() < a.rect.top();
    }
  };

  std::vector<Entry> entries;
  for (const auto& index : view_->selectionModel()->selectedRows())
  {
    auto rect = view_->visualRect(index);
    // Contrary to the documentation, rect is not invalid if index is not visible.  Manually check if the index is visible.
    auto height = view_->height() - view_->header()->height();
    if (!rect.isValid() || (rect.bottom() < 0)  || (rect.top() > height))
      continue;

    entries.push_back(Entry(rect, index));
  }

  std::sort(entries.begin(), entries.end());
  for (const auto& entry : entries)
    setVisible_.push_back(model_->uniqueId(proxyModel_->mapToSource(entry.index)));
}

void EntityTreeWidget::keepVisible_()
{
  for (auto id : setVisible_)
  {
    auto index = proxyModel_->mapFromSource(model_->index(id));

    // if the entity was deleted, continue to the next one
    if (!index.isValid())
      continue;

    view_->scrollTo(index);
    break;
  }

  setVisible_.clear();
}

void EntityTreeWidget::clearSelection()
{
  // Since the world is telling us to change the selection, we do not need to tell the world the selection has changed.
  ScopedSignalBlocker blockSignals(*view_);
  view_->clearSelection();
  selectionList_.clear();
  selectionSet_.clear();
}

int EntityTreeWidget::setSelected(uint64_t id)
{
  if (model_ == nullptr)
    return 1;

  if ((selectionList_.size() == 1) && (selectionList_.front() == id))
    return 1;

  // Ignore the signal so that selectionList_ does not get re-calculated
  processSelectionModelSignals_ = false;

  selectionSet_.clear();
  selectionList_.clear();

  QModelIndex index = proxyModel_->mapFromSource(model_->index(id));
  if (index != QModelIndex())
  {
    selectionSet_.insert(id);
    selectionList_.append(id);

    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect;
    view_->selectionModel()->select(index, flags);
    view_->selectionModel()->setCurrentIndex(index, flags);
  }
  else
  {
    view_->selectionModel()->clear();
  }

  // Stop ignoring the signal
  processSelectionModelSignals_ = true;

  // Tell listeners about the new selections (could be empty list)
  emit itemsSelected(selectionList_);
  return 0;
}

int EntityTreeWidget::setSelected(const QList<uint64_t>& list)
{
  if (model_ == nullptr)
    return 1;

  QSet<uint64_t> newSet;  ///< Use new set to detected changes with selectionSet_
  QItemSelection selections;  ///< The selected entities
  QModelIndex current;  ///< The current index

  // if all entities are selected and in list view just do one selection
  if ((list.size() == numberOfEntities_(QModelIndex())) && !treeView_)
  {
    for (int ii = 0; ii < list.count(); ii++)
    {
      uint64_t id = list[ii];

      QModelIndex index = proxyModel_->mapFromSource(model_->index(id));
      if (index == QModelIndex())
        continue;

      newSet.insert(id);
    }

    QModelIndex upperLeft = proxyModel_->index(0, 0);
    QModelIndex lowerRight = proxyModel_->index(proxyModel_->rowCount() - 1, 0);
    selections.select(upperLeft, lowerRight);
  }
  else
  {
    // keep track of indexes by parent to minimize the number of selections by combining neighboring indexes into one range
    std::set< std::pair<QModelIndex, QModelIndex> > indexes;

    for (int ii = 0; ii < list.count(); ii++)
    {
      uint64_t id = list[ii];

      QModelIndex index = proxyModel_->mapFromSource(model_->index(id));
      if (index == QModelIndex())
        continue;

      if (current.row() == -1)
        current = index;

      newSet.insert(id);
      indexes.insert(std::make_pair(index.parent(), index));
    }

    // Minimize the number of selections by combining neighboring indexes into one range
    if (!indexes.empty())
    {
      auto it = indexes.begin();
      QModelIndex parentIndex = it->first;
      QModelIndex startIndex = it->second;
      QModelIndex currentIndex = startIndex;
      ++it;
      for (; it != indexes.end(); ++it)
      {
        // If parents are different then finish a range and start a new one
        if (parentIndex != it->first)
        {
          selections.select(startIndex, currentIndex);
          parentIndex = it->first;
          startIndex = it->second;
          currentIndex = startIndex;
        }
        else
        {
          // If children are not neighbors then finish a range and start a new one
          if (it->second.row() != (currentIndex.row() + 1))
          {
            selections.select(startIndex, currentIndex);
            startIndex = it->second;
            currentIndex = startIndex;
          }
          else
            currentIndex = it->second;  // neighbors so keep going
        }
      }

      // The last range needs to be committed
      selections.select(startIndex, currentIndex);
    }
  }

  if (newSet == selectionSet_)
    return 1;

  // Ignore the signal so that selectionList_ does not get re-calculated
  processSelectionModelSignals_ = false;

  if (!newSet.empty())
  {
    view_->selectionModel()->select(selections, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    if (current.isValid())
      view_->selectionModel()->setCurrentIndex(current, QItemSelectionModel::Rows | QItemSelectionModel::Select);
  }
  else
  {
    view_->selectionModel()->clear();
  }

  // Stop ignoring the signal
  processSelectionModelSignals_ = true;

  selectionSet_ = newSet;
  selectionList_.clear();
  for (auto it = selectionSet_.begin(); it != selectionSet_.end(); ++it)
    selectionList_.push_back(*it);

  // Tell listeners about the new selections (could be empty list)
  emit itemsSelected(selectionList_);
  return 0;
}

int EntityTreeWidget::numberOfEntities_(const QModelIndex& index)
{
  int partial = model_->rowCount(index);
  int total = partial;
  for (int ii = 0; ii < partial; ++ii)
    total += numberOfEntities_(model_->index(ii, 0, index));

  return total;
}

void EntityTreeWidget::scrollTo(uint64_t id, QAbstractItemView::ScrollHint hint)
{
  QModelIndex index = proxyModel_->mapFromSource(model_->index(id));
  if (index.isValid())
    view_->scrollTo(index, hint);
}

QAbstractItemView::SelectionMode EntityTreeWidget::selectionMode() const
{
  return view_->selectionMode();
}

void EntityTreeWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
  view_->setSelectionMode(mode);
}

simData::ObjectId EntityTreeWidget::alwaysShow() const
{
  return proxyModel_->alwaysShow();
}

void EntityTreeWidget::setAlwaysShow(simData::ObjectId id)
{
  proxyModel_->setAlwaysShow(id);
}

void EntityTreeWidget::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  proxyModel_->getFilterSettings(settings);
}

void EntityTreeWidget::setCountEntityType(simData::ObjectType type)
{
  if (countEntityTypes_ == type)
    return;

  countEntityTypes_ = type;
  sendNumFilteredItems_();
}

simData::ObjectType EntityTreeWidget::countEntityTypes() const
{
  return countEntityTypes_;
}

void EntityTreeWidget::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  const QList<uint64_t>& entities = selectedItems();
  proxyModel_->setFilterSettings(settings);
  if (entities.empty())
    return;
  // try to scroll to the last selected item if it's valid, same behavior as setSelected
  auto iter = entities.end();
  --iter;
  for (; iter != entities.begin(); --iter)
  {
    const QModelIndex index = proxyModel_->mapFromSource(model_->index(*iter));
    if (index.isValid())
    {
      scrollTo(*iter, QAbstractItemView::PositionAtCenter);
      return;
    }
  }
  const QModelIndex index = proxyModel_->mapFromSource(model_->index(entities.front()));
  if (index.isValid())
    scrollTo(entities.front(), QAbstractItemView::PositionAtCenter);
}

QList<uint64_t> EntityTreeWidget::selectedItems() const
{
  return selectionList_;
}

void EntityTreeWidget::setSettings(SettingsPtr settings)
{
  settings_ = settings;
  // initialize settings
  if (settings_)
  {
    settingsObserver_ = Settings::ObserverPtr(new EntitySettingsObserver(this));
    QVariant treeView = settings_->value(EntityTreeWidgetViewSetting, settingsObserver_);
    if (treeView.toBool() != treeView_)
      toggleTreeView(treeView.toBool());
  }
}

bool EntityTreeWidget::isTreeView() const
{
  return treeView_;
}

void EntityTreeWidget::initializeSettings(SettingsPtr settings)
{
  settings->value(EntityTreeWidgetViewSetting, Settings::MetaData::makeBoolean(false, "Show Entity list in tree view", Settings::DEFAULT));
}

void EntityTreeWidget::setToTreeView()
{
  if (treeView_)
    return;
  treeView_ = true;
  if (model_ == nullptr)
    return;

  QList<uint64_t> entities = selectedItems();
  // need to clear selection set here to ensure selected items get reselected properly in setSelected, since toggleTreeView resets the model
  selectionSet_.clear();
  model_->setToTreeView();
  setSelected(entities);
  if (settings_)
    settings_->setValue(EntityTreeWidgetViewSetting, treeView_, settingsObserver_);
}

void EntityTreeWidget::setToListView()
{
  if (!treeView_)
    return;
  treeView_ = false;
  if (model_ == nullptr)
    return;

  QList<uint64_t> entities = selectedItems();
  // need to clear selection set here to ensure selected items get reselected properly in setSelected, since toggleTreeView resets the model
  selectionSet_.clear();
  model_->setToListView();
  setSelected(entities);
  if (settings_)
    settings_->setValue(EntityTreeWidgetViewSetting, treeView_, settingsObserver_);
}

void EntityTreeWidget::toggleTreeView(bool useTree)
{
  if (useTree == treeView_)
    return;
  treeView_ = useTree;

  // Set the indentation appropriate to the current view
  static const int TREE_INDENT = 20;
  static const int LIST_INDENT = 4;
  view_->setIndentation(useTree ? TREE_INDENT : LIST_INDENT);

  if (model_ == nullptr)
    return;

  QList<uint64_t> entities = selectedItems();
  // need to clear selection set here to ensure selected items get reselected properly in setSelected, since toggleTreeView resets the model
  selectionSet_.clear();
  model_->toggleTreeView(useTree);
  setSelected(entities);

  // Save the flag into settings
  if (settings_)
    settings_->setValue(EntityTreeWidgetViewSetting, treeView_, settingsObserver_);
}

void EntityTreeWidget::forceRefresh()
{
  if (model_ == nullptr)
    return;

  model_->forceRefresh();
}

void EntityTreeWidget::selectionCleared_()
{
  if (!selectionList_.isEmpty())
  {
    selectionList_.clear();
    selectionSet_.clear();
    emit itemsSelected(selectionList_);
  }
}

void EntityTreeWidget::emitItemsSelected_()
{
  // Clear out our selection
  selectionList_.clear();
  selectionSet_.clear();

  // Iterate over each item in the list that is selected
  const QModelIndexList selectedItems = view_->selectionModel()->selectedRows();
  for (auto it = selectedItems.begin(); it != selectedItems.end(); ++it)
  {
    // Pull out the item from the index, which contains the ID
    const QModelIndex index2 = proxyModel_->mapToSource(*it);
    const AbstractEntityTreeItem* item = static_cast<AbstractEntityTreeItem*>(index2.internalPointer());
    if (item == nullptr)
      continue;
    // Add the ID to both lists
    const uint64_t id = item->id();
    selectionSet_.insert(id);
    selectionList_.push_back(id);
  }

  // Validates that there were no duplicates in the rows.  Assertion trigger means
  // that either the data store reported a duplicate ID, or tree is storing a dupe ID
  assert(selectionSet_.size() == selectionList_.size());

  // Tell listeners about the new selections (could be empty list)
  emit itemsSelected(selectionList_);
  emitItemsSelectedTimer_->stop();
  lastSelectionChangedTime_ = simCore::getSystemTime();
}

void EntityTreeWidget::selectionChanged_(const QItemSelection& selected, const QItemSelection& deselected)
{
  // Because of blocked signals, we cannot trust that this is called
  // as often as is needed.  As a result, selected/deselected cannot be trusted as the
  // correct delta from one call to the next call.

  // It is possible this is called while selecting multiple ids at once, so
  // return early if our flag isn't set.
  if (!processSelectionModelSignals_)
    return;

  // Timer is running and call emitItemSelected_() correctly
  if (emitItemsSelectedTimer_->isActive())
    return;
  // If the current time is too soon after the last time we got here, we might be in a tight loop.  If so,
  // then start the timer, queueing up processing.
  double now = simCore::getSystemTime();
  if (now < lastSelectionChangedTime_ + 0.1)  // 100 millsecond tolerance
  {
    // Queue up the emitItemsSelected_
    emitItemsSelectedTimer_->start();
  }
  else
  {
    emitItemsSelected_();
  }
  // Save the time so successive signals get grouped up
  lastSelectionChangedTime_ = now;
}

void EntityTreeWidget::doubleClicked_(const QModelIndex& index)
{
  QModelIndex index2 = proxyModel_->mapToSource(index);
  AbstractEntityTreeItem *item = static_cast<AbstractEntityTreeItem*>(index2.internalPointer());
  if (item != nullptr)
    emit itemDoubleClicked(item->id());
}

void EntityTreeWidget::delaySend_()
{
  if (!pendingSendNumItems_)
  {
    // Compress all row count changes for the next 100 milliseconds into one numFilteredItemsChanged signal
    QTimer::singleShot(100, this, SLOT(emitSend_()));
    pendingSendNumItems_ = true;
  }
}

void EntityTreeWidget::emitSend_()
{
  pendingSendNumItems_ = false;
  sendNumFilteredItems_();
}

void EntityTreeWidget::sendNumFilteredItems_()
{
  if ((proxyModel_ != nullptr) && (model_ != nullptr))
    emit numFilteredItemsChanged(proxyModel_->rowCount(), model_->countEntityTypes(countEntityTypes_));
}

}
