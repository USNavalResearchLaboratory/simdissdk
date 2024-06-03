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
#ifndef SIMCORE_CALC_RANGE_CALCULATIONS_H
#define SIMCORE_CALC_RANGE_CALCULATIONS_H

/** @file
* This file defines a number of calculations that can be performed between two locations or entities
* in space, including distances and angles.  Unless otherwise specified, units are in meters for distance,
* radians for latitude/longitude and other angles, and meters per second for velocity.
*/

#include "simCore/Common/Common.h"
#include "simCore/Calc/NumericalAnalysis.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/CoordinateSystem.h"

namespace simCore
{
  /**
  * @brief Calculates the relative azimuth, elevation, and composite angles between two entities
  *
  * Calculates the relative azimuth, elevation, and composite angles from one entity to another in the given coordinate frame
  * along the from entity's line of sight.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] fromOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity, to which the angles are calculated
  * @param[out] azim Azimuth value from one entity to another along the from's line of sight
  * @param[out] elev Elevation value from one entity to another along the from's line of sight
  * @param[out] cmp Composite value from one entity to another along the from's line of sight; composite is sometimes known as the bore sight angle
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @pre one of the azim, elev and cmp params must be valid and coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT void calculateRelAzEl(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, double* azim, double* elev, double* cmp, const EarthModelCalculations model, const CoordinateConverter* coordConv);

  /**
  * @brief Calculates the absolute azimuth, elevation, and composite angles between two entities
  *
  * Calculates the absolute azimuth, elevation, and composite angles from one entity to another in the given coordinate frame.
  * The calculation is performed with 0 degrees at True North
  * @param[in ] fromLla Vector of latitude, longitude, altitude (double[3]) that describes current position for the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity, to which the angles are calculated
  * @param[out] azim Azimuth value from one entity to another
  * @param[out] elev Elevation value from one entity to another
  * @param[out] cmp Composite value from one entity to another
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @pre one of the azim, elev and cmp params must be valid and coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT void calculateAbsAzEl(const Vec3 &fromLla, const Vec3 &toLla, double* azim, double* elev, double* cmp, const EarthModelCalculations model, const CoordinateConverter* coordConv);

  /**
  * @brief Calculates the slant distance between two entities
  *
  * Calculates the slant distance between two positions in space in the given coordinate system.  Order of entities (from/to)
  * will not affect the calculation.
  * @param[in ] fromLla Location in space consisting of latitude, longitude, altitude that is the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @return Slant distance between two objects in meters, with a 0 return indicating an error (or equality of points)
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateSlant(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv);

  /**
  * @brief Calculates the ground distance between two entities
  *
  * Calculates the ground distance from one object to another.  This is calculated by "dropping a line" to the surface of the
  * earth for both entities and calculating the distance of the line that connects the two surface points.  Order of the entities
  * (from/to) will not affect the calculation.
  * @param[in ] fromLla Location in space consisting of latitude, longitude, altitude that is the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @return Ground distance between two objects in meters, with a 0 return indicating an error (or equality of points)
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateGroundDist(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv);

  /**
  * @brief Calculates the altitude difference between two entities
  *
  * Calculates the altitude difference from one object to another.  Order of the entities (from/to) will negate the
  * result of the calculation.  A "higher" to altitude will return a positive value.
  * @param[in ] fromLla Location in space consisting of latitude, longitude, altitude that is the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @return Altitude difference between two objects in meters, with a 0 return indicating an error (or equality of points); if toLla is higher than fromLla, this value should be positive
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateAltitude(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv);

  /**
  * @brief Calculates the downrange, crossrange, and down values between two entities
  *
  * Calculates the downrange, crossrange, and down values between two entities in space along the pointing angle specified by the
  * from entity's state.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] yaw Yaw (heading) pointing angle for the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity, to which the angles are calculated
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[out] downRng Range along the x-axis normal to the to entity, where the x-axis is aligned with the yaw of the from entity
  * @param[out] crossRng Distance measured along a line whose direction is either 90deg CW (positive) or 90deg CCW (neg) to the projection of from's yaw into a horizontal plane
  * @param[out] downValue Shortest distance between a plane tangent to the earth at from's position and altitude and the to entity
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT void calculateDRCRDownValue(const Vec3 &fromLla, const double &yaw, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, double* downRng, double* crossRng, double* downValue);

  /**
  * @brief Calculates the geodesic downrange and crossrange values between two entities
  *
  * Calculates the geodesic downrange and crossrange values between two entities in space referenced to the from entity's state using the Sodano method.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] yaw The yaw (heading) pointing angle for the 'from' entity
  * @param[in ] toLla Location in space consisting of latitude, longitude, altitude that is the 'to' entity, to which the angles are calculated
  * @param[out] downRng Range (m) along the x-axis normal to the to entity, where the x-axis is aligned with the yaw of the from entity
  * @param[out] crossRng Range (m) measured along a line whose direction is either 90deg CW (positive) or 90deg CCW (neg) to the projection of from's yaw into a horizontal plane
  * @param[in ] minDR Minimum downrange value (m) for algorithm to converge
  * @param[in ] minCR Minimum crossrange value (m) for algorithm to converge
  * @return the calculation's convergence status
  */
  SDKCORE_EXPORT NumericalSearchType calculateGeodesicDRCR(const Vec3 &fromLla, const double &yaw, const Vec3 &toLla, double* downRng, double* crossRng, const double minDR = 0.0005, const double minCR = 0.0005);

  /**
  * @brief Calculates the closing velocity between two entities
  *
  * Calculates the closing velocity, which is the velocity at which the from and to entity are moving towards one another.  Closing
  * velocity is positive when the distance between two entities is decreasing (moving towards one another), and negative when moving apart.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] toLla Vector of latitude, longitude, and altitude that describes current position for the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[in ] fromVel Velocity X/Y/Z for the from entity in m/s in an LLA frame
  * @param[in ] toVel Velocity X/Y/Z for the to entity in m/s in an LLA frame
  * @return Closing velocity in m/s based on input values.
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateClosingVelocity(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel);

  /**
  * @brief Calculates the velocity delta between two entities
  *
  * Calculates the velocity delta, which is the difference of the squares of the differences of velocity components and is always
  * positive.  This is similar to the closing velocity, but does not alter the return value based on the velocity component that
  * is along the pointing vector.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] toLla Vector of latitude, longitude, and altitude that describes current position for the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[in ] fromVel Velocity X/Y/Z for the from entity in m/s in an LLA frame
  * @param[in ] toVel Velocity X/Y/Z for the to entity in m/s in an LLA frame
  * @return Velocity Delta in m/s based on input values.
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateVelocityDelta(const Vec3 &fromLla, const Vec3 &toLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel);

  /**
  * @brief Calculates the range rate between two entities
  *
  * Calculates the range rate in m/sec between two entities.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] fromOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'from' entity
  * @param[in ] toLla Vector of latitude, longitude, and altitude that describes current position for the 'to' entity
  * @param[in ] toOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[in ] fromVel Velocity X/Y/Z for the from entity in m/s in an LLA frame
  * @param[in ] toVel Velocity X/Y/Z for the to entity in m/s in an LLA frame
  * @return Range rate in m/sec
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateRangeRate(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, const Vec3 &toOriLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel);

  /**
  * @brief Calculates the bearing rate between two entities
  *
  * Calculates the bearing rate in rad/sec between two entities.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] fromOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'from' entity
  * @param[in ] toLla Vector of latitude, longitude, and altitude that describes current position for the 'to' entity
  * @param[in ] toOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'to' entity
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[in ] fromVel Velocity X/Y/Z for the from entity in m/s in an LLA frame
  * @param[in ] toVel Velocity X/Y/Z for the to entity in m/s in an LLA frame
  * @return Bearing rate in rad/sec
  * @pre coordConv must be valid if model is flat earth or tangent plane
  */
  SDKCORE_EXPORT double calculateBearingRate(const Vec3 &fromLla, const Vec3 &fromOriLla, const Vec3 &toLla, const Vec3 &toOriLla, const EarthModelCalculations model, const CoordinateConverter* coordConv, const Vec3 &fromVel, const Vec3 &toVel);

  /**
  * @brief Calculates the aspect angle between two entities
  *
  * Calculates the aspect angle from one object to another along the longitudinal axis of the 'to' object.
  * The return angles are relative to a body axis from the 'to' entity.
  * @param[in ] fromLla Vector of latitude, longitude, and altitude that describes current position for the 'from' entity
  * @param[in ] toLla Vector of latitude, longitude, and altitude that describes current position for the 'to' entity
  * @param[in ] toOriLla Vector of yaw, pitch, roll that describes current pointing angles for the 'to' entity
  * @return aspect angle in radians
  */
  SDKCORE_EXPORT double calculateAspectAngle(const Vec3 &fromLla, const Vec3 &toLla, const Vec3 &toOriLla);

  //////////////////////////////////////////////////////////////////////
  ////////////////// Helper functions for Calculation //////////////////
  //////////////////////////////////////////////////////////////////////

  /**
  * @brief Converts the given perfect sphere earth XYZ position to X-East Tangent Plane position
  *
  * Converts the given perfect sphere earth XYZ values to X-East Tangent Plane values, given
  * the tangent plane's latitude, longitude, and altitude.  Note: If tangent plane's perfect
  * sphere Earth XYZ values are available, they can be given for a faster calculation.
  * @param[in ] llaVec Latitude, longitude, and altitude values of the tangent plane origin
  * @param[in ] sphereVec Given XYZ values to convert to an ENU location
  * @param[out] tpVec Distance in X-East that the sphereVec is from the llaVec location
  * @param[in ] sphereTpOrigin Perfect sphere coordinates of the tangent plane origin, if known.  If this value is not known, it will be calculated from llaVec.  Calculation is faster if this is provided.
  */
  SDKCORE_EXPORT void sphere2TangentPlane(const Vec3& llaVec, const Vec3& sphereVec, Vec3& tpVec, const Vec3* sphereTpOrigin=nullptr);

  /**
  * @brief Converts the given X-East Tangent Plane position to a perfect sphere earth XYZ position
  *
  * Converts the given X-East Tangent Plane vector to perfect sphere Earth XYZ vector, given
  * the tangent planes latitude, longitude, and altitude.  Note: If tangent plane's perfect
  * sphere Earth XYZ values are available, they can be given for a faster calculation.
  * @param[in ] llaVec Latitude, longitude, and altitude values of the tangent plane origin
  * @param[in ] tpVec Given X-East values to convert to a perfect sphere XYZ location
  * @param[out] sphereVec XYZ vector generated by conversion from the X-East location
  * @param[in ] sphereTpOrigin Perfect sphere coordinates of the tangent plane origin, if known.  If this value is not known, it will be calculated from llaVec.  Calculation is faster if this is provided.
  */
  SDKCORE_EXPORT void tangentPlane2Sphere(const Vec3 &llaVec, const Vec3 &tpVec, Vec3& sphereVec, const Vec3* sphereTpOrigin=nullptr);

  /**
  * @brief Converts a geodetic position into a perfect sphere XYZ position
  *
  * Converts a geodetic position into a perfect sphere XYZ position.  For the sphere model:
  *  Each axis corresponds to the following positions (lat,lon):
  *  +X = (0,-180) -X = (0,0)
  *  +Y = (0, -90) -Y = (0,90)
  *  +Z = (90,0)   -Z = (-90,0)
  * @param[in ] lat Latitude of point to convert
  * @param[in ] lon Longitude of point to convert
  * @param[in ] alt Altitude from the surface of point to convert
  * @param[out] point XYZ position of the point on the perfect sphere
  */
  SDKCORE_EXPORT void geodeticToSpherical(const double lat, const double lon, const double alt, Vec3 &point);

  /**
  * @brief Calculates the relative angles between an ENU vector and a set of geodetic Euler angles
  *
  * Calculates the relative angles between an ENU vector and a set of geodetic Euler angles
  * @param[in ] enuVec East, North, Up (ENU) based pointing vector
  * @param[in ] refOri Reference orientation based on geodetic Euler angles (YPR) to compute relative angles
  * @param[out] azim Relative azimuth value
  * @param[out] elev Relative elevation value
  * @param[out] cmp Relative composite angle
  * @pre one of the azim, elev or cmp params must be valid
  */
  SDKCORE_EXPORT void calculateRelAng(const Vec3 &enuVec, const Vec3 &refOri, double* azim, double* elev, double* cmp);

  /**
  * @brief Calculates the body relative angles from a set of host geodetic Euler angles to true az/el vector
  *
  * Calculates the body relative angles from a set of host geodetic Euler angles to a true az/el vector.  This can
  * be used (for example) to calculate a beam's body az/el given its true az/el and host yaw/pitch/roll.  This
  * function is related to simCore::rotateEulerAngle(), which can calculate a beam's true az/el given a beam's
  * body az/el and host yaw/pitch/roll.
  * @param[in ] trueAz True azimuth component (rad)
  * @param[in ] trueEl True elevation component (rad)
  * @param[in ] refOri Reference orientation based on geodetic Euler angles (YPR) to compute relative angles (rad)
  * @param[out] azim Relative azimuth value (rad)
  * @param[out] elev Relative elevation value (rad)
  * @param[out] cmp Relative composite angle (rad)
  * @pre one of the azim, elev or cmp params must be valid
  */
  SDKCORE_EXPORT void calculateRelAngToTrueAzEl(double trueAz, double trueEl, const Vec3& refOri, double* azim, double* elev, double* cmp);

  /**
  * @brief Find the lat/lon of a point 'dist' away along the given angle 'azfwd'
  *
  * This function implements Sodano's direct solution algorithm to determine geodetic
  * longitude and latitude and back azimuth given a geodetic reference longitude
  * and latitude, a geodesic length, a forward azimuth  and an ellipsoid definition.
  * @param[in ] refLat Geodetic latitude of reference point (rad)
  * @param[in ] refLon Geodetic longitude of reference point (rad)
  * @param[in ] refAlt Height above ellipsoid of reference point (m)
  * @param[in ] dist Geodesic length from reference to second point along the forward azimuth (m)
  * @param[in ] azfwd Forward azimuth from reference to second point (rad)
  * @param[out] latOut Geodetic latitude of point 2 (rad)
  * @param[out] lonOut Geodetic longitude of point 2 (rad)
  * @param[out] azbck Backward azimuth from second point to reference (rad)
  * @pre one of the lat, lon or azbck params must be valid
  */
  SDKCORE_EXPORT void sodanoDirect(const double refLat, const double refLon, const double refAlt, const double dist, const double azfwd, double *latOut, double *lonOut, double *azbck=nullptr);

  /**
  * @brief Calculates the geodesic length, forward and backward azimuth using Sodano's indirect solution
  *
  * This function implements Sodano's indirect algorithm
  * to determine geodesic length or distance, forward
  * azimuth, and backward azimuth from a given pair of
  * geodetic longitudes and latitudes and a given ellipsoid.
  * @param[in ] refLat Geodetic latitude of reference point (rad)
  * @param[in ] refLon Geodetic longitude of reference point (rad)
  * @param[in ] refAlt Height above ellipsoid of reference point (m)
  * @param[in ] lat Geodetic latitude of point 2 (rad)
  * @param[in ] lon Geodetic longitude of point 2 (rad)
  * @param[out] azfwd Forward azimuth from reference to second point clockwise from North (rad), not calculated if nullptr
  * @param[out] azbck Backward azimuth from second point to reference (rad), not calculated if nullptr
  * @return dist, Geodesic length or distance from reference to second point (m)
  */
  SDKCORE_EXPORT double sodanoInverse(const double refLat, const double refLon, const double refAlt, const double lat, const double lon, double *azfwd=nullptr, double *azbck=nullptr);

  /**
  * @brief Calculates the Earth's radius at the given latitude
  *
  * Calculates the distance from the Earth's center to a point on the spheroid
  * surface at the geodetic latitude using WGS-84 parameters.
  * @param[in ] latitude Geodetic latitude location (rad)
  * @return radius of Earth in meters.
  */
  SDKCORE_EXPORT double calculateEarthRadius(const double latitude);

  /**
  * @brief Clamps a point in ECEF space to the surface of the WGS84 ellipsoid.
  *
  * Computes a point referenced to WGS84 whose value is on the geodetic surface.
  * @param ecef ECEF coordinate in meters
  * @return Surface-clamped ECEF value in meters, clamped to WGS84 ellipsoid.
  */
  SDKCORE_EXPORT Vec3 clampEcefPointToGeodeticSurface(const Vec3& ecef);

  /**
  * @brief Calculates the horizon distance for either geometric, optical or radar
  *
  * Calculates the horizon distance for either geometric, optical or radar.
  * Equations derived from a perfect sphere using Pythagorean Theorem
  * Optical horizon uses a 1.06 earth radius to account for refraction effects (constant lapse rate and homogeneous atmosphere from EW Handbook).
  * Radar horizon uses a 4/3 earth radius to account for refraction effects.
  * Earth radius is based on WGS-84 ellipsoid
  * @param[in ] lla Observer's geodetic location in lat, lon and alt (rad, rad, m)
  * @param[in ] horizonType HorizonCalculations specifies type of horizon to use in calculation
  * @param[in ] opticalRadius effective Earth radius scalar for optical horizon.
  * @param[in ] rfRadius effective Earth radius scalar for RF horizon
  * @return distance to specified horizon in meters.
  */
  SDKCORE_EXPORT double calculateHorizonDist(const Vec3& lla, const HorizonCalculations horizonType=GEOMETRIC_HORIZON, const double opticalRadius=1.06, const double rfRadius=4./3.);

  /**
  * @brief Converts the input locations to the specified coordinate system
  *
  * Converts the input locations to the specified coordinate system.
  * @param[in ] fromState Location of the 'from' entity in geodetic (LLA)
  * @param[in ] toState Location of the 'to' entity in geodetic (LLA)
  * @param[in ] model Earth model to perform the calculation in
  * @param[in ] coordConv If model is flat earth, then this must point to an initialized CoordinateConverter structure with a reference origin set. Optional for WGS84 models
  * @param[out] fromPos Converted location in specified frame
  * @param[out] toPos Converted location in specified frame
  * @return true on success, false on failure.
  * @pre coordConv if model is flat earth
  */
  SDKCORE_EXPORT bool convertLocations(const Coordinate& fromState, const Coordinate& toState, const EarthModelCalculations model, const CoordinateConverter* coordConv, Coordinate& fromPos, Coordinate& toPos);

  /**
  * @brief Computes the X component of the body unit vector
  *
  * Given a yaw and pitch value and a yaw-pitch-roll (1-2-3) rotation sequence
  * The X component of the vehicle's body unit vector is computed
  * @param[in ] yaw Yaw (psi) rotation in radians
  * @param[in ] pitch Pitch (theta) rotation in radians
  * @param[out] vecX Vec3 representing the X component of the body unit vector
  */
  SDKCORE_EXPORT void calculateBodyUnitX(const double yaw, const double pitch, Vec3 &vecX);

 /**
  * @brief Computes the Y component of the body unit vector
  *
  * Given a yaw, pitch and roll value and a yaw-pitch-roll (1-2-3) rotation sequence
  * The Y component of the vehicle's body unit vector is computed
  * @param[in ] yaw Yaw (psi) rotation in radians
  * @param[in ] pitch Pitch (theta) rotation in radians
  * @param[in ] roll Roll (phi) rotation in radians
  * @param[out] vecY Vec3 representing the Y component of the body unit vector
  */
 SDKCORE_EXPORT void calculateBodyUnitY(double yaw, double pitch, double roll, Vec3 &vecY);

 /**
  * @brief Computes the Z component of the body unit vector
  *
  * Given a yaw, pitch and roll value and a yaw-pitch-roll (1-2-3) rotation sequence
  * The Z component of the vehicle's body unit vector is computed
  * @param[in ] yaw Yaw (psi) rotation in radians
  * @param[in ] pitch Pitch (theta) rotation in radians
  * @param[in ] roll Roll (phi) rotation in radians
  * @param[out] vecZ Vec3 representing the Z component of the body unit vector
  */
 SDKCORE_EXPORT void calculateBodyUnitZ(double yaw, double pitch, double roll, Vec3 &vecZ);

  /**
  * @brief Decomposes the X component of the unit body vector into yaw and pitch angles
  *
  * Given the X component of the vehicle's body unit vector and a yaw-pitch-roll (1-2-3) rotation sequence
  * The yaw and pitch angles are computed
  * @param[in ] vecX Vec3 representing the X component of the body unit vector
  * @param[out] yawOut Yaw (psi) rotation in radians
  * @param[out] pitchOut Pitch (theta) rotation in radians
  */
  SDKCORE_EXPORT void calculateYawPitchFromBodyUnitX(const Vec3 &vecX, double &yawOut, double &pitchOut);

  /**
  * @brief Calculates an ENU geodetic velocity vector based on a local (moving) tangent plane
  *
  * Calculates an ENU geodetic velocity vector based on a local (moving) tangent plane
  * whose origin is set based on the first geodetic position value
  * @param[in ] currPos Current geodetic position
  * @param[in ] prevPos Previous geodetic position
  * @param[in ] deltaTime Time between positions (sec)
  * @param[out] velVec East, North, and Up based geodetic velocity vector
  */
  SDKCORE_EXPORT void calculateVelFromGeodeticPos(const Vec3 &currPos, const Vec3 &prevPos, const double deltaTime, Vec3 &velVec);

  /**
  * @brief Calculates an ENU geodetic velocity vector based on dp/dt, and flight path angle orientation from velocity
  *
  * Calculates an ENU geodetic velocity vector based on dp/dt, and flight path angle orientation from velocity.  This method
  * is suitable for positions that are relatively close together.  The error will accumulate with positions farther away.  This
  * method is not suitable for points that span far across the surface of the earth.
  * @param[in ] currPos Current position
  * @param[in ] prevPos Previous position
  * @param[in ] deltaTime Time difference between two data points (sec)
  * @param[in ] sysIn Coordinate system of input data points; ECI/GTP are not supported
  * @param[out] velOut Computed East, North, and Up based geodetic velocity vector based on dp/dt
  * @param[out] oriOut Derived flight path orientation based on velocity vector
  * @param[in ] refLLA Reference geodetic origin (needed for conversions between geocentric and local systems)
  * @param[in ] sysOut Coordinate system of output velocity and orientation; ECI/GTP are not supported
  * @return conversion success (sysIn != COORD_SYS_ECI/GTP), conversion not supported
  */
  SDKCORE_EXPORT bool calculateVelOriFromPos(const Vec3 &currPos, const Vec3 &prevPos, const double deltaTime, const CoordinateSystem sysIn, Vec3 &velOut, Vec3 &oriOut, const Vec3 &refLLA, const CoordinateSystem sysOut=COORD_SYS_XEAST);

  /**
  * @brief Calculates a geodetic Euler orientation from angles relative to another geodetic Euler orientation
  *
  * Calculates a geodetic Euler orientation from angles relative to another geodetic Euler orientation
  * @param[in ] hostYpr Host geodetic Euler orientation (rad,rad,rad)
  * @param[in ] relYpr Relative geodetic Euler orientation (rad,rad,rad)
  * @param[out] ypr Geodetic Euler orientation (rad/rad/rad)
  */
  SDKCORE_EXPORT void calculateGeodeticOriFromRelOri(const Vec3 &hostYpr, const Vec3 &relYpr, Vec3 &ypr);

  /**
  * Calculates a geodetic position from the given offset position and orientation vectors
  *
  * @param[in ] llaBgnPos LLA(rad/rad/meters) of the origin from which the offset will be calculated
  * @param[in ] bodyOriOffset Body orientation offset (Y, P, R) in radians
  * @param[in ] bodyPosOffset Body offset position (X, Y, Z) in meters
  * @param[out] offsetLla (rad/rad/meters) LLA position of offset
  */
  SDKCORE_EXPORT void calculateGeodeticOffsetPos(const simCore::Vec3& llaBgnPos, const simCore::Vec3& bodyOriOffset, const simCore::Vec3& bodyPosOffset, simCore::Vec3& offsetLla);

  /**
  * @brief Calculates the geodetic end point of a vector based on a specified azimuth, elevation and range from a given geodetic position
  *
  * Calculates the geodetic end point of a vector based on a specified azimuth, elevation and range from a given geodetic position
  * @param[in ] llaBgnPos Geodetic begin position of vector (rad/rad/m)
  * @param[in ] az Geodetic Euler azimuth of the vector (rad)
  * @param[in ] el Geodetic Euler elevation of the vector (rad)
  * @param[in ] rng Range of the vector (m)
  * @param[out] llaEndPos Geodetic end position output value (rad/rad/m)
  */
  SDKCORE_EXPORT void calculateGeodeticEndPoint(const Vec3 &llaBgnPos, const double az, const double el, const double rng, Vec3 &llaEndPos);

  /**
   * Calculates the midpoint between two LLA coordinates.  Accounts properly for wrapping around the antimeridian (dateline).  Provides
   * two variants on the calculuation:
   *   - Low resolution, simple average (accounting for dateline) that is good for determining a center position for a view that
   *     encompasses both points.  With this method, the parameter ordering matters, as the calculation considers a west-to-east
   *     ordering on parameters.  In the event that parameters are not west-to-east, the algorithm presumes a whole-earth swath.
   *   - High resolution based on the Sodano method that accounts for the curvature of the earth.  More appropriate for calculations
   *     requiring a midpoint along the shortest line between two points.  Because this method uses the Sodano method, it picks the
   *     shortest path between points.  The high resolution version will never give a midpoint further than quarter of the
   *     circumference of the earth away.
   * @param[in ] llaBgnPos Geodetic (radians, radians, meters) position of the start coordinate; considered left and below the llaEndPos.
   *   If the longitude is greater than llaEndPos, then the area is considered to wrap the antimeridian (dateline).
   * @param[in ] llaEndPos Geodetic (radians, radians, meters) position of the start coordinate; considered right and above the llaBgnPos.
   *   If the longitude is less than that of llaBgnPos, then the area is considered to wrap the antimeridian (dateline).
   * @param[in ] highResolution If true, a high resolution calculation is performed to calculate the midpoint, which is more expensive.  If
   *   false, the average of the positions is taken (accounting for the antimeridian), which is faster.  The high resolution version will use
   *   a great circle method, which is best for calculating the midpoint along the path between the two points, but unsuitable for determining
   *   (for example) the optimum center position for fitting both points in a 3D view.
   * @param[out] midpoint Geodetic center position calculated by averaging the geodetic values.  The altitude is given as the average of the input
   *   altitudes and will not cut through the earth (unless one or more input values is below 0.0 altitude).
   * @param[out] wrapsDateline Set to true if non-null and if the area wraps the antimeridian (dateline).  nullptr is acceptable.
   */
  SDKCORE_EXPORT void calculateGeodeticMidPoint(const Vec3& llaBgnPos, const Vec3& llaEndPos, bool highResolution, simCore::Vec3& midpoint, bool* wrapsDateline=nullptr);

  /**
   * @brief Calculates flight path angles from an ENU geodetic velocity vector
   *
   * Calculates flight path angles from an ENU geodetic velocity vector
   * @param[in ] velVec East, North, and Up based geodetic velocity vector
   * @param[out] fpa Calculated flight path angles (heading, pitch, roll=0)
   */
  SDKCORE_EXPORT void calculateFlightPathAngles(const Vec3 &velVec, Vec3 &fpa);

  /**
   * @brief Calculates an ENU based geodetic velocity vector from speed, heading and pitch (flight path angles)
   *
   * Calculates an ENU based geodetic velocity vector from speed, heading and pitch (flight path angles)
   * @param[in ] speed Speed (m/sec)
   * @param[in ] heading (rad) heading (relative to North)
   * @param[in ] pitch (rad) pitch (relative to Horizon)
   * @param[out] velVec Calculated East, North, and Up based geodetic velocity vector
   */
  SDKCORE_EXPORT void calculateVelocity(const double speed, const double heading, const double pitch, Vec3 &velVec);

  /**
   * Calculates the angle of attack, side slip, and total angle of attack from a ENU
   * geodetic velocity vector and a set of geodetic Euler angles (yaw, pitch, roll)
   * @param[in ] enuVel East, North, and Up geodetic velocity vector for the vehicle
   * @param[in ] ypr Yaw, pitch, roll based geodetic Euler angles in radians
   * @param[in ] useRoll Boolean, true: aerodynamic version, false: rocketry version (Air Ballistic Axis) that does not use roll
   * @param[out] aoa Vertical angle of attack for vehicle in radians; measure of difference between velocity and pointing direction
   * @param[out] ss Side slip angle for vehicle in radians; measure of difference between velocity and pointing direction
   * @param[out] totalAoA Total angle of attack for vehicle in radians
   */
  SDKCORE_EXPORT void calculateAoaSideslipTotalAoa(const Vec3& enuVel, const Vec3& ypr, const bool useRoll, double* aoa, double* ss, double* totalAoA);

  /**
   * Returns the distance between a line segment and a point. The calculations are done in COORD_SYS_XEAST so the results are only approximate.
   * The code is taken from SIMDIS 9 code SimVisBeam::GetClosestPoint and generalized for WGS-84.
   * @param[in ] startLla Start point of the line segment in (rad, rad, m) in WGS-84
   * @param[in ] endLla End point of the line segment in (rad, rad, m) in WGS-84
   * @param[in ] toLla The point for the distance calculation in (rad, rad, m) in WGS-84
   * @param[out] closestLla The point on the line segment closest to the toLla point
   * @return The distance, in meters, between a line segment and the toLla point.
   */
  SDKCORE_EXPORT double getClosestPoint(const simCore::Vec3& startLla, const simCore::Vec3& endLla, const simCore::Vec3& toLla, simCore::Vec3& closestLla);

  /**
   * Given an object and its gate, determines whether or not a given
   * position falls inside the gate.  A position on the edge is
   * considered outside.  This function only works with linear
   * range gates.
   * @param gateHostLLA LLA location of the entity that hosts the gate
   * @param positionLLA LLA location of the position to check if it's inside the gate
   * @param azimuthRad Gate azimuth in radians
   * @param elevRad Gate elevation in radians
   * @param widthRad Gate width in radians
   * @param heightRad Gate height in radians
   * @param minRangeM Gate minimum range in meters
   * @param maxRangeM Gate maximum range in meters
   * @param earthModel Earth model for calculations
   * @param cc If coordinateSystem is flat earth or tangent plane, then this must point to an initialized CoordConvert structure with a reference origin set. Optional for WGS84 models
   * @return true if given lla position does fall within gate, false otherwise
   */
  SDKCORE_EXPORT bool positionInGate(const simCore::Vec3& gateHostLLA, const simCore::Vec3& positionLLA,
    double azimuthRad, double elevRad, double widthRad, double heightRad, double minRangeM, double maxRangeM,
    simCore::EarthModelCalculations earthModel, const CoordinateConverter& cc);

  /**
   * Given a laser (line) and its gate, determines whether or not the entirety of
   * the laser falls within the gate.  This function only works with linear
   * range gates, as it is based upon positionInGate().
   * @param gateHostLLA LLA location of the entity that hosts the gate
   * @param laserHostLLA LLA location of the laser host entity to check if it's inside the gate
   * @param gAzimuthRad Gate azimuth in radians
   * @param gElevRad Gate elevation in radians
   * @param gWidthRad Gate width in radians
   * @param gHeightRad Gate height in radians
   * @param gMinRangeM Gate minimum range in meters
   * @param gMaxRangeM Gate maximum range in meters
   * @param laserAzRad Laser azimuth in radians from laserHostLLA
   * @param laserElRad Laser elevation in radians from laserHostLLA
   * @param laserRngM Laser range in meters from laserHostLLA
   * @param earthModel Coordinate system to perform the calculation in
   * @param cc If earthModel is flat earth or tangent plane, then this must point to an initialized CoordConvert structure with a reference origin set. Optional for WGS84 models
   * @param numPoints number of points to test for presence within the gate
   * @return true if given lla position does fall within gate, false otherwise
   */
  SDKCORE_EXPORT bool laserInGate(const simCore::Vec3& gateHostLLA, const simCore::Vec3& laserHostLLA,
    double gAzimuthRad, double gElevRad, double gWidthRad, double gHeightRad, double gMinRangeM, double gMaxRangeM,
    double laserAzRad, double laserElRad, double laserRngM,
    simCore::EarthModelCalculations earthModel, const CoordinateConverter& cc,
    int numPoints=20);
}

#endif /* SIMCORE_CALC_RANGE_CALCULATIONS_H */
