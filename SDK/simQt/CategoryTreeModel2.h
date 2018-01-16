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
#ifndef SIMQT_CATEGORYTREEMODEL2_H
#define SIMQT_CATEGORYTREEMODEL2_H

#include <map>
#include <memory>
#include <vector>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include "simCore/Common/Common.h"

class QFont;
namespace simData {
  class CategoryFilter;
  class DataStore;
}

namespace simQt {

/**
* Container class that keeps track of a set of pointers.  The container is indexed to
* provide O(lg n) responses to indexOf() while maintaining O(1) on access-by-index.
* The trade-off is a second internal container that maintains a list of indices.
*
* This is a template container.  Typename T can be any type that can be deleted.
*
* This class is particularly useful for Abstract Item Models that need to know things like
* the indexOf() for a particular entry.
*/
template <typename T>
class SDKQT_EXPORT IndexedPointerContainer
{
public:
  IndexedPointerContainer();
  virtual ~IndexedPointerContainer();

  /** Retrieves the item at the given index.  Not range checked.  O(1). */
  T* operator[](int index) const;
  /** Retrieves the index of the given item.  Returns -1 on not-found.  O(lg n). */
  int indexOf(const T* ptr) const;
  /** Returns the number of items in the container. */
  int size() const;
  /** Adds an item into the container.  Must be a unique item. */
  void push_back(T* item);
  /** Removes all items from container; does not delete memory. */
  void clear();
  /** Convenience method to delete each item, then clear(). */
  void deleteAll();

private:
  /** Vector of pointers. */
  typename std::vector<T*> vec_;
  /** Maps pointers to their index in the vector. */
  typename std::map<T*, int> itemToIndex_;
};

/** Single tier tree model that maintains and allows users to edit a simData::CategoryFilter. */
class SDKQT_EXPORT CategoryTreeModel2 : public QAbstractItemModel
{
  Q_OBJECT;
public:
  explicit CategoryTreeModel2(QObject* parent = NULL);
  virtual ~CategoryTreeModel2();

  /** Changes the data store, updating what categories and values are shown. */
  void setDataStore(simData::DataStore* dataStore);

  /** Enumeration of user roles supported by data() */
  enum {
    ROLE_SORT_STRING = Qt::UserRole,
    ROLE_EXCLUDE
  };

  // QAbstractItemModel overrides
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  virtual QModelIndex parent(const QModelIndex &child) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

public slots:
  /** Changes the model state to match the values in the filter. */
  void setFilter(const simData::CategoryFilter& filter);

signals:
  /** The internal filter has changed, possibly from user editing or programmatically. */
  void filterChanged(const simData::CategoryFilter& filter);
  /** The internal filter has changed from user editing. */
  void filterEdited(const simData::CategoryFilter& filter);

private:
  class CategoryItem;
  class ValueItem;

  /** Adds the category name into the tree structure. */
  void addName_(int nameInt);
  /** Adds the category value into the tree structure under the given name. */
  void addValue_(int nameInt, int valueInt);
  /** Remove all categories and values. */
  void clearTree_();
  /** Retrieve the CategoryItem representing the name provided. */
  CategoryItem* findNameTree_(int nameInt) const;

  /** Quick-search vector of category tree items */
  simQt::IndexedPointerContainer<CategoryItem> categories_;
  /** Maps category int values to CategoryItem pointers */
  std::map<int, CategoryItem*> categoryIntToItem_;

  /** Data store providing the name manager we depend on */
  simData::DataStore* dataStore_;
  /** Internal representation of the GUI settings in the form of a simData::CategoryFilter. */
  simData::CategoryFilter* filter_;

  /** Listens to CategoryNameManager to know when new categories and values are added. */
  class CategoryFilterListener;
  std::shared_ptr<CategoryFilterListener> listener_;

  /** Font used for the Category Name tree items */
  QFont* categoryFont_;
};

/**
 * Item delegate that provides custom styling for a QTreeView with a CategoryTreeModel2.  This
 * delegate is required in order to get "Unlisted Value" editing working properly with
 * CategoryTreeModel2.  The Unlisted Value editing is shown as an EXCLUDE flag on the category
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
  explicit CategoryTreeItemDelegate(QObject* parent = NULL);
  virtual ~CategoryTreeItemDelegate();

  /** Overrides from QStyledItemDelegate */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  virtual bool editorEvent(QEvent* evt, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index);

  /** Overrides from QAbstractItemDelegate */
  virtual bool helpEvent(QHelpEvent* evt, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index);

signals:
  /** User clicked on the custom expand button and index needs to be expanded/collapsed. */
  void expandClicked(const QModelIndex& index);

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
    SE_EXCLUDE_TOGGLE
  };
  /** Contains the rectangles for all sub-elements for an index. */
  struct ChildRects;
  /** Calculate the drawn rectangle areas for each sub-element of a given index. */
  void calculateRects_(const QStyleOptionViewItem& option, const QModelIndex& index, ChildRects& rects) const;
  /** Determine which sub-element, if any, was hit in a mouse click.  See QMouseEvent::pos(). */
  SubElement hit_(const QPoint& pos, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

}

#endif /* SIMQT_CATEGORYTREEMODEL2_H */
