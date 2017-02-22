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
#ifndef SIMQT_VIEWMANAGERDATAMODEL_H
#define SIMQT_VIEWMANAGERDATAMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "osg/observer_ptr"
#include "simCore/Common/Export.h"

namespace simVis
{
  class ViewManager;
  class View;
}

namespace simQt
{

/**
 * Data model that ties in directly with a simQt::ViewManager.  You can use this
 * data model in your own views to display a list of active views.
 */
class SDKQT_EXPORT ViewManagerDataModel : public QAbstractItemModel
{
  Q_OBJECT;
public:
  /** Role to use when requesting the simVis::View* of an index through data(); raw simVis::View* */
  static const int VIEW_ROLE = Qt::UserRole;

  /** Constructor */
  explicit ViewManagerDataModel(QObject* parent=NULL);
  virtual ~ViewManagerDataModel();

  /// Binds to a given view manager
  void bindTo(simVis::ViewManager* viewManager);
  /// Unbinds from the current view manager (if any)
  void unbind();

  ///@return the index for the given row and column
  virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  ///@return the index of the parent of the item given by index
  virtual QModelIndex parent(const QModelIndex &child) const;
  ///@return the number of rows in the data
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  ///@return number of columns needed to hold data
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  ///@return data for given item
  virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  ///@return the header data for given section
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  ///@return the flags on the given item
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  /// set the value of the given item
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  /// Returns true when hierarchical option is on, false when flat
  bool isHierarchical() const;
  /// Returns true if the enable/disable checkboxes are shown, false if hidden
  bool isUserCheckable() const;

public slots:
  /// Changes between a flat display and a hierarchical display
  void setHierarchical(bool useHierarchy);
  /// Changes between mode where checkboxes are shown for enable/disable, and not
  void setUserCheckable(bool isCheckable);

private:
  /// View has been removed from the view manager
  void notifyViewRemoved_(simVis::View* view);
  /// View has been added from the view manager
  void notifyViewAdded_(simVis::View* view);
  /// Notifies us from callback when a view parameter (name, visibility) changes
  void notifyViewParamChange_(simVis::View* view);
  /// Returns a QModelIndex representing the given simVis::View
  QModelIndex createIndex_(simVis::View* view) const;
  /// Returns a simVis::View from the QModelIndex provided
  simVis::View* viewFromIndex_(const QModelIndex& index) const;

  /// Points to the view manager
  osg::observer_ptr<simVis::ViewManager> viewManager_;
  /// Class that implements ViewManager::Callback
  class TopLevelViewChange;
  /// Instance of our callback
  osg::ref_ptr<TopLevelViewChange> viewManagerCB_;

  /// Typedef the observer ptr to a View for easy reuse
  typedef osg::observer_ptr<simVis::View> ViewObserverPtr;
  /// List of all top level views (not maintained directly by ViewManager)
  QList<ViewObserverPtr> topLevelViews_;
  /// List of all non Super HUD views
  QList<ViewObserverPtr> userViews_;

  /// View parameter change observer
  class ViewParameterChange;
  /// Notifies the model when a parameter of interest in the view changes
  osg::ref_ptr<ViewParameterChange> viewParamCB_;

  /// Use hierarchy or not
  bool useHierarchy_;
  /// Use checkboxes for enable/disable or not
  bool isCheckable_;
};

}

#endif /* SIMQT_VIEWMANAGERDATAMODEL_H */
