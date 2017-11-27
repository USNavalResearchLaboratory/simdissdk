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
#include "simNotify/Notify.h"
#include "simVis/Locator.h"
#include "simVis/Utils.h"
#include "simVis/PlatformInertialTransform.h"

namespace simVis
{

PlatformInertialTransform::PlatformInertialTransform()
  : Transform()
{
  setName("PlatformInertialTransform");
  callback_ = new SyncLocatorCallback<PlatformInertialTransform>(this);
}

PlatformInertialTransform::PlatformInertialTransform(const PlatformInertialTransform& rhs, const osg::CopyOp& copyop)
  : Transform(rhs, copyop),
    rotation_(rhs.rotation_),
    locator_(rhs.locator_),
    callback_(static_cast<LocatorCallback*>(copyop(rhs.callback_)))
{
}

bool PlatformInertialTransform::computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  matrix.preMultRotate(rotation_);
  return true;
}

bool PlatformInertialTransform::computeWorldToLocalMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const
{
  matrix.postMultRotate(rotationInverse_);
  return true;
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
    locator_->removeCallback(callback_);
  locator_ = locator;
  if (locator_.valid())
  {
    locator_->addCallback(callback_);
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
    if (q != rotationInverse_)
    {
      // We only really care about the inverse of the angle, but we're storing both for efficiency
      rotation_ = q.inverse();
      rotationInverse_ = q;
    }
  }
}

}
