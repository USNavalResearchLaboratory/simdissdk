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
#ifndef SIMVIS_LOCATORNODE_H
#define SIMVIS_LOCATORNODE_H

#include "osg/MatrixTransform"
#include "osg/observer_ptr"
#include "osgEarth/Revisioning"
#include "simCore/Calc/CoordinateSystem.h"
#include "simVis/Locator.h"

namespace simVis
{
class EntityNode;

/// Track the transform of a parent LocatorNode with a Locator
class SDKVIS_EXPORT LocatorNode : public osg::MatrixTransform
{
public:
  /// Provides OSG features for the LocatorNode
  META_Node(simVis, LocatorNode);

  /// Default constructor
  LocatorNode();
  /// Creates a LocatorNode using the locator provided as the position
  explicit LocatorNode(Locator* locator, unsigned int componentsToTrack = Locator::COMP_ALL);
  /// Creates a LocatorNode using the locator provided as the position, adding the child provided to this
  LocatorNode(Locator* locator, osg::Node* child);

  /// locator that is driving this locator node
  Locator* getLocator()       { return locator_.get(); }
  /// locator that is driving this locator node (tail const)
  const Locator* getLocator() const { return locator_.get(); }

  /// set the Locator for this LocatorNode, recalculates the transform matrix
  void setLocator(Locator *locator, unsigned int componentsToTrack = Locator::COMP_ALL);

  /// Turns on or off a flag to hint to use Overhead Mode for bounds computation when NodeVisitor is nullptr
  void setOverheadModeHint(bool overheadMode);
  /// Retrieves a previously set overhead mode hint, used for bounds computation in intersection visitors
  bool overheadModeHint() const;

  /**
  * Gets the world position for this LocatorNode. This is a convenience
  * function that extracts the Position information (not rotation) from the
  * locatorNode matrix.
  *
  * @param[out] out_position If not nullptr, resulting position stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return 0 if the output parameter is populated successfully, nonzero on failure
  */
  int getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Gets the world position reflected by this Locator. This is a convenience
  * function that extracts the Position information and rotation from the
  * locatorNode matrix.
  *
  * @param[out] out_position If not nullptr, resulting position stored here
  * @param[out] out_orientation If not nullptr, resulting orientation stored here
  * @param[in ] coordsys Requested coord sys of the output position (only LLA, ECEF, or ECI supported)
  * @return 0 if the output parameter is populated successfully, nonzero on failure
  */
  int getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
    simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const;

  /**
  * Links the locatorNode to an entity such that the isActive() state of the entity determines whether this node is active
  * @param entity entity to track
  */
  void setEntityToMonitor(EntityNode* entity);

public:
  /// Synchronizes the transform matrix with the locator
  virtual void syncWithLocator();

public: // osg::MatrixTransform
  /// override to support Overhead Mode
  virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;

protected:
  /// osg::Referenced-derived
  virtual ~LocatorNode();
  /// OSG copy constructor implementation, required by META_Node
  LocatorNode(const LocatorNode &rhs, const osg::CopyOp& = osg::CopyOp::SHALLOW_COPY);

private: // data
  osg::ref_ptr<Locator> locator_;
  osgEarth::Util::Revision matrixRevision_;
  osg::ref_ptr<LocatorCallback> locatorCallback_;
  osg::observer_ptr<EntityNode> entityToMonitor_;  ///< if set, the entity whose isActive() state determines the active state of this locatorNode
  unsigned int componentsToTrack_; ///< Locator::Components mask

  /// Sometimes bounds are computed without a node visitor and we need to know if in overhead mode; this flag caches that.
  bool overheadModeHint_;
};

//----------------------------------------------------------------------------
/**
 * Changes the Overhead Mode hint on all LocatorNodes in the scene.
 * This is primarily useful for intersection tests with entities in the scenario when using overhead
 * mode.  This will turn on the overhead mode hint on LocatorNodes so that their bounds computation
 * will return the correct bounds for Overhead mode processing for hit detection.  This should be
 * turned on prior to intersection detection and turned back off after intersection detection done.
 */
class SDKVIS_EXPORT SetOverheadModeHintVisitor : public osg::NodeVisitor
{
public:
  /** Initializes the visitor with the value to set the hint to */
  SetOverheadModeHintVisitor(bool hint, TraversalMode tm=osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

  /** Changes the value of the hint */
  void setOverheadModeHint(bool hint);

  /** Applies to matrices.  Locator nodes are matrix transforms */
  virtual void apply(osg::MatrixTransform& mx);

private:
  /** No copy constructor implemented */
  SetOverheadModeHintVisitor(const SetOverheadModeHintVisitor&);

  bool hint_;
};

}

#endif // SIMVIS_LOCATORNODE_H
