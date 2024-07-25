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
#ifndef SIMQT_ACTIONITEMMODEL_H
#define SIMQT_ACTIONITEMMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QMap>
#include <QString>
#include "simCore/Common/Common.h"

// For backwards compatibility
#include "simQt/KeySequenceEdit.h"

namespace simQt {

class ActionRegistry;
class Action;

/// Abstract item model that represents an action registry with items
class SDKQT_EXPORT ActionItemModel : public QAbstractItemModel
{
  Q_OBJECT;
public:
  explicit ActionItemModel(QObject* parent=nullptr);
  virtual ~ActionItemModel();

  enum ColumnIndex
  {
    /** Action or group name */
    COL_ACTION = 0,
    /** Primary hot key assignment */
    COL_PRIMARY,
    /** Secondary hot key assignment */
    COL_SECONDARY,

    /** Convenience entry for total number of columns */
    NUM_COLUMNS
  };

  /// Changes the registry that is represented in the item model
  void setRegistry(ActionRegistry* registry);

  // QAbstractItemModel overrides
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &child) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

Q_SIGNALS:
  /// Emitted when a new group is added; useful for connecting to an expand() slot
  void groupAdded(const QModelIndex& idx);

private Q_SLOTS:
  void actionAdded(simQt::Action* action);
  void actionRemoved(const simQt::Action* action);
  void hotKeysChanged(simQt::Action* action);

private:
  /// Internal helper classes, used to organize actions into groups for tree display
  class TreeItem;
  class GroupItem;
  class ActionItem;

  /// Hooks up signals/slots to get notified of changes
  void connect_(ActionRegistry* newRegistry);
  /// Removes signals/slots from a registry that's being disabled
  void disconnect_(ActionRegistry* oldRegistry);
  /// Gathers a new ActionSet from the
  void createGroupedList_(QList<GroupItem*>& groups) const;

  /// Finds a tree item based on the group name
  GroupItem* findGroup_(const QString& name) const;
  /// Finds a tree item based on the action pointer
  TreeItem* findAction_(const Action* action) const;
  /// Retrieves a QModelIndex representing the action
  QModelIndex indexOfAction_(Action* action) const;

  /// Pointer to the current action registry
  ActionRegistry* registry_ = nullptr;
  /// Actions in the registry, sorted by group name
  QList<GroupItem*> groups_;
};

/** Delegate used for editing hotkeys in the ActionItemModel.  Uses the KeySequenceEdit
 * class to represent the hotkeys in a QLineEdit-like format.
 */
class SDKQT_EXPORT ActionItemModelDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  /// constructor
  explicit ActionItemModelDelegate(QObject* parent=nullptr);
  virtual ~ActionItemModelDelegate();

  /// Override createEditor() to return our KeySequenceEdit
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /// Sets the editor data based on the given index
  virtual void setEditorData(QWidget* editWidget, const QModelIndex& index) const;
  /// Called to actually set the data into the data model
  virtual void setModelData(QWidget* editWidget, QAbstractItemModel* model, const QModelIndex& index) const;

protected:
  /// Override the default event filter to permit esc, tab, etc. to pass through to editor
  virtual bool eventFilter(QObject* editor, QEvent* evt);

private Q_SLOTS:
  /// Helper to emit signals for commit/close in order
  void closeAndCommitEditor_();
};

} // namespace

#endif /* SIMQT_ACTIONITEMMODEL_H */
