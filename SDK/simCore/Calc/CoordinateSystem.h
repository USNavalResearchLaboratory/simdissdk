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
#ifndef SIMCORE_CALC_COORDINATESYSTEM_H
#define SIMCORE_CALC_COORDINATESYSTEM_H

#include <string>
#include "simCore/Common/Common.h"

/// Container for enumerations and constants relating to coordinate system calculations and conversion
namespace simCore
{
  /// Enumeration of supported coordinate system types
  enum CoordinateSystem
  {
    COORD_SYS_NONE,     ///< Coordinate system is not defined
    COORD_SYS_NED,      ///< Scaled Flat Earth NED coordinate system: +X=North, +Y=East, +Z=Down Flag
    COORD_SYS_NWU,      ///< Scaled Flat Earth NWU coordinate system: +X=North, +Y=West, +Z=Up Flag
    COORD_SYS_ENU,      ///< Scaled Flat Earth ENU coordinate system: +X=East, +Y=North, +Z=Up Flag
    COORD_SYS_LLA,      ///< Geodetic coordinate system; degrees decimal
    COORD_SYS_ECEF,     ///< Earth Centered, Earth Fixed Geocentric coordinate system: based on WGS-84
    COORD_SYS_ECI,      ///< Earth Centered Inertial Geocentric coordinate system: based on WGS-84
    COORD_SYS_XEAST,    ///< Flat Earth: +X=East, Tangent Plane
    COORD_SYS_GTP,      ///< Flat Earth: Generic Tangent Plane, User defined X-Y rotation and tangential offset
    COORD_SYS_MAX       ///< Maximum number of available coordinate systems
  };

  /// Enumeration of Earth model representations used in calculations
  enum EarthModelCalculations
  {
    WGS_84 = 0,             ///< Earth modeled as a WGS-84 ellipsoid
    FLAT_EARTH,             ///< Earth modeled as a flat plane, scaled based on latitude
    TANGENT_PLANE_WGS_84,   ///< Earth modeled as a flat plane tangent to a point on the WGS-84 ellipsoid
    PERFECT_SPHERE          ///< Earth modeled as a perfect sphere
  };

  /// Enumeration of horizon types for distance calculations
  enum HorizonCalculations
  {
    GEOMETRIC_HORIZON = 0,  ///< Distance to horizon without refraction
    OPTICAL_HORIZON,        ///< Distance to horizon with refraction
    RADAR_HORIZON           ///< Distance to horizon with refraction using Effective Earth radius (4/3)
  };

  //---WGS-84 constants from NIMA TR8350.2, amendment 1, 3 Jan 2000
  inline constexpr double WGS_A   = 6378137.0;                 ///< Semi-major axis of the earth (m)
  inline constexpr double WGS_E   = 0.081819190842622;         ///< Earth eccentricity of ellipsoid
  inline constexpr double WGS_ESQ = 0.00669437999014;          ///< Ellipsoid eccentricity squared: E^2, also calculable as (A2-B2)/A2
  inline constexpr double WGS_F = 1.0/298.257223563;           ///< Earth flattening constant
  inline constexpr double WGS_B = WGS_A * (1.0 - WGS_F);       ///< (m) Semi-minor axis of the earth: (1.0 - F)*A
  inline constexpr double WGS_A2 = WGS_A * WGS_A;              ///< (m^2) Semi-major axis squared: A*A
  inline constexpr double WGS_B2 = WGS_B * WGS_B;              ///< (m^2) Semi-minor axis squared: B*B
  inline constexpr double WGS_EP2 = (WGS_A2 - WGS_B2)/WGS_B2;  ///<  E'^2 = (A2-B2)/B2
  inline constexpr double WGS_ESQC = 1.0 - WGS_ESQ;            ///<  1.0 - ESQ

  //---Earth related constants
  inline constexpr double EARTH_RADIUS = WGS_A;                    ///< (m) Spherical earth radius
  inline constexpr double EARTH_ROTATION_RATE = 7292115.1467e-11;  ///< (rad/sec) Earth's rotation rate: International Astronomical Union (IAU) GRS 67
  inline constexpr double LATLON_ERR_TOL_DOUBLE = 1.0e-10;         ///< floating point error tolerance for geodetic angle conversions

  /**
   * Given a coordinate system, returns an appropriate string constant.
   * @param coordSystem Coordinate system to get string value; COORD_SYS_MAX and COORD_SYS_NONE not supported.
   * @return String representation of the coordinate system
   */
  SDKCORE_EXPORT std::string coordinateSystemToString(simCore::CoordinateSystem coordSystem);

  /**
   * Given a coordinate system string constant, returns the appropriate system, returning 0 on success.
   * Inverse of simCore::coordinateSystemToString(), accepting strings that it returns.
   * @param str Coordinate string to process, from output of coordinateSystemToString().  Also accepts, for
   *   legacy reasons, LLA_DMD and LLA_DMS, returning a valid COORD_SYS_LLA flag.
   * @param outSystem Output parameter for the coordinate system string.
   * @return 0 on success, non-zero on error.  In error conditions, outSystem is initialized to simCore::COORD_SYS_LLA.
   */
  SDKCORE_EXPORT int coordinateSystemFromString(const std::string& str, simCore::CoordinateSystem& outSystem);

} // End of namespace simCore

#endif /* SIMCORE_CALC_COORDINATESYSTEM_H */
