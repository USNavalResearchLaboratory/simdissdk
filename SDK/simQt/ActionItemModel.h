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
#ifndef SIMQT_ACTIONITEMMODEL_H
#define SIMQT_ACTIONITEMMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QMap>
#include <QString>
#include "simCore/Common/Common.h"

namespace simQt {

class ActionRegistry;
class Action;

/// Abstract item model that represents an action registry with items
class SDKQT_EXPORT ActionItemModel : public QAbstractItemModel
{
  Q_OBJECT;
public:
  /// constructor
  explicit ActionItemModel(QObject* parent=nullptr);
  virtual ~ActionItemModel();

  /// Changes the registry that is represented in the item model
  void setRegistry(ActionRegistry* registry);
  /// Sets whether the action hotkeys are editable are not (default: editable)
  void setReadOnly(bool readOnly);

  // QAbstractItemModel overrides
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &child) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

signals:
  /// Emitted when a new group is added; useful for connecting to an expand() slot
  void groupAdded(const QModelIndex& idx);

private slots:
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
  ActionRegistry* registry_;
  /// Actions in the registry, sorted by group name
  QList<GroupItem*> groups_;
  /// Maintains read-only flag to control editing
  bool readOnly_;
};

/** Line edit class for editing QKeySequences.  The widget supports most keys that are
 * not preprocessed by the operating system (e.g. Shift+Esc, Alt+Tab).  This has been
 * designed to be used alongside the ActionItemModelDelegate editor for the ActionItemModel
 * item model.
 */
class SDKQT_EXPORT KeySequenceEdit : public QLineEdit
{
  Q_OBJECT;
public:
  /// constructor
  explicit KeySequenceEdit(QWidget* parent=nullptr);
  virtual ~KeySequenceEdit();

  ///@return most recent key set by this widget
  QKeySequence key() const;
  ///@return true if the key sequence is valid
  bool isKeyValid() const;
  /// Sets a key sequence, optionally emitting keyChanged
  void setKey(const QKeySequence& key, bool emitSignal=false);

public slots:

  /**
   * Call this function to notify on key press.  Note that this can be called from the
   * QStyledItemDelegate::eventFilter() function.  This is present in order to accept a
   * larger set of keys than would be available without the eventFilter() override.
   */
  void acceptKey(const QKeyEvent* keyEvent);

protected:
  /**
   * Override the QLineEdit's keyPressEvent and pass the event to acceptKey().  If you
   * have problems with keys like Tab, Shift Tab, Escape, etc., consider looking at whether
   * there is an event filter set up that will omit these keys.  In the case of
   * ActionItemModelDelegate, the eventFilter() code forwards key events to acceptKey()
   * directly in order to bypass filtering of these special keys.
   */
  virtual void keyPressEvent(QKeyEvent* keyEvent);

  /** Override event() to ignore Shortcut and ShortcutOverride events */
  virtual bool event(QEvent* evt);

signals:
  /// Hot key has been changed; newKey.isEmpty() means the key was removed
  void keyChanged(const QKeySequence& newKey);

private:
  QKeySequence key_;
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

private slots:
  /// Helper to emit signals for commit/close in order
  void closeAndCommitEditor_();
};
} // namespace

#endif /* SIMQT_ACTIONITEMMODEL_H */

