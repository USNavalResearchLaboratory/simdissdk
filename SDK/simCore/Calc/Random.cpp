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
#include "simCore/Calc/Random.h"

namespace simCore {

//=======================================================================
//
// Basic random number generator
//
// This random generator procedure is a common source of data for
// all random number objects in this file.  A common generator
// is used to guarantee independence among all objects that produce
// random data.
//
// This generator produces non-negative double-precision floating-point
// values uniformly distributed over the interval [0.0, 1.0]
//
// The implementation of this procedure depends on the host system,
// although most C++ compilers adhere to the standard C convention
// so modification is unlikely.
//
// Although it is not recommended practice, constant default
// initial values will be supplied automatically if the
// random seeds are not set.
//
//=======================================================================


//=======================================================================
//
// basicUniformVariable function
//
//=======================================================================

double basicUniformVariable(double *seed)
{
  //
  // constants for the pseudo-random number generator
  //
  static const double sA = 16807;
  static const double sM = 2147483647;

  if (!*seed)
  {
    // Use automatic random seeds (default= 10259)
    *seed = 10259;
  }

  const double temp = sA * (*seed);
  *seed = temp - sM * floor(temp/sM);
  return *seed / sM;
}

//=======================================================================
//
// Normal variable member functions
//
//=======================================================================

// Polar form of the Box-Muller transformation.
// Transforms uniformly distributed random variables, to a new set of random
// variables with a Gaussian (or Normal) distribution.
// http://www.taygeta.com/random/gaussian.html
double NormalVariable::operator()()
{
  double sampleVar;

  if (boxMullerSwitch_)
  {
    // Start with two independent random numbers, x1 and x2, which are a
    // uniform distribution [0 to 1].
    double x1, x2, w;
    do
    {
      x1 = 2.0 * basicUniformVariable(&seeds_) - 1.0;
      x2 = 2.0 * basicUniformVariable(&seeds_) - 1.0;
      w = x1 * x1 + x2 * x2;
    } while (w >= 1.0);

    // Apply transformations to get two new independent random numbers which
    // have a Gaussian distribution with zero mean and a standard deviation of one.
    if (w != 0.0)
      w = sqrt((-2.0 * log(w)) / w);
    sampleVar = x1 * w;
    uSample_ = x2 * w;
  }
  else
  {
    // Use the second random number, generated on a previous call
    sampleVar = uSample_;
  }

  // flip switch to swap random numbers for the next call
  boxMullerSwitch_ = !boxMullerSwitch_;

  return stdDev_ * sampleVar + mean_;
}

//=======================================================================
//
// Gaussian variable member functions
//
//=======================================================================

Complex GaussianVariable::operator()()
{
  double var = basicUniformVariable(&seeds_);
  if (var == 1.0)
    var = 0.0; // Avoid log(0) below
  double uSample1 = stdDev_ * sqrt(-2.0 * log(1.0 - var));
  double uSample2 = basicUniformVariable(&seeds_);
  double r = uSample1 * cos(M_TWOPI * uSample2) + mean_;
  double i = uSample1 * sin(M_TWOPI * uSample2) + mean_;
  return Complex(r, i);
}

//=======================================================================
//
// Uniform (real) variable member functions
//
//=======================================================================

double UniformVariable::operator()()
{
  return basicUniformVariable(&seeds_) * range_ + min_;
}

//=======================================================================
//
// Exponential variable member functions
//
//=======================================================================

double ExponentialVariable::operator()()
{
  double var = basicUniformVariable(&seeds_);
  if (var == 0.0)
    var = 0.1; // avoid log(0) below
  return -mean_ * log(var);
}

//=======================================================================
//
// Poisson variable member functions
//
//=======================================================================

PoissonVariable::PoissonVariable(double  mean)
  : DiscreteRandomVariable(), mean_(exp(-mean))
{}

//------------------------------------------------------------------------

void PoissonVariable::setMean(double  mean)
{
  mean_ = exp(-mean);
}

//------------------------------------------------------------------------

int PoissonVariable::operator()()
{
  int    count   = 0;
  double product = basicUniformVariable(&seeds_);

  while (product >= mean_)
  {
    count   ++;
    product *= basicUniformVariable(&seeds_);
  }

  return count;
}

//=======================================================================
//
// Geometric variable member functions
//
//=======================================================================

GeometricVariable::GeometricVariable(double mean)
  : DiscreteRandomVariable()
{
  if (mean == 0.0 || mean == 1.0)
    mean = 0.5; // Avoid log(0) and divide by zero (log(1) == 0) below
  beta_ = 1.0 / log(1.0 - mean);
}

//------------------------------------------------------------------------

void GeometricVariable::setBeta(double val)
{
  if (val == 0.0 || val == 1.0)
    val = 0.5; // Avoid log(0) and divide by zero (log(1) == 0) below
  beta_ = 1.0 / log(1.0 - val);
}

//------------------------------------------------------------------------

int GeometricVariable::operator()()
{
  double var = basicUniformVariable(&seeds_);
  if (var == 0.0)
    var = 0.1; // Avoid log(0) below
  return 1 + static_cast<int>(beta_ * log(var));
}

//=======================================================================
//
// Binomial variable member functions
//
//=======================================================================

int BinomialVariable::operator()()
{
  int count = 0;

  for (int ix=0; ix < numTrials_; ix++)
  {
    if (basicUniformVariable(&seeds_) <= pr_)
    {
      count ++;
    }
  }

  return count;
}

//=======================================================================
//
// Discrete uniform variable member functions
//
//=======================================================================

int DiscreteUniformVariable::operator()()
{
  return static_cast<int>(min_ + range_ * basicUniformVariable(&seeds_) + 0.5);
}

}