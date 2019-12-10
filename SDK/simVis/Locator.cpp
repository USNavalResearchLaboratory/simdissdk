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
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/Utils.h"
#include "simVis/Locator.h"

#undef LC
#define LC "[Locator] "

namespace simVis
{

Locator::Locator(const osgEarth::SpatialReference* mapSRS) :
  mapSRS_(mapSRS),
  componentsToInherit_(COMP_ALL),
  rotOrder_(HPR),
  isEmpty_(true),
  ecefCoordIsSet_(false),
  offsetsAreSet_(false),
  timestamp_(std::numeric_limits<double>::max()),
  eciRefTime_(0.0)
{
  if (!mapSRS_.valid())
    osg::notify(osg::WARN) << "simVis::Locator: illegal, cannot create a Locator with a NULL map SRS." << std::endl;

  ecefCoord_.setCoordinateSystem(simCore::COORD_SYS_ECEF);
}

Locator::Locator(Locator* parentLoc, unsigned int inheritMask) :
  rotOrder_(HPR),
  isEmpty_(false),
  ecefCoordIsSet_(false),
  offsetsAreSet_(false),
  timestamp_(std::numeric_limits<double>::max()),
  eciRefTime_(0.0)
{
  setParentLocator(parentLoc, inheritMask);
  ecefCoord_.setCoordinateSystem(simCore::COORD_SYS_ECEF);
}

void Locator::setMapSRS(const osgEarth::SpatialReference* mapSRS)
{
  if (mapSRS && mapSRS != mapSRS_.get())
  {
    mapSRS_ = mapSRS;
    if (!isEmpty_)
      notifyListeners_();
  }
}

bool Locator::isEmpty() const
{
  return parentLoc_.valid() ? parentLoc_->isEmpty() : isEmpty_;
}

void Locator::setParentLocator(Locator* newParent, unsigned int inheritMask, bool notify)
{
  if (newParent == this)
  {
    osg::notify(osg::WARN)
      << "simVis::Locator: illegal state, Locator cannot self-parent"
      << std::endl;
    return;
  }

  if (newParent == NULL)
  {
    // remove me from my old parent's child list:
    if (parentLoc_.valid())
      parentLoc_->children_.erase(this);
  }

  parentLoc_ = newParent;
  componentsToInherit_ = inheritMask;

  if (newParent)
  {
    mapSRS_ = newParent->getSRS();
    newParent->children_.insert(this);
  }

  if (!mapSRS_.valid())
    osg::notify(osg::WARN) << "simVis::Locator: illegal, cannot create a Locator with a NULL map SRS."
    << std::endl;

  if (notify)
    notifyListeners_();
}

void Locator::setComponentsToInherit(unsigned int value, bool notify)
{
  componentsToInherit_ = value;

  if (notify)
    notifyListeners_();
}

void Locator::setCoordinate(const simCore::Coordinate& coord, bool notify)
{
  if (coord.coordinateSystem() != simCore::COORD_SYS_ECEF)
  {
    simCore::CoordinateConverter conv;
    conv.convert(coord, ecefCoord_, simCore::COORD_SYS_ECEF);
  }
  else
  {
    ecefCoord_ = coord;
  }
  if (coord.elapsedEciTime() != 0)
    timestamp_ = coord.elapsedEciTime() + getEciRefTime();

  isEmpty_ = false;

  ecefCoordIsSet_ = true;

  if (notify)
    notifyListeners_();
}

void Locator::setCoordinate(const simCore::Coordinate& coord, double timestamp, double eciRefTime, bool notify)
{
  timestamp_ = timestamp;
  // Make sure we aren't overwriting a potentially good reference time with the default value
  if (eciRefTime != std::numeric_limits<double>::max())
    setEciRefTime(eciRefTime);

  if (coord.coordinateSystem() != simCore::COORD_SYS_ECEF)
  {
    simCore::Coordinate temp = coord;
    // Ignore whatever is in the coordinate's ECI time and instead use the internal reference time and timestamp
    temp.setElapsedEciTime(getElapsedEciTime());
    simCore::CoordinateConverter conv;
    conv.convert(temp, ecefCoord_, simCore::COORD_SYS_ECEF);
  }
  else
  {
    ecefCoord_ = coord;
  }

  isEmpty_ = false;

  ecefCoordIsSet_ = true;

  if (notify)
    notifyListeners_();
}

void Locator::setLocalOffsets(const simCore::Vec3& pos, const simCore::Vec3& ori, double timestamp, bool notify)
{
  offsetPos_ = pos;
  offsetOri_ = ori;
  offsetsAreSet_ =
      pos.x()   != 0.0 || pos.y()     != 0.0 || pos.z()    != 0.0 ||
      ori.yaw() != 0.0 || ori.pitch() != 0.0 || ori.roll() != 0.0;

  if (timestamp != std::numeric_limits<double>::max())
  {
    timestamp_ = timestamp;
    isEmpty_ = false; // mark this locator as non-empty, since it has a timestamp
  }
  else if (offsetsAreSet_)
    isEmpty_ = false;

  if (notify)
    notifyListeners_();
}

bool Locator::getCoordinate(simCore::Coordinate* out_coord, const simCore::CoordinateSystem& coordsys) const
{
  if (!out_coord || isEmpty())
    return false;

  simCore::Coordinate temp = ecefCoord_;

  if (!ecefCoordIsSet_ && parentLoc_.valid() && componentsToInherit_ != 0)
  {
    simCore::Coordinate parent;
    if (parentLoc_->getCoordinate(&parent))
    {
      if ((componentsToInherit_ & COMP_POSITION) != COMP_NONE)
      {
        temp.setPosition(parent.position());
      }
      if ((componentsToInherit_ & COMP_ORIENTATION) != COMP_NONE)
      {
        temp.setOrientation(parent.orientation());
      }
    }
  }
  temp.setElapsedEciTime(getElapsedEciTime());

  if (coordsys != ecefCoord_.coordinateSystem())
  {
    simCore::CoordinateConverter conv;
    conv.convert(temp, *out_coord, coordsys);
  }
  else
  {
    *out_coord = temp;
  }

  return true;
}

bool Locator::getLocalOffsets(simCore::Vec3& pos, simCore::Vec3& ori) const
{
  if (isEmpty() || !offsetsAreSet_)
  {
    pos.set(0, 0, 0);
    ori.set(0, 0, 0);
    return false;
  }

  pos = offsetPos_;
  ori = offsetOri_;
  return true;
}

#ifdef USE_DEPRECATED_SIMDISSDK_API
void Locator::setRotationOrder(const Locator::RotationOrder& order, bool notify)
{
  rotOrder_ = order;
  if (notify)
    notifyListeners_();
}
#endif

void Locator::resetToLocalTangentPlane(bool notify)
{
  if (notify)
    notifyListeners_();
}


void Locator::endUpdate()
{
  notifyListeners_();
}

void Locator::setTime(double stamp, bool notify)
{
  timestamp_ = stamp;
  isEmpty_ = false;
}

bool Locator::setEciRefTime(double eciRefTime)
{
  // Make sure the current locator is the top-level parent (i.e. has no parent)
  if (!parentLoc_.valid())
  {
    eciRefTime_ = eciRefTime;
    return true;
  }
  return false;
}

double Locator::getTime() const
{
  // Get the parent's timestamp if it exists and check whether it's newer than the current one
  if (parentLoc_.valid())
  {
    double parentTime = parentLoc_->getTime();
    if (parentTime > timestamp_ && parentTime != std::numeric_limits<double>::max())
      return parentTime;
  }
  // If the timestamp is still invalid at this point, default it to 0.
  if (timestamp_ == std::numeric_limits<double>::max())
    return 0.0;
  return timestamp_;
}

double Locator::getTime_() const
{
  // If a timestamp isn't set, try getting a parent timestamp instead
  return (timestamp_ == std::numeric_limits<double>::max() && parentLoc_.valid()) ? parentLoc_->getTime_() : timestamp_;
}

double Locator::getEciRefTime() const
{
  return parentLoc_.valid() ? parentLoc_->getEciRefTime() : eciRefTime_;
}

double Locator::getElapsedEciTime() const
{
  // If no valid timestamp is found, just return 0
  double timestamp = getTime_();
  return timestamp != std::numeric_limits<double>::max() ? timestamp - getEciRefTime() : 0.0;
}

bool Locator::inherits_(unsigned int mask) const
{
  return (componentsToInherit_ & mask) != COMP_NONE;
}

bool Locator::getLocatorPosition(simCore::Vec3* out_position, const simCore::CoordinateSystem& coordsys) const
{
  if (!out_position)
    return false;

  osg::Matrix m;
  if (!getLocatorMatrix(m))
    return false;
  const osg::Vec3d& ecefPos = m.getTrans();

  if (coordsys == simCore::COORD_SYS_ECEF)
  {
    out_position->set(ecefPos.x(), ecefPos.y(), ecefPos.z());
    return true;
  }
  if (coordsys == simCore::COORD_SYS_LLA)
  {
    simCore::CoordinateConverter::convertEcefToGeodeticPos(simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()), *out_position);
    return true;
  }
  if (coordsys == simCore::COORD_SYS_ECI)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, simCore::Vec3(ecefPos.x(), ecefPos.y(), ecefPos.z()), getElapsedEciTime());
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToEci(in, out);
    *out_position = out.position();
    return true;
  }
  // unsupported coordsys
  return false;
}

bool Locator::getLocatorPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, const simCore::CoordinateSystem& coordsys) const
{
  if ((!out_position) || (!out_orientation))
    return false;

  osg::Matrix m;
  if (!getLocatorMatrix(m))
    return false;

  const osg::Vec3d& v = m.getTrans();
  out_position->set(v.x(), v.y(), v.z());
  simVis::Math::enuRotMatrixToEcefEuler(m, *out_orientation);

  if (coordsys == simCore::COORD_SYS_ECEF)
    return true;
  if (coordsys == simCore::COORD_SYS_LLA)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, *out_position, *out_orientation);
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToGeodetic(in, out);
    *out_position = out.position();
    *out_orientation = out.orientation();
    return true;
  }
  if (coordsys == simCore::COORD_SYS_ECI)
  {
    const simCore::Coordinate in(simCore::COORD_SYS_ECEF, *out_position, *out_orientation, getElapsedEciTime());
    simCore::Coordinate out;
    simCore::CoordinateConverter::convertEcefToEci(in, out);
    *out_position = out.position();
    *out_orientation = out.orientation();
    return true;
  }
  // unsupported coordsys
  return false;
}

void Locator::applyOffsets_(osg::Matrixd& output, unsigned int comps) const
{
  // Start by collecting any offsets in this Locator's parent, recursively
  if (parentLoc_.valid())
  {
    parentLoc_->applyOffsets_(output, (comps & componentsToInherit_));
  }

  // now apply this locator's own offsets
  applyLocalOffsets_(output, comps);
}

void Locator::applyLocalOffsets_(osg::Matrixd& output, unsigned int comps) const
{
  if (!offsetsAreSet_)
    return;

  const bool haveOriOffset = ((comps & COMP_ORIENTATION) != COMP_NONE) && (offsetOri_.yaw() != 0 || offsetOri_.pitch() != 0 || offsetOri_.roll() != 0);
  const bool havePosOffset = ((comps & COMP_POSITION) != COMP_NONE) && (offsetPos_.x() != 0 || offsetPos_.y() != 0 || offsetPos_.z() != 0);

  if (havePosOffset)
  {
    output.preMultTranslate(osg::Vec3d(offsetPos_.x(), offsetPos_.y(), offsetPos_.z()));
  }

  if (haveOriOffset)
  {
    if ((comps & COMP_ORIENTATION) == COMP_ORIENTATION)
    {
      const osg::Quat& oq = Math::eulerRadToQuat(offsetOri_.yaw(), offsetOri_.pitch(), offsetOri_.roll());
      output.preMultRotate(oq);
    }
    else
    {
      // partial (not all components requested)
      simCore::Vec3 partialOri(
        (comps & COMP_HEADING) != COMP_NONE ? offsetOri_.yaw() : 0.0,
        (comps & COMP_PITCH) != COMP_NONE ? offsetOri_.pitch() : 0.0,
        (comps & COMP_ROLL) != COMP_NONE ? offsetOri_.roll() : 0.0);

      const osg::Quat& oq = Math::eulerRadToQuat(partialOri.yaw(), partialOri.pitch(), partialOri.roll());
      output.preMultRotate(oq);
    }
  }
}

bool Locator::getLocatorMatrix(osg::Matrixd& output, unsigned int comps) const
{
  osg::Vec3d pos;
  const bool posFound = getPosition_(pos, comps);

  if (getOrientation_(output, comps))
  {
    if (posFound)
      output.postMultTranslate(pos);
  }
  else if (posFound)
  {
    // if we only inherit (or find) a position, convert the matrix to a local tangent plane.
    getSRS()->getEllipsoid()->computeLocalToWorldTransformFromXYZ(pos.x(), pos.y(), pos.z(), output);
  }

  applyOffsets_(output, comps);
  return true;
}

bool Locator::getPosition_(osg::Vec3d& pos, unsigned int comps) const
{
  if (isEmpty())
    return false;

  comps &= componentsToInherit_;
  if ((comps & COMP_POSITION) == COMP_NONE)
    return false;

  if (ecefCoordIsSet_)
  {
    pos = osg::Vec3d(ecefCoord_.x(), ecefCoord_.y(), ecefCoord_.z());
    return true;
  }

  if (getParentLocator())
    return getParentLocator()->getPosition_(pos, comps);

  return false;
}

bool Locator::getOrientation_(osg::Matrixd& rot, unsigned int comps) const
{
  if (isEmpty() || (comps & COMP_ORIENTATION) == COMP_NONE)
    return false;

  if (!ecefCoordIsSet_)
  {
    if (getParentLocator())
      return getParentLocator()->getOrientation_(rot, comps & getComponentsToInherit());
  }
  else if (ecefCoord_.hasOrientation())
  {
    // find the base orientation, then apply offsets
    if ((comps & COMP_ORIENTATION) == COMP_ORIENTATION)
    {
      // easy, use all orientation components
      simVis::Math::ecefEulerToEnuRotMatrix(ecefCoord_.orientation(), rot);
      return true;
    }
    else
    {
      // above, this condition is reason for early exit (if no orientation information is requested in comps)
      // if assert fails, check for changes to the early exit test
      assert((comps & COMP_ORIENTATION) != 0);
      // painful: need to convert to body-local, remove unwanted components,
      // and convert back to ECEF.
      simCore::CoordinateConverter conv;
      simCore::Coordinate lla;
      conv.convert(ecefCoord_, lla, simCore::COORD_SYS_LLA);
      assert(lla.hasOrientation());
      lla.setOrientation(
        (comps & COMP_HEADING) != 0 ? lla.yaw() : 0.0,
        (comps & COMP_PITCH) != 0 ? lla.pitch() : 0.0,
        (comps & COMP_ROLL) != 0 ? lla.roll() : 0.0);

      simCore::Coordinate ecef;
      conv.convert(lla, ecef, simCore::COORD_SYS_ECEF);
      simVis::Math::ecefEulerToEnuRotMatrix(ecef.orientation(), rot);
      return true;
    }
  }
  return false;
}

bool Locator::getLocalTangentPlaneToWorldMatrix(osg::Matrixd& output) const
{
  osg::Matrixd lm;
  if (!getLocatorMatrix(lm))
    return false;

  const osg::Vec3d& ecef = lm.getTrans();
  mapSRS_->getEllipsoid()->computeLocalToWorldTransformFromXYZ(
    ecef.x(), ecef.y(), ecef.z(),
    output);

  return true;
}

void Locator::addCallback(LocatorCallback* callback)
{
  callbacks_.push_back(callback);
}

void Locator::removeCallback(LocatorCallback* callback)
{
  for (std::vector< osg::ref_ptr<LocatorCallback> >::iterator i = callbacks_.begin(); i != callbacks_.end(); ++i)
  {
    if (i->get() == callback)
    {
      callbacks_.erase(i);
      break;
    }
  }
}

void Locator::notifyListeners_()
{
  dirty();

  for (std::vector< osg::ref_ptr<LocatorCallback> >::iterator i = callbacks_.begin(); i != callbacks_.end();)
  {
    LocatorCallback* cb = i->get();
    if (cb)
    {
      (*cb)(this);
      ++i;
    }
    else
    {
      i = callbacks_.erase(i);
    }
  }

  for (std::set< osg::observer_ptr<Locator> >::iterator i = children_.begin(); i != children_.end();)
  {
    Locator* child = i->get();
    if (child)
    {
      child->notifyListeners_();
      ++i;
    }
    else
    {
      // erase(i++) will not work for vector, but does work for std::set
      children_.erase(i++);
    }
  }
}

//---------------------------------------------------------------------------

CachingLocator::CachingLocator(const osgEarth::SpatialReference* mapSRS)
  : Locator(mapSRS)
{}

CachingLocator::CachingLocator(Locator* parentLoc, unsigned int inheritMask)
  : Locator(parentLoc, inheritMask)
{}

bool CachingLocator::getLocatorPosition(simCore::Vec3* out_position, const simCore::CoordinateSystem& coordsys) const
{
  if (!out_position)
    return false;

  if (coordsys != simCore::COORD_SYS_LLA)
    return Locator::getLocatorPosition(out_position, coordsys);

  // use the cached lla position if it is valid
  assert(coordsys == simCore::COORD_SYS_LLA);
  if (inSyncWith(llaPositionCacheRevision_))
  {
    *out_position = llaPositionCache_;
    return true;
  }
  if (Locator::getLocatorPosition(out_position, coordsys))
  {
    llaPositionCache_ = *out_position;
    sync(llaPositionCacheRevision_);
    return true;
  }
  return false;
}

bool CachingLocator::getLocatorPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation, const simCore::CoordinateSystem& coordsys) const
{
  if ((!out_position) || (!out_orientation))
    return false;

  if (coordsys != simCore::COORD_SYS_LLA)
    return Locator::getLocatorPositionOrientation(out_position, out_orientation, coordsys);

  // use the cached lla position & orientation if they are valid
  assert(coordsys == simCore::COORD_SYS_LLA);
  if (inSyncWith(llaPositionCacheRevision_) && inSyncWith(llaOrientationCacheRevision_))
  {
    *out_position = llaPositionCache_;
    *out_orientation = llaOrientationCache_;
    return true;
  }
  if (Locator::getLocatorPositionOrientation(out_position, out_orientation, coordsys))
  {
    llaPositionCache_ = *out_position;
    llaOrientationCache_ = *out_orientation;
    sync(llaPositionCacheRevision_);
    sync(llaOrientationCacheRevision_);
    return true;
  }
  return false;
}

//---------------------------------------------------------------------------

ResolvedPositionOrientationLocator::ResolvedPositionOrientationLocator(const osgEarth::SpatialReference* mapSRS)
  : Locator(mapSRS)
{}

ResolvedPositionOrientationLocator::ResolvedPositionOrientationLocator(Locator* parentLoc, unsigned int inheritMask)
  : Locator(parentLoc, inheritMask)
{}

// ignores comps arg, since children's comps do not affect the resolved position (but do affect subsequent offsets)
bool ResolvedPositionOrientationLocator::getPosition_(osg::Vec3d& pos, unsigned int comps) const
{
  osg::Matrixd mat;
  // resolved position is not modified by children's inheritance orientation components
  if (getParentLocator() && getParentLocator()->getLocatorMatrix(mat, getComponentsToInherit()))
  {
    // strip out orientation and scale
    pos = mat.getTrans();
    return true;
  }
  return false;
}
// only apply our local offsets
// do not apply parent offsets, since they have already been processed to produce the resolved position
void ResolvedPositionOrientationLocator::applyOffsets_(osg::Matrixd& output, unsigned int comps) const
{
  // apply only this locator's own offsets
  applyLocalOffsets_(output, comps);
}

//---------------------------------------------------------------------------

ResolvedPositionLocator::ResolvedPositionLocator(const osgEarth::SpatialReference* mapSRS)
  : ResolvedPositionOrientationLocator(mapSRS)
{}

ResolvedPositionLocator::ResolvedPositionLocator(Locator* parentLoc, unsigned int inheritMask)
  : ResolvedPositionOrientationLocator(parentLoc, inheritMask)
{}

// strip out all orientation components, so that this locator returns a position with an identity orientation
bool ResolvedPositionLocator::getOrientation_(osg::Matrixd& ori, unsigned int comps) const
{
  return false;
}
}
