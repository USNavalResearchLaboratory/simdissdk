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
#ifndef SIMQT_CATEGORY_TREE_MODEL_H
#define SIMQT_CATEGORY_TREE_MODEL_H

#include <QTreeWidgetItem>
#include <QSortFilterProxyModel>

#include "simCore/Common/Common.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/CategoryData/CategoryNameManager.h"

namespace simData { class DataStore; }

namespace simQt {

/// represent either one category name or on category value
class SDKQT_EXPORT CategoryTreeItem
{
public:
  /// constructor
  CategoryTreeItem(const QString& text, const QString& sortText, int categoryIndex, CategoryTreeItem *parent = NULL);
  virtual ~CategoryTreeItem();

  /// Returns the category name or category value
  QString text() const;
  /// For sorting force "Unlisted Value" and "No Value" to the top
  QString sortText() const;
  /// Returns the index for the category name or category value
  int categoryIndex() const;
  /// Returns the check box state
  Qt::CheckState state() const;
  /// Sets the check box state
  void setState(Qt::CheckState value);

  /**@name Tree management routines
   *@{
   */
  void appendChild(CategoryTreeItem *item);
  void removeChild(CategoryTreeItem *item);
  CategoryTreeItem *child(int row);
  int childCount() const;
  CategoryTreeItem *parent();
  int columnCount() const;
  int row() const;
  ///@}

protected:
  /// Update the numCheckedChildren_ value based on the provided value, which will increment or decrement the total
  void updateNumCheckedChildren_(Qt::CheckState value);

  QString text_;  ///< the category name or category value
  QString sortText_; ///< If category value is "Unlisted Value" or "No Value" change to " "  and "  " to force them to the top of the sort
  int categoryIndex_; ///< the index for the category name or category value
  CategoryTreeItem *parentItem_;  ///< parent of the item.  Null if top item
  Qt::CheckState state_; ///< the check box state
  int numCheckedChildren_; ///< keep track of the number of checked children, to manage current check state
  QList<CategoryTreeItem*> childItems_;  ///< Children of item, if any.  If no children, than item is a category value
};

/// Used to sort and filter the CategoryTreeModel
class SDKQT_EXPORT CategoryProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  /// constructor passes parent to QSortFilterProxyModel
  CategoryProxyModel(QObject *parent = 0);
  virtual ~CategoryProxyModel();

public slots:
  /// string to filter against
  void setFilterText(const QString& filter);
  /// Rests the filter by calling invalidateFilter
  void resetFilter();

protected:
  /// filtering function
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
  QString filter_; ///< string to filter against
};


/// model (data representation) for a tree of Entities (Platforms, Beams, Gates, etc.)
class SDKQT_EXPORT CategoryTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  /// constructor
  CategoryTreeModel(QObject *parent = NULL, simData::DataStore* dataStore = NULL, simData::CategoryFilter* categoryFilter = NULL);
  virtual ~CategoryTreeModel();

  /**
   * Set providers
   * @param dataStore The datastore
   * @param categoryFilter The category filter
   **/
  void setProviders(simData::DataStore* dataStore, simData::CategoryFilter* categoryFilter);
  /**
   * Returns the text for the given index
   * @param index The index
   * @return The text for the given index, will be "" if index is invalid
   **/
  QString text(const QModelIndex &index) const;

  /** @copydoc QAbstractItemModel::columnCount() */
  virtual int columnCount(const QModelIndex &parent) const;
  /** @copydoc QAbstractItemModel::data() */
  virtual QVariant data(const QModelIndex &index, int role) const;
  /** @copydoc QAbstractItemModel::setData() */
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
  /** @copydoc QAbstractItemModel::flags() */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  /** @copydoc QAbstractItemModel::index() */
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  /** @copydoc QAbstractItemModel::parent() */
  virtual QModelIndex parent(const QModelIndex &index) const;
  /** @copydoc QAbstractItemModel::rowCount() */
  virtual int rowCount(const QModelIndex &parent) const;

  /// data role for obtaining names that are remapped to force "Unlisted Value" and "No Value" to the top
  static const int SortRole = Qt::UserRole + 1;

public slots:
  /** Updates the contents of the frame */
  virtual void forceRefresh();
  /// Update the filter to the passed in filter
  void setFilter(const simData::CategoryFilter& categoryFilter);

signals:
  /** Emitted whenever the underlying category filter changes */
  void categoryFilterChanged(const simData::CategoryFilter& categoryFilter);

private:
  class CategoryFilterListener;

  /** Helper function to create and add all the category value QTreeWidgetItems to the parent QTreeWidgetItem */
  void addAllValues_(const simData::CategoryFilter::ValuesCheck& iter, CategoryTreeItem* nameItem, const simData::CategoryNameManager& categoryNameManager);
  /** Sets the stat for the given item and all its children */
  int setState_(CategoryTreeItem* item, Qt::CheckState state);
  /** Updates user interface tree based on passed in value */
  void addCategoryName_(int nameIndex);
  /** Updates user interface tree based on passed in value; if rebuild is true then rebuild the category filter */
  void addCategoryValue_(int nameIndex, int valueIndex, bool rebuild);
  /** Clear all data */
  void clear_();
  /** Returns the item for the given index; may return NULL */
  CategoryTreeItem* findCategoryName_(int nameIndex);
  /** Returns the item for the given parent index; may return NULL */
  CategoryTreeItem* findCategoryValue_(CategoryTreeItem* parent, int valueIndex);
  /** Setup the tree */
  void buildTree_();
  /** Convert value into a sortable name */
  QString valueSortName_(const QString& value) const;

  CategoryTreeItem *rootItem_;
  CategoryTreeItem *allCategoriesItem_;
  simData::DataStore* dataStore_;
  simData::CategoryFilter* categoryFilter_;
  simData::CategoryNameManager::ListenerPtr listenerPtr_;
};

}

#endif


