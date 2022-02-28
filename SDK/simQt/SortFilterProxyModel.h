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
#ifndef SIMQT_SORTFILTERPROXYMODEL_H
#define SIMQT_SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include "simCore/Common/Export.h"

namespace simQt
{

/**
 * Defines a generic sort filter proxy model that sorts based on a primary
 * column and always sorts on a secondary column instead of falling back to
 * what the end user chose as a secondary column.  A tertiary column can be
 * set as well.
 */
class SDKQT_EXPORT SortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:

  /** Constant for specifying not to use a secondary or tertiary sort column */
  static const int SORT_COLUMN_NOT_USED = -1;

  /**
   * Constructs a sort filter proxy model for the given Qt object.  If
   * secondary or tertiary sort columns are specified, then those columns will
   * be used for sorting in the case that the primary column being sorted has
   * values that are equal to each other.  By default secondary and tertiary
   * sort columns are set to the constant, SORT_COLUMN_NOT_USED, and you can
   * expect the default behavior from QSortFilterProxyModel.  If the constant,
   * SORT_COLUMN_NOT_USED, is used for the secondary sort column parameter,
   * but not for the tertiary sort column, then the tertiary sort column
   * effectively becomes a secondary sort column.
   */
  explicit SortFilterProxyModel(QObject* parent = nullptr,
      int secondarySortColumn = SORT_COLUMN_NOT_USED,
      int tertiarySortColumn = SORT_COLUMN_NOT_USED);

  /** Virtual destructor */
  virtual ~SortFilterProxyModel();

  /** Sets the secondary sort column. */
  void setSecondarySortColumn(int secondarySortColumn);
  /** Returns the secondary sort column. */
  int secondarySortColumn() const;
  /** Sets the tertiary sort column. */
  void setTertiarySortColumn(int tertiarySortColumn);
  /** Returns the tertiary sort column. */
  int tertiarySortColumn() const;

protected:
  /** Override QSortFilterProxyModel methods */
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

private:
  /** Index of the secondary sort column */
  int secondarySortColumn_;
  /** Index of the tertiary sort column */
  int tertiarySortColumn_;
};

} // simQt

#endif /* SIMQT_SORTFILTERPROXYMODEL_H */
