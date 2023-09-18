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


    // SIM-12482: In the context of numbers that can be represented by TimeStamp,
    // a double can resolve a 1 microsecond time difference.
    double startTime = maxTimeStampSecs.Double();
    // double can resolve 1 microsecond
    rv += SDK_ASSERT((startTime - 1e-06) < startTime);
    // cannot resolve 100ns
    rv += SDK_ASSERT((startTime - 1e-07) == startTime);

    // Converting any double into and out of TimeStamp does not lose any precision (that double can resolve).
    for (int i = 1; i < 1000; ++i)
    {
      double newTime = startTime - 1e-06;
      // verify that double can detect the difference
      rv += SDK_ASSERT(newTime < startTime);

      simCore::TimeStamp inTimeStamp(1970, newTime);
      double outTime = inTimeStamp.secondsSinceRefYear(1970);
      rv += SDK_ASSERT(newTime == outTime);

      startTime = newTime;
    }
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

  int testTimeStampStrStrptime()
  {
    int rv = 0;
    simCore::TimeStampStr tss;
    simCore::TimeStamp ts;
    std::string remain;

    // Test individual components
    rv += SDK_ASSERT(tss.strptime(ts, "10", "%d", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 9 * 86400));
    rv += SDK_ASSERT(remain.empty());

    // %y on Linux systems demonstrated different behavior than MSVC 2022. For example,
    // "10" was parsed as 2010 (110) on Windows, but only 10 (1910) on Linux

    rv += SDK_ASSERT(tss.strptime(ts, "2010", "%Y", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2010));
    rv += SDK_ASSERT(remain.empty());

    rv += SDK_ASSERT(tss.strptime(ts, "2", "%m", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 * 31));
    rv += SDK_ASSERT(remain.empty());

    rv += SDK_ASSERT(tss.strptime(ts, "10", "%H", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 36000));
    rv += SDK_ASSERT(remain.empty());

    rv += SDK_ASSERT(tss.strptime(ts, "10", "%M", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 600));
    rv += SDK_ASSERT(remain.empty());

    rv += SDK_ASSERT(tss.strptime(ts, "10", "%S", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 10));
    rv += SDK_ASSERT(remain.empty());

    // Test some combined times/dates
    rv += SDK_ASSERT(tss.strptime(ts, "1:02:03", "%H:%M:%S", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 3600 + 120 + 3));
    rv += SDK_ASSERT(remain.empty());

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03", "%m/%d/%Y %H:%M:%S", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));
    rv += SDK_ASSERT(remain.empty());

    // Testing demonstrated errors in either C++ documentation or MSVC 2022
    // implementation with %j. %j is "day of year as decimal range 001-366",
    // so 001 is January 1. This is tm_yday of 0. %j indicates this is
    // tm_yday of 1, which is wrong.

    // Test failing content:
    // Letters for a number
    rv += SDK_ASSERT(tss.strptime(ts, "abc", "%S", &remain) != 0);
    // Missing tokens (:) and not enough digits
    rv += SDK_ASSERT(tss.strptime(ts, "1 2", "&H:%M:%S", &remain) != 0);
    // Out of bounds values
    rv += SDK_ASSERT(tss.strptime(ts, "1:2:63", "%H:%M:%S", &remain) != 0);
    // Invalid format string
    rv += SDK_ASSERT(tss.strptime(ts, "1:2:3", "&H:%M:%S", &remain) != 0);
    rv += SDK_ASSERT(tss.strptime(ts, "1", "%f", &remain) != 0);

    // Successes with trailing characters
    rv += SDK_ASSERT(tss.strptime(ts, "15.f", "%S", &remain) == 0);
    rv += SDK_ASSERT(remain == ".f");
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.5", "%m/%d/%Y %H:%M:%S", &remain) == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));
    rv += SDK_ASSERT(remain == ".5");

    // Successes with trailing fraction
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.1", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.1));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.12", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.12));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.123", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.123));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.1234", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.1234));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.12345", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.12345));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.123456", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(simCore::areEqual(ts.secondsSinceRefYear(), 86400 + 3600 + 120 + 3 + 0.123456));
    rv += SDK_ASSERT(simCore::areEqual(ts.referenceYear(), 2012));

    // Allow trailing extra characters
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.1f", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.1 UTC", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.f", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.f1", "%m/%d/%Y %H:%M:%S") == 0);
    rv += SDK_ASSERT(tss.strptime(ts, "1/2/2012 1:02:03.1.1", "%m/%d/%Y %H:%M:%S") == 0);

    return rv;
  }

  int testTimeStampStrStrftime()
  {
    int rv = 0;

    simCore::TimeStampStr tss;
    // January 3, 14:52:17.8
    const simCore::TimeStamp jan4_14_52_17(2022, 86400 * 3 + 3600 * 14 + 60 * 52 + 17.8);

    // Test individual components: Year
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%Y") == "2022");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%y") == "22");

    // Month (%b and %B are locale-dependent and may fail on some systems)
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%b") == "Jan");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%B") == "January");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%m") == "01");

    // Day of month
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%j") == "004");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%d") == "04");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%e") == " 4");

    // H/M/S
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%H") == "14");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%I") == "02");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%M") == "52");
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%S") == "17");
    // Note, no millisecond representation

    // Attempt to "flood" output string. This is white box testing, since we know that
    // the C implementation can have a format string significantly smaller than the output
    // and the C function doesn't tell you exactly how big to make the buffer.
    std::string manyJan;
    std::string formatStr;
    for (int k = 0; k < 1500; ++k)
    {
      formatStr += "%B";
      manyJan += "January";
    }
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, formatStr) == manyJan);

    // Percentage encoded
    rv += SDK_ASSERT(tss.strftime(jan4_14_52_17, "%%") == "%");

    // Bounds checking
    rv += SDK_ASSERT(tss.strftime(simCore::MIN_TIME_STAMP, "%m/%d/%Y %H:%M:%S") == "01/01/1970 00:00:00");
    rv += SDK_ASSERT(tss.strftime(simCore::MAX_TIME_STAMP, "%m/%d/%Y %H:%M:%S") == "12/31/2200 23:59:59");
    rv += SDK_ASSERT(tss.strftime(simCore::INFINITE_TIME_STAMP, "%m/%d/%Y %H:%M:%S") == "");

#if !defined(_MSC_VER) || defined(NDEBUG)
    // Invalid specifier. This asserts in MSVC code (even with invalid handler),
    // so do not test this in debug mode. The return value on MSVC is empty string
    // because it cannot process the input. On Linux, the input string is returned.
    // Therefore we permit either empty string or input string here in failure.
#ifdef _MSC_VER
    std::cout << "Invalid argument being passed in, exception that follows is normal:\n";
#endif
    const std::string& failResult = tss.strftime(jan4_14_52_17, "%3");
    rv += SDK_ASSERT(failResult == "" || failResult == " %3" || failResult == "3" || failResult == "%3");
#endif

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
  rv += testTimeStampStrStrptime();
  rv += testTimeStampStrStrftime();

  std::cout << "TimeClassTest " << (rv == 0 ? "PASSED" : "FAILED") << std::endl;

  return rv;
}
