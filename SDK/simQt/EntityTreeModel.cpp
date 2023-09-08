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
#include <QString>
#include <QTimer>

#include "simCore/String/Utils.h"
#include "simData/DataStoreHelpers.h"
#ifdef HAVE_SIMVIS
#include "simVis/Registry.h"
#endif
#include "simQt/EntityTreeModel.h"

namespace simQt {

// Performance can drop dramatically if there are too many regions to delete; stop after 50 regions and reset the model
static const size_t MAX_REGIONS = 50;


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
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot) override
  {
    parent_->queueAdd_(newId);
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot) override
  {
    parent_->queueRemoval_(removedId);
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId) override
  {
    parent_->queueNameChange_(changeId);
  }

  /// something has changed in the entity category data
  virtual void onCategoryDataChange(simData::DataStore *source, simData::ObjectId changedId, simData::ObjectType ot) override
  {
    parent_->queueCategoryDataChange_(changedId);
  }

  /// The scenario is about to be deleted
  virtual void onScenarioDelete(simData::DataStore* source) override
  {
    parent_->removeAllEntities_();
  }

  virtual void onChange(simData::DataStore* source) override
  {
    parent_->commitAllDelayed_();
  }

  // Fulfill the interface
  virtual void onPostRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot) override {}
  virtual void onPrefsChange(simData::DataStore *source, simData::ObjectId id) override {}
  virtual void onPropertiesChange(simData::DataStore *source, simData::ObjectId id) override {}
  virtual void onFlush(simData::DataStore* source, simData::ObjectId id) override {}

private:
  EntityTreeModel *parent_; ///< model which receives notices
};

//----------------------------------------------------------------------------
EntityTreeItem::EntityTreeItem(simData::DataStore* ds, simData::ObjectId id, simData::ObjectType type, EntityTreeItem *parent)
  : id_(id),
    type_(type),
    parentItem_(parent),
    markForRemoval_(false)
{
  if (id_ != 0)
  {
    displayName_ = QString::fromStdString(simData::DataStoreHelpers::nameOrAliasFromId(id_, ds));
    typeString_ = QString::fromStdString(simData::DataStoreHelpers::typeFromId(id_, ds));
    checkForHighlight(ds);
  }
  else
  {
    displayName_ = "Scenario Data";
    typeString_ = "";
    highlight_ = false;
  }
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

simData::ObjectType EntityTreeItem::type() const
{
  return type_;
}

QString EntityTreeItem::displayName() const
{
  return displayName_;
}

void EntityTreeItem::setDisplayName(const QString& name)
{
  displayName_ = name;
}

QString EntityTreeItem::typeString() const
{
  return typeString_;
}

void EntityTreeItem::checkForHighlight(simData::DataStore* ds)
{
  if (ds == nullptr)
    return;

  simData::DataStore::Transaction transaction;
  const simData::CommonPrefs* prefs = ds->commonPrefs(id_, &transaction);
  highlight_ = (prefs && prefs->usealias() && prefs->alias().empty());
}

bool EntityTreeItem::highlight() const
{
  return highlight_;
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
    {
      // verify the childToRowIndex_ map is correct
      assert(parentItem_->childItems_[it->second] == this);
      return it->second;
    }
  }

  return 0;
}

void EntityTreeItem::markForRemoval()
{
  // Der error, should not delete the root node
  assert(parentItem_ != nullptr);

  markForRemoval_ = true;
  if (parentItem_ != nullptr)
    parentItem_->notifyParentForRemoval_(this);

  // Technically marking children is not necessary because the
  // data store should automatically delete children.  To be
  // safe, children will still be marked for removal.
  for (auto& child : childItems_)
    child->markChildrenForRemoval_();
}

bool EntityTreeItem::isMarked() const
{
  return markForRemoval_;
}

void EntityTreeItem::notifyParentForRemoval_(EntityTreeItem* child)
{
  childrenMarked_.insert(child->row());
}

void EntityTreeItem::markChildrenForRemoval_()
{
  markForRemoval_ = true;

  for (auto child : childItems_)
    child->markChildrenForRemoval_();
}

int EntityTreeItem::removeMarkedChildren(EntityTreeModel* model)
{
  markForRemoval_ = false;

  // Trim the tree from bottom up
  for (auto child : childItems_)
  {
    if (child->removeMarkedChildren(model) != 0)
      return 1;
  }

  // Nothing to do or leaf node
  if (childrenMarked_.empty())
    return 0;

  // Everything was deleted so clear out and return
  if (static_cast<int>(childrenMarked_.size()) == childItems_.size())
  {
    model->beginRemoval(this, 0, childItems_.size() - 1);
    for (auto ii = 0; ii < childItems_.size(); ++ii)
      model->clearIndex(childItems_[ii]->id());
    childItems_.clear();
    childToRowIndex_.clear();
    childrenMarked_.clear();
    model->endRemoval();
    return 0;
  }

  // For better performance delete continuous regions of children

  // Calculate regions
  std::map<int, int> indexToDelta;
  auto removalIt = childrenMarked_.begin();
  int lastIndex = *removalIt;
  int previousIndex = lastIndex;
  ++removalIt;
  int delta = 1;
  for (; removalIt != childrenMarked_.end(); ++removalIt)
  {
    // If not continuous make a new entry
    if (*removalIt != (previousIndex + 1))
    {
      // If too many, give up and reset the model
      if (indexToDelta.size() > MAX_REGIONS)
        return 1;

      indexToDelta[lastIndex] = delta;
      lastIndex = *removalIt;
      delta = 1;
    }
    else
      ++delta;

    previousIndex = *removalIt;
  }

  indexToDelta[lastIndex] = delta;

  // Delete regions backwards so indexes do not need to be recalculated
  for (auto it = indexToDelta.rbegin(); it != indexToDelta.rend(); ++it)
  {
    // minus one on last argument because Qt is inclusive
    model->beginRemoval(this, it->first, it->first + it->second - 1);

    // remove from the childToRowIndex_ map and from the model's index map
    for (auto ii = it->first; ii < it->first + it->second; ++ii)
    {
      childToRowIndex_.erase(childItems_[ii]);
      model->clearIndex(childItems_[ii]->id());
    }

    // remove from list
    childItems_.erase(childItems_.begin() + it->first, childItems_.begin() + it->first + it->second);

    // Apply the deltas
    for (auto& rowIndex : childToRowIndex_)
    {
      // only need to modify the indexes above the removed region
      if (rowIndex.second > it->first)
        rowIndex.second -= it->second;
    }

    model->endRemoval();
  }

  childrenMarked_.clear();
  return 0;
}

//-----------------------------------------------------------------------------------------

EntityTreeModel::EntityTreeModel(QObject *parent, simData::DataStore* dataStore)
  : AbstractEntityTreeModel(parent),
    rootItem_(nullptr),
    treeView_(false),
    dataStore_(nullptr),
    delayedRemovals_(false),
    delayedCategoryDataChanges_(false),
    timeChangeEntityThreshold_(-1),  // emit signals with each time change
    activeCategoryFilter_(false),
    modelState_(NOMINAL),
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

void EntityTreeModel::queueAdd_(uint64_t entityId)
{
  delayedAdds_.push_back(entityId);
}

void EntityTreeModel::queueNameChange_(uint64_t id)
{
  delayedRenames_.push_back(id);
}

void EntityTreeModel::queueCategoryDataChange_(uint64_t id)
{
  if (!activeCategoryFilter_)
    return;

  delayedCategoryDataChanges_ = true;
}

void EntityTreeModel::commitDelayedAdd_()
{
  for (auto uniqueId : delayedAdds_)
  {
    simData::ObjectType entityType = dataStore_->objectType(uniqueId);
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
      hostId = dataStore_->entityHostId(uniqueId);

    bool entityTypeNeedsHost = (entityType != simData::PLATFORM);
    if (customAsTopLevel_)
      entityTypeNeedsHost = ((entityType != simData::PLATFORM) && (entityType != simData::CUSTOM_RENDERING));

    // Only add the item if it's a valid top level entity, or if it has a valid host
    assert(!((hostId == 0) && entityTypeNeedsHost));
    if ((hostId > 0 || !entityTypeNeedsHost))
    {
      addTreeItem_(uniqueId, entityType, hostId);
    }
  }
  delayedAdds_.clear();
}

void EntityTreeModel::commitAllDelayed_()
{
  // Always kick out while data is changing
  if (modelState_ == DATA_CHANGES)
    return;

  // Kick out early during time change based on the user's option; never kick out if timeChangeEntityThreshold_ = -1
  if (modelState_ == TIME_CHANGES)
  {
    // Kick out until the time changes are done
    if (timeChangeEntityThreshold_ == 0)
      return;

    // Kick out if the number of enties exceeds the threshold value.
    if ((timeChangeEntityThreshold_ > 0) && (static_cast<int>(itemsById_.size()) >= timeChangeEntityThreshold_))
      return;
  }

  // Kick out eary if nothing changed; can happen if only changing time
  if (delayedAdds_.empty() && delayedRenames_.empty() && !delayedRemovals_ && !delayedCategoryDataChanges_)
    return;

  // if model is empyty, just do a rebuild
  if (itemsById_.empty())
  {
    forceRefresh();
    return;
  }

  if (!delayedAdds_.empty() || delayedRemovals_)
  {
    Q_EMIT beginExtendedChanges();

    commitDelayedRemoval_();
    commitDelayedAdd_();
    commitDelayedNameChanged_();
    delayedCategoryDataChanges_ = false;

    Q_EMIT endExtendedChanges();
    return;
  }

  if (delayedCategoryDataChanges_)
  {
    // emit something
    delayedCategoryDataChanges_ = false;
    Q_EMIT requestApplyFilters();
  }

  commitDelayedNameChanged_();
}

void EntityTreeModel::commitDelayedNameChanged_()
{
  if (delayedRenames_.empty())
    return;

  for (auto id : delayedRenames_)
  {
    EntityTreeItem* found = findItem_(id);
    if (found)
    {
      found->setDisplayName(QString::fromStdString(simData::DataStoreHelpers::nameOrAliasFromId(id, dataStore_)));
      found->checkForHighlight(dataStore_);
    }
  }

  if (delayedRenames_.size() == 1)
  {
    EntityTreeItem* found = findItem_(delayedRenames_.front());
    if (found)
    {
      QModelIndex index = createIndex(found->row(), 0, found);
      Q_EMIT dataChanged(index, index);
    }
  }
  else
  {
    if (rootItem_->childCount() > 0)
    {
      QModelIndex start = createIndex(0, 0, rootItem_);
      QModelIndex end = createIndex(rootItem_->childCount() - 1, 0, rootItem_);
      Q_EMIT dataChanged(start, end);
    }
  }

  delayedRenames_.clear();
}

void EntityTreeModel::beginExtendedChange(bool causedByTimeChanges)
{
  modelState_ = causedByTimeChanges ? TIME_CHANGES : DATA_CHANGES;
}

void EntityTreeModel::endExtendedChange()
{
  modelState_ = NOMINAL;
  commitAllDelayed_();
}

void EntityTreeModel::setTimeChangeEntityThreshold(int timeChangeThreshold)
{
  timeChangeEntityThreshold_ = timeChangeThreshold;
}

void EntityTreeModel::setActiveCategoryFilter(bool active)
{
  activeCategoryFilter_ = active;
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
    rootItem_ = new EntityTreeItem(dataStore_, 0, simData::NONE, nullptr); // has no parent
    delayedAdds_.clear();  // clear any delayed entities since building from the data store
    delayedRenames_.clear();
    delayedRemovals_ = false;
    delayedCategoryDataChanges_ = false;
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
    queueRemoval_(0);
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

  if (parentItem == nullptr)
  {
    // itemsById_ is out of sync WRT tree
    assert(false);
    return;
  }

  if ((parentItem != rootItem_) && treeView_)
  {
    beginInsertRows(createIndex(parentItem->row(), 0, parentItem), parentItem->childCount(), parentItem->childCount());
    EntityTreeItem* newItem = new EntityTreeItem(dataStore_, id, type, parentItem);
    itemsById_[id] = newItem;
    parentItem->appendChild(newItem);
    endInsertRows();
  }
  else
  {
    beginInsertRows(QModelIndex(), rootItem_->childCount(), rootItem_->childCount());
    EntityTreeItem* newItem = new EntityTreeItem(dataStore_, id, type, rootItem_);
    itemsById_[id] = newItem;
    rootItem_->appendChild(newItem);
    endInsertRows();
  }
}

void EntityTreeModel::queueRemoval_(uint64_t id)
{
  EntityTreeItem* found = findItem_(id);
  if (found == nullptr)
  {
    // slight chance it might be delayed
    std::vector<simData::ObjectId>::iterator it = std::find(delayedAdds_.begin(), delayedAdds_.end(), id);
    if (it != delayedAdds_.end())
      delayedAdds_.erase(it);

    // lost track of it, this can happen if the parent is deleted before its children,
    // or calling removeEntity after deleting scenario
    return;
  }

  delayedRemovals_ = true;
  found->markForRemoval();
}

void EntityTreeModel::removeAllEntities_()
{
  if (!dataStore_)
    return;

  delayedAdds_.clear();
  delayedRenames_.clear();
  delayedRemovals_ = false;
  delayedCategoryDataChanges_ = false;

  // no point in reseting an empty model
  if ((rootItem_ != nullptr) && (rootItem_->childCount() == 0))
    return;

  beginResetModel();

  delete rootItem_;
  rootItem_ = new EntityTreeItem(dataStore_, 0, simData::NONE, nullptr);
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
  if ((item == nullptr) || item->isMarked())
    return QVariant();

  switch (role)
  {
  case Qt::DisplayRole:
    if (index.column() == 0)
      return item->displayName();
    if (index.column() == 1)
    {
      if ((useEntityIcons_) || (item->id() == 0))
        return QVariant();
      return item->typeString();
    }
    if (index.column() == 2)
    {
      if (item->id() == 0)
        return QVariant();
      return static_cast<qulonglong>(simData::DataStoreHelpers::originalIdFromId(item->id(), dataStore_));
    }

    // Invalid index encountered
    assert(0);
    break;

  case Qt::DecorationRole:
    // Only show icon if icons are enabled
    if (useEntityIcons_ && index.column() == 1)
    {
      switch (item->type())
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
      if (item->highlight())
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
      return static_cast<int>(item->type());
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
    commitDelayedAdd_();
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
                                 const simData::DataStore::IdList& ids, EntityTreeItem *parent)
{
  for (simData::DataStore::IdList::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
  {
    EntityTreeItem* newItem;
    if (parent && treeView_)
      newItem = new EntityTreeItem(dataStore_, *iter, type, parent);
    else
      newItem = new EntityTreeItem(dataStore_, *iter, type, rootItem_);

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

int EntityTreeModel::countEntityTypes(simData::ObjectType type) const
{
  return countEntityTypes_(rootItem_, type);
}

int EntityTreeModel::countEntityTypes_(EntityTreeItem* parent, simData::ObjectType type) const
{
  int count = 0;

  if ((parent->type() & type) != 0)
    ++count;

  for (auto ii = 0; ii < parent->childCount(); ++ii)
    count += countEntityTypes_(parent->child(ii), type);

  return count;
}

void EntityTreeModel::commitDelayedRemoval_()
{
  // A pending add can force the removal of entities before the one shot fires
  if (!delayedRemovals_)
    return;

  delayedRemovals_ = false;
  if (rootItem_->removeMarkedChildren(this) != 0)
  {
    // too many regions to delete, give up and reset the model
    forceRefresh();
  }
}

void EntityTreeModel::beginRemoval(EntityTreeItem* parent, int begin, int end)
{
  if (parent != rootItem_)
  {
    const QModelIndex parentIndex = createIndex(parent->row(), 0, parent);
    beginRemoveRows(parentIndex, begin, end);
  }
  else
  {
    beginRemoveRows(QModelIndex(), begin, end);
  }
}

void EntityTreeModel::endRemoval()
{
  endRemoveRows();
}

void EntityTreeModel::clearIndex(uint64_t id)
{
  itemsById_.erase(id);
}

}
