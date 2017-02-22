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
#include <QTimer>

#include "simCore/String/Utils.h"
#include "simData/DataStoreHelpers.h"
#include "simVis/Registry.h"
#include "simQt/EntityTreeModel.h"

namespace simQt {

/// notify the tree model about data store changes
class EntityTreeModel::TreeListener : public simData::DataStore::DefaultListener
{
public:
  /// constructor
  explicit TreeListener(EntityTreeModel *parent)
  : parent_(parent)
  {
  }

  /// new entity has been added, with the given id and type
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::DataStore::ObjectType ot)
  {
    parent_->addEntity_(newId);
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::DataStore::ObjectType ot)
  {
    parent_->removeEntity_(removedId);
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    parent_->emitEntityDataChanged_(changeId);
  }

  /// something has changed in the entity category data
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::DataStore::ObjectType ot)
  {
    parent_->emitEntityDataChanged_(changedId);
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source)
  {
    parent_->removeAllEntities_();
  }

protected:
  EntityTreeModel *parent_; ///< model which receives notices
};

//----------------------------------------------------------------------------
EntityTreeItem::EntityTreeItem(simData::ObjectId id, EntityTreeItem *parent)
{
  id_ = id;
  parentItem_ = parent;
}

EntityTreeItem::~EntityTreeItem()
{
  qDeleteAll(childItems_);
}

void EntityTreeItem::appendChild(EntityTreeItem *item)
{
  childItems_.append(item);
}

void EntityTreeItem::removeChild(EntityTreeItem *item)
{
  childItems_.removeOne(item);
  delete item;
}

EntityTreeItem* EntityTreeItem::child(int row)
{
  return childItems_.value(row);
}

int EntityTreeItem::childCount() const
{
  return childItems_.count();
}

simData::ObjectId EntityTreeItem::id() const
{
  return id_;
}

void EntityTreeItem::getChildrenIds(std::vector<uint64_t>& ids) const
{
  Q_FOREACH(const EntityTreeItem* child, childItems_)
  {
    ids.push_back(child->id());
    child->getChildrenIds(ids);
  }
}

EntityTreeItem* EntityTreeItem::parent()
{
  return parentItem_;
}

int EntityTreeItem::row() const
{
  if (parentItem_)
    return parentItem_->childItems_.indexOf(const_cast<EntityTreeItem*>(this));

  return 0;
}

//-----------------------------------------------------------------------------------------

EntityTreeModel::EntityTreeModel(QObject *parent, simData::DataStore* dataStore)
  : AbstractEntityTreeModel(parent),
    rootItem_(NULL),
    treeView_(false),
    dataStore_(NULL)
{
  // create observers/listeners
  listener_ = simData::DataStore::ListenerPtr(new TreeListener(this));

  // setting the data store will register our observer and listener
  setDataStore(dataStore);

  // fill the tree model
  forceRefresh();
}

EntityTreeModel::~EntityTreeModel()
{
  setDataStore(NULL);
  delete rootItem_;
}

void EntityTreeModel::setDataStore(simData::DataStore* dataStore)
{
  if (dataStore == dataStore_)
    return;

  // Remove the prefs observers on the data store
  if (dataStore_)
  {
    dataStore_->removeListener(listener_);
  }

  // Update the pointer
  dataStore_ = dataStore;

  // re-add the prefs observers
  if (dataStore_)
  {
    dataStore->addListener(listener_);
  }
}

simData::DataStore* EntityTreeModel::dataStore() const
{
  return dataStore_;
}

void EntityTreeModel::addEntity_(uint64_t entityId)
{
  if (delayedAdds_.empty())
    QTimer::singleShot(100, this, SLOT(commitDelayedEntities_()));
  delayedAdds_.push_back(entityId);
}

void EntityTreeModel::commitDelayedEntities_()
{
  for (std::vector<simData::ObjectId>::const_iterator it = delayedAdds_.begin(); it != delayedAdds_.end(); ++it)
  {
    simData::DataStore::ObjectType entityType = dataStore_->objectType(*it);
    if (simData::DataStore::NONE)
    {
      // the entity should have been removed from the vector
      assert(false);
      continue;
    }

    // Pick out the host's id (0 for platforms)
    uint64_t hostId = 0;
    if (entityType != simData::DataStore::PLATFORM)
      hostId = dataStore_->entityHostId(*it);

    // Only add the item if it's a platform, or if it has a valid host
    assert(!((hostId == 0) && (entityType != simData::DataStore::PLATFORM)));
    if ((hostId > 0 || entityType == simData::DataStore::PLATFORM))
    {
      addTreeItem_(*it, entityType, hostId);
    }
  }
  delayedAdds_.clear();
}

void EntityTreeModel::emitEntityDataChanged_(uint64_t entityId)
{
  EntityTreeItem* found = findItem_(entityId);
  if (!found)
    return;

  QModelIndex start = createIndex(found->row(), 0, found);
  QModelIndex end = createIndex(found->row(), 2, found);
  emit dataChanged(start, end);
}

void EntityTreeModel::setToTreeView()
{
  if (!treeView_)
  {
    treeView_ = true;
    forceRefresh();
  }
}

void EntityTreeModel::setToListView()
{
  if (treeView_)
  {
    treeView_ = false;
    forceRefresh();
  }
}

void EntityTreeModel::toggleTreeView(bool useTree)
{
  if (useTree)
    setToTreeView();
  else
    setToListView();
}

void EntityTreeModel::forceRefresh()
{
  if (dataStore_)
  {
    // NOTE: for now, this is a tight coupling between the data
    // and the view.  may want to separate this, so that data
    // retrieval and view restructure are isolated

    beginResetModel();

    // clean up tree widget
    delete rootItem_;
    rootItem_ = new EntityTreeItem(0, NULL); // has no parent
    delayedAdds_.clear();  // clear any delayed entities since building from the data store
    itemsById_.clear();

    // Get platform objects from DataStore
    simData::DataStore::IdList platformList;
    dataStore_->idList(&platformList, simData::DataStore::PLATFORM);
    buildTree_(simData::DataStore::PLATFORM, dataStore_, platformList, NULL);
    endResetModel();
  }
}

EntityTreeItem* EntityTreeModel::findItem_(uint64_t entityId) const
{
  std::map<simData::ObjectId, EntityTreeItem*>::const_iterator it = itemsById_.find(entityId);
  if (it != itemsById_.end())
    return it->second;

  return NULL;
}

void EntityTreeModel::addTreeItem_(uint64_t id, simData::DataStore::ObjectType type, uint64_t parentId)
{
  EntityTreeItem* found = findItem_(id);
  // adding a duplicate
  assert(found == NULL);
  if (found != NULL)
    return;

  EntityTreeItem* parentItem;
  if (parentId == 0)
    parentItem = rootItem_;
  else
    parentItem = findItem_(parentId);

  // itemsById_ is out of sync WRT tree
  assert(itemsById_.find(id) == itemsById_.end());

  if ((parentItem != rootItem_) && treeView_)
  {
    EntityTreeItem* newItem = new EntityTreeItem(id, parentItem);
    itemsById_[id] = newItem;
    parentItem->appendChild(newItem);
    if (newItem->row() == 0)
      emit(layoutAboutToBeChanged());
    beginInsertRows(createIndex(parentItem->row(), 0, parentItem), newItem->row(), newItem->row());
    insertRow(newItem->row(), createIndex(newItem->row(), 0, newItem));
    endInsertRows();
    if (newItem->row() == 0)  // if first one, needed to get the |> to show up on the tree
      emit(layoutChanged());
  }
  else
  {
    beginInsertRows(QModelIndex(), rootItem_->childCount(), rootItem_->childCount());
    EntityTreeItem* newItem = new EntityTreeItem(id, rootItem_);
    itemsById_[id] = newItem;
    rootItem_->appendChild(newItem);
    insertRow(newItem->row());
    endInsertRows();
  }
}

void EntityTreeModel::removeEntity_(uint64_t id)
{
  EntityTreeItem* found = findItem_(id);
  if (found == NULL)
  {
    // slight chance it might be delayed
    std::vector<simData::ObjectId>::iterator it = std::find(delayedAdds_.begin(), delayedAdds_.end(), id);
    if (it != delayedAdds_.end())
      delayedAdds_.erase(it);

    // lost track of it, this can happen if the parent is deleted before its children
    return;
  }

  // Qt requires we notify it of all the rows to be removed
  if (found->parent() != rootItem_)
  {
    beginRemoveRows(createIndex(found->row(), 0, found->parent()), found->row(), found->row() + found->childCount());
  }
  else
  {
    beginRemoveRows(QModelIndex(), found->row(), found->row() + found->childCount());
  }

  removeRows(found->row(), found->childCount());

  // Get any children before deleting
  std::vector<uint64_t> ids;
  found->getChildrenIds(ids);

  // tell the found item's parent to remove that item
  found->parent()->removeChild(found);

  // remove the item
  itemsById_.erase(id);
  // now remove any children
  Q_FOREACH(uint64_t deleteId, ids)
    itemsById_.erase(deleteId);

  endRemoveRows();
}

void EntityTreeModel::removeAllEntities_()
{
  if (dataStore_)
  {
    beginResetModel();

    delete rootItem_;
    rootItem_ = new EntityTreeItem(0, NULL);
    delayedAdds_.clear();
    itemsById_.clear();

    endResetModel();
  }
}

int EntityTreeModel::columnCount(const QModelIndex &parent) const
{
  return 3;  // The 3 columns of Name, Type and Original ID
}

QVariant EntityTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole)
  {
    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
    if (index.column() == 0)
      return QString::fromStdString(simData::DataStoreHelpers::nameOrAliasFromId(item->id(), dataStore_));
    if (index.column() == 1)
      return QString::fromStdString(simData::DataStoreHelpers::typeFromId(item->id(), dataStore_));
    if (index.column() == 2)
      return QString("%1").arg(simData::DataStoreHelpers::originalIdFromId(item->id(), dataStore_));

    assert(0);
    return QVariant();
  }

  if (role == Qt::TextColorRole)
  {
    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
    if (index.column() == 0)
    {
      // If the user asked for alias, but it is empty use gray color for the displayed name
      simData::DataStore::Transaction transaction;
      const simData::CommonPrefs* prefs = dataStore_->commonPrefs(item->id(), &transaction);
      if (prefs && prefs->usealias() && prefs->alias().empty())
        return QColor(Qt::gray);
    }

    return QVariant();
  }

  if (role == Qt::ToolTipRole)
  {
    EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
    if (index.column() == 0)
    {
      QString toolTip;
      toolTip = "Name: ";
      toolTip += QString::fromStdString(simData::DataStoreHelpers::nameFromId(item->id(), dataStore_));
      toolTip += "\n";
      toolTip += "Alias: ";
      toolTip += QString::fromStdString(simData::DataStoreHelpers::aliasFromId(item->id(), dataStore_));
      toolTip += "\n";
      toolTip += "Type: ";
      toolTip += QString::fromStdString(simData::DataStoreHelpers::fullTypeFromId(item->id(), dataStore_));
      toolTip += "\n";
      toolTip += "Original ID: ";
      toolTip += QString("%1").arg(simData::DataStoreHelpers::originalIdFromId(item->id(), dataStore_));

      simData::DataStore::Transaction transaction;
      const simData::PlatformPrefs* prefs = dataStore_->platformPrefs(item->id(), &transaction);
      if (prefs != NULL)
      {
        toolTip += "\n";
        toolTip += "Model: ";
        std::string model = simVis::Registry::instance()->findModelFile(prefs->icon());
        if (model.empty())
        {
          model = "Model \"" + simCore::toNativeSeparators(prefs->icon()) + "\" not found";
        }
        else
        {
          model = simCore::toNativeSeparators(model);
        }
        toolTip += QString::fromStdString(model);
      }

      return toolTip;
    }

    if (index.column() == 1)
      return QString::fromStdString(simData::DataStoreHelpers::fullTypeFromId(item->id(), dataStore_));

    if (index.column() == 2)
      return "Original ID";

    return QVariant();
  }

  return QVariant();
}

QVariant EntityTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
  {
    if (section == 0)
       return "Name";
    if (section == 1)
      return "Type";
    if (section == 2)
      return "ID";

    assert(0);
    return QVariant();
  }

  // Isn't the bar across the top -- fall back to whatever QAIM does
  return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex EntityTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  EntityTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<EntityTreeItem*>(parent.internalPointer());

  EntityTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

QModelIndex EntityTreeModel::index(uint64_t id) const
{
  EntityTreeItem* item = findItem_(id);
  if (item != NULL)
  {
    return createIndex(item->row(), 0, item);
  }

  return QModelIndex();
}

uint64_t EntityTreeModel::uniqueId(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  EntityTreeItem *childItem = static_cast<EntityTreeItem*>(index.internalPointer());
  if (childItem == NULL)
    return 0;

  return childItem->id();
}

QModelIndex EntityTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  EntityTreeItem *childItem = static_cast<EntityTreeItem*>(index.internalPointer());
  if (childItem == NULL)
    return QModelIndex();

  EntityTreeItem *parentItem = childItem->parent();

  if (parentItem == NULL)
    return QModelIndex();

  if (parentItem == rootItem_)
    return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int EntityTreeModel::rowCount(const QModelIndex &parent) const
{
  EntityTreeItem *parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid())
    parentItem = rootItem_;
  else
    parentItem = static_cast<EntityTreeItem*>(parent.internalPointer());

  return parentItem->childCount();
}

void EntityTreeModel::buildTree_(simData::DataStore::ObjectType type, const simData::DataStore* dataStore,
                                 const simData::DataStore::IdList& idList, EntityTreeItem *parent)
{
  for (simData::DataStore::IdList::const_iterator iter = idList.begin(); iter != idList.end(); ++iter)
  {
    EntityTreeItem* newItem;
    if (parent && treeView_)
      newItem = new EntityTreeItem(*iter, parent);
    else
      newItem = new EntityTreeItem(*iter, rootItem_);

    if (type == simData::DataStore::PLATFORM)
    {
      // for platforms, find all child beams, lasers, lobs, and projectors
      simData::DataStore::IdList idList;
      dataStore->beamIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::BEAM, dataStore, idList, newItem);
      idList.clear();
      dataStore->laserIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::LASER, dataStore, idList, newItem);
      idList.clear();
      dataStore->lobGroupIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::LOB_GROUP, dataStore, idList, newItem);
      idList.clear();
      dataStore->projectorIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::PROJECTOR, dataStore, idList, newItem);
    }
    else if (type == simData::DataStore::BEAM)
    {
      // for beams, find all child gates
      simData::DataStore::IdList idList;
      dataStore->gateIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::GATE, dataStore, idList, newItem);
      // and all projectors
      idList.clear();
      dataStore->projectorIdListForHost(*iter, &idList);
      buildTree_(simData::DataStore::PROJECTOR, dataStore, idList, newItem);
    }

    // now add to tree appropriately
    if (parent && treeView_)
      parent->appendChild(newItem);
    else
      rootItem_->appendChild(newItem);

    itemsById_[newItem->id()] = newItem;
  }
}

}
