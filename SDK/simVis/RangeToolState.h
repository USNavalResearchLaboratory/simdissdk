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
#ifndef SIMVIS_STATE_H
#define SIMVIS_STATE_H

#include "osg/MatrixTransform"
#include "osgEarth/optional"
#include "osgEarth/MapNode"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Time/TimeClass.h"
#include "simData/ObjectId.h"

namespace simRF { class RFPropagationFacade; }

namespace simVis
{

class EntityNode;
class ScenarioManager;

/// Number of enumerations in State::Coord
const size_t COORD_CACHE_SIZE = 16;

/**
* Entity state needed to do basic Range calculations
*/
struct SDKVIS_EXPORT EntityState
{
  simCore::Vec3 lla_;  ///< Lat, lon, alt in rad, rad, m
  simCore::Vec3 ypr_;  ///< Yaw, pitch, roll in rad, rad, rad
  simCore::Vec3 vel_;  ///< X, Y and Z velocities in m/s
  simData::ObjectType type_;  ///< The type of the entity
  simData::ObjectId id_;   ///< Unique ID of the entity
  simData::ObjectId hostId_;   ///< Unique ID of the host entity; for platforms and custom renderings hostId_ == id_

  EntityState()
    : type_(simData::NONE),
    id_(0),
    hostId_(0)
  {
  }
  virtual ~EntityState()
  {
  }
};

/**
* Internal state class for Graphic rendering. Graphic primitives and
* Measurements receive a State object internally when rendering in order
* to track object locations and other shared data.
*/
struct SDKVIS_EXPORT RangeToolState
{
  /**
   * Constructor
   * @param beginEntity State for the begin entity; takes ownership of the memory
   * @param endEntity State for the end entity; takes ownership of the memory
   */
  RangeToolState(EntityState* beginEntity, EntityState* endEntity);
  virtual ~RangeToolState();

  /**
  * Coordinate data saved in the coord_ member variable for later use
  * Local coordinate mean LTP with OBJ 0 at the origin
  */
  enum Coord
  {
    COORD_OBJ_0,  ///< The "to" object in local coordinates
    COORD_OBJ_1,  ///< The "from" object in local coordinates
    COORD_OBJ_0_0HAE,  ///< The "to" object forced to zero altitude, in local coordinates
    COORD_OBJ_1_0HAE,  ///< The "from" object forced to zero altitude, in local coordinates
    COORD_OBJ_0_AT_OBJ_1_ALT,  ///< The "to" object at the "from" object altitude, in local coordinates
    COORD_OBJ_1_AT_OBJ_0_ALT,  ///< The "from" object at the "to" object altitude, in local coordinates
    COORD_DR,  ///< Down Range inflection point (the corner of the "L") in local coordinates
    COORD_VEL_AZIM_DR, ///< Velocity Azimuth Down Range inflection point (the corner of the "L") in local coordinates
    COORD_BEAM_LLA_0,  ///< The "to" object for beam calculation (closest point) in LLA (rad, rad, m)
    COORD_BEAM_LLA_1,  ///< TThe "from" object for beam calculation (closest point) in LLA (rad, rad, m)
    COORD_BEAM_0,  ///< The "to" object for beam calculation (closest point) in local coordinates
    COORD_BEAM_1,  ///< TThe "from" object for beam calculation (closest point) in local coordinates
    COORD_BEAM_0_0HAE,  ///< The "to" beam forced to zero altitude, in local coordinates
    COORD_BEAM_1_0HAE,  ///< The "from" beam forced to zero altitude, in local coordinates
    COORD_BEAM_0_AT_BEAM_1_ALT,  ///< The "to" beam at the "from" object altitude, in local coordinates
    COORD_BEAM_1_AT_BEAM_0_ALT,  ///< The "from" beam at the "to" object altitude, in local coordinates
  };

  /**
  * Calculates and caches the requested values
  * @param coord the type value to calculate and cache
  * @return the requested values, the type of values detailed in Coord
  */
  virtual osg::Vec3d coord(Coord coord);

  /**
  * Converts osg::Vec3d to simCore::Vec3
  * @param point the data to convert
  * @return simCore::Vec3 of the given point
  */
  simCore::Vec3 osg2simCore(const osg::Vec3d& point) const;

  /**
  * Converts simCore::Vec3 to osg::Vec3d
  * @param point the data to convert
  * @return osg::Vec3d of the given point
  */
  ///@return osg:Vec3d of the given point
  osg::Vec3d simCore2osg(const simCore::Vec3& point) const;

  /// Interpolate 'numSeg' positions between 'lla0' and 'lla1', adding them to 'verts'
  void line(
    const simCore::Vec3& lla0,
    const simCore::Vec3& lla1,
    double               altOffset,
    osg::Vec3Array*      verts);

  /**
  * Generate a list of lat/lon points between lla0 and lla1 at intervals of at most distDelta.  List does not include lla0 or lla1.
  * If lla0 == lla1, list will be empty.
  * @param lla0 Start point to generate intermediate points from
  * @param lla1 End point to generate intermediate points towards
  * @param distDelta Maximum distance between intermediate points.  Actual distance between may be lower.
  * @param[out] llaPointsOut Vector of intermediate points.  All points have an altitude of 0
  */
  void intermediatePoints(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double distDelta, std::vector<simCore::Vec3>& llaPointsOut) const;

  /// Returns the midpoint between the two given positions
  simCore::Vec3 midPoint(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset);

  ///@return the given lla to relative values scaled to the local frame (xyz)
  osg::Vec3 lla2local(double lat_rad, double lon_rad, double alt_m) const;

  ///@return lla values for the given position relative to the local frame
  simCore::Vec3 local2lla(const osg::Vec3d& local);

  ///@return the local/ENU vector produced by rotating the start->end vector by specified az, rotated in the ltp
  osg::Vec3d rotateEndVec(double az);


  /**
  * Resets the coord cache to initial state
  */
  void resetCoordCache();

  /**@name internal state (TODO: make private)
  *@{
  */
  osg::Matrixd                     world2local_;     // world to local tangent plane
  osg::Matrixd                     local2world_;     // reverse of above
  EntityState*                     beginEntity_;
  EntityState*                     endEntity_;
  simCore::EarthModelCalculations  earthModel_;
  simCore::CoordinateConverter     coordConv_;
  osgEarth::optional<osg::Vec3d>   coord_[COORD_CACHE_SIZE];  // number of enumerations in State::Coord
  simCore::TimeStamp timeStamp_; // the timeStamp of the last update
  osg::observer_ptr<osgEarth::MapNode>  mapNode_;
  ///@}
};

}

#endif
