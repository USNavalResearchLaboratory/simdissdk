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
#ifndef SIMCORE_CALC_RANDOM_H
#define SIMCORE_CALC_RANDOM_H

#include <complex>

#include "simCore/Common/Common.h"
#include "simCore/Calc/Math.h"

// Portable Random Number Generator Class
//
// Classes:      RandomVariable,
//               ComplexRandomVariable,
//               UniformVariable,
//               NormalVariable,
//               ExponentialVariable,
//               PoissonVariable
//
//               DiscreteRandomVariable,
//               DiscreteUniformVariable,
//               GeometricVariable,
//               BinomialVariable
//
//
// Represents:   Random variables:
//
//
// State:        History information necessary to generate a sequence of
//               unique samples.
//
//
// Behavior:     This object produces a sequence of random data samples.
//               The distribution of these samples is governed by 1) the
//               specific class chosen (i.e. uniform, normal, etc.) and
//               2) the constructor parameters (which typically
//               characterize the mean, range or other parameters of
//               the distribution).
//
//               Although it is not recommended practice, constant default
//               initial values will be supplied automatically if the
//               random seeds are not set.
//
//               Independence between random variables is ensured by the
//               implementation (a common pseudo-random generator is used
//               among all the random variables).
//
//               To use:
//
//               1 - select the class with the desired distribution
//               2 - declare an instance of the class object; provide
//                   the mean, variance or any other parameters for the
//                   random variable.
//               3 - use the () operator repeatedly to get sequence of
//                   pseudo-random samples.
//
//
// Example:
//
//               This code adds random noise onto a computer
//               generated signal.  The standard deviation of the noise
//               is 0.1 and its mean is 0.0.
//
//                 NormalVariable   noise (0.0, 0.1) ;
//                 double            x ;
//
//                 while ( ... )
//                 {
//                    // compute a clean signal value
//                    x = ... ;
//
//                    // add on noise
//                    x += noise () ;
//
//                    // save noisy signal sample
//                    ...
//
//                 } // end signal generation
//
//
//               This next example simulates dice rolling, which is
//               useful for game simulations:
//
//                 DiscreteUniformVariable  die_1 (1, 6) ;
//                 DiscreteUniformVariable  die_2 (1, 6) ;
//
//                 while (...)
//                 {
//                    int next_roll = die_1 + die_2 ;
//                      .
//                      .
//                      .
//                 } // end while game continues
//
//
// Notes:        This implementation is built upon information in:
//
//               Keppel, "Random Variables Made Simple",
//               Computer Language (6/93)
//
// =======================================================================

namespace simCore
{
  /** Double precision complex number */
  typedef std::complex<double> Complex;

  /**
  * @brief Common source for all portable random number objects
  *
  * Function used by CRandomVariable and CDiscreteRandomVariable.
  * This random generator function is a common source of data for
  * all random number objects in this file.  A common generator
  * is used to guarantee independence among all objects that produce
  * random data.
  * @param[in ] seeds Series of random seeds
  * @return basic uniformly distributed variable
  */
  SDKCORE_EXPORT double basicUniformVariable(double *seeds);


  /// Abstract class.  Defines a common interface for all random variables.
  class SDKCORE_EXPORT RandomVariable
  {
  public:
    /**
    * This method sets the random number seed
    * @param[in ] seed1 Random seed
    */
    void setSeeds(double seed1 = 0) { seeds_ = seed1; }

    /**
    * This method gets the random number seed
    * @return random number seed
    */
    double seeds() const { return seeds_; }

    /**
    * This method overrides the () operator to generate a pseudo random number sequence
    * @return random number sequence
    */
    virtual double operator()() = 0;

    /// CRandomVariable default constructor
    RandomVariable() { seeds_ = 0; }

    /// CRandomVariable destructor
    virtual ~RandomVariable() {}

  protected:
    double seeds_;  /**< seed for random number generator */
  }; // RandomVariable


  /// Abstract class.  Defines a common interface for all complex random variables.
  class SDKCORE_EXPORT ComplexRandomVariable
  {
  public:
    /**
    * This method sets the random number seed
    * @param[in ] seed1 Random seed
    */
    void setSeeds(double seed1 = 0) { seeds_ = seed1; }

    /**
    * This method gets the random number seed
    * @return random number seed
    */
    double seeds() const { return seeds_; }

    /**
    * This method overrides the () operator to generate a pseudo complex random number sequence
    * @return complex random number sequence
    */
    virtual Complex operator()() = 0;

    /// CComplexRandomVariable default constructor
    ComplexRandomVariable() { seeds_ = 0; }

    /// CComplexRandomVariable destructor
    virtual ~ComplexRandomVariable() {}

  protected:
    double seeds_;  /**< seed for random number generator */

  }; // ComplexRandomVariable


  /// Normal(Gaussian) distribution.  Good for simulating noise.
  class SDKCORE_EXPORT NormalVariable : public RandomVariable
  {
  public:
    /// CNormalVariable constructor, the mean and standard deviation are specified in the constructor
    NormalVariable(double mean = 0.0, double stddev = 1.0)
      : RandomVariable(), mean_(mean), stdDev_(stddev), uSample_(0.), boxMullerSwitch_(true)
    {}

    /// CNormalVariable destructor
    virtual ~NormalVariable() {}

    /**
    * This method overrides the () operator to generate a normal(Gaussian) distribution
    * @return normal(Gaussian) distribution
    */
    double operator()();

    /**
    * This method sets the mean value for the distribution
    * @param[in ] val Mean value for the distribution
    */
    void setMean(double val) { mean_ = val; }

    /**
    * This method sets the standard deviation(variance) for the distribution
    * @param[in ] val Standard deviation(variance) for the distribution
    */
    void setStdDev(double val) { stdDev_ = val; }

    /**
    * This method returns the mean value for the distribution
    * @return mean value for the distribution
    */
    double mean() const { return mean_; }

    /**
    * This method returns the standard deviation(variance) for the distribution
    * @return standard deviation(variance) for the distribution
    */
    double stdDev() const { return stdDev_; }

  protected:
    double    mean_;             /**< mean value of distribution */
    double    stdDev_;           /**< standard deviation(variance) of distribution */

    double    uSample_;          /**< 2nd uniform sample from Gaussian distribution */
    bool      boxMullerSwitch_;  /**< Flag to control use of a bivariate normal distribution or a two-dimension continuous uniform distribution */
  }; // end NormalVariable


  /// Complex(Gaussian) distribution.  Good for simulating noise.
  class SDKCORE_EXPORT GaussianVariable : public ComplexRandomVariable
  {
  public:
    /// CGaussianVariable constructor, the mean and standard deviation are specified in the constructor
    GaussianVariable(double mean = 0.0, double stddev = 1.0)
      : ComplexRandomVariable(), mean_(mean), stdDev_(stddev)
    {}

    /// CGaussianVariable destructor
    virtual ~GaussianVariable() {}

    /**
    * This method overrides the () operator to generate a complex(Gaussian) distribution
    * @return complex(Gaussian) distribution
    */
    Complex operator()();

    /**
    * This method sets the mean value for the distribution
    * @param[in ] val Mean value for the distribution
    */
    void setMean(double val) { mean_ = val; }

    /**
    * This method sets the standard deviation(variance) for the distribution
    * @param[in ] val Standard deviation(variance) for the distribution
    */
    void setStdDev(double val) { stdDev_ = val; }

    /**
    * This method returns the mean value for the distribution
    * @return mean value for the distribution
    */
    double mean() const { return mean_; }

    /**
    * This method returns the standard deviation(variance) for the distribution
    * @return standard deviation(variance) for the distribution
    */
    double stdDev() const { return stdDev_; }

  protected:
    double    mean_;             /**< mean value of distribution */
    double    stdDev_;           /**< standard deviation(variance) of distribution */
  }; // end GaussianVariable


  /// Uniform distribution.  Specify the min and max of the distribution.
  class SDKCORE_EXPORT UniformVariable : public RandomVariable
  {
  public:
    /// CUniformVariable constructor, the min and max values of the distribution are specified in the constructor
    UniformVariable(double min = 0.0, double max = 1.0)
      : RandomVariable(), min_(min), range_(max - min)
    {}

    /// CUniformVariable destructor
    virtual ~UniformVariable() {}

    /**
    * This method overrides the () operator to generate a uniform distribution
    * @return uniform distribution
    */
    double operator()();

    /**
    * This method sets the min and max values of the uniform distribution
    * @param[in ] min value of the uniform distribution
    * @param[in ] max value of the uniform distribution
    */
    void setMinMax(double min, double max) { min_=min; range_=max-min; }

    /**
    * This method returns the minimum value of the uniform distribution
    * @return minimum value of the uniform distribution
    */
    double min() const {return min_;}

    /**
    * This method returns the range of the uniform distribution
    * @return range of the uniform distribution
    */
    double range() const {return range_;}

  protected:
    double    min_;    /**< minimum value of the uniform distribution */
    double    range_;  /**< range of the uniform distribution */
  }; // end UniformVariable


  /// Exponential distribution, companion to Poisson.  Good for simulating amount of time between random events.  E.g. minutes between raindrops, hours between customers, etc.
  class SDKCORE_EXPORT ExponentialVariable : public RandomVariable
  {
  public:
    /**
    * ExponentialVariable constructor, the mean is specified in the constructor
    * @param[in ] mean Mean value for the distribution
    */
    ExponentialVariable(double mean = 1.0)
      : RandomVariable(), mean_(mean)
    {}

    /// ExponentialVariable destructor
    virtual ~ExponentialVariable() {}

    /**
    * This method overrides the () operator to generate an exponential distribution
    * @return exponential distribution
    */
    double operator()();

    /**
    * This method sets the mean value for the distribution
    * @param[in ] val Mean value for the distribution
    */
    void setMean(double val) { mean_ = val; }

    /**
    * This method returns the mean value for the distribution
    * @return mean value for the distribution
    */
    double mean() const { return mean_; }

  protected:
    double    mean_;   /**< mean value of distribution */
  }; // end ExponentialVariable


  /// DiscreteRandomVariable Abstract class.  Defines a common interface for integer distributions.
  class SDKCORE_EXPORT DiscreteRandomVariable
  {
  public:
    /**
    * This method sets the random number seed
    * @param[in ] seed1 Random number seed
    */
    void setSeeds(double seed1 = 0) { seeds_ = seed1; }

    /**
    * This method gets the random number seed
    * @return random number seed
    */
    double seeds() const { return seeds_; }

    /**
    * This method overrides the () operator to generate a pseudo integer random number sequence
    * @return integer random number sequence
    */
    virtual int operator()() = 0;

    /// CDiscreteRandomVariable default constructor
    DiscreteRandomVariable() { seeds_=0; }

    /// CDiscreteRandomVariable destructor
    virtual ~DiscreteRandomVariable() {}

  protected:
    double seeds_;  /**< seed for random number generator */
  }; // end DiscreteRandomVariable


  /// Poisson distribution.  Good for simulating how many random events occur in a fixed time period(e.g. raindrops / min or customers / hour).
  class SDKCORE_EXPORT PoissonVariable : public DiscreteRandomVariable
  {
  public:
    /**
    * CPoissonVariable constructor, the mean is specified in the constructor
    * @param[in ] mean Mean value for the distribution
    */
    PoissonVariable(double mean = 1.0);

    /// CPoissonVariable destructor
    virtual ~PoissonVariable() {}

    /**
    * This method overrides the () operator to generate a Poisson distribution
    * @return Poisson distribution
    */
    int operator()();

    /**
    * This method sets the mean value for the distribution
    * @param[in ] mean Mean value for the distribution
    */
    void setMean(double mean);

    /**
    * This method returns the mean value for the distribution
    * @return mean value for the distribution
    */
    double mean() const { return mean_; }

  protected:
    double    mean_;  /**< mean value of distribution */
  }; // end PoissonVariable


  /// Geometric distribution.  Suppose the probability of getting "heads" when flipping a particular coin is "p"(which may be other than 0.5). This random variable can be used to simulate number of flips required to get the first "heads".
  class SDKCORE_EXPORT GeometricVariable : public DiscreteRandomVariable
  {
  public:
    /**
    * GeometricVariable constructor, the beta is specified in the constructor
    * @param[in ] beta Beta value for the distribution
    */
    GeometricVariable(double beta = 0.5);

    /// GeometricVariable destructor
    virtual ~GeometricVariable() {}

    /**
    * This method overrides the () operator to generate an Geometric distribution
    * @return Geometric distribution
    */
    int operator()();

    /**
    * This method sets the beta value for the distribution
    * @param[in ] val Beta value for the distribution
    */
    void setBeta(double val);

    /**
    * This method returns the beta value for the distribution
    * @return beta value for the distribution
    */
    double beta() const {return beta_;}

  protected:
    double    beta_;  /**< beta value of distribution */
  }; // end GeometricVariable


  /// Binomial distribution.  Suppose the probability of getting "heads" when flipping a particular coin is "p"(which may be other than 0.5). This random variable can be used to simulate number of "heads" that you'll get in "n" flips of the coin.
  class SDKCORE_EXPORT BinomialVariable : public DiscreteRandomVariable
  {
  public:
    /**
    * CBinomialVariable constructor, the number of trials and probability is specified in the constructor
    * @param[in ] n number of trials for the binomial distribution
    * @param[in ] p probability value of the binomial distribution
    */
    BinomialVariable(int n = 1, double p = 0.5)
      : DiscreteRandomVariable(), numTrials_(n), pr_(p)
    { }

    /// CExponentialVariable destructor
    virtual ~BinomialVariable() {}

    /**
    * This method overrides the () operator to generate an binomial distribution
    * @return binomial distribution
    */
    int operator()();

    /**
    * This method sets the number of trials and probability for the binomial distribution
    * @param[in ] pb probability value of the binomial distribution
    * @param[in ] num number of trials for the binomial distribution
    */
    void setProbNumber(double pb, int num) {pr_=pb; numTrials_=num;}

    /**
    * This method returns the number of trials for the binomial distribution
    * @return number of trials for the binomial distribution
    */
    int numTrials() const {return numTrials_;}

    /**
    * This method returns the probability of the binomial distribution
    * @return probability of the binomial distribution
    */
    double prob() const {return pr_;}

  protected:
    int     numTrials_;   /**< number of trials */
    double  pr_;          /**< probability in obtaining exactly n successes out of numTrials_ number of trials */
  }; // end BinomialVariable


  /// Uniform distribution of integer values.  Good for simulating single coin flips or dice rolls
  class SDKCORE_EXPORT DiscreteUniformVariable : public DiscreteRandomVariable
  {
  public:
    /**
    * CDiscreteUniformVariable constructor, the min and max endpoints of the distribution are specified in the constructor
    * @param[in ] min value of the discrete uniform distribution
    * @param[in ] max value of the discrete uniform distribution
    */
    DiscreteUniformVariable(int min = 0, int max = 1)
      : DiscreteRandomVariable(), min_(min), range_(max - min)
    { }

    /// CDiscreteUniformVariable destructor
    virtual ~DiscreteUniformVariable() {}

    /**
    * This method overrides the () operator to generate a discrete uniform distribution
    * @return discrete uniform distribution
    */
    int operator()();

    /**
    * This method sets the min and max values of the discrete uniform distribution
    * @param[in ] min value of the discrete uniform distribution
    * @param[in ] max value of the discrete uniform distribution
    */
    void setMinMax(int min, int max) { min_=min; range_=max-min; }

    /**
    * This method returns the minimum value of the discrete uniform distribution
    * @return minimum value of the discrete uniform distribution
    */
    int min() const {return min_;}

    /**
    * This method returns the range of the discrete uniform distribution
    * @return range of the discrete uniform distribution
    */
    int range() const {return range_;}

  protected:
    int min_;     /**< minimum value of the discrete uniform distribution */
    int range_;   /**< range of the discrete uniform distribution */
  }; // end DiscreteUniformVariable

} // namespace simCore

#endif /* SIMCORE_CALC_RANDOM_H */
