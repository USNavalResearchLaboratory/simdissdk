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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Interpolation.h"

namespace
{
  int threeArgLinearInterpolateTest()
  {
    int rv = 0;

    // Test correct responses when inputs are equal
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 0.0, 0.0), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 0.0, 1.0), 0.0));

    // Test responses when a < b
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 1.0, 0.0), 0.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 1.0, 0.3), 0.3));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 1.0, 0.7), 0.7));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(0.0, 1.0, 1.0), 1.0));

    // Test responses when b < a
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(1.0, 0.0, 0.0), 1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(1.0, 0.0, 0.3), 0.7));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(1.0, 0.0, 0.7), 0.3));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(1.0, 0.0, 1.0), 0.0));

    // Test when inputs are negative
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-11.0, -1.0, 0.0), -11.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-11.0, -1.0, 0.3), -8.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-11.0, -1.0, 0.7), -4.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-11.0, -1.0, 1.0), -1.0));

    // Test when inputs are mixed
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-1.0, 1.0, 0.0), -1.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-1.0, 1.0, 0.3), -0.4));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-1.0, 1.0, 0.7), 0.4));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(-1.0, 1.0, 1.0), 1.0));

    return rv;
  }

  int fiveArgLinearInterpolateTest()
  {
    int rv = 0;

    // The five-arg implementation uses the three-arg to interpolate between these, so we don't need to vary these inputs
    const double lowVal = 10.0;
    const double highVal = 20.0;

    // Test correct responses when inputs are equal
    double xLow = 0.0;
    double xHigh = 0.0;
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 0.0, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 1.0, xHigh), highVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -1.0, xHigh), lowVal));

    // Test around the edges
    xHigh = 1.0;
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, xLow - 1, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, xLow, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, xHigh, xHigh), highVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, xHigh + 1, xHigh), highVal));

    // Test responses when a < b
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 0.0, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 0.3, xHigh), 13.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 0.7, xHigh), 17.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 1.0, xHigh), highVal));

    // Test responses when b < a
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xHigh, 1.0, xLow), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xHigh, 0.7, xLow), 13.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xHigh, 0.3, xLow), 17.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xHigh, 0.0, xLow), highVal));

    // Test when inputs are negative
    xLow = -2.0;
    xHigh = -1.0;
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -2.0, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -1.7, xHigh), 13.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -1.3, xHigh), 17.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -1.0, xHigh), highVal));

    // Test when inputs are mixed
    xLow = -1.0;
    xHigh = 1.0;
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -1.0, xHigh), lowVal));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, -0.4, xHigh), 13.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 0.4, xHigh), 17.0));
    rv += SDK_ASSERT(simCore::areEqual(simCore::linearInterpolate(lowVal, highVal, xLow, 1.0, xHigh), highVal));

    return rv;
  }

  int linearInterpolateAngleTest()
  {
    int rv = 0;
    double angle = simCore::linearInterpolateAngle(0.0, 0.0, 0.0);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 0.0));

    angle = simCore::linearInterpolateAngle(1.0, 2.0, 0.5);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 1.5));

    angle = simCore::linearInterpolateAngle(0.0, 360.*simCore::DEG2RAD, 0.0);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 0.0));

    angle = simCore::linearInterpolateAngle(0.0, 360.*simCore::DEG2RAD, 0.5);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 0.0));

    angle = simCore::linearInterpolateAngle(350.*simCore::DEG2RAD, 10.*simCore::DEG2RAD, 0.25);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 355.*simCore::DEG2RAD));

    angle = simCore::linearInterpolateAngle(350.*simCore::DEG2RAD, 10.*simCore::DEG2RAD, 0.5);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 0.0));

    angle = simCore::linearInterpolateAngle(350.*simCore::DEG2RAD, 10.*simCore::DEG2RAD, 0.75);
    rv += SDK_ASSERT(simCore::areAnglesEqual(angle, 5.*simCore::DEG2RAD));

    return rv;
  }

  int linearInterpolateMapTest()
  {
    int rv = 0;
    std::map<double, double> container;
    container[10] = 1;
    container[20] = 2;
    container[30] = 3;
    container[40] = 4;
    container[50] = 5;

    // test clampBgn bounds
    double value = 0;
    bool result = simCore::linearInterpolate(container, 1., value);
    rv += SDK_ASSERT(!result);
    result = simCore::linearInterpolate(container, 1., value, 1e-6, true, false);
    rv += SDK_ASSERT(result && simCore::areEqual(value, 1.));

    // test clampEnd bounds
    result = simCore::linearInterpolate(container, 100., value);
    rv += SDK_ASSERT(!result);
    result = simCore::linearInterpolate(container, 100., value, 1e-6, false, true);
    rv += SDK_ASSERT(result && simCore::areEqual(value, 5.));

    // test exact value
    result = simCore::linearInterpolate(container, 30., value);
    rv += SDK_ASSERT(result && simCore::areEqual(value, 3.));

    // test interpolated value
    result = simCore::linearInterpolate(container, 25., value);
    rv += SDK_ASSERT(result && simCore::areEqual(value, 2.5));

    return rv;
  }

  int bilinearInterpolateTest()
  {
    int rv = 0;
    double value = simCore::bilinearInterpolate(0., 0., 0., 0., 0., 0., 0., 0., 0., 0.);
    rv += SDK_ASSERT(simCore::areEqual(value, 0.));

    // bounds specified counter clock-wise: Type ll, Type lr, Type ur, Type ul
    value = simCore::bilinearInterpolate(1., 3., 3., 1., 1., 2., 3., 1., 2., 3.);
    rv += SDK_ASSERT(simCore::areEqual(value, 2.));

    // bounds specified counter clock-wise: Type ll, Type lr, Type ur, Type ul
    value = simCore::bilinearInterpolate(1., 3., 3., 1., 1., 2., 33., 1., 2., 33.);
    rv += SDK_ASSERT(simCore::areEqual(value, 1.0625));

    return rv;
  }

  int nearestNeighborInterpolateTest()
  {
    int rv = 0;
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., -1.) == 1.);
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., 0.) == 1.);
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., .1) == 1.);
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., .5) == 2.);
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., .6) == 2.);
    rv += SDK_ASSERT(simCore::nearestNeighborInterpolate(1., 2., 1.) == 2.);

    return rv;
  }
}

int InterpolationTest(int argc, char* argv[])
{
  int rv = 0;

  rv += threeArgLinearInterpolateTest();
  rv += fiveArgLinearInterpolateTest();
  rv += linearInterpolateAngleTest();
  rv += linearInterpolateMapTest();
  rv += bilinearInterpolateTest();
  rv += nearestNeighborInterpolateTest();

  std::cout << "InterpolationTest " << ((rv == 0) ? "Passed" : "Failed") << std::endl;

  return rv;
}
