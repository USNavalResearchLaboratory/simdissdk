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
#ifndef SIMQT_ENTITYTREE_MODEL_H
#define SIMQT_ENTITYTREE_MODEL_H

#include <QTreeWidgetItem>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simQt/AbstractEntityTreeModel.h"

namespace simQt {

/// represent one entity (by Unique ID) kept in a tree
class SDKQT_EXPORT EntityTreeItem : public simQt::AbstractEntityTreeItem
{
public:
  /// constructor
  EntityTreeItem(simData::ObjectId id, EntityTreeItem *parent=NULL);
  virtual ~EntityTreeItem();

  virtual uint64_t id() const;

  /// Return all the IDs of the children and their children
  void getChildrenIds(std::vector<uint64_t>& ids) const;

  /**@name Tree management routines
   *@{
   */
  void appendChild(EntityTreeItem *item);
  void removeChild(EntityTreeItem *item);
  EntityTreeItem *child(int row);
  int childCount() const;
  EntityTreeItem *parent();
  int columnCount() const;
  int row() const;
  ///@}

protected:
  simData::ObjectId id_; ///< id of the entity represented
  EntityTreeItem *parentItem_;  ///< parent of the item.  Null if top item
  QList<EntityTreeItem*> childItems_;  ///< Children of item, if any.  If no children, than item is a leaf
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
  /// Return the index of the parent of the item given by index
  virtual QModelIndex parent(const QModelIndex &index) const;
  /// Return the number of rows in the data
  virtual int rowCount(const QModelIndex &parent) const;

  /// Return the unique id for the given index; return 0 on error
  virtual uint64_t uniqueId(const QModelIndex &index) const;

  /** Returns whether we use an entity icon or type abbreviation for the entity type column */
  virtual bool useEntityIcons() const;

  /// Return the dataStore
  simData::DataStore* dataStore() const;

  /// Set whether to use custom rendering objects as top-level items. Defaults to true.
  void setCustomRenderingAsTopLevelItem(bool customAsTopLevel);

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

private slots:
  /** Added any delayed entities */
  void commitDelayedEntities_();

private:
  class TreeListener;

  // Setup the tree
  void setupModelData_(EntityTreeItem *parent, simData::DataStore* dataStore);
  void buildTree_(simData::ObjectType type, const simData::DataStore* dataStore,
    const simData::DataStore::IdList& idList, EntityTreeItem *parent);
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

