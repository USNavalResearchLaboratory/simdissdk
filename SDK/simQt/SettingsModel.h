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
#ifndef SIMQT_SETTINGSMODEL_H
#define SIMQT_SETTINGSMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QIcon>
#include <QSet>
#include <QSettings>
#include "simCore/Common/Common.h"
#include "simQt/Settings.h"

namespace simQt {

/**
 * Abstract item model used to display values from a hierarchical QSettings.
 * Inspired by Mark Petryk's http://qtnode.net/wiki/Creating_Models_from_Scratch
 *
 * Setting Model takes the given settings and reads the contents into memory.
 * The expectation is the given settings is the default settings.  Request
 * for data and changes to data are done only with the in memory copy. The
 * destructor writes out the in memory copy back out to the setting.  The in
 * memory copy reduces the conflicts when multiple copies of the same
 * executable are running on the same computer.  A user is not allow to
 * overwrite the currently active settings file.
 */
class SDKQT_EXPORT SettingsModel : public QAbstractItemModel, public Settings
{
  Q_OBJECT;

public:
  /// Qt role for Data Level (DataLevel enum) in data() calls
  static const int DataLevelRole = Qt::UserRole + 1;
  /// Qt role for Fully Qualified Name (e.g. "Units/Precision" instead of just "Precision")
  static const int FullyQualifiedNameRole = Qt::UserRole + 2;
  /// Qt role for Settings::MetaData data
  static const int MetaDataRole = Qt::UserRole + 3;

  /// Instantiates a new settings model from the provided parent
  SettingsModel(QObject* parent, QSettings& settings);
  /// Instantiates new settings model with no dedicated filename; save() is a noop and fileName() is empty.
  explicit SettingsModel(QObject* parent = nullptr);
  virtual ~SettingsModel();

  // from  QAbstractItemModel

  ///@return the index for the given row and column
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  ///@return the index of the parent of the item given by index
  virtual QModelIndex parent(const QModelIndex &child) const;
  ///@return the number of rows in the data
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  ///@return number of columns needed to hold data
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  ///@return data for given item
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  ///@return the header data for given section
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  ///@return the flags on the given item
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  /// set the value of the given item
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  // from Settings

  /// Removes all the entries including metadata and callbacks
  virtual void clear();
  /// Reset factory default value for all settings
  virtual void resetDefaults();
  /// Reset factory default values for settings of a single group and its sub-groups
  virtual void resetDefaults(const QString& name);
  /// Set value; will create if does not exist
  virtual void setValue(const QString& name, const QVariant& value);
  /// Set value; will create if does not exist; updates the metaData and observer, will not add a duplicate observer
  virtual void setValue(const QString& name, const QVariant& value, const MetaData& metaData, ObserverPtr observer=simQt::Settings::ObserverPtr());
  /// Set value; will create if does not exist; updates the metaData; will not call the specified observer to prevent a feedback loop
  virtual void setValue(const QString& name, const QVariant& value, ObserverPtr skipThisObserver);
  /// Returns value for specified name; will return QVariant::Invalid if name does not exist
  virtual QVariant value(const QString& name) const;
  /// Returns value for specified name; will create if it does not exist and return default value in MetaData
  virtual QVariant value(const QString& name, const MetaData& metaData, ObserverPtr observer=simQt::Settings::ObserverPtr());
  /// Returns value for specified name; will create if it does not exist and use default value in MetaData
  virtual QVariant value(const QString& name, ObserverPtr observer);
  /// Returns true if the name exists
  virtual bool contains(const QString& name) const;
  /// Saves widget info, see Settings.h for details
  virtual void saveWidget(QWidget* widget);
  /// Loads widget info, see Settings.h for details
  virtual void loadWidget(QWidget* widget);
  /// Returns all names
  virtual QStringList allNames() const;

  /// Get/Set Metadata
  virtual int setMetaData(const QString& name, const MetaData& metaData);
  virtual int metaData(const QString& name, MetaData& metaData) const;

  /// Local Observers; only when this entry changes do a callback
  virtual int addObserver(const QString& name, ObserverPtr observer);
  virtual int removeObserver(const QString& name, ObserverPtr observer);

  /// Global Observers; if any entry changes do a callback
  virtual void addObserver(ObserverPtr observer);
  virtual int removeObserver(ObserverPtr observer);

  /// Retrieves the path to the underlying storage; on Windows if using NativeFormat, this could be a registry path and not a file path; might be empty
  virtual QString fileName() const;

  /// Creates a memento for saving and restoring state (keys and values only)
  virtual Memento* createMemento() const;

  // Misc.

  ///@return True when there are items to undo
  bool canUndo() const;
  ///@return True when there are items to redo
  bool canRedo() const;

  ///@return True if read-only; read-only settings cannot save(), but can saveSettingsFileAs()
  bool isReadOnly() const;
  /// Change the read-only flag; read-only settings cannot save(), but can saveSettingsFileAs()
  void setReadOnly(bool readOnly);

  /**
  * Set the saveOnlyActivated_ flag, which will filter out any loaded settings that were not activated when saving to a file.
  * This ensures that any loaded settings not applicable to this SettingsModel instance are not written out the next time the settings file is saved.
  * It requires that all settings that will be saved must be activated with a direct call to either setValue() or value(). A call to loadSettingsFile()
  * will not activate settings, nor will changes that happen directly to the underlying QSettings.
  * @param setOnlyActivated true if only activated settings should save to a file, false if all loaded settings should save to a file.
  */
  void setSaveOnlyActivated(bool saveOnlyActivated);

Q_SIGNALS:
  /// Indicates that settings are about to be saved to a file
  void aboutToSaveSettingsFile(const QString& path);
  /// Indicates that a layout setting has been loaded
  void layoutLoaded();
  /// Indicates that a setting has changed
  void settingChanged();

public Q_SLOTS:
  /// Undoes an edit
  void undo();
  /// Redoes an edit
  void redo();
  /// Undo all actions
  void undoAll();
  /// Clear out undo/redo history
  void clearUndoHistory();
  /// Reload the model from current QSettings
  void reloadModel();
  //// Loads a settings file into this data model, returns 0 on success. Will emit the layoutLoaded signal if the specified settings file contains LAYOUT data
  int loadSettingsFile(const QString& path);
  /// Saves the settings to a file. If onlyDeltas is true, saves out only settings whose value differs from default value. Return 0 on success.
  int saveSettingsFileAs(const QString& path, bool onlyDeltas=false);

  /// Saves out data to the default QSettings location; noop if isReadOnly() is true or no filename
  void save();

private:
  /// QSettings is stored in a tree structure
  class TreeNode;
  /// Command pattern for undo/redo
  class UserEditCommand;
  /// Implementation of Settings::Memento
  class MementoImpl;

  /// Returns the node for the given string; returns nullptr if name does not exist
  TreeNode* getNode_(const QString& name) const;
  /// Fires off the given observer list for the given name and value
  void fireObservers_(const QList<ObserverPtr>& observers, const QString& name, const QVariant& value, ObserverPtr skipThisObserver = simQt::Settings::ObserverPtr());
  /// Initializes the underlying tree held in rootNode_ (recursive)
  void initModelData_(QSettings& settings, SettingsModel::TreeNode* parent, const QString& fullPath, bool forceToPrivate);
  /// Retrieves the TreeNode associated with a model index
  TreeNode* treeNode_(const QModelIndex& index) const;
  /// Called when 'key' inside settings changes, triggering a data refresh in the tree
  void refreshKey_(const QString& key);
  /// Locates a QModelIndex, given a relative path and a model index representing that path
  QModelIndex findKey_(const QString& relativeKey, const QModelIndex& fromParent = QModelIndex()) const;
  /// Adds a given key string (separated by slashes) to the root node
  QModelIndex addKeyToTree_(const QString& key);
  /// If an observer is pending for the specified name add it to the specified tree node
  void addPendingObserver_(const QString& name, TreeNode* node);
  /// Reloads model from a specific QSettings
  void reloadModel_(QSettings* settings);
  /// Returns all the leaf nodes names under node
  void allNames_(TreeNode* node, QStringList& all) const;
  /// Stores the changed leaf nodes under node to settings; store all leaf nodes if force is true
  void storeNodes_(QSettings& settings, TreeNode* node, bool force) const;
  /// Stores the leaf nodes under node to settings only if the value differs from the original default value. Recursively calls itself on children of node.
  void storeNodesDeltas_(QSettings& settings, TreeNode* node) const;
  /// Initializes the meta data from persistent storage. Note that meta data will not override
  void initMetaData_(QSettings& settings);
  /// Saves meta data into persistent storage
  void storeMetaData_(QSettings& settings);
  /// Save the meta data of node and its children (recursive)
  void storeMetaData_(QSettings& settings, TreeNode* node);
  /// Resets to default values for node and its children (recursive)
  void resetDefaults_(TreeNode* node);
  /// Common initialization from both constructors
  void init_();

  /// Root item describes the top of the tree
  TreeNode* rootNode_ = nullptr;
  /// Stack of all undoable actions
  QList<UserEditCommand*> undoStack_;
  /// Stack of all re-doable actions
  QList<UserEditCommand*> redoStack_;
  /// Global observers; if any entry changes do a callback
  QList<ObserverPtr> observers_;
  /// Define container for pending observers
  typedef std::multimap<QString, ObserverPtr> PendingMap;
  /// Pending observers for settings not yet initialized
  PendingMap pendingObservers_;
  /// The filename from the setting provided in the constructor
  QString filename_;
  /// The format from the setting provided in the constructor
  QSettings::Format format_ = QSettings::InvalidFormat;
  /// Icon for non-leaf nodes
  QIcon folderIcon_;
  /// Icon for leaf nodes
  QIcon noIcon_;

  /// Indicates that save() should be a noop
  bool readOnly_ = false;
  /// Indicates that only activated settings should be saved
  bool saveOnlyActivated_ = false;
  /// Indicates that settings are being loaded so they will not trigger as activated
  bool loading_ = false;
};

}

#endif /* SIMQT_SETTINGSMODEL_H */
