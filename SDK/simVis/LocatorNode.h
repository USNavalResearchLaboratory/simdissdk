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
#ifndef SIMVIS_LOCATORNODE_H
#define SIMVIS_LOCATORNODE_H

#include "osg/MatrixTransform"
#include "osgEarth/Revisioning"

namespace simVis
{
class Locator;
struct LocatorCallback;

//----------------------------------------------------------------------------
/// Track the transform of a parent LocatorNode with a Locator
class SDKVIS_EXPORT LocatorNode : public osg::MatrixTransform
{
public:
  /// Provides OSG features for the LocatorNode
  META_Node(simVis, LocatorNode);

  /// Default constructor
  LocatorNode();
  /// Creates a LocatorNode using the locator provided as the position
  LocatorNode(Locator* locator);
  /// Creates a LocatorNode using the locator provided as the position, adding the child provided to this
  LocatorNode(Locator* locator, osg::Node* child);
  /// OSG copy constructor implementation
  LocatorNode(const LocatorNode &rhs, const osg::CopyOp& = osg::CopyOp::SHALLOW_COPY);

  /// locator that is driving this locator node
  Locator* getLocator()       { return locator_.get(); }
  /// locator that is driving this locator node (tail const)
  const Locator* getLocator() const { return locator_.get(); }

  /// convenience function to extract the current world coordinates
  void getWorldPosition(osg::Vec3d& out_ecef) const
  {
    out_ecef = getMatrix().getTrans();
  }

  /// set the Locator for this LocatorNode, recalculates the transform matrix
  void setLocator(Locator *locator);

  /// Turns on or off a flag to hint to use Overhead Mode for bounds computation when NodeVisitor is NULL
  void setOverheadModeHint(bool overheadMode);
  /// Retrieves a previously set overhead mode hint, used for bounds computation in intersection visitors
  bool overheadModeHint() const;

public:
  /// Synchronizes the transform matrix with the locator
  virtual void syncWithLocator();

public: // osg::MatrixTransform
  /// override to support Overhead Mode
  virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;

protected:
  /// osg::Referenced-derived
  virtual ~LocatorNode();

private: // data
  osg::ref_ptr<Locator> locator_;
  osgEarth::Revision    matrixRevision_;
  osg::ref_ptr<LocatorCallback> locatorCallback_;

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
