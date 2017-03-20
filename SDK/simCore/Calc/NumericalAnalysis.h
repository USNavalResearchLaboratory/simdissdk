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
#ifndef SIMCORE_CALC_NUMERICALANALYSIS_H
#define SIMCORE_CALC_NUMERICALANALYSIS_H

#include <vector>
#include "simCore/Common/Common.h"

namespace simCore
{
  /// Enumeration of supported state tokens for searches */
  enum NumericalSearchType
  {
    SEARCH_INIT_X=-1,
    SEARCH_INACTIVE,
    SEARCH_INIT,
    SEARCH_SECOND_PASS,
    SEARCH_THIRD_PASS,
    SEARCH_FOURTH_PASS,
    SEARCH_FIFTH_PASS,
    SEARCH_CONVERGED,
    SEARCH_FAILED,
    SEARCH_NO_ROOT,
    SEARCH_MAX_ITER
  };

  /**
  * @brief Bisection iterative search used to find the root of a function
  */
  class SDKCORE_EXPORT BisectionSearch
  {
  public:
    /**
    * BisectionSearch constructor
    * @param[in] miter Maximum number of iterations
    * @param[in] tol Convergence tolerance on y
    */
    BisectionSearch(int miter=50, double tol=1.0e-10);

    /**
    * @brief Numerically solve for the root of a function
    *
    * The search for x is limited to a region between a low value and a high
    * value of x.  A simple bi-section method is used, and the function
    * f(x) is evaluated outside of the root function.
    * @param[in,out] x Initial/current estimate of solution
    * @param[in    ] y Function value for current x value (ignored in 1st call)
    * @param[in,out] xlo Current lower search boundary
    * @param[in,out] xhi Current upper search boundary
    * @param[in    ] type State of the search
    * @return solution indicator (SEARCH_CONVERGED when converged, SEARCH_MAX_ITER if exceeded specified miter before converging, and SEARCH_FAILED otherwise)
    * @pre x, xlo and xhi valid params
    */
    NumericalSearchType searchX(double &x, double y, double &xlo, double &xhi, NumericalSearchType type);

    /**
    * BisectionSearch count
    * @return number of iterations during last search
    */
    int count() const { return counter_; }

  protected:
    double x1_; ///< internal storage of X parameter
    double x2_; ///< internal storage of X parameter
    double y1_; ///< internal storage of Y parameter
    double y2_; ///< internal storage of Y parameter
    int counter_; ///< iteration counter
    int maxIter_; ///< maximum number of iterations
    double toleranceY_; ///< convergence tolerance on y
  };

  /**
  * @brief Linear iterative search used to find the root of a function
  */
  class SDKCORE_EXPORT LinearSearch
  {
  public:
    /**
    * LinearSearch constructor
    * @param[in] maxIter Maximum number of iterations
    * @param[in] tol Convergence tolerance on y
    */
    LinearSearch(int maxIter=50, double tol=1.0e-10);

    /**
    * @brief Numerically solve for the root of a function
    *
    * The search for x is limited to a region between a low value and a high
    * value of x.  This function uses a simple Secant method starting
    * with an initial estimate of the solution.  Convergence is based on
    * y having a value less than toleranceY_.
    * The search interval needs to bound the root and be sufficiently close.
    * For details http://en.wikipedia.org/wiki/Secant_method
    * @param[in,out] x Initial/current estimate of solution
    * @param[in   ] y Function value for current x value (ignored in 1st call)
    * @param[in   ] xlo Current lower search boundary
    * @param[in   ] xhi Current upper search boundary
    * @param[in   ] fdx Factor for delta-x for derivative (typically 1.0e-6)
    * @param[in   ] type State of the search
    * @return solution indicator (SEARCH_CONVERGED when converged, SEARCH_MAX_ITER if exceeded specified maxIter before converging, and SEARCH_FAILED otherwise)
    */
    NumericalSearchType searchX(double &x, double y, double xlo, double xhi, double fdx, NumericalSearchType type);

    /**
    * LinearSearch count
    * @return number of iterations during last search
    */
    int count() const { return counter_; }

  protected:
    double x1_;         ///< internal storage of X parameter
    double x2_;         ///< internal storage of X parameter
    double y1_;         ///< internal storage of Y parameter
    double y2_;         ///< internal storage of Y parameter
    int counter_;       ///< iteration counter
    int lo_;            ///< low value of search
    int hi_;            ///< high value of search
    int maxIter_;       ///< maximum number of iterations
    double toleranceY_; ///< convergence tolerance on y
  };

  /**
  * @brief Second-degree Newton interpolation of tabular function f(t) at given time
  *
  * Second-degree Newton interpolation of tabular function f(t) at given time
  * @param[in ] t0 Time at which value of the function is desired
  * @param[in ] t Values of independent variable at which value of function is known
  * @param[in ] f Array giving the values of the function at times 't'
  * @param[out] funcAtT0 Value of the function f(t) at 't0'
  * @return error condition, 0: success, 1 failure
  */
  SDKCORE_EXPORT int newtonInterp(double t0, double t[3], double f[3], double &funcAtT0);

  /**
  * @brief Compute time at which tabular function f(t) has a given value
  *
  * Compute time at which tabular function f(t) has a given value
  * @param[in ] funcAtT0 Value of the function f(t) at 't0'
  * @param[in ] t Array of values of the independent variable at which the value of the function is known
  * @param[in ] f array giving the values of the function at times 't'
  * @param[in ] tol Precision tolerance to which the time 't0' is needed, in the same units as the 't' array
  * @param[out] t0 Pointer to output array
  * @return error condition 0: success, 1: value of 'funcAtT0' does not lie
  * between f[0] and f[2], 2: process failed to converge after 'max_iter' iterations
  */
  SDKCORE_EXPORT int invLinearInterp(double funcAtT0, double t[3], double f[3], double tol, double &t0);

}

#endif /* SIMCORE_CALC_NUMERICALANALYSIS_H */
