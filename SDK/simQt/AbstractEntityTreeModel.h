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
#ifndef SIMQT_ABSTRACT_ENTITYTREE_MODEL_H
#define SIMQT_ABSTRACT_ENTITYTREE_MODEL_H

#include <QTreeWidgetItem>
#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"

namespace simQt {

  enum EntityTreeModelRoles
  {
    SORT_BY_ENTITY_ROLE = Qt::UserRole
  };

  /** An item in the AbstractEntityTreeModel, would be platform, beam, etc */
  class AbstractEntityTreeItem
  {
  public:
    virtual ~AbstractEntityTreeItem() {}

    /** Needs to return the entity's unique ID */
    virtual uint64_t id() const = 0;
  };

  /** An entity model based on QAbstractItemModel, a DataStore version is Simdis::Gui::EntityTreeModel */
  class SDKQT_EXPORT AbstractEntityTreeModel : public QAbstractItemModel
  {
    Q_OBJECT

  public:
    /** Constructor requires parent object */
    AbstractEntityTreeModel(QObject* parent) : QAbstractItemModel(parent) {}
    virtual ~AbstractEntityTreeModel() {}

    /** QAbstractItemModel interface */
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const = 0;

    /** Return an Index based on the entity's ID */
    virtual QModelIndex index(uint64_t id) const = 0;
    /** Return an Index based on the entity's ID; if necessary, process any pending adds */
    virtual QModelIndex index(uint64_t id) = 0;

    /** Return the entity's ID for a given index */
    virtual uint64_t uniqueId(const QModelIndex &index) const = 0;

    /** Returns whether we use an entity icon or type abbreviation for the entity type column */
    virtual bool useEntityIcons() const = 0;

    /** Returns the number of entities that match the given type(s) */
    virtual int countEntityTypes(simData::ObjectType type) const = 0;

  Q_SIGNALS:
    /** Model is about to make extensive changes, the view may want to suppend updates */
    void beginExtendedChanges();
    /** Model finished making extensive changes, the view may want to refresh and start processing changes */
    void endExtendedChanges();
    /** Category data has changed and the model requests the filters be applied */
    void requestApplyFilters();

  public Q_SLOTS:
    /** Swaps the view to the hierarchy tree */
    virtual void setToTreeView() = 0;
    /** Swaps the view to a non-hierarchical list */
    virtual void setToListView() = 0;
    /** Swaps between tree and list view based on a Boolean */
    virtual void toggleTreeView(bool useTree) = 0;
    /** Updates the contents of the frame */
    virtual void forceRefresh() = 0;

    /** Turns on or off entity icons */
    virtual void setUseEntityIcons(bool useIcons) = 0;
  };

}

#endif

