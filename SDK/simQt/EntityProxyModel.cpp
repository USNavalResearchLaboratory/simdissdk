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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
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
      model_(nullptr)
  {
  }

  EntityProxyModel::~EntityProxyModel()
  {
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
      delete *it;
    entityFilters_.clear();
  }

  void EntityProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
  {
    if (model_ != nullptr)
    {
      disconnect(model_, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)), this, SLOT(entitiesRemoved_(const QModelIndex&, int, int)));
      disconnect(model_, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(entitiesUpdated_()));
      disconnect(model_, SIGNAL(modelReset()), this, SLOT(entitiesUpdated_()));
    }
    alwaysShow_ = 0;
    // QSortFilterProxyModel::setSourceModel may make calls to EntityProxyModel::data, need to guarantee validity of model_
    model_ = dynamic_cast<AbstractEntityTreeModel*>(sourceModel);
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (model_ != nullptr)
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

    // If there are no pinned or alwaysShow entities, return early
    if ((alwaysShow_ == 0) && pinnedEntities_.isEmpty())
      return rv;

    // Avoid dereferencing model_ before it has been set
    if (model_ == nullptr)
      return rv;

    const QModelIndex sourceIndex = mapToSource(index);
    const simData::ObjectId id = model_->uniqueId(sourceIndex);

    const bool isAlwaysShow = ((alwaysShow_ != 0) && (alwaysShow_ == id));
    const bool isPinned = pinnedEntities_.contains(id);

    // Return unless the entity is pinned or always shown
    if (!isAlwaysShow && !isPinned)
      return rv;

    // Pinned and alwaysShow entities are italicized
    if (role == Qt::FontRole)
    {
      QFont font;
      font.setItalic(true);
      return font;
    }

    if (role == Qt::ToolTipRole)
    {
      if (isPinned)
        return rv.toString().append(tr("\n\nThis entity is pinned and remains visible despite current filter settings."));
      else
        return rv.toString().append(tr("\n\nThis entity was manually selected but does not pass current filter settings."));
    }
    return rv;
  }

  void EntityProxyModel::addEntityFilter(EntityFilter* entityFilter)
  {
#if QT_VERSION >= QT_VERSION_CHECK(6,10,0)
    beginFilterChange();
#endif

    connect(entityFilter, SIGNAL(filterUpdated()), this, SLOT(filterUpdated_()));
    connect(entityFilter, SIGNAL(filterUpdated()), this, SIGNAL(filterChanged()));
    entityFilters_.push_back(entityFilter);
    // Do the initial apply of the filter
    alwaysShow_ = 0;

#if QT_VERSION < QT_VERSION_CHECK(6,10,0)
    invalidateFilter();
#else
    endFilterChange();
#endif
  }

  QList<QWidget*> EntityProxyModel::filterWidgets(QWidget* newWidgetParent) const
  {
    QList<QWidget*> rv;
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
    {
      QWidget* filterWidget = (*it)->widget(newWidgetParent);
      if (filterWidget != nullptr) // only add the widget if not nullptr
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
    if ((alwaysShow_ == id) || (model_ == nullptr))
      return;

    // If item passes the filters, no need to set it to always show
    if (checkFilters_(id))
      alwaysShow_ = 0; // unset previous id
    else
      alwaysShow_ = id;
    invalidate();
  }

  QSet<simData::ObjectId> EntityProxyModel::pinnedEntities() const
  {
    return pinnedEntities_;
  }

  void EntityProxyModel::setPinnedEntities(const QSet<simData::ObjectId>& ids)
  {
    QSet<simData::ObjectId> newPins = ids;
    newPins.remove(0);

    if (newPins == pinnedEntities_)
      return;

    // Determine which pinned entities are being removed
    QSet<simData::ObjectId> removedPins = pinnedEntities_;
    for (const auto id : newPins)
      removedPins.remove(id);

    // Clear the temporary exception if its entity is being unpinned
    if (removedPins.contains(alwaysShow_))
      alwaysShow_ = 0;

    pinnedEntities_ = newPins;
    invalidate();
    Q_EMIT pinnedEntitiesChanged(pinnedEntities_.values());
  }

  bool EntityProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
  {
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    // now get entity id
    AbstractEntityTreeItem *item = static_cast<AbstractEntityTreeItem*>(index0.internalPointer());
    simData::ObjectId id = 0;
    if (item != nullptr)
      id = item->id();
    else
    {
      assert(0);
      return false;
    }

    // Pinned and alwaysShow entities bypass normal filters, pinned entities remain visible when filters change
    if (((alwaysShow_ != 0) && (alwaysShow_ == id)) || pinnedEntities_.contains(id))
      return true;

    // check against all filters
    if (checkFilters_(id))
      return true;

    // didn't pass, check children
    int numChildren = sourceModel()->rowCount(index0);
    for (int i = 0; i < numChildren; ++i)
    {
      if (filterAcceptsRow(i, index0))
        return true;
    }

    return false;
  }

  bool EntityProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
  {
    // Get entity IDs from column 0 regardless of the sort column
    const QModelIndex leftEntityIndex = left.sibling(left.row(), 0);
    const QModelIndex rightEntityIndex = right.sibling(right.row(), 0);

    const simData::ObjectId leftId = model_ ? model_->uniqueId(leftEntityIndex) : 0;
    const simData::ObjectId rightId = model_ ? model_->uniqueId(rightEntityIndex) : 0;

    const bool leftPinned = pinnedEntities_.contains(leftId);
    const bool rightPinned = pinnedEntities_.contains(rightId);

    // Sort pinned entities before unpinned entities
    if (leftPinned != rightPinned)
      return leftPinned;

    if (left.column() == 0)
    {
      const auto& leftString = left.data().toString();
      const auto& rightString = right.data().toString();

      auto leftIt = leftString.begin();
      auto rightIt = rightString.begin();

      while ((leftIt != leftString.end()) && (rightIt != rightString.end()))
      {
        // Keep going as long as the two string match independently of character or digit
        if (*leftIt == *rightIt)
        {
          ++leftIt;
          ++rightIt;
          continue;
        }

        // If the difference is due to digits then get all the following digits to compare the number
        if (leftIt->isDigit() && rightIt->isDigit())
        {
          // Get all the remaining digits in the number for the left side string
          auto leftEnd = leftIt;
          ++leftEnd;
          while (leftEnd != leftString.end())
          {
            if (!leftEnd->isDigit())
              break;
            ++leftEnd;
          }

          // Get all the remaining digits in the number for the right side string
          auto rightEnd = rightIt;
          ++rightEnd;
          while (rightEnd != rightString.end())
          {
            if (!rightEnd->isDigit())
              break;
            ++rightEnd;
          }

          // Compare the number of digits in the strings
          auto leftSize = std::distance(leftIt, leftEnd);
          auto rightSize = std::distance(rightIt, rightEnd);

          // return true if left has fewer digits or false if right has fewer digits
          if (leftSize != rightSize)
            return leftSize < rightSize;

          // same number of digits and the first one must be different, so drop down to the return and just check the first digits
        }

        return *leftIt < *rightIt;
      }

      // If left is at the end and the right is not then it is less
      return ((leftIt == leftString.end()) && (rightIt != rightString.end()));
    }

    // Sorting Original ID as numbers
    if (left.column() == 2)
    {
      uint64_t leftId = sourceModel()->data(left).toULongLong();
      uint64_t rightId = sourceModel()->data(right).toULongLong();
      return leftId < rightId;
    }

    // Sorting based on entity type
    if (left.column() == 1)
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
#if QT_VERSION >= QT_VERSION_CHECK(6,10,0)
    beginFilterChange();
#endif
    // Changing a filter clears the always show entity
    alwaysShow_ = 0;

#if QT_VERSION < QT_VERSION_CHECK(6,10,0)
    invalidateFilter();
#else
    endFilterChange();
#endif

    // apply new filter, invalidate current one
    QMap<QString, QVariant> settings;
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
      (*it)->getFilterSettings(settings);
    Q_EMIT filterSettingsChanged(settings);
  }

  void EntityProxyModel::getFilterSettings(QMap<QString, QVariant>& settings) const
  {
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
      (*it)->getFilterSettings(settings);
  }

  void EntityProxyModel::setFilterSettings(const QMap<QString, QVariant>& settings)
  {
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
      (*it)->setFilterSettings(settings);
  }

  bool EntityProxyModel::checkFilters_(simData::ObjectId id) const
  {
    for (auto it = entityFilters_.begin(); it != entityFilters_.end(); ++it)
    {
      // only need one failure to fail
      if (!(*it)->acceptEntity(id))
        return false;
    }
    return true;
  }

  void EntityProxyModel::entitiesRemoved_(const QModelIndex &parent, int start, int end)
  {
    if ((alwaysShow_ == 0) && pinnedEntities_.isEmpty())
      return;

    bool pinsChanged = false;

    // Clear temporary and pinned entities that are being removed
    for (int ii = start; ii <= end; ++ii)
    {
      const QModelIndex index = model_->index(ii, 0, parent);
      const simData::ObjectId id = model_->uniqueId(index);

      if (id == alwaysShow_)
        alwaysShow_ = 0;

      if (pinnedEntities_.remove(id))
        pinsChanged = true;
    }

    if (pinsChanged)
      Q_EMIT pinnedEntitiesChanged(pinnedEntities_.values());
  }

  void EntityProxyModel::entitiesUpdated_()
  {
    if ((alwaysShow_ == 0) && pinnedEntities_.isEmpty())
      return;

    // Clear if entity does not exist anymore
    if ((alwaysShow_ != 0) && (model_->index(alwaysShow_) == QModelIndex()))
      alwaysShow_ = 0;

    // Remove stale pins
    bool pinsChanged = false;
    for (auto iter = pinnedEntities_.begin(); iter != pinnedEntities_.end(); )
    {
      if (model_->index(*iter) == QModelIndex())
      {
        iter = pinnedEntities_.erase(iter);
        pinsChanged = true;
      }
      else
        ++iter;
    }

    if (pinsChanged)
    {
      invalidate();
      Q_EMIT pinnedEntitiesChanged(pinnedEntities_.values());
    }
  }
}

