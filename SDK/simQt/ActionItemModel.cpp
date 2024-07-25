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
#include <cassert>
#include <QAction>
#include <QKeyEvent>
#include "simQt/ActionRegistry.h"
#include "simQt/KeySequenceEdit.h"
#include "simQt/ActionItemModel.h"

namespace simQt {

/**@file ActionItemModel.cpp SYNOPSIS for ActionItemModel
 * The ActionItemModel shows the current state of a ActionRegistry.
 *
 * Internally, the action registry is represented as a tree.  The tree is stored
 * in groups_, which stores all the group tree items along the root (represented
 * by class GroupItem).  Each GroupItem has at least one child representing the
 * action, using the class ActionItem.  GroupItem and ActionItem derive from a common
 * base class, and the single interface makes the model interaction much easier.
 */


/** Interface for a helper class that organizes the tree display */
class ActionItemModel::TreeItem
{
public:
  virtual ~TreeItem() {}

  /**@name accessors
   *@{
   */
  virtual QString title() const = 0;
  virtual QVariant text(int col) const = 0;
  virtual QVariant decoration(int col) const = 0;
  virtual Qt::ItemFlags flags(int col) const = 0;

  virtual int row() const = 0;
  virtual int numColumns() const = 0;

  /// Like QAbstractItemModel::setData(), returns true on successful handle
  virtual bool setData(int col, const QVariant& value) = 0;
  ///@}

  /**@name tree actions
   *@{
   */
  virtual TreeItem* parent() const = 0;
  virtual TreeItem* child(int row) const = 0;
  virtual int indexOf(TreeItem* child) const = 0;
  virtual int numChildren() const = 0;

  virtual void deleteChild(TreeItem* child) = 0;
  ///@}

  /// find an item corresponding to the given action
  virtual TreeItem* find(const Action* action) const = 0;
};

/** Group-based implementation for the helper tree item class */
class ActionItemModel::GroupItem : public ActionItemModel::TreeItem
{
public:
  /// constructor
  GroupItem(const ActionItemModel* model, QString name) : model_(model), name_(name) {}
  virtual ~GroupItem() { qDeleteAll(children_); }

  /**@name accessors
   *@{
   */
  virtual QString title() const { return name_; }
  virtual QVariant text(int col) const { return (col == COL_ACTION) ? QVariant(title()) : QVariant(); }
  virtual QVariant decoration(int col) const { return QVariant(); }
  virtual Qt::ItemFlags flags(int col) const { return Qt::ItemIsEnabled; }

  virtual int row() const { return model_->groups_.indexOf(const_cast<GroupItem*>(this)); }
  virtual int numColumns() const { return 1; }

  /// Like QAbstractItemModel::setData(), returns true on successful handle
  virtual bool setData(int col, const QVariant& value) { return false; }
  ///@}

  /**@name tree actions
   *@{
   */
  virtual TreeItem* parent() const { return nullptr; }
  virtual TreeItem* child(int row) const { return children_[row]; }
  virtual int indexOf(TreeItem* child) const { return children_.indexOf(child); }
  virtual int numChildren() const { return children_.size(); }

  virtual void deleteChild(TreeItem* child)
  {
    children_.removeOne(child);
    delete child;
  }
  ///@}

  /// find an item corresponding to the given action
  virtual TreeItem* find(const Action* action) const
  {
    if (action != nullptr && action->group() == name_)
    { // Search children
      for (auto it = children_.begin(); it != children_.end(); ++it)
      {
        TreeItem* child = (*it)->find(action);
        if (child != nullptr)
          return child;
      }
    }
    return nullptr;
  }

  //--- local functions

  /// add child to end of children
  void appendChild(TreeItem* child)
  {
    children_.push_back(child);
  }

  /// add child at position
  void insertChild(int atIndex, TreeItem* child)
  {
    children_.insert(atIndex, child);
  }

  ///Return the index for alphabetical insertion
  int positionToInsert(const QString& childName) const
  {
    // Find newPosition, the index pointing to the alphabetical insertion point
    int newPosition;
    for (newPosition = 0; newPosition < children_.size(); ++newPosition)
    {
      if (childName < children_[newPosition]->title())
        break;
    }
    return newPosition;
  }

  /**
   * Finds the child with the given name
   *@param name name to match
   *@param rowIndex row containing the match (-1 on no match)
   *@return the item corresponding to the given name, will return nullptr on no match
   */
  TreeItem* findChild(const QString& name, int& rowIndex) const
  {
    int k = 0;
    for (auto it = children_.begin(); it != children_.end(); ++it)
    {
      if ((*it)->title() == name)
      {
        rowIndex = k;
        return *it;
      }
      k++;
    }
    rowIndex = -1;
    return nullptr;
  }

private:
  const ActionItemModel* model_ = nullptr;
  QString name_;
  QList<TreeItem*> children_;
};

/** Action-based implementation for the helper tree item class */
class ActionItemModel::ActionItem : public ActionItemModel::TreeItem
{
public:
  /// constructor
  ActionItem(ActionItemModel::GroupItem* group, Action* action)
    : parent_(group), action_(action)
  {
  }

  /**@name accessors
   *@{
   */
  virtual QString title() const { return action_->description(); }
  virtual QVariant text(int col) const
  {
    if (col == COL_ACTION)
      return action_->description();
    QKeySequence key = action_->hotkeys()[col - 1];
    return key;
  }
  virtual QVariant decoration(int col) const
  {
    if (col == COL_ACTION && action_->action() != nullptr)
      return action_->action()->icon();
    return QVariant();
  }
  virtual Qt::ItemFlags flags(int col) const
  {
    if (col == COL_PRIMARY || col == COL_SECONDARY)
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled;
  }
  virtual int row() const { return parent()->indexOf(const_cast<ActionItem*>(this)); }
  virtual int numColumns() const { return 1 + action_->hotkeys().size(); }

  /// Like QAbstractItemModel::setData(), returns true on successful handle
  virtual bool setData(int col, const QVariant& value)
  {
    if (col != COL_PRIMARY && col != COL_SECONDARY)
      return false;

    const int keyNum = col - 1;
    assert(keyNum == 0 || keyNum == 1); // Guaranteed by above ColumnIndex check
    if (!value.isValid() || value.toString().isEmpty())
    {
      action_->removeHotKey(keyNum);
      return true;
    }
    QKeySequence key(value.toString());
    if (key.isEmpty())
      return false;
    // Set up a new vector of keys
    QList<QKeySequence> keys = action_->hotkeys();
    if (keyNum < keys.size()) // Replace
      keys.replace(keyNum, key);
    else // Append
      keys.push_back(key);
    action_->setHotKeys(keys);
    return true;
  }
  ///@}

  /**@name tree actions
   *@{
   */
  virtual TreeItem* parent() const { return parent_; }
  virtual TreeItem* child(int row) const { return nullptr; }
  virtual int indexOf(TreeItem* child) const { return -1; }
  virtual int numChildren() const { return 0; }

  virtual void deleteChild(TreeItem* child) {}
  ///@}

  /// find an item corresponding to the given action
  virtual TreeItem* find(const Action* action) const { return (action == action_) ? const_cast<ActionItem*>(this) : nullptr; }

private:
  ActionItemModel::GroupItem* parent_ = nullptr;
  Action* action_ = nullptr;
};

//////////////////////////////////////////////////////////////////////
ActionItemModel::ActionItemModel(QObject* parent)
  : QAbstractItemModel(parent),
    registry_(nullptr)
{
}

ActionItemModel::~ActionItemModel()
{
  disconnect_(registry_);
  for (auto it = groups_.begin(); it != groups_.end(); ++it)
    delete *it;
}

void ActionItemModel::setRegistry(ActionRegistry* registry)
{
  // Avoid expensive recalculations if no-op
  if (registry == registry_)
    return;

  beginResetModel();
  disconnect_(registry_);
  registry_ = registry;
  connect_(registry_);

  // Initialize the data for the new registry
  createGroupedList_(groups_);
  endResetModel();
  // Iterate through groups and emit signals
  int row = 0;
  auto groupsCopy = groups_;
  for (auto it = groupsCopy.begin(); it != groupsCopy.end(); ++it)
    Q_EMIT groupAdded(createIndex(row++, 0, *it));
}

QModelIndex ActionItemModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();
  if (!parent.isValid())
    return createIndex(row, column, groups_[row]);
  TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
  // Item was not made correctly, check index()
  assert(parentItem != nullptr);
  return createIndex(row, column, parentItem->child(row));
}

QModelIndex ActionItemModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  TreeItem *childItem = static_cast<TreeItem*>(child.internalPointer());
  // Item was not made correctly, check index()
  assert(childItem != nullptr);
  if (childItem == nullptr)
    return QModelIndex();
  // parentItem should be pointing to a group item
  GroupItem *parentItem = dynamic_cast<GroupItem*>(childItem->parent());
  if (parentItem == nullptr)
    return QModelIndex();
  return createIndex(groups_.indexOf(parentItem), 0, parentItem);
}

int ActionItemModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
  {
    if (parent.column() != COL_ACTION)
      return 0;
    TreeItem* parentItem = static_cast<TreeItem*>(parent.internalPointer());
    return (parentItem == nullptr) ? 0 : parentItem->numChildren();
  }
  return groups_.size();
}

int ActionItemModel::columnCount(const QModelIndex &parent) const
{
  return NUM_COLUMNS;
}

QVariant ActionItemModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (role == Qt::DisplayRole || role == Qt::EditRole)
  {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    // Item was not made correctly, check index()
    assert(item);
    if (index.column() >= item->numColumns())
      return QVariant();
    return item->text(index.column());
  }
  else if (role == Qt::DecorationRole)
  {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item)
      return item->decoration(index.column());
  }
  return QVariant();
}

Qt::ItemFlags ActionItemModel::flags(const QModelIndex& index) const
{
  if (index.isValid())
  {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item != nullptr)
      return item->flags(index.column());
    return Qt::ItemIsEnabled;
  }
  return Qt::NoItemFlags;
}

bool ActionItemModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (index.isValid() && role == Qt::EditRole)
  {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item != nullptr)
      return item->setData(index.column(), value);
  }
  return QAbstractItemModel::setData(index, value, role);
}

QVariant ActionItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
  {
    switch (section)
    {
    case COL_ACTION:
      return tr("Action");
    case COL_PRIMARY:
      return tr("Primary");
    case COL_SECONDARY:
      return tr("Secondary");
    case NUM_COLUMNS:
      // A column was added and this section was not updated
      assert(0);
      return QVariant();
    }
  }

  // Isn't the bar across the top -- fall back to whatever QAIM does
  return QAbstractItemModel::headerData(section, orientation, role);
}

void ActionItemModel::connect_(ActionRegistry* newRegistry)
{
  if (newRegistry == nullptr)
    return;
  connect(newRegistry, SIGNAL(actionAdded(simQt::Action*)), this, SLOT(actionAdded(simQt::Action*)));
  connect(newRegistry, SIGNAL(actionRemoved(const simQt::Action*)), this, SLOT(actionRemoved(const simQt::Action*)));
  connect(newRegistry, SIGNAL(hotKeysChanged(simQt::Action*)), this, SLOT(hotKeysChanged(simQt::Action*)));
}

void ActionItemModel::disconnect_(ActionRegistry* oldRegistry)
{
  if (oldRegistry == nullptr)
    return;
  disconnect(oldRegistry, SIGNAL(actionAdded(simQt::Action*)), this, SLOT(actionAdded(simQt::Action*)));
  disconnect(oldRegistry, SIGNAL(actionRemoved(const simQt::Action*)), this, SLOT(actionRemoved(const simQt::Action*)));
  disconnect(oldRegistry, SIGNAL(hotKeysChanged(simQt::Action*)), this, SLOT(hotKeysChanged(simQt::Action*)));
}

void ActionItemModel::createGroupedList_(QList<GroupItem*>& groups) const
{
  if (registry_ == nullptr)
    return;

  // Query the registry and sort into groups
  QList<Action*> actions = registry_->actions();
  QMap<QString, GroupItem*> sortedMap;
  for (auto it = actions.begin(); it != actions.end(); ++it)
  {
    Action* action = *it;
    QMap<QString, GroupItem*>::iterator iter = sortedMap.find(action->group());
    if (iter == sortedMap.end())
    {
      // Create new group
      GroupItem* newGroup = new GroupItem(this, action->group());
      sortedMap.insert(action->group(), newGroup);
      ActionItem* newAction = new ActionItem(newGroup, action);
      newGroup->appendChild(newAction);
    }
    else
    {
      // Append to existing set
      ActionItem* newAction = new ActionItem(*iter, action);
      (*iter)->appendChild(newAction);
    }
  }

  // Transfer from the sorted map into the list
  for (auto it = groups.begin(); it != groups.end(); ++it)
      delete *it;
  groups.clear();
  for (auto it = sortedMap.begin(); it != sortedMap.end(); ++it)
    groups.push_back(*it);
}

void ActionItemModel::actionAdded(Action* action)
{
  if (action == nullptr)
    return;
  GroupItem* group = findGroup_(action->group());
  // New group to create?
  if (group == nullptr)
  {
    // Find newPosition, the index pointing to the alphabetical insertion point
    int newPosition;
    for (newPosition = 0; newPosition < groups_.size(); ++newPosition)
    {
      if (action->group() < groups_[newPosition]->title())
        break;
    }
    // Insert at "newPosition"
    beginInsertRows(QModelIndex(), newPosition, newPosition);
    group = new GroupItem(this, action->group());
    groups_.insert(newPosition, group);
    endInsertRows();
    // Make sure the item was put in the correct location
    assert(newPosition == group->row());
    Q_EMIT(groupAdded(createIndex(newPosition, 0, group)));
  }

  // Add to group
  int newPosition = group->positionToInsert(action->description());
  QModelIndex groupIdx = createIndex(group->row(), 0, group);
  beginInsertRows(groupIdx, newPosition, newPosition);
  ActionItem* newAction = new ActionItem(group, action);
  group->insertChild(newPosition, newAction);
  endInsertRows();
}

void ActionItemModel::actionRemoved(const Action* action)
{
  TreeItem* item = findAction_(action);
  if (item == nullptr || item->parent() == nullptr)
    return;
  // Case 1: last item in the list, let's just remove the whole group
  TreeItem* parent = item->parent();
  if (parent->numChildren() == 1)
  {
    int parentRow = parent->row();
    beginRemoveRows(QModelIndex(), parentRow, parentRow);
    groups_.removeOne(static_cast<GroupItem*>(parent));
    endRemoveRows();
    delete parent;
    return;
  }

  // Case 2: Is not the last item in the list, just remove this one item
  QModelIndex parentIndex = createIndex(parent->row(), 0, parent);
  int childRow = item->row();
  beginRemoveRows(parentIndex, childRow, childRow);
  parent->deleteChild(item);
  endRemoveRows();
}

void ActionItemModel::hotKeysChanged(Action* action)
{
  QModelIndex idx1 = indexOfAction_(action);
  if (idx1.isValid())
    Q_EMIT(dataChanged(idx1, createIndex(idx1.row(), 2, idx1.internalPointer())));
}

ActionItemModel::GroupItem* ActionItemModel::findGroup_(const QString& name) const
{
  for (auto it = groups_.begin(); it != groups_.end(); ++it)
  {
    if ((*it)->title() == name)
      return *it;
  }
  return nullptr;
}

ActionItemModel::TreeItem* ActionItemModel::findAction_(const Action* action) const
{
  if (action == nullptr)
    return nullptr;
  GroupItem* group = findGroup_(action->group());
  return (group == nullptr) ? nullptr : group->find(action);
}

QModelIndex ActionItemModel::indexOfAction_(Action* action) const
{
  TreeItem* treeItem = findAction_(action);
  if (treeItem != nullptr)
    return createIndex(treeItem->row(), 0, treeItem);
  return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////

ActionItemModelDelegate::ActionItemModelDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

ActionItemModelDelegate::~ActionItemModelDelegate()
{
}

void ActionItemModelDelegate::closeAndCommitEditor_()
{
  KeySequenceEdit* editor = qobject_cast<KeySequenceEdit*>(sender());
  assert(editor != nullptr);
  Q_EMIT commitData(editor);
  Q_EMIT closeEditor(editor);
}

QWidget* ActionItemModelDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  KeySequenceEdit* editor = new KeySequenceEdit(parent);
  connect(editor, SIGNAL(keyChanged(QKeySequence)), this, SLOT(closeAndCommitEditor_()));
  return editor;
}

void ActionItemModelDelegate::setEditorData(QWidget* editWidget, const QModelIndex& index) const
{
  assert(index.isValid());
  if (!index.isValid())
    return;
  KeySequenceEdit* editor = qobject_cast<KeySequenceEdit*>(editWidget);
  assert(editor != nullptr);
  // Pull out the QVariant data from the data model
  QVariant itemData = index.model()->data(index, Qt::DisplayRole);
  if (itemData.isValid())
    editor->setKey(itemData.value<QKeySequence>(), false);
}

void ActionItemModelDelegate::setModelData(QWidget* editWidget, QAbstractItemModel* model, const QModelIndex& index) const
{
  assert(index.isValid());
  if (!index.isValid())
    return;
  // Set the data in the model from our data
  KeySequenceEdit* editor = qobject_cast<KeySequenceEdit*>(editWidget);
  assert(editor != nullptr);
  model->setData(index, editor->key(), Qt::EditRole);
}

bool ActionItemModelDelegate::eventFilter(QObject* editor, QEvent* evt)
{
  if (evt->type() == QEvent::KeyPress)
  {
    const QKeyEvent* keyEvent = static_cast<const QKeyEvent*>(evt);
    KeySequenceEdit* edit = qobject_cast<KeySequenceEdit*>(editor);
    assert(edit != nullptr);
    if (edit != nullptr)
    {
      // Preprocess the key; don't give the filters a chance to handle the key.
      // This prevents weird focus problems with keys like Tab, and avoids issues
      // with special keys like Esc and Enter
      edit->acceptKey(keyEvent);
      return true;
    }
  }
  return QStyledItemDelegate::eventFilter(editor, evt);
}

}
