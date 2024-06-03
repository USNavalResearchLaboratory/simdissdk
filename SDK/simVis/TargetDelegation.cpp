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
#include "simVis/GeoFence.h"
#include "simVis/Platform.h"
#include "simVis/TargetDelegation.h"

#undef LC
#define LC "[TargetDelegation] "

namespace simVis
{
TargetDelegation::TargetDelegation()
{
}

TargetDelegation::~TargetDelegation()
{
}

void TargetDelegation::setGeoFence(const GeoFence* fence)
{
  fence_ = fence;
}

void TargetDelegation::addUpdateGeometryCallback(UpdateGeometryCallback* cb)
{
  updateGeometryCallbacks_.push_back(cb);
}

void TargetDelegation::updateGeometry_(osg::MatrixTransform* xform, const osg::Vec3d& ecef)
{
  for (UpdateGeometryCallbacks::const_iterator i = updateGeometryCallbacks_.begin();
      i != updateGeometryCallbacks_.end();
      ++i)
  {
    i->get()->operator()(xform, ecef);
  }
}

void TargetDelegation::addOrUpdate(const PlatformNode* platform)
{
  // determine whether we're already tracking this platform:
  const TargetNodeMap::iterator t = targetNodes_.find(platform);
  const bool isTracked = t != targetNodes_.end();
  osg::MatrixTransform* mt = isTracked? t->second.get() : nullptr;

  // get the ECEF position of the target
  const simData::PlatformUpdate* update = platform->update();
  if (update == nullptr)
  {
    // this probably means the platform should have been removed
    assert(0);
    return;
  }

  // if it's inside the fence, care about it.
  const osg::Vec3d ecef(update->x(), update->y(), update->z());
  const bool needToTrack = fence_.valid() ? fence_->contains(ecef) : true;
  if (needToTrack)
  {
    if (!isTracked)
    {
      SIM_DEBUG << LC << "START tracking: " << platform->getId() << std::endl;

      // attached the shared geometry to a new MT.
      mt = new osg::MatrixTransform();

      // put the MT in the targets group.
      this->addChild(mt);
      targetNodes_[platform] = mt;
    }

    // update the tracking geometry to reflect the new location.
    updateGeometry_(mt, ecef);
  }

  // if it's outside out fence, ignore it.
  else if (isTracked)
  {
    SIM_DEBUG << LC << "STOP tracking: " << platform->getId() << std::endl;

    if (mt->getNumParents() > 0)
      this->removeChild(mt);
    targetNodes_.erase(t);
  }
}

void TargetDelegation::remove(const PlatformNode* platform)
{
  // untrack if tracked
  const TargetNodeMap::iterator i = targetNodes_.find(platform);
  if (i != targetNodes_.end())
  {
    if (i->second->getNumParents() > 0)
      this->removeChild(i->second);
    targetNodes_.erase(i);
  }
}

void TargetDelegation::removeAll()
{
  if (getNumChildren() > 0)
    removeChildren(0, getNumChildren());
  targetNodes_.clear();
}

}
