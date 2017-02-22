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
#ifndef SIMCORE_CALC_VEC3_H
#define SIMCORE_CALC_VEC3_H

#include <cstdlib>
#include <cassert>

namespace simCore
{
  // Header only class; doesn't need to be exported
  /// (static sized) vector of three doubles
  class Vec3
  {
  public:
    /// Default constructor gives {0,0,0}
    Vec3() { zero(); }

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
    Vec3(const double v[3])
    {
      if (v == NULL)
      {
        zero();
        return;
      }
      set(v[0], v[1], v[2]);
    }

    /// Allow pass by value
    Vec3(const Vec3 &value) { set(value); }
    /// Allow assignment
    Vec3& operator=(const Vec3 &value) { set(value); return *this; }
    /// Equality
    bool operator==(const Vec3 &value) const
    {
      if (&value == this) return true;
      return (v[0] == value.v[0] && v[1] == value.v[1] && v[2] == value.v[2]);
    }
    /// Inequality
    bool operator!=(const Vec3& value) const
    {
      return !operator==(value);
    }

    /// Set vector elements to zero
    void zero() { v[0] = v[1] = v[2] = 0; }

    /**@name Read and Write Access
    * @{
    */
    /// Behave like an array
    double  operator[](size_t index) const
    {
      assert(index < 3);
      if (index > 2)
        index = 2;
      return v[index];
    }
    /// Behave like an array
    double& operator[](size_t index)
    {
      assert(index < 3);
      if (index > 2)
        index = 2;
      return v[index];
    }

    /// Copy contents to a double[3] pointer
    void toD3(double dVec[3]) const
    {
      if (dVec == NULL) return;
      dVec[0] = v[0]; dVec[1] = v[1]; dVec[2] = v[2];
    }

    /// Set first element component
    void setV0(double value) { v[0] = value; }
    /// Set second element component
    void setV1(double value) { v[1] = value; }
    /// Set third element component
    void setV2(double value) { v[2] = value; }

    /// Set all elements
    void set(double v0, double v1, double v2) { v[0] = v0; v[1] = v1; v[2] = v2; }
    /// Set all elements
    void set(const Vec3 &value) { v[0] = value[0]; v[1] = value[1]; v[2] = value[2]; }
    ///@}

    /**@name Mappings for {x,y,z}, {yaw,pitch,roll}, etc.
    * @{
    */
    double x() const { return v[0]; }
    double y() const { return v[1]; }
    double z() const { return v[2]; }

    double lat() const { return v[0]; }
    double lon() const { return v[1]; }
    double alt() const { return v[2]; }

    double range() const { return v[0]; }
    double raeAz() const { return v[1]; }
    double raeEl() const { return v[2]; }

    double yaw() const { return v[0]; }
    double pitch() const { return v[1]; }
    double roll() const { return v[2]; }

    double psi() const { return v[0]; }
    double theta() const { return v[1]; }
    double phi() const { return v[2]; }

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

  private:
    double v[3];
  };

} // namespace simCore

#endif /* SIMCORE_CALC_VEC3_H */
