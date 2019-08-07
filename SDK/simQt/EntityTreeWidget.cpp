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
#include <set>
#include <QSortFilterProxyModel>
#include <QTreeWidget>
#include <QTimer>
#include "simNotify/Notify.h"
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
    model_(NULL),
    proxyModel_(NULL),
    settings_(SettingsPtr()),
    treeView_(false),
    pendingSendNumItems_(false),
    emitSelectionChanged_(true)
{
  proxyModel_ = new simQt::EntityProxyModel(this);
  proxyModel_->setDynamicSortFilter(true);
  view_->setModel(proxyModel_);
  view_->setSortingEnabled(true);
  view_->sortByColumn(0, Qt::AscendingOrder);
  view_->setIndentation(4);  // The default indentation for a list view

  connect(proxyModel_, SIGNAL(modelReset()), this, SLOT(selectionCleared_()));
  connect(proxyModel_, SIGNAL(modelReset()), this, SLOT(sendNumFilteredItems_()));
  connect(proxyModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(proxyModel_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(proxyModel_, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>)), this, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>))); // Echo out the signal
  connect(view_->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(selectionChanged_(const QItemSelection&, const QItemSelection&)));
  connect(view_, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked_(const QModelIndex&)));
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
  if (model_ != NULL)
  {
    disconnect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(delaySend_()));
    disconnect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(delaySend_()));
  }

  model_ = model;
  proxyModel_->setSourceModel(model_);

  connect(model_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(delaySend_()));
  connect(model_, SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(delaySend_()));

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

void EntityTreeWidget::clearSelection()
{
  // Since the world is telling us to change the selection, we do not need to tell the world the selection has changed.
  ScopedSignalBlocker blockSignals(*view_);
  view_->clearSelection();
  selectionList_.clear();
  selectionSet_.clear();
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
void EntityTreeWidget::setSelected(uint64_t id, bool selected, bool signalItemsSelected)
{
  if (model_ == NULL)
    return;

  // Pull out the index from the proxy
  QModelIndex index = proxyModel_->mapFromSource(model_->index(id));
  // If it's invalid, break out
  if (index == QModelIndex())
  {
    // Make sure the item is not in the selection list (can happen in swap to tree list if
    // a gate was selected and beams were filtered)
    if (selectionSet_.remove(id))
      selectionList_.removeOne(id);
    return;
  }

  // If the item is already selected/deselected, then ignore the request
  if (view_->selectionModel()->isSelected(index) == selected)
  {
    // Validate that our cache is consistent with this request
    assert(selectionSet_.contains(id) == selected);
    return;
  }

  // For internal consistency, update the selection_ cache BEFORE changing selection
  if (selected)
  {
    // It's possible that this check could fail if swapping between tree and list, because
    // in some cases signals can be blocked and we don't get the update on selections
    if (!selectionSet_.contains(id))
    {
      selectionSet_.insert(id);
      selectionList_.append(id);
    }
  }
  else
  {
    // Assertion failure means the model thinks we had a selection, but we didn't cache it
    assert(selectionSet_.contains(id));
    // Only remove from list, if remove from set succeeds
    if (selectionSet_.remove(id))
      selectionList_.removeOne(id);
  }
  // Update our flag to match the signalItemsSelected flag. Do this so that selectionModel()->select()
  // properly tells the view_ to update graphically, but so that we don't unnecessarily update in selectionChanged_()
  emitSelectionChanged_ = signalItemsSelected;
  // Update the selection
  QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::Rows | (selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
  view_->selectionModel()->select(index, flags);
  view_->selectionModel()->setCurrentIndex(index, flags);
  // Restore the flag to true, so that single selections work as expected
  emitSelectionChanged_ = true;
}

void EntityTreeWidget::setSelected(QList<uint64_t> list, bool selected)
{
  for (int ii = 0; ii < list.count(); ii++)
    setSelected(list[ii], selected, ii == (list.count()-1));  // cause a GUI update on the last selection
}
#endif

int EntityTreeWidget::setSelected(uint64_t id)
{
  if (model_ == NULL)
    return 1;

  if ((selectionList_.size() == 1) && (selectionList_.front() == id))
    return 1;

  // Ignore the signal so that selectionList_ does not get re-calculated
  emitSelectionChanged_ = false;

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
  emitSelectionChanged_ = true;

  // Tell listeners about the new selections (could be empty list)
  emit itemsSelected(selectionList_);
  return 0;
}

int EntityTreeWidget::setSelected(const QList<uint64_t>& list)
{
  if (model_ == NULL)
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
  emitSelectionChanged_ = false;

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
  emitSelectionChanged_ = true;

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
  if (model_ == NULL)
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
  if (model_ == NULL)
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

  if (model_ == NULL)
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
  if (model_ == NULL)
    return;

  model_->forceRefresh();
}

void EntityTreeWidget::selectionCleared_()
{
  emit itemsSelected(QList<uint64_t>());
}

void EntityTreeWidget::selectionChanged_(const QItemSelection& selected, const QItemSelection& deselected)
{
  // Because of blocked signals, we cannot trust that this is called
  // as often as is needed.  As a result, selected/deselected cannot be trusted as the
  // correct delta from one call to the next call.

  // It is possible this is called while selecting multiple ids at once, so
  // return early if our flag isn't set.
  if (!emitSelectionChanged_)
    return;

  // Clear out our selection
  selectionList_.clear();
  selectionSet_.clear();

  // Iterate over each item in the list that is selected
  const QModelIndexList selectedItems = view_->selectionModel()->selectedRows();
  Q_FOREACH(const QModelIndex index, selectedItems)
  {
    // Pull out the item from the index, which contains the ID
    const QModelIndex index2 = proxyModel_->mapToSource(index);
    const AbstractEntityTreeItem *item = static_cast<AbstractEntityTreeItem*>(index2.internalPointer());
    if (item == NULL)
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
}

void EntityTreeWidget::doubleClicked_(const QModelIndex& index)
{
  QModelIndex index2 = proxyModel_->mapToSource(index);
  AbstractEntityTreeItem *item = static_cast<AbstractEntityTreeItem*>(index2.internalPointer());
  if (item != NULL)
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
  if ((proxyModel_ != NULL) && (model_ != NULL))
    emit numFilteredItemsChanged(proxyModel_->rowCount(), model_->rowCount());
}

}
