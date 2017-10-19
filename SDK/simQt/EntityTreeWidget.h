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
#ifndef SIMQT_ENTITYTREEWIDGET_H
#define SIMQT_ENTITYTREEWIDGET_H

#include <QList>
#include <QSet>
#include <QTreeWidgetItem>
#include <QAbstractItemView>
#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"
#include "simQt/Settings.h"

class QTreeWidget;
class QTreeWidgetItem;
class QSortFilterProxyModel;

namespace simQt {

class AbstractEntityTreeModel;
class EntityProxyModel;
class EntityFilter;

/** Wraps a QTreeView to provide entity list functionality */
class SDKQT_EXPORT EntityTreeWidget : public QObject
{
  Q_OBJECT;
public:
  /** Constructor needs the view to wrap */
  EntityTreeWidget(QTreeView* view);
  virtual ~EntityTreeWidget();

  /** Adds an entity filter to the proxy model.  NOTE: the proxy model takes ownership of the memory */
  void addEntityFilter(EntityFilter* entityFilter);
  /** Get all the filter widgets from the proxy model; caller responsible for memory */
  QList<QWidget*> filterWidgets(QWidget* newWidgetParent) const;
  /** Sets the tree model to view */
  void setModel(AbstractEntityTreeModel* model);

  /** Sets/clears the selected ID in the entity list; does NOT generate a itemsSelected signal */
  void setSelected(uint64_t id, bool selected, bool signalItemsSelected=false);
  /** Sets/clears selection for the IDs in 'list' */
  void setSelected(QList<uint64_t> list, bool selected);
  /** Clears all selections; does NOT generate a itemsSelected signal */
  void clearSelection();
  /** Gets a list of all the selected IDs in the entity list */
  QList<uint64_t> selectedItems() const;

  /** Pass in global settings reference */
  void setSettings(SettingsPtr settings);
  /** Initialize all settings for this widget; The method is static because it is called in the extension start-up before the dialog is created */
  static void initializeSettings(SettingsPtr settings);

  /** Returns true if the widget is in tree view mode */
  bool isTreeView() const;

  /** Retrieves the widget's selection mode */
  QAbstractItemView::SelectionMode selectionMode() const;
  /** Change the widget selection mode */
  void setSelectionMode(QAbstractItemView::SelectionMode mode);

  /** Return the tree view to allow for customization */
  QTreeView* view() const { return view_; }

  /** Returns the ID that always pass;  zero means no ID always pass*/
  simData::ObjectId alwaysShow() const;
  /** The given ID will always pass all filters; zero means no ID always pass */
  void setAlwaysShow(simData::ObjectId id);

  /** Get the settings for all the filters */
  void getFilterSettings(QMap<QString, QVariant>& settings) const;

public slots:
  /** Swaps the view to the hierarchy tree */
  void setToTreeView();
  /** Swaps the view to a non-hierarchical list */
  void setToListView();
  /** Swaps between tree and list view based on a Boolean */
  void toggleTreeView(bool useTree);
  /** Updates the contents of the frame */
  void forceRefresh();
  /** Scrolls the list so that the item is visible */
  void scrollTo(uint64_t id, QAbstractItemView::ScrollHint hint=QAbstractItemView::EnsureVisible);
  /** Set filters to the given settings */
  void setFilterSettings(const QMap<QString, QVariant>& settings);

signals:
  /** Gives an unsorted list of currently selected entities */
  void itemsSelected(QList<uint64_t> ids);
  /** The unique ID of the entity just double clicked */
  void itemDoubleClicked(uint64_t id);
  /** Sends out update that number of filtered items has changed, with the new number of filtered items and the total number of items */
  void numFilteredItemsChanged(int numFilteredItems, int numTotalItems);
  /** A filter setting was changed */
  void filterSettingsChanged(const QMap<QString, QVariant>& settings);

private slots:
  /** Intercepts the clicked message from the tree, so we can send the signal for itemsSelected() */
  void selectionChanged_(const QItemSelection& s1, const QItemSelection& s2);
  /** When selection has been cleared, sends out an empty itemsSelected signal */
  void selectionCleared_();
  /** Intercepts the double clicked message from the tree, so we can return the entity ID instead of the index */
  void doubleClicked_(const QModelIndex& index);
  /** Send out a signal indicating the number of filtered items has changed */
  void sendNumFilteredItems_();
  /** Start a delay when a row count changes to compress many row count changes into one emit of sendNumFilteredItems_ */
  void delaySend_();
  /** Finish the delay an emit sendNumFilteredItems_ */
  void emitSend_();

protected:
  QTreeView* view_; ///< wrapped view
  AbstractEntityTreeModel* model_; ///< original data model
  EntityProxyModel* proxyModel_; ///< proxy model stands between view and 'model_'

private:
  class EntitySettingsObserver; ///< private class to manage settings change notifications
  SettingsPtr settings_; ///< reference to the global settings object
  Settings::ObserverPtr settingsObserver_; ///< observer to listen to settings changes
  bool treeView_; ///< true if the tree view should show as a tree, false shows as a list
  bool pendingSendNumItems_; ///< true if waiting to emit a sendNumFilteredItems_ signal

  // Maintain a list (to match return value) and a set (for fast searches) of selections
  QList<uint64_t> selectionList_; ///< Cached version of all selected entities
  QSet<uint64_t> selectionSet_; ///< Parallel cache of all selected entities
};

}

#endif /* SIMQT_ENTITYTREEWIDGET_H */

