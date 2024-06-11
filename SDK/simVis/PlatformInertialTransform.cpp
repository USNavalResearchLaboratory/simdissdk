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
#include "osg/Version"
#include "osgUtil/CullVisitor"
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simVis/Locator.h"
#include "simVis/Utils.h"
#include "simVis/PlatformInertialTransform.h"

namespace simVis
{

PlatformInertialTransform::PlatformInertialTransform()
  : Transform()
{
  // Because the matrix changes based on other input, this must be marked
  // dynamic to avoid being marked as redundant by an optimizer pass.
  setDataVariance(osg::Object::DYNAMIC);
  setName("PlatformInertialTransform");
  callback_ = new SyncLocatorCallback<PlatformInertialTransform>(this);
}

PlatformInertialTransform::PlatformInertialTransform(const PlatformInertialTransform& rhs, const osg::CopyOp& copyop)
  : Transform(rhs, copyop),
    entityRotationInverse_(rhs.entityRotationInverse_),
    locator_(rhs.locator_),
    callback_(static_cast<LocatorCallback*>(copyop(rhs.callback_.get())))
{
}

PlatformInertialTransform::~PlatformInertialTransform()
{
  if (locator_.valid())
    locator_->removeCallback(callback_.get());
}

bool PlatformInertialTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  // Pre-3.6, there's no access to the MV stack.  As a result, billboard image icons
  // may not correctly deal with inertial angles.
#if OSG_MIN_VERSION_REQUIRED(3,6,0)
  // Do not perform any recalculation if visitor is not a cull visitor, if there
  // are no children nodes, or if there aren't the anticipated number of matrices
  // in the model view stack
  osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
  if (!cv || getNumChildren() == 0 || cv->getModelViewStack().size() < 3)
  {
    matrix.preMultRotate(entityRotationInverse_);
    return true;
  }

  // Drop off the last matrix which came from the simVis::BillboardAutoTransform
  osg::CullStack::MatrixStack stack = cv->getModelViewStack();
  stack.pop_back();

  // Save the dynamic scale values.  We're going to want to uniformly re-scale.  If
  // we fail to do so, non-uniform scaling (e.g. from ScaleXYZ) will scale the original
  // rotation vector too, which causes problems with display.
  const auto& dynamicScale = stack.back()->getScale();
  const double maxScale = simCore::sdkMax(dynamicScale.x(), simCore::sdkMax(dynamicScale.y(), dynamicScale.z()));

  // Pop off the dynamic scale transform's matrix, and start with that matrix
  stack.pop_back();
  matrix = *stack.back();

  // Now uniformly scale back up to match dynamic scale
  matrix.preMultScale(osg::Vec3f(maxScale, maxScale, maxScale));

#endif

  // Apply the rotation to get into inertial space
  matrix.preMultRotate(entityRotationInverse_);
  return true;
}

bool PlatformInertialTransform::computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  // Not supported; not required in the use case of setting up inertial axis at this time,
  // and implementation could be expensive.
  return false;
}

void PlatformInertialTransform::childInserted(unsigned int pos)
{
  // If this is the first child, resync with locator to ensure it's up to date
  if (getNumChildren() == 1)
    syncWithLocator();
}

void PlatformInertialTransform::setLocator(simVis::Locator* locator)
{
  if (locator_ == locator)
    return;
  if (locator_.valid())
    locator_->removeCallback(callback_.get());
  locator_ = locator;
  if (locator_.valid())
  {
    locator_->addCallback(callback_.get());
    syncWithLocator();
  }
}

void PlatformInertialTransform::syncWithLocator()
{
  osg::ref_ptr<Locator> locator;
  locator_.lock(locator);
  // Avoid any math if there are no children or if locator is not valid
  if (getNumChildren() == 0 || !locator.valid())
    return;

  // Figure out the right rotation to get to point north
  simCore::Vec3 llaPos;
  simCore::Vec3 llaOri;
  if (locator->getLocatorPositionOrientation(&llaPos, &llaOri, simCore::COORD_SYS_LLA))
  {
    // Reverse the rotation relative to the host platform's locator
    const osg::Quat& q = simVis::Math::eulerRadToQuat(llaOri.yaw(), llaOri.pitch(), llaOri.roll());
    entityRotationInverse_ = q.inverse();
  }
}

}
