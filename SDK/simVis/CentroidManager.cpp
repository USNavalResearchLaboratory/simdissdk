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
#include <cassert>
#include "osg/Observer"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/CentroidManager.h"

namespace simVis
{

/** ViewManager callback that listens for view removals and notifies the CentroidManager. */
class CentroidManager::ViewsWatcher : public simVis::ViewManager::Callback
{
public:
  explicit ViewsWatcher(CentroidManager* manager)
    : manager_(manager)
  {
  }

  virtual void operator()(simVis::View* inset, const EventType& e)
  {
    switch (e)
    {
    case VIEW_REMOVED:
      if (manager_.valid())
        manager_->removeView_(inset);
      break;
    default:
      break;
    }
  }

private:
  osg::observer_ptr<CentroidManager> manager_;
};

//////////////////////////////////////////////////////////

CentroidManager::CentroidManager()
{
}

CentroidManager::~CentroidManager()
{
}

AveragePositionNode* CentroidManager::createCentroid(const std::vector<EntityNode*>& inNodes, View* view)
{
  // Nothing to do with an empty node vector or a nullptr view
  if (!view || inNodes.empty())
    return nullptr;

  // Remove nullptr nodes
  std::vector<EntityNode*> nodes(inNodes);
  nodes.erase(std::remove(nodes.begin(), nodes.end(), static_cast<EntityNode*>(nullptr)), nodes.end());
  if (nodes.empty())
    return nullptr;

  CentroidInfo info;
  auto viewIter = centroids_.find(view);
  if (viewIter != centroids_.end())
  {
    // Already a centroid on the view. First, check the view is valid
    info = viewIter->second;
    if (!info.viewObs.valid())
    {
      // View is not valid, so remove from the map.
      centroids_.erase(viewIter);
      return nullptr;
    }
    else
    {
      // View is valid. Update its CentroidInfo with the new node list
      removeChild(info.node);
      info.node = new AveragePositionNode(nodes);
      viewIter->second = info;
    }
  }

  else
  {
    // Create the centroid and its info
    info.node = new AveragePositionNode(nodes);
    info.viewObs = view->getOrCreateObserverSet();

    // Install a view manager callback if this is the first view
    if (centroids_.empty())
      initViewCallback_(*view->getViewManager());

    // Add to map
    centroids_[view] = info;
  }

  // Add the node as a child to the manager
  addChild(info.node);
  return info.node.get();
}

void CentroidManager::centerViewOn(const std::vector<EntityNode*>& nodes, View* view)
{
  // Nothing to do with an empty node vector or a nullptr view
  if (!view || nodes.empty())
    return;

  osg::ref_ptr<AveragePositionNode> node = createCentroid(nodes, view);
  if (node.valid())
    view->tetherCamera(node.get());
}

void CentroidManager::initViewCallback_(ViewManager& vm)
{
  vm.addCallback(new ViewsWatcher(this));
}

void CentroidManager::removeView_(View* view)
{
  // Nothing to do if there's no centroid for this view
  auto viewIter = centroids_.find(view);
  if (viewIter == centroids_.end())
    return;

  // Remove the node from the manager
  removeChild(viewIter->second.node);
  // Remove the entry from the map
  centroids_.erase(viewIter);
}

}
