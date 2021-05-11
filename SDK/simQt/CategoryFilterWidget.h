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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_CATEGORYFILTERWIDGET_H
#define SIMQT_CATEGORYFILTERWIDGET_H

#include <memory>
#include <QStyledItemDelegate>
#include "simCore/Common/Common.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/ObjectId.h"

class QAction;
class QTreeView;
namespace simData {
  class CategoryFilter;
  class DataStore;
}

namespace simQt {

class AsyncCategoryCounter;
struct CategoryCountResults;
class CategoryProxyModel;
class CategoryTreeModel;
class Settings;

/**
 * Item delegate that provides custom styling for a QTreeView with a CategoryTreeModel.  This
 * delegate is required in order to get "Unlisted Value" editing working properly with
 * CategoryTreeModel.  The Unlisted Value editing is shown as an EXCLUDE flag on the category
 * itself, using a toggle switch to draw the on/off state.  Clicking on the toggle will change
 * the value in the tree model and therefore in the filter.
 *
 * Because the item delegate does not have direct access to the QTreeView on which it is
 * placed, it cannot correctly deal with clicking on expand/collapse icons.  Please listen
 * for the expandClicked() signal when using this class in order to deal with expanding
 * and collapsing trees.
 */
class SDKQT_EXPORT CategoryTreeItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT;
public:
  explicit CategoryTreeItemDelegate(QObject* parent = nullptr);
  virtual ~CategoryTreeItemDelegate();

  /** Overrides from QStyledItemDelegate */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual bool editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Overrides from QAbstractItemDelegate */
  virtual bool helpEvent(QHelpEvent* evt, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index);

signals:
  /** User clicked on the custom expand button and index needs to be expanded/collapsed. */
  void expandClicked(const QModelIndex& index);
  /** User clicked on the custom RegExp edit button and index needs a RegExp assigned. */
  void editRegExpClicked(const QModelIndex& index);

private:
  /** Handles paint() for category name items */
  void paintCategory_(QPainter* painter, QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Handles paint() for value items */
  void paintValue_(QPainter* painter, QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Handles editorEvent() for category name items */
  bool categoryEvent_(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);
  /** Handles editorEvent() for value items. */
  bool valueEvent_(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Sub-elements vary depending on the type of index to draw. */
  enum SubElement
  {
    SE_NONE = 0,
    SE_BACKGROUND,
    SE_CHECKBOX,
    SE_BRANCH,
    SE_TEXT,
    SE_EXCLUDE_TOGGLE,
    SE_REGEXP_BUTTON
  };
  /** Contains the rectangles for all sub-elements for an index. */
  struct ChildRects;
  /** Calculate the drawn rectangle areas for each sub-element of a given index. */
  void calculateRects_(const QStyleOptionViewItem& option, const QModelIndex& index, ChildRects& rects) const;
  /** Determine which sub-element, if any, was hit in a mouse click.  See QMouseEvent::pos(). */
  SubElement hit_(const QPoint& pos, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Keeps track of the QModelIndex being clicked. */
  QModelIndex clickedIndex_;
  /** Sub-element being clicked */
  SubElement clickedElement_;
};

/**
 * Widget that includes a QTreeView with a Category Tree Model and a Search Filter
 * widget that will display a given category filter.  This is an easy-to-use wrapper
 * around the CategoryTreeModel class that provides a view widget and search field.
 */
class SDKQT_EXPORT CategoryFilterWidget : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(bool showEntityCount READ showEntityCount WRITE setShowEntityCount);

public:
  explicit CategoryFilterWidget(QWidget* parent = 0);
  virtual ~CategoryFilterWidget();

  /** Sets the data store, updating the category tree based on changes to that data store. */
  void setDataStore(simData::DataStore* dataStore);
  /** Retrieves the category filter.  Only call this if the Data Store has been set. */
  const simData::CategoryFilter& categoryFilter() const;
  /** Sets the settings and the key prefix for saving and loading the locked states */
  void setSettings(Settings* settings, const QString& settingsKeyPrefix);

  /** Returns true if the entity count should be shown next to values. */
  bool showEntityCount() const;
  /** Changes whether entity count is shown next to category values. */
  void setShowEntityCount(bool show);
  /** Sets a filter on the entity counter, on the entity's object type. Only useful if showEntityCount() is on. */
  void setEntityCountObjectTypes(simData::ObjectType counterObjectTypes);

public slots:
  /** Changes the model state to match the values in the filter. */
  void setFilter(const simData::CategoryFilter& filter);
  /** Updates the (#) count next to category values with the given category value counts. */
  void processCategoryCounts(const simQt::CategoryCountResults& results);

  /**
   * Marks the entity count as dirty; call this when adding or removing entities, or category data changes.
   * If associated with a data store, this is automatically called properly. Only matters if entity count on.
   */
  void setEntityCountDirty();

signals:
  /** The internal filter has changed, possibly from user editing or programmatically. */
  void filterChanged(const simData::CategoryFilter& filter);
  /** The internal filter has changed from user editing. */
  void filterEdited(const simData::CategoryFilter& filter);

private slots:
  /** Shows a GUI for editing the regular expression of a given index */
  void showRegExpEditGui_(const QModelIndex& index);

  /** Expand the given index from the proxy if filtering */
  void expandDueToProxy_(const QModelIndex& parentIndex, int to, int from);
  /** Conditionally expand tree after filter edited. */
  void expandAfterFilterEdited_(const QString& filterText);

  /** Called by delegate to expand an item */
  void toggleExpanded_(const QModelIndex& proxyIndex);
  /** Right click occurred on the QTreeView, point relative to QTreeView */
  void showContextMenu_(const QPoint& point);

  /** Reset the active filter, clearing all values */
  void resetFilter_();
  /** Sets the regular expression on the item saved from showContextMenu_ */
  void setRegularExpression_();
  /** Clears the regular expression on the item saved from showContextMenu_ */
  void clearRegularExpression_();
  /** Locks the current category saved from the showContextMenu_ */
  void toggleLockCategory_();
  /** Expands all unlocked categories */
  void expandUnlockedCategories_();
  /** Start a recount of the category values if countDirty_ is true */
  void recountCategories_();

private:
  class DataStoreListener;

  /** The tree */
  QTreeView* treeView_;
  /** Hold the category data */
  simQt::CategoryTreeModel* treeModel_;
  /** Provides sorting and filtering */
  simQt::CategoryProxyModel* proxy_;
  /** If true the category values are filtered; used to conditionally expand tree. */
  bool activeFiltering_;
  /** If true the category values show a (#) count after them. */
  bool showEntityCount_;
  /** Counter object that provides values for entity counting. */
  AsyncCategoryCounter* counter_;
  /** Records what entity types are used by the Async Category Counter */
  simData::ObjectType counterObjectTypes_;
  /** Action used for setting regular expressions */
  QAction* setRegExpAction_;
  /** Action used for clearing regular expressions */
  QAction* clearRegExpAction_;
  /** Action used for toggling the lock state of a category */
  QAction* toggleLockCategoryAction_;
  /** Listener for datastore entity events */
  std::shared_ptr<DataStoreListener> dsListener_;
  /** If true then the category counts need to be redone */
  bool countDirty_;
};

}

#endif /* SIMQT_CATEGORYFILTERWIDGET_H */
