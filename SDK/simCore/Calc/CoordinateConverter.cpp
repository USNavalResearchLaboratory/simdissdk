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
#include <cstring>
#include <cmath>
#include <cassert>
#include <limits>

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"

using namespace simCore;

//------------------------------------------------------------------------
Coordinate::Coordinate()
  : system_(COORD_SYS_NONE),
  elapsedEciTime_(0.0),
  hasVel_(false),
  hasOri_(false),
  hasAcc_(false)
{
}

Coordinate::Coordinate(CoordinateSystem system, const Vec3 &pos, double elapsedECITime)
  : system_(system),
  elapsedEciTime_(elapsedECITime),
  hasVel_(false),
  hasOri_(false),
  hasAcc_(false)
{
  setPosition(pos);
}

Coordinate::Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, double elapsedECITime)
  : system_(system),
  elapsedEciTime_(elapsedECITime),
  hasVel_(false),
  hasOri_(false),
  hasAcc_(false)
{
  setPosition(pos);
  setOrientation(ori);
}

Coordinate::Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, const Vec3 &vel, double elapsedECITime)
  : system_(system),
  elapsedEciTime_(elapsedECITime),
  hasVel_(false),
  hasOri_(false),
  hasAcc_(false)
{
  setPosition(pos);
  setOrientation(ori);
  setVelocity(vel);
}

Coordinate::Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, const Vec3 &vel, const Vec3 &acc, double elapsedECITime)
  : system_(system),
  elapsedEciTime_(elapsedECITime),
  hasVel_(false),
  hasOri_(false),
  hasAcc_(false)
{
  setPosition(pos);
  setOrientation(ori);
  setVelocity(vel);
  setAcceleration(acc);
}

Coordinate::Coordinate(const Coordinate &coord)
  : system_(coord.system_),
  pos_(coord.pos_),
  vel_(coord.vel_),
  ori_(coord.ori_),
  acc_(coord.acc_),
  elapsedEciTime_(coord.elapsedEciTime_),
  hasVel_(coord.hasVel_),
  hasOri_(coord.hasOri_),
  hasAcc_(coord.hasAcc_)
{
}

Coordinate& Coordinate::operator=(const Coordinate &coord)
{
  if (&coord == this)
    return *this;
  system_ = coord.system_;
  pos_.set(coord.pos_);
  vel_.set(coord.vel_);
  ori_.set(coord.ori_);
  acc_.set(coord.acc_);

  elapsedEciTime_ = coord.elapsedEciTime_;

  hasVel_ = coord.hasVel_;
  hasOri_ = coord.hasOri_;
  hasAcc_ = coord.hasAcc_;

  return *this;
}

void Coordinate::clear()
{
  system_ = COORD_SYS_NONE;
  elapsedEciTime_ = 0.0;
  hasVel_ = false;
  hasOri_ = false;
  hasAcc_ = false;

  pos_.zero();
  vel_.zero();
  ori_.zero();
  acc_.zero();
}

/// Sets the individual position state components
void Coordinate::setPosition(double x, double y, double z)
{
  assert(finite(x) && finite(y) && finite(z));
  pos_.set(x, y, z);
}

/// Sets the individual position state components for a geodetic position
void Coordinate::setPositionLLA(double lat, double lon, double alt)
{
  assert(finite(lat) && finite(lon) && finite(alt));
  pos_.set(lat, lon, alt);
}

/// Sets the position state vector
void Coordinate::setPosition(const Vec3 &pos)
{
  assert(finite(pos.x()) && finite(pos.y()) && finite(pos.z()));
  pos_.set(pos);
}

/// Sets the individual orientation state components
void Coordinate::setOrientation(double yaw, double pitch, double roll)
{
  assert(finite(yaw) && finite(pitch) && finite(roll));
  hasOri_ = true;
  ori_.set(yaw, pitch, roll);
}

/// Sets the individual Euler orientation state components
void Coordinate::setOrientationEuler(double psi, double theta, double phi)
{
  assert(finite(psi) && finite(theta) && finite(phi));
  hasOri_ = true;
  ori_.set(psi, theta, phi);
}

/// Sets the orientation state vector
void Coordinate::setOrientation(const Vec3 &ori)
{
  assert(finite(ori.yaw()) && finite(ori.pitch()) && finite(ori.roll()));
  hasOri_ = true;
  ori_.set(ori);
}

/// Sets the individual velocity state components
void Coordinate::setVelocity(double x, double y, double z)
{
  assert(finite(x) && finite(y) && finite(z));
  hasVel_ = true;
  vel_.set(x, y, z);
}

/// Sets the velocity state vector
void Coordinate::setVelocity(const Vec3 &vel)
{
  assert(finite(vel.x()) && finite(vel.y()) && finite(vel.z()));
  hasVel_ = true;
  vel_.set(vel);
}

/// Sets the individual acceleration state components
void Coordinate::setAcceleration(double x, double y, double z)
{
  assert(finite(x) && finite(y) && finite(z));
  hasAcc_ = true;
  acc_.set(x, y, z);
}

/// Sets the acceleration state vector
void Coordinate::setAcceleration(const Vec3 &acc)
{
  assert(finite(acc.x()) && finite(acc.y()) && finite(acc.z()));
  hasAcc_ = true;
  acc_.set(acc);
}

/// Sets the elapsed time since the ECI frame was defined
void Coordinate::setElapsedEciTime(double elapsedEciTime)
{
  elapsedEciTime_ = elapsedEciTime;
}

//------------------------------------------------------------------------
CoordinateConverter::CoordinateConverter()
  : latRadius_(0.0),
  lonRadius_(0.0),
  invLatRadius_(0.0),
  invLonRadius_(0.0),
  referenceOrigin_(HUGE_VAL, HUGE_VAL, 0.0),
  tangentPlaneOffsetX_(0.0),
  tangentPlaneOffsetY_(0.0),
  tangentPlaneRotation_(0.0),
  cosTPR_(cos(tangentPlaneRotation_)),
  sinTPR_(sin(tangentPlaneRotation_)),
  refOriginStatus_(REF_ORIGIN_NOT_SET)
{
}

CoordinateConverter::CoordinateConverter(const CoordinateConverter& other)
{
  operator=(other);
}

CoordinateConverter& CoordinateConverter::operator=(const CoordinateConverter& other)
{
  if (&other == this)
    return *this;

  latRadius_ = other.latRadius_;
  lonRadius_ = other.lonRadius_;
  invLatRadius_ = other.invLatRadius_;
  invLonRadius_ = other.invLonRadius_;
  tangentPlaneOffsetX_ = other.tangentPlaneOffsetX_;
  tangentPlaneOffsetY_ = other.tangentPlaneOffsetY_;
  tangentPlaneRotation_ = other.tangentPlaneRotation_;
  cosTPR_ = other.cosTPR_;
  sinTPR_ = other.sinTPR_;
  refOriginStatus_ = other.refOriginStatus_;
  memcpy(rotationMatrixNED_, other.rotationMatrixNED_, sizeof(double) * 9);
  memcpy(rotationMatrixENU_, other.rotationMatrixENU_, sizeof(double) * 9);
  tangentPlaneTranslation_.set(other.tangentPlaneTranslation_);
  referenceOrigin_.set(other.referenceOrigin_);

  return *this;
}

///Returns reference latitude of CoordinateConverter
double CoordinateConverter::referenceLat() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter reference latitude has not been set" << std::endl;
  return referenceOrigin_.lat();
}

///Returns reference longitude of CoordinateConverter
double CoordinateConverter::referenceLon() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter reference longitude has not been set" << std::endl;
  return referenceOrigin_.lon();
}

///Returns reference altitude of CoordinateConverter
double CoordinateConverter::referenceAlt() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter reference altitude has not been set" << std::endl;
  return referenceOrigin_.alt();
}

///Returns reference origin of CoordinateConverter
const Vec3& CoordinateConverter::referenceOrigin() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter reference origin has not been set" << std::endl;
  return referenceOrigin_;
}

///Returns the calculated radius of earth at reference longitude (m)
double CoordinateConverter::lonRadius() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter longitude radius has not been set" << std::endl;
  return lonRadius_;
}

///Returns the calculated radius of earth at reference latitude (m)
double CoordinateConverter::latRadius() const
{
  if (!hasReferenceOrigin())
    SIM_WARN << "CoordinateConverter latitude radius has not been set" << std::endl;
  return latRadius_;
}

/// calculate scaled earth radii based on a given lat/lon origin, plus the tangent plane rotation/translation matrix
void CoordinateConverter::calculateReferenceRadius_(const Vec3 &lla)
{
  // Latitude limits are +/-90.0
  simCore::Vec3 fixedLla;
  fixedLla.setLat(angFixPI2(lla.lat()));
  // Longitude limits are +/-180.0
  fixedLla.setLon(angFixPI(lla.lon()));
  fixedLla.setAlt(lla.alt());

  // prevent redundant calculations when the identical origin is specified
  if (hasReferenceOrigin() && (referenceOrigin_ == fixedLla))
    return;
  referenceOrigin_ = fixedLla;

  // compute radius of curvature for scaled flat earth systems (ENU/NED/NWU) based on latitude
  // http://www.oc.nps.edu/oc2902w/geodesy/radiigeo.pdf
  const double sposla = sin(referenceOrigin_[0]);
  const double x = 1.0 - WGS_ESQ * (sposla * sposla);
  // radius of curvature in prime vertical
  const double rN = WGS_A / sqrt(x);
  // radius of curvature in meridian
  latRadius_ = rN * (1. - WGS_ESQ) / x;
  // adjust radius of curvature for prime vertical based on latitude
  lonRadius_ = rN * cos(referenceOrigin_[0]);
  // origin values at/near poles are degenerate for scaled flat earth conversions
  refOriginStatus_ = (simCore::areEqual(fabs(referenceOrigin_[0]), M_PI_2, 1e-5)) ? REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE : REF_ORIGIN_SET;

  // prevent divide by zero errors
  assert(latRadius_ != 0);
  assert(lonRadius_ != 0);
  invLatRadius_ = (simCore::areEqual(latRadius_, 0, 1e-5)) ? std::numeric_limits<float>::max() : 1.0 / latRadius_;
  // at the pole, the radius of curvature will be infinite
  invLonRadius_ = (simCore::areEqual(lonRadius_, 0, 1e-5)) ? std::numeric_limits<float>::max() : 1.0 / lonRadius_;

  CoordinateConverter::setLocalToEarthMatrix(lla.lat(), lla.lon(), LOCAL_LEVEL_FRAME_NED, rotationMatrixNED_);

  // set tangent plane translation matrix
  const double sinlat = sin(lla.lat()); // sin of latitude
  const double coslat = cos(lla.lat()); // cosine of latitude
  const double sinlon = sin(lla.lon()); // sin of longitude
  const double coslon = cos(lla.lon()); // cosine of longitude

  // set ENU (X-EAST) rotation matrix
  // local x unit vector
  rotationMatrixENU_[0][0] = -sinlon;
  rotationMatrixENU_[0][1] =  coslon;
  rotationMatrixENU_[0][2] =  0.0;

  // local y unit vector
  rotationMatrixENU_[1][0] = -sinlat * coslon;
  rotationMatrixENU_[1][1] = -sinlat * sinlon;
  rotationMatrixENU_[1][2] =  coslat;

  // local z unit vector
  rotationMatrixENU_[2][0] = coslat * coslon;
  rotationMatrixENU_[2][1] = coslat * sinlon;
  rotationMatrixENU_[2][2] = sinlat;

  // intermediate variables for translation calculation
  const double c3 = WGS_A / sqrt(1.0 - WGS_ESQ * sinlat * sinlat);
  const double c4 = (c3 + lla.alt()) * coslat;
  const double c5 = (WGS_ESQC * c3 + lla.alt()) * sinlat;

  // translation vector for XEAST
  tangentPlaneTranslation_.set(coslon * c4, sinlon * c4, c5);
}

void CoordinateConverter::setReferenceOriginDegrees(double lat, double lon, double alt)
{
  calculateReferenceRadius_(Vec3(lat*DEG2RAD, lon*DEG2RAD, alt));
}

void CoordinateConverter::setReferenceOriginDegrees(const Vec3 &lla)
{
  calculateReferenceRadius_(Vec3(lla.lat()*DEG2RAD, lla.lon()*DEG2RAD, lla.alt()));
}

void CoordinateConverter::setReferenceOrigin(double lat, double lon, double alt)
{
  calculateReferenceRadius_(Vec3(lat, lon, alt));
}

void CoordinateConverter::setReferenceOrigin(const Vec3 &lla)
{
  calculateReferenceRadius_(lla);
}

void CoordinateConverter::setTangentPlaneRotation(double d)
{
  tangentPlaneRotation_ = d;
  cosTPR_ = cos(tangentPlaneRotation_);
  sinTPR_ = sin(tangentPlaneRotation_);
}

void CoordinateConverter::setTangentPlaneOffsets(double xShift, double yShift, double angle)
{
  tangentPlaneRotation_ = angle;
  tangentPlaneOffsetX_ = xShift;
  tangentPlaneOffsetY_ = yShift;
  cosTPR_ = cos(tangentPlaneRotation_);
  sinTPR_ = sin(tangentPlaneRotation_);
}

///@pre tpCoord valid
void CoordinateConverter::applyTPOffsetRotate_(Coordinate &tpCoord) const
{
  // tangentPlaneOffsetY_ is the true north distance of the desired
  // (0,0) origin as seen from the tangential point, and
  // tangentPlaneOffsetX_ is the true east distance.
  // tangentPlaneOffsetY_ is positive if the origin lies to the north
  // of the tangential point, and it is negative if the origin lies to
  // the south.  tangentPlaneOffsetX_ is positive if the origin lies to
  // the east of the tangential point, and it is negative if the origin
  // lies to the west.  tangentPlaneRotation_ is the desired angle
  // to apply to rotate the X-Y plane, and a positive value means a
  // clockwise rotation.  That is, the angle from true north clockwise
  // to the +y axis is equal to tangentPlaneRotation_.  The "raw" tangent
  // plane values (XEAST) are first translated by tangentPlaneOffsetX_ and
  // tangentPlaneOffsetY_, and then the rotation is applied.

  Vec3 pos(tpCoord.position());
  const double tmpX = pos.x();
  pos.setX((tmpX-tangentPlaneOffsetX_)*cosTPR_ - (pos.y()-tangentPlaneOffsetY_)*sinTPR_);
  // Note: pos.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpX)
  pos.setY((tmpX-tangentPlaneOffsetX_)*sinTPR_ + (pos.y()-tangentPlaneOffsetY_)*cosTPR_);
  tpCoord.setPosition(pos);

  if (tpCoord.hasOrientation())
  {
    Vec3 eul(tpCoord.orientation());
    eul.setPsi(angFix2PI(eul.psi() - tangentPlaneRotation_));
    tpCoord.setOrientation(eul);
  }

  if (tpCoord.hasVelocity())
  {
    Vec3 vel(tpCoord.velocity());
    const double tmpVx = vel.x();
    vel.setX(tmpVx*cosTPR_ - vel.y()*sinTPR_);
    // Note: vel.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpVx)
    vel.setY(tmpVx*sinTPR_ + vel.y()*cosTPR_);
    tpCoord.setVelocity(vel);
  }

  if (tpCoord.hasAcceleration())
  {
    Vec3 acc(tpCoord.acceleration());
    const double tmpAx = acc.x();
    acc.setX(tmpAx*cosTPR_ - acc.y()*sinTPR_);
    // Note: acc.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpAx)
    acc.setY(tmpAx*sinTPR_ + acc.y()*cosTPR_);
    tpCoord.setAcceleration(acc);
  }
}

///@pre tpCoord valid
void CoordinateConverter::reverseTPOffsetRotate_(Coordinate &gtpCoord) const
{
  // reverse rotation and then tangent plane X&Y offsets
  Vec3 pos(gtpCoord.position());
  const double tmpX = pos.x();
  pos.setX((tmpX*cosTPR_ + pos.y()*sinTPR_) + tangentPlaneOffsetX_);
  // Note: pos.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpX)
  pos.setY((-tmpX*sinTPR_ + pos.y()*cosTPR_) + tangentPlaneOffsetY_);
  gtpCoord.setPosition(pos);

  if (gtpCoord.hasOrientation())
  {
    Vec3 eul(gtpCoord.orientation());
    eul.setPsi(angFix2PI(eul.psi() + tangentPlaneRotation_));
    gtpCoord.setOrientation(eul);
  }

  if (gtpCoord.hasVelocity())
  {
    Vec3 vel(gtpCoord.velocity());
    const double tmpVx = vel.x();
    vel.setX(tmpVx*cosTPR_ + vel.y()*sinTPR_);
    // Note: vel.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpVx)
    vel.setY(-tmpVx*sinTPR_ + vel.y()*cosTPR_);
    gtpCoord.setVelocity(vel);
  }

  if (gtpCoord.hasAcceleration())
  {
    Vec3 acc(gtpCoord.acceleration());
    const double tmpAx = acc.x();
    acc.setX(tmpAx*cosTPR_ + acc.y()*sinTPR_);
    // Note: acc.setX() changes the .x() return value, changing the call below.  Use a temp variable (tmpAx)
    acc.setY(-tmpAx*sinTPR_ + acc.y()*cosTPR_);
    gtpCoord.setAcceleration(acc);
  }
}

/// convert 'inCoord' to 'outSystem', velocity, acceleration & Eulers are referenced to a NED system
///@param[in ] inCoord  incoming data (Coordinate contains position, orientation, velocity and acceleration); when converting to/from ECI, inCoord must contain a valid elapsedEciTime value)
///@param[out] outCoord outgoing data (Coordinate contains position, orientation, velocity and acceleration)
///@param[in ] outSystem projection system to use
///@return 0 on success, !0 on failure
///@pre outCoord valid and outCoord does not alias inCoord
int CoordinateConverter::convert(const Coordinate &inCoord, Coordinate &outCoord, CoordinateSystem outSystem) const
{
  // Test for same input/output -- this function cannot handle case of inCoord == outCoord
  if (&inCoord == &outCoord)
  {
    assert(0);
    return 1;
  }

  if (outSystem == inCoord.coordinateSystem()) // easy case
  {
    outCoord = inCoord;
    return 0; // done
  }

  // clear any data in outCoord and set its coordinate system and time
  outCoord.clear();
  outCoord.setCoordinateSystem(outSystem);
  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  switch (inCoord.coordinateSystem())
  {
  case COORD_SYS_LLA:
    switch (outSystem)
    {
    case COORD_SYS_NED:
    case COORD_SYS_NWU:
    case COORD_SYS_ENU:
      if (convertGeodeticToFlat_(inCoord, outCoord, outSystem) != 0)
        return 1;
      break;
    case COORD_SYS_ECEF:
      CoordinateConverter::convertGeodeticToEcef(inCoord, outCoord);
      break;
    case COORD_SYS_XEAST:
    case COORD_SYS_GTP:
      if (convertGeodeticToXEast_(inCoord, outCoord) != 0)
        return 1;
      if (outSystem == COORD_SYS_GTP)
      {
        applyTPOffsetRotate_(outCoord);
      }
      break;
    case COORD_SYS_ECI:
      {
        Coordinate ecefCoord;
        CoordinateConverter::convertGeodeticToEcef(inCoord, ecefCoord);
        CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
        break;
      }
    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_NED:
    switch (outSystem)
    {
    case COORD_SYS_NWU:
      // x = x, y = -y, z = -z
      CoordinateConverter::swapNedNwu(inCoord, outCoord);
      break;

    case COORD_SYS_ENU:
      // x = y, y = x, z = -z
      CoordinateConverter::swapNedEnu(inCoord, outCoord);
      break;

    case COORD_SYS_LLA:
      if (convertFlatToGeodetic_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_ECEF:
      if (convertFlatToEcef_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_ECI:
    case COORD_SYS_XEAST:
    case COORD_SYS_GTP:
      {
        Coordinate ecefCoord;

        if (convertFlatToEcef_(inCoord, ecefCoord) != 0)
          return 1;
        switch (outSystem)
        {
        case COORD_SYS_ECI:
          CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
          break;
        case COORD_SYS_XEAST:
        case COORD_SYS_GTP:
          if (convertEcefToXEast_(ecefCoord, outCoord) != 0)
            return 1;
          if (outSystem == COORD_SYS_GTP)
          {
            applyTPOffsetRotate_(outCoord);
          }
          break;
        default:
          assert(0);
          return 1;
          break;
        }
        break;
      }

    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_NWU:
    switch (outSystem)
    {
    case COORD_SYS_NED:
      // x = x, y = -y, z = -z
      CoordinateConverter::swapNedNwu(inCoord, outCoord);
      break;

    case COORD_SYS_ENU:
      // x = y, y = -x, z = z
      CoordinateConverter::convertNwuToEnu(inCoord, outCoord);
      break;

    case COORD_SYS_LLA:
      if (convertFlatToGeodetic_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_ECEF:
      if (convertFlatToEcef_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_ECI:
    case COORD_SYS_XEAST:
    case COORD_SYS_GTP:
      {
        Coordinate ecefCoord;
        if (convertFlatToEcef_(inCoord, ecefCoord) != 0)
          return 1;
        switch (outSystem)
        {
        case COORD_SYS_ECI:
          CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
          break;
        case COORD_SYS_XEAST:
        case COORD_SYS_GTP:
          if (convertEcefToXEast_(ecefCoord, outCoord) != 0)
            return 1;
          if (outSystem == COORD_SYS_GTP)
          {
            applyTPOffsetRotate_(outCoord);
          }
          break;
        default:
          assert(0);
          return 1;
          break;
        }
        break;
      }
    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_ENU:
    switch (outSystem)
    {
    case COORD_SYS_NED:
      // x = y, y = x, z = -z
      CoordinateConverter::swapNedEnu(inCoord, outCoord);
      break;
    case COORD_SYS_NWU:
      // x = y, y = -x, z = z
      CoordinateConverter::convertEnuToNwu(inCoord, outCoord);
      break;
    case COORD_SYS_LLA:
      if (convertFlatToGeodetic_(inCoord, outCoord) != 0)
        return 1;
      break;
    case COORD_SYS_ECEF:
      if (convertFlatToEcef_(inCoord, outCoord) != 0)
        return 1;
      break;
    case COORD_SYS_ECI:
    case COORD_SYS_XEAST:
    case COORD_SYS_GTP:
      {
        Coordinate ecefCoord;
        if (convertFlatToEcef_(inCoord, ecefCoord) != 0)
          return 1;
        switch (outSystem)
        {
        case COORD_SYS_ECI:
          CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
          break;
        case COORD_SYS_XEAST:
        case COORD_SYS_GTP:
          if (convertEcefToXEast_(ecefCoord, outCoord) != 0)
            return 1;
          if (outSystem == COORD_SYS_GTP)
          {
            applyTPOffsetRotate_(outCoord);
          }
          break;
        default:
          assert(0);
          return 1;
          break;
        }
        break;
      }
    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_ECEF:
    switch (outSystem)
    {
    case COORD_SYS_NED:
    case COORD_SYS_NWU:
    case COORD_SYS_ENU:
      if (convertEcefToFlat_(inCoord, outCoord, outSystem) != 0)
        return 1;
      break;

    case COORD_SYS_LLA:
      CoordinateConverter::convertEcefToGeodetic(inCoord, outCoord);
      break;

    case COORD_SYS_ECI:
      CoordinateConverter::convertEcefToEci(inCoord, outCoord, inCoord.elapsedEciTime());
      break;

    case COORD_SYS_XEAST:
    case COORD_SYS_GTP:
      if (convertEcefToXEast_(inCoord, outCoord) != 0)
        return 1;
      if (outSystem == COORD_SYS_GTP)
      {
        applyTPOffsetRotate_(outCoord);
      }
      break;

    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_XEAST:
    switch (outSystem)
    {
    case COORD_SYS_NED:
    case COORD_SYS_NWU:
    case COORD_SYS_ENU:
    case COORD_SYS_ECI:
      {
        Coordinate ecefCoord;
        if (convertXEastToEcef_(inCoord, ecefCoord) != 0)
          return 1;
        switch (outSystem)
        {
        case COORD_SYS_NED:
        case COORD_SYS_NWU:
        case COORD_SYS_ENU:
          if (convertEcefToFlat_(ecefCoord, outCoord, outSystem) != 0)
            return 1;
          break;
        case COORD_SYS_ECI:
          CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
          break;
        default:
          assert(0);
          return 1;
          break;
        }
        break;
      }

    case COORD_SYS_LLA:
      if (convertXEastToGeodetic_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_ECEF:
      if (convertXEastToEcef_(inCoord, outCoord) != 0)
        return 1;
      break;

    case COORD_SYS_GTP:
      outCoord.setPosition(inCoord.position());

      if (inCoord.hasVelocity())
      {
        outCoord.setVelocity(inCoord.velocity());
      }

      if (inCoord.hasAcceleration())
      {
        outCoord.setAcceleration(inCoord.acceleration());
      }

      if (inCoord.hasOrientation())
      {
        outCoord.setOrientation(inCoord.orientation());
      }

      applyTPOffsetRotate_(outCoord);
      break;

    default:
      assert(0);
      return 1;
      break;
    }
    break;

  case COORD_SYS_GTP:
    {
      Coordinate xeastCoord(inCoord);
      xeastCoord.setCoordinateSystem(COORD_SYS_XEAST);

      reverseTPOffsetRotate_(xeastCoord);

      switch (outSystem)
      {
      case COORD_SYS_NED:
      case COORD_SYS_NWU:
      case COORD_SYS_ENU:
      case COORD_SYS_ECI:
        {
          Coordinate ecefCoord;
          if (convertXEastToEcef_(xeastCoord, ecefCoord) != 0)
            return 1;
          switch (outSystem)
          {
          case COORD_SYS_NED:
          case COORD_SYS_NWU:
          case COORD_SYS_ENU:
            if (convertEcefToFlat_(ecefCoord, outCoord, outSystem) != 0)
              return 1;
            break;
          case COORD_SYS_ECI:
            CoordinateConverter::convertEcefToEci(ecefCoord, outCoord, inCoord.elapsedEciTime());
            break;
          default:
            assert(0);
            return 1;
            break;
          }
          break;
        }

      case COORD_SYS_LLA:
        if (convertXEastToGeodetic_(xeastCoord, outCoord) != 0)
          return 1;
        break;

      case COORD_SYS_ECEF:
        if (convertXEastToEcef_(xeastCoord, outCoord) != 0)
          return 1;
        break;

      case COORD_SYS_XEAST:
        outCoord.setPosition(xeastCoord.position());

        if (xeastCoord.hasVelocity())
        {
          outCoord.setVelocity(xeastCoord.velocity());
        }

        if (xeastCoord.hasAcceleration())
        {
          outCoord.setAcceleration(xeastCoord.acceleration());
        }

        if (xeastCoord.hasOrientation())
        {
          outCoord.setOrientation(xeastCoord.orientation());
        }
        break;

      default:
        assert(0);
        return 1;
        break;
      }
      break;
    }

  case COORD_SYS_ECI:
    {
      // convert ECI to ECEF, then apply coordinate transformations based on an ECEF system
      Coordinate ecefCoord;
      CoordinateConverter::convertEciToEcef(inCoord, ecefCoord);

      switch (outSystem)
      {
      case COORD_SYS_NED:
      case COORD_SYS_NWU:
      case COORD_SYS_ENU:
        if (convertEcefToFlat_(ecefCoord, outCoord, outSystem) != 0)
          return 1;
        break;

      case COORD_SYS_LLA:
        CoordinateConverter::convertEcefToGeodetic(ecefCoord, outCoord);
        break;

      case COORD_SYS_ECEF:
        outCoord.setPosition(ecefCoord.position());

        if (ecefCoord.hasVelocity())
        {
          outCoord.setVelocity(ecefCoord.velocity());
        }

        if (ecefCoord.hasAcceleration())
        {
          outCoord.setAcceleration(ecefCoord.acceleration());
        }

        if (ecefCoord.hasOrientation())
        {
          outCoord.setOrientation(ecefCoord.orientation());
        }
        break;

      case COORD_SYS_XEAST:
      case COORD_SYS_GTP:
        if (convertEcefToXEast_(ecefCoord, outCoord) != 0)
          return 1;
        if (outSystem == COORD_SYS_GTP)
        {
          applyTPOffsetRotate_(outCoord);
        }
        break;

      default:
        assert(0);
        return 1;
        break;
      }
      break;
    }

  default:
    assert(0);
    return 1;
    break;
  }
  // Note: Some of the transformations change the coordinate system
  // This is most true of GTP conversions, that rely on XEast functions
  // So here, we reset the outgoing coordinate system
  outCoord.setCoordinateSystem(outSystem);
  return 0;
}

/// convert geodetic projection (LLA) to flat earth projection (NED/NWU/ENU)
///@pre flatCoord valid, ref origin set, in coord is LLA, system is NED/NWU/ENU, llaCoord does not alias flatCoord
int CoordinateConverter::convertGeodeticToFlat_(const Coordinate &llaCoord, Coordinate &flatCoord, CoordinateSystem system) const
{
  // Test for same input/output -- this function cannot handle case of llaCoord == flatCoord
  if (&llaCoord == &flatCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertGeodeticToFlat, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (llaCoord.coordinateSystem() != COORD_SYS_LLA)
  {
    SIM_ERROR << "convertGeodeticToFlat, input system is not LLA: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (!(system == COORD_SYS_NED || system == COORD_SYS_NWU || system == COORD_SYS_ENU))
  {
    SIM_ERROR << "convertGeodeticToFlat, invalid local coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (refOriginStatus_ == REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE)
  {
    SIM_ERROR << "convertGeodeticToFlat, degenerate reference origin at/near pole: " << __LINE__ << std::endl;
    return 1;
  }

  // clear any existing data from the output coordinate
  flatCoord.clear();

  // set coordinate system and preserve ECI time
  flatCoord.setCoordinateSystem(system);
  flatCoord.setElapsedEciTime(llaCoord.elapsedEciTime());

  // Euler angles are the same convention no matter the local system
  if (llaCoord.hasOrientation())
  {
    flatCoord.setOrientation(llaCoord.orientation());
  }

  // input lat and lon in radians, alt in meters, output values in meters
  if (system == COORD_SYS_NED)
  {
    Vec3 llaPos(llaCoord.position());
    //  (North East Down system)
    // +X is North, Latitude is North-South
    double x = angFixPI2(llaPos.lat() - referenceOrigin_.lat()) * latRadius_;
    // +Y is East, Longitude is East-West
    double y = angFixPI(llaPos.lon() - referenceOrigin_.lon()) * lonRadius_;
    // +Z is down, Altitude (+Z) is up
    double z = -(llaPos.alt() - referenceOrigin_.alt());

    flatCoord.setPosition(x, y, z);

    // Geodetic system aligned with Earth so that it follows the
    // ENU convention, hence we must flip X & Y and negate Z
    if (llaCoord.hasVelocity())
    {
      Vec3 velVec;
      CoordinateConverter::swapNedEnu(llaCoord.velocity(), velVec);
      flatCoord.setVelocity(velVec);
    }

    if (llaCoord.hasAcceleration())
    {
      Vec3 accVec;
      CoordinateConverter::swapNedEnu(llaCoord.acceleration(), accVec);
      flatCoord.setAcceleration(accVec);
    }
  }
  else if (system == COORD_SYS_ENU)
  {
    Vec3 llaPos(llaCoord.position());
    // meters (East North Up system)
    // +X is East, Longitude is East-West
    double x = angFixPI(llaPos.lon() - referenceOrigin_.lon()) * lonRadius_;
    // +Y is North, Latitude is North-South
    double y = angFixPI2(llaPos.lat() - referenceOrigin_.lat()) * latRadius_;
    // +Z is up, Altitude (+Z) is up
    double z =  llaPos.alt() - referenceOrigin_.alt();

    flatCoord.setPosition(x, y, z);

    // A Geodetic system is aligned with the Earth such that it follows the
    // ENU convention, hence same systems
    if (llaCoord.hasVelocity())
    {
      flatCoord.setVelocity(llaCoord.velocity());
    }
    if (llaCoord.hasAcceleration())
    {
      flatCoord.setAcceleration(llaCoord.acceleration());
    }
  }
  else if (system == COORD_SYS_NWU)
  {
    Vec3 llaPos(llaCoord.position());
    // meters (North West Up system)
    // +X is North, Latitude is North-South
    double x =  angFixPI2(llaPos.lat() - referenceOrigin_.lat()) * latRadius_;
    // +Y is West, Longitude is East-West
    double y = -angFixPI(llaPos.lon() - referenceOrigin_.lon()) * lonRadius_;
    // +Z is up, Altitude (+Z) is up
    double z =   llaPos.alt() - referenceOrigin_.alt();

    flatCoord.setPosition(x, y, z);

    // A Geodetic system is aligned with the Earth such that it follows the
    // ENU convention, hence in addition to flipping X & Y, we need to negate Y
    if (llaCoord.hasVelocity())
    {
      Vec3 velVec;
      CoordinateConverter::convertEnuToNwu(llaCoord.velocity(), velVec);
      flatCoord.setVelocity(velVec);
    }

    if (llaCoord.hasAcceleration())
    {
      Vec3 accVec;
      CoordinateConverter::convertEnuToNwu(llaCoord.acceleration(), accVec);
      flatCoord.setAcceleration(accVec);
    }
  }
  return 0;
}

/// convert flat earth (NED/NWU/ENU) projection to geodetic (LLA) projection
///@param[in ] flatCoord input coordinate
///@param[out] llaCoord output
///@return 0 on success, !0 on failure
///@pre llaCoord valid, ref origin calculated, in coord is NED/NWU/ENU, llaCoord does not alias flatCoord
int CoordinateConverter::convertFlatToGeodetic_(const Coordinate &flatCoord, Coordinate &llaCoord) const
{
  // Test for same input/output -- this function cannot handle case of llaCoord == flatCoord
  if (&llaCoord == &flatCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertFlatToGeodetic, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (!(flatCoord.coordinateSystem() == COORD_SYS_NED || flatCoord.coordinateSystem() == COORD_SYS_NWU || flatCoord.coordinateSystem() == COORD_SYS_ENU))
  {
    SIM_ERROR << "convertFlatToGeodetic, invalid local coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (refOriginStatus_ == REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE)
  {
    SIM_ERROR << "convertFlatToGeodetic, degenerate reference origin at/near pole: " << __LINE__ << std::endl;
    return 1;
  }

  // clear any existing data from output coordinate
  llaCoord.clear();

  // set coordinate system and preserve ECI time
  llaCoord.setCoordinateSystem(COORD_SYS_LLA);
  llaCoord.setElapsedEciTime(flatCoord.elapsedEciTime());

  // Euler angles are the same convention no matter the local system
  if (flatCoord.hasOrientation())
  {
    llaCoord.setOrientation(flatCoord.orientation());
  }

  // input values in meters, output lat and lon in radians, alt in meters
  if (flatCoord.coordinateSystem() == COORD_SYS_NED)
  {
    Vec3 nedPos(flatCoord.position());
    // meters (North East Down system)
    // +X is North, Latitude is North-South
    double lat =  nedPos.x() * invLatRadius_ + referenceOrigin_.lat();
    // +Y is East, Longitude is East-West
    double lon =  nedPos.y() * invLonRadius_ + referenceOrigin_.lon();
    // +Z is down, Altitude (+Z) is up
    double alt = -nedPos.z() + referenceOrigin_.alt();

    llaCoord.setPositionLLA(lat, lon, alt);

    // A Geodetic system is aligned with the Earth such that it follows the
    // ENU convention, hence we must flip X & Y and negate Z
    if (flatCoord.hasVelocity())
    {
      Vec3 llaVel;
      CoordinateConverter::swapNedEnu(flatCoord.velocity(), llaVel);
      llaCoord.setVelocity(llaVel);
    }

    if (flatCoord.hasAcceleration())
    {
      Vec3 llaAcc;
      CoordinateConverter::swapNedEnu(flatCoord.acceleration(), llaAcc);
      llaCoord.setAcceleration(llaAcc);
    }
  }
  else if (flatCoord.coordinateSystem() == COORD_SYS_ENU)
  {
    Vec3 enuPos(flatCoord.position());
    // meters (East North Up system)
    // +X is East, Longitude is East-West
    double lat = enuPos.y() * invLatRadius_ + referenceOrigin_.lat();
    // +Y is North, Latitude is North-South
    double lon = enuPos.x() * invLonRadius_ + referenceOrigin_.lon();
    // +Z is up, Altitude (+Z) is up
    double alt = enuPos.z() + referenceOrigin_.alt();

    llaCoord.setPositionLLA(lat, lon, alt);

    // A Geodetic system is aligned with the Earth such that it follows the
    // ENU convention, hence the same orientation
    if (flatCoord.hasVelocity())
    {
      llaCoord.setVelocity(flatCoord.velocity());
    }

    if (flatCoord.hasAcceleration())
    {
      llaCoord.setAcceleration(flatCoord.acceleration());
    }
  }
  else if (flatCoord.coordinateSystem() == COORD_SYS_NWU)
  {
    Vec3 nwuPos(flatCoord.position());
    // meters (North West Up system)
    // +X is North, Latitude is North-South
    double lat =  nwuPos.x() * invLatRadius_ + referenceOrigin_.x();
    // +Y is West, Longitude is East-West
    double lon = -nwuPos.y() * invLonRadius_ + referenceOrigin_.y();
    // +Z is up, Altitude (+Z) is up
    double alt =  nwuPos.z() + referenceOrigin_.z();

    llaCoord.setPositionLLA(lat, lon, alt);

    // A Geodetic system is aligned with the Earth such that it follows the
    // ENU convention, hence in addition to flipping X & Y, we need to negate Y
    if (flatCoord.hasVelocity())
    {
      Vec3 llaVel;
      CoordinateConverter::convertNwuToEnu(flatCoord.velocity(), llaVel);
      llaCoord.setVelocity(llaVel);
    }

    if (flatCoord.hasAcceleration())
    {
      Vec3 llaAcc;
      CoordinateConverter::convertNwuToEnu(flatCoord.acceleration(), llaAcc);
      llaCoord.setAcceleration(llaAcc);
    }
  }
  return 0;
}

/// convert earth centered, earth fixed projection to flat earth projection
///@note assert/clear/prop assignments are handled by the conversion functions called
int CoordinateConverter::convertEcefToFlat_(const Coordinate &ecefCoord, Coordinate &flatCoord, CoordinateSystem system) const
{
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertEcefToFlat, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (ecefCoord.coordinateSystem() != COORD_SYS_ECEF)
  {
    SIM_ERROR << "convertEcefToFlat, input system is not ECEF: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (!(system == COORD_SYS_NED || system == COORD_SYS_NWU || system == COORD_SYS_ENU))
  {
    SIM_ERROR << "convertEcefToFlat, invalid local coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (refOriginStatus_ == REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE)
  {
    SIM_ERROR << "convertEcefToFlat, degenerate reference origin at/near pole: " << __LINE__ << std::endl;
    return 1;
  }

  Coordinate llaCoord;
  CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord);

  // convert from geodetic lat, lon, alt to Flat Earth Topographic
  // (x,y,z) in meters
  return convertGeodeticToFlat_(llaCoord, flatCoord, system);
}

/// convert flat earth projection to earth centered, earth fixed projection
///@note assert/clear/prop assignments are handled by the conversion functions called
int CoordinateConverter::convertFlatToEcef_(const Coordinate &flatCoord, Coordinate &ecefCoord) const
{
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertFlatToEcef, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (!(flatCoord.coordinateSystem() == COORD_SYS_NED || flatCoord.coordinateSystem() == COORD_SYS_NWU || flatCoord.coordinateSystem() == COORD_SYS_ENU))
  {
    SIM_ERROR << "convertFlatToEcef, invalid local coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (refOriginStatus_ == REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE)
  {
    SIM_ERROR << "convertFlatToEcef, degenerate reference origin at/near pole: " << __LINE__ << std::endl;
    return 1;
  }

  // calculate lat and lon of input topo position
  Coordinate llaCoord;
  convertFlatToGeodetic_(flatCoord, llaCoord);

  // convert lat, lon, alt to ECEF geocentric using WGS84 ellipsoidal
  // earth model
  CoordinateConverter::convertGeodeticToEcef(llaCoord, ecefCoord);
  return 0;
}

/// convert tangent plane projection to earth centered, earth fixed projection
///@pre ecefCoord valid, ref origin calculated, in coord is XEAST, tpCoord does not alias ecefCoord
int CoordinateConverter::convertXEastToEcef_(const Coordinate &tpCoord, Coordinate &ecefCoord) const
{
  // Test for same input/output -- this function cannot handle case of tpCoord == ecefCoord
  if (&tpCoord == &ecefCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertXEastToEcef, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (tpCoord.coordinateSystem() != COORD_SYS_XEAST)
  {
    SIM_ERROR << "convertXEastToEcef, input system is not XEAST: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }

  ecefCoord.clear();

  // set coordinate system and preserve ECI time
  ecefCoord.setCoordinateSystem(COORD_SYS_ECEF);
  ecefCoord.setElapsedEciTime(tpCoord.elapsedEciTime());

  Vec3 pos;
  // rotate to geocentric direction
  d3MTv3Mult(rotationMatrixENU_, tpCoord.position(), pos);

  // apply translation to earth center origin
  Vec3 ecefPos;
  v3Add(pos, tangentPlaneTranslation_, ecefPos);
  ecefCoord.setPosition(ecefPos);

  if (tpCoord.hasVelocity())
  {
    Vec3 ecefVel;
    d3MTv3Mult(rotationMatrixENU_, tpCoord.velocity(), ecefVel);
    ecefCoord.setVelocity(ecefVel);
  }

  if (tpCoord.hasAcceleration())
  {
    Vec3 ecefAcc;
    d3MTv3Mult(rotationMatrixENU_, tpCoord.acceleration(), ecefAcc);
    ecefCoord.setAcceleration(ecefAcc);
  }

  if (tpCoord.hasOrientation())
  {
    // calculate Body to Local rotation matrix using Local Eulers
    double BL[3][3];
    d3EulertoDCM(tpCoord.orientation(), BL);

    // calculate Body to Earth rotation matrix fBE
    double BE[3][3];
    d3MMmult(BL, rotationMatrixNED_, BE);

    // calculate Euler angles for platform body in ECEF coordinates
    Vec3 ecefEul;
    d3DCMtoEuler(BE, ecefEul);
    ecefCoord.setOrientation(ecefEul);
  }
  return 0;
}

/// convert earth centered, earth fixed projection to tangent plane projection
///@pre tpCoord valid, ref origin calculated, in coord ECEF, tpCoord does not alias ecefCoord
int CoordinateConverter::convertEcefToXEast_(const Coordinate &ecefCoord, Coordinate &tpCoord) const
{
  // Test for same input/output -- this function cannot handle case of tpCoord == ecefCoord
  if (&tpCoord == &ecefCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertEcefToXEast, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (ecefCoord.coordinateSystem() != COORD_SYS_ECEF)
  {
    SIM_ERROR << "convertEcefToXEast, input system is not ECEF: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }

  tpCoord.clear();

  // set coordinate system and preserve ECI time
  tpCoord.setCoordinateSystem(COORD_SYS_XEAST);
  tpCoord.setElapsedEciTime(ecefCoord.elapsedEciTime());

  Vec3 pos;
  // apply translation to tangent plane origin
  v3Subtract(ecefCoord.position(), tangentPlaneTranslation_, pos);

  // rotate to X-East
  Vec3 tpPos;
  d3Mv3Mult(rotationMatrixENU_, pos, tpPos);
  tpCoord.setPosition(tpPos);

  if (ecefCoord.hasVelocity())
  {
    Vec3 tpVel;
    d3Mv3Mult(rotationMatrixENU_, ecefCoord.velocity(), tpVel);
    tpCoord.setVelocity(tpVel);
  }

  if (ecefCoord.hasAcceleration())
  {
    Vec3 tpAcc;
    d3Mv3Mult(rotationMatrixENU_, ecefCoord.acceleration(), tpAcc);
    tpCoord.setAcceleration(tpAcc);
  }

  if (ecefCoord.hasOrientation())
  {
    // create Body to Earth rotation matrix
    double BE[3][3];
    d3EulertoDCM(ecefCoord.orientation(), BE);

    // multiply BE * (LE) transpose = BL (Body to Local Topo rotation matrix)
    double BL[3][3];
    d3MMTmult(BE, rotationMatrixNED_, BL);

    // get local Eulers
    Vec3 tpEul;
    d3DCMtoEuler(BL, tpEul);
    tpCoord.setOrientation(tpEul);
  }
  return 0;
}

/// convert from lla -> ecef -> xeast
///@note assert/clear/prop assignments handled by conversion functions called
int CoordinateConverter::convertGeodeticToXEast_(const Coordinate &llaCoord, Coordinate &tpCoord) const
{
  // Test for same input/output -- this function cannot handle case of tpCoord == llaCoord
  if (&tpCoord == &llaCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertGeodeticToXEast, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (llaCoord.coordinateSystem() != COORD_SYS_LLA)
  {
    SIM_ERROR << "convertGeodeticToXEast, input system is not LLA: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }

  // clear orientation from llaCoord copy; computation for lla -> ecef -> x-east is not necessary
  Coordinate llaCoordNoOri(llaCoord);
  llaCoordNoOri.clearOrientation();

  // convert to ECEF geocentric using WGS84 ellipsoidal earth model from
  // lat(rad), lon(rad) & alt(m)
  Coordinate ecefCoord;
  CoordinateConverter::convertGeodeticToEcef(llaCoordNoOri, ecefCoord);

  // convert from ECEF geocentric x, y, z to x east tangent plane
  // (x,y,z) in meters
  convertEcefToXEast_(ecefCoord, tpCoord);

  // Eulers remain unchanged
  if (llaCoord.hasOrientation())
  {
    tpCoord.setOrientation(llaCoord.orientation());
  }
  return 0;
}

/// convert from xeast -> ecef -> lla
///@note assert/clear/prop assignments are handled by the conversion functions called
int CoordinateConverter::convertXEastToGeodetic_(const Coordinate &tpCoord, Coordinate &llaCoord) const
{
  // Test for same input/output -- this function cannot handle case of tpCoord == llaCoord
  if (&tpCoord == &llaCoord)
  {
    assert(0);
    return 1;
  }
  // make sure earth radius has been set before flat earth conversion
  if (!hasReferenceOrigin())
  {
    SIM_ERROR << "convertXEastToGeodetic, reference origin not set: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }
  if (tpCoord.coordinateSystem() != COORD_SYS_XEAST)
  {
    SIM_ERROR << "convertXEastToGeodetic, input system is not XEAST: " << __LINE__ << std::endl;
    assert(0);
    return 1;
  }

  // clear orientation from tpCoord copy; computation for x-east -> ecef -> lla is not necessary
  Coordinate tpCoordNoOri(tpCoord);
  tpCoordNoOri.clearOrientation();

  // convert to ECEF geocentric using WGS84 ellipsoidal earth model from
  // x east tangent plane
  Coordinate ecefCoord;
  convertXEastToEcef_(tpCoordNoOri, ecefCoord);

  // convert from ECEF geocentric x, y, z to geodetic lat(rad), lon(rad), alt(m)
  CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord);

  // Eulers remain unchanged
  if (tpCoord.hasOrientation())
  {
    llaCoord.setOrientation(tpCoord.orientation());
  }
  return 0;
}

// state independent CoordinateConverter members
//--------------------------------------------------------------------------
///@pre system is NED/NWU/ENU
void CoordinateConverter::setLocalToEarthMatrix(double lat, double lon, const LocalLevelFrame localLevelFrame, double localToEarth[][3])
{
  double slat = sin(lat);
  double clat = cos(lat);
  double slon = sin(lon);
  double clon = cos(lon);

  // Compute local to Earth rotation matrix based on input coordinate system
  switch (localLevelFrame)
  {
  case LOCAL_LEVEL_FRAME_NED:
    {
      // NED local level frame relative to ECEF
      //
      // local x unit vector
      localToEarth[0][0] = -slat * clon;
      localToEarth[0][1] = -slat * slon;
      localToEarth[0][2] =  clat;

      // local y unit vector
      localToEarth[1][0] = -slon;
      localToEarth[1][1] =  clon;
      localToEarth[1][2] =  0.0;

      // local z unit vector
      localToEarth[2][0] = -clat * clon;
      localToEarth[2][1] = -clat * slon;
      localToEarth[2][2] = -slat;
    }
    break;

  case LOCAL_LEVEL_FRAME_NWU:
    {
      // NWU local level frame relative to ECEF
      //
      // local x unit vector
      localToEarth[0][0] = -slat * clon;
      localToEarth[0][1] = -slat * slon;
      localToEarth[0][2] =  clat;

      // local y unit vector
      localToEarth[1][0] =  slon;
      localToEarth[1][1] = -clon;
      localToEarth[1][2] =  0.0;

      // local z unit vector
      localToEarth[2][0] = clat * clon;
      localToEarth[2][1] = clat * slon;
      localToEarth[2][2] = slat;
    }
    break;

  case LOCAL_LEVEL_FRAME_ENU:
  default:
    {
      // ENU local level frame relative to ECEF
      //
      // local x unit vector
      localToEarth[0][0] = -slon;
      localToEarth[0][1] =  clon;
      localToEarth[0][2] =  0.0;

      // local y unit vector
      localToEarth[1][0] = -slat * clon;
      localToEarth[1][1] = -slat * slon;
      localToEarth[1][2] =  clat;

      // local z unit vector
      localToEarth[2][0] = clat * clon;
      localToEarth[2][1] = clat * slon;
      localToEarth[2][2] = slat;
    }
    break;
  }
}

/// convert vector between ENU and NED
void CoordinateConverter::swapNedEnu(const Vec3 &inVec, Vec3 &outVec)
{
  outVec.set(inVec.y(), inVec.x(), -inVec.z());
}

/// convert coordinates between ENU and NED
///@pre outCoord valid, inCoord system is NED or ENU, inCoord does not alias outCoord
void CoordinateConverter::swapNedEnu(const Coordinate &inCoord, Coordinate &outCoord)
{
  // Test for same input/output -- this function cannot handle case of inCoord == outCoord
  if (&inCoord == &outCoord)
  {
    assert(0);
    return;
  }
  assert((inCoord.coordinateSystem() == COORD_SYS_NED || inCoord.coordinateSystem() == COORD_SYS_ENU));
  if (!(inCoord.coordinateSystem() == COORD_SYS_NED || inCoord.coordinateSystem() == COORD_SYS_ENU))
  {
    SIM_ERROR << "swapNedEnu, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // clear any existing data from the output coordinate
  outCoord.clear();
  outCoord.setCoordinateSystem(inCoord.coordinateSystem() == COORD_SYS_NED ? COORD_SYS_ENU : COORD_SYS_NED);

  // preserve elapsed ECI time
  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  Vec3 outPos;
  CoordinateConverter::swapNedEnu(inCoord.position(), outPos);
  outCoord.setPosition(outPos);

  if (inCoord.hasVelocity())
  {
    Vec3 outVel;
    CoordinateConverter::swapNedEnu(inCoord.velocity(), outVel);
    outCoord.setVelocity(outVel);
  }

  if (inCoord.hasAcceleration())
  {
    Vec3 outAcc;
    CoordinateConverter::swapNedEnu(inCoord.acceleration(), outAcc);
    outCoord.setAcceleration(outAcc);
  }

  if (inCoord.hasOrientation())
  {
    outCoord.setOrientation(inCoord.orientation());
  }
}

/// convert vector between NED and NWU
void CoordinateConverter::swapNedNwu(const Vec3 &inVec, Vec3 &outVec)
{
  outVec.set(inVec.x(), -inVec.y(), -inVec.z());
}

/// convert coordinate between NED and NWU
///@pre outCoord valid, in coord is NED/NWU, inCoord does not alias outCoord
void CoordinateConverter::swapNedNwu(const Coordinate &inCoord, Coordinate &outCoord)
{
  // Test for same input/output -- this function cannot handle case of inCoord == outCoord
  if (&inCoord == &outCoord)
  {
    assert(0);
    return;
  }
  assert((inCoord.coordinateSystem() == COORD_SYS_NED || inCoord.coordinateSystem() == COORD_SYS_NWU));
  if (!(inCoord.coordinateSystem() == COORD_SYS_NED || inCoord.coordinateSystem() == COORD_SYS_NWU))
  {
    SIM_ERROR << "swapNedNwu, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // clear any existing data from the output coordinate
  outCoord.clear();
  outCoord.setCoordinateSystem(inCoord.coordinateSystem() == COORD_SYS_NED ? COORD_SYS_NWU : COORD_SYS_NED);

  // preserve elapsed ECI time
  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  Vec3 outPos;
  CoordinateConverter::swapNedNwu(inCoord.position(), outPos);
  outCoord.setPosition(outPos);

  if (inCoord.hasVelocity())
  {
    Vec3 outVel;
    CoordinateConverter::swapNedNwu(inCoord.velocity(), outVel);
    outCoord.setVelocity(outVel);
  }

  if (inCoord.hasAcceleration())
  {
    Vec3 outAcc;
    CoordinateConverter::swapNedNwu(inCoord.acceleration(), outAcc);
    outCoord.setAcceleration(outAcc);
  }

  if (inCoord.hasOrientation())
  {
    outCoord.setOrientation(inCoord.orientation());
  }
}

/// convert vector from ENU to NWU
void CoordinateConverter::convertEnuToNwu(const Vec3 &inVec, Vec3 &outVec)
{
  outVec.set(inVec.y(), -inVec.x(), inVec.z());
}

/// convert coordinate from ENU to NWU
///@pre outCoord valid, in coord is ENU, inCoord does not alias outCoord
void CoordinateConverter::convertEnuToNwu(const Coordinate &inCoord, Coordinate &outCoord)
{
  // Test for same input/output -- this function cannot handle case of inCoord == outCoord
  if (&inCoord == &outCoord)
  {
    assert(0);
    return;
  }
  assert(inCoord.coordinateSystem() == COORD_SYS_ENU);
  if (inCoord.coordinateSystem() != COORD_SYS_ENU)
  {
    SIM_ERROR << "convertEnuToNwu, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // clear any existing data from the output coordinate
  outCoord.clear();
  outCoord.setCoordinateSystem(COORD_SYS_NWU);

  // preserve elapsed ECI time
  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  Vec3 outPos;
  CoordinateConverter::convertEnuToNwu(inCoord.position(), outPos);
  outCoord.setPosition(outPos);

  if (inCoord.hasVelocity())
  {
    Vec3 outVel;
    CoordinateConverter::convertEnuToNwu(inCoord.velocity(), outVel);
    outCoord.setVelocity(outVel);
  }

  if (inCoord.hasAcceleration())
  {
    Vec3 outAcc;
    CoordinateConverter::convertEnuToNwu(inCoord.acceleration(), outAcc);
    outCoord.setAcceleration(outAcc);
  }

  if (inCoord.hasOrientation())
  {
    outCoord.setOrientation(inCoord.orientation());
  }
}

/// convert vector from NWU to ENU
void CoordinateConverter::convertNwuToEnu(const Vec3 &inVec, Vec3 &outVec)
{
  outVec.set(-inVec.y(), inVec.x(), inVec.z());
}

/// convert coordinate from NWU to ENU
///@pre outCoord valid, in coord is NWU, inCoord does not alias outCoord
void CoordinateConverter::convertNwuToEnu(const Coordinate &inCoord, Coordinate &outCoord)
{
  // Test for same input/output -- this function cannot handle case of inCoord == outCoord
  if (&inCoord == &outCoord)
  {
    assert(0);
    return;
  }
  assert(inCoord.coordinateSystem() == COORD_SYS_NWU);
  if (inCoord.coordinateSystem() != COORD_SYS_NWU)
  {
    SIM_ERROR << "convertNwuToEnu, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // clear any existing data from the output coordinate
  outCoord.clear();
  outCoord.setCoordinateSystem(COORD_SYS_ENU);

  // preserve elapsed ECI time
  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  Vec3 outPos;
  CoordinateConverter::convertNwuToEnu(inCoord.position(), outPos);
  outCoord.setPosition(outPos);

  if (inCoord.hasVelocity())
  {
    Vec3 outVel;
    CoordinateConverter::convertNwuToEnu(inCoord.velocity(), outVel);
    outCoord.setVelocity(outVel);
  }

  if (inCoord.hasAcceleration())
  {
    Vec3 outAcc;
    CoordinateConverter::convertNwuToEnu(inCoord.acceleration(), outAcc);
    outCoord.setAcceleration(outAcc);
  }

  if (inCoord.hasOrientation())
  {
    outCoord.setOrientation(inCoord.orientation());
  }
}

/// convert geodetic projection (LLA) to earth centered, earth fixed projection
///@pre ecefCoord valid, in coord is LLA, llaCoord does not alias ecefCoord
void CoordinateConverter::convertGeodeticToEcef(const Coordinate &llaCoord, Coordinate &ecefCoord, LocalLevelFrame localLevelFrame)
{
  // Test for same input/output -- this function cannot handle case of llaCoord == ecefCoord
  if (&llaCoord == &ecefCoord)
  {
    assert(0);
    return;
  }
  if (llaCoord.coordinateSystem() != COORD_SYS_LLA)
  {
    SIM_ERROR << "convertGeodeticToEcef, invalid coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return;
  }

  // clear any existing data from the output coordinate
  ecefCoord.clear();
  ecefCoord.setCoordinateSystem(COORD_SYS_ECEF);

  // preserve elapsed ECI time
  ecefCoord.setElapsedEciTime(llaCoord.elapsedEciTime());

  // convert lat, lon, alt to ECEF geocentric using WGS84 ellipsoidal earth model
  Vec3 ecefPos;
  CoordinateConverter::convertGeodeticPosToEcef(llaCoord.position(), ecefPos);
  ecefCoord.setPosition(ecefPos);

  // calculate Local To Earth rotation matrix at lat, lon position of input platform
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the platform)
  //
  // LE is referenced to a NED system (geodetic)
  double LE[3][3];
  if (llaCoord.hasOrientation() || llaCoord.hasVelocity() || llaCoord.hasAcceleration())
  {
    const Vec3 llaPos(llaCoord.position());
    CoordinateConverter::setLocalToEarthMatrix(llaPos[0], llaPos[1], localLevelFrame, LE);
  }

  // convert topo Euler from Flat Earth Local coordinates to ECEF coordinates
  if (llaCoord.hasOrientation())
  {
    double BL[3][3];
    double BE[3][3];

    // calculate Body to Local rotation matrix using Local Eulers
    d3EulertoDCM(llaCoord.orientation(), BL);

    // calculate Body to Earth rotation matrix fBE
    d3MMmult(BL, LE, BE);

    // calculate Euler angles for platform body in ECEF coordinates
    Vec3 ecefOri;
    d3DCMtoEuler(BE, ecefOri);
    ecefCoord.setOrientation(ecefOri);
  }

  if (llaCoord.hasVelocity() || llaCoord.hasAcceleration())
  {
    // convert topo velocity vector from Flat Earth Local coordinates to ECEF coordinates
    //
    // (note that as with orientations, the transformation to ECEF is done using a tangent plane
    // at the lat, lon of the platform)
    Vec3 nedVec;

    if (llaCoord.hasVelocity())
    {
      if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
      {
        CoordinateConverter::swapNedEnu(llaCoord.velocity(), nedVec);
      }
      else if (localLevelFrame == LOCAL_LEVEL_FRAME_NWU)
      {
        CoordinateConverter::convertEnuToNwu(llaCoord.velocity(), nedVec);
      }
      else //(localLevelFrame == LOCAL_LEVEL_FRAME_ENU)
      {
        nedVec = llaCoord.velocity();
      }

      Vec3 ecefVel;
      d3MTv3Mult(LE, nedVec, ecefVel);
      ecefCoord.setVelocity(ecefVel);
    }

    // convert topo acceleration vector from Flat Earth Local coordinates to ECEF coordinates
    //
    // (note that as with orientations, the transformation to ECEF is done using a tangent plane
    // at the lat, lon of the platform)
    if (llaCoord.hasAcceleration())
    {
      if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
      {
        CoordinateConverter::swapNedEnu(llaCoord.acceleration(), nedVec);
      }
      else if (localLevelFrame == LOCAL_LEVEL_FRAME_NWU)
      {
        CoordinateConverter::convertEnuToNwu(llaCoord.acceleration(), nedVec);
      }
      else //(localLevelFrame == LOCAL_LEVEL_FRAME_ENU)
      {
        nedVec = llaCoord.acceleration();
      }

      Vec3 ecefAcc;
      d3MTv3Mult(LE, nedVec, ecefAcc);
      ecefCoord.setAcceleration(ecefAcc);
    }
  }
}

/// convert earth centered, earth fixed projection to geodetic projection
void CoordinateConverter::convertEcefToGeodetic(const Coordinate &ecefCoord, Coordinate &llaCoord, LocalLevelFrame localLevelFrame)
{
  // Test for same input/output -- this function cannot handle case of llaCoord == ecefCoord
  if (&llaCoord == &ecefCoord)
  {
    assert(0);
    return;
  }
  // make sure vector is valid before conversion
  if (ecefCoord.coordinateSystem() != COORD_SYS_ECEF)
  {
    SIM_ERROR << "convertEcefToGeodetic, invalid coordinate system: " << __LINE__ << std::endl;
    assert(0);
    return;
  }

  // clear any existing data from the output coordinate
  llaCoord.clear();
  llaCoord.setCoordinateSystem(COORD_SYS_LLA);

  // preserve elapsed ECI time
  llaCoord.setElapsedEciTime(ecefCoord.elapsedEciTime());

  Vec3 llaPos;
  CoordinateConverter::convertEcefToGeodeticPos(ecefCoord.position(), llaPos);
  llaCoord.setPosition(llaPos);

  // calculate Local To Earth rotation matrix at lat, lon position of input platform
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the platform)
  //
  // LE is referenced to a NED system (geodetic)
  double LE[3][3];
  if (ecefCoord.hasOrientation() || ecefCoord.hasVelocity() || ecefCoord.hasAcceleration())
  {
    CoordinateConverter::setLocalToEarthMatrix(llaPos[0], llaPos[1], localLevelFrame, LE);
  }

  if (ecefCoord.hasOrientation())
  {
    double BL[3][3];
    double BE[3][3];

    // create Body to Earth rotation matrix
    d3EulertoDCM(ecefCoord.orientation(), BE);

    // Multiply BE * (LE) transpose = BL (Body to Local Topo rotation matrix)
    d3MMTmult(BE, LE, BL);

    // get local Eulers
    Vec3 llaOri;
    d3DCMtoEuler(BL, llaOri);
    llaCoord.setOrientation(llaOri);
  }

  if (ecefCoord.hasVelocity() || ecefCoord.hasAcceleration())
  {
    Vec3 nedVec;

    // convert ECEF velocity vector to Flat Earth velocity vector
    //
    // (note that as with orientations, the transformation of vectors from ECEF to Flat Earth
    // is done using transformation to a tangent plane at the lat, lon of the platform)
    if (ecefCoord.hasVelocity())
    {
      d3Mv3Mult(LE, ecefCoord.velocity(), nedVec);
      if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
      {
        Vec3 llaVel;
        CoordinateConverter::swapNedEnu(nedVec, llaVel);
        llaCoord.setVelocity(llaVel);
      }
      else if (localLevelFrame == LOCAL_LEVEL_FRAME_NWU)
      {
        Vec3 llaVel;
        CoordinateConverter::convertNwuToEnu(nedVec, llaVel);
        llaCoord.setVelocity(llaVel);
      }
      else //(localLevelFrame == LOCAL_LEVEL_FRAME_ENU)
      {
        llaCoord.setVelocity(nedVec);
      }
    }

    // convert ECEF acceleration vector to Flat Earth acceleration vector
    //
    // (note that as with orientations, the transformation of vectors from ECEF to Flat Earth
    // is done using transformations to a tangent plane at the lat, lon of the platform)
    if (ecefCoord.hasAcceleration())
    {
      d3Mv3Mult(LE, ecefCoord.acceleration(), nedVec);
      if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
      {
        Vec3 llaAcc;
        CoordinateConverter::swapNedEnu(nedVec, llaAcc);
        llaCoord.setAcceleration(llaAcc);
      }
      else if (localLevelFrame == LOCAL_LEVEL_FRAME_NWU)
      {
        Vec3 llaAcc;
        CoordinateConverter::convertNwuToEnu(nedVec, llaAcc);
        llaCoord.setAcceleration(llaAcc);
      }
      else //(localLevelFrame == LOCAL_LEVEL_FRAME_ENU)
      {
        llaCoord.setAcceleration(nedVec);
      }
    }
  }
}

// conversion of ECI->ECEF, or ECEF->ECI is just a rotation about z axis. only the direction of rotation differs.
void CoordinateConverter::convertEciEcef_(const Coordinate &inCoord, Coordinate &outCoord)
{
  assert(inCoord.coordinateSystem() == COORD_SYS_ECI || inCoord.coordinateSystem() == COORD_SYS_ECEF);
  assert(outCoord.coordinateSystem() == COORD_SYS_ECI || outCoord.coordinateSystem() == COORD_SYS_ECEF);
  assert(inCoord.coordinateSystem() != outCoord.coordinateSystem());
  assert(&inCoord != &outCoord);

  outCoord.setElapsedEciTime(inCoord.elapsedEciTime());

  // if converting to eci to ecef, then rotation is negative
  const double rotationRate = (outCoord.coordinateSystem() == COORD_SYS_ECEF) ? -EARTH_ROTATION_RATE : EARTH_ROTATION_RATE;
  // z axis rotation of omega
  const double eciRotation = angFix2PI(rotationRate * inCoord.elapsedEciTime());
  const double cosOmega = cos(eciRotation);
  const double sinOmega = sin(eciRotation);

  const Vec3& inPos(inCoord.position());

  // z component is unchanged in a z-axis rotation
  outCoord.setPosition((cosOmega * inPos.x() - sinOmega * inPos.y()), (cosOmega * inPos.y() + sinOmega * inPos.x()), inPos.z());

  if (inCoord.hasOrientation())
  {
    // create Body to Earth rotation matrix
    double BE[3][3];
    d3EulertoDCM(inCoord.orientation(), BE);

    double BL[3][3];
    // +omega rotation around z axis
    const double zRot[3][3] = {{cosOmega, sinOmega, 0.}, {-sinOmega, cosOmega, 0.}, {0., 0., 1.}};
    d3MMmult(BE, zRot, BL);

    // get local Eulers
    Vec3 outEul;
    d3DCMtoEuler(BL, outEul);
    outCoord.setOrientation(outEul);
  }

  if (inCoord.hasVelocity())
  {
    // compute inertial earth velocity
    const Vec3& inVel(inCoord.velocity());
    const double xVel = inVel.x() - rotationRate * inPos.y();
    const double yVel = inVel.y() + rotationRate * inPos.x();
    // z rotation of velocity vector
    outCoord.setVelocity((xVel * cosOmega - yVel * sinOmega), (yVel * cosOmega + xVel * sinOmega), inVel.z());

    if (inCoord.hasAcceleration())
    {
      // compute inertial earth acceleration
      const Vec3& inAcc(inCoord.acceleration());
      // preserve the sign/direction of the rotation in this square
      const double rotationRate2 = rotationRate * fabs(rotationRate);
      const double xAcc = inAcc.x() - (2.0 * rotationRate * inVel.y()) - (rotationRate2 * inPos.x());
      const double yAcc = inAcc.y() + (2.0 * rotationRate * inVel.x()) - (rotationRate2 * inPos.y());
      // z rotation of acc vector
      outCoord.setAcceleration((xAcc * cosOmega - yAcc * sinOmega), (yAcc * cosOmega + xAcc * sinOmega), inAcc.z());
    }
  }
}

/// convert ECI projection to ECEF projection
///@pre ecefCoord valid, in coord is ECI, eciCoord does not alias ecefCoord
void CoordinateConverter::convertEciToEcef(const Coordinate &eciCoord, Coordinate &ecefCoord)
{
  // Test for same input/output -- this function cannot handle case of eciCoord == ecefCoord
  if (&eciCoord == &ecefCoord)
  {
    assert(0);
    return;
  }
  // check inputs
  assert(eciCoord.coordinateSystem() == COORD_SYS_ECI);
  if (eciCoord.coordinateSystem() != COORD_SYS_ECI)
  {
    SIM_ERROR << "convertEciToEcef, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // Clear any existing data from the output coordinate
  ecefCoord.clear();
  ecefCoord.setCoordinateSystem(COORD_SYS_ECEF);
  // note that you cannot avoid calc when elapsedEciTime is zero, due to ECI velocity & acceleration having earth rotation components
  convertEciEcef_(eciCoord, ecefCoord);
}

/// convert ECEF projection to ECI projection
///@pre eciCoord valid, in coord is ECEF, eciCoord does not alias ecefCoord
void CoordinateConverter::convertEcefToEci(const Coordinate &ecefCoord, Coordinate &eciCoord, double elapsedEciTime)
{
  // Test for same input/output -- this function cannot handle case of eciCoord == ecefCoord
  if (&eciCoord == &ecefCoord)
  {
    assert(0);
    return;
  }
  // check inputs
  if (ecefCoord.coordinateSystem() != COORD_SYS_ECEF)
  {
    SIM_ERROR << "convertEcefToEci, invalid coordinate system: " << __LINE__ << std::endl;
    return;
  }

  // Clear any existing data from the output coordinate
  eciCoord.clear();
  eciCoord.setCoordinateSystem(COORD_SYS_ECI);
  // note that you cannot avoid calc when elapsedEciTime is zero, due to ECI velocity & acceleration having earth rotation components
  convertEciEcef_(ecefCoord, eciCoord);
}

/// convert earth centered, earth fixed projection to geodetic (LLA) projection
///@pre llaPos valid, ecefPos does not alias llaPos
void CoordinateConverter::convertEcefToGeodeticPos(const Vec3 &ecefPos, Vec3 &llaPos)
{
  // Test for same input/output -- this function cannot handle case of ecefPos == llaPos
  if (&ecefPos == &llaPos)
  {
    assert(0);
    return;
  }
  // derived from:
  // 'An Improved Algorithm for Geocentric to Geodetic Coordinate Conversion',
  // by Ralph Toms, February 1996  UCRL-JC-123138
  // Note: Variable names follow the notation used in Toms, Feb 1996

  // indicates location is in polar region
  bool atPole = false;

  if (ecefPos.x() != 0.0)
  {
    llaPos.setLon(atan2(ecefPos.y(), ecefPos.x()));
  }
  else
  {
    if (ecefPos.y() > 0.0)
    {
      llaPos.setLon(M_PI_2);
    }
    else if (ecefPos.y() < 0.0)
    {
      llaPos.setLon(-M_PI_2);
    }
    else
    {
      atPole = true;
      llaPos.setLon(0.0);
      if (ecefPos.z() > 0.0)
      { // north pole
        llaPos.setLat(M_PI_2);
      }
      else if (ecefPos.z() < 0.0)
      { // south pole
        llaPos.setLat(-M_PI_2);
      }
      else
      { // center of earth
        llaPos.setLat(M_PI_2);
        llaPos.setAlt(-WGS_B);
        return; // done
      }
    }
  }

  // square of distance from Z axis
  const double W2 = square(ecefPos.x()) + square(ecefPos.y());
  // distance from Z axis
  const double W = sqrt(W2);
  // initial estimate of vertical component
  // 1.0026000 is Ralph Toms' region 1 constant
  const double T0 = ecefPos.z() * 1.0026000;
  // initial estimate of horizontal component
  const double S0 = sqrt(T0 * T0 + W2);
  // sin(B0), B0 is estimate of Bowring aux variable
  const double Sin_B0 = T0 / S0;
  // cos(B0)
  const double Cos_B0 = W / S0;
  // cube of sin(B0)
  const double Sin3_B0 = Sin_B0 * Sin_B0 * Sin_B0;
  // corrected estimate of vertical component
  const double T1 = ecefPos.z() + WGS_B * WGS_EP2 * Sin3_B0;
  // numerator of cos(phi1)
  const double Sum = W - WGS_A * WGS_ESQ * Cos_B0 * Cos_B0 * Cos_B0;
  // corrected estimate of horizontal component
  const double S1 = sqrt(square(T1) + square(Sum));
  // sin(phi1), phi1 is estimated latitude
  const double Sin_p1 = T1 / S1;
  // cos(phi1)
  const double Cos_p1 = Sum / S1;
  // Earth radius at location
  const double Rn = WGS_A / sqrt(1.0 - WGS_ESQ * Sin_p1 * Sin_p1);

  // cosine of 67.5 degrees
  const double cos_67_5 = 0.3826834323650897717284599840304;

  if (Cos_p1 >= cos_67_5)
  {
    llaPos.setAlt(W / Cos_p1 - Rn);
  }
  else if (Cos_p1 <= -cos_67_5)
  {
    llaPos.setAlt(W / -Cos_p1 - Rn);
  }
  else
  {
    llaPos.setAlt(ecefPos.z() / Sin_p1 + Rn * (WGS_ESQ - 1.0));
  }

  if (!atPole)
  {
    llaPos.setLat(atan(Sin_p1 / Cos_p1));
  }
}

/// convert geodetic projection to earth centered, earth fixed projection
///@pre ecefPos valid
void CoordinateConverter::convertGeodeticPosToEcef(const Vec3 &llaPos, Vec3 &ecefPos, const double semiMajor, const double eccentricitySquared)
{
  // convert lat, lon, alt to ECEF geocentric using WGS84 ellipsoidal earth model
  const double sLat = sin(llaPos.lat());
  const double Rn = semiMajor / sqrt(1.0 - eccentricitySquared * square(sLat));
  const double cLat = cos(llaPos.lat());

  ecefPos.set((Rn + llaPos.alt()) * cLat * cos(llaPos.lon()),
    (Rn + llaPos.alt()) * cLat * sin(llaPos.lon()),
    (Rn * (1.0-eccentricitySquared) + llaPos.alt()) * sLat);
}

/// Converts an Earth Centered Earth Fixed (ECEF) velocity to geodetic
void CoordinateConverter::convertEcefToGeodeticVel(const Vec3 &llaPos, const Vec3 &ecefVel, Vec3 &llaVel, LocalLevelFrame localLevelFrame)
{
  // calculate Local To Earth rotation matrix at lat, lon position of input platform
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the platform)
  //
  // LE is referenced to a NED system (geodetic)
  double LE[3][3];
  CoordinateConverter::setLocalToEarthMatrix(llaPos.lat(), llaPos.lon(), localLevelFrame, LE);

  // convert ECEF velocity vector to Flat Earth velocity vector
  //
  // (note that as with orientations, the transformation of vectors from ECEF to Flat Earth
  // is done using transformation to a tangent plane at the lat, lon of the platform)
  Vec3 nedVector;
  d3Mv3Mult(LE, ecefVel, nedVector);

  if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
  {
    CoordinateConverter::swapNedEnu(nedVector, llaVel);
  }
}

/// Convert Earth Centered Earth Fixed (ECEF) orientation to geodetic
void CoordinateConverter::convertEcefToGeodeticOri(const Vec3 &llaPos, const Vec3 &ecefOri, Vec3 &llaOri, LocalLevelFrame localLevelFrame)
{
  // calculate Local To Earth rotation matrix at lat, lon position of input platform
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the platform)
  double LE[3][3];
  CoordinateConverter::setLocalToEarthMatrix(llaPos.lat(), llaPos.lon(), localLevelFrame, LE);

  // create Body to Earth rotation matrix
  double BE[3][3];
  d3EulertoDCM(ecefOri, BE);

  // Multiply BE * (LE) transpose = BL (Body to Local Topo rotation matrix)
  double BL[3][3];
  d3MMTmult(BE, LE, BL);

  // get local Eulers
  d3DCMtoEuler(BL, llaOri);
}

/// Convert Earth Centered Earth Fixed (ECEF) acceleration to geodetic
void CoordinateConverter::convertEcefToGeodeticAccel(const Vec3 &llaPos, const Vec3 &ecefAcc, Vec3 &llaAcc, LocalLevelFrame localLevelFrame)
{
  // calculate Local To Earth rotation matrix at lat, lon position of input platform
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the platform)
  double LE[3][3];
  CoordinateConverter::setLocalToEarthMatrix(llaPos.lat(), llaPos.lon(), localLevelFrame, LE);

  // convert ECEF acceleration vector to Flat Earth acceleration vector
  //
  // (note that as with orientations, the transformation of vectors from ECEF to Flat Earth
  // is done using transformations to a tangent plane at the lat, lon of the platform)
  Vec3 nedVector;
  d3Mv3Mult(LE, ecefAcc, nedVector);

  if (localLevelFrame == LOCAL_LEVEL_FRAME_NED)
  {
    CoordinateConverter::swapNedEnu(nedVector, llaAcc);
  }
}

/// Converts geodetic Euler angles to an Earth Centered Earth Fixed (ECEF) Euler orientation
void CoordinateConverter::convertGeodeticOriToEcef(const Vec3 &llaPos, const Vec3 &llaOri, Vec3 &ecefOri, LocalLevelFrame localLevelFrame)
{
  // calculate rotation matrix at geodetic position
  double LE[3][3];
  CoordinateConverter::setLocalToEarthMatrix(llaPos[0], llaPos[1], localLevelFrame, LE);

  // calculate Body to Local rotation matrix using geodetic Eulers
  double BL[3][3];
  d3EulertoDCM(llaOri, BL);

  // calculate Body to Earth rotation matrix
  double BE[3][3];
  d3MMmult(BL, LE, BE);

  // calculate Euler angles for platform body in ECEF coordinates
  d3DCMtoEuler(BE, ecefOri);
}
