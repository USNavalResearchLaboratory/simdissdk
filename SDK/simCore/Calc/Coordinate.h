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
#ifndef SIMCORE_CALC_COORDINATE_H
#define SIMCORE_CALC_COORDINATE_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/CoordinateSystem.h"

namespace simCore
{
  /**
  * @brief Container for position, velocity, orientation and acceleration of a coordinate projection
  *
  * Position, velocity, orientation and acceleration vectors of a coordinate
  * projection. Designed to help manage input and output from CoordConvert
  */
  class SDKCORE_EXPORT Coordinate
  {
  public:
    Coordinate();
    /// value constructor
    Coordinate(CoordinateSystem system, const Vec3 &pos, double elapsedECITime = 0.0);
    /// value constructor
    Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, double elapsedECITime = 0.0);
    /// value constructor
    Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, const Vec3 &vel, double elapsedECITime = 0.0);
    /// value constructor
    Coordinate(CoordinateSystem system, const Vec3 &pos, const Vec3 &ori, const Vec3 &vel, const Vec3 &acc, double elapsedECITime = 0.0);
    /// copy constructor
    Coordinate(const Coordinate &coord);
    virtual ~Coordinate() {}

    /// assignment operator
    Coordinate& operator=(const Coordinate &coord);

    /// reset all values to default settings
    void clear();

    /**
    * Sets the coordinate system for the coordinate values,
    * sets the elapsed time since the ECI frame was defined,
    * and resets all other values to default settings
    *
    * @param[in] system Coordinate system for the coordinate values
    * @param[in] elapsedEciTime ECI time (sec)
    */
    void clear(CoordinateSystem system, double elapsedEciTime);

    /// clear the position field
    void clearPosition() { pos_.zero(); }
    /// clear the optional orientation field
    void clearOrientation() { hasOri_ = false; ori_.zero(); }
    /// clear the optional velocity field
    void clearVelocity() { hasVel_ = false; vel_.zero(); }
    /// clear the optional acceleration field
    void clearAcceleration() { hasAcc_ = false; acc_.zero(); }

    //---set individual properties

    /**
    * @brief Sets the coordinate system for the coordinate values
    *
    * @param[in] system Coordinate system for the coordinate values
    */
    void setCoordinateSystem(CoordinateSystem system) { system_ = system; }

    /**
    * @brief Sets the individual position state components
    *
    * @param[in] x Sets position x component (m)
    * @param[in] y Sets position y component (m)
    * @param[in] z Sets position z component (m)
    */
    void setPosition(double x, double y, double z);

    /**
    * @brief Sets the individual position state components
    *
    * @param[in] lat Sets position latitude component (rad)
    * @param[in] lon Sets position longitude component (rad)
    * @param[in] alt Sets position altitude component (m)
    */
    void setPositionLLA(double lat, double lon, double alt);

    /**
    * @brief Sets the position state vector
    *
    * @param[in] pos Vector representing position as {0:x|lat|range, 1:y|lon|az, 2:z|alt|el}
    */
    void setPosition(const Vec3 &pos);

    /**
    * @brief Sets the individual orientation state components
    *
    * @param[in] yaw Sets orientation yaw component (rad)
    * @param[in] pitch Sets orientation pitch component (rad)
    * @param[in] roll Sets orientation roll component (rad)
    */
    void setOrientation(double yaw, double pitch, double roll);

    /**
    * @brief Sets the individual Euler orientation state components
    *
    * @param[in] psi Sets Euler psi component (rad)
    * @param[in] theta Sets Euler theta component (rad)
    * @param[in] phi Sets Euler phi component (rad)
    */
    void setOrientationEuler(double psi, double theta, double phi);

    /**
    * @brief Sets the orientation state vector
    *
    * @param[in] ori Vector representing orientation as {0:yaw|psi, 1:pitch|theta, 2:roll|phi}
    */
    void setOrientation(const Vec3 &ori);

    /**
    * @brief Sets the individual velocity state components
    *
    * @param[in] x Sets velocity x component (m/sec)
    * @param[in] y Sets velocity y component (m/sec)
    * @param[in] z Sets velocity z component (m/sec)
    */
    void setVelocity(double x, double y, double z);

    /**
    * @brief Sets the velocity state vector
    *
    * @param[in] vel Vector representing velocity as {0:x, 1:y, 2:z}
    */
    void setVelocity(const Vec3 &vel);

    /**
    * @brief Sets the individual acceleration state components
    *
    * @param[in] x Sets acceleration x component (m/sec^2)
    * @param[in] y Sets acceleration y component (m/sec^2)
    * @param[in] z Sets acceleration z component (m/sec^2)
    */
    void setAcceleration(double x, double y, double z);

    /**
    * @brief Sets the acceleration state vector
    *
    * @param[in] acc Vector representing acceleration as {0:x, 1:y, 2:z}
    */
    void setAcceleration(const Vec3 &acc);

    /**
    * @brief Sets the elapsed time since the ECI frame was defined
    *
    * @param[in] elapsedEciTime ECI time (sec)
    */
    void setElapsedEciTime(double elapsedEciTime);

    /**
    * @brief Returns whether or not Coordinate has orientation
    *
    * @return bool true if orientation exists
    */
    bool hasOrientation() const { return hasOri_; }

    /**
    * @brief Returns whether or not Coordinate has velocity
    *
    * @return bool true if velocity exists
    */
    bool hasVelocity() const { return hasVel_; }

    /**
    * @brief Returns whether or not Coordinate has acceleration
    *
    * @return bool true if acceleration exists
    */
    bool hasAcceleration() const {return hasAcc_;}

    /**
    * @brief Returns coordinate system for Coordinate
    *
    * @return CoordinateSystem
    */
    CoordinateSystem coordinateSystem() const { return system_; }

    /**
    * @brief Returns position state vector for Coordinate
    *
    * @return Vec3 Position state vector
    */
    const Vec3 &position() const { return pos_; }

    /**
    * @brief Returns orientation state vector for Coordinate
    *
    * @return Vec3 Orientation state vector
    */
    const Vec3 &orientation() const { return ori_; }

    /**
    * @brief Returns velocity state vector for Coordinate
    *
    * @return Vec3 Velocity state vector
    */
    const Vec3 &velocity() const { return vel_; }

    /**
    * @brief Returns acceleration state vector for Coordinate
    *
    * @return Vec3 Acceleration state vector
    */
    const Vec3 &acceleration() const { return acc_; }

    /**
    * @brief Returns elapsed ECI time in seconds
    *
    * @return double Elapsed ECI time in seconds
    */
    double elapsedEciTime() const { return elapsedEciTime_; }

    /**
    * @brief Returns position X component
    *
    * @return double position X component (m)
    */
    double x() const { return pos_.x(); }

    /**
    * @brief Returns position Y component
    *
    * @return double position Y component (m)
    */
    double y() const { return pos_.y(); }

    /**
    * @brief Returns position Z component
    *
    * @return double position Z component (m)
    */
    double z() const { return pos_.z(); }

    /**
    * @brief Returns position latitude component
    *
    * @return double position latitude component (rad)
    */
    double lat() const { return pos_.lat(); }

    /**
    * @brief Returns position longitude component
    *
    * @return double position longitude component (rad)
    */
    double lon() const { return pos_.lon(); }

    /**
    * @brief Returns position altitude component
    *
    * @return double position altitude component (m)
    */
    double alt() const { return pos_.alt(); }

    /**
    * @brief Returns orientation yaw component
    *
    * @return double orientation yaw component (rad)
    */
    double yaw() const { return ori_.yaw(); }

    /**
    * @brief Returns orientation pitch component
    *
    * @return double orientation pitch component (rad)
    */
    double pitch() const { return ori_.pitch(); }

    /**
    * @brief Returns orientation roll component
    *
    * @return double orientation roll component (rad)
    */
    double roll() const { return ori_.roll(); }

    /**
    * @brief Returns orientation psi component
    *
    * @return double orientation psi component (rad)
    */
    double psi() const { return ori_.psi(); }

    /**
    * @brief Returns orientation theta component
    *
    * @return double orientation theta component (rad)
    */
    double theta() const { return ori_.theta(); }

    /**
    * @brief Returns orientation phi component
    *
    * @return double orientation phi component (rad)
    */
    double phi() const { return ori_.phi(); }

    /**
    * @brief Returns velocity X component
    *
    * @return double velocity X component (m/sec)
    */
    double vx() const { return vel_.x(); }

    /**
    * @brief Returns velocity Y component
    *
    * @return double velocity Y component (m/sec)
    */
    double vy() const { return vel_.y(); }

    /**
    * @brief Returns velocity Z component
    *
    * @return double velocity Z component (m/sec)
    */
    double vz() const { return vel_.z(); }

    /**
    * @brief Returns acceleration X component
    *
    * @return double acceleration X component (m/sec^2)
    */
    double ax() const { return acc_.x(); }

    /**
    * @brief Returns acceleration Y component
    *
    * @return double acceleration Y component (m/sec^2)
    */
    double ay() const { return acc_.y(); }

    /**
    * @brief Returns acceleration Z component
    *
    * @return double acceleration Z component (m/sec^2)
    */
    double az() const { return acc_.z(); }

  private:
    CoordinateSystem system_;

    Vec3 pos_;                ///< position: radians and meter for geodetic, meters for remaining coordinates
    Vec3 vel_;                ///< velocity: meters/sec
    Vec3 ori_;                ///< orientation: radians
    Vec3 acc_;                ///< acceleration: meters/sec^2

    double elapsedEciTime_;   ///< elapsed time since definition of ECI reference frame (Greenwich Mean Sidereal Time) (sec)

    bool hasVel_;             ///< velocity is valid
    bool hasOri_;             ///< orientation is valid
    bool hasAcc_;             ///< acceleration is valid

  };

} // End namespace simCore

#endif /* SIMCORE_CALC_COORDINATE_H */
