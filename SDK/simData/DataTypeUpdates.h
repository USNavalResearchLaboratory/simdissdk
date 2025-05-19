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
#ifndef DATATYPE_UPDATES_H
#define DATATYPE_UPDATES_H

#include <limits>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"
#include "simData/DataTypeBasics.h"

// Although proto buffers are flexible, they consume too much memory
// Below are memory efficient definitions for the POSIT classes

namespace simData
{
/** Platform TSPI
  * (interface matches a Google protobuf message)
  */
class PlatformUpdate
{
public:
  /// copy over this with 'from'
  void CopyFrom(const PlatformUpdate& from) { if (&from == this) return; *this = from; }

  /**@name accessors -------------------------------------------------------
    *@{
    */
  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_position() const { return has_x() && has_y() && has_z(); }
  inline void position(simCore::Vec3& vec) const { vec.setX(x()); vec.setY(y()); vec.setZ(z()); }
  inline void setPosition(const simCore::Vec3& vec) { x_ = vec.x(); y_ = vec.y(); z_ = vec.z(); }

  inline bool has_x() const { return x_.has_value(); }
  inline void clear_x() { x_.reset(); }
  inline double x() const { return x_.value(); }
  inline void set_x(double value) { x_ = value; }

  inline bool has_y() const { return y_.has_value(); }
  inline void clear_y() { y_.reset(); }
  inline double y() const { return y_.value(); }
  inline void set_y(double value) { y_ = value; }

  inline bool has_z() const { return z_.has_value(); }
  inline void clear_z() { z_.reset(); }
  inline double z() const { return z_.value(); }
  inline void set_z(double value) { z_ = value; }

  inline bool has_orientation() const { return has_psi() && has_theta() && has_phi(); }
  inline void orientation(simCore::Vec3& vec) const { vec.setPsi(psi()); vec.setTheta(theta()); vec.setPhi(phi()); }
  inline void setOrientation(const simCore::Vec3& vec) { set_psi(vec.psi()); set_theta(vec.theta()); set_phi(vec.phi()); }

  inline bool has_psi() const { return psi_.has_value(); }
  inline void clear_psi() { psi_.reset(); }
  inline double psi() const { return psi_.value(); }
  inline void set_psi(double value) { psi_ = static_cast<float>(value); }

  inline bool has_theta() const { return theta_.has_value(); }
  inline void clear_theta() { theta_.reset(); }
  inline double theta() const { return theta_.value(); }
  inline void set_theta(double value) { theta_ = static_cast<float>(value); }

  inline bool has_phi() const { return phi_.has_value(); }
  inline void clear_phi() { phi_.reset(); }
  inline double phi() const { return phi_.value(); }
  inline void set_phi(double value) { phi_ = static_cast<float>(value); }

  inline bool has_velocity() const { return has_vx() && has_vy() && has_vz(); }
  inline void velocity(simCore::Vec3& vec) const { vec.setV0(vx()); vec.setV1(vy()); vec.setV2(vz()); }
  inline void setVelocity(const simCore::Vec3& vec) { set_vx(vec.x()); set_vy(vec.y()); set_vz(vec.z()); }

  inline bool has_vx() const { return vx_.has_value(); }
  inline void clear_vx() { vx_.reset(); }
  inline double vx() const { return vx_.value(); }
  inline void set_vx(double value) { vx_ = static_cast<float>(value); }

  inline bool has_vy() const { return vy_.has_value(); }
  inline void clear_vy() { vy_.reset(); }
  inline double vy() const { return vy_.value(); }
  inline void set_vy(double value) { vy_ = static_cast<float>(value); }

  inline bool has_vz() const { return vz_.has_value(); }
  inline void clear_vz() { vz_.reset(); }
  inline double vz() const { return vz_.value(); }
  inline void set_vz(double value) { vz_ = static_cast<float>(value); }
  ///@}

private:
  /// Seconds since the reference year
  OptionalDouble time_;

  /// Position is in ECEF coordinates, meters
  OptionalDouble x_;
  OptionalDouble y_;
  OptionalDouble z_;

  /// The following are declared as floats to save space
  /// Alignment of a body in 3D space, angles in radians; earth centric
  OptionalFloat psi_;
  OptionalFloat theta_;
  OptionalFloat phi_;

  /// 3D vector for velocity, m/s
  OptionalFloat vx_;
  OptionalFloat vy_;
  OptionalFloat vz_;
};

/// Beam time and RAE data
class BeamUpdate
{
public:
  void CopyFrom(const BeamUpdate& from) { if (&from == this) return; *this = from; }

  void Clear() { clear_time(); clear_range(); clear_azimuth(); clear_elevation(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_range() const { return range_.has_value(); }
  inline void clear_range() { range_.reset(); }
  inline double range() const { return range_.value(); }
  inline void set_range(double value) { range_ = value; }

  inline bool has_azimuth() const { return azimuth_.has_value(); }
  inline void clear_azimuth() { azimuth_.reset(); }
  inline double azimuth() const { return azimuth_.value(); }
  inline void set_azimuth(double value) { azimuth_ = value; }

  inline bool has_elevation() const { return elevation_.has_value(); }
  inline void clear_elevation() { elevation_.reset(); }
  inline double elevation() const { return elevation_.value(); }
  inline void set_elevation(double value) { elevation_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Range in meters from the platform origin
  OptionalDouble range_;
  /// Azimuth; relative to north for linear beams, or relative to platform orientation for body beams; radians
  OptionalDouble azimuth_;
  /// Elevation; relative to horizon for linear beams, or relative to platform orientation for body beams; radians
  OptionalDouble elevation_;
};


/// Gate time and data
class GateUpdate
{
public:
  void CopyFrom(const GateUpdate& from) { if (&from == this) return; *this = from; }

  void Clear() { clear_time(); clear_azimuth(); clear_elevation(); clear_width(); clear_height();
    clear_minrange(); clear_maxrange(); clear_centroid(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_azimuth() const { return azimuth_.has_value(); }
  inline void clear_azimuth() { azimuth_.reset(); }
  inline double azimuth() const { return azimuth_.value(); }
  inline void set_azimuth(double value) { azimuth_ = value; }

  inline bool has_elevation() const { return elevation_.has_value(); }
  inline void clear_elevation() { elevation_.reset(); }
  inline double elevation() const { return elevation_.value(); }
  inline void set_elevation(double value) { elevation_ = value; }

  inline bool has_width() const { return width_.has_value(); }
  inline void clear_width() { width_.reset(); }
  inline double width() const { return width_.value(); }
  inline void set_width(double value) { width_ = value; }

  inline bool has_height() const { return height_.has_value(); }
  inline void clear_height() { height_.reset(); }
  inline double height() const { return height_.value(); }
  inline void set_height(double value) { height_ = value; }

  inline bool has_minrange() const { return minRange_.has_value(); }
  inline void clear_minrange() { minRange_.reset(); }
  inline double minrange() const { return minRange_.value(); }
  inline void set_minrange(double value) { minRange_ = value; }

  inline bool has_maxrange() const { return maxRange_.has_value(); }
  inline void clear_maxrange() { maxRange_.reset(); }
  inline double maxrange() const { return maxRange_.value(); }
  inline void set_maxrange(double value) { maxRange_ = value; }

  inline bool has_centroid() const { return centroid_.has_value(); }
  inline void clear_centroid() { centroid_.reset(); }
  inline double centroid() const { return centroid_.value(); }
  inline void set_centroid(double value) { centroid_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Azimuth; relative to north for linear gates, or relative to platform orientation for body gates; radians
  OptionalDouble azimuth_;
  /// Elevation; relative to horizon for linear gates, or relative to platform orientation for body gates; radians
  OptionalDouble elevation_;
  /// Full width of the gate in radians
  OptionalDouble width_;
  /// Full height of the gate in radians
  OptionalDouble height_;
  /// Range in meters from the platform origin to start of gate
  OptionalDouble minRange_;
  /// Range in meters from the platform origin to end of gate
  OptionalDouble maxRange_;
  /// Range in meters from the platform origin to centroid; often the middle of start and end
  OptionalDouble centroid_;
};

/// Projector time and field of view values
class ProjectorUpdate
{
public:
  void CopyFrom(const ProjectorUpdate& from) { if (&from == this) return; *this = from; };

  void Clear() { clear_time(); clear_fov(); clear_hfov(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_fov() const { return fov_.has_value(); }
  inline void clear_fov() { fov_.reset(); }
  inline double fov() const { return fov_.value(); }
  inline void set_fov(double value) { fov_ = value; }

  inline bool has_hfov() const { return hFov_.has_value(); }
  inline void clear_hfov() { hFov_.reset(); }
  inline double hfov() const { return hFov_.value(); }
  inline void set_hfov(double value) { hFov_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Projector vertical field of view: radians
  OptionalDouble fov_;
  /// Projector horizontal field of view: radians; <= 0 means to calculate from aspect ratio
  OptionalDouble hFov_;
};

/// Custom Rendering update; not used, just a placeholder for the templates
class CustomRenderingUpdate
{
public:
  void CopyFrom(const CustomRenderingUpdate& from) { if (&from == this) return; *this = from; };

  void Clear() { clear_time(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
};

/// Laser time and orientation values
class LaserUpdate
{
public:
  void CopyFrom(const LaserUpdate& from) { if (&from == this) return; *this = from; };

  void Clear() { clear_time(); clear_yaw(); clear_pitch(); clear_roll(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_yaw() const { return yaw_.has_value(); }
  inline void clear_yaw() { yaw_.reset(); }
  inline double yaw() const { return yaw_.value(); }
  inline void set_yaw(double value) { yaw_ = value; }

  inline bool has_pitch() const { return pitch_.has_value(); }
  inline void clear_pitch() { pitch_.reset(); }
  inline double pitch() const { return pitch_.value(); }
  inline void set_pitch(double value) { pitch_ = value; }

  inline bool has_roll() const { return roll_.has_value(); }
  inline void clear_roll() { roll_.reset(); }
  inline double roll() const { return roll_.value(); }
  inline void set_roll(double value) { roll_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Laser yaw: radians
  OptionalDouble yaw_;
  /// Laser pitch: radians
  OptionalDouble pitch_;
  /// Laser roll: radians
  OptionalDouble roll_;
};

/// LOB Group point for time and RAE data
class LobGroupUpdatePoint
{
public:
  void CopyFrom(const LobGroupUpdatePoint& from) { if (&from == this) return; *this = from; }

  void Clear() { clear_time(); clear_range(); clear_azimuth(); clear_elevation(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_range() const { return range_.has_value(); }
  inline void clear_range() { range_.reset(); }
  inline double range() const { return range_.value(); }
  inline void set_range(double value) { range_ = value; }

  inline bool has_azimuth() const { return azimuth_.has_value(); }
  inline void clear_azimuth() { azimuth_.reset(); }
  inline double azimuth() const { return azimuth_.value(); }
  inline void set_azimuth(double value) { azimuth_ = value; }

  inline bool has_elevation() const { return elevation_.has_value(); }
  inline void clear_elevation() { elevation_.reset(); }
  inline double elevation() const { return elevation_.value(); }
  inline void set_elevation(double value) { elevation_ = value; }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Range in meters from the platform origin
  OptionalDouble range_;
  /// Azimuth; radians
  OptionalDouble azimuth_;
  /// Elevation; radians
  OptionalDouble elevation_;
};

/// LOB Group time and points
class LobGroupUpdate
{
public:
  void CopyFrom(const LobGroupUpdate& from) { if (&from == this) return; *this = from; }

  void Clear() { clear_time(); dataPoints_.clear(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  /// Returns the number of data points for the update
  int datapoints_size() const { return static_cast<int>(dataPoints_.size()); }
  /// Constant version of the data points
  const std::vector<LobGroupUpdatePoint>& datapoints() const { return dataPoints_; }
  /// Returns the requested data point;
  const LobGroupUpdatePoint& datapoints(int index) const { return dataPoints_[index]; }
  /// Mutable version of the points
  std::vector<LobGroupUpdatePoint>* mutable_datapoints() { return &dataPoints_; }
  /// Add a data point
  LobGroupUpdatePoint* add_datapoints() { dataPoints_.push_back(LobGroupUpdatePoint()); return &dataPoints_.back(); }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Points all at the same time
  std::vector<LobGroupUpdatePoint> dataPoints_;
};

/// key,value data intended to categorize a platform
/// for example: friendly, hostile; ship, plane
class CategoryData
{
public:
  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  class Entry
  {
  public:
    inline std::string key() const { return key_; }
    inline void set_key(const std::string& value) { key_ = value; }

    inline std::string value() const { return value_; }
    inline void set_value(const std::string& value) { value_ = value; }

  private:
    std::string key_;
    std::string value_;
  };

  /// Returns the number of entries for the update
  int entry_size() const { return static_cast<int>(entries_.size()); }
  /// Returns a vector of entries
  const std::vector<Entry>& entry() const { return entries_; }
  /// Returns the requested entry;
  const Entry& entry(int index) const { return entries_[index]; }
  /// Add an entry
  Entry* add_entry() { entries_.push_back(Entry()); return &entries_.back(); }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Points all at the same time
  std::vector<Entry> entries_;
};

// Backwards compatibility with protobuf
using CategoryData_Entry = CategoryData::Entry;

/// key,value data which is attached to time, but unrelated to spatial
/// location or orientation (display)
/// for example: fuel, or temperature
class GenericData
{
public:
  void CopyFrom(const GenericData& from) { if (&from == this) return; *this = from; }

  void Clear() { clear_time(); clear_duration();  entries_.clear(); }

  inline bool has_time() const { return time_.has_value(); }
  inline void clear_time() { time_.reset(); }
  inline double time() const { return time_.value(); }
  inline void set_time(double value) { time_ = value; }

  inline bool has_duration() const { return duration_.has_value(); }
  inline void clear_duration() { duration_.reset(); }
  inline double duration() const { return duration_.value(); }
  inline void set_duration(double value) { duration_ = value; }

  class Entry
  {
  public:
    inline std::string key() const { return key_; }
    inline void set_key(const std::string& value) { key_ = value; }

    inline std::string value() const { return value_; }
    inline void set_value(const std::string& value) { value_ = value; }

  private:
    std::string key_;
    std::string value_;
  };

  /// Returns the number of entries for the update
  int entry_size() const { return static_cast<int>(entries_.size()); }
  /// Returns a vector of entries
  const std::vector<Entry>& entry() const { return entries_; }
  /// Returns the requested entry;
  const Entry& entry(int index) const { return entries_[index]; }
  /// Remove all entries
  void clear_entry() { entries_.clear(); }
  /// Add an entry
  Entry* add_entry() { entries_.push_back(Entry()); return &entries_.back(); }

private:
  /// Seconds since scenario reference year for the data posit time
  OptionalDouble time_;
  /// Duration in seconds for the generic data
  OptionalDouble duration_;
  /// Points all at the same time
  std::vector<Entry> entries_;
};

// Backwards compatibility with protobuf
using GenericData_Entry = GenericData::Entry;
}

#endif

