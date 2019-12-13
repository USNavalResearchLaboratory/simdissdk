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
#ifdef USE_DEPRECATED_SIMDISSDK_API
 // this is deprecated; use simQt::CategoryFilterWidget2 found in CategoryTreeModel2.h

#ifndef SIMQT_CATEGORY_FILTER_WIDGET_H
#define SIMQT_CATEGORY_FILTER_WIDGET_H

#include <QWidget>
#include "simCore/Common/Common.h"
#include "simData/CategoryData/CategoryNameManager.h"

class QLineEdit;
class QString;
class QTreeView;
class QTreeWidgetItem;
class QModelIndex;

namespace simData { class CategoryFilter; class DataStore; }

namespace simQt {

class SearchLineEdit;
class CategoryTreeModel;
class CategoryProxyModel;

/**
* Widget for displaying the Category Filter in a tree view with check boxes next to the names and values
* call setProviders with a reference to the DataStore, which allows the widget to query the DataStore for
* all possible categories to fill its tree, as well as to listen to the DataStore updates, which will
* adjust the tree.
*/
class SDKQT_EXPORT CategoryFilterWidget : public QWidget
{
  Q_OBJECT;

public:
  /** Constructor */
  CategoryFilterWidget(QWidget * parent = 0);

  /** Destructor */
  virtual ~CategoryFilterWidget();

  /**
  * Set the reference to the data store, which will update the category tree based on changes to the data store
  * @param dataStore
  */
  void setProviders(simData::DataStore* dataStore);

  /**
  * Returns the current state of all the check boxes in a CategoryFilter object
  * @return CategoryFilter
  */
  const simData::CategoryFilter& categoryFilter() const { return *categoryFilter_; }

public slots:
  /**
  * Updates local categoryFilter and the check boxes on the tree from the categoryFilter supplied
  * @param categoryFilter
  */
  void setFilter(const simData::CategoryFilter& categoryFilter);

signals:
  /** Emitted whenever the underlying category filter changes */
  void categoryFilterChanged(const simData::CategoryFilter& categoryFilter);

private slots:
  /** Collapse the leaves of the tree */
  void collapseLeaves_();
  /** Expand the given index from the model if filtering */
  void expandDueToModel_(const QModelIndex& parentIndex, int to, int from);
  /** Expand the given index from the proxy if filtering */
  void expandDueToProxy_(const QModelIndex& parentIndex, int to, int from);
  /** Text for filtering */
  void newFilterText_(const QString& text);

private:
  /** Helper class for managing all the category information */
  simData::CategoryFilter* categoryFilter_;
  /** The tree */
  QTreeView* treeView_;
  /** Hold the category data */
  CategoryTreeModel* treeModel_;
  /** Provides sorting and filtering */
  CategoryProxyModel* proxy_;
  /** Category search text */
  SearchLineEdit* search_;
  /** The data store */
  simData::DataStore* dataStore_;
  /** Action to collapse the leaves in the tree widget */
  QAction* collapseAction_;
  /** Action to expand all elements of the tree widget */
  QAction* expandAction_;
  /** If true the category values are filtered */
  bool activeFiltering_;
};

}


#endif
#endif
