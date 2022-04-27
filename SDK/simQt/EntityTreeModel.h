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
#ifndef SIMQT_ENTITYTREE_MODEL_H
#define SIMQT_ENTITYTREE_MODEL_H

#include <set>
#include <QTreeWidgetItem>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simQt/AbstractEntityTreeModel.h"

namespace simQt {

class EntityTreeModel;

/// represent one entity (by Unique ID) kept in a tree
class SDKQT_EXPORT EntityTreeItem : public simQt::AbstractEntityTreeItem
{
public:
  /// constructor
  EntityTreeItem(simData::ObjectId id, simData::ObjectType type, EntityTreeItem *parent=nullptr);
  virtual ~EntityTreeItem();

  /// Returns the Unique ID for the item
  virtual uint64_t id() const;

  /// Returns the type for the item
  simData::ObjectType type() const;

  /// Return all the IDs of the children and their children
  void getChildrenIds(std::vector<uint64_t>& ids) const;

  /**@name Tree management routines
   *@{
   */
  void appendChild(EntityTreeItem *item);
  EntityTreeItem *child(int row);
  int childCount() const;
  EntityTreeItem *parent();
  int columnCount() const;
  int row() const;
  ///@}

  /// Mark the item for removal
  void markForRemoval();
  /// Return true if the item is marked for removal
  bool isMarked() const;
  /**
   * Remove the children marked for removal; recursive down to the leaf node
   * @param model The model for the items, needed to generate the appropriate Qt signals
   * @return 0 on success; non zero on failure and the model must be rebuilt.
   */
  int removeMarkedChildren(EntityTreeModel* model);

protected:
  void notifyParentForRemoval_(EntityTreeItem* child);
  void markChildrenForRemoval_();

  simData::ObjectId id_; ///< id of the entity represented
  simData::ObjectType type_; ///< type of the entity
  EntityTreeItem *parentItem_;  ///< parent of the item.  Null if top item
  QList<EntityTreeItem*> childItems_;  ///< Children of item, if any.  If no children, than item is a leaf
  std::map<const EntityTreeItem*, int> childToRowIndex_; ///< Use a map to cache the row index for better performance
  bool markForRemoval_;  ///< This item is marked for removal
  std::set<int> childrenMarked_;  ///< Children of this item that are marked for removal
};

/// model (data representation) for a tree of Entities (Platforms, Beams, Gates, etc.)
class SDKQT_EXPORT EntityTreeModel : public simQt::AbstractEntityTreeModel
{
  Q_OBJECT

public:
  /// constructor
  EntityTreeModel(QObject *parent, simData::DataStore* dataStore);
  virtual ~EntityTreeModel();

  /// Return number of columns needed to hold data
  virtual int columnCount(const QModelIndex &parent) const;
  /// Return data for given item
  virtual QVariant data(const QModelIndex &index, int role) const;
  /// Return the header data for given section
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  /// Return the index for the given row and column
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  /// Return the index for the given id
  virtual QModelIndex index(uint64_t id) const;
  /// Return an Index based on the entity's ID; if necessary, process any pending adds
  virtual QModelIndex index(uint64_t id);
  /// Return the index of the parent of the item given by index
  virtual QModelIndex parent(const QModelIndex &index) const;
  /// Return the number of rows in the data
  virtual int rowCount(const QModelIndex &parent) const;

  /// Return the unique id for the given index; return 0 on error
  virtual uint64_t uniqueId(const QModelIndex &index) const;

  /** Returns whether we use an entity icon or type abbreviation for the entity type column */
  virtual bool useEntityIcons() const;

  /** Returns the number of entities that match the given type(s) */
  virtual int countEntityTypes(simData::ObjectType type) const;

  /// Return the dataStore
  simData::DataStore* dataStore() const;

  /// Set whether to use custom rendering objects as top-level items. Defaults to true.
  void setCustomRenderingAsTopLevelItem(bool customAsTopLevel);

  /// Should only be called by EntityTreeItem.  Starts the removal of items with the model
  void beginRemoval(EntityTreeItem* parent, int begin, int end);
  /// Should only be called by EntityTreeItem.  Ends the removal of items with the model
  void endRemoval();
  /// Should only be called by QtEntityTreeItem.  Removes the id from itemsById_
  void clearIndex(uint64_t id);

public slots:
  /** Swaps the view to the hierarchy tree */
  virtual void setToTreeView();
  /** Swaps the view to a non-hierarchical list */
  virtual void setToListView();
  /** Swaps between tree and list view based on a Boolean */
  virtual void toggleTreeView(bool useTree);
  /** Updates the contents of the frame */
  virtual void forceRefresh();

  /** Turns on or off entity icons */
  virtual void setUseEntityIcons(bool useIcons);

  /** Changes out the data store pointers, unregistering and re-registering observers */
  void setDataStore(simData::DataStore* dataStore);

  /**
   * Allows for the displaying / hiding of a scenario entry
   * @param showScenario If true the model will contain an entry for scenario
   */
  void setIncludeScenario(bool showScenario);

private slots:
  /** Add any delayed entities */
  void commitDelayedAdd_();
  /** Remove any delayed entities */
  void commitDelayedRemoval_();

private:
  class TreeListener;

  // Setup the tree
  void buildTree_(simData::ObjectType type, const simData::DataStore* dataStore,
    const simData::DataStore::IdList& ids, EntityTreeItem *parent);
  EntityTreeItem* findItem_(uint64_t entityId) const;
  void addTreeItem_(uint64_t id, simData::ObjectType type, uint64_t parentId);

  /// Removes the entity specified by the id
  void removeEntity_(uint64_t id);
  /// Remove all entities from the model
  void removeAllEntities_();
  /// Adds the entity specified by the id
  void addEntity_(uint64_t entityId);
  /// The entity specified by the id has either an new name or its category data changed
  void emitEntityDataChanged_(uint64_t entityId);
  /// Recursively counts the entities that match the given type(s)
  int countEntityTypes_(EntityTreeItem* parent, simData::ObjectType type) const;

  EntityTreeItem *rootItem_;  ///< Top of the entity tree
  std::map<simData::ObjectId, EntityTreeItem*> itemsById_; ///< same information as rootItem, but keyed off of Object ID
  bool treeView_;   ///< true = tree view; false = list view
  simData::DataStore* dataStore_;
  simData::DataStore::ListenerPtr listener_;

  /**
   * Immediately adding an entity can cause flashing if the tree view has category filtering.
   * Delay the adding of the entity to give some time for the data source to set the category
   * data.
   */
  std::vector<simData::ObjectId> delayedAdds_;
  /**
   * Deleting immediately can result in poor performance.  Instead, mark entities for removal
   * and efficiently remove everyone at the same time.
   */
  bool pendingRemoval_;

  /** Icons for entity types */
  QIcon platformIcon_;
  QIcon beamIcon_;
  QIcon customRenderingIcon_;
  QIcon gateIcon_;
  QIcon laserIcon_;
  QIcon lobIcon_;
  QIcon projectorIcon_;

  /// If true, entity icons are used instead of entity letters
  bool useEntityIcons_;
  /// If true, use custom rendering objects as top-level objects. Defaults to true
  bool customAsTopLevel_;
};

} // namespace

#endif /* SIMQT_ENTITYTREE_MODEL_H */

