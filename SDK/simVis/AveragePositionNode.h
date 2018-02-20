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
#ifndef SIMVIS_AVERAGEPOSITIONNODE_H
#define SIMVIS_AVERAGEPOSITIONNODE_H

#include "osg/observer_ptr"
#include "osg/BoundingSphere"
#include "osg/MatrixTransform"
#include "simCore/Common/Common.h"

namespace simVis {

class EntityNode;

/**
* Node that is placed at the center of the bounding sphere
* created by the positions of the tracked simVis::EntityNodes.
*/
class SDKVIS_EXPORT AveragePositionNode : public osg::MatrixTransform
{
public:
  AveragePositionNode();

  /**
  * Add a node to be tracked.
  * @param node EntityNode to track
  */
  void addTrackedNode(EntityNode* node);
  /**
  * Remove a node from being tracked.
  * @param node EntityNode to stop tracking
  */
  void removeTrackedNode(EntityNode* node);
  /**
  * Determine if the given node is being tracked.
  * @param node EntityNode to check for
  * @return True if given node is being tracked, false if not
  */
  bool isTrackingNode(EntityNode* node) const;
  /**
  * Retrieve the radius of the node's bounding sphere.
  * @return radius of the node's bounding sphere
  */
  double boundingSphereRadius() const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "AveragePositionNode"; }

protected:
  /** Protect osg::Referenced-derived destructor */
  virtual ~AveragePositionNode();

private:
  /** Callback to recalculate the bounding sphere */
  class RecalcUpdateCallback;
  /** Recalculate the bounding sphere and translate to the sphere's center. */
  void updateAveragePosition_();

  /** Bounding sphere created by the positions of the tracked EntityNodes. */
  osg::BoundingSphere boundingSphere_;
  /** Vector of EntityNodes being tracked. */
  std::vector<osg::observer_ptr<EntityNode> > nodes_;
};

}

#endif /* SIMVIS_AVERAGEPOSITIONNODE_H */
