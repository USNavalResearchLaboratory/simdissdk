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
#include <cassert>
#include <vector>
#include "simNotify/Notify.h"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simQt/ViewManagerDataModel.h"

namespace simQt
{

/// Monitor the adding and removing of view
class ViewManagerDataModel::TopLevelViewChange : public simVis::ViewManager::Callback
{
public:
  /** Constructor */
  explicit TopLevelViewChange(ViewManagerDataModel* dataModel)
    : Callback(),
      dataModel_(dataModel)
  {
    // NULL Data model is not supported (doesn't make sense here)
    assert(dataModel_ != NULL);
  }
  virtual void operator()(simVis::View* inset, const EventType& e)
  {
    switch (e)
    {
    case simVis::ViewManager::Callback::VIEW_REMOVED:
      dataModel_->notifyViewRemoved_(inset);
      break;
    case simVis::ViewManager::Callback::VIEW_ADDED:
      dataModel_->notifyViewAdded_(inset);
      break;
    }
  }

protected:
  virtual ~TopLevelViewChange() {}

private:
  ViewManagerDataModel* dataModel_;
};

///////////////////////////////////////////

/// Monitor changes in a view
class ViewManagerDataModel::ViewParameterChange : public simVis::View::Callback
{
public:
  /** Constructor */
  explicit ViewParameterChange(ViewManagerDataModel* dataModel)
    : Callback(),
      dataModel_(dataModel)
  {
    // NULL Data model is not supported (doesn't make sense here)
    assert(dataModel_ != NULL);
  }
  virtual void operator()(simVis::View* view, const EventType& e)
  {
    switch (e)
    {
    case simVis::View::Callback::VIEW_NAME_CHANGE:
    case simVis::View::Callback::VIEW_VISIBILITY_CHANGE:
      dataModel_->notifyViewParamChange_(view);
      break;
    case simVis::View::Callback::VIEW_COCKPIT_CHANGE:
    case simVis::View::Callback::VIEW_ORTHO_CHANGE:
    case simVis::View::Callback::VIEW_EXTENT_CHANGE:
      break;
    }
  }

protected:
  virtual ~ViewParameterChange() {}

private:
  ViewManagerDataModel* dataModel_;
};

///////////////////////////////////////////

ViewManagerDataModel::ViewManagerDataModel(QObject* parent)
  : QAbstractItemModel(parent),
    useHierarchy_(true),
    isCheckable_(true)
{
  viewManagerCB_ = new TopLevelViewChange(this);
  viewParamCB_ = new ViewParameterChange(this);
}

ViewManagerDataModel::~ViewManagerDataModel()
{
  unbind();
}

void ViewManagerDataModel::bindTo(simVis::ViewManager* viewManager)
{
  if (viewManager == NULL)
  {
    unbind();
    return;
  }

  // Start the reset
  beginResetModel();

  // Assign view manager and hook into callbacks
  viewManager_ = viewManager;
  viewManager_->addCallback(viewManagerCB_);

  // Fill out the top level views
  std::vector<simVis::View*> viewsVec;
  viewManager_->getViews(viewsVec);
  topLevelViews_.clear();
  userViews_.clear();
  for (std::vector<simVis::View*>::const_iterator i = viewsVec.begin(); i != viewsVec.end(); ++i)
  {
    if ((*i)->type() == simVis::View::VIEW_TOPLEVEL)
      topLevelViews_.push_back(ViewObserverPtr(*i));
    if ((*i)->type() != simVis::View::VIEW_SUPERHUD)
      userViews_.push_back(ViewObserverPtr(*i));
    (*i)->addCallback(viewParamCB_);
  }

  // Complete the reset
  endResetModel();
}

void ViewManagerDataModel::unbind()
{
  if (!viewManager_.valid())
    return;
  beginResetModel();
  topLevelViews_.clear();
  userViews_.clear();

  viewManager_->removeCallback(viewManagerCB_);
  // If old view manager is non-NULL, remove callback
  viewManager_->removeCallback(viewManagerCB_);
  for (unsigned int k = 0; k < viewManager_->getNumViews(); ++k)
    viewManager_->getView(k)->removeCallback(viewParamCB_);

  viewManager_ = NULL;
  endResetModel();
}

QModelIndex ViewManagerDataModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  // Flat mode relies on userViews (all but Super HUD view port)
  if (!isHierarchical())
  {
    if (parent.isValid())
      return QModelIndex();
    return createIndex(row, column, userViews_.at(row).get());
  }

  // Top level view (typically MainView)
  if (!parent.isValid())
  {
    // Assertion failure means we cannot trust hasIndex() to return correct values
    assert(viewManager_.valid());
    // Return a row/column based on the view manager's child
    return createIndex(row, column, topLevelViews_.at(row).get());
  }

  // Must be an inset view; pull out the view host from "parent"
  const simVis::View* hostView = viewFromIndex_(parent);
  if (hostView == NULL)
  {
    return QModelIndex();
  }
  return createIndex(row, column, hostView->getInset(row));
}

QModelIndex ViewManagerDataModel::parent(const QModelIndex &child) const
{
  // Flat mode children never have parents
  if (!child.isValid() || !viewManager_.valid() || !isHierarchical())
    return QModelIndex();
  const simVis::View* childView = viewFromIndex_(child);
  // If view is NULL then there is no parent
  if (childView == NULL)
    return QModelIndex();
  // Get the first level parent from the child
  simVis::View* parentView = childView->getHostView();
  return createIndex_(parentView);
}

int ViewManagerDataModel::rowCount(const QModelIndex &parent) const
{
  if (!viewManager_.valid())
    return 0;

  // Flat mode only has rows in the invalid parent
  if (!isHierarchical())
    return parent.isValid() ? 0 : static_cast<int>(userViews_.count());

  // Hierarchical needs to test the number of insets
  if (!parent.isValid())
    return topLevelViews_.count();
  const simVis::View* view = viewFromIndex_(parent);
  return view != NULL ? view->getNumInsets() : 0;
}

int ViewManagerDataModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

QVariant ViewManagerDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  simVis::View* view = viewFromIndex_(index);
  switch (role)
  {
  case Qt::DisplayRole:
  case Qt::EditRole:
    if (view != NULL)
    {
      std::string name = view->getName();
      if (name.empty() && role == Qt::DisplayRole)
        return "[empty name]";
      return QString::fromStdString(view->getName());
    }
    break;

  case Qt::CheckStateRole:
    // Only show check state for views that are not top level
    if (view != NULL && view->getHostView() != NULL && isUserCheckable())
      return view->isVisible() ? Qt::Checked : Qt::Unchecked;
    break;

  case VIEW_ROLE:
    return qVariantFromValue(static_cast<void*>(view));
  }
  return QVariant();
}

QVariant ViewManagerDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    return "Name";
  return QVariant();
}

Qt::ItemFlags ViewManagerDataModel::flags(const QModelIndex& index) const
{
  if (index.isValid())
  {
    const simVis::View* view = viewFromIndex_(index);
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    // Only insets can be turned on and off or have its name edited
    if (view != NULL && view->getHostView() != NULL && isUserCheckable())
      flags |= (Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
    return flags;
  }
  return Qt::NoItemFlags;
}

bool ViewManagerDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  simVis::View* view = viewFromIndex_(index);
  if (!index.isValid() || view == NULL)
    return QAbstractItemModel::setData(index, value, role);

  if (role == Qt::EditRole)
  {
    if (view->getHostView() != NULL)
    {
      // Use the trimmed name
      const std::string trimmed = value.toString().trimmed().toStdString();
      if (view->getHostView()->isValidNewInsetName(trimmed, view))
      {
        view->setName(trimmed);
        // Emit is handled by the round-trip callback
        return true;
      }
      SIM_WARN << "The inset name \"" << trimmed << "\" is invalid\n";
    }
    return false;
  }
  else if (role == Qt::CheckStateRole)
  {
    view->setVisible(value.toBool());
    // Emit is handled by the round-trip callback
    return true;
  }
  return QAbstractItemModel::setData(index, value, role);
}

void ViewManagerDataModel::notifyViewRemoved_(simVis::View* view)
{
  // Update the flat mode back-end w/o sending any gui notifications
  if (isHierarchical() && view->type() != simVis::View::VIEW_SUPERHUD)
  {
    int idx = userViews_.indexOf(ViewObserverPtr(view));
    userViews_.removeAt(idx);
  }

  // See if it's in the list of top level views and sync if so
  if (view->getHostView() == NULL && isHierarchical())
  {
    int idx = topLevelViews_.indexOf(ViewObserverPtr(view));
    // Assertion failure means we are out of sync in the list
    assert(idx >= 0);
    beginRemoveRows(QModelIndex(), idx, idx);
    topLevelViews_.removeAt(idx);
    endRemoveRows();
    return;
  }

  // Non-hierarchy optimization: simply remove an item, then emit changed
  if (!isHierarchical())
  {
    int idx = userViews_.indexOf(ViewObserverPtr(view));
    beginRemoveRows(QModelIndex(), idx, idx);
    userViews_.removeAt(idx);
    endRemoveRows();
    return;
  }

  // When an inset is removed, we won't know its index, so we can just try to reset the
  // model.  We might be able to instead remove/insert sequentially, but given the expected
  // number of views, it should be safe to just reset.
  beginResetModel();
  endResetModel();
}

QModelIndex ViewManagerDataModel::createIndex_(simVis::View* view) const
{
  if (view == NULL)
    return QModelIndex();
  // In flat mode we query the view manager directly
  if (!isHierarchical())
    return createIndex(viewManager_->getIndexOf(view), 0, view);

  simVis::View* parent = view->getHostView();

  // Case 1 is that the parent is NULL (e.g. view is top level)
  int idx = -1;
  if (parent == NULL)
    idx = topLevelViews_.indexOf(ViewObserverPtr(view));
  else
    idx = parent->getIndexOfInset(view);
  // Assertion failure indicates our topLevelWidgets_ out of sync or bad registration for inset
  assert(idx >= 0);
  return createIndex(idx, 0, view);
}

void ViewManagerDataModel::notifyViewAdded_(simVis::View* view)
{
  // Add a callback for the new view
  view->addCallback(viewParamCB_);

  // Update the flat mode back-end w/o sending any gui notifications
  if (isHierarchical() && view->type() != simVis::View::VIEW_SUPERHUD)
    userViews_.push_back(ViewObserverPtr(view));

  // Is it top level?  If so, sync our topLevelViews_
  if (view->getHostView() == NULL && isHierarchical())
  {
    beginInsertRows(QModelIndex(), topLevelViews_.count(), topLevelViews_.count());
    topLevelViews_.push_back(ViewObserverPtr(view));
    endInsertRows();
    return;
  }

  // Flat mode add item and make sure model is updated
  if (!isHierarchical() && view->type() != simVis::View::VIEW_SUPERHUD)
  {
    // Emitting this signal will force a redraw
    beginInsertRows(QModelIndex(), userViews_.count(), userViews_.count());
    userViews_.push_back(ViewObserverPtr(view));
    endInsertRows();
    return;
  }

  // Must be an inset
  beginResetModel();
  endResetModel();
}

void ViewManagerDataModel::notifyViewParamChange_(simVis::View* view)
{
  QModelIndex idx = createIndex_(view);
  emit(dataChanged(idx, idx));
}

bool ViewManagerDataModel::isHierarchical() const
{
  return useHierarchy_;
}

void ViewManagerDataModel::setHierarchical(bool useHierarchy)
{
  if (useHierarchy == useHierarchy_)
    return;
  // Just reset the whole thing and repopulate
  beginResetModel();
  useHierarchy_ = useHierarchy;
  endResetModel();
}

bool ViewManagerDataModel::isUserCheckable() const
{
  return isCheckable_;
}

void ViewManagerDataModel::setUserCheckable(bool isCheckable)
{
  if (isCheckable == isCheckable_)
    return;
  isCheckable_ = isCheckable;
  // Emitting this signal will force a redraw
  if (rowCount() > 0)
    emit(dataChanged(index(0, 0), index(rowCount() - 1, 0)));
}

simVis::View* ViewManagerDataModel::viewFromIndex_(const QModelIndex& index) const
{
  // Hierarchical views can use the internal pointer safely, because on view removal
  // the data pointers are completely reset with a beginResetModel() and endResetModel().
  if (isHierarchical())
    return static_cast<simVis::View*>(index.internalPointer());

  // Non-hierarchical views cannot do the same, because they use begin/endRemoveRows().
  // Because of the way View Manager is structured, we can't know exactly which index
  // was removed, so we instead just do the lookup by index (row)
  if (index.internalPointer() != NULL)
    return userViews_.at(index.row()).get();
  return NULL;
}


}
