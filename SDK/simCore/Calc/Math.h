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
#ifndef SIMCORE_CALC_MATH_H
#define SIMCORE_CALC_MATH_H

#include <complex>
#include <cmath>
#include <cassert>
#include <limits>

#include "simCore/Common/Export.h"
#include "simCore/Calc/MathConstants.h"
#include "simCore/Calc/Vec3.h"

// Define isnan, finite to match Linux
#if defined(_MSC_VER)
  #include <float.h>
  #ifndef finite
    #define finite _finite
  #endif

  #if _MSC_VER < 1800 && !defined(HAVE_STD_ISNAN)
    #define HAVE_STD_ISNAN
    namespace std {
      /** std::isnan() implementation for older non-compliant MSVC versions */
      template <typename T>
      inline bool isnan(const T &x) { return _isnan(x) != 0; }

      /** std::isinf() implementation for older non-compliant MSVC versions */
      template <typename T>
      inline bool isinf(const T &x) { return _finite(x) == 0; }
    }
  #endif
#endif

namespace simCore
{
  // Define min and max functions.  Note that MIN/MAX and min/max were not chosen
  // due to likelihood of corruption through common #define names in Windows headers
  // and other third party libraries.  STL's std::min and std::max were also not
  // chosen for similar reasons.  For more information, see links like:
  // http://www.suodenjoki.dk/us/archive/2010/min-max.htm

  /**
  * Compute the maximum of the two incoming values
  * @param[in ] a First value
  * @param[in ] b Second value
  * @return maximum of the two values
  */
  template<typename T>
  T sdkMax(const T &a, const T& b)
  {
    return (a > b) ? a : b;
  }

  /**
  * Compute the minimum of the two incoming values
  * @param[in ] a First value
  * @param[in ] b Second value
  * @return minimum of the two values
  */
  template<typename T>
  T sdkMin(const T &a, const T& b)
  {
    return (a < b) ? a : b;
  }

  /**
  * Computes the nearest integer or even of the incoming value
  * @param[in ] x
  * @return nearest integer, ties (.5) are rounded to the nearest even integer
  */
  inline double rint(double x)
  {
#ifdef WIN32
    // round to nearest or even value
    // in case of ties, (.5) values are rounded to the nearest even integer
    double a = floor(x);
    if (x - a < .5) return a;
    if (x - a > .5) return a + 1.0;
    return (fmod(a, 2) == 0.0) ? a : a + 1.0;
#else
    return ::rint(x);
#endif
  }

  /**
  * Rounds the incoming value to the nearest integer
  * @param[in ] x
  * @return nearest integer
  */
  inline double round(double x)
  {
    // round to nearest integer, provides consistent behavior across all OS
    return (x > 0.0) ? floor(x + 0.5) : ceil(x - 0.5);
  }

  //--------------------------------------------------------------------------
  //---general functions

  /// Determines if an integer is odd
  /**
  * Determines if an integer is odd
  * @param[in ] n an integer to be tested for oddness
  * @return true if n is odd; false otherwise
  */
  inline bool odd(int n)
  {
    return ((n&1) != 0 ? true : false);
  }

  /**
  * Compute the square of incoming value
  * @param[in ] in
  * @return square of the input
  */
  template<typename T>
  T square(const T &in)
  {
    return in * in;
  }

  /**
  * Determines the sign of incoming value
  * @param[in ] in
  * @return sign, 0: no sign, 1: positive, -1: negative
  */
  template<typename T>
  T sign(const T &in)
  {
    return in >= 0 ? (in == 0 ? 0 : 1) : -1;
  }

  /**
  * Checks the equality of two values based on a tolerance
  * @param[in ] a First value to compare
  * @param[in ] b Second value to compare
  * @param[in ] t Comparison tolerance
  * @return the equality of two values based on a tolerance
  */
  inline bool areEqual(double a, double b, double t=1.0e-6)
  {
    return fabs(a - b) < t;
  }

  //--------------------------------------------------------------------------
  //---Vec3 related functions

  /**
   * Returns true if all 3 elements are finite
   * @param[in ] u vector
   * @return true if all 3 elements are finite
   */
  inline bool isFinite(const Vec3 &u)
  {
    return finite(u[0]) && finite(u[1]) && finite(u[2]);
  }

  /**
   * Breaks up value into a mantissa (or significand) and exponent value, for base 10
   * values.  Returns mantissa such that mantissa * pow(10.0, exp) == value.
   * @param value Argument to represent in scientific notation.
   * @param exp If non-NULL, will receive the exponent portion of the value.
   * @return Mantissa value.  Will be 0.0 for input of 0.0.  Otherwise, is a value with
   *   an absolute value between [+1.0, +10.0) with a sign set appropriately.  Negative
   *   input values result in a negative return value.
   */
  SDKCORE_EXPORT double toScientific(double value, int* exp);

  /**
  * Find the distance from u to v
  * @param[in ] u first vector
  * @param[in ] v second vector
  * @return distance
  */
  inline double v3Distance(const Vec3 &u, const Vec3 &v)
  {
    // use sqrt of sum of squares of deltas
    return sqrt(square(u[0] - v[0]) + square(u[1] - v[1]) + square(u[2] - v[2]));
  }

  /**
  * Magnitude or length of a vector
  * @param[in ] u vector to consider
  * @return length (Euclidean norm or magnitude) of the vector
  */
  inline double v3Length(const Vec3 &u)
  {
    return sqrt(square(u[0]) + square(u[1]) + square(u[2]));
  }

  /**
  * Scale a vector; v  =  s * u
  * @param[in ] s scale factor (scalar)
  * @param[in ] u starting vector
  * @param[out] v output
  */
  inline void v3Scale(double s, const Vec3 &u, Vec3 &v)
  {
    v.set(s * u[0], s * u[1], s * u[2]);
  }

  /**
  * Turns u into a unit vector, and returns original | u |
  * Converts 3 element double vector u into a unit vector
  * @param[in,out] u vector to be set to unit length
  * @return | u | before transformation applied
  */
  inline double v3Unit(Vec3 &u)
  {
    double t = v3Length(u);
    if (t > 0.0)
    {
      v3Scale(1.0/t, u, u);
    }
    return t;
  }

  /**
  * Returns the normal of vector u
  * @param[ in] u Vector to determine normal of (3 element double vector)
  * @param[out] v output vector to contain normal of u
  * @param[in ] t Comparison tolerance for a zero value
  */
  inline void v3Norm(const Vec3 &u, Vec3 &v, double t=1.0e-9)
  {
    double len = v3Length(u);
    // prevent divide by zero
    if (len > 0.0)
    {
      v3Scale(1.0/len, u, v);
      // if very small values are detected, set to zero
      if (fabs(v.x()) < t) v.setX(0);
      if (fabs(v.y()) < t) v.setY(0);
      if (fabs(v.z()) < t) v.setZ(0);
    }
    else
    {
      v.zero();
    }
  }

  /**
  * Add two vectors; w = u + v
  * @param[in ] u first vector
  * @param[in ] v second vector
  * @param[out] w output
  */
  inline void v3Add(const Vec3 &u, const Vec3 &v, Vec3 &w)
  {
    w.set(u[0] + v[0], u[1] + v[1], u[2] + v[2]);
  }

  /**
  * Subtract two vectors; w = u - v
  * @param[in ] u left side of subtraction
  * @param[in ] v subtracted from u
  * @param[out] w output
  */
  inline void v3Subtract(const Vec3 &u, const Vec3 &v, Vec3 &w)
  {
    w.set(u[0] - v[0], u[1] - v[1], u[2] - v[2]);
  }

  /**
  * Dot product of two vectors
  * @param[in ] u first vector
  * @param[in ] v second vector
  */
  inline double v3Dot(const Vec3 &u, const Vec3 &v)
  {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
  }

  /**
  * Cross product of two vectors; w = u cross v
  * @param[in ] u first vector
  * @param[in ] v second vector
  * @param[out] w output
  */
  inline void v3Cross(const Vec3 &u, const Vec3 &v, Vec3& w)
  {
    w.set(u[1]*v[2] - u[2]*v[1],
          u[2]*v[0] - u[0]*v[2],
          u[0]*v[1] - u[1]*v[0]);
  }

  /**
  * Negate a vector
  * @param[in ] u input vector
  * @param[out] w output
  */
  inline void v3Negate(const Vec3 &u, Vec3& w)
  {
    w.set(-u[0], -u[1], -u[2]);
  }

  /**
  * Convert a spherical coordinate to a rectangular coordinate
  * @param[in ] rng range
  * @param[in ] az azimuth (rad)
  * @param[in ] el elevation (rad)
  * @param[out] v output vector in units of range
  */
  inline void v3SphtoRec(double rng, double az, double el, Vec3 &v)
  {
    // NOTE: elevation measured off horizon (XY plane) instead of Z axis
    v.set(rng * sin(az) * cos(el),  // X (v[0])
          rng * cos(az) * cos(el),  // Y (v[1])
          rng * sin(el));           // Z (v[2])
  }

  /**
  * Convert a spherical coordinate to a rectangular coordinate
  * @param[in ] rae input vector of range, azimuth (rad) an elevation (rad)
  * @param[out] v output vector in units of range component
  */
  inline void v3SphtoRec(const Vec3 &rae, Vec3 &v)
  {
    v3SphtoRec(rae.range(), rae.raeAz(), rae.raeEl(), v);
  }

  /**
  * Compares two vectors for equality within the specified tolerance
  * @param[in ] u input vector to compare
  * @param[in ] v input vector to compare
  * @param[in ] t comparison tolerance
  * @return true if u is equal to v (within tolerance 't')
  */
  SDKCORE_EXPORT bool v3AreEqual(const Vec3 &u, const Vec3 &v, double const t=1.0e-6);

  /**
  * Rotates given vector about X axis
  * @param[in ] a input vector
  * @param[in ] ang rotation angle about X axis (rad)
  * @param[out] vp output vector
  */
  SDKCORE_EXPORT void v3RotX(const Vec3 &a, double ang, Vec3 &vp);

  /**
  * Rotates given vector about Y axis
  * @param[in ] a input vector
  * @param[in ] ang rotation angle about Y axis (rad)
  * @param[out] vp output vector
  */
  SDKCORE_EXPORT void v3RotY(const Vec3 &a, double ang, Vec3 &vp);

  /**
  * Computes angle between two input vectors
  * @param[in ] u input vector 1
  * @param[in ] v input vector 2
  * @return angle between u and v
  */
  SDKCORE_EXPORT double v3Angle(const Vec3 &u, const Vec3 &v);

  //--------------------------------------------------------------------------
  //---Matrix (double[3][3]) related functions
  //
  // In the function descriptions and prototypes below, a, b, and c are
  // 3-by-3 matrices; u and v are 3-vectors (Vec3); and q is a four-element
  // quaternion.  For efficiency, the matrices as well as the vectors and
  // quaternions are treated as one-dimensional within the code, by use of
  // an intermediary pointer (aa, bb, or cc).  The relationship of
  // internal storage to mathematical concept is as shown:
  //
  //        [ a[0]  a[1]  a[2] ]           [ u[0] ]         [ q[0] ]
  //  a  =  [ a[3]  a[4]  a[5] ],    u  =  [ u[1] ],    q = [ q[1] ].
  //        [ a[6]  a[7]  a[8] ]           [ u[2] ]         [ q[2] ]
  //                                                        [ q[3] ]

  /**
  * Matrix multiply; c[3][3] = a[3][3] * b[3][3]
  * @param[in ] a first matrix
  * @param[in ] b second matrix
  * @param[out] c output matrix
  * @pre All matrices valid
  */
  SDKCORE_EXPORT void d3MMmult(const double a[][3], const double b[][3], double c[][3]);

  /**
  * Matrix to vector multiply; v[3] =  a[3][3] * u[3]
  * @param[in ] a matrix
  * @param[in ] u vector
  * @param[out] v output vector
  * @pre matrix valid, u and v cannot be the same vector
  */
  SDKCORE_EXPORT void d3Mv3Mult(const double a[][3], const Vec3 &u, Vec3 &v);

  /**
  * Transposed matrix to vector multiply; v[3] = transpose(a[3][3]) * u[3]
  * @param[in ] a matrix
  * @param[in ] u vector
  * @param[out] v output vector
  * @pre matrix valid, u and v cannot be the same vector
  */
  SDKCORE_EXPORT void d3MTv3Mult(const double a[][3], const Vec3 &u, Vec3 &v);

  /**
  * Transposed matrix multiply; c[3][3] = a[3][3] * transpose(b[3][3])
  * @param[in ] a first matrix
  * @param[in ] b second matrix
  * @param[out] c output matrix
  * @pre All matrices valid
  */
  SDKCORE_EXPORT void d3MMTmult(const double a[][3], const double b[][3], double c[][3]);

  //--------------------------------------------------------------------------
  //---Quaternion related functions
  //
  // Functions expect a quaternion in the form:  q0 + q1i + q2j + q3k  (w,x,y,z)

  /**
  * Returns the normal of quaternion q
  * @param[in ] q 4 element double vector quaternion to normalize
  * @param[out] n 4 element double vector quaternion that contains normalized q
  * @param[in ] t Comparison tolerance for a zero value
  * @pre All vectors valid
  */
  SDKCORE_EXPORT void dQNorm(const double q[4], double n[4], double t=1.0e-9);

  /**
  * Returns the multiplication of two quaternions, where result = q1 * q2
  * @param[in ] q1 4 element double vector quaternion
  * @param[in ] q2 4 element double vector quaternion
  * @param[out] result 4 element double vector quaternion containing q1 * q2
  * @pre All vectors valid
  */
  SDKCORE_EXPORT void dQMult(const double q1[4], const double q2[4], double result[4]);

  //--------------------------------------------------------------------------
  //---Euler angle conversion functions

  /**
  * Convert a direction cosine matrix to Euler angles using a NED frame
  * @param[in ] dcm Direction cosine matrix
  * @param[out] ea Euler angles
  * @pre DCM valid
  */
  SDKCORE_EXPORT void d3DCMtoEuler(const double dcm[][3], Vec3 &ea);

  /**
  * Convert Euler angles to a direction cosine matrix using a NED frame
  * @param[in ] ea Euler angles
  * @param[out] dcm Direction cosine matrix (DCM)
  * @pre DCM valid
  */
  SDKCORE_EXPORT void d3EulertoDCM(const Vec3 &ea, double dcm[][3]);

  /**
  * Converts Euler angles to a quaternion vector using a NED frame
  * @param[in ] ea 3 element double vector of Euler angles (psi/yaw, theta/pitch, phi/roll)
  * @param[out] q Result of conversion of Euler angles ea to a 4 element double vector quaternion
  * @pre quaternion valid
  */
  SDKCORE_EXPORT void d3EulertoQ(const Vec3 &ea, double q[4]);

  /**
  * Converts a quaternion vector to Euler angles using a NED frame
  * @param[in ] q 4 element double vector quaternion
  * @param[out] ea Result of conversion of quaternion vector to a 3 element double vector of Euler angles (psi/yaw, theta/pitch, phi/roll)
  * @pre quaternion valid
  */
  SDKCORE_EXPORT void d3QtoEuler(const double q[4], Vec3 &ea);

} // End of namespace simCore

#endif /* SIMCORE_CALC_MATH_H */
