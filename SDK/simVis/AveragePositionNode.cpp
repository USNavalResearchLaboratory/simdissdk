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
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*/

#include "osg/BoundingSphere"
#include "simVis/Entity.h"
#include "simVis/AveragePositionNode.h"

namespace simVis
{

/** Update callback that recalculates the average position on each update cycle */
class AveragePositionNode::RecalcUpdateCallback : public osg::Callback
{
public:
  explicit RecalcUpdateCallback(AveragePositionNode& avgNode)
    : avgNode_(avgNode)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    avgNode_.updateAveragePosition_();
    return traverse(object, data);
  }

private:
  AveragePositionNode& avgNode_;
};

///////////////////////////////////////////////////////////////////////

AveragePositionNode::AveragePositionNode()
{
  addUpdateCallback(new RecalcUpdateCallback(*this));
}

AveragePositionNode::~AveragePositionNode()
{
}

void AveragePositionNode::addTrackedNode(EntityNode* node)
{
  if (node)
    nodes_.insert(node);
}

void AveragePositionNode::removeTrackedNode(EntityNode* node)
{
  if (node)
    nodes_.erase(node);
}

bool AveragePositionNode::isTrackingNode(EntityNode* node) const
{
  return (nodes_.find(node) != nodes_.end());
}

double AveragePositionNode::boundingSphereRadius() const
{
  if (!boundingSphere_.valid())
    return 0.0;
  return boundingSphere_.radius();
}

void AveragePositionNode::updateAveragePosition_()
{
  if (nodes_.empty())
    return;

  // Reset bounding sphere
  boundingSphere_.init();

  // Expand bounding sphere by each tracked node's position
  auto it = nodes_.begin();
  while (it != nodes_.end())
  {
    if (!it->valid())
    {
      // Remove invalid nodes
      nodes_.erase(it++);
      continue;
    }
    simCore::Vec3 pos;
    if ((*it)->getPosition(&pos) == 0)
      boundingSphere_.expandBy(osg::Vec3d(pos.x(), pos.y(), pos.z()));
    ++it;
  }

  // Translate the matrix to the center of the bounding sphere
  setMatrix(osg::Matrix::translate(boundingSphere_.center()));
}

}
