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
#include <algorithm>
#include "osg/BoundingSphere"
#include "simVis/Entity.h"
#include "simVis/AveragePositionNode.h"

namespace simVis
{

AveragePositionNode::AveragePositionNode()
{
  callback_ = new RecalcUpdateCallback(*this);
}

AveragePositionNode::AveragePositionNode(const std::vector<EntityNode*>& nodes)
{
  callback_ = new RecalcUpdateCallback(*this);
  for (auto it = nodes.begin(); it != nodes.end(); ++it)
    addTrackedNode(*it);
}

AveragePositionNode::~AveragePositionNode()
{
}

void AveragePositionNode::addTrackedNode(EntityNode* node)
{
  if (!node)
    return;

  // Add update callback if this is the first node
  if (nodes_.empty())
    addUpdateCallback(callback_);

  if (std::find(nodes_.begin(), nodes_.end(), node) == nodes_.end())
    nodes_.push_back(node);
}

void AveragePositionNode::removeTrackedNode(EntityNode* node)
{
  if (!node)
    return;

  auto found = std::find(nodes_.begin(), nodes_.end(), node);
  if (found != nodes_.end())
    nodes_.erase(found);

  // Remove the update callback if we're not tracking any nodes
  if (nodes_.empty())
    removeUpdateCallback(callback_);
}

bool AveragePositionNode::isTrackingNode(EntityNode* node) const
{
  if (!node)
    return false;
  return (std::find(nodes_.begin(), nodes_.end(), node) != nodes_.end());
}

int AveragePositionNode::getNumTrackedNodes() const
{
  return static_cast<int>(nodes_.size());
}

void AveragePositionNode::getTrackedIds(std::vector<uint64_t>& ids) const
{
  for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
  {
    if (it->valid())
      ids.push_back((*it)->getId());
  }
}

double AveragePositionNode::boundingSphereRadius() const
{
  if (!boundingSphere_.valid())
    return 0.0;
  return boundingSphere_.radius();
}

void AveragePositionNode::updateAveragePosition_()
{
  bool isExpanded = false;

  if (nodes_.empty())
    return;

  // Reset bounding sphere
  boundingSphere_.init();

  // Remove invalid nodes
  nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(), [](const osg::observer_ptr<simVis::EntityNode>& ptr) { return !ptr.valid(); }), nodes_.end());

  // Expand bounding sphere by each tracked node's position
  for (auto it = nodes_.begin(); it != nodes_.end(); ++it)
  {
    if (!it->valid() || !(*it)->isVisible())
      continue;
    simCore::Vec3 pos;
    if ((*it)->getPosition(&pos) == 0)
    {
      boundingSphere_.expandBy(osg::Vec3d(pos.x(), pos.y(), pos.z()));
      isExpanded = true;
    }
  }

  if (isExpanded)
  {
    // Translate the matrix to the center of the bounding sphere
    setMatrix(osg::Matrix::translate(boundingSphere_.center()));
  }
}

}
