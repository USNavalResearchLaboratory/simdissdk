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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cmath>
#include "simCore/Calc/Math.h"
#include "simCore/Calc/NumericalAnalysis.h"

//------------------------------------------------------------------------
namespace simCore
{

  BisectionSearch::BisectionSearch(int miter, double tol)
    : x1_(0),
    x2_(0),
    y1_(0),
    y2_(0),
    counter_(0),
    maxIter_(miter),
    toleranceY_(tol)
  {
  }

  // Numerically solve for the root of a function (x such that y=0 where y=f(x))
  NumericalSearchType BisectionSearch::searchX(double &x, double y, double &xlo, double &xhi, NumericalSearchType type)
  {
    // If first pass through function, then setup to evaluate "xlo"
    if (type == SEARCH_INIT)
    {
      x1_ = 0;
      x2_ = 0;
      y1_ = 0;
      y2_ = 0;
      counter_ = 1;

      x = xlo;
      return SEARCH_SECOND_PASS;
    }

    // Reached the acceptable error so return success
    if (fabs(y) < toleranceY_)
      return SEARCH_CONVERGED;

    ++counter_;

    // Second pass through function, setup to evaluate "xhi"
    if (type == SEARCH_SECOND_PASS)
    {
      x1_ = x;
      y1_ = y;
      x = xhi;
      return SEARCH_THIRD_PASS;
    }

    // Third pass through function, evaluate mid-point...
    if (type == SEARCH_THIRD_PASS)
    {
      x2_ = x;
      y2_ = y;
      x = 0.5 * (x1_ + x2_);
      return SEARCH_FOURTH_PASS;
    }

    // Subsequent passes through the search
    if (counter_ > maxIter_)
      return SEARCH_MAX_ITER;

    if (y * y1_ < 0.0)
    {
      x2_ = x;
      y2_ = y;
      xhi = x;
      x = 0.5 * (x1_ + x2_);
      return SEARCH_FOURTH_PASS;
    }

    if (y * y2_ < 0.0)
    {
      x1_ = x;
      y1_ = y;
      xlo = x;
      x = 0.5 * (x1_ + x2_);
      return SEARCH_FOURTH_PASS;
    }

    if (fabs(y1_) < fabs(y) && fabs(y1_) < fabs(y2_))
    {
      x = x1_;
    }
    else if (fabs(y2_) < fabs(y) && fabs(y2_) < fabs(y1_))
    {
      x = x2_;
    }

    return SEARCH_FAILED;
  }

  //------------------------------------------------------------------------
  LinearSearch::LinearSearch(int maxIter, double tol)
    : x1_(0),
    x2_(0),
    y1_(0),
    y2_(0),
    counter_(0),
    lo_(0),
    hi_(0),
    maxIter_(maxIter),
    toleranceY_(tol)
  {
  }

  // Numerically solve for the root of a function (x such that y=0 where y=f(x))
  NumericalSearchType LinearSearch::searchX(double &x, double y, double xlo, double xhi, double fdx, NumericalSearchType type)
  {
    // If first pass through function, then setup to evaluate initial est.
    if (type == SEARCH_INIT)
    {
      counter_ = 1;
      x1_ = 0;
      x2_ = 0;
      y1_ = 0;
      y2_ = 0;
      lo_ = 0;
      hi_ = 0;

      return SEARCH_SECOND_PASS;
    }

    // Reached the acceptable error so return success
    if (fabs(y) < toleranceY_)
      return SEARCH_CONVERGED;

    ++counter_;

    // Second pass through function, setup to evaluate x + dx
    if (type == SEARCH_SECOND_PASS)
    {
      x1_ = x;
      y1_ = y;
      const double dx = fdx + fdx*fabs(x);
      x += dx;
      return SEARCH_THIRD_PASS;
    }

    // Subsequent passes thru the search

    // Check if the algorithm is taking too long
    if (counter_ > maxIter_)
      return SEARCH_MAX_ITER;

    // Check if the algorithm is stuck on a boundary
    if (lo_ > 2)
      return SEARCH_FAILED;
    if (hi_ > 2)
      return SEARCH_FAILED;

    // Move the boundaries in an alternating fashion, to narrow the gap
    if (type == SEARCH_THIRD_PASS)
    {
      x2_ = x;
      y2_ = y;
    }
    else
    {
      x1_ = x;
      y1_ = y;
    }

    // Prevent divide-by-zero
    if (y2_ == y1_)
      return SEARCH_FAILED;

    // Compute new x value...
    // Calculate an x intercept using a point pair (x1, y1) and the inverse slope (x2_-x1_)/(y2_-y1_)
    x = x1_ - y1_*(x2_-x1_)/(y2_-y1_);

    if (x < xlo)
    {
      // Hitting lower limit, so increment error counter
      x = xlo;
      lo_++;
    }
    else if (x > xhi)
    {
      // Hitting upper limit, so increment error counter
      x = xhi;
      hi_++;
    }
    else
    {
      // Good calculation, so clear out counters
      lo_ = hi_ = 0;
    }

    // Toggle back and forth
    return type == SEARCH_THIRD_PASS ? SEARCH_FOURTH_PASS : SEARCH_THIRD_PASS;
  }

  //------------------------------------------------------------------------
  // Second-degree Newton interpolation to compute the value
  // of a tabular function f(t) at a given time.
  //
  // Reference:
  //  Hildebrand. F.B. (1974). Introduction to Numerical Analysis.
  //  New York: Dover Publications; pp. 58-60.
  //
  // Notes:
  //  1. Value of 't0' must lie between t[0] and t[2].
  //  2. This calculation uses the "zig zag" path through the data; see
  //  reference for details.
  int newtonInterp(double t0, const double t[3], const double f[3], double &funcAtT0)
  {
    // Make sure 't0' lies between t[0] and t[2].
    if (((t0 < t[0]) && (t0 < t[2])) ||
      ((t0 > t[0]) && (t0 > t[2])))
    {
      return 1;
    }

    // Compute divided differences.
    double a = (f[1] - f[0]) / (t[1] - t[0]);
    double b = (f[2] - f[1]) / (t[2] - t[1]);
    double c = (b - a) / (t[2] - t[0]);

    // Compute the value of function 'f' at 't0'.
    double d = t0 - t[1];
    funcAtT0 = f[1] + (d * b) + (d * (t0 - t[2]) * c);

    return 0;
  }

  //------------------------------------------------------------------------
  // Compute the time at which a tabular function f(t) has a given value.
  //
  // Reference:
  //  Hildebrand. F.B. (1974). Introduction to Numerical Analysis.
  //  New York: Dover Publications; pp. 69-70.
  //
  // Notes:
  //  1. Value of 'funcAtT0' must lie between f[0] and f[2].
  //  2. Requires function 'NewtonInterp', a second-degree (three point)
  //  Newton interpolation scheme.
  int invLinearInterp(double funcAtT0, double t[3], double f[3], double tol, double &t0)
  {
    // Make sure 'funcAtT0' lies between f[1] and f[2].
    if ((funcAtT0 < f[0] && funcAtT0 < f[2]) ||
      (funcAtT0 > f[0] && funcAtT0 > f[2]))
    {
      return 1;
    }

    // Begin iterative procedure.  Use linear inverse interpolation to get
    // estimate of 't0' ('ti') given 'funcAtT0'.  Then, do direct
    // interpolation to find the value of 'f' ('fi') at 'ti'.  Use 'fi' in
    // linear inverse interpolation to find a new value of 'ti'.  Repeat
    // until process converges.
    const int maxIter = 50;
    double ta = 0.0;
    double tb = 0.0;
    double fa = 0.0;
    double fb = 0.0;
    double fi = 0.0;
    double funcLast = 0.0;
    double ti = 0.0;

    int n = 0;
    do
    {
      ++n;
      if (n > maxIter)
        return 2;

      // Determine values to be used for linear inverse interpolation.
      if (n == 1)
      {
        ta = t[0];
        tb = t[2];
        fa = f[0];
        fb = f[2];
      }
      else
      {
        ta = ti;
        fa = fi;
        if (((funcAtT0 >= fi) && (funcAtT0 <= f[2])) ||
          ((funcAtT0 <= fi) && (funcAtT0 >= f[2])))
        {
          tb = t[2];
          fb = f[2];
        }
        else
        {
          tb = t[0];
          fb = f[0];
        }
      }

      // Do linear inverse interpolation.
      ti = (((funcAtT0 - fb) / (fa - fb)) * (ta - tb)) + tb;

      // Do higher-order direct interpolation to find value of 'f' at 'ti'.
      funcLast = (n == 1) ? funcAtT0 : fi;

      int error = newtonInterp(ti, t, f, fi);
      if (error != 0)
        return 30 + error;

    } while (fabs(fi - funcLast) > tol);

    // Once process converges, set the value of 't0'.
    t0 = ti;

    return 0;
  }

}
