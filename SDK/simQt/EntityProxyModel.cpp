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
#include "simCore/Common/Common.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "simQt/EntityFilter.h"
#include "simQt/EntityProxyModel.h"

namespace simQt {

  EntityProxyModel::EntityProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
      alwaysShow_(0),
      model_(NULL)
  {
  }

  EntityProxyModel::~EntityProxyModel()
  {
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      delete filter;
    }
    entityFilters_.clear();
  }

  void EntityProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
  {
    if (model_ != NULL)
    {
      disconnect(model_, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(entitiesRemoved_(const QModelIndex&, int, int)));
      disconnect(model_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(entitiesUpdated_()));
      disconnect(model_, SIGNAL(modelReset()), this, SLOT(entitiesUpdated_()));
    }
    alwaysShow_ = 0;
    // QSortFilterProxyModel::setSourceModel may make calls to EntityProxyModel::data, need to guarantee validity of model_
    model_ = dynamic_cast<AbstractEntityTreeModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (model_ != NULL)
    {
      connect(model_, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(entitiesRemoved_(const QModelIndex&, int, int)));
      connect(model_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(entitiesUpdated_()));
      connect(model_, SIGNAL(modelReset()), this, SLOT(entitiesUpdated_()));
    }
  }

  QVariant EntityProxyModel::data(const QModelIndex& index, int role) const
  {
    // Let the model handle the data call as normal
    const QVariant rv = QSortFilterProxyModel::data(index, role);

    const QModelIndex sourceIndex = mapToSource(index);
    const QModelIndex showIndex = model_->index(alwaysShow_);

    // If the index in question is the always shown index,
    // handle the special cases for Qt::FontRole and Qt::ToolTipRole
    if (sourceIndex != showIndex)
      return rv;

    if (role == Qt::FontRole)
    {
      QFont font;
      font.setItalic(true);
      return font;
    }

    if (role == Qt::ToolTipRole)
      return rv.toString().append(tr("\n\nThis entity was manually selected but does not pass current filter settings."));

    return rv;
  }

  void EntityProxyModel::addEntityFilter(EntityFilter* entityFilter)
  {
    connect(entityFilter, SIGNAL(filterUpdated()), this, SLOT(filterUpdated_()));
    connect(entityFilter, SIGNAL(filterUpdated()), this, SIGNAL(filterChanged()));
    entityFilters_.push_back(entityFilter);
    // Do the initial apply of the filter
    alwaysShow_ = 0;
    invalidateFilter();
  }

  QList<QWidget*> EntityProxyModel::filterWidgets(QWidget* newWidgetParent) const
  {
    QList<QWidget*> rv;
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      QWidget* filterWidget = filter->widget(newWidgetParent);
      if (filterWidget != NULL) // only add the widget if not NULL
        rv.push_back(filterWidget);
    }
    return rv;
  }

  simData::ObjectId EntityProxyModel::alwaysShow() const
  {
    return alwaysShow_;
  }

  void EntityProxyModel::setAlwaysShow(simData::ObjectId id)
  {
    if ((alwaysShow_ == id) || (model_ == NULL))
      return;

    // If item passes the filters, no need to set it to always show
    if (checkFilters_(id))
      return;

    alwaysShow_ = id;
    invalidate();
  }

  bool EntityProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
  {
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    // now get entity id
    AbstractEntityTreeItem *item = static_cast<AbstractEntityTreeItem*>(index0.internalPointer());
    simData::ObjectId id = 0;
    if (item != NULL)
      id = item->id();
    else
    {
      assert(0);
      return false;
    }

    if (alwaysShow_ == id)
      return true;

    // check against all filters
    return checkFilters_(id);
  }

  bool EntityProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
  {
    // Sorting Original ID as numbers
    if (left.column() == 2)
    {
      uint64_t leftId = sourceModel()->data(left).toULongLong();
      uint64_t rightId = sourceModel()->data(right).toULongLong();
      return leftId < rightId;
    }
    // Sorting based on entity type
    else if (left.column() == 1)
    {
      int leftSortVal = sourceModel()->data(left, SORT_BY_ENTITY_ROLE).toInt();
      int rightSortVal = sourceModel()->data(right, SORT_BY_ENTITY_ROLE).toInt();
      return leftSortVal < rightSortVal;
    }

    // default sort method used for other columns
    return QSortFilterProxyModel::lessThan(left, right);
  }

  void EntityProxyModel::filterUpdated_()
  {
    // Changing a filter clears the always show entity
    alwaysShow_ = 0;
    // apply new filter, invalidate current one
    invalidateFilter();
    QMap<QString, QVariant> settings;
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      filter->getFilterSettings(settings);
    }
    emit filterSettingsChanged(settings);
  }

  void EntityProxyModel::getFilterSettings(QMap<QString, QVariant>& settings) const
  {
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      filter->getFilterSettings(settings);
    }
  }

  void EntityProxyModel::setFilterSettings(const QMap<QString, QVariant>& settings)
  {
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      filter->setFilterSettings(settings);
    }
  }

  bool EntityProxyModel::checkFilters_(simData::ObjectId id) const
  {
    Q_FOREACH(EntityFilter* filter, entityFilters_)
    {
      // only need one failure to fail
      if (!filter->acceptEntity(id))
        return false;
    }
    return true;
  }

  void EntityProxyModel::entitiesRemoved_(const QModelIndex &parent, int start, int end)
  {
    if (alwaysShow_ == 0)
      return;

    // Clear if the entity is about to be removed
    for (int ii = start; ii < end; ++ii)
    {
      QModelIndex index = model_->index(ii, 0, parent);
      if (model_->uniqueId(index) == alwaysShow_)
      {
        alwaysShow_ = 0;
        break;
      }
    }
  }

  void EntityProxyModel::entitiesUpdated_()
  {
    if (alwaysShow_ == 0)
      return;

    // Clear if entity does not exist anymore
    if (model_->index(alwaysShow_) == QModelIndex())
      alwaysShow_ = 0;
  }
}

