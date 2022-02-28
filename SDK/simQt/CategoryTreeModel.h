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
#ifndef SIMQT_CATEGORYTREEMODEL_H
#define SIMQT_CATEGORYTREEMODEL_H

#include <map>
#include <memory>
#include <vector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "simCore/Common/Common.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/ObjectId.h"

namespace simData {
  class CategoryFilter;
  class DataStore;
}

namespace simQt {

struct CategoryCountResults;
class Settings;

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
  /** Convenience method to delete each item, then clear(). */
  void deleteAll();

private:
  /** Vector of pointers. */
  typename std::vector<T*> vec_;
  /** Maps pointers to their index in the vector. */
  typename std::map<T*, int> itemToIndex_;
};

/// Used to sort and filter the CategoryTreeModel
class SDKQT_EXPORT CategoryProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  /// constructor passes parent to QSortFilterProxyModel
  explicit CategoryProxyModel(QObject *parent = 0);
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
  /// string to filter against
  QString filter_;
};

/** Single tier tree model that maintains and allows users to edit a simData::CategoryFilter. */
class SDKQT_EXPORT CategoryTreeModel : public QAbstractItemModel
{
  Q_OBJECT;
public:
  explicit CategoryTreeModel(QObject* parent = nullptr);
  virtual ~CategoryTreeModel();

  /** Changes the data store, updating what categories and values are shown. */
  void setDataStore(simData::DataStore* dataStore);
  /** Retrieves the category filter.  Only call this if the Data Store has been set. */
  const simData::CategoryFilter& categoryFilter() const;
  /** Sets the settings and the key prefix for saving and loading the locked states */
  void setSettings(Settings* settings, const QString& settingsKeyPrefix);

  /** Enumeration of user roles supported by data() */
  enum {
    ROLE_SORT_STRING = Qt::UserRole,
    ROLE_EXCLUDE,
    ROLE_CATEGORY_NAME,
    ROLE_REGEXP_STRING,
    ROLE_LOCKED_STATE
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

  /// data role for obtaining names that are remapped to force "Unlisted Value" and "No Value" to the top
  static const int SortRole = Qt::UserRole + 1;

public slots:
  /** Changes the model state to match the values in the filter. */
  void setFilter(const simData::CategoryFilter& filter);
  /** Given results of a category count, updates the text for each category. */
  void processCategoryCounts(const simQt::CategoryCountResults& results);

signals:
  /** The internal filter has changed, possibly from user editing or programmatically. */
  void filterChanged(const simData::CategoryFilter& filter);
  /** The internal filter has changed from user editing. */
  void filterEdited(const simData::CategoryFilter& filter);
  /** Called when the match/exclude button changes.  Only emitted if filterChanged() is not emitted. */
  void excludeEdited(int nameInt = 0, bool excludeMode = true);

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
  /**
  * Update the locked state of the specified category if its name appears in the lockedCategories list.
  * This method should only be called on data that is updating, since it doesn't emit its own signal for a data change
  */
  void updateLockedState_(const QStringList& lockedCategories, CategoryItem& category);
  /** Emits dataChanged() signal for all child entries (non-recursive) */
  void emitChildrenDataChanged_(const QModelIndex& parent);

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

  /** Ptr to settings for storing locked states */
  Settings* settings_;
  /** Key for accessing the setting */
  QString settingsKey_;
};

}

#endif /* SIMQT_CATEGORYTREEMODEL_H */
