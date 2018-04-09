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
#ifndef SIMQT_ACTIONREGISTRY_H
#define SIMQT_ACTIONREGISTRY_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QKeySequence>
#include "simCore/Common/Common.h"

class QAction;
class QWidget;
class QSettings;
class QTimer;

namespace simQt {

class Action;
class ActionRegistry;

/** Manages adding and updating hot key descriptions in action tool tips */
class ToolTipUpdater : public QObject
{
  Q_OBJECT
public:
  explicit ToolTipUpdater(QObject* parent = NULL);
public slots:
  /** Add an action to the list of actions waiting to have their tool tip updated. */
  void addPending(simQt::Action* action);
  /** Remove an action from the pending list, if it exists in the list. */
  void removeAction(const simQt::Action* action);
private slots:
  /** Update the tool tips on all pending actions. */
  void updateToolTips_();
private:
  QTimer* timer_;
  std::vector<simQt::Action*> pendingActions_;
};

/** Actions can only be instantiated and destroyed by ActionRegistry */
class SDKQT_EXPORT Action
{
  friend class ActionRegistry;
public:
  /** Retrieve a group name for this action */
  QString group() const;
  /** Retrieve a unique description associated with this action */
  QString description() const;

  /** Retrieve the action pointer. */
  QAction* action() const;
  /** Retrieves a pointer to owning registry */
  simQt::ActionRegistry* actionRegistry() const;

  /** Retrieve a list of all key sequences associated with this action */
  QList<QKeySequence> hotkeys() const;

  /** Remove the hot key at the given index */
  int removeHotKey(unsigned int bindingNum);
  /** Sets the primary hot key on the action */
  int setHotKey(QKeySequence hotkey);
  /** Sets multiple hot keys for the action */
  int setHotKeys(const QList<QKeySequence>& hotkeys);

  /// trigger the action's side-effects
  void execute() const;

private:
  Action(ActionRegistry* registry, const QString& group, const QString& description, QAction* action);
  virtual ~Action();

private:
  ActionRegistry* registry_;
  QString group_;
  QString description_;
  QAction* action_;
};

/// Manager for all registered actions
class SDKQT_EXPORT ActionRegistry : public QObject
{
  Q_OBJECT;
public:
  /// constructor
  explicit ActionRegistry(QWidget* mainWindow);
  virtual ~ActionRegistry();

  /** Creates an action based off a QAction and some meta data */
  Action* registerAction(const QString& group, const QString& description, QAction* action);
  /** Add an alias to the given action */
  int registerAlias(const QString& actionDesc, const QString& alias);
  /** Searches for and executes action, returning 0 on success */
  int execute(const QString& actionDesc);

  /**@return the action corresponding to the given description; will search aliases */
  Action* findAction(const QString& desc) const;
  /**@return the action corresponding to the given hot key */
  Action* findAction(QKeySequence hotKey) const;

  /**@return all actions */
  QList<Action*> actions() const;

  /** remove the action corresponding to the given description; will not search aliases */
  int removeAction(const QString& desc);

  /** remove an action no longer needed, returns 0 on success */
  int removeUnknownAction(const QString& desc);

  /** remove from the given action the given key binding, returns 0 on success */
  int removeHotKey(Action* action, unsigned int bindingNum);
  /** bind the given action to the given hot key, returns 0 on success */
  int setHotKey(Action* action, QKeySequence hotkey);
  /** bind the given action to the given list of keys, returns 0 on success */
  int setHotKeys(Action* action, const QList<QKeySequence>& hotkeys);
  /** add a binding to the described action, will search aliases; returns 0 on success */
  int addHotKey(const QString& actionDesc, QKeySequence hotkey);

  /** Memento interface (narrow) for saving and restoring settings opaquely */
  class SettingsMemento
  {
  public:
    virtual ~SettingsMemento() {}
    /** Restore the memento to an action registry */
    virtual int restore(ActionRegistry& registry) const = 0;
  };

  /** Create a memento of all the hotkey sequences stored */
  SettingsMemento* createMemento() const;
  /** Save to a QSettings, returning 0 on success and non-zero on failure. */
  int serialize(QSettings& settings, const QString& groupName="KeyBindings") const;
  /** Saves to a data file, returning 0 on success and non-zero on failure. */
  int serialize(const QString& settingsFile, const QString& groupName="KeyBindings") const;
  /** Restore from a QSettings, returning 0 on success and non-zero on error */
  int deserialize(QSettings& settings, const QString& groupName="KeyBindings");
  /** Loads the file provided, returning 0 on success and non-zero on error */
  int deserialize(const QString& settingsFile, const QString& groupName="KeyBindings");

signals:
  /** notice that a new action has been registered */
  void actionAdded(simQt::Action* action);
  /** notice that an action has been unregistered */
  void actionRemoved(const simQt::Action* action);
  /** notice that the hotkeys for an action have changed */
  void hotKeysChanged(simQt::Action* action);
  /** notice that a hot key has been removed from an action */
  void hotKeyLost(const simQt::Action* fromAction, const QKeySequence& hotkey);

private:
  /** In debug mode, will validate all actions to ensure no sync loss between action registry and action */
  void assertActionsByKeyValid_() const;

  /// Main window pointer, used for making hotkeys global
  QWidget* mainWindow_;
  /// Sorted by description
  QMap<QString, Action*> actionsByDesc_;
  /// Sorted by hotkey
  QMap<QKeySequence, Action*> actionsByKey_;
  /// Sorted by alias
  QMap<QString, QString> aliases_;

  /// Maintains a list of hotkeys associated with a given action, by description
  struct UnknownAction
  {
    QString description;
    QList<QKeySequence> hotkeys;
  };
  /// List of all unknown actions
  QMap<QString, UnknownAction*> unknownActions_;

  /// Search only actionsByDesc_ for an action
  Action* findWithoutAliases_(const QString& desc) const;
  /// Retrieves list of unknown hotkeys and removes it from the unknowns list
  QList<QKeySequence> takeUnknown_(const QString& actionDesc);
  /// Initializes an action with hotkeys after construction
  void combineAndSetKeys_(Action* action, const QList<QKeySequence>& originalKeys, const QList<QKeySequence>& unknownKeys);
  /// Unbinds the hotkey from the action, both updating internal structures and optionally from QAction
  int removeBinding_(Action* fromAction, QKeySequence key, bool updateQAction);
  /// Converts list of hotkeys into a list of unique hotkeys
  QList<QKeySequence> makeUnique_(const QList<QKeySequence>& keys) const;

  /** Private memento implementation */
  class MementoImpl;

  /** Manages updating tool tips when hot keys change */
  ToolTipUpdater* toolTipUpdater_;
};

}

#endif /* SIMQT_ACTIONREGISTRY_H */

