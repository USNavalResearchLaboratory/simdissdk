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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Time/Constants.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"
#include "simCore/Calc/Math.h"

namespace
{
  int testAdditionSeconds()
  {
    int rv = 0;

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(5, 0);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 6.));
    }

    {
      simCore::Seconds s1(0, 0);
      simCore::Seconds s2(0, 0);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.));
    }

    {
      simCore::Seconds s1(0, .1);
      simCore::Seconds s2(0, .5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.6));
    }

    {
      simCore::Seconds s1(0, -.1);
      simCore::Seconds s2(0, -.5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.6));
    }

    {
      simCore::Seconds s1(-0.1);
      simCore::Seconds s2(-0.5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.6));
    }

    {
      simCore::Seconds s1(0, 1);
      simCore::Seconds s2(0, 5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 6e-9, 1e-9));
    }

    {
      simCore::Seconds s1(1);
      simCore::Seconds s2(1);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 2));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(0);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.1));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(2.07);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 3.17));
    }

    {
      simCore::Seconds s1(1, .5);
      simCore::Seconds s2(-1, 0);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(0, -0.5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(-1, -0.5);
      simCore::Seconds result = s1 + s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.5));
    }

    {
      simCore::Seconds result(20, 0);
      result += -1.5;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 18.5));
    }

    {
      simCore::Seconds result(-20, 0);
      result += -1.5;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -21.5));
    }

    {
      simCore::Seconds result(-20, 0);
      result += 1.5;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -18.5));
    }

    {
      simCore::Seconds result(20, 0);
      result += 1.5;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 21.5));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result += 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 10.1));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result += .1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.1));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        ++result;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 10.1));
    }

    return rv;
  }

  int testSubtractionSeconds()
  {
    int rv = 0;

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(5, 0);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -4.));
    }

    {
      simCore::Seconds s1(0, 0);
      simCore::Seconds s2(0, 0);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.));
    }

    {
      simCore::Seconds s1(0, .1);
      simCore::Seconds s2(0, .5);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.4));
    }

    {
      simCore::Seconds s1(0, 1);
      simCore::Seconds s2(0, 5);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -4e-9, 1e-9));
    }

    {
      simCore::Seconds s1(1);
      simCore::Seconds s2(1);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(0);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.1));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(2.07);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.97));
    }

    {
      simCore::Seconds s1(1, .5);
      simCore::Seconds s2(-1, 0);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 2.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(0, -0.5);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(-1, -0.5);
      simCore::Seconds result = s1 - s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 2.5));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result -= 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -9.9));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result -= .1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.9));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        --result;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -9.9));
    }

    return rv;
  }

  int testMultiplicationSeconds()
  {
    int rv = 0;

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(5, 0);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 5.));
    }

    {
      simCore::Seconds s1(0, 0);
      simCore::Seconds s2(0, 0);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.));
    }

    {
      simCore::Seconds s1(0, .1);
      simCore::Seconds s2(0, .5);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.05));
    }

    {
      simCore::Seconds s1(1);
      simCore::Seconds s2(1);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(0);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds result(0);
      result.scale(1.1);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds result(1.1);
      result.scale(0);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(2.07);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 2.277));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds result(2.07);
      result.scale(1.1);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 2.277));
    }

    {
      simCore::Seconds s1(1, .5);
      simCore::Seconds s2(-1, 0);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(0, -0.5);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.5));
    }

    {
      simCore::Seconds result(0, -0.5);
      result.scale(1);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(-1, -0.5);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds result(-1, -0.5);
      result.scale(1);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds s1(-1, 0);
      simCore::Seconds s2(-1, -0.5);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.5));
    }

    {
      simCore::Seconds result(-1, -0.5);
      result.scale(-1);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.5));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result *= 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), .1));
    }

    // failures that exceed storage limits of Seconds class

    {
      simCore::Seconds s1(0, 1);
      simCore::Seconds s2(0, 5);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(!simCore::areEqual(result.Double(), 5e-18, 1e-20));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result *= .1;
      rv += SDK_ASSERT(!simCore::areEqual(result.Double(), 1e-10, 1e-10));
    }

    return rv;
  }

  int testDivisionSeconds()
  {
    int rv = 0;

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(5, 0);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), .2));
    }

    {
      simCore::Seconds s1(0, 0);
      simCore::Seconds s2(0, 0);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.));
    }

    {
      simCore::Seconds s1(0, .1);
      simCore::Seconds s2(0, .5);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.2));
    }

    {
      simCore::Seconds s1(0, 1);
      simCore::Seconds s2(0, 5);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), .2));
    }

    {
      simCore::Seconds s1(1);
      simCore::Seconds s2(1);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1));
    }

    {
      // NOTE: divide by zero will result in zero seconds
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(0);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds s1(0);
      simCore::Seconds s2(1.1);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0));
    }

    {
      simCore::Seconds s1(1.1);
      simCore::Seconds s2(2.07);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.5314001));
    }

    {
      simCore::Seconds s1(1, .5);
      simCore::Seconds s2(-1, 0);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(0, -0.5);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -2));
    }

    {
      simCore::Seconds s1(1, 0);
      simCore::Seconds s2(-1, -0.5);
      simCore::Seconds result = s1 / s2;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.6666667));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i = 0; i < 10; i++)
        result /= 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), .1));
    }

    return rv;
  }

  int testInput()
  {
    int rv = 0;

    {
      simCore::Seconds result(-1, .5);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.5));
    }

    {
      simCore::Seconds result(1, -.5);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 0.5));
    }

    {
      simCore::Seconds result(-1, -.5);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds result = -1.5;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds result(-1.5);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    {
      simCore::Seconds result(-1, 500000000);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.5));
    }

    {
      simCore::Seconds result(0, -1500000000);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -1.5));
    }

    // test going over the limits
    {
      simCore::Seconds result(std::numeric_limits<int64_t>::max() * 10.0);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), static_cast<double>(std::numeric_limits<int64_t>::max())));
    }

    {
      simCore::Seconds result(std::numeric_limits<int64_t>::min() * 10.0);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), static_cast<double>(std::numeric_limits<int64_t>::min())));
    }

    return rv;
  }

  int testTimeRounding()
  {
    int rv = 0;
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.4).rounded(0).Double(), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.499).rounded(0).Double(), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.5).rounded(0).Double(), 1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.0).rounded(0).Double(), 1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.499).rounded(0).Double(), 1.0));

    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.4).rounded(1).Double(), 0.4));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.499).rounded(1).Double(), 0.5));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.5).rounded(1).Double(), 0.5));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.0).rounded(1).Double(), 1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.499).rounded(1).Double(), 1.5));

    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.04).rounded(1).Double(), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.0499).rounded(1).Double(), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.05).rounded(1).Double(), 0.1));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.00).rounded(1).Double(), 1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.0499).rounded(1).Double(), 1.0));

    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.304).rounded(2).Double(), 0.30));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.30499).rounded(2).Double(), 0.30));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(0.305).rounded(2).Double(), 0.31));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.300).rounded(2).Double(), 1.30));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.30499).rounded(2).Double(), 1.30));

    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.993456789).rounded(9).Double(), 1.993456789, 1e-9));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.993456789).rounded(8).Double(), 1.99345679, 1e-9));
    rv += SDK_ASSERT(simCore::areEqual(simCore::Seconds(1.993456789).rounded(7).Double(), 1.9934568, 1e-9));
    return rv;
  }

  int testTimeStamp()
  {
    int rv = 0;

    // Make sure the constants are consistent
    rv += SDK_ASSERT(simCore::MIN_TIME_STAMP.referenceYear() == simCore::MIN_TIME_YEAR);
    rv += SDK_ASSERT(simCore::MAX_TIME_STAMP.referenceYear() == simCore::MAX_TIME_YEAR);

    // Make sure there is microsecond resolution
    rv += SDK_ASSERT((simCore::MIN_TIME_STAMP + simCore::Seconds(0, 1000)).secondsSinceRefYear().Double() != simCore::MIN_TIME_STAMP.secondsSinceRefYear().Double());
    rv += SDK_ASSERT((simCore::MAX_TIME_STAMP - simCore::Seconds(0, 1000)).secondsSinceRefYear().Double() != simCore::MAX_TIME_STAMP.secondsSinceRefYear().Double());
    rv += SDK_ASSERT((simCore::MAX_TIME_STAMP - simCore::Seconds(0, 1000)).secondsSinceRefYear(simCore::MIN_TIME_YEAR).Double() != simCore::MAX_TIME_STAMP.secondsSinceRefYear(simCore::MIN_TIME_YEAR).Double());

    // Handle bogus values
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_STAMP.referenceYear(), std::numeric_limits<double>::max()) == simCore::MAX_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_STAMP.referenceYear(), -0.1) == simCore::MIN_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_STAMP.referenceYear(), -std::numeric_limits<double>::max()) == simCore::MIN_TIME_STAMP);

    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MAX_TIME_STAMP.referenceYear(), 365 * simCore::SECPERDAY) == simCore::MAX_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MAX_TIME_STAMP.referenceYear(), std::numeric_limits<double>::max()) == simCore::MAX_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MAX_TIME_STAMP.referenceYear(), -std::numeric_limits<double>::max()) == simCore::MIN_TIME_STAMP);

    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_YEAR - 1, 0) == simCore::MIN_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MAX_TIME_YEAR + 1, 0) == simCore::MAX_TIME_STAMP);

    // test correct fix from min to max
    const simCore::Seconds secs = simCore::MAX_TIME_STAMP.secondsSinceRefYear(simCore::MIN_TIME_YEAR);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_YEAR, secs) == simCore::MAX_TIME_STAMP);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_YEAR, (secs - simCore::Seconds(0.1))) != simCore::MAX_TIME_STAMP);

    // test correct fix from max to min
    const simCore::Seconds negSecs = secs * simCore::Seconds(-1, 0);
    rv += SDK_ASSERT((simCore::MAX_TIME_STAMP + negSecs) == simCore::MIN_TIME_STAMP);
    rv += SDK_ASSERT((simCore::MAX_TIME_STAMP + (negSecs + simCore::Seconds(0.1))) != simCore::MIN_TIME_STAMP);

    // test a particular case for correct leap day behavior
    const simCore::TimeStamp ts2001(2001, 3. * simCore::SECPERDAY);
    const simCore::Seconds& secs2001_1972 = ts2001.secondsSinceRefYear(1972);
    simCore::TimeStamp ts1972(1972, secs2001_1972);
    rv += SDK_ASSERT(ts2001 == ts1972);
    const simCore::Seconds& secs1972_2001 = ts1972.secondsSinceRefYear(2001);
    rv += SDK_ASSERT(secs1972_2001.getSeconds() == (3. * simCore::SECPERDAY));

    const simCore::Seconds& secs2001_1973 = ts2001.secondsSinceRefYear(1973);
    simCore::TimeStamp ts1973(1973, secs2001_1973);
    rv += SDK_ASSERT(ts2001 == ts1973);
    const simCore::Seconds& secs1973_2001 = ts1973.secondsSinceRefYear(2001);
    rv += SDK_ASSERT(secs1973_2001.getSeconds() == (3. * simCore::SECPERDAY));

    // Test reference time when result is negative
    const simCore::TimeStamp ts1990(1990, 1.0);
    rv += SDK_ASSERT(ts1990.secondsSinceRefYear() == 1.0);
    rv += SDK_ASSERT(ts1990.secondsSinceRefYear(1990) == 1.0);
    const int SECPERYEAR = 365 * simCore::SECPERDAY;
    rv += SDK_ASSERT(ts1990.secondsSinceRefYear(1989) == SECPERYEAR + 1);
    rv += SDK_ASSERT(ts1990.secondsSinceRefYear(1991) == 1 - SECPERYEAR);

    return rv;
  }
  int testTimeStampComparison()
  {
    int rv = 0;

    // tests below demonstrate that there is variability in ability of Seconds::compare() to detect a difference between two timestamps
    // due to limits in the resolution of double especially when its magnitude is large.
    // that is: two timestamps may be different but compare()'d to be equal
    // in normal cases, this tolerance is on the order of a 1 or 2 ns; in the worst case (below), the tolerance is 71ns.

    // this variability in resolution is acceptable as long as the detection of an inequality is consistent and correct.
    // that is: two timestamps that compare() detects as inequal are actually inequal.
    //     and: two timestamps that are/(should be) equal are always detected to be equal.

    // various TimeStringTest cases demonstrate that two timestamps that should be equal are off by 1ns
    // due to noise from string->double->Seconds conversion.
    // this +/- 1ns limitation of comparison resolution is coded into Seconds::compare().

    // test that Seconds::compare correctly ignore +/- 1 ns differences in its fractional part
    rv += SDK_ASSERT(simCore::TimeStamp(1973, simCore::Seconds(10, 0)) == simCore::TimeStamp(1973, simCore::Seconds(10, 1)));
    rv += SDK_ASSERT(simCore::TimeStamp(1973, simCore::Seconds(10, 1)) == simCore::TimeStamp(1973, simCore::Seconds(10, 2)));
    rv += SDK_ASSERT(simCore::TimeStamp(1973, 0.0) == simCore::TimeStamp(1973, 1e-09));

    // this is a special case: difference is not in the fractional part
    rv += SDK_ASSERT(simCore::TimeStamp(1973, simCore::Seconds(10, 0)) > simCore::TimeStamp(1973, simCore::Seconds(10, -1)));

    // correctly detect +/- 2ns differences
    rv += SDK_ASSERT(simCore::TimeStamp(1973, simCore::Seconds(0, 1)) < simCore::TimeStamp(1973, simCore::Seconds(0, 3)));
    rv += SDK_ASSERT(simCore::TimeStamp(1973, simCore::Seconds(0, 3)) > simCore::TimeStamp(1973, simCore::Seconds(0, 1)));
    rv += SDK_ASSERT(simCore::TimeStamp(1973, 0.0) < simCore::TimeStamp(1973, 2e-09));


    // this is the max value of a Seconds instance that can be obtained from a TimeStamp
    const simCore::Seconds& maxTimeStampSecs = simCore::MAX_TIME_STAMP.secondsSinceRefYear(simCore::MIN_TIME_YEAR);

    // can't resolve 71ns difference due to loss of precision when using a (large) double to construct a Seconds instance.
    const simCore::Seconds secsm71 = maxTimeStampSecs - simCore::Seconds(0, 71);
    const simCore::TimeStamp max(simCore::MIN_TIME_YEAR, maxTimeStampSecs.Double());
    const simCore::TimeStamp maxm71(simCore::MIN_TIME_YEAR, secsm71.Double());
    rv += SDK_ASSERT(max.secondsSinceRefYear().getFractionLong() == maxm71.secondsSinceRefYear().getFractionLong());
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_YEAR, maxTimeStampSecs.Double()) == simCore::TimeStamp(simCore::MIN_TIME_YEAR, secsm71.Double()));

    // can resolve 72ns
    const simCore::Seconds secsm72 = maxTimeStampSecs - simCore::Seconds(0, 72);
    rv += SDK_ASSERT(simCore::TimeStamp(simCore::MIN_TIME_YEAR, maxTimeStampSecs.Double()) > simCore::TimeStamp(simCore::MIN_TIME_YEAR, secsm72.Double()));

    return rv;
  }

  int testNegativeSeconds()
  {
    int rv = 0;

    rv += SDK_ASSERT(simCore::TimeStamp(2020, simCore::Seconds(-1, 0)) == simCore::TimeStamp(2019, simCore::Seconds(365 * simCore::SECPERDAY - 1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2021, simCore::Seconds(-1, 0)) == simCore::TimeStamp(2020, simCore::Seconds(366 * simCore::SECPERDAY - 1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2021, simCore::Seconds(-366 * simCore::SECPERDAY - 1, 0)) == simCore::TimeStamp(2019, simCore::Seconds(365 * simCore::SECPERDAY - 1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2021, simCore::Seconds(-(366 + 365) * simCore::SECPERDAY - 1, 0)) == simCore::TimeStamp(2018, simCore::Seconds(365 * simCore::SECPERDAY - 1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2021, simCore::Seconds(-(2 * 366 + 4 * 365) * simCore::SECPERDAY - 1, 0)) == simCore::TimeStamp(2014, simCore::Seconds(365 * simCore::SECPERDAY - 1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2021, simCore::Seconds(-(13 * 366 + 37 * 365) * simCore::SECPERDAY - 1, 0)) == simCore::TimeStamp(1970, simCore::Seconds(365 * simCore::SECPERDAY - 1, 0)));

    return rv;
  }

  int testPositiveSeconds()
  {
    int rv = 0;

    rv += SDK_ASSERT(simCore::TimeStamp(2019, simCore::Seconds(1, 0)) == simCore::TimeStamp(2019, simCore::Seconds(1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2019, simCore::Seconds(365 * simCore::SECPERDAY + 1, 0)) == simCore::TimeStamp(2020, simCore::Seconds(1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2019, simCore::Seconds((365 + 366) * simCore::SECPERDAY + 1, 0)) == simCore::TimeStamp(2021, simCore::Seconds(1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(2015, simCore::Seconds((2 * 366 + 4 * 365) * simCore::SECPERDAY + 1, 0)) == simCore::TimeStamp(2021, simCore::Seconds(1, 0)));
    rv += SDK_ASSERT(simCore::TimeStamp(1970, simCore::Seconds((13 * 366 + 38 * 365) * simCore::SECPERDAY + 1, 0)) == simCore::TimeStamp(2021, simCore::Seconds(1, 0)));

    return rv;
  }
}

int TimeClassTest(int argc, char* argv[])
{
  int rv = 0;

  rv += testAdditionSeconds();
  rv += testSubtractionSeconds();
  rv += testMultiplicationSeconds();
  rv += testDivisionSeconds();
  rv += testInput();
  rv += testTimeRounding();
  rv += testTimeStamp();
  rv += testNegativeSeconds();
  rv += testPositiveSeconds();
  rv += testTimeStampComparison();

  std::cout << "TimeClassTest " << (rv == 0 ? "PASSED" : "FAILED") << std::endl;

  return rv;
}
