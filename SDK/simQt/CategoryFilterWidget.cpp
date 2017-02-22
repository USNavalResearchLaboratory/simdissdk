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
#include <QLabel>
#include <QLineEdit>
#include <QMetaType>
#include <QTreeWidget>
#include <QLayout>
#include <QAction>
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/CategoryFilter.h"
#include "simQt/SearchLineEdit.h"
#include "simQt/CategoryFilterWidget.h"
#include "simQt/CategoryTreeModel.h"

namespace simQt {

/** Keep the indexes in the tree so it is not necessary to compare strings */
static const int CategoryNameRole = Qt::UserRole + 0;
static const int CategoryValueRole = Qt::UserRole + 1;

CategoryFilterWidget::CategoryFilterWidget(QWidget * parent)
  : QWidget(parent),
    categoryFilter_(NULL),
    dataStore_(NULL),
    collapseAction_(NULL),
    expandAction_(NULL),
    activeFiltering_(false)
{
  setWindowTitle("Category Data Filter:");
  setObjectName("CategoryFilterWidget");

  treeModel_ = new simQt::CategoryTreeModel(this);
  proxy_ = new CategoryProxyModel();
  proxy_->setSortRole(simQt::CategoryTreeModel::SortRole);
  proxy_->setSourceModel(treeModel_);

  treeView_ = new QTreeView();
  treeView_->setObjectName("CategoryFilterTree");
  treeView_->setHeaderHidden(true);
  treeView_->setRootIsDecorated(false);
  treeView_->setSortingEnabled(true);
  treeView_->sortByColumn(0, Qt::AscendingOrder);
  treeView_->setModel(proxy_);

  collapseAction_ = new QAction(tr("Collapse Values"), this);
  connect(collapseAction_, SIGNAL(triggered()), this, SLOT(collapseLeaves_()));
  collapseAction_->setIcon(QIcon(":/simQt/images/Collapse.png"));
  expandAction_ = new QAction(tr("Expand Values"), this);
  connect(expandAction_, SIGNAL(triggered()), treeView_, SLOT(expandAll()));
  expandAction_->setIcon(QIcon(":/simQt/images/Expand.png"));
  treeView_->setContextMenuPolicy(Qt::ActionsContextMenu);
  treeView_->addAction(collapseAction_);
  treeView_->addAction(expandAction_);

  search_ = new SearchLineEdit();
  search_->setPlaceholderText(tr("Search Category Data"));
  QHBoxLayout* searchLayout = new QHBoxLayout();
  searchLayout->addWidget(search_);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setObjectName("CategoryFilterWidgetVBox");
  layout->setMargin(0);
  layout->addItem(searchLayout);
  layout->addWidget(treeView_);
  setLayout(layout);

  connect(treeModel_, SIGNAL(categoryFilterChanged(simData::CategoryFilter)), this, SIGNAL(categoryFilterChanged(simData::CategoryFilter)));
  connect(treeModel_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToModel_(QModelIndex, int, int)));
  connect(proxy_, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(expandDueToProxy_(QModelIndex, int, int)));
  connect(search_, SIGNAL(textChanged(QString)), this, SLOT(newFilterText_(QString)));
  connect(search_, SIGNAL(textChanged(QString)), proxy_, SLOT(setFilterText(QString)));
}

CategoryFilterWidget::~CategoryFilterWidget()
{
  delete treeView_;
  treeView_ = NULL;
  delete collapseAction_;
  collapseAction_ = NULL;
  delete expandAction_;
  expandAction_ = NULL;

  delete proxy_;
  proxy_ = NULL;

  delete treeModel_;
  treeModel_ = NULL;

  delete categoryFilter_;
  categoryFilter_ = NULL;
}

void CategoryFilterWidget::setProviders(simData::DataStore* dataStore)
{
  treeModel_->setProviders(NULL, NULL);
  delete categoryFilter_;
  categoryFilter_ = NULL;
  dataStore_ = dataStore;
  if (dataStore_ == NULL)
    return;

  categoryFilter_ = new simData::CategoryFilter(dataStore_, true);

  treeModel_->setProviders(dataStore_, categoryFilter_);
  treeView_->expandAll();
}

void CategoryFilterWidget::setFilter(const simData::CategoryFilter& categoryFilter)
{
  treeModel_->setFilter(categoryFilter);
}

void CategoryFilterWidget::collapseLeaves_()
{
  treeView_->clearSelection();
  treeView_->collapseAll();
  QModelIndex index = treeModel_->index(0, 0, QModelIndex());
  treeView_->expand(proxy_->mapFromSource(index));
}

void CategoryFilterWidget::newFilterText_(const QString& text)
{
  if (text.isEmpty())
  {
    // Just remove the last character of a search so expand all to make everything visible
    if (activeFiltering_)
      treeView_->expandAll();

    activeFiltering_ = false;
  }
  else
  {
    // Just started a search so expand all to make everything visible
    if (!activeFiltering_)
      treeView_->expandAll();

    activeFiltering_ = true;
  }
}

void CategoryFilterWidget::expandDueToModel_(const QModelIndex& parentIndex, int to, int from)
{
  if (!activeFiltering_)
    return;

  if (!parentIndex.isValid())
    return;

  bool isCategory = !parentIndex.parent().isValid();
  if (isCategory)
    return;

  if (!treeView_->isExpanded(parentIndex))
    proxy_->resetFilter();
}

void CategoryFilterWidget::expandDueToProxy_(const QModelIndex& parentIndex, int to, int from)
{
  if (!parentIndex.isValid())
    return;

  bool isCategory = !parentIndex.parent().isValid();
  if (isCategory)
  {
    // check that category name parent (the 'All Categories' node) is expanded
    QModelIndex index = treeModel_->index(0, 0, QModelIndex());
    if (!treeView_->isExpanded(index))
      treeView_->expand(proxy_->mapFromSource(index));

    // The category names are the "to" to "from" and they just showed up, so expand them
    for (int ii = to; ii <= from; ++ii)
    {
      QModelIndex catIndex = parentIndex.model()->index(ii, 0, parentIndex);
      treeView_->expand(catIndex);
    }
  }
  else
  {
    if (activeFiltering_)
    {
      // Adding a category value; make sure it is visible by expanding its parent
      if (!treeView_->isExpanded(parentIndex))
        treeView_->expand(parentIndex);
    }
  }
}

}

