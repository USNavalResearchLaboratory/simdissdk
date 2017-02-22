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
#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Calc/Math.h"

namespace
{
  int testAdditonSeconds()
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
      for (size_t i=0; i<10; i++)
        result += 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 10.1));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i=0; i<10; i++)
        result += .1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), 1.1));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i=0; i<10; i++)
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
      for (size_t i=0; i<10; i++)
        result -= 1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -9.9));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i=0; i<10; i++)
        result -= .1;
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), -0.9));
    }

    {
      simCore::Seconds result(0, .1);
      for (size_t i=0; i<10; i++)
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
      for (size_t i=0; i<10; i++)
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
      for (size_t i=0; i<10; i++)
        result *= .1;
      rv += SDK_ASSERT(!simCore::areEqual(result.Double(), 1e-10, 1e-10));
    }

    {
      simCore::Seconds s1(86400);
      simCore::Seconds s2(86400);
      simCore::Seconds result = s1 * s2;
      rv += SDK_ASSERT(!simCore::areEqual(result.Double(), 7464960000.0));
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
      for (size_t i=0; i<10; i++)
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
      simCore::Seconds result(2147483647000.);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), std::numeric_limits<int>::max()));
    }

    {
      simCore::Seconds result(-2147483647000.);
      rv += SDK_ASSERT(simCore::areEqual(result.Double(), std::numeric_limits<int>::min()));
    }

    return rv;
  }

  int testRound()
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
}

int TimeClassTest(int argc, char* argv[])
{
  int rv = 0;

  rv += testAdditonSeconds();
  rv += testSubtractionSeconds();
  rv += testMultiplicationSeconds();
  rv += testDivisionSeconds();
  rv += testInput();
  rv += testRound();

  std::cout << "TimeClassTest " << (rv == 0 ? "PASSED" : "FAILED") << std::endl;

  return rv;
}
