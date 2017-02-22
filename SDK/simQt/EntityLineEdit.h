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
#ifndef SIMQT_ENTITY_LINE_EDIT_H
#define SIMQT_ENTITY_LINE_EDIT_H

#include <QDialog>
#include <QWidget>
#include <QCompleter>
#include <QAbstractItemView>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simQt/Settings.h"

class QCloseEvent;
class QModelIndex;
class Ui_EntityLineEdit;

namespace simQt {

class EntityTreeModel;
class EntityTreeComposite;
class EntityProxyModel;

/** A dialog for displaying the EntityTreeComposite that is configured for single select */
class SDKQT_EXPORT EntityDialog : public QDialog
{
  Q_OBJECT;
public:
  /** Constructor */
  EntityDialog(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::DataStore::ObjectType type);
  virtual ~EntityDialog();

  /** Set the entity via Unique ID */
  void setItemSelected(uint64_t id);

protected:
  /** Override the QDialog close event to emit the closedGui signal */
  virtual void closeEvent(QCloseEvent* ev);

signals:
  /** Signal emitted when the user selects an entity */
  void itemSelected(uint64_t id);
  /** Signal emitted when this dialog is closed */
  void closedGui();

private slots:
  /** Gets the selection from EntityTreeComposite; EntityTreeComposite is in single select mode so QList will only have one entry*/
  void setSelected_(QList<uint64_t> ids);

private:
  simQt::EntityTreeModel* entityTreeModel_;
  simQt::EntityTreeComposite* tree_;  ///< It may be a EntityTreeComposite, but will be hard-coded into List view
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
  EntityLineEdit(QWidget* parent, simQt::EntityTreeModel* entityTreeModel = NULL, simData::DataStore::ObjectType type = simData::DataStore::ALL);
  virtual ~EntityLineEdit();

  /** Returns the Unique ID of the currently selected Entity; returns zero if none */
  uint64_t selected() const;
  /** Returns the name of the currently selected Entity; returns "" if none */
  QString selectedName() const;
  /** The model that holds all the entity information filtered by type */
  void setModel(simQt::EntityTreeModel* model, simData::DataStore::ObjectType type = simData::DataStore::ALL);

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

public slots:
  /** Sets the Unique ID for the entity to display in the QEditLine */
  int setSelected(uint64_t id);
  /** Closes the entity dialog */
  void closeEntityDialog();

signals:
  /** Signal emitted when the user selects an entity */
  void itemSelected(uint64_t id);

protected:
  /** Re-implement eventFilter() to allow a double click to display the dialog */
  virtual bool eventFilter(QObject* obj, QEvent* evt);

private slots:
  /** Called when the user clicks the button for showing the Entity Tree Composite dialog */
  void showEntityDialog_();
  /** Called when the user selects an options from the QCompleter popup menu */
  void wasActived_(const QModelIndex & index);
  /** Called with the user finished editing the name and the name can be verified */
  void editingFinished_();
  /** Called with the user edits the name */
  void textEdited_(const QString & text);

private:
  class DataStoreListener;

  Ui_EntityLineEdit* composite_; ///< The actual user interface
  simQt::EntityTreeModel* entityTreeModel_;  ///< Will not be set in QtDesigner so always NULL check
  simData::DataStore::ListenerPtr dataListenerPtr_; ///< Will not be set in QtDesigner so always NULL check
  EntityDialog* entityDialog_;  ///< The Entity Tree Composite Dialog
  uint64_t uniqueId_;  ///< The Unique ID of the entity, can be zero if current name is not valid
  bool needToVerify_; ///< True means the user typed in a name so it must be verified
  simData::DataStore::ObjectType type_; ///< Limits the entity types to display
  simQt::EntityProxyModel* proxy_;  ///< Allow filtering by entity type
};

}

#endif
