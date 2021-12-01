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
#include "simCore/Calc/Calculations.h"
#include "simVis/Entity.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/LocatorNode.h"
#include "simVis/Utils.h"

#undef LC
#define LC "[LocatorNode] "

namespace simVis
{

LocatorNode::LocatorNode()
  : locatorCallback_(nullptr),
    overheadModeHint_(false)
{
  // LocatorNode is valid without any locator; it functions as a group.
}

LocatorNode::LocatorNode(Locator* locator, unsigned int componentsToTrack)
  : locatorCallback_(nullptr),
  componentsToTrack_(componentsToTrack),
  overheadModeHint_(false)
{
  setLocator(locator);
}

LocatorNode::LocatorNode(const LocatorNode& rhs, const osg::CopyOp& op)
  : osg::MatrixTransform(rhs, op),
  matrixRevision_(rhs.matrixRevision_),
  locatorCallback_(nullptr),
  componentsToTrack_(rhs.componentsToTrack_),
  overheadModeHint_(rhs.overheadModeHint_)
{
  setLocator(locator_.get()); // to update the trav count
}

LocatorNode::LocatorNode(Locator* locator, osg::Node* child)
  : LocatorNode(locator)
{
  if (child)
    this->addChild(child);
}

LocatorNode::~LocatorNode()
{
  if (locator_.valid() && locatorCallback_.valid())
    locator_->removeCallback(locatorCallback_.get());
}

void LocatorNode::setLocator(Locator* locator, unsigned int componentsToTrack)
{
  if (locator_.valid() && locatorCallback_.valid())
    locator_->removeCallback(locatorCallback_.get());

  locator_ = locator;
  matrixRevision_.reset();
  componentsToTrack_ = componentsToTrack;

  if (locator)
  {
    locatorCallback_ = new SyncLocatorCallback<LocatorNode>(this);
    locator->addCallback(locatorCallback_.get());
    syncWithLocator();
  }
}

int LocatorNode::getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys) const
{
  if (!out_position)
    return 1;
  if (getNodeMask() == 0 || !locator_.valid())
  {
    // this locatorNode is not active, and does not have a valid position
    return 2;
  }
  if (entityToMonitor_.valid() && !entityToMonitor_->isActive())
  {
    // locatorNode is inactive: the locatorNode is tracking an entity, and that entity is inactive
    return 3;
  }

  const osg::Vec3d& locatorNodeEcef = getMatrix().getTrans();
  if (coordsys == simCore::COORD_SYS_LLA)
  {
    return simCore::CoordinateConverter::convertEcefToGeodeticPos(simCore::Vec3(locatorNodeEcef.x(), locatorNodeEcef.y(), locatorNodeEcef.z()), *out_position);
  }

  out_position->set(locatorNodeEcef.x(), locatorNodeEcef.y(), locatorNodeEcef.z());

  if (coordsys == simCore::COORD_SYS_ECEF)
    return 0;
  if (coordsys == simCore::COORD_SYS_ECI)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, *out_position, locator_->getElapsedEciTime());
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToEci(in, out);
    *out_position = out.position();
    return 0;
  }
  // unsupported coordsys
  return 3;
}

int LocatorNode::getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, simCore::CoordinateSystem coordsys) const
{
  if (!out_position || !out_orientation)
    return 1;
  if (getNodeMask() == 0 || !locator_.valid())
  {
    // this locatorNode is not active, and does not have a valid position
    return 2;
  }
  if (entityToMonitor_.valid() && !entityToMonitor_->isActive())
  {
    // locatorNode is inactive: the locatorNode is tracking an entity, and that entity is inactive
    return 3;
  }

  const osg::Matrixd& m = getMatrix();
  const osg::Vec3d& locatorNodeEcef = m.getTrans();
  out_position->set(locatorNodeEcef.x(), locatorNodeEcef.y(), locatorNodeEcef.z());
  simVis::Math::enuRotMatrixToEcefEuler(m, *out_orientation);

  if (coordsys == simCore::COORD_SYS_ECEF)
    return 0;
  if (coordsys == simCore::COORD_SYS_LLA)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, *out_position, *out_orientation);
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToGeodetic(in, out);
    *out_position = out.position();
    *out_orientation = out.orientation();
    return 0;
  }
  if (coordsys == simCore::COORD_SYS_ECI)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, *out_position, *out_orientation, getLocator()->getElapsedEciTime());
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToEci(in, out);
    *out_position = out.position();
    *out_orientation = out.orientation();
    return 0;
  }
  // unsupported coordsys
  return 3;
}

void LocatorNode::syncWithLocator()
{
  if (getNodeMask() == 0 || !locator_.valid())
  {
    // this locatorNode is not active, and does not have a valid position
    return;
  }
  if (entityToMonitor_.valid() && !entityToMonitor_->isActive())
  {
    // locatorNode is inactive: the locatorNode is tracking an entity, and that entity is inactive
    return;
  }
  if (locator_->outOfSyncWith(matrixRevision_))
  {
    osg::Matrix matrix;

    if (locator_->getLocatorMatrix(matrix, componentsToTrack_) )
    {
      this->setMatrix(matrix);
      locator_->sync(matrixRevision_);
    }
  }
}

bool LocatorNode::computeLocalToWorldMatrix(osg::Matrix& out, osg::NodeVisitor* nv) const
{
  if (!locator_.valid())
  {
    // locatorNode with no locator has the position of its parent
    return true;
  }
  osg::Matrix matrix = getMatrix();

  // It is possible that nv is nullptr if calling computeBound(), which can happen during intersection
  // visitor processing.  To address this, the overheadModeHint_ can be set.  If set and the Node
  // visitor is nullptr, then we do overhead mode calculations for bounding area.
  if (simVis::OverheadMode::isActive(nv) || (overheadModeHint_ && !nv))
  {
    simCore::Vec3 p( matrix(3,0), matrix(3,1), matrix(3,2) );
    p = simCore::clampEcefPointToGeodeticSurface(p);
    matrix.setTrans(p.x(), p.y(), p.z());
  }
  out.preMult(matrix);
  return true;
}

void LocatorNode::setOverheadModeHint(bool overheadMode)
{
  if (overheadMode != overheadModeHint_)
  {
    overheadModeHint_ = overheadMode;
    dirtyBound();
  }
}

bool LocatorNode::overheadModeHint() const
{
  return overheadModeHint_;
}

void LocatorNode::setEntityToMonitor(EntityNode* entity)
{
  entityToMonitor_ = entity;
}

//---------------------------------------------------------------------------

SetOverheadModeHintVisitor::SetOverheadModeHintVisitor(bool hint, TraversalMode tm)
  : NodeVisitor(tm),
    hint_(hint)
{
}

void SetOverheadModeHintVisitor::setOverheadModeHint(bool hint)
{
  hint_ = hint;
}

void SetOverheadModeHintVisitor::apply(osg::MatrixTransform& mx)
{
  simVis::LocatorNode* locatorNode = dynamic_cast<simVis::LocatorNode*>(&mx);
  if (locatorNode)
    locatorNode->setOverheadModeHint(hint_);
  traverse(mx);
}
}
