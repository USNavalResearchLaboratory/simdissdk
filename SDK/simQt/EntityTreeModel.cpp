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
#include <QString>
#include <QTimer>

#include "simCore/String/Utils.h"
#include "simData/DataStoreHelpers.h"
#ifdef HAVE_SIMVIS
#include "simVis/Registry.h"
#endif
#include "simQt/EntityTreeModel.h"

namespace simQt {

/// notify the tree model about data store changes
class EntityTreeModel::TreeListener : public simData::DataStore::Listener
{
public:
  /// constructor
  explicit TreeListener(EntityTreeModel *parent)
  : parent_(parent)
  {
  }

  /// new entity has been added, with the given id and type
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    parent_->addEntity_(newId);
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    parent_->removeEntity_(removedId);
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    parent_->emitEntityDataChanged_(changeId);
  }

  /// something has changed in the entity category data
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot)
  {
    parent_->emitEntityDataChanged_(changedId);
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source)
  {
    parent_->removeAllEntities_();
  }

  // Fulfill the interface
  virtual void onPostRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot) {}
  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id) {}
  virtual void onChange(simData::DataStore *source) {}
  virtual void onFlush(simData::DataStore* source, simData::ObjectId id) {}

private:
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
  childToRowIndex_[item] = childItems_.size();
  childItems_.append(item);
}

void EntityTreeItem::removeChild(EntityTreeItem *item)
{
  auto it = childToRowIndex_.find(item);
  if (it != childToRowIndex_.end())
  {
    int removedIndex = it->second;
    for (auto& index : childToRowIndex_)
    {
      if (index.second > removedIndex)
        index.second--;
    }
  }
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
  for (auto it = childItems_.begin(); it != childItems_.end(); ++it)
  {
    ids.push_back((*it)->id());
    (*it)->getChildrenIds(ids);
  }
}

EntityTreeItem* EntityTreeItem::parent()
{
  return parentItem_;
}

int EntityTreeItem::row() const
{
  if (parentItem_)
  {
    auto it = parentItem_->childToRowIndex_.find(this);
    if (it != parentItem_->childToRowIndex_.end())
      return it->second;
  }

  return 0;
}

//-----------------------------------------------------------------------------------------

EntityTreeModel::EntityTreeModel(QObject *parent, simData::DataStore* dataStore)
  : AbstractEntityTreeModel(parent),
    rootItem_(nullptr),
    treeView_(false),
    dataStore_(nullptr),
    platformIcon_(":/simQt/images/platform.png"),
    beamIcon_(":/simQt/images/beam.png"),
    customRenderingIcon_(":/simQt/images/CustomRender.png"),
    gateIcon_(":/simQt/images/gate.png"),
    laserIcon_(":/simQt/images/laser.png"),
    lobIcon_(":/simQt/images/lob.png"),
    projectorIcon_(":/simQt/images/projector.png"),
    useEntityIcons_(true),
    customAsTopLevel_(true)
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
  setDataStore(nullptr);
  delete rootItem_;
}

void EntityTreeModel::setCustomRenderingAsTopLevelItem(bool customAsTopLevel)
{
  if (customAsTopLevel_ == customAsTopLevel)
    return;
  customAsTopLevel_ = customAsTopLevel;
  forceRefresh();
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
    simData::ObjectType entityType = dataStore_->objectType(*it);
    if (entityType == simData::NONE)
    {
      // the entity should have been removed from the vector
      assert(false);
      continue;
    }

    // Pick out the host's id (0 for platforms (and some custom renderings if they are being treated as top-level))
    uint64_t hostId = 0;
    bool getHostId = (entityType != simData::PLATFORM);
    // Even if allowing custom rendering to be top level, only those with host ID = 0 are.  Still need to check host ID
    if (getHostId)
      hostId = dataStore_->entityHostId(*it);

    bool entityTypeNeedsHost = (entityType != simData::PLATFORM);
    if (customAsTopLevel_)
      entityTypeNeedsHost = ((entityType != simData::PLATFORM) && (entityType != simData::CUSTOM_RENDERING));

    // Only add the item if it's a valid top level entity, or if it has a valid host
    assert(!((hostId == 0) && entityTypeNeedsHost));
    if ((hostId > 0 || !entityTypeNeedsHost))
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
    rootItem_ = new EntityTreeItem(0, nullptr); // has no parent
    delayedAdds_.clear();  // clear any delayed entities since building from the data store
    itemsById_.clear();

    // Get platform objects from DataStore
    simData::DataStore::IdList platformList;
    dataStore_->idList(&platformList, simData::PLATFORM);
    buildTree_(simData::PLATFORM, dataStore_, platformList, nullptr);
    if (customAsTopLevel_)
    {
      // Get custom rendering objects from DataStore
      simData::DataStore::IdList crList;
      dataStore_->idList(&crList, simData::CUSTOM_RENDERING);
      // Only use top-level custom renderings
      simData::DataStore::IdList topLevelCrList;
      for (auto it = crList.begin(); it != crList.end(); ++it)
      {
        auto hostId = dataStore_->entityHostId(*it);
        if (hostId == 0)
          topLevelCrList.push_back(*it);
      }
      buildTree_(simData::CUSTOM_RENDERING, dataStore_, topLevelCrList, nullptr);
    }
    endResetModel();
  }
}

EntityTreeItem* EntityTreeModel::findItem_(uint64_t entityId) const
{
  std::map<simData::ObjectId, EntityTreeItem*>::const_iterator it = itemsById_.find(entityId);
  if (it != itemsById_.end())
    return it->second;

  return nullptr;
}

void EntityTreeModel::setIncludeScenario(bool showScenario)
{
  bool currentShow = (findItem_(0) != nullptr);
  if (currentShow == showScenario)
    return;  // nothing changed

  if (showScenario)
    addTreeItem_(0, simData::NONE, 0);
  else
    removeEntity_(0);
}

void EntityTreeModel::addTreeItem_(uint64_t id, simData::ObjectType type, uint64_t parentId)
{
  EntityTreeItem* found = findItem_(id);
  // adding a duplicate
  assert(found == nullptr);
  if (found != nullptr)
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
    beginInsertRows(createIndex(parentItem->row(), 0, parentItem), parentItem->childCount(), parentItem->childCount());
    EntityTreeItem* newItem = new EntityTreeItem(id, parentItem);
    itemsById_[id] = newItem;
    parentItem->appendChild(newItem);
    endInsertRows();
  }
  else
  {
    beginInsertRows(QModelIndex(), rootItem_->childCount(), rootItem_->childCount());
    EntityTreeItem* newItem = new EntityTreeItem(id, rootItem_);
    itemsById_[id] = newItem;
    rootItem_->appendChild(newItem);
    endInsertRows();
  }
}

void EntityTreeModel::removeEntity_(uint64_t id)
{
  EntityTreeItem* found = findItem_(id);
  if (found == nullptr)
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
    const QModelIndex parentIndex = createIndex(found->parent()->row(), 0, found->parent());
    beginRemoveRows(parentIndex, found->row(), found->row());
  }
  else
  {
    beginRemoveRows(QModelIndex(), found->row(), found->row());
  }

  // Get any children before deleting
  std::vector<uint64_t> ids;
  found->getChildrenIds(ids);

  // tell the found item's parent to remove that item
  found->parent()->removeChild(found);

  // remove the item
  itemsById_.erase(id);
  // now remove any children
  for (auto it = ids.begin(); it != ids.end(); ++it)
    itemsById_.erase(*it);

  endRemoveRows();
}

void EntityTreeModel::removeAllEntities_()
{
  if (!dataStore_)
    return;

  delayedAdds_.clear();

  // no point in reseting an empty model
  if ((rootItem_ != nullptr) && (rootItem_->childCount() == 0))
    return;

  beginResetModel();

  delete rootItem_;
  rootItem_ = new EntityTreeItem(0, nullptr);
  itemsById_.clear();

  endResetModel();
}

int EntityTreeModel::columnCount(const QModelIndex &parent) const
{
  return 3;  // The 3 columns of Name, Type and Original ID
}

QVariant EntityTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  EntityTreeItem *item = static_cast<EntityTreeItem*>(index.internalPointer());
  if (item == nullptr)
    return QVariant();

  switch (role)
  {
  case Qt::DisplayRole:
    if (index.column() == 0)
    {
      if (item->id() == 0)
        return "Scenario Data";
      return QString::fromStdString(simData::DataStoreHelpers::nameOrAliasFromId(item->id(), dataStore_));
    }
    if (index.column() == 1)
    {
      if ((useEntityIcons_) || (item->id() == 0))
        return QVariant();
      return QString::fromStdString(simData::DataStoreHelpers::typeFromId(item->id(), dataStore_));
    }
    if (index.column() == 2)
    {
      if (item->id() == 0)
        return QVariant();
      return QString("%1").arg(simData::DataStoreHelpers::originalIdFromId(item->id(), dataStore_));
    }

    // Invalid index encountered
    assert(0);
    break;

  case Qt::DecorationRole:
    // Only show icon if icons are enabled
    if (useEntityIcons_ && index.column() == 1)
    {
      switch (dataStore_->objectType(item->id()))
      {
      case simData::PLATFORM:
        return platformIcon_;
      case simData::BEAM:
        return beamIcon_;
      case simData::CUSTOM_RENDERING:
        return customRenderingIcon_;
      case simData::GATE:
        return gateIcon_;
      case simData::LASER:
        return laserIcon_;
      case simData::LOB_GROUP:
        return lobIcon_;
      case simData::PROJECTOR:
        return projectorIcon_;
      case simData::NONE:
      case simData::ALL:
        break;
      }
    }
    break;

  case Qt::TextColorRole:
    if (index.column() == 0)
    {
      // If the user asked for alias, but it is empty use gray color for the displayed name
      simData::DataStore::Transaction transaction;
      const simData::CommonPrefs* prefs = dataStore_->commonPrefs(item->id(), &transaction);
      if (prefs && prefs->usealias() && prefs->alias().empty())
        return QColor(Qt::gray);
    }
    break;

  case Qt::ToolTipRole:
    if (index.column() == 0)
    {
      if (item->id() == 0)
        return tr("Scenario Data");

      QString toolTip = tr("Name: %1\nAlias: %2\nType: %3\nOriginal ID: %4")
        .arg(QString::fromStdString(simData::DataStoreHelpers::nameFromId(item->id(), dataStore_)))
        .arg(QString::fromStdString(simData::DataStoreHelpers::aliasFromId(item->id(), dataStore_)))
        .arg(QString::fromStdString(simData::DataStoreHelpers::fullTypeFromId(item->id(), dataStore_)))
        .arg(simData::DataStoreHelpers::originalIdFromId(item->id(), dataStore_));

#ifdef HAVE_SIMVIS
      simData::DataStore::Transaction transaction;
      const simData::PlatformPrefs* prefs = dataStore_->platformPrefs(item->id(), &transaction);
      if (prefs == nullptr)
        return toolTip;

      const std::string model = simVis::Registry::instance()->findModelFile(prefs->icon());
      QString modelTip;
      if (model.empty())
        modelTip = tr("Model: Model \"%1\" not found").arg(QString::fromStdString(simCore::toNativeSeparators(prefs->icon())));
      else
        modelTip = tr("Model: %1").arg(QString::fromStdString(simCore::toNativeSeparators(model)));
      return tr("%1\n%2").arg(toolTip, modelTip);
#else
      return toolTip;
#endif
    }

    if (index.column() == 1)
      return QString::fromStdString(simData::DataStoreHelpers::fullTypeFromId(item->id(), dataStore_));

    if (index.column() == 2)
    {
      if (item->id() != 0)
        return tr("Original ID");
    }
    break;

  case SORT_BY_ENTITY_ROLE:
    if (index.column() == 1)
    {
      // Use ints to force entity types into desired order whether they're currently being displayed as icons or text
      return static_cast<int>(dataStore_->objectType(item->id()));
    }
    break;
  }

  return QVariant();
}

QVariant EntityTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
  {
    if (role == Qt::DisplayRole)
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
    // Explain special display cases to the user in the name column's tooltip
    if (role == Qt::ToolTipRole && section == 0)
    {
      return tr("Entities which are set to use their alias but have no alias to use are listed in gray.\n\nEntities which are set to be listed despite not matching the current filter are listed in italics.");
    }
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
  if (item != nullptr)
  {
    return createIndex(item->row(), 0, item);
  }

  return QModelIndex();
}

QModelIndex EntityTreeModel::index(uint64_t id)
{
  EntityTreeItem* item = findItem_(id);
  if (item == nullptr)
  {
    commitDelayedEntities_();
    item = findItem_(id);
    if (item == nullptr)
      return QModelIndex();
  }

  return createIndex(item->row(), 0, item);
}

uint64_t EntityTreeModel::uniqueId(const QModelIndex &index) const
{
  if (!index.isValid())
    return 0;

  EntityTreeItem *childItem = static_cast<EntityTreeItem*>(index.internalPointer());
  if (childItem == nullptr)
    return 0;

  return childItem->id();
}

QModelIndex EntityTreeModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  EntityTreeItem *childItem = static_cast<EntityTreeItem*>(index.internalPointer());
  if (childItem == nullptr)
    return QModelIndex();

  EntityTreeItem *parentItem = childItem->parent();

  if (parentItem == nullptr)
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

void EntityTreeModel::buildTree_(simData::ObjectType type, const simData::DataStore* dataStore,
                                 const simData::DataStore::IdList& idList, EntityTreeItem *parent)
{
  for (simData::DataStore::IdList::const_iterator iter = idList.begin(); iter != idList.end(); ++iter)
  {
    EntityTreeItem* newItem;
    if (parent && treeView_)
      newItem = new EntityTreeItem(*iter, parent);
    else
      newItem = new EntityTreeItem(*iter, rootItem_);

    if (type == simData::PLATFORM)
    {
      // for platforms, find all child beams, lasers, lobs, and projectors
      simData::DataStore::IdList idList;
      dataStore->beamIdListForHost(*iter, &idList);
      buildTree_(simData::BEAM, dataStore, idList, newItem);
      idList.clear();
      dataStore->customRenderingIdListForHost(*iter, &idList);
      buildTree_(simData::CUSTOM_RENDERING, dataStore, idList, newItem);
      idList.clear();
      dataStore->laserIdListForHost(*iter, &idList);
      buildTree_(simData::LASER, dataStore, idList, newItem);
      idList.clear();
      dataStore->lobGroupIdListForHost(*iter, &idList);
      buildTree_(simData::LOB_GROUP, dataStore, idList, newItem);
      idList.clear();
      dataStore->projectorIdListForHost(*iter, &idList);
      buildTree_(simData::PROJECTOR, dataStore, idList, newItem);
    }
    else if (type == simData::BEAM)
    {
      // for beams, find all child gates
      simData::DataStore::IdList idList;
      dataStore->gateIdListForHost(*iter, &idList);
      buildTree_(simData::GATE, dataStore, idList, newItem);
      // and all projectors
      idList.clear();
      dataStore->projectorIdListForHost(*iter, &idList);
      buildTree_(simData::PROJECTOR, dataStore, idList, newItem);
    }

    // other object types are not expected to have any children objects

    // now add to tree appropriately
    if (parent && treeView_)
      parent->appendChild(newItem);
    else
      rootItem_->appendChild(newItem);

    itemsById_[newItem->id()] = newItem;
  }
}

void EntityTreeModel::setUseEntityIcons(bool useIcons)
{
  if (useEntityIcons_ != useIcons)
  {
    useEntityIcons_ = useIcons;
    forceRefresh();
  }
}

bool EntityTreeModel::useEntityIcons() const
{
  return useEntityIcons_;
}

}
