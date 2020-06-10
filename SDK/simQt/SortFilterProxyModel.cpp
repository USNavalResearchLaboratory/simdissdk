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

#include "simQt/SortFilterProxyModel.h"

namespace simQt
{

SortFilterProxyModel::SortFilterProxyModel(QObject* parent, int secondarySortColumn, int tertiarySortColumn)
  : QSortFilterProxyModel(parent),
    secondarySortColumn_(secondarySortColumn),
    tertiarySortColumn_(tertiarySortColumn)
{
}

SortFilterProxyModel::~SortFilterProxyModel()
{
}

bool SortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  // Use the configured column for secondary sorting
  QVariant leftVariant = sourceModel()->data(left, sortRole());
  QVariant rightVariant = sourceModel()->data(right, sortRole());

  if (leftVariant == rightVariant)
  {
    if (left.column() != secondarySortColumn_ && secondarySortColumn_ != SORT_COLUMN_NOT_USED)
    {
      leftVariant = sourceModel()->data(sourceModel()->index(left.row(), secondarySortColumn_, left.parent()), sortRole());
      rightVariant = sourceModel()->data(sourceModel()->index(right.row(), secondarySortColumn_, right.parent()), sortRole());

      if (leftVariant == rightVariant)
      {
        if (left.column() != tertiarySortColumn_ && tertiarySortColumn_ != SORT_COLUMN_NOT_USED)
        {
          leftVariant = sourceModel()->data(sourceModel()->index(left.row(), tertiarySortColumn_, left.parent()), sortRole());
          rightVariant = sourceModel()->data(sourceModel()->index(right.row(), tertiarySortColumn_, right.parent()), sortRole());
        }
      }

      return leftVariant < rightVariant;
    }

    if (tertiarySortColumn_ != SORT_COLUMN_NOT_USED)
    {
      leftVariant = sourceModel()->data(sourceModel()->index(left.row(), tertiarySortColumn_, left.parent()), sortRole());
      rightVariant = sourceModel()->data(sourceModel()->index(right.row(), tertiarySortColumn_, right.parent()), sortRole());
    }
  }

  return leftVariant < rightVariant;
}

void SortFilterProxyModel::setSecondarySortColumn(int secondarySortColumn)
{
  secondarySortColumn_ = secondarySortColumn;
}

int SortFilterProxyModel::secondarySortColumn() const
{
  return secondarySortColumn_;
}

void SortFilterProxyModel::setTertiarySortColumn(int tertiarySortColumn)
{
  tertiarySortColumn_ = tertiarySortColumn;
}

int SortFilterProxyModel::tertiarySortColumn() const
{
  return tertiarySortColumn_;
}

} // simQt
