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
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <time.h>

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Calculations.h"

namespace
{
  // note that this returns const simCore::CoordinateConverter& - we can guarantee that the static cc will be present;
  // if the caller provides a suitably initialized coordConv, then we just give it back.
  const simCore::CoordinateConverter& initConverter(const simCore::CoordinateConverter* coordConv, const simCore::Vec3& refLla)
  {
    static simCore::CoordinateConverter cc;
    if (coordConv && coordConv->hasReferenceOrigin() && coordConv->referenceOrigin() == refLla)
      return *coordConv;
    cc.setReferenceOrigin(refLla);
    return cc;
  }
}

namespace simCore {

//------------------------------------------------------------------------

/**
* Calculates the relative azimuth, elevation, and composite angles from one entity to another in the given coordinate frame
* along the from entity's line of sight.
*/
void calculateRelAzEl(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, double *azim, double *elev, double *cmp, const EarthModelCalculations model, const CoordinateConverter* coordConv)
{
  assert(azim || elev || cmp);
  if (!azim && !elev && !cmp)
  {
    SIM_ERROR << "calculateRelAzEl, invalid angles: " << __LINE__ << std::endl;
    return;
  }

  Coordinate toPos;
  if (model == TANGENT_PLANE_WGS_84 || model == WGS_84)
  {
    const CoordinateConverter& cc = initConverter(coordConv, fromLla);
    cc.convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_XEAST);
    calculateRelAng(toPos.position(), fromOriLla, azim, elev, cmp);
  }
  else if (model == FLAT_EARTH && coordConv && coordConv->hasReferenceOrigin())
  {
    const Coordinate cvTo(COORD_SYS_LLA, fromLla);
    Coordinate fromPos;
    coordConv->convert(cvTo, fromPos, COORD_SYS_ENU);
    coordConv->convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_ENU);
    Vec3 ENUDelta;
    v3Subtract(toPos.position(), fromPos.position(), ENUDelta);
    calculateRelAng(ENUDelta, fromOriLla, azim, elev, cmp);
  }
  else
  {
    SIM_WARN << "Could not calculate relative angles: " << __LINE__ << std::endl;
    assert(false);

    if (azim) *azim = 0.0;
    if (elev) *elev = 0.0;
    if (cmp) *cmp = 0.0;

    return;
  }
}

/**
* Calculates the absolute azimuth, elevation, and composite angles from one entity to another in the given coordinate frame.
* The calculation is performed with 0 degrees at true north
*/
void calculateAbsAzEl(const Vec3 &fromLla, const Vec3 &toLla, double *azim, double *elev, double *cmp, const EarthModelCalculations model, const CoordinateConverter *coordConv)
{
  assert(azim || elev || cmp);
  if (!azim && !elev && !cmp)
  {
    SIM_ERROR << "calculateAbsAzEl, invalid angles: " << __LINE__ << std::endl;
    return;
  }

  Vec3 ENUDelta;
  Coordinate toPos;

  if (model == WGS_84 || model == TANGENT_PLANE_WGS_84)
  {
    //create a tangent plane referenced to the fromLla platform
    const CoordinateConverter& cc = initConverter(coordConv, fromLla);
    cc.convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_XEAST);
    ENUDelta = toPos.position();
  }
  else if (model == FLAT_EARTH && coordConv && coordConv->hasReferenceOrigin())
  {
    Coordinate fromPos;
    coordConv->convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_ENU);
    coordConv->convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_ENU);
    v3Subtract(toPos.position(), fromPos.position(), ENUDelta);
  }
  else if (model == PERFECT_SPHERE)
  {
    Vec3 posVal;
    geodeticToSpherical(toLla[0], toLla[1], toLla[2], posVal);
    sphere2TangentPlane(fromLla, posVal, ENUDelta);
  }
  else
  {
    SIM_WARN << "Could not calculate true angles: " << __LINE__ << std::endl;
    assert(false);

    if (azim) *azim = 0.0;
    if (elev) *elev = 0.0;
    if (cmp) *cmp = 0.0;

    return;
  }

  if (azim)
    *azim = angFix2PI(atan2(ENUDelta[0], ENUDelta[1]));

  if (elev)
    *elev = atan2(ENUDelta[2], sqrt(ENUDelta[0]*ENUDelta[0] + ENUDelta[1]*ENUDelta[1]));

  if (cmp)
  {
    Vec3 northVector(0.0, 1.0, 0.0);
    *cmp = v3Angle(northVector, ENUDelta);
  }
}

/**
* Calculates the slant distance between two positions in space in the given coordinate system.  Order of entities (from/to)
* will not affect the calculation.
*/
double calculateSlant(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv)
{
  // determine correct object locations based on coordinate system
  Coordinate fromPos;
  Coordinate toPos;
  switch (model)
  {
  case WGS_84:
    {
      CoordinateConverter::convertGeodeticToEcef(Coordinate(COORD_SYS_LLA, fromLla), fromPos);
      CoordinateConverter::convertGeodeticToEcef(Coordinate(COORD_SYS_LLA, toLla), toPos);
    }
    break;

  case TANGENT_PLANE_WGS_84:
    {
      const CoordinateConverter& cc = initConverter(coordConv, fromLla);
      cc.convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_XEAST);
      cc.convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_XEAST);
    }
    break;

  case FLAT_EARTH:
    {
      if (!coordConv || !coordConv->hasReferenceOrigin())
      {
        SIM_WARN << "Could not calculate \"slant range\", CoordinateConverter not set for FLAT_EARTH: " << __LINE__ << std::endl;
        assert(false);
        return 0;
      }
      coordConv->convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_ENU);
      coordConv->convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_ENU);
    }
    break;

  case PERFECT_SPHERE:
    {
      Vec3 pos1;
      Vec3 pos2;
      geodeticToSpherical(fromLla[0], fromLla[1], fromLla[2], pos1);
      geodeticToSpherical(toLla[0], toLla[1], toLla[2], pos2);
      return v3Distance(pos2, pos1);
    }
    break;

  default:
    SIM_WARN << "Could not calculate \"slant range\", Unknown coord system: " << __LINE__ << std::endl;
    assert(false);
    return 0;
  }

  return v3Distance(toPos.position(), fromPos.position());
}

/**
* Calculates the ground distance from one object to another.  This is calculated by "dropping a line" to the surface of the
* earth for both entities and calculating the distance of the line that connects the two surface points.  Order of the entities
* (from/to) will not affect the calculation.
*/
double calculateGroundDist(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv)
{
  // from and to are the states(double [9])
  if (model == WGS_84)
  {
    return sodanoInverse(fromLla[0], fromLla[1], 0., toLla[0], toLla[1]);
  }
  else if (model == TANGENT_PLANE_WGS_84)
  {
    Coordinate fromPos;
    Coordinate toPos;
    const CoordinateConverter& cc = initConverter(coordConv, fromLla);
    cc.convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_XEAST);
    cc.convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_XEAST);

    const Vec3 &toVal = toPos.position();
    const Vec3 &fromVal = fromPos.position();
    return sqrt(square(toVal[0]- fromVal[0]) + square(toVal[1]- fromVal[1]));
  }
  else if (coordConv)
  {
    Coordinate fromPos;
    Coordinate toPos;

    if (model == FLAT_EARTH && coordConv && coordConv->hasReferenceOrigin())
    {
      coordConv->convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_ENU);
      coordConv->convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_ENU);
    }
    else
    {
      SIM_WARN << "Could not calculate \"ground\" distance: " << __LINE__ << std::endl;
      assert(false);
      return 0;
    }

    const Vec3 &toVal = toPos.position();
    const Vec3 &fromVal = fromPos.position();
    return sqrt(square(toVal[0] - fromVal[0]) + square(toVal[1] - fromVal[1]));
  }

  SIM_WARN << "Could not calculate \"ground\" distance: " << __LINE__ << std::endl;
  assert(false);
  return 0;
}

/**
* Calculates the altitude difference from one object to another.  Order of the entities (from/to) will negate the
* result of the calculation.  A "higher" to altitude will return a positive value.
*/
double calculateAltitude(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv)
{
  if (model == WGS_84)
  {
    return toLla[2] - fromLla[2];  // difference in altitude values
  }

  if (model == TANGENT_PLANE_WGS_84)
  {
    Coordinate fromPos;
    Coordinate toPos;
    const CoordinateConverter& cc = initConverter(coordConv, fromLla);
    cc.convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_XEAST);
    cc.convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_XEAST);
    return toPos.z() - fromPos.z();
  }
  if (model == FLAT_EARTH && coordConv && coordConv->hasReferenceOrigin())
  {
    Coordinate fromPos;
    Coordinate toPos;
    coordConv->convert(Coordinate(COORD_SYS_LLA, fromLla), fromPos, COORD_SYS_ENU);
    coordConv->convert(Coordinate(COORD_SYS_LLA, toLla), toPos, COORD_SYS_ENU);
    return toPos.z() - fromPos.z();
  }
  else
  {
    SIM_WARN << "Could not calculate altitude: " << __LINE__ << std::endl;
    assert(false);
  }

  return 0;
}

/**
* Calculates the downrange, crossrange, and down values between two entities in space along the pointing angle specified by the
* from entity's state.
*/
void calculateDRCRDownValue(const Vec3 &fromLla, const double &yaw, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, double* downRng, double* crossRng, double* downValue)
{
  assert(downRng || crossRng || downValue);
  if (!downRng && !crossRng && !downValue)
  {
    SIM_ERROR << "calculateDRCRDownValue, invalid ranges: " << __LINE__ << std::endl;
    return;
  }

  const CoordinateConverter& cc = initConverter(coordConv, fromLla);

  // get the slant distance from "fromLla" to "toLla"
  const double slantDistance = calculateSlant(fromLla, toLla, model, &cc);

  // get the true azimuth and elevation from "fromLla" to "toLla"
  double trueAzimuth = 0;
  double trueElevation = 0;
  calculateAbsAzEl(fromLla, toLla, &trueAzimuth, &trueElevation, NULL, model, &cc);

  // get the down value
  if (downValue)
    *downValue = slantDistance * sin(trueElevation);

  const double downRangeCrossRangeAngle = trueAzimuth - yaw;
  const double downRangeCrossRangeHypotenuse = slantDistance * cos(trueElevation);

  // calculates the downrange and crossrange
  if (downRng)
    *downRng = downRangeCrossRangeHypotenuse * cos(downRangeCrossRangeAngle);

  if (crossRng)
    *crossRng = downRangeCrossRangeHypotenuse * sin(downRangeCrossRangeAngle);
}

/**
* Calculates the closing velocity, which is the velocity at which the from and to entity are moving towards one another.  Closing
* velocity is positive when the distance between two entities is decreasing (moving towards one another), and negative when moving apart.
*/
double calculateClosingVelocity(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel)
{
  Coordinate fromPos;
  Coordinate toPos;
  Coordinate fromState(COORD_SYS_LLA, fromLla, Vec3(0., 0., 0.), fromVel);
  Coordinate   toState(COORD_SYS_LLA, toLla, Vec3(0., 0., 0.), toVel);
  const CoordinateConverter& cc = initConverter(coordConv, fromLla);

  // determine correct object locations based on coordinate system
  if (convertLocations(fromState, toState, model, &cc, fromPos, toPos))
  {
    // create a unit position vector
    Vec3 unitPosVec;
    v3Subtract(toPos.position(), fromPos.position(), unitPosVec);
    simCore::v3Unit(unitPosVec);

    // closing velocity will be the difference of the velocity
    // vectors dotted with the normalized position difference.
    simCore::Vec3 diff;
    v3Subtract(fromPos.velocity(), toPos.velocity(), diff);
    return v3Dot(diff, unitPosVec);
  }

  SIM_ERROR << "calculateClosingVelocity, unable to perform calculation: " << __LINE__ << std::endl;
  return 0;
}

/**
* Calculates the velocity delta, which is the difference of the squares of the differences of velocity components and is always
* positive.  This is similar to the closing velocity, but does not alter the return value based on the velocity component that
* is along the pointing vector.
*/
double calculateVelocityDelta(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel)
{
  Coordinate fromPos;
  Coordinate toPos;
  Coordinate fromState(COORD_SYS_LLA, fromLla, Vec3(0., 0., 0.), fromVel);
  Coordinate   toState(COORD_SYS_LLA, toLla, Vec3(0., 0., 0.), toVel);
  const CoordinateConverter& cc = initConverter(coordConv, fromLla);

  // determine correct object locations based on coordinate system
  if (convertLocations(fromState, toState, model, &cc, fromPos, toPos))
  {
    return v3Distance(fromPos.velocity(), toPos.velocity());
  }

  SIM_ERROR << "calculateVelocityDelta, unable to perform calculation: " << __LINE__ << std::endl;
  return 0;
}

/// Calculates the range rate in m/sec between two entities.
double calculateRangeRate(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, const Vec3 &toOriLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel)
{
  Coordinate fromPos;
  Coordinate toPos;
  Coordinate fromState(COORD_SYS_LLA, fromLla, fromOriLla, fromVel);
  Coordinate   toState(COORD_SYS_LLA, toLla, toOriLla, toVel);
  const CoordinateConverter& cc = initConverter(coordConv, fromLla);

  if (!convertLocations(fromState, toState, model, &cc, fromPos, toPos))
  {
    SIM_ERROR << "calculateRangeRate, unable to perform calculation: " << __LINE__ << std::endl;
    return 0;
  }

  double bearing = 0;
  calculateRelAzEl(fromLla, fromOriLla, toLla, &bearing, NULL, NULL, model, &cc);
  return v3Length(fromVel) * cos(fromOriLla[0] - bearing) - (v3Length(toVel) * cos(toOriLla[0] - bearing));
}

/// Calculates the bearing rate in rad/sec between two entities
double simCore::calculateBearingRate(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, const Vec3 &toOriLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel)
{
  Coordinate fromPos;
  Coordinate   toPos;
  Coordinate fromState(COORD_SYS_LLA, fromLla, fromOriLla, fromVel);
  Coordinate   toState(COORD_SYS_LLA, toLla, toOriLla, toVel);
  const CoordinateConverter& cc = initConverter(coordConv, fromLla);

  if (!convertLocations(fromState, toState, model, &cc, fromPos, toPos))
  {
    SIM_ERROR << "calculateBearingRate, unable to perform calculation: " << __LINE__ << std::endl;
    return 0;
  }

  double bearing = 0;
  calculateRelAzEl(fromLla, fromOriLla, toLla, &bearing, NULL, NULL, model, &cc);

  const double range = calculateGroundDist(fromLla, toLla, model, &cc);
  const double tspd  = v3Length(toVel);
  const double ospd  = v3Length(fromVel);

  return (tspd * sin(toOriLla[0]) - ospd * sin(fromOriLla[0])) *
    cos(bearing) - (tspd * cos(toOriLla[0]) - ospd * cos(fromOriLla[0])) *
    sin(bearing) / range;
}

/**
* Calculates the aspect angle between two objects in space in the given coordinate system.  Aspect angle is the angle between
* the line of sight of the 'from' entity to the 'to' entity and the longitudinal axis of the 'to' entity.
*/
double calculateAspectAngle(const Vec3 &fromLla, const Vec3 &toLla, const Vec3 &toOriLla)
{
  // Determine geodetic unit vector for 'to' entity
  // LE is referenced to a NED system (Local Geodetic Horizon Coordinate System)
  double LE[3][3];
  CoordinateConverter::setLocalToEarthMatrix(toLla.lat(), toLla.lon(), simCore::LOCAL_LEVEL_FRAME_NED, LE);

  // Calculate body unit vector for the 'to' entity
  Vec3 bodyVec;
  calculateBodyUnitX(toOriLla.yaw(), toOriLla.pitch(), bodyVec);
  Vec3 bodyUnitVecX;
  d3MTv3Mult(LE, bodyVec, bodyUnitVecX);

  // Compute line of sight (LOS) vector in ECEF, relative to 'from' entity
  Vec3 fromPosECEF;
  CoordinateConverter::convertGeodeticPosToEcef(fromLla, fromPosECEF);
  Vec3 toPosECEF;
  CoordinateConverter::convertGeodeticPosToEcef(toLla, toPosECEF);
  Vec3 losECEF;
  v3Subtract(toPosECEF, fromPosECEF, losECEF);

  // normalize prior to computing aspect angle
  Vec3 losNOM;
  v3Norm(losECEF, losNOM);

  // Compute aspect angle, relative to the 'to' entity
  double cosAspectAng = -(v3Dot(losNOM, bodyUnitVecX));
  return simCore::inverseCosine(cosAspectAng);
}

/**
* This function implements Sodano's direct solution algorithm to determine geodetic
* longitude and latitude and back azimuth given a geodetic reference longitude
* and latitude, a geodesic length, a forward azimuth  and an ellipsoid definition.
*/
void sodanoDirect(const double refLat, const double refLon, const double refAlt, const double dist, const double azfwd, double *lat, double *lon, double *azbck)
{
  // Reference:
  // E. M. Sodano and T. A. Robinson,
  // "Direct and Inverse Solutions in Geodesics Technical Report 7"
  // U.S. Army Map Service, Washington, DC 1963 pp. 15-27.
  assert(lat || lon || azbck);
  if (!lat && !lon && !azbck)
  {
    SIM_ERROR << "sodanoDirect, invalid output params: " << __LINE__ << std::endl;
    return;
  }

  const double reqtr = WGS_A + refAlt;
  const double rpolr = reqtr * (1.0 - WGS_F);
  const double flat = 1. - (rpolr/reqtr);
  const double ecc2 = (reqtr*reqtr - rpolr*rpolr)/(rpolr*rpolr);
  const double theta = dist / rpolr;
  const double beta1 = atan2(rpolr*sin(refLat), reqtr*cos(refLat));

  const double sbeta1 = sin(beta1);
  const double cbeta1 = cos(beta1);
  const double stheta = sin(theta);
  const double ctheta = cos(theta);
  const double saz = sin(azfwd);
  const double caz = cos(azfwd);

  const double g = cbeta1*caz;
  const double h = cbeta1*saz;

  const double m = (1.+0.5*ecc2*sbeta1*sbeta1)*(1.-h*h)*0.5;
  const double n = (1.+0.5*ecc2*sbeta1*sbeta1)*(ctheta*sbeta1*sbeta1+g*sbeta1*stheta)*0.5;
  const double length = h*(-flat*theta+3.*flat*flat*n*stheta+3.*flat*flat*m*(theta-stheta*ctheta)*0.5);
  const double capm = m*ecc2;
  const double capn = n*ecc2;
  const double delta = theta - capn*stheta + 0.5*capm*(stheta*ctheta - theta) + (5./2.)*capn*capn*stheta*ctheta +
    (capm*capm/16.)*(11.*theta-13.*stheta*ctheta-8.*theta*ctheta*ctheta+10.*stheta*ctheta*ctheta*ctheta)
    + 0.5*capm*capn*(3.*stheta+2.*theta*ctheta-5.*stheta*ctheta*ctheta);

  const double sdel = sin(delta);
  const double cdel = cos(delta);
  const double f = g*cdel - sbeta1*sdel;
  const double sbeta2 = sbeta1*cdel + g*sdel;
  const double cbeta2 = sqrt(h*h + f*f);
  const double lamda = atan2((sdel*saz), (cbeta1*cdel - sbeta1*sdel*caz));

  // Set second latitude and longitude point
  if (lat) *lat = atan2(reqtr*sbeta2, rpolr*cbeta2);
  if (lon) *lon = refLon + lamda + length;

  // Back azimuth
  if (azbck) *azbck = atan2(-h, (sbeta1*sdel - g*cdel));
}

/**
* This function implements Sodano's indirect algorithm to determine geodesic length or distance, forward
* azimuth, and backward azimuth from a given pair of geodetic longitudes and latitudes and a given ellipsoid.
*/
double sodanoInverse(const double refLat, const double refLon, const double refAlt, const double lat, const double lon, double *azfwd, double *azbck)
{
  // Reference:
  // E. M. Sodano and T. A. Robinson,
  // "Direct and Inverse Solutions in Geodesics Technical Report 7"
  // U.S. Army Map Service, Washington, DC 1963 pp. 15-27.
  if (refLat == lat && refLon == lon)
  {
    if (azfwd) *azfwd = 0;
    if (azbck) *azbck = 0;
    return 0.0;
  }

  const double reqtr = WGS_A + refAlt;
  const double rpolr = reqtr * (1.0 - WGS_F);
  const double flat = 1. - (rpolr/reqtr);
  const double deltaLon = lon - refLon;
  const double beta1 = atan2((rpolr*sin(refLat)), (reqtr*cos(refLat)));
  const double beta2 = atan2((rpolr*sin(lat)), (reqtr*cos(lat)));
  const double sbet1 = sin(beta1);
  const double sbet2 = sin(beta2);
  const double cbet1 = cos(beta1);
  const double cbet2 = cos(beta2);
  const double sl = sin(deltaLon);
  const double sl2 = sin(0.5*deltaLon);

  const double a = sbet1*sbet2;
  const double b = cbet1*cbet2;
  const double cdel = a + b*cos(deltaLon);
  const double n = (reqtr-rpolr) / (reqtr+rpolr);
  const double b2mb1 = (lat-refLat) + 2.*(a*(n + n*n + n*n*n)-b*(n - n*n + n*n*n)) * sin(lat-refLat);

  const double d = sin(b2mb1) + 2.*cbet2*sbet1*sl2*sl2;
  const double sdel = sqrt(sl*sl*cbet2*cbet2 + d*d);
  const double delta = fabs(atan2(sdel, cdel));

  const double c = b*sl/sdel;
  const double m = 1. - c*c;
  const double f2 = flat*flat;
  const double d2 = delta*delta;

  if (azfwd || azbck)
  {
    // Forward and back azimuths
    const double lamda = deltaLon+c*((flat+f2)*delta-0.5*a*f2*(sdel+2.*d2/sdel)+
      0.25*m*f2*(sdel*cdel-5.*delta+4.*d2/tan(delta)));

    const double slam = sin(lamda);
    const double slam2 = sin(0.5*lamda);

    if (azfwd) *azfwd = atan2((cbet2*slam), (sin(b2mb1) + 2.*cbet2*sbet1*slam2*slam2));
    if (azbck) *azbck = atan2((-cbet1*slam), (2.*cbet1*sbet2*slam2*slam2 - sin(b2mb1)));
  }

  // Geodesic length
  return rpolr*((1.+flat+f2)*delta + a*((flat+f2)*sdel-f2*d2/(2.*sdel))
    -0.5*m*((flat+f2)*(delta+sdel*cdel)-f2*d2/tan(delta))
    -0.5*a*a*f2*sdel*cdel+(f2*m*m/16.)
    *(delta+sdel*cdel-2.*sdel*cdel*cdel*cdel-8.*d2/tan(delta))
    +0.5*a*m*f2*(sdel*cdel*cdel+d2/sdel));
}

/**
* Calculates the geodesic downrange and crossrange values between two entities in space
* referenced to the from entity's state using the Sodano method.
*/
NumericalSearchType calculateGeodesicDRCR(const Vec3 &fromLla, const double &yaw, const Vec3 &toLla, double* downRng, double* crossRng, const double minDR, const double minCR)
{
  // requires one output pointer
  if (!downRng && !crossRng)
  {
    assert(0);
    SIM_ERROR << "calculateGeodesicDRCR, invalid output params: " << __LINE__ << std::endl;
    return SEARCH_FAILED;
  }

  // patch a missing output pointer
  double tmpDRCRValue;
  if (!downRng)
    downRng = &tmpDRCRValue;
  else if (!crossRng)
    crossRng = &tmpDRCRValue;

  // assign zero ranges
  *downRng  = 0.0;
  *crossRng = 0.0;

  // set reference location
  const double latref = fromLla[0];
  const double lonref = fromLla[1];
  const double azref  = yaw;

  // Get downrange/crossrange reference point and azimuth
  const double lat = toLla[0];
  const double lon = toLla[1];

  // Compute distance to vehicle and if there is no crossrange, then
  // no search is necessary
  double dwnrng = 0;
  double azf = 0;
  if (fabs(lonref - lon) > LATLON_ERR_TOL_DOUBLE || fabs(latref - lat) > LATLON_ERR_TOL_DOUBLE)
  {
    dwnrng = sodanoInverse(latref, lonref, fromLla[2], lat, lon, &azf);
  }

  // if vehicle at reference point, return zero ranges
  if (dwnrng < minDR)
  {
    return SEARCH_CONVERGED;
  }

  // determine if "to" is ahead-of or behind "from"
  const double a1 = angFix2PI(azref);
  const double a2 = angFix2PI(azf);
  // Azimuth to vehicle minus reference azimuth, [0, 2PI]
  const double a2ma1 = fabs(a2 - a1);
  // normalized to [0, M_PI] (anything in 3rd or 4th quadrant is mirrored into 2nd or 1st quadrant)
  const double da = simCore::sdkMin(a2ma1, fabs(a2ma1 - M_TWOPI));
  assert(da >= 0 && da <= M_PI);

  // if no crossrange, then don't need to search
  if ((dwnrng * sin(da)) < minCR)
  {
    *downRng = dwnrng;

    // if forward azimuth is >90 deg off reference azimuth (in either direction), then set
    // the downrange to negative
    if (da > M_PI_2)
    {
      *downRng = -dwnrng;
    }
    return SEARCH_CONVERGED;
  }

  double dwnlo = 0;
  double dwnhi = 0;
  if (da > M_PI_2)
  {
    dwnlo = -1.20 * dwnrng;
    dwnhi = -minDR;
  }
  else
  {
    dwnlo = minDR;
    dwnhi = 1.20 * dwnrng;
  }

  // Vary downrange along reference azimuth until the backwards azimuth from lonref,latref
  // to lon2,lat2 plus the forward azimuth from lon2,lat2 to lon,lat is 90 degrees.

  // The distance from lonref, latref to lon2, lat2 is the
  // downrange, and the distance from lon2, lat2 to lon,lat is the crossrange

  double lat2 = 0;
  double lon2 = 0;
  double azbk = 0;
  double delaz = 0;
  double crsrng = 0;
  double err = 2.0e30;

  // limited testing suggests that, for a linear search tolerance on the order of 1e-06, a bisect search tolerance of .33 is optimal.
  // this is based on the judgement that bisect search costs less than linear search: so that,
  //   one extra bisect iteration is judged better than one extra linear iteration
  //
  // one test case required linear tolerance > 5.09e-07; the linear tolerance was therefore increased to 1e-06
  //
  // testing suggests that decreasing the bisect tolerance (<.33) requires extra bisect iteration but does not reduce linear iteration.
  // one data point showed no extra linear iteration when tolerance was 0.33 rather than 0.3
  // in that test case, converging at 0.33 meant one less iteration of bisect but required no extra linear iteration
  //
  // another data point showed no improvement when bisect tolerance increased to 0.52
  // in that test case, one less iteration of bisect did require one more iteration of linear

  NumericalSearchType type;
  // Coarse search limits are 25 iterations or a tolerance of .33 radians, whichever comes first
  BisectionSearch bisect(25, 0.33);
  // Fine search limits are 50 iterations or a tolerance of 1e-6 radians, whichever comes first
  LinearSearch linear(50, 1e-6);

  // Use a for loop to do both searches with the same calculations
  for (size_t ii = 0; ii < 2; ii++)
  {
    type = SEARCH_INIT;
    do
    {
      if (ii == 0)
      {
        // begin search with bisection method until near solution
        type = bisect.searchX(dwnrng, err, dwnlo, dwnhi, type);
        // note that we move on to linear search even if we failed to converge here
      }
      else
      {
        // finish converging solution with linear method
        type = linear.searchX(dwnrng, err, dwnlo, dwnhi, 1.0e-7, type);
      }
      if (type < SEARCH_CONVERGED)
      {
        // compute lat/lng and azimuth back to reference point
        // of a point at "dwnrng" along reference azimuth
        if (dwnrng > 0.01 * minDR)
        {
          sodanoDirect(latref, lonref, fromLla[2], dwnrng, azref, &lat2, &lon2, &azbk);
        }
        else if (dwnrng < -0.01 * minDR)
        {
          sodanoDirect(latref, lonref, fromLla[2], -dwnrng, azref+M_PI, &lat2, &lon2, &azbk);
        }
        else
        {
          lon2 = lonref;
          lat2 = latref;
          azbk = (dwnrng < 0.0) ? azref : -azref;
        }

        // compute azimuth and distance from lat2/lon2 point at "dwnrng"
        // along reference azimuth and to the vehicle's location
        azf = 0.0;
        crsrng = 0.0;
        if (fabs(lonref - lon) > LATLON_ERR_TOL_DOUBLE ||
          fabs(latref - lat) > LATLON_ERR_TOL_DOUBLE)
        {
          crsrng = sodanoInverse(lat2, lon2, fromLla[2], lat, lon, &azf);
        }

        // Compute difference between forward azimuth to vehicle
        // and backward azimuth to reference point
        delaz = angFixPI(azf - azbk);

        // The error tolerance nears zero as the difference in azimuths near 90 deg.
        err = M_PI_2 - fabs(delaz);
      } // if not converged
    } while (type < SEARCH_CONVERGED);
  } // for each search type

  // search converged, adjust signs on down/crossrange
  if (type != SEARCH_NO_ROOT)
  {
    if (delaz > 0.0)
      crsrng = -crsrng;
    if (dwnrng < 0.0)
      crsrng = -crsrng;

    *downRng = dwnrng;
    *crossRng = crsrng;
  }

  // algorithm (delaz/err) assumes we can model the shape formed by points refPt, lat2lon2, latlon as a right triangle.
  // where refpt->lat2lon2->latlon forms the right angle; the sum of the other two angles should then be 90.
  // but in some cases, the assumption of a right triangle is is incorrect:
  // when latlon and lat2lon2 are near a pole, sodanoInverse may return angles that cut across the pole and do not fit the assumption.
  //   we can identify those cases when the sum of the two angles != 90.

  // angle from lat2lon2, refpt, latlon
  const double angle1 = (a2 - a1);

  // angle from lat2lon2, latlon, refpt (where azb is the back azimuth from latlon to lat2lon2, corresponding to the calculated azf)
  // angle2 = angFixPI(azb - a2);
  // if we approximate azb = -(M_PI - azf), then we don't need to calculate azb at all
  const double angle2 = (azf - M_PI) - a2;
  const double angleSum = angFixPI(angle1 + angle2);

  // for valid calcs, angleSum should approach +/- M_PI_2 as the err approaches 0
  assert((type == SEARCH_FAILED) || simCore::areEqual(angleSum, M_PI_2, 0.04) || simCore::areEqual(angleSum, -M_PI_2, 0.04));

  // note that SEARCH_FAILED may occur if tolerance is too tight: instead of iterating to max iterations, may be detected as failure
  if (type == SEARCH_FAILED)
  {
    const double maxrng = std::max(fabs(dwnrng), fabs(crsrng));
    // if we failed and downrange or crossrange was > 10000km, do not message. probably such points are extraneous.
    if (maxrng > 1e7)
      return type;

    // only message on failures that do not involve the condition described above
    if (simCore::areEqual(angleSum, M_PI_2, 0.04) || simCore::areEqual(angleSum, -M_PI_2, 0.04))
    {
      time_t currtime;
      time(&currtime);
      // notify when error occurred using current local time
      SIM_ERROR << "calculateGeodesicDRCR linear search failed to converge to an answer @ " << ctime(&currtime) << std::endl;
      // note that 15 decimal places may be necessary to reproduce a failing case
    }
  }
  else if (type == SEARCH_MAX_ITER)
  {
    time_t currtime;
    time(&currtime);
    // notify when error occurred using current local time
    SIM_ERROR << "calculateGeodesicDRCR linear search did not converge to an answer within allowed number of iterations @ " << ctime(&currtime) << std::endl;
  }
  return type;
}

double calculateEarthRadius(const double latitude)
{
  double sLat = sin(latitude);
  double cLat = cos(latitude);
  double Re = sqrt((square(WGS_A2 * cLat) + square(WGS_B2 * sLat)) / (square(WGS_A * cLat) + square(WGS_B * sLat)));
  return Re;
}

Vec3 clampEcefPointToGeodeticSurface(const Vec3& p)
{
  Vec3 lla;
  CoordinateConverter::convertEcefToGeodeticPos(p, lla);
  // If we are near the surface, we are done
  // Value of 5 mm is based on a 4.4e-3 precision resolution comment
  // in a SDK ECEF to LLA CoordConvertLibTest unit test
  if (areEqual(lla.alt(), 0.0, 5.0e-3))
    return p;
  // Otherwise, clamp to surface and convert back to ECEF
  lla.setAlt(0.0);
  Vec3 ecef;
  CoordinateConverter::convertGeodeticPosToEcef(lla, ecef);
  return ecef;
}

/**
* Calculates the horizon distance for either geometric, optical or radar.
* Equations derived from a perfect sphere using Pythagorean Theorem
* Optical horizon uses a 1.06 effective Earth radius to account for refraction effects (constant lapse rate and homogeneous atmosphere).
* Radar horizon uses a 4/3 effective Earth radius to account for refraction effects.
* Earth radius is based on WGS-84 ellipsoid
*/
double calculateHorizonDist(const Vec3& lla, const simCore::HorizonCalculations horizonType, const double opticalRadius, const double rfRadius)
{
  // Return if at/under ground
  if (lla.alt() <= 0)
    return 0.0;
  // compute radius of Earth at observer's latitude location
  double twoRe = 2.0 * simCore::calculateEarthRadius(lla.lat());
  double dist = 0.0;
  switch (horizonType)
  {
  case simCore::OPTICAL_HORIZON:
    // with refraction
    dist = sqrt(twoRe * lla.alt() * opticalRadius + square(lla.alt()));
    break;
  case simCore::RADAR_HORIZON:
    // empirical Earth radius for RF refraction @ std atmosphere
    dist = sqrt(twoRe * lla.alt() * rfRadius + square(lla.alt()));
    break;
  default: // simCore::GEOMETRIC_HORIZON
    // no refraction
    assert(horizonType == simCore::GEOMETRIC_HORIZON);
    dist = sqrt(twoRe * lla.alt() + square(lla.alt()));
    break;
  }
  return dist;
}

/**
* Converts the input locations to the specified coordinate system.
*/
bool convertLocations(const Coordinate &fromState, const Coordinate &toState, const EarthModelCalculations model, const CoordinateConverter* coordConv, Coordinate& fromPos, Coordinate& toPos)
{
  // Test for same input/output -- this function cannot handle case of fromPos == toState
  if (&fromPos == &toState)
  { // Note that fromState == toState, fromPos == fromState, and toState == toPos are handled fine
    assert(0);
    return false;
  }
  // determine correct object locations based on coordinate system
  switch (model)
  {
  case WGS_84:
    {
      if (!coordConv)
      {
        SIM_WARN << "Could not convert location, CoordinateConverter not set for WGS_84: " << __LINE__ << std::endl;
        assert(false);
        return false;
      }

      coordConv->convert(fromState, fromPos, COORD_SYS_ECEF);
      coordConv->convert(toState, toPos, COORD_SYS_ECEF);
    }
    break;

  case TANGENT_PLANE_WGS_84:
    {
      CoordinateConverter cc;
      cc.setReferenceOrigin(fromState.position());

      cc.convert(fromState, fromPos, COORD_SYS_XEAST);
      cc.convert(toState, toPos, COORD_SYS_XEAST);
    }
    break;

  case FLAT_EARTH:
    {
      if (!coordConv || !coordConv->hasReferenceOrigin())
      {
        SIM_WARN << "Could not convert location, CoordinateConverter not set for FLAT_EARTH: " << __LINE__ << std::endl;
        assert(false);
        return false;
      }

      coordConv->convert(fromState, fromPos, COORD_SYS_ENU);
      coordConv->convert(toState, toPos, COORD_SYS_ENU);
    }
    break;

  default:
    SIM_WARN << "Could not convert location, Unknown coord system: " << __LINE__ << std::endl;
    assert(false);
    return false;
  }

  return true;
}

/**
* Converts the given perfect sphere earth XYZ values to ENU Tangent Plane values, given
* the tangent plane's latitude, longitude, and altitude.  Note: If tangent plane's perfect
* sphere Earth XYZ values are available, they can be given for a faster calculation.
*/
void sphere2TangentPlane(const Vec3& llaVec, const Vec3& sphereVec, Vec3& tpVec, const Vec3* sphereTpOrigin)
{
  // Test for same input/output -- this function cannot handle case of llaVec == tpVec
  if (&llaVec == &tpVec)
  { // Note that sphereVec == tpVec IS handled correctly
    assert(0);
    return;
  }
  // if tangent plane's perfect sphere Earth XYZ values do not exist,
  // then calculate based on input LLA
  Vec3 tempSphereXYZ;
  if (sphereTpOrigin)
  {
    tempSphereXYZ = *sphereTpOrigin;
  }
  else
  {
    geodeticToSpherical(llaVec[0], llaVec[1], llaVec[2], tempSphereXYZ);
  }

  // get the delta spherical XYZ from the tangent plane to the given sphereVec point
  v3Subtract(sphereVec, tempSphereXYZ, tempSphereXYZ);

  // figure out the the tangent plane ENU values if the tangent plane
  // was at Lat = 0, Lon = 0
  tpVec[0] = -tempSphereXYZ[1];
  tpVec[1] = tempSphereXYZ[2];
  tpVec[2] = -tempSphereXYZ[0];

  // Correctly rotate at end of sphere2TangentPlane()
  // adjusts the tangent plane ENU values based on the given tangent plane Lat and Lon values
  v3RotY(tpVec, -llaVec[1], tpVec);
  v3RotX(tpVec,  llaVec[0], tpVec);
}

/**
* Converts the given ENU Tangent Plane values to perfect sphere Earth XYZ values, given
* the tangent planes latitude, longitude, and altitude.  Note: If tangent plane's perfect
* sphere Earth XYZ values are available, they can be given for a faster calculation.
*/
void tangentPlane2Sphere(const Vec3 &llaVec, const Vec3 &tpVec, Vec3& sphereVec, const Vec3* sphereTpOrigin)
{
  // Test for same input/output -- this function cannot handle case of llaVec == sphereVec
  if (&llaVec == &sphereVec)
  { // Note that sphereVec == tpVec IS handled correctly
    assert(0);
    return;
  }
  Vec3 tempTpENU(tpVec);

  // move the given ENU values from the given tangent plane to the
  //   tangent plane at Lat = 0, Lon = 0
  v3RotX(tempTpENU, -llaVec[0], tempTpENU);
  v3RotY(tempTpENU,  llaVec[1], tempTpENU);

  // get the spherical XYZ values
  sphereVec[0] = -tempTpENU[2];
  sphereVec[1] = -tempTpENU[0];
  sphereVec[2] =  tempTpENU[1];

  // get the spherical XYZ coordinates of the tangent plane
  Vec3 tempSphereXYZ;
  if (!sphereTpOrigin)
  {
    geodeticToSpherical(llaVec[0], llaVec[1], llaVec[2], tempSphereXYZ);
  }
  else
  {
    tempSphereXYZ = *sphereTpOrigin;
  }

  // get the delta spherical XYZ from the tangent plane to the given sphereVec point
  v3Add(sphereVec, tempSphereXYZ, sphereVec);
}

/**
* Converts a geodetic LLA position into a perfect sphere XYZ position.  For the sphere model:
*  Each axis corresponds to the following positions (lat,lon):
*  +X = (0,-180) -X = (0,0)
*  +Y = (0, -90) -Y = (0,90)
*  +Z = (90,0)   -Z = (-90,0)
*/
void geodeticToSpherical(const double lat, const double lon, const double alt, Vec3 &point)
{
  const double altscale = EARTH_RADIUS + alt;
  const double coslat = cos(lat);

  point[0] = -coslat * cos(lon) * altscale;
  point[1] = -coslat * sin(lon) * altscale;
  point[2] = sin(lat) * altscale;
}

/**
* Calculates the relative angles between an ENU vector and a set of geodetic Euler angles
*/
void calculateRelAng(const Vec3 &enuVec, const Vec3 &refOri, double *azim, double *elev, double *cmp)
{
  assert(azim || elev || cmp);
  if (!azim && !elev && !cmp)
  {
    SIM_ERROR << "calculateRelAng, invalid output params: " << __LINE__ << std::endl;
    return;
  }

  if (azim || elev)
  {
    // compute rotation matrix based on reference geodetic Euler angles
    double rotMat[3][3];
    d3EulertoDCM(refOri, rotMat);

    // compute an inertial pointing vector based on ENU vector
    Vec3 pntVec;
    calculateBodyUnitX(atan2(enuVec[0], enuVec[1]), atan2(enuVec[2], sqrt(square(enuVec[0]) + square(enuVec[1]))), pntVec);

    // rotate inertial pointing vector to an body pointing vector
    Vec3 body;
    d3Mv3Mult(rotMat, pntVec, body);

    // decompose azimuth and elevation values from new body pointing vector
    double az, el;
    calculateYawPitchFromBodyUnitX(body, az, el);

    if (azim)
      *azim = az;

    if (elev)
      *elev = el;
  }

  // compute composite angle between an ENU and a reference vector
  if (cmp)
  {
    Vec3 pntVec;
    pntVec[0] = sin(refOri[0]);
    pntVec[1] = cos(refOri[0]);
    pntVec[2] = tan(refOri[1]);
    *cmp = v3Angle(pntVec, enuVec);
  }
}

/**
* Calculates the body relative angles from a set of geodetic Euler angles to a true az/el vector
*/
void calculateRelAngToTrueAzEl(double trueAz, double trueEl, const Vec3& refOri, double* azim, double* elev, double* cmp)
{
  // calculates a ENU unit vector from trueAz/trueEl
  Vec3 tmpUnitVecNED;
  simCore::calculateBodyUnitX(trueAz, trueEl, tmpUnitVecNED);
  const Vec3 tmpUnitVecENU(tmpUnitVecNED.y(), tmpUnitVecNED.x(), -tmpUnitVecNED.z());

  // calculates the body-relative angles from refOri to trueAz/trueEl
  simCore::calculateRelAng(tmpUnitVecENU, refOri, azim, elev, cmp);
}

/// Computes the X component of the body unit vector
void calculateBodyUnitX(const double yaw, const double pitch, Vec3 &vecX)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 26, Eqn 1.3-20
  vecX.set(cos(yaw) * cos(pitch), sin(yaw) * cos(pitch), -sin(pitch));
}

/// Computes the Y component of the body unit vector
void calculateBodyUnitY(const double yaw, const double pitch, const double roll, Vec3 &vecY)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 26, Eqn 1.3-20
  double sinYaw = sin(yaw);
  double cosYaw = cos(yaw);
  double sinPitch = sin(pitch);
  double cosPitch = cos(pitch);
  double sinRoll = sin(roll);
  double cosRoll = cos(roll);
  vecY.set((sinRoll * sinPitch * cosYaw) - (cosRoll * sinYaw),
           (sinRoll * sinPitch * sinYaw) + (cosRoll * cosYaw),
           sinRoll * cosPitch);
}

/// Computes the Z component of the body unit vector
void calculateBodyUnitZ(const double yaw, const double pitch, const double roll, Vec3 &vecZ)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 26, Eqn 1.3-20
  double sinYaw = sin(yaw);
  double cosYaw = cos(yaw);
  double sinPitch = sin(pitch);
  double cosPitch = cos(pitch);
  double sinRoll = sin(roll);
  double cosRoll = cos(roll);
  vecZ.set((cosRoll * sinPitch * cosYaw) + (sinRoll * sinYaw),
           (cosRoll * sinPitch * sinYaw) - (sinRoll * cosYaw),
           cosRoll * cosPitch);
}

/// Decomposes the X component of the unit body vector into yaw and pitch angles
void calculateYawPitchFromBodyUnitX(const Vec3 &vecX, double &yaw, double &pitch)
{
  // From Aircraft Control and Simulation 2nd Edition
  // B. Stevens & F. Lewis  2003
  // ISBN 0-471-37145-9
  // p. 29, Eqn 1.3-24

  // prevent division by zero and inverse trig function arguments of
  // magnitude greater than unity
  if (areEqual(vecX[2], 1.0))
  {
    yaw = 0.0;
    pitch = -M_PI_2;
  }
  else if (areEqual(vecX[2], -1.0))
  {
    yaw = 0.0;
    pitch = M_PI_2;
  }
  else
  {
    // no gimbal lock
    // atan2 returns in the range -pi to pi
    // inverseSine returns in the range -pi/2 to pi/2
    yaw = atan2(vecX[1], vecX[0]);
    pitch = simCore::inverseSine(-vecX[2]);
  }
}

/// Calculates an ENU geodetic velocity vector based on a local (moving) tangent plane whose origin is the current position
void calculateVelFromGeodeticPos(const Vec3 &currPos, const Vec3 &prevPos, const double deltaTime, Vec3 &velVec)
{
  // Tolerance of at least half inch at equator, in radians, using 60nm = 1 deg
  // 1/2 inch = 1.14290857 x 10-7 degrees or 1.99475176 x 10-9 radians.  Round to 1e-9
  static const double HALF_INCH_AT_EQUATOR_IN_RADIANS = 1e-9;

  if (areEqual(deltaTime, 0) || v3AreEqual(currPos, prevPos, HALF_INCH_AT_EQUATOR_IN_RADIANS))
  {
    velVec.zero();
    return;
  }

  CoordinateConverter cc;
  cc.setReferenceOrigin(currPos);

  Coordinate pnt1;
  cc.convert(Coordinate(COORD_SYS_LLA, currPos), pnt1, COORD_SYS_XEAST);

  Coordinate pnt2;
  cc.convert(Coordinate(COORD_SYS_LLA, prevPos), pnt2, COORD_SYS_XEAST);

  Vec3 posDiff;
  v3Subtract(pnt1.position(), pnt2.position(), posDiff);
  v3Scale(1.0/deltaTime, posDiff, velVec);
}

/// Calculates an ENU geodetic velocity vector based on dp/dt, and flight path angle orientation from velocity
bool calculateVelOriFromPos(const Vec3 &currPos, const Vec3 &prevPos, const double deltaTime, const CoordinateSystem sysIn, Vec3 &velOut, Vec3 &oriOut, const Vec3 &refLLA, const CoordinateSystem sysOut)
{
  // Calculates velocity vector based on dp/dt and derives orientation from velocity.
  // velocity vector calculated in switch, used to determine flight path angles
  Vec3 velVec;
  Coordinate lla2; // prevPos in LLA
  lla2.setCoordinateSystem(simCore::COORD_SYS_NONE);

  switch (sysIn)
  {
  case COORD_SYS_LLA:
    lla2 = simCore::Coordinate(simCore::COORD_SYS_LLA, prevPos);
    calculateVelFromGeodeticPos(currPos, prevPos, deltaTime, velVec);
    break;

  case COORD_SYS_ECEF:
    {
      simCore::Vec3 posLla1;
      simCore::CoordinateConverter::convertEcefToGeodeticPos(currPos, posLla1);
      CoordinateConverter::convertEcefToGeodetic(Coordinate(COORD_SYS_ECEF, prevPos), lla2);
      calculateVelFromGeodeticPos(posLla1, lla2.position(), deltaTime, velVec);
    }
    break;

  case COORD_SYS_XEAST:
  case COORD_SYS_ENU:
    {
      Vec3 posDiff;
      v3Subtract(currPos, prevPos, posDiff);
      v3Scale(1.0/deltaTime, posDiff, velVec);
    }
    break;

  case COORD_SYS_NED:
    {
      Vec3 posDiff;
      Vec3 enu1;
      Vec3 enu2;
      CoordinateConverter::swapNedEnu(currPos, enu1);
      CoordinateConverter::swapNedEnu(prevPos, enu2);
      v3Subtract(enu1, enu2, posDiff);
      v3Scale(1.0/deltaTime, posDiff, velVec);
    }
    break;

  case COORD_SYS_NWU:
    {
      Vec3 posDiff;
      Vec3 enu1;
      Vec3 enu2;
      CoordinateConverter::convertNwuToEnu(currPos, enu1);
      CoordinateConverter::convertNwuToEnu(prevPos, enu2);
      v3Subtract(enu1, enu2, posDiff);
      v3Scale(1.0/deltaTime, posDiff, velVec);
    }
    break;

  case COORD_SYS_ECI:
  case COORD_SYS_GTP:
    // conversions not supported
    return false;

  default:
    assert(0);
    break;
  }

  // calculate flight path angles from a geodetic (ENU) velocity vector
  Vec3 cprVec;
  calculateFlightPathAngles(velVec, cprVec);

  switch (sysOut)
  {
  case COORD_SYS_LLA:
  case COORD_SYS_XEAST:
  case COORD_SYS_ENU:
    velOut.set(velVec);
    oriOut.set(cprVec);
    break;

  case COORD_SYS_ECEF:
  {
    // Need to get the position (prevPos|lla2) in LLA so we can convert the vel/cpr from LLA frame to ECEF
    if (lla2.coordinateSystem() == simCore::COORD_SYS_NONE)
    {
      CoordinateConverter cc;
      cc.setReferenceOrigin(refLLA);
      cc.convert(Coordinate(sysIn, prevPos), lla2, COORD_SYS_LLA);
    }
    // Put in the orientation and velocity, and convert out
    lla2.setOrientation(cprVec);
    lla2.setVelocity(velVec);
    simCore::Coordinate ecefCoordinate;
    CoordinateConverter::convertGeodeticToEcef(lla2, ecefCoordinate);
    velOut.set(ecefCoordinate.velocity());
    oriOut.set(ecefCoordinate.orientation());
    break;
  }

  case COORD_SYS_NED:
    CoordinateConverter::swapNedEnu(velVec, velOut);
    oriOut.set(cprVec);
    break;

  case COORD_SYS_NWU:
    CoordinateConverter::convertEnuToNwu(velVec, velOut);
    oriOut.set(cprVec);
    break;

  default:
    assert(0);
    break;
  }

  return true;
}

/// Calculates a geodetic Euler orientation from angles relative to another geodetic Euler orientation
void calculateGeodeticOriFromRelOri(const Vec3 &hostYpr, const Vec3 &relYpr, Vec3 &ypr)
{
  // create DCM based on host orientation
  double dcm[3][3];
  d3EulertoDCM(hostYpr, dcm);

  // create DCM based on relative orientation
  double relDcm[3][3];
  d3EulertoDCM(relYpr, relDcm);

  // multiply DCMs
  double yprDcm[3][3];
  d3MMmult(relDcm, dcm, yprDcm);

  // convert new DCM to Euler angles
  d3DCMtoEuler(yprDcm, ypr);
}

/// Calculates a geodetic position from the given offset position and orientation vectors
void calculateGeodeticOffsetPos(const simCore::Vec3& llaBgnPos, const simCore::Vec3& bodyOriOffset, const simCore::Vec3& bodyPosOffset, simCore::Vec3& offsetLla)
{
  // create DCM based on specified orientation (NED frame)
  double dcm[3][3];
  simCore::d3EulertoDCM(bodyOriOffset, dcm);

  // create unit vector along body axis (NED frame), then rotate body vector to align with local level frame
  // SIMDIS FLU body coordinates changed to a FRD system in order to align to the NED frame
  Vec3 geoVec;
  d3MTv3Mult(dcm, simCore::Vec3(bodyPosOffset[0], -bodyPosOffset[1], -bodyPosOffset[2]), geoVec);

  // calculate Local To Earth rotation matrix at begin lat, lon position
  // (orientation is translated to geocentric Eulers based on the transformation from a local
  // tangent plane coordinate system at the lat and lon of the specified position)
  double localToEarth[3][3];
  simCore::CoordinateConverter::setLocalToEarthMatrix(llaBgnPos.lat(), llaBgnPos.lon(), simCore::LOCAL_LEVEL_FRAME_NED, localToEarth);

  // convert local level NED system to geocentric
  simCore::Vec3 geoOffVec;
  simCore::d3MTv3Mult(localToEarth, geoVec, geoOffVec);

  // convert LLA to ECEF
  simCore::Vec3 originGeo;
  simCore::CoordinateConverter::convertGeodeticPosToEcef(llaBgnPos, originGeo);

  // compute offset, then convert geocentric back to geodetic
  simCore::Vec3 offsetGeo;
  simCore::v3Add(originGeo, geoOffVec, offsetGeo);
  simCore::CoordinateConverter::convertEcefToGeodeticPos(offsetGeo, offsetLla);
}

/// Calculates the geodetic end point of a vector based on a specified azimuth, elevation and range from a given geodetic position
void calculateGeodeticEndPoint(const Vec3 &llaBgnPos, const double az, const double el, const double rng, Vec3 &llaEndPos)
{
  if (simCore::areEqual(rng, 0.))
  {
    llaEndPos = llaBgnPos;
    return;
  }

  calculateGeodeticOffsetPos(llaBgnPos, simCore::Vec3(az, el, 0), simCore::Vec3(rng, 0, 0), llaEndPos);
}

/// Calculates the middle position between two points on the globe, moving from west to east.
void calculateGeodeticMidPoint(const Vec3& llaBgnPos, const Vec3& llaEndPos, bool highResolution, simCore::Vec3& midpoint, bool* wrapsDateline)
{
  bool wrap = false;

  // Regardless of method, altitude calculation is the same
  midpoint.setAlt((llaBgnPos.alt() + llaEndPos.alt()) * 0.5);

  // High resolution calculation does a sodanoInverse to get angle, then a sodanoDirect to calculate midpoint
  if (highResolution)
  {
    double azimuth = 0.0;
    const double distance = sodanoInverse(llaBgnPos.lat(), llaBgnPos.lon(), 0.0, llaEndPos.lat(), llaEndPos.lon(), &azimuth);
    double lat = 0.0;
    double lon = 0.0;
    sodanoDirect(llaBgnPos.lat(), llaBgnPos.lon(), 0.0, distance * 0.5, azimuth, &lat, &lon);
    midpoint.setLat(lat);
    midpoint.setLon(lon);
    // Determine whether it wraps by looking at the longitude positions
    wrap = (angFixPI(llaBgnPos.lon()) > angFixPI(llaEndPos.lon()));
  }
  else
  {
    // Determine wrapping, which changes the longitudinal center point
    midpoint.setLat(angFixPI2((llaBgnPos.lat() + llaEndPos.lat()) * 0.5));
    const double bgnLon = angFixPI(llaBgnPos.lon());
    const double endLon = angFixPI(llaEndPos.lon());
    if (bgnLon <= endLon)
      midpoint.setLon(angFixPI((endLon + bgnLon) * 0.5));
    else
    {
      midpoint.setLon(angFixPI((endLon + bgnLon) * 0.5 + M_PI));
      wrap = true;
    }
  }

  if (wrapsDateline)
    *wrapsDateline = wrap;
}

/// Calculates flight path angles from an ENU geodetic velocity vector
void calculateFlightPathAngles(const Vec3 &velVec, Vec3 &fpa)
{
  // Test for same input/output -- this function cannot handle case of velVec == fpa
  if (&velVec == &fpa)
  {
    assert(0);
    return;
  }
  // check for zero velocity vector
  if (areEqual(velVec[0], 0.) &&
      areEqual(velVec[1], 0.) &&
      areEqual(velVec[2], 0.))
  {
    fpa.zero();
    return;
  }

  // horizontal flight path angle (heading): atan2(x,y) with x=east, y=north, and z=up
  // measured off of North with +pi/2 = East, right turn
  fpa.setYaw(angFix2PI(atan2(velVec.x(), velVec.y())));

  // vertical flight path angle (pitch)
  // positive pitch (climbing platforms) nose up
  fpa.setPitch(atan2(velVec.z(), sqrt(square(velVec.x()) + square(velVec.y()))));

  // positive roll, right wing down
  fpa.setRoll(0.0);
}

/// Calculates an ENU geodetic velocity vector from speed, heading and pitch (flight path angles)
void calculateVelocity(const double speed, const double heading, const double pitch, Vec3 &velVec)
{
  const double cPitch = cos(pitch);
  velVec.setY(speed * cos(heading) * cPitch);
  velVec.setX(speed * sin(heading) * cPitch);
  velVec.setZ(speed * sin(pitch));
}

/**
 * Calculates the angle of attack, side slip, and total angle of attack from an ENU
 * geodetic velocity vector and a set of geodetic Euler angles (yaw, pitch, roll)
 */
void calculateAoaSideslipTotalAoa(const Vec3& enuVel, const Vec3& ypr, const bool useRoll, double* aoa, double* ss, double* totalAoa)
{
  // aerodynamic version that accounts for roll
  Vec3 refOri = ypr;
  if (!useRoll)
  {
    // rocketry version (Air Ballistic Axis) that does not use roll,
    // prevents oscillation of alpha(aoa) and beta(sideslip) due to a rolling vehicle
    refOri.setRoll(0);
  }

  // compute alpha(aoa) and beta(sideslip) angles relative to velocity vector and body orientation
  calculateRelAng(enuVel, refOri, ss, aoa, totalAoa);
  // Negate aoa and ss values, calculateRelAng computes angles relative to reference ypr
  // For aoa and ss, the measure of difference is relative to the velocity vector
  if (aoa) *aoa = -*aoa;
  if (ss) *ss = -*ss;
}

double getClosestPoint(const simCore::Vec3& startLla, const simCore::Vec3& endLla, const simCore::Vec3& toLLA_, simCore::Vec3& closestLLa)
{
  simCore::Coordinate cvIn;
  simCore::Coordinate cvOut;
  simCore::CoordinateConverter tempFromECEF;
  simCore::Vec3 toPnt;
  simCore::Vec3 closestPnt;
  simCore::Vec3 pointingVector;
  simCore::Vec3 enuDelta;
  double length = 0;

  // create direction vector for line segment, since begin point of line segment is the origin of
  // the CoordConverter, the end point will also be the direction vector
  tempFromECEF.setReferenceOrigin(startLla);
  cvIn.setCoordinateSystem(simCore::COORD_SYS_LLA);
  cvIn.setPosition(endLla);
  tempFromECEF.convert(cvIn, cvOut, simCore::COORD_SYS_XEAST);
  // NOTE: this is also the direction vector for line segment
  pointingVector = cvOut.position();

  // create reference point in XEAST to determine closest point along line segment
  cvIn.setPosition(toLLA_);
  cvIn.setCoordinateSystem(simCore::COORD_SYS_LLA);
  cvOut.clear();
  tempFromECEF.convert(cvIn, cvOut, simCore::COORD_SYS_XEAST);
  enuDelta = cvOut.position();
  // store end point for use later
  toPnt = enuDelta;

  // ------------------------------------------------
  // gets the length (along the line segment pointing vector)
  // to the location of the line segment's "closest point"
  const double actualLength = simCore::v3Length(pointingVector);

  // prevent divide by zero
  if (simCore::areEqual(actualLength, 0.0))
    return 0.0;

  const double angle = simCore::v3Angle(pointingVector, enuDelta);
  if (angle > M_PI_2)
  {
    length = 0.0;
  }
  else
  {
    length = simCore::v3Length(enuDelta) * cos(angle);
    if (length > actualLength)
      length = actualLength;
  }

  // calculate the projection of the reference direction along the line segment direction vector
  simCore::v3Scale(length/actualLength, pointingVector, closestPnt);

  // convert closest point on line segment to a LLA value
  cvOut.clear();
  cvIn.setPosition(closestPnt);
  cvIn.setCoordinateSystem(simCore::COORD_SYS_XEAST);
  tempFromECEF.convert(cvIn, cvOut, simCore::COORD_SYS_LLA);
  closestLLa = cvOut.position();

  simCore::Vec3 delta;
  simCore::v3Subtract(toPnt, closestPnt, delta);
  return simCore::v3Length(delta);
}

bool positionInGate(const simCore::Vec3& gateHostLLA, const simCore::Vec3& positionLLA,
  double azimuthRad, double elevRad, double widthRad, double heightRad, double minRangeM, double maxRangeM,
  simCore::EarthModelCalculations earthModel, const CoordinateConverter& cc)
{
  double rae[3];

  // gets the azimuth, elevation, and length from the host platform to the position of interest
  simCore::calculateAbsAzEl(gateHostLLA, positionLLA, &rae[1], &rae[2], NULL, earthModel, &cc);
  rae[0] = simCore::calculateSlant(gateHostLLA, positionLLA, earthModel, &cc);

  const double halfW = widthRad / 2.0;
  const double halfH = heightRad / 2.0;

  if (rae[0] >= minRangeM && rae[0] <= maxRangeM)
  {
    if (rae[1] <= azimuthRad + halfW && rae[1] >= azimuthRad - halfW)
    {
      if (rae[2] <= elevRad + halfH && rae[2] >= elevRad - halfH)
	return true;
    }
  }

  return false;
}

bool laserInGate(const simCore::Vec3& gateHostLLA, const simCore::Vec3& laserHostLLA,
    double gAzimuthRad, double gElevRad, double gWidthRad, double gHeightRad, double gMinRangeM, double gMaxRangeM,
    double laserAzRad, double laserElRad, double laserRngM,
    simCore::EarthModelCalculations earthModel, const CoordinateConverter& cc,
    int numPoints)
{
  // Check if the host is in the gate
  if (positionInGate(gateHostLLA, laserHostLLA, gAzimuthRad, gElevRad, gWidthRad, gHeightRad, gMinRangeM, gMaxRangeM, earthModel, cc))
  {
    // Check the laser's points at equal intervals
    for (int ii = 0; ii < numPoints; ++ii)
    {
      double range = static_cast<double>(ii) * (laserRngM / static_cast<double>(numPoints));
      double endPoint[3] = {0.};
      simCore::Vec3 tmp;
      simCore::calculateGeodeticEndPoint(laserHostLLA, laserAzRad, laserElRad, range, tmp);
      tmp.toD3(endPoint);

      // Check if the point is in the gate
      if (!positionInGate(gateHostLLA, Vec3(endPoint), gAzimuthRad, gElevRad, gWidthRad, gHeightRad, gMinRangeM, gMaxRangeM, earthModel, cc))
        return false;
    }

    // We reached here, no points were found outside the gate
    return true;
  }

  // At least one of the laser's endpoints was not in the gate
  return false;
}

}