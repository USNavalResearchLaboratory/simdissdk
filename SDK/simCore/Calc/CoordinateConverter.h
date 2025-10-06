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
#ifndef SIMCORE_CALC_COORDCONVERT_H
#define SIMCORE_CALC_COORDCONVERT_H

#include <cassert>

#include "simCore/Common/Common.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Coordinate.h"

namespace simCore
{
  class Dcm;
  /**
  * @brief Performs coordinate conversions between various projections.
  *
  * Contains data required to convert and store results of conversions
  * between various projections. Supported coordinate conversions
  * are ECEF, Geodetic, Flat Earth (ENU, NED, NWU) and X-East Tangent Plane.
  *
  * Earth-Centered Earth-Fixed (ECEF):  The earth model used is based
  * on a WGS-84 ellipsoidal earth model.  WGS-84 earth model is a
  * geocentric right-handed rectangular coordinate system in which
  * the origin is the center of the earth.  The +X-axis lies in the
  * equatorial plane and points toward the Greenwich meridian. The
  * +Y-axis lies in the equatorial plane and points toward 90 degrees
  * east longitude. Finally, the +Z-axis is coincident with the earth's
  * polar axis and is directed toward the north pole.
  *
  * Geodetic:  Latitude(rad), Longitude(rad) & Altitude(m).  Latitude
  * is a number that specifies location in the North-South direction.
  * Longitude specifies location in the East-West direction.  Altitude
  * specifies location above/below the ellipsoid (surface).  This system
  * is aligned with the Earth such that:
  *   ENU = +X (Lon) is East, +Y (Lat) is North, and +Z is up.  Course is
  * CW about Z
  *
  * Scaled Flat Earth:  The earth's surface is projected (warped) onto
  * an X-Y plane of a Cartesian coordinate system, with the reference
  * origin at some specified lat/lon surface location.  This system can
  * be aligned with the Earth such that:
  *   ENU = +X is East, +Y is North, and +Z is up.  Course is CW about Z
  *   NED = +X is North, +Y is East, and +Z is down.  Course is CW about Z
  *   NWU = +X is North, +Y is West, and +Z is up.  Course is CW about Z
  * The scaling of the latitude and longitude values into the flat earth
  * system is based on the values of the reference origin.  The resulting
  * scaled flat earth only maintains proper scale, direction, distance and
  * area within a short range of the reference origin.
  * NOTE: reference origin values at/near the poles are degenerate for these systems.
  *
  * Tangent Plane:  A Cartesian coordinate system based on a flat plane
  * tangent to the earth's surface at a specific reference origin.  This
  * system is aligned with the Earth such that:
  *   ENU = +X is East, +Y is North, and +Z is up.  Course is CW about Z
  * Lines of equal distortion are concentric about the origin.  Further
  * distance from the origin, the greater the distortion.
  *
  * Velocity, Acceleration and Orientation/Euler angles can be converted as well.
  * The input/return units are:
  *   Velocity (m/s)
  *   Acceleration (m/s^2)
  *   Orientation/Euler (rad)
  *
  *  Orientation/Euler Angle notation:
  *
  *  Yaw (psi) :  rotation about inertial Z to align inertial X-axis with body X-axis in azimuth
  *  Positive yaw  :  right turn
  *
  *  Pitch (theta) :  rotation about the new inertial Y-axis to align inertial X-axis with body X-axis
  *  Positive pitch:  nose up
  *
  *  Roll  (phi) :  rotation about the new inertial X-axis to align inertial Z-axis with body Z-axis
  *  Positive roll :  right wing down (clockwise)
  *
  *  When converting to/from a Scaled Flat Earth or Tangent Plane, a reference latitude and longitude
  *  must be established first.
  *
  */

  /** Enumeration of supported local level frame (LLF) types.  A LLF represents a vehicle's
  * attitude, velocity and acceleration when on or near the surface of the Earth.  This
  * frame is also known as the local geodetic horizon or navigation frame.
  */
  enum LocalLevelFrame
  {
    LOCAL_LEVEL_FRAME_NED,    ///< Local level NED frame: +X=North, +Y=East, +Z=Down, perpendicular to Earth surface
    LOCAL_LEVEL_FRAME_NWU,    ///< Local level NWU frame: +X=North, +Y=West, +Z=Up, perpendicular to Earth surface
    LOCAL_LEVEL_FRAME_ENU     ///< Local level ENU frame: +X=East, +Y=North, +Z=Up, perpendicular to Earth surface
  };

  class SDKCORE_EXPORT CoordinateConverter
  {
  public:

    /// Status of reference origin, used for conversions between various coordinate systems
    enum ReferenceOriginStatus
    {
      /** Reference origin has not been set, conversions not supported */
      REF_ORIGIN_NOT_SET = 0,
      /** Reference origin has been set, conversions are supported */
      REF_ORIGIN_SET = 1,
      /**
      * Reference origin has been set, however conversions using a scaled flat earth
      * system (ENU/NED/NWU) will fail due to an origin at/near the pole.
      */
      REF_ORIGIN_SCALED_FLAT_EARTH_DEGENERATE = 2
    };

    CoordinateConverter();
    /// copy constructor
    CoordinateConverter(const CoordinateConverter& other);
    virtual ~CoordinateConverter() {}

    /**
    * @brief Copies a coordinate convert to another coordinate convert
    *
    * Copies a CoordConvert to this CoordConvert
    * @param[in ] other Source to copy data from
    * @return Reference to the this
    */
    CoordinateConverter &operator=(const CoordinateConverter& other);

    /**
    * @brief Returns whether or not reference origin has been set
    *
    * @return true if reference origin has been set, false if not set
    */
    bool hasReferenceOrigin() const { return refOriginStatus_ != REF_ORIGIN_NOT_SET; }

    //---accessors
    /**
    * @brief Returns reference latitude of CoordinateConverter
    *
    * @return reference latitude (rad)
    */
    double referenceLat() const;

    /**
    * @brief Returns reference longitude of CoordinateConverter
    *
    * @return reference longitude (rad)
    */
    double referenceLon() const;

    /**
    * @brief Returns reference altitude of CoordinateConverter
    *
    * @return reference altitude (m)
    */
    double referenceAlt() const;

    /**
    * @brief Returns reference origin of CoordinateConverter
    *
    * @return reference lat/lon/alt (rad/rad/m)
    */
    const Vec3& referenceOrigin() const;

    /**
    * @brief Returns the calculated radius of earth at reference longitude (m)
    *
    * @return calculated radius of earth at reference longitude (m)
    */
    double lonRadius() const;

    /**
    * @brief Returns the calculated radius of earth at reference latitude (m)
    *
    * @return calculated radius of earth at reference latitude (m)
    */
    double latRadius() const;

    /**
    * @brief Returns the X offset of the tangent plane origin (m)
    *
    * @return X offset of the tangent plane origin (m)
    */
    double tangentPlaneOffsetX() const { return tangentPlaneOffsetX_; }

    /**
    * @brief Returns the Y offset of the tangent plane origin (m)
    *
    * @return Y offset of the tangent plane origin (m)
    */
    double tangentPlaneOffsetY() const { return tangentPlaneOffsetY_; }

    /**
    * @brief Returns the rotation angle of the X-Y tangent plane origin (rad)
    *
    * @return rotation angle of the X-Y tangent plane origin (rad)
    */
    double tangentPlaneRotation() const { return tangentPlaneRotation_; }

    /**
    * @brief Set the X offset of the tangent plane origin (m)
    *
    * @param[in ] x offset of the tangent plane origin (m)
    */
    void setTangentPlaneOffsetX(double x) { tangentPlaneOffsetX_ = x; }

    /**
    * @brief Set the Y offset of the tangent plane origin (m)
    *
    * @param[in ] y offset of the tangent plane origin (m)
    */
    void setTangentPlaneOffsetY(double y) { tangentPlaneOffsetY_ = y; }

    /**
    * @brief Set the rotation angle of the X-Y tangent plane origin (rad)
    *
    * @param[in ] d rotation angle of the X-Y tangent plane origin (rad)
    */
    void setTangentPlaneRotation(double d);

    /**
    * @brief Set the reference origin (using degrees for latitude and longitude)
    * Changes the reference origin for the coordinate converter.  This is a potentially expensive operation
    * because rotation matrices must be regenerated when the origin changes.  Avoid calling this frequently.
    *
    * @param[in ] lat Latitude value of origin, in degrees decimal
    * @param[in ] lon Longitude value of origin, in degrees decimal
    * @param[in ] alt Altitude value of origin, in meters
    */
    void setReferenceOriginDegrees(double lat = 0.0, double lon = 0.0, double alt = 0.0);

    /**
    * @brief Set the reference origin (using degrees for latitude and longitude)
    * Changes the reference origin for the coordinate converter.  This is a potentially expensive operation
    * because rotation matrices must be regenerated when the origin changes.  Avoid calling this frequently.
    *
    * @param[in ] lla Vector that contains the latitude (degrees decimal), longitude (degrees decimal), and altitude (meters) values of the reference origin as {0:lat, 1:lon, 2:alt}
    */
    void setReferenceOriginDegrees(const Vec3 &lla);

    /**
    * @brief Set the reference origin (using radians for the latitude and longitude)
    * Changes the reference origin for the coordinate converter.  This is a potentially expensive operation
    * because rotation matrices must be regenerated when the origin changes.  Avoid calling this frequently.
    *
    * @param[in ] lat Latitude value of origin, in radians
    * @param[in ] lon Longitude value of origin, in radians
    * @param[in ] alt Altitude value of origin, in meters
    */
    void setReferenceOrigin(double lat = 0.0, double lon = 0.0, double alt = 0.0);

    /**
    * @brief Set the reference origin (using radians for the latitude and longitude)
    * Changes the reference origin for the coordinate converter.  This is a potentially expensive operation
    * because rotation matrices must be regenerated when the origin changes.  Avoid calling this frequently.
    *
    * @param[in ] lla Vector that contains the latitude (radians), longitude (radians), and altitude (meters) values of the reference origin as {0:lat, 1:lon, 2:alt}
    */
    void setReferenceOrigin(const Vec3 &lla);

    /**
    * @brief Set the X & Y offsets of the tangent plane origin and the rotation angle of the X-Y plane about the origin
    *
    * Set the X & Y offsets of the tangent plane origin and the rotation angle of the X-Y plane about the origin
    * @param[in ] xOffset X position offset of tangent plane origin in meters
    * @param[in ] yOffset Y position offset of tangent plane origin in meters
    * @param[in ] angle rotation angle of the X-Y tangent plane about the origin in radians
    */
    void setTangentPlaneOffsets(double xOffset, double yOffset, double angle = 0.0);

    /**
    * @brief Perform coordinate conversions between the supported projections
    *
    * Perform coordinate conversions between the supported projections, velocity, acceleration & Euler are referenced to a NED system
    * @param[in ] inCoord Coordinate value class that contains the position, orientation, velocity and acceleration data of the incoming projection system; when converting tp/from ECI, inCoord must contain a valid elapsedEciTime value
    * @param[out] outCoord Coordinate value class that contains the position, orientation, velocity and acceleration data of the outgoing projection system
    * @param[in ] outSystem Flag that describes the outgoing projection system
    * @return 0 on success, !0 on failure
    */
    int convert(const Coordinate &inCoord, Coordinate &outCoord, CoordinateSystem outSystem) const;

    //------------------------------------------------------------------------
    // Static functions which perform coordinate system conversions but do not
    // maintain any state information in the CoordinateConverter class

    /**
    * @brief Compute local to Earth rotation matrix based on input coordinate system
    *
    * Compute local to Earth rotation matrix based on input coordinate system
    * @param[in ] lat latitude value (rad)
    * @param[in ] lon longitude value (rad)
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @param[out] localToEarth 3x3 rotation matrix
    * @pre out param valid
    */
    static void setLocalToEarthMatrix(double lat, double lon, LocalLevelFrame localLevelFrame, double localToEarth[][3]);

    /**
    * @brief Compute local to Earth DCM rotation matrix based on input coordinate system
    *
    * Compute local to Earth DCM based on input coordinate system
    * @param[in ] lat latitude value (rad)
    * @param[in ] lon longitude value (rad)
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @param[out] localToEarth 3x3 DCM
    */
    static void setLocalToEarthDcm(double lat, double lon, LocalLevelFrame localLevelFrame, Dcm& localToEarth);

    /**
    * @brief Swaps input vector between NED and ENU systems
    *
    * Swaps input vector between NED and ENU systems
    * @param[in ] inVec vector in the form of a NED or ENU system
    * @param[out] outVec vector swapped to the opposite of the input (NED->ENU or ENU->NED)
    * @pre out param valid
    */
    static void swapNedEnu(const Vec3 &inVec, Vec3 &outVec);

    /**
    * @brief Swaps input coordinate between NED and ENU systems
    *
    * Swaps input coordinate between NED and ENU systems
    * @param[in ] inCoord coordinate in the form of a NED or ENU system
    * @param[out] outCoord coordinate swapped to the opposite of the input (NED->ENU or ENU->NED)
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int swapNedEnu(const Coordinate &inCoord, Coordinate &outCoord);

    /**
    * @brief Swaps input vector between NED and NWU systems
    *
    * Swaps input vector between NED and NWU systems
    * @param[in ] inVec vector in the form of a NED or NWU system
    * @param[out] outVec vector swapped to the opposite of the input (NED->NWU or NWU->NED)
    * @pre out param valid
    */
    static void swapNedNwu(const Vec3 &inVec, Vec3 &outVec);

    /**
    * @brief Swaps input coordinate between NED and NWU systems
    *
    * Swaps input coordinate between NED and NWU systems
    * @param[in ] inCoord coordinate in the form of a NED or NWU system
    * @param[out] outCoord coordinate swapped to the opposite of the input (NED->NWU or NWU->NED)
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int swapNedNwu(const Coordinate &inCoord, Coordinate &outCoord);

    /**
    * @brief Converts input ENU vector to a NWU vector
    *
    * Converts input ENU vector to a NWU vector
    * @param[in ] inVec vector in the form of a ENU system
    * @param[out] outVec vector swapped to a NWU system
    * @pre out param valid
    */
    static void convertEnuToNwu(const Vec3 &inVec, Vec3 &outVec);

    /**
    * @brief Converts input ENU coordinate to a NWU coordinate
    *
    * Converts input ENU coordinate to a NWU vector
    * @param[in ] inCoord coordinate in the form of a ENU system
    * @param[out] outCoord coordinate swapped to a NWU system
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertEnuToNwu(const Coordinate &inCoord, Coordinate &outCoord);

    /**
    * @brief Converts input NWU vector to an ENU vector
    *
    * Converts input NWU vector to an ENU vector
    * @param[in ] inVec vector in the form of a NWU system
    * @param[out] outVec vector swapped to an ENU system
    * @pre out param valid
    */
    static void convertNwuToEnu(const Vec3 &inVec, Vec3 &outVec);

    /**
    * @brief Converts input NWU coordinate to an ENU coordinate
    *
    * Converts input NWU coordinate to an ENU vector
    * @param[in ] inCoord coordinate in the form of a NWU system
    * @param[out] outCoord coordinate swapped to an ENU system
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertNwuToEnu(const Coordinate &inCoord, Coordinate &outCoord);

    /**
    * @brief Converts a geodetic coordinate to an Earth Centered Earth Fixed (ECEF) coordinate
    *
    * Converts a geodetic coordinate to an Earth Centered Earth Fixed (ECEF) coordinate
    * @param[in ] llaCoord
    * @param[out] ecefCoord
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertGeodeticToEcef(const Coordinate &llaCoord, Coordinate &ecefCoord, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) coordinate to a geodetic coordinate
    *
    * Converts an Earth Centered Earth Fixed (ECEF) coordinate to a geodetic coordinate
    * @param[in ] ecefCoord
    * @param[out] llaCoord
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertEcefToGeodetic(const Coordinate &ecefCoord, Coordinate &llaCoord, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

    /**
    * @brief Converts an Earth Centered Inertial (ECI) coordinate to an Earth Centered Earth Fixed (ECEF) coordinate
    *
    * Converts an Earth Centered Inertial (ECI) coordinate to an Earth Centered Earth Fixed (ECEF) coordinate
    * @param[in ] eciCoord
    * @param[out] ecefCoord
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertEciToEcef(const Coordinate &eciCoord, Coordinate &ecefCoord);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) coordinate to an Earth Centered Inertial (ECI) coordinate
    *
    * Converts an Earth Centered Earth Fixed (ECEF) coordinate to an Earth Centered Inertial (ECI) coordinate
    * @param[in ] ecefCoord
    * @param[out] eciCoord
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertEcefToEci(const Coordinate &ecefCoord, Coordinate &eciCoord);

    /**
    * @brief Converts a geodetic position to an Earth Centered Earth Fixed (ECEF) position
    *
    * Converts a geodetic position to an Earth Centered Earth Fixed (ECEF) position
    * @param[in ] llaPos latitude (rad), longitude (rad), altitude (m)
    * @param[out] ecefPos X (m), Y (m), Z (m)
    * @param[in ] semiMajor semi major Earth radius
    * @param[in ] eccentricitySquared Earth eccentricity, squared
    * @pre out param valid
    */
    static void convertGeodeticPosToEcef(const Vec3 &llaPos, Vec3 &ecefPos, double semiMajor = WGS_A, double eccentricitySquared = WGS_ESQ);

    /**
    * @brief Converts geodetic Euler angles to an Earth Centered Earth Fixed (ECEF) Euler orientation
    *
    * Converts geodetic Euler angles to an Earth Centered Earth Fixed (ECEF) Euler orientation
    * @param[in ] llaPos
    * @param[in ] llaOri
    * @param[out] ecefOri
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @pre out param valid
    */
    static void convertGeodeticOriToEcef(const Vec3 &llaPos, const Vec3 &llaOri, Vec3 &ecefOri, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) position to geodetic
    *
    * Converts an Earth Centered Earth Fixed (ECEF) position to geodetic
    * @param[in ] ecefPos
    * @param[out] llaPos
    * @return 0 on success, !0 on failure
    * @pre out param valid
    */
    static int convertEcefToGeodeticPos(const Vec3 &ecefPos, Vec3 &llaPos);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) velocity to geodetic
    *
    * Converts an Earth Centered Earth Fixed (ECEF) velocity to geodetic
    * @param[in ] llaPos
    * @param[in ] ecefVel
    * @param[out] llaVel
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @pre out param valid
    */
    static void convertEcefToGeodeticVel(const Vec3 &llaPos, const Vec3 &ecefVel, Vec3 &llaVel, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) orientation to geodetic
    *
    * Converts Earth Centered Earth Fixed (ECEF) orientation to geodetic Euler angles
    * @param[in ] llaPos
    * @param[in ] ecefOri
    * @param[out] llaOri
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @pre out param valid
    */
    static void convertEcefToGeodeticOri(const Vec3 &llaPos, const Vec3 &ecefOri, Vec3 &llaOri, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) acceleration to geodetic
    *
    * Converts Earth Centered Earth Fixed (ECEF) acceleration to geodetic
    * @param[in ] llaPos
    * @param[in ] ecefAcc
    * @param[out] llaAcc
    * @param[in ] localLevelFrame alignment of local geodetic horizon system (NED, ENU, NWU)
    * @pre out param valid
    */
    static void convertEcefToGeodeticAccel(const Vec3 &llaPos, const Vec3 &ecefAcc, Vec3 &llaAcc, LocalLevelFrame localLevelFrame = LOCAL_LEVEL_FRAME_NED);

  private: // data
    double latRadius_;                   /// radius of earth at reference  latitude (m)
    double lonRadius_;                   /// radius of earth at reference longitude (m)
    double invLatRadius_;                /// inverse radius of earth at reference  latitude (1/m)
    double invLonRadius_;                /// inverse radius of earth at reference longitude (1/m)

    Vec3 referenceOrigin_;               /// reference origin, lat(rad), lon(rad), alt relative to surface of ellipsoid (m)
    double rotationMatrixNED_[3][3];     /// NED orientation rotation matrix (tangent plane)
    double rotationMatrixENU_[3][3];     /// ENU rotation matrix (tangent plane)
    Vec3 tangentPlaneTranslation_;       /// ENU tangent plane translation vector

    double tangentPlaneOffsetX_;         /// X offset of tangent plane origin, relative to the tangential point (m)
    double tangentPlaneOffsetY_;         /// X offset of tangent plane origin, relative to the tangential point (m)
    double tangentPlaneRotation_;        /// rotation angle of X-Y tangent plane, rotation about the tangential point (rad)
    double cosTPR_;                      /// cosine of rotation angle of X-Y tangent plane
    double sinTPR_;                      /// sine of rotation angle of X-Y tangent plane

    ReferenceOriginStatus refOriginStatus_; /// current status of reference origin

  private: // methods

    /**
    * @brief Converts an ECEF or ECI coordinate to an ECI or ECEF coordinate, by performing rotation around Z-axis
    *
    * Converts an ECEF or ECI coordinate to an ECI or ECEF coordinate, by performing rotation around Z-axis
    * Direction of rotation is positive for ECEF->ECI, negative for ECI->ECEF
    * @param[in ] inCoord
    * @param[out] outCoord
    * @pre out param valid
    */
    static void convertEciEcef_(const Coordinate &inCoord, Coordinate &outCoord);

    /**
    * @brief Calculates the scaled earth radii and rotation matrices
    *
    * Calculates the scaled earth radii based on a given lat/lon origin in addition to the tangent plane rotation/translation matrices
    * @param[in ] lla reference origin specified as lat, lon, and alt (rad/rad/m)
    */
    void calculateReferenceRadius_(const Vec3 &lla);

    /**
    * @brief Converts a geodetic projection to a scaled flat earth projection
    *
    * @param[in ] llaCoord geodetic coordinate
    * @param[out] flatCoord scaled flat earth coordinate
    * @param[in ] system flat earth system (NED, ENU, NWU)
    * @return 0 on success, !0 on failure
    * @pre flatCoord param valid and reference origin must be set
    */
    int convertGeodeticToFlat_(const Coordinate &llaCoord, Coordinate &flatCoord, CoordinateSystem system) const;

    /**
    * @brief Converts a scaled flat earth projection to geodetic projection
    *
    * @param[in ] flatCoord scaled flat earth coordinate
    * @param[out] llaCoord geodetic coordinate
    * @return 0 on success, !0 on failure
    * @pre llaCoord param valid and reference origin must be set
    */
    int convertFlatToGeodetic_(const Coordinate &flatCoord, Coordinate &llaCoord) const;

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) projection to a scaled flat earth projection
    *
    * @param[in ] ecefCoord ECEF coordinate
    * @param[out] flatCoord scaled flat earth coordinate
    * @param[in ] system flat earth system (NED, ENU, NWU)
    * @return 0 on success, !0 on failure
    * @pre flatCoord param valid and reference origin must be set
    */
    int convertEcefToFlat_(const Coordinate &ecefCoord, Coordinate &flatCoord, CoordinateSystem system) const;

    /**
    * @brief Converts a scaled flat earth projection to an Earth Centered Earth Fixed (ECEF) projection
    *
    * @param[in ] flatCoord scaled flat earth coordinate
    * @param[out] ecefCoord ECEF coordinate
    * @return 0 on success, !0 on failure
    * @pre ecefCoord param valid and reference origin must be set
    */
    int convertFlatToEcef_(const Coordinate &flatCoord, Coordinate &ecefCoord) const;

    /**
    * @brief Converts a tangent plane projection to an Earth Centered Earth Fixed (ECEF) projection
    *
    * @param[in ] tpCoord tangent plane coordinate
    * @param[out] ecefCoord ECEF coordinate
    * @return 0 on success, !0 on failure
    * @pre ecefCoord param valid and reference origin must be set
    */
    int convertXEastToEcef_(const Coordinate &tpCoord, Coordinate &ecefCoord) const;

    /**
    * @brief Converts an Earth Centered Earth Fixed (ECEF) projection to a tangent plane projection
    *
    * @param[in ] ecefCoord ECEF coordinate
    * @param[out] tpCoord tangent plane coordinate
    * @return 0 on success, !0 on failure
    * @pre tpCoord param valid and reference origin must be set
    */
    int convertEcefToXEast_(const Coordinate &ecefCoord, Coordinate &tpCoord) const;

    /**
    * @brief Converts a geodetic projection to a tangent plane projection
    *
    * @param[in ] llaCoord geodetic coordinate
    * @param[out] tpCoord tangent plane coordinate
    * @return 0 on success, !0 on failure
    * @pre tpCoord param valid and reference origin must be set
    */
    int convertGeodeticToXEast_(const Coordinate &llaCoord, Coordinate &tpCoord) const;

    /**
    * @brief Converts a tangent plane projection to a geodetic projection
    *
    * @param[in ] tpCoord tangent plane coordinate
    * @param[out] llaCoord geodetic coordinate
    * @return 0 on success, !0 on failure
    * @pre tpCoord != llaCoord and reference origin must be set
    */
    int convertXEastToGeodetic_(const Coordinate &tpCoord, Coordinate &llaCoord) const;

    /**
    * @brief Converts a X-East tangent plane projection to a translated and rotated tangent plane projection
    *
    * @param[out] tpCoord tangent plane coordinate
    * @pre tpCoord param valid and reference origin must be set
    */
    void applyTPOffsetRotate_(Coordinate &tpCoord) const;

    /// Reverses X&Y offsets and rotation from a rotated & translated tangent plane to an X-EAST tangent plane
    /**
    * @brief Converts a translated and rotated tangent plane projection to a X-East tangent plane projection
    *
    * @param[out] gtpCoord generic (translated and rotated) tangent plane coordinate
    * @pre gtpCoord param valid and reference origin must be set
    */
    void reverseTPOffsetRotate_(Coordinate &gtpCoord) const;
  };

} // End namespace simCore

#endif /* SIMCORE_CALC_COORDCONVERT_H */
