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
#ifndef SIMQT_ENTITY_TREE_COMPOSITE_H
#define SIMQT_ENTITY_TREE_COMPOSITE_H

#include <QDialog>
#include <QWidget>
#include <QLayout>
#include <QAbstractItemView>
#include "simCore/Common/Common.h"
#include "simData/DataStore.h"
#include "simQt/Settings.h"

class QCloseEvent;
class QModelIndex;
class QToolButton;
class QTreeView;
class Ui_EntityTreeComposite;

namespace simQt {

class EntityFilter;
class EntityNameFilter;
class EntityTreeWidget;
class AbstractEntityTreeModel;

/** Wrapper class for the QDialog, so we can be notified when the dialog has been closed */
class SDKQT_EXPORT FilterDialog : public QDialog
{
  Q_OBJECT;
public:
  /** Constructor */
  FilterDialog(QWidget* parent = NULL);
  virtual ~FilterDialog(){};

  /** Override the QDialog close event to emit the closedGui signal */
  virtual void closeEvent(QCloseEvent*);

signals:
  /** Signal emitted when this dialog is closed */
  void closedGui();
};


/**
 * Composite of entity view, filter, and entity model, provides connectivity between all participants.
 * Buttons can be added to the row with the filter text field to support features like Range Tool with its extra buttons.
 */
class SDKQT_EXPORT EntityTreeComposite : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(bool useEntityIcons READ useEntityIcons WRITE setUseEntityIcons);

public:
  /** Constructor needs the parent widget */
  EntityTreeComposite(QWidget* parent);
  virtual ~EntityTreeComposite();

  /** Adds an entity filter to the entity tree widget's proxy model.  NOTE: the proxy model takes ownership of the memory */
  void addEntityFilter(EntityFilter* entityFilter);
  /** The model that holds all the entity information */
  void setModel(AbstractEntityTreeModel* model);

  /** Sets/clears the selected ID in the entity list */
  void setSelected(uint64_t id, bool selected);
  /** Sets/clears selection of the IDs in 'list' */
  void setSelected(QList<uint64_t> list, bool selected);
  /** Clears all selections */
  void clearSelection();
  /** Gets a list of all the selected IDs in the entity list */
  QList<uint64_t> selectedItems() const;

  /** Adds a button after the filter text field */
  void addButton(QWidget* button);
  /** Pass in the global settings reference */
  void setSettings(SettingsPtr settings);

  /** Initialize all settings for this widget */
  static void initializeSettings(SettingsPtr settings);

  /** Retrieves the widget's selection mode */
  QAbstractItemView::SelectionMode selectionMode() const;
  /** Change the widget selection mode */
  void setSelectionMode(QAbstractItemView::SelectionMode mode);

  /** Return the tree view to allow for customization */
  QTreeView* view() const;

  /** Returns the ID that always pass;  zero means no ID always pass*/
  simData::ObjectId alwaysShow() const;
  /** The given ID will always pass all filters; zero means no ID always pass */
  void setAlwaysShow(simData::ObjectId id);

  /**
   * Get the settings for all the filters
   * @param settings Filters add data to the setting using a global unique key
   */
  void getFilterSettings(QMap<QString, QVariant>& settings) const;

  /** Returns true if icons are shown instead of text for the entity tree list Entity Type column */
  bool useEntityIcons() const;
  /** Shows icons instead of text for the entity tree list Entity Type column */
  void setUseEntityIcons(bool showIcons);
  /** Sets the ability to use the context menu center action, which is disabled by default */
  void setUseCenterAction(bool use);

  /** Class to store information about an Entity Tab Filter Configuration */
  class FilterConfiguration
  {
  public:
    FilterConfiguration();
    virtual ~FilterConfiguration();

    FilterConfiguration(const FilterConfiguration& rhs);
    FilterConfiguration(const QString& description, const QMap<QString, QVariant>& configuration);

    QString description() const;
    void setDescription(const QString& description);

    QMap<QString, QVariant> configuration() const;
    void setConfiguration(const QMap<QString, QVariant>& configuration);

  private:
    QString description_;                   ///< User-supplied description of the configuration
    QMap<QString, QVariant> configuration_; ///< Map of all filter configuration settings
  };

public slots:
  /** If true expand the tree on double click */
  void setExpandsOnDoubleClick(bool value);
  /** Returns true if double clicking on the tree expands the tree */
  bool expandsOnDoubleClick() const;
  /** Scrolls the list so that the item is visible */
  void scrollTo(uint64_t id, QAbstractItemView::ScrollHint hint=QAbstractItemView::EnsureVisible);
  /** Sets the visibility of the button that switches between List view and Tree view */
  void setListTreeButtonDisplayed(bool value);
  /**
   * Set filters to the given settings
   * @param settings Filters get data from the setting using a global unique key
   */
  void setFilterSettings(const QMap<QString, QVariant>& settings);

signals:
  /** Gives an unsorted list of currently selected entities */
  void itemsSelected(QList<uint64_t> ids);
  /** The unique ID of the entity just double clicked */
  void itemDoubleClicked(uint64_t id);
  /** Fired when the Center On Entity context menu action is triggered */
  void centerOnEntityRequested(uint64_t id);
  /**
   * A filter setting was changed
   * @param settings Filters get data from the setting using a global unique key
   */
  void filterSettingsChanged(const QMap<QString, QVariant>& settings);

protected slots:
  /** Receive notice of an inserted row */
  void rowsInserted_(const QModelIndex & parent, int start, int end);
  /** Receive notice to show filters */
  void showFilters_();
  /** Receive notice that filters GUI is closed, to clean up resources*/
  void closeFilters_();

private slots:
  /** Update the label displaying number of items after filter is applied */
  void setNumFilteredItemsLabel_(int numFilteredItems, int numTotalItems);
  /** The user has changed what, if any, entities are selected; use to enable the copy action */
  void onItemsChanged_(const QList<uint64_t>& ids);
  /** Called when the user want to copy the selected entity names to the clipboard */
  void copySelection_();
  /** Called when a user clicks the center action from the context menu */
  void centerOnSelection_();
  /** Toggle the tree/list view and update related UI component and action states */
  void setTreeView_(bool useTreeView);

  /** Loads the filter configuration indicated by the index provided */
  void loadFilterConfig_(int index);
  /** Saves over the filter configuration indicated by the index provided */
  void saveFilterConfig_(int index);
  /** Clears the filter configuration indicated by the index provided */
  void clearFilterConfig_(int index);

private:
  /** Watch for settings changes related to the buttons */
  class Observer;

  /** Update Collapse All and Expand All action enabled states */
  void updateActionEnables_();
  /** Retrieves the QToolButton associated with the filter configuration index */
  QToolButton* configButtonForIndex_(int index) const;
  /** Retrieves the QIcon associated with the filter configuration index */
  QIcon configIconForIndex_(int index) const;

  Ui_EntityTreeComposite* composite_;
  EntityTreeWidget* entityTreeWidget_;
  AbstractEntityTreeModel* model_;
  EntityNameFilter* nameFilter_;
  QDialog* filterDialog_;
  QAction* copyAction_;
  QAction* centerAction_;
  QAction* toggleTreeViewAction_;
  QAction* collapseAllAction_;
  QAction* expandAllAction_;
  bool useCenterAction_;

  SettingsPtr settings_;
  simQt::Settings::ObserverPtr observer_;

  class ButtonActions;
  std::vector<ButtonActions*> buttonActions_;

  /// Whether or not to use the entity icons, vs the names
  bool useEntityIcons_;
  /// If true, a call to setUseEntityIcons() was explicitly made by caller
  bool useEntityIconsSet_;
};

}

/** Declarations to make QVariant and QSettings work */
Q_DECLARE_METATYPE(simQt::EntityTreeComposite::FilterConfiguration);
QDataStream &operator<<(QDataStream& out, const simQt::EntityTreeComposite::FilterConfiguration& myObj);
QDataStream &operator>>(QDataStream& in, simQt::EntityTreeComposite::FilterConfiguration& myObj);



#endif /* SIMQT_ENTITY_TREE_COMPOSITE_H */
