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
#ifndef SIMQT_ENTITY_LINE_EDIT_H
#define SIMQT_ENTITY_LINE_EDIT_H

#include <QAbstractItemView>
#include <QCompleter>
#include <QDialog>
#include <QPointer>
#include <QWidget>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simQt/BoundSettings.h"
#include "EntityStateFilter.h"
#include "simQt/Settings.h"

class QCloseEvent;
class QModelIndex;
class Ui_EntityLineEdit;

namespace simCore { class Clock; }

namespace simQt {

class BindCenterEntityToEntityTreeComposite;
class CenterEntity;
class EntityProxyModel;
class EntityTreeComposite;
class EntityTreeModel;

/** A dialog for displaying the EntityTreeComposite that is configured for single select */
class SDKQT_EXPORT EntityDialog : public QDialog
{
  Q_OBJECT;
public:
  /** Constructor */
  EntityDialog(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::ObjectType type, simCore::Clock* clock, SettingsPtr settings);
  virtual ~EntityDialog();

  /** Set the entity via Unique ID */
  void setItemSelected(uint64_t id);

  /** Set the state filter to the given state */
  void setStateFilter(EntityStateFilter::State state);
  /** Returns the current state filter */
  EntityStateFilter::State stateFilter() const;
  /** Set the Center Entity to support the centering feature in the dialog */
  void setCenterEntity(CenterEntity* centerEntity);

protected:
  /** Override the QDialog close event to emit the closedGui signal */
  virtual void closeEvent(QCloseEvent* ev);

Q_SIGNALS:
  /** Signal emitted when the user selects an entity */
  void itemSelected(uint64_t id);
  /** Signal emitted when this dialog is closed */
  void closedGui();

private Q_SLOTS:
  /** Sets the selection from EntityTreeComposite; EntityTreeComposite is in single select mode so QList will only have one entry*/
  void setSelected_(const QList<uint64_t>& ids);
  /** Accept the changes and emit closedGui */
  void accept_();

private:
  EntityTreeModel* entityTreeModel_;
  EntityTreeComposite* tree_;  ///< It may be a EntityTreeComposite, but will be hard-coded into List view
  EntityStateFilter* entityStateFilter_;
  BindCenterEntityToEntityTreeComposite* centerBind_;
};

/** A class for displaying a QLineEdit with a QCompleter for specifying an entity by name */
class SDKQT_EXPORT EntityLineEdit : public QWidget
{
  Q_OBJECT

  /** Sets/gets the tool tip in Qt Designer */
  Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip);
  /** Sets/gets the placeholder text in Qt Designer */
  Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText);
  /** Sets/gets the dialog button in Qt Designer */
  Q_PROPERTY(bool includeDialogButton READ includeDialogButton WRITE setIncludeDialogButton);

public:
  /** Constructor */
  EntityLineEdit(QWidget* parent, simQt::EntityTreeModel* entityTreeModel = nullptr, simData::ObjectType type = simData::ALL);
  virtual ~EntityLineEdit();

  /** Returns the Unique ID of the currently selected Entity; returns zero if none */
  uint64_t selected() const;
  /** Returns the name of the currently selected Entity; returns "" if none */
  QString selectedName() const;
  /** The model that holds all the entity information filtered by type */
  void setModel(simQt::EntityTreeModel* model, simData::ObjectType type = simData::ALL, simCore::Clock* clock = nullptr);

  // Options for customizing the widget

  /** The tool tip for the label before the text field */
  QString tooltip() const;
  /** The tool tip for the label before the text field */
  void setTooltip(const QString& label);

  /** Optionally include the button to display the Entity Tree Composite Dialog. */
  bool includeDialogButton() const;
  /** Optionally include the button to display the Entity Tree Composite Dialog. */
  void setIncludeDialogButton(bool value);

  /** Place holder text in the line edit */
  QString placeholderText() const;
  /** Place holder text in the line edit */
  void setPlaceholderText(const QString& text);

  /** Returns the current state filter */
  EntityStateFilter::State stateFilter() const;

  /** Pass in the global settings reference */
  void setSettings(SettingsPtr settings);

  /** Set the Center Entity to support the centering feature in the dialog */
  void setCenterEntity(CenterEntity* centerEntity);

public Q_SLOTS:
  /** Set the state filter to the given state */
  void setStateFilter(simQt::EntityStateFilter::State state);
  /** Sets the Unique ID for the entity to display in the QEditLine */
  int setSelected(uint64_t id);
  /** Closes the entity dialog */
  void closeEntityDialog();
  /** The given id is in use by something else and should be considered not available, clear by setting to 0 */
  void setUnavailable(uint64_t id);

Q_SIGNALS:
  /** Signal emitted when the user selects an entity */
  void itemSelected(uint64_t id);
  /** Signal emitted when the user presses enter on an existing valid entity */
  void reapplied(uint64_t id);
  /** Signal emitted when state filter changes */
  void stateFilterChanged(simQt::EntityStateFilter::State state);

protected:
  /** Re-implement eventFilter() to allow a double click to display the dialog */
  virtual bool eventFilter(QObject* obj, QEvent* evt);

private Q_SLOTS:
  /** Called when the user clicks the button for showing the Entity Tree Composite dialog */
  void showEntityDialog_();
  /** Called when the user selects an options from the QCompleter popup menu */
  void wasActivated_(const QModelIndex& index);
  /** Called with the user finished editing the name and the name can be verified */
  void editingFinished_();
  /** Called with the user edits the name */
  void textEdited_(const QString& text);
  /** Emit reapply() if the user pressed enter on an existing valid entity */
  void checkForReapply_();

private:
  class DataStoreListener;

  void setTextStyle_(bool valid);

  Ui_EntityLineEdit* composite_; ///< The actual user interface
  simQt::EntityTreeModel* entityTreeModel_;  ///< Will not be set in QtDesigner so always NULL check
  simData::DataStore::ListenerPtr dataListenerPtr_; ///< Will not be set in QtDesigner so always NULL check
  EntityDialog* entityDialog_;  ///< The Entity Tree Composite Dialog
  uint64_t uniqueId_;  ///< The Unique ID of the entity, can be zero if current name is not valid
  uint64_t unavailableId_;  ///< The unique ID of an unavailable entity ID, set to zero to clear
  bool valid_; ///< True if the text field has a valid entity or is empty or is currently under edit
  bool needToVerify_; ///< True means the user typed in a name so it must be verified
  simData::ObjectType type_; ///< Limits the entity types to display
  simQt::EntityProxyModel* proxy_;  ///< Allow filtering by entity type
  simCore::Clock* clock_;  ///< Allow filtering by active/inactive
  simQt::EntityStateFilter* entityStateFilter_; ///< Filtering based on entity state
  EntityStateFilter::State state_; ///< Current state of filtering
  SettingsPtr settings_; ///< Pointer to global settings
  QPointer<CenterEntity> centerEntity_;  ///< Passed to the dialog for the centering feature
};

/** Helper class to bind a EntityLineEdit object to Settings*/
class SDKQT_EXPORT BoundEntityLineEdit : public BoundIntegerSetting
{
  Q_OBJECT

public:
  /** Constructor, will set the value of parent */
  BoundEntityLineEdit(EntityLineEdit* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData);
  virtual ~BoundEntityLineEdit();

  /** Returns standard meta data for entity active/inactive state */
  static simQt::Settings::MetaData metaData();

private Q_SLOTS:
  void setStateFromLineEdit_(simQt::EntityStateFilter::State state);
  void setStateFromSettings_(int state);
};

}

#endif
