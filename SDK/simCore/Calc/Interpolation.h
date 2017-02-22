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
#ifndef SIMCORE_CALC_INTERPOLATION_H
#define SIMCORE_CALC_INTERPOLATION_H

/**@file
* Defines various functions for interpolating between data points
*/

#include <map>
#include "simCore/Common/Export.h"
#include "simCore/Calc/Math.h"

namespace simCore
{
  /**
  * @brief Computes a scale factor [0,1] between a set of bounded values
  *
  * Computes a scale factor [0,1] between a set of bounded values at the specified value
  * @param[in ] lowVal Low value
  * @param[in ] exactVal Exact value to determine scale
  * @param[in ] highVal High value
  * @return scale factor between [0,1] where 0 is the low and 1 is the high value
  */
  inline double getFactor(const double &lowVal, const double &exactVal, const double &highVal)
  {
    // Perform bounds check and prevent divide by zero
    if (exactVal <= lowVal) return 0.;
    if (exactVal >= highVal || (highVal - lowVal) == 0) return 1.;
    return (exactVal - lowVal) / (highVal - lowVal);
  }

  /**
  * @brief Interpolates between values "a" and "b" using nearest-neighbor method
  *
  * Interpolate between values "a" and "b" using nearest-neighbor:
  *   when factor is < 0.5, the value "a" is used.
  *   when factor is >= 0.5, the value "b" is used.
  *
  * @param[in ] a First value
  * @param[in ] b Second value
  * @param[in ] factor scale factor [0,1] between a and b
  * @return the value "a" or "b", determined by factor
  */
  inline double nearestNeighborInterpolate(double a, double b, double factor)
  {
    return (factor < 0.5) ? a : b;
  }

  /**
  * @brief Performs linear interpolation between a set of bounded values
  *
  * Performs linear interpolation between a set of bounded values at the specified value
  *   when factor is < 0.0, the low value is used.
  *   when factor is >= 1.0, the high value is used.
  *   when factor is >= 0.0 and < 1.0, the calculated value uses a weighting of the low and high values based on factor
  * @param[in ] lowVal Low value of first bound
  * @param[in ] highVal High value of first bound
  * @param[in ] xFactor interpolation scale factor
  * @return interpolated value
  */
  template <class Type>
  inline Type linearInterpolate(const Type &lowVal, const Type &highVal, double xFactor)
  {
    // NOTE: casts to double are needed to avoid loss of precision
    return static_cast<Type>(lowVal + (static_cast<double>(highVal) - static_cast<double>(lowVal)) * xFactor);
  }

  /**
  * @brief Performs linear interpolation between a set of bounded values
  *
  * Performs linear interpolation between a set of bounded values at the specified value
  *   when factor is < 0.0, the low value is used.
  *   when factor is >= 1.0, the high value is used.
  *   when factor is >= 0.0 and < 1.0, the calculated value uses a weighting of the low and high values based on factor
  * @param[in ] lowVal Low value of first bound
  * @param[in ] highVal High value of first bound
  * @param[in ] xLow Low value of second bound
  * @param[in ] xVal value to perform interpolation
  * @param[in ] xHigh High value of second bound
  * @return interpolated value
  */
  template <class Type>
  inline Type linearInterpolate(const Type &lowVal, const Type &highVal, double xLow, double xVal, double xHigh)
  {
    double xFactor = getFactor(xLow, xVal, xHigh);
    return linearInterpolate(lowVal, highVal, xFactor);
  }

  /**
  * @brief Performs linear interpolation between a set of bounded values in a std::map container
  *
  * Performs linear interpolation between a set of bounded values pulled from the std::map at the specified value
  * @param[in ] container STL map
  * @param[in ] atPos value to perform interpolation
  * @param[out] interpVal Interpolated return value
  * @param[in ] tol Tolerance value for comparison of atPos and result of lower bound to determine a match
  * @param[in ] clampBgn Boolean, if true interpVal will be clamped to the beginning of the container if the atPos occurs before
  * @param[in ] clampEnd Boolean, if true interpVal will be clamped to the end of the container if the atPos occurs after
  * @return Boolean where true indicates success of interpolation
  */
  template <typename T1, typename T2>
  inline bool linearInterpolate(const std::map<T1, T2>& container, const T1& atPos, T2& interpVal, double tol=1.0e-6, bool clampBgn=false, bool clampEnd=false)
  {
    if (container.empty())
      return false;

    typename std::map<T1, T2>::const_iterator lowIter = container.lower_bound(atPos);
    // return if value is after the end() of the map
    if (lowIter == container.end())
    {
      // only set interpVal if clamp was requested
      if (clampEnd) interpVal = container.rbegin()->second;
      return clampEnd;
    }

    // check for a match within specified tolerance
    if (simCore::areEqual(atPos, lowIter->first, tol))
    {
      interpVal = lowIter->second;
      return true;
    }

    // return if value is before first element in map
    if (lowIter == container.begin())
    {
      // only set interpVal if clamp was requested
      if (clampBgn) interpVal = container.begin()->second;
      return clampBgn;
    }

    // otherwise interpolate
    T1 hiRng = lowIter->first;
    T2 hiHgt = lowIter->second;
    --lowIter;

    interpVal = linearInterpolate(lowIter->second, hiHgt, lowIter->first, atPos, hiRng);
    return true;
  }

  /**
  * @brief Performs bilinear interpolation between a set of bounded values
  *
  * Performs bilinear interpolation between two sets of bounded values at the specified values
  * @param[in ] ll Lower left bound
  * @param[in ] lr Lower right bound
  * @param[in ] ur Upper right bound
  * @param[in ] ul Upper left bound
  * @param[in ] xFactor X axis interpolation scale factor
  * @param[in ] yFactor Y axis interpolation scale factor
  * @return bilinear interpolated value
  */
  template <class Type>
  inline Type bilinearInterpolate(Type ll, Type lr, Type ur, Type ul, const double &xFactor, const double &yFactor)
  {
    return static_cast<Type>(
      ll * (1 - xFactor) * (1 - yFactor) +
      lr * xFactor * (1 - yFactor) +
      ur * xFactor * yFactor +
      ul * (1 - xFactor) * yFactor);
  }

  /**
  * @brief Performs bilinear interpolation between a set of bounded values
  *
  * Performs bilinear interpolation between two sets of bounded values at the specified values
  * @param[in ] ll Lower left bound
  * @param[in ] lr Lower right bound
  * @param[in ] ur Upper right bound
  * @param[in ] ul Upper left bound
  * @param[in ] xLow Low value of X bound
  * @param[in ] xVal value to perform interpolation along X axis
  * @param[in ] xHigh High value of X bound
  * @param[in ] yLow Low value of Y bound
  * @param[in ] yVal value to perform interpolation along Y axis
  * @param[in ] yHigh High value of Y bound
  * @return bilinear interpolated value
  */
  template <class Type>
  inline Type bilinearInterpolate(Type ll, Type lr, Type ur, Type ul, const double &xLow, const double &xVal, const double &xHigh, const double &yLow, const double &yVal, const double &yHigh)
  {
    double xFactor = getFactor(xLow, xVal, xHigh);
    double yFactor = getFactor(yLow, yVal, yHigh);
    return bilinearInterpolate(ll, lr, ur, ul, xFactor, yFactor);
  }

  /**
  * @brief Performs linear interpolation between two angles
  *
  * Interpolate between values "a" and "b" using linear interpolation:
  *   when factor is < 0.0, the value "a" is used.
  *   when factor is >= 1.0, the value "b" is used.
  *   when factor is >= 0.0 and < 1.0, the calculated value uses a weighting of "a" and "b" based on factor
  *
  * @param[in ] a Start angle (rad)
  * @param[in ] b End angle (rad)
  * @param[in ] factor Weighting factor
  * @return a weighted combination of "a" and "b", with the weighting determined by "t"
  */
  SDKCORE_EXPORT double linearInterpolateAngle(double a, double b, double factor);

  /**
  * @brief Performs linear interpolation between two radian angles
  *
  * Interpolate between radian angles "a" and "b" using linear interpolation.
  * @param[in ] a start angle (rad)
  * @param[in ] b end angle (rad)
  * @param[in ] ta time at value a
  * @param[in ] t interpolation time
  * @param[in ] tb time at value b
  * @return an angle between 0 and 2pi.  The angle is a weighted combination of "a" and "b", with the weighting determined by "t"
  */
  SDKCORE_EXPORT double linearInterpolateAngle(double a, double b, double ta, double t, double tb);

  /// template specialization for Vec3; @see simCore::linearInterpolate<>()
  template<>
  SDKCORE_EXPORT
  Vec3 linearInterpolate<Vec3>(const Vec3 &prev, const Vec3 &next, double mixFactor);
}

#endif /*SIMCORE_CALC_INTERPOLATION_H */
