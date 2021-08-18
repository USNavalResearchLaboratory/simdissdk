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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_ENTITY_PROXY_MODEL_H
#define SIMQT_ENTITY_PROXY_MODEL_H

#include <QDate>
#include <QList>
#include <QSortFilterProxyModel>
#include "simCore/Common/Export.h"
#include "simData/ObjectId.h"
#include "simQt/AbstractEntityTreeModel.h"

namespace simQt {

class EntityFilter;

/// This class does the sorting and filtering of the Entity Tree Model
/// It works between the View and the Model.  Currently only the sorting
/// is different than the standard implementation.
class SDKQT_EXPORT EntityProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  /// constructor passes parent to QSortFilterProxyModel
  EntityProxyModel(QObject *parent = 0);
  virtual ~EntityProxyModel();

  /// QAbstractItemModel interface
  virtual void setSourceModel(QAbstractItemModel *sourceModel);
  virtual QVariant data(const QModelIndex & index, int role) const;

  /** Adds an entity filter to the proxy model.  NOTE: the proxy model takes ownership of the memory */
  void addEntityFilter(EntityFilter* entityFilter);
  /** Get all the filter widgets from the proxy model. NOTE: the caller takes ownership of the memory of all the widgets in the list */
  QList<QWidget*> filterWidgets(QWidget* newWidgetParent) const;
  /** Returns the ID that always pass;  zero means no ID always pass*/
  simData::ObjectId alwaysShow() const;
  /** The given ID will always pass all filters; zero means no ID always pass */
  void setAlwaysShow(simData::ObjectId id);
  /** Get the settings for all the filters */
  void getFilterSettings(QMap<QString, QVariant>& settings) const;
  /** Set filters to the given settings */
  void setFilterSettings(const QMap<QString, QVariant>& settings);

signals:
  /** Emitted when the entity filter has changed */
  void filterChanged();
  /** A filter setting was changed */
  void filterSettingsChanged(const QMap<QString, QVariant>& settings);

protected:
  /// filtering function
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
  /// sorting function
  virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private slots:
  /// Responds to the filters changing
  void filterUpdated_();
  /** Clear the AlwaysShow if parent has a child with the AlwaysShow ID */
  void entitiesRemoved_(const QModelIndex &parent, int start, int end);
  /** Clear the AlwaysShow if the entity went away during a reset or data change */
  void entitiesUpdated_();

private:
  bool checkFilters_(simData::ObjectId id) const;
  QList<EntityFilter*> entityFilters_;
  simData::ObjectId alwaysShow_;
  AbstractEntityTreeModel* model_;
};

}
#endif

