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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_CALC_VEC3_H
#define SIMCORE_CALC_VEC3_H

#include <cassert>
#include <cmath>
#include <cstdlib>

namespace simCore
{

// Header only class; doesn't need to be exported
/// (static sized) vector of three doubles
class Vec3
{
public:
  /// Default constructor gives {0,0,0}
  Vec3() { }

  /**
   * Value constructor
   * @param[in] v0 The component at index 0; pos/ori/vel/acc
   * @param[in] v1 The component at index 1; pos/ori/vel/acc
   * @param[in] v2 The component at index 2; pos/ori/vel/acc
   */
  Vec3(double v0, double v1, double v2) { set(v0, v1, v2); }

  /**
   * Value constructor
   * @param[in] v Vector elements
   * @pre valid input pointer, if not valid internal elements are set to zero
   */
  explicit Vec3(const double v[3])
  {
    if (v == nullptr)
    {
      zero();
      return;
    }
    set(v[0], v[1], v[2]);
  }

  /**@name Copy and move operators and assignment
   * @{
   */
  Vec3(const Vec3& rhs) = default;
  Vec3(Vec3&& rhs) = default;
  Vec3& operator=(const Vec3& rhs) = default;
  Vec3& operator=(Vec3&& rhs) = default;
  ///@}

  /// Equality
  bool operator==(const Vec3& value) const
  {
    if (&value == this) return true;
    return (v_[0] == value.v_[0] && v_[1] == value.v_[1] && v_[2] == value.v_[2]);
  }
  /// Inequality
  bool operator!=(const Vec3& value) const
  {
    return !operator==(value);
  }

  /// Set vector elements to zero
  void zero() { v_[0] = v_[1] = v_[2] = 0; }

  /**@name Read and Write Access
   * @{
   */
  /// Behave like an array
  double operator[](size_t index) const
  {
    assert(index < 3);
    if (index > 2)
      index = 2;
    return v_[index];
  }
  /// Behave like an array
  double& operator[](size_t index)
  {
    assert(index < 3);
    if (index > 2)
      index = 2;
    return v_[index];
  }

  /// Copy contents to a double[3] pointer
  void toD3(double dVec[3]) const
  {
    if (dVec == nullptr) return;
    dVec[0] = v_[0]; dVec[1] = v_[1]; dVec[2] = v_[2];
  }

  /// Set first element component
  void setV0(double value) { v_[0] = value; }
  /// Set second element component
  void setV1(double value) { v_[1] = value; }
  /// Set third element component
  void setV2(double value) { v_[2] = value; }

  /// Set all elements
  void set(double v0, double v1, double v2) { v_[0] = v0; v_[1] = v1; v_[2] = v2; }
  /// Set all elements
  void set(const Vec3& value) { v_[0] = value[0]; v_[1] = value[1]; v_[2] = value[2]; }

  /// Scales all elements
  void scale(double value) { v_[0] *= value; v_[1] *= value; v_[2] *= value; }
  ///@}

  /**@name Mappings for {x,y,z}, {yaw,pitch,roll}, etc.
   * @{
   */
  double x() const { return v_[0]; }
  double y() const { return v_[1]; }
  double z() const { return v_[2]; }

  double lat() const { return v_[0]; }
  double lon() const { return v_[1]; }
  double alt() const { return v_[2]; }

  double range() const { return v_[0]; }
  double raeAz() const { return v_[1]; }
  double raeEl() const { return v_[2]; }

  double yaw() const { return v_[0]; }
  double pitch() const { return v_[1]; }
  double roll() const { return v_[2]; }

  double psi() const { return v_[0]; }
  double theta() const { return v_[1]; }
  double phi() const { return v_[2]; }

  void setX(double value) { setV0(value); }
  void setY(double value) { setV1(value); }
  void setZ(double value) { setV2(value); }

  void setLat(double value) { setV0(value); }
  void setLon(double value) { setV1(value); }
  void setAlt(double value) { setV2(value); }

  void setRange(double value) { setV0(value); }
  void setRaeAz(double value) { setV1(value); }
  void setRaeEl(double value) { setV2(value); }

  void setYaw(double value) { setV0(value); }
  void setPitch(double value) { setV1(value); }
  void setRoll(double value) { setV2(value); }

  void setPsi(double value) { setV0(value); }
  void setTheta(double value) { setV1(value); }
  void setPhi(double value) { setV2(value); }
  ///@}

  /**@name Convenience operator overrides
   * @{
   */

  /** Add vectors */
  Vec3 operator+(const Vec3& r) const { return Vec3(x() + r.x(), y() + r.y(), z() + r.z()); }
  /** Add and assign vector */
  Vec3& operator+=(const Vec3& r) { set(x() + r.x(), y() + r.y(), z() + r.z()); return *this; }

  /** Subtract vectors */
  Vec3 operator-(const Vec3& r) const { return Vec3(x() - r.x(), y() - r.y(), z() - r.z()); }
  /** Subtract and assign vector */
  Vec3& operator-=(const Vec3& r) { set(x() - r.x(), y() - r.y(), z() - r.z()); return *this; }

  /** Scale by a value */
  Vec3 operator*(double scalar) const { return Vec3(x() * scalar, y() * scalar, z() * scalar); }
  /** Scale by a value and assign vector */
  Vec3& operator*=(double scalar) { set(x() * scalar, y() * scalar, z() * scalar); return *this; }

  /** Divide by a value */
  Vec3 operator/(double divisor) const {
    if (divisor == 0.) return Vec3();
    return Vec3(x() / divisor, y() / divisor, z() / divisor);
  }
  /** Divide by a value and assign vector */
  Vec3& operator/=(double divisor) {
    if (divisor == 0.)
      zero();
    else
      set(x() / divisor, y() / divisor, z() / divisor);
    return *this;
  }

  /** Negate vector */
  Vec3 operator-() const { return Vec3(-x(), -y(), -z()); }

  ///@}

  /** Calculates the dot product of this vector and another vector */
  double dot(const Vec3& r) const { return x() * r.x() + y() * r.y() + z() * r.z(); }
  /** Calculates the cross product of this vector and another vector */
  Vec3 cross(const Vec3& r) const {
    return Vec3(v_[1] * r.v_[2] - v_[2] * r.v_[1],
      v_[2] * r.v_[0] - v_[0] * r.v_[2],
      v_[0] * r.v_[1] - v_[1] * r.v_[0]);
  }
  /** Calculates the length of the vector */
  double length() const { return std::sqrt(x() * x() + y() * y() + z() * z()); }
  /** Calculates the unit length of the vector by normalizing; values within specified tolerance are set to 0. */
  Vec3 normalize(double t = 1.0e-9) const {
    const double len = length();
    if (len == 0.)
      return Vec3();
    Vec3 rv = *this;
    rv /= len;
    if (std::fabs(rv.x()) < t) rv.setX(0.);
    if (std::fabs(rv.y()) < t) rv.setY(0.);
    if (std::fabs(rv.z()) < t) rv.setZ(0.);
    return rv;
  }

private:
  double v_[3] = { 0., 0., 0. };
};

} // namespace simCore

#endif /* SIMCORE_CALC_VEC3_H */
