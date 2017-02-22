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
#include <deque>
#include <QSettings>
#include <QStringList>
#include <QFileIconProvider>
#include <QApplication>
#include "simNotify/Notify.h"
#include "simQt/WidgetSettings.h"
#include "simQt/SettingsModel.h"

namespace simQt {

static const QString& HEADER_NAME = "Name";
static const QString& HEADER_VALUE = "Value";

/** Meta data is stored persistently in QSettings under this folder */
static const QString& METADATA_GROUP = "_MetaData";
/** Meta data about the metadata entries */
static const Settings::MetaData METADATA_METADATA(Settings::MetaData::makeString(QVariant(), "", simQt::Settings::PRIVATE));
/** Meta data for entries without meta data */
static const Settings::MetaData DEFAULT_METADATA(Settings::MetaData::makeString(QVariant(), "", simQt::Settings::ADVANCED));

/** Command Pattern entity for editing a QSettings value. */
class SettingsModel::UserEditCommand
{
public:
  /**
   * Constructs (but does not execute) the user command.
   * @param path Full path to the settings value (key)
   * @param before Value before the edit
   * @param after Value after the edit
   */
  UserEditCommand(QString path, const QVariant& before, const QVariant& after)
    : path_(path),
      before_(before),
      after_(after)
  {
  }
  /** Executes the command on the provided QSettings object */
  void execute(simQt::Settings* settings)
  {
    settings->setValue(path_, after_);
  }
  /** Changes the value back */
  void unexecute(simQt::Settings* settings)
  {
    settings->setValue(path_, before_);
  }
  /** Retrieves the path or key to the value */
  QString key() const
  {
    return path_;
  }
private:
  /// Path (or key) inside QSettings for the value
  QString path_;
  /// Value before it was changed
  QVariant before_;
  /// Value to change to after execution
  QVariant after_;
};


/**
 * Represents a single node inside the settings tree.  Inspired by TreeItem from
 *  http://harmattan-dev.nokia.com/docs/platform-api-reference/xml/daily-docs/libqt4/itemviews-simpletreemodel.html
 */
class SettingsModel::TreeNode
{
public:
  /// Display column 0 is the name of the setting
  static const int COLUMN_NAME = 0;
  /// Display column 1 is the value for the setting
  static const int COLUMN_VALUE = 1;

  /// Root item constructor
  TreeNode(QIcon& icon, const QList<QVariant> &data, TreeNode *parent=NULL)
    : icon_(icon),
      itemData_(data),
      parentItem_(parent),
      forceToPrivate_(false),
      hasMetaData_(false),
      valueChanged_(false),
      metaDataChanged_(false)
  {
    flags_ << (Qt::ItemIsEnabled | Qt::ItemIsSelectable) << (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }

  /// Mid-level nodes representing trees
  TreeNode(QIcon& icon, const QString& nodeName, TreeNode* parent, bool forceToPrivate)
    : icon_(icon),
      parentItem_(parent),
      forceToPrivate_(forceToPrivate),
      hasMetaData_(false),
      valueChanged_(false),
      metaDataChanged_(false)
  {
    itemData_ << nodeName;
    itemData_ << QVariant();
    flags_ << (Qt::ItemIsEnabled | Qt::ItemIsSelectable) << (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  }

  /// Leaf nodes representing values
  TreeNode(QIcon& icon, const QString& nodeName, const QVariant& value, TreeNode* parent, bool forceToPrivate)
    : icon_(icon),
      parentItem_(parent),
      forceToPrivate_(forceToPrivate),
      hasMetaData_(false),
      valueChanged_(false),
      metaDataChanged_(false)
  {
    // Must set metaData with a call to setMetaData
    setMetaData(DEFAULT_METADATA);
    hasMetaData_ = false;  // need to reset after call to setMetaData
    metaDataChanged_ = false;   // need to reset after call to setMetaData
    itemData_ << nodeName;
    itemData_ << value;
  }

  /// Destructor cleans up children (owned)
  virtual ~TreeNode()
  {
    qDeleteAll(childItems_);
  }

  /// Directly set the cached QVariant data value for this node
  void setDataValue(const QVariant& value)
  {
    assert(itemData_.size() > 1);
    itemData_[1] = value;
    valueChanged_ = true;
  }

  /// Adds a new child to the tree; ownership transfers to this
  void appendChild(TreeNode *child)
  {
    childItems_.append(child);
  }

  /// Retrieve the child at the given row index
  TreeNode *child(int row)
  {
    return childItems_.value(row);
  }

  /// Finds a child by name
  TreeNode *findChild(const QString& name)
  {
    for (int k = 0; k < childCount(); ++k)
    {
      TreeNode* childAtK = child(k);
      if (childAtK->path() == name)
        return childAtK;
    }
    return NULL;
  }

  /// Number of children for this item
  int childCount() const
  {
    return childItems_.count();
  }

  /// Columns for this item
  int columnCount() const
  {
    return itemData_.count();
  }

  /// Data call from the model
  QVariant data(int role, int column) const
  {
    switch (role)
    {
    case Qt::DisplayRole: // Used in display
      if (column == COLUMN_NAME)
      {
        if (childCount() == 0) // must be a single setting
          return settingText(fullPath());
        // must be a group; cover special case for root item
        if (isRootItem())
          return itemData_.value(COLUMN_NAME).toString();
        return groupText(fullPath());
      }
      else if (column == COLUMN_VALUE)
      {
        return itemData_.value(column);
      }
      return QVariant();

    case Qt::EditRole: // Used in edit actions
      if (column == COLUMN_VALUE)
        return itemData_.value(column);
      return QVariant();

    case Qt::CheckStateRole: // For check fields
      if (column == COLUMN_VALUE && flags_[column] & Qt::ItemIsUserCheckable)
        return itemData_.value(column).toBool() ? Qt::Checked : Qt::Unchecked;
      return QVariant();

    case Qt::DecorationRole:
      if (isRootItem())
        return QVariant();
      if (column == COLUMN_NAME) // icons only on the first column
        return icon_;
      break;

    case Qt::ToolTipRole:
      if (isRootItem())
        return QVariant();
      if (childCount() == 0) // must be a single setting
        return settingTooltip(fullPath());
      return groupTooltip(fullPath());

    case SettingsModel::DataLevelRole:
      if (forceToPrivate_)
        return simQt::Settings::PRIVATE;
      return settingDataLevel(fullPath());

    case SettingsModel::FullyQualifiedNameRole:
      return fullPath();

    case SettingsModel::MetaDataRole:
    {
      return QVariant::fromValue<simQt::Settings::MetaData>(metaData());
    }
    }

    // Default return value
    return QVariant();
  }

  /// Flags for this data item
  Qt::ItemFlags flags(int column) const
  {
    return flags_.value(column);
  }

  /// Row of this item inside the parent
  int row() const
  {
    if (!isRootItem())
      return parentItem_->childItems_.indexOf(const_cast<TreeNode*>(this));
    // No parent, top level item
    return 0;
  }

  /// Retrieve parent node if any
  TreeNode *parent()
  {
    return parentItem_;
  }

  /// Path without parent (local name)
  QString path() const
  {
    if (isRootItem())
      return "";
    return itemData_.value(COLUMN_NAME).toString();
  }

  /// Fully qualified path (includes parent)
  QString fullPath() const
  {
    if (isRootItem())
      return "";
    // Avoid "/path/to/variable" -- which should be "path/to/variable"
    QString parentPath = parentItem_->fullPath();
    if (!parentPath.isEmpty())
      parentPath += "/";
    return parentPath + itemData_.value(COLUMN_NAME).toString();
  }

  /// Creates a new Command to set the value for this tree item
  UserEditCommand* setValue(const QVariant& oldValue, const QVariant& toValue)
  {
    if (oldValue == toValue)
      return NULL; // noop
    return new UserEditCommand(fullPath(), oldValue, toValue);
  }

  /// Indicates that this is the root / top level item
  bool isRootItem() const
  {
    return parentItem_ == NULL;
  }

  const Settings::MetaData& metaData() const
  {
    return metaData_;
  }

  /**
  * Update meta data value. Note that override is the default.
  * return 0 if meta data changed, non-zero if no change occurred
  */
  int setMetaData(const Settings::MetaData& metaData, bool overrideValue = true)
  {
    // only initialize meta data once with valid values, unless overriding
    if (!overrideValue && hasMetaData_ && metaData_.level() != UNKNOWN)
      return 1;

    // if no change, nothing to do
    if (metaData_ == metaData)
      return 1;

    // overrideValue true means not from a file so an actual change
    if (overrideValue)
      metaDataChanged_ = true;

    hasMetaData_ = true;
    flags_.clear();
    metaData_ = metaData;
    // Flags are special here, because item can be edited
    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    flags_ << itemFlags; // refers to the name

    // Data element flags next
    switch (metaData_.type())
    {
    case Settings::BOOLEAN:
      itemFlags |= Qt::ItemIsUserCheckable;
      break;
    default:
      itemFlags |= Qt::ItemIsEditable;
      break;
    }

    flags_ << itemFlags; // refers to the data
    return 0;
  }

  void fireSettingChange(ObserverPtr skipObserver = ObserverPtr()) const
  {
    QString name = fullPath();
    QVariant value = itemData_.value(COLUMN_VALUE);
    Q_FOREACH(const ObserverPtr& ptr, observers_)
    {
      if (ptr != skipObserver && ptr != NULL)
      {
        ptr->onSettingChange(name, value);
      }
    }
  }

  void addObserver(ObserverPtr observer)
  {
    // Don't add NULL pointers
    if (observer == NULL)
      return;

    // Check for existence of the observer first
    if (observers_.contains(observer))
        return;

    observers_.push_back(observer);
  }

  int removeObserver(ObserverPtr observer)
  {
    for (QList<ObserverPtr>::iterator i = observers_.begin(); i != observers_.end(); ++i)
    {
      if ((*i) == observer)
      {
        observers_.erase(i);
        return 0;
      }
    }
    // Didn't find the observer
    return 1;
  }

  QString groupText(const QString& group) const
  {
    return group.section('/', -1).replace("_", " ");
  }

  QString groupTooltip(const QString& group) const
  {
    return group;
  }

  QString settingText(const QString& setting) const
  {
    return setting.section('/', -1).replace("_", " ");
  }

  QString settingTooltip(const QString& setting) const
  {
    if (metaData_.toolTip().isEmpty())
      return setting;
    return metaData_.toolTip();
  }

  simQt::Settings::DataLevel settingDataLevel(const QString& setting) const
  {
    // Make sure it's not metadata (special case exception)
    if (setting.startsWith(METADATA_GROUP))
      return METADATA_METADATA.level();

    return metaData_.level();
  }

  bool hasValueChanged() const
  {
    return valueChanged_;
  }

  bool hasMetaData() const
  {
    return hasMetaData_;
  }

  bool hasMetaDataChanged() const
  {
    return metaDataChanged_;
  }

private:
  /// Parent / owning data model
  QIcon icon_;
  /// Nodes under this one
  QList<TreeNode*> childItems_;
  /// QList of data for the data() call
  QList<QVariant> itemData_;
  /// List of all flags
  QList<Qt::ItemFlags> flags_;
  /// Points to the parent in the hierarchy
  TreeNode *parentItem_;
  /// If true, ignore the data setting and always return private
  bool forceToPrivate_;
  // Metadata for the node
  MetaData metaData_;
  /// True if metaData_ was set by an EXTERNAL source. The default value set by the node does not count
  bool hasMetaData_;
  /// Local observers; only when this entry changes do a callback
  QList<ObserverPtr> observers_;
  /// True if the value has changed since the initial value
  bool valueChanged_;
  /// True if the metaData has changed since the initial value
  bool metaDataChanged_;
};

////////////////////////////////////////////////////////////////////////////

/**
 * Implements the Settings::Memento interface by storing all tree values and
 * restoring each one.  This has a disadvantage in that newly created settings are
 * not cleared out, but that limitation simplifies the logic here.
 */
class SettingsModel::MementoImpl : public Settings::Memento
{
public:
  explicit MementoImpl(TreeNode* rootNode)
  {
    saveNode_(rootNode);
  }

  /** Restores each saved setting value */
  virtual int restore(Settings& settings) const
  {
    for (auto iter = values_.begin(); iter != values_.end(); ++iter)
      settings.setValue(iter->first, iter->second);
    return 0;
  }

private:
  /** Recursively save the value and iterate through children */
  void saveNode_(TreeNode* node)
  {
    if (node == NULL)
      return;
    const QVariant value = node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);
    if (!node->isRootItem() && value.isValid())
    {
      // Leaf item -- save data and return
      values_[node->fullPath()] = value;
      return;
    }

    // Recurse through child nodes
    int size = node->childCount();
    for (int ii = 0; ii < size; ++ii)
      saveNode_(node->child(ii));
  }

  std::map<QString, QVariant> values_;
};

////////////////////////////////////////////////////////////////////////////

SettingsModel::SettingsModel(QObject* parent, QSettings& settings)
  : QAbstractItemModel(parent),
    rootNode_(NULL)
{
  // Register meta data types so MetaData can go into a QSettings
  qRegisterMetaTypeStreamOperators<simQt::Settings::MetaData>("simQt::Settings::MetaData");

  // Note that QFileIconProvider requires QApplication and will crash with QCoreApplication
  bool hasGuiApp = (qobject_cast<QApplication*>(QCoreApplication::instance()) != NULL);
  if (hasGuiApp)
  {
    QFileIconProvider provider;
    folderIcon_ = provider.icon(QFileIconProvider::Folder);
  }
  else
    folderIcon_ = QIcon();
  noIcon_ = QIcon();

  // reloadModel makes the tree and initMetaData adds to the tree
  reloadModel_(settings);
  initMetaData_(settings);

  // No copy constructor for QSettings so capture the info to re-create for write out
  format_ = settings.format();
  filename_ = settings.fileName();
}

SettingsModel::~SettingsModel()
{
  save();

  delete rootNode_;
  rootNode_ = NULL;
  qDeleteAll(undoStack_);
  undoStack_.clear();
  qDeleteAll(redoStack_);
  redoStack_.clear();
}

void SettingsModel::save()
{
  QSettings settings(filename_, format_);
  // Create a settings file for output
  if (settings.isWritable())
  {
    // Cannot call settings.clear() here because some code by pass the SettingModel and work directly with QSetting
    storeNodes_(settings, rootNode_, false);
    storeMetaData_(settings);
  }
}

void SettingsModel::reloadModel_(QSettings& settings)
{
  beginResetModel();
  delete rootNode_;

  // List headers
  QList<QVariant> rootData;
  rootData << HEADER_NAME << HEADER_VALUE;
  rootNode_ = new TreeNode(noIcon_, rootData);
  initModelData_(settings, rootNode_, rootNode_->path(), false);
  endResetModel(); // calls reset() automatically
}

QModelIndex SettingsModel::addKeyToTree_(const QString& key)
{
  if (key.isEmpty())
  {
    // this function shouldn't be called with an empty key, something likely wrong
    // that warrants further investigation.
    assert(0);
    return QModelIndex();
  }
  TreeNode* fromNode = rootNode_;
  // Assertion failure means this was called prior to finish of construction
  assert(fromNode != NULL);

  QStringList directories = key.split('/');
  // Assertion failure means that key.isEmpty(), indicates something deeper wrong
  assert(!directories.isEmpty());
  QModelIndex parentIndex = QModelIndex();

  // Remove the ending 'key' value, leaving only directories in the nodes list
  QString endKeyName = directories.last();
  directories.pop_back();
  bool forceToPrivate = false;
  // Loop through each directory, creating the hierarchy
  Q_FOREACH(const QString& directory, directories)
  {
    if (directory.compare("Private", Qt::CaseInsensitive) == 0)
      forceToPrivate = true;

    TreeNode* child = fromNode->findChild(directory);
    // Create the child if it's not found
    if (child == NULL)
    {
      child = new TreeNode(folderIcon_, directory, fromNode, forceToPrivate);

      // Add the child, alerting Qt appropriately
      int newRow = fromNode->childCount();
      beginInsertRows(parentIndex, newRow, newRow);
      fromNode->appendChild(child);
      endInsertRows();
    }

    // Push the parentIndex down to the child (child becomes new parent)
    parentIndex = createIndex(child->row(), 0, child);
    fromNode = child;
  }

  // Assertion failure means Q_FOREACH has logic failure and ended with a fromNode
  // that isn't really correct
  assert(fromNode != NULL);

  int newRow = fromNode->childCount();
  TreeNode* child = new TreeNode(noIcon_, endKeyName, QVariant(), fromNode, forceToPrivate);
  beginInsertRows(parentIndex, newRow, newRow);
  fromNode->appendChild(child);
  endInsertRows();
  return createIndex(child->row(), 0, child);
}


void SettingsModel::reloadModel()
{
  // After redesign where everything is kept in memory this routine is a no-op.
}

void SettingsModel::initModelData_(QSettings& settings, SettingsModel::TreeNode* parent, const QString& fullPath, bool forceToPrivate)
{
  // Use begin/endGroup to simplify
  settings.beginGroup(parent->path());

  // Loop through all groups under this one (recursive)
  Q_FOREACH(const QString& group, settings.childGroups())
  {
    // If parent is private, all its children are private.  If child private, parent is not necessarily private.
    bool localForcePrivate = forceToPrivate;
    if (group.toCaseFolded() == "private")
      localForcePrivate = true;

    if (group == METADATA_GROUP)
      continue;

    TreeNode* node = new TreeNode(folderIcon_, group, parent, localForcePrivate);
    // keep track of full path name to find in our entries list
    QString newPath = fullPath + group;
    if (!newPath.isEmpty())
      newPath += "/";
    initModelData_(settings, node, newPath, localForcePrivate);
    parent->appendChild(node);
  }

  // Loop through all leaf nodes under this one
  Q_FOREACH(const QString& key, settings.childKeys())
  {
    TreeNode* node = new TreeNode(noIcon_, key, settings.value(key), parent, forceToPrivate);
    parent->appendChild(node);
  }

  // Close out the group
  settings.endGroup();
}

QModelIndex SettingsModel::index(int row, int column, const QModelIndex &parent) const
{
  // Error check validity
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  // Pull out tree node for the child
  TreeNode *parentItem;
  if (!parent.isValid())
    parentItem = rootNode_;
  else
    parentItem = static_cast<TreeNode*>(parent.internalPointer());
  TreeNode *childItem = parentItem->child(row);

  // Create the model index from child's data, along with parent reference
  if (childItem)
    return createIndex(row, column, childItem);
  return QModelIndex();
}

QModelIndex SettingsModel::parent(const QModelIndex &child) const
{
  TreeNode *childItem = treeNode_(child);
  if (!childItem)
    return QModelIndex();

  // Use data inside the TreeNode to form the parent's index
  TreeNode *parentItem = childItem->parent();
  if (parentItem == rootNode_)
    return QModelIndex();
  return createIndex(parentItem->row(), 0, parentItem);
}

int SettingsModel::rowCount(const QModelIndex &parent) const
{
  // Children only exist on the 0th column
  if (parent.column() > 0)
    return 0;

  // Pull out the child count from the TreeNode
  TreeNode *parentItem;
  if (!parent.isValid())
    parentItem = rootNode_;
  else
    parentItem = static_cast<TreeNode*>(parent.internalPointer());
  return parentItem->childCount();
}

int SettingsModel::columnCount(const QModelIndex &parent) const
{
  // Total number of columns for the index
  if (parent.isValid())
    return static_cast<TreeNode*>(parent.internalPointer())->columnCount();
  return rootNode_->columnCount();
}

QVariant SettingsModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  // Fall back to TreeNode's implementation
  TreeNode *item = static_cast<TreeNode*>(index.internalPointer());
  return item->data(role, index.column());
}

QVariant SettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  // Header data is stored in rootNode_
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return rootNode_->data(Qt::DisplayRole, section);
  return QVariant();
}

Qt::ItemFlags SettingsModel::flags(const QModelIndex& index) const
{
  TreeNode *item = treeNode_(index);
  if (!item)
    return Qt::NoItemFlags;
  // Fall back to TreeNode's implementation
  return item->flags(index.column());
}

bool SettingsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole && role != Qt::CheckStateRole)
    return QAbstractItemModel::setData(index, value, role);
  TreeNode *item = treeNode_(index);
  if (!item)
    return false;

  QVariant oldValue = item->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);
  UserEditCommand* editCommand;
  if (role == Qt::CheckStateRole)
    editCommand = item->setValue(oldValue, (value == Qt::Checked));
  else
    editCommand = item->setValue(oldValue, value);
  if (editCommand == NULL)
    return false; // error

  // Put it into the redo stack, then redo it
  redoStack_.push_back(editCommand);
  redo();
  return true;
}

SettingsModel::TreeNode* SettingsModel::treeNode_(const QModelIndex& index) const
{
  if (!index.isValid())
    return NULL;
  // TreeNode is stored in the internalPointer()
  return static_cast<TreeNode*>(index.internalPointer());
}

bool SettingsModel::canUndo() const
{
  return !undoStack_.empty();
}

bool SettingsModel::canRedo() const
{
  return !redoStack_.empty();
}

void SettingsModel::undo()
{
  if (!undoStack_.isEmpty())
  {
    // Remove the command from undo
    UserEditCommand* cmd = undoStack_.back();
    undoStack_.pop_back();

    // Un-execute the command (changes the QSettings)
    cmd->unexecute(this);

    // Put it back into redo
    redoStack_.push_back(cmd);

    // Fix the GUI display
    refreshKey_(cmd->key());
    emit(settingChanged());
  }
}

void SettingsModel::undoAll()
{
  // Loop through undoing all undoable actions
  while (!undoStack_.empty())
    undo();
}

void SettingsModel::redo()
{
  if (!redoStack_.isEmpty())
  {
    // Remove the command from redo
    UserEditCommand* cmd = redoStack_.back();
    redoStack_.pop_back();

    // Execute the command (changes the QSettings)
    cmd->execute(this);

    // Put it back into undo
    undoStack_.push_back(cmd);

    // Fix the GUI display
    refreshKey_(cmd->key());
    emit(settingChanged());
  }
}

void SettingsModel::clearUndoHistory()
{
  qDeleteAll(undoStack_);
  qDeleteAll(redoStack_);
  undoStack_.clear();
  redoStack_.clear();
}

QModelIndex SettingsModel::findKey_(const QString& relativeKey, const QModelIndex& fromParent) const
{
  // Break out the top and bottom tiers of directory structure
  QString dir = relativeKey.section('/', 0, 0);
  QString underDir = relativeKey.section('/', 1);
  if (relativeKey.isEmpty()) // found match, return immediately
    return fromParent;

  // Loop through children looking for next part of the path
  int numChildren = rowCount(fromParent);
  for (int row = 0; row < numChildren; ++row)
  {
    // Create index and pull out its TreeNode
    QModelIndex idx = index(row, 0, fromParent);
    TreeNode* treeNode = treeNode_(idx);
    // If the path() portion matches our portion, continue recursing
    if (treeNode != NULL && treeNode->path() == dir)
      return findKey_(underDir, idx);
  }
  // No match found
  return QModelIndex();
}

void SettingsModel::refreshKey_(const QString& key)
{
  QModelIndex idx = findKey_(key, QModelIndex());
  if (!idx.isValid())
  {
    // This means that the key is not in our internal hierarchy under rootNode_.
    // For example, if this model gets a setValue() after its initial load and
    // the setValue() is for "Item/That/Doesnt/Exist", this is valid -- but it's
    // still an item we don't know about.  So we need to add the item into our
    // list of values.
    idx = addKeyToTree_(key);

    // Assertion failure means addKeyToTree_() failed and failed badly; it should
    // always create a TreeNode that is findable.
    assert(idx == findKey_(key, QModelIndex()));
    if (!idx.isValid())
    {
      SIM_ERROR << "Attempt to add key " << key.toStdString() << " to settings model tree failed." << std::endl;
      return;
    }
  }

  emit(dataChanged(
    index(idx.row(), 0, idx.parent()),
    index(idx.row(), columnCount(idx) - 1, idx.parent())));
}

int SettingsModel::loadSettingsFile(const QString& path)
{
  if (path.isEmpty() || !QFile::exists(path))
    return 1;
  QSettings settings(path, QSettings::IniFormat);
  if (settings.status() != QSettings::NoError)
    return 1;

  // Load the values from the file into the global settings
  QStringList allKeys = settings.allKeys();
  if (allKeys.empty()) // Empty file
    return 1;
  Q_FOREACH(const QString& key, allKeys)
  {
    setValue(key, settings.value(key));
  }
  // need to update settings meta data based on the meta data loaded from the file
  initMetaData_(settings);

  emit settingsFileLoaded(path);

  return 0;
}

int SettingsModel::saveSettingsFileAs(const QString& path)
{
  if (path.isEmpty())
    return 1;

  emit aboutToSaveSettingsFile(path);

  // Do not overwrite default file.
  const QFileInfo defaultFileInfo(fileName());
  const QFileInfo targetFileInfo(path);
  if (defaultFileInfo == targetFileInfo)
    return 1;

  // Create a settings file for output
  QSettings settings(path, QSettings::IniFormat);
  if (!settings.isWritable())
    return 1;
  // Start fresh
  settings.clear();

  storeNodes_(settings, rootNode_, true);
  return 0;
}


void SettingsModel::storeNodes_(QSettings& settings, TreeNode* node, bool force) const
{
  if (node == NULL)
    return;

  QVariant value = node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);
  // Only need to write out leaves
  if (!node->isRootItem() && (value != QVariant()))
  {
    if (force || node->hasValueChanged())
      settings.setValue(node->fullPath(), value);

    // It is a leaf so return
    return;
  }

  int size = node->childCount();
  for (int ii = 0; ii < size; ++ii)
    storeNodes_(settings, node->child(ii), force);
}

void SettingsModel::clear()
{
  emit beginResetModel();
  delete rootNode_;
  QList<QVariant> rootData;
  rootData << HEADER_NAME << HEADER_VALUE;
  rootNode_ = new TreeNode(noIcon_, rootData);
  observers_.clear();
  emit endResetModel();
}

void SettingsModel::resetDefaults()
{
  resetDefaults_(rootNode_);
}

void SettingsModel::resetDefaults(const QString& name)
{
  TreeNode* rootNode = getNode_(name);
  if (rootNode != NULL)
    resetDefaults_(rootNode);
  else
  {
    SIM_ERROR << "rootNode is NULL, cannot reset defaults for a NULL node\n";
    assert(0);
  }
}

void SettingsModel::resetDefaults_(TreeNode* node)
{
  if (node == NULL)
    return;

  int size = node->childCount();

  if ((size == 0) && !node->isRootItem())
  {
    QVariant value = node->metaData().defaultValue();
    QString name = node->path();
    if (node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE) != value)
    {
      node->setDataValue(value);
      fireObservers_(observers_, name, value);
      node->fireSettingChange();
    }
    // It is a leaf so return
    return;
  }

  for (int ii = 0; ii < size; ++ii)
    resetDefaults_(node->child(ii));
}

SettingsModel::TreeNode* SettingsModel::getNode_(const QString& name) const
{
  QModelIndex idx = findKey_(name, QModelIndex());
  if (!idx.isValid())
    return NULL;

  return static_cast<TreeNode*>(idx.internalPointer());
}

void SettingsModel::setValue(const QString& name, const QVariant& value)
{
  setValue(name, value, simQt::SettingsModel::ObserverPtr());
}

void SettingsModel::setValue(const QString& name, const QVariant& value, const MetaData& metaData, ObserverPtr observer)
{
  bool fire = true;

  TreeNode* node = getNode_(name);
  if (node == NULL)
  {
    QModelIndex idx = addKeyToTree_(name);
    node = static_cast<TreeNode*>(idx.internalPointer());
    node->setMetaData(metaData);
    node->setDataValue(value);
    refreshKey_(name);
  }
  else
  {
    // try to update meta data, since this may be replacing unknown values with known values. Refresh the key if meta data changed
    if (node->setMetaData(metaData) == 0)
      refreshKey_(name);

    if (node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE) != value)
    {
      node->setDataValue(value);
      refreshKey_(name);
    }
    else
      fire = false;
  }
  node->addObserver(observer);

  if (fire)
  {
    fireObservers_(observers_, name, value);
    node->fireSettingChange();
  }
}

void SettingsModel::setValue(const QString& name, const QVariant& value, ObserverPtr skipThisObserver)
{
  TreeNode* node = getNode_(name);
  if (node != NULL)
  {
    if (node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE) != value)
    {
      node->setDataValue(value);
      refreshKey_(name);
      fireObservers_(observers_, name, value, skipThisObserver);
      node->fireSettingChange(skipThisObserver);
    }
  }
  else // use a default meta data, set level to UNKNOWN to ensure it will be overridden with valid meta data
    setValue(name, value, MetaData(STRING, QVariant(), "", UNKNOWN), simQt::SettingsModel::ObserverPtr());
}

QVariant SettingsModel::value(const QString& name) const
{
  TreeNode* node = getNode_(name);
  if (node != NULL)
    return node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);

  return QVariant();
}

QVariant SettingsModel::value(const QString& name, const MetaData& metaData, ObserverPtr observer)
{
  TreeNode* node = getNode_(name);
  if (node != NULL)
  {
    node->addObserver(observer);
    if (node->setMetaData(metaData) == 0)
      refreshKey_(name);
    return node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);
  }

  QModelIndex idx = addKeyToTree_(name);
  node = static_cast<TreeNode*>(idx.internalPointer());
  node->setMetaData(metaData);
  node->addObserver(observer);
  node->setDataValue(metaData.defaultValue());
  refreshKey_(name);

  return metaData.defaultValue();
}

QVariant SettingsModel::value(const QString& name, ObserverPtr observer)
{
  TreeNode* node = getNode_(name);
  if (node != NULL)
  {
    addObserver(name, observer);
    return node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE);
  }

  return value(name, MetaData(), observer);
}

bool SettingsModel::contains(const QString& name) const
{
  return getNode_(name) != NULL;
}

QStringList SettingsModel::allNames() const
{
  QStringList all;
  allNames_(rootNode_, all);
  return all;
}

void SettingsModel::allNames_(TreeNode* node, QStringList& all) const
{
  if (node == NULL)
    return;

  int size = node->childCount();
  // Only need to save leaves
  if (size == 0)
  {
    if (!node->isRootItem())
      all.push_back(node->fullPath());
    return;
  }

  for (int ii = 0; ii < size; ++ii)
  {
    allNames_(node->child(ii), all);
  }
}

int SettingsModel::setMetaData(const QString& name, const SettingsModel::MetaData& metaData)
{
  TreeNode* node = getNode_(name);
  if (node == NULL)
    return 1;  // Did not find it so error out

  node->setMetaData(metaData);
  return 0;
}

int SettingsModel::metaData(const QString& name, MetaData& metaData) const
{
  TreeNode* node = getNode_(name);
  if (node == NULL)
    return 1;  // Did not find it so error out

  // Does it begin with our special _MetaData tag?  If so, return PRIVATE metadata
  if (name.startsWith(METADATA_GROUP))
  {
    metaData = METADATA_METADATA;
    return 0;
  }

  metaData = node->metaData();
  return 0;
}

int SettingsModel::addObserver(const QString& name, ObserverPtr observer)
{
  TreeNode* node = getNode_(name);
  if (node == NULL)
    return 1;  // Did not find it so error out

  if (observer == NULL)
    return 0;

  node->addObserver(observer);
  return 0;
}

int SettingsModel::removeObserver(const QString& name, ObserverPtr observer)
{
  TreeNode* node = getNode_(name);
  if (node == NULL)
    return 1;  // Did not find it so error out

  if (observer == NULL)
    return 0;

  return node->removeObserver(observer);
}

void SettingsModel::addObserver(ObserverPtr observer)
{
  // Don't add NULL pointers
  if (observer == NULL)
    return;

  // Check for existence of the observer first
  if (observers_.contains(observer))
    return;

  observers_.push_back(observer);
}

int SettingsModel::removeObserver(ObserverPtr observer)
{
  for (QList<ObserverPtr>::iterator it = observers_.begin(); it != observers_.end(); ++it)
  {
    if (*it == observer)
    {
      observers_.erase(it);
      return 0;
    }
  }

  return 1;
}

void SettingsModel::fireObservers_(const QList<ObserverPtr>& observers, const QString& name, const QVariant& value, ObserverPtr skipThisObserver)
{
  Q_FOREACH(const SettingsModel::ObserverPtr& ob, observers)
  {
    if (ob != skipThisObserver)
      ob->onSettingChange(name, value);
  }
}


void SettingsModel::saveWidget(QWidget* widget)
{
  WidgetSettings::saveWidget(*this, widget);
}

void SettingsModel::loadWidget(QWidget* widget)
{
  WidgetSettings::loadWidget(*this, widget);
}

void SettingsModel::initMetaData_(QSettings& settings)
{
  // We actually store the meta data in Settings!
  settings.beginGroup(METADATA_GROUP);
  Q_FOREACH(const QString& key, settings.allKeys())
  {
    QVariant qvMetaData = settings.value(key, QVariant());
    // Note that in some rare cases, metadata can be lost, so cannot assert on canConvert()
    if (qvMetaData.canConvert<MetaData>())
    {
      TreeNode* node = getNode_(key);
      // set the meta data, but do not override
      if (node != NULL)
        node->setMetaData(qvMetaData.value<MetaData>(), false);
    }
  }
  settings.endGroup();
}

void SettingsModel::storeMetaData_(QSettings& settings)
{
  // Save the meta data in persistent storage
  settings.beginGroup(METADATA_GROUP);
  storeMetaData_(settings, rootNode_);
  settings.endGroup();
}

void SettingsModel::storeMetaData_(QSettings& settings, TreeNode* node)
{
  if (node == NULL)
    return;

  int size = node->childCount();
  if ((size == 0) && !node->isRootItem())
  {
    const bool hasValue = node->data(Qt::DisplayRole, TreeNode::COLUMN_VALUE).isValid();

    // Note: if the default role for data was private, then we could further optimize this by doing something like:
    // const bool relevant = (node->data(DataLevelRole, TreeNode::COLUMN_NAME).toInt() != simQt::Settings::PRIVATE);
    // ... then only write out the data if relevant is true.

    if (hasValue && node->hasMetaDataChanged())
    {
      settings.setValue(node->fullPath(), QVariant::fromValue(node->metaData()));
    }

    // It is a leaf so return
    return;
  }

  for (int ii = 0; ii < size; ++ii)
    storeMetaData_(settings, node->child(ii));
}

QString SettingsModel::fileName() const
{
  return filename_;
}

Settings::Memento* SettingsModel::createMemento() const
{
  return new MementoImpl(rootNode_);
}

}
