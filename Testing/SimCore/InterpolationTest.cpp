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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Interpolation.h"

namespace
{
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

  rv += linearInterpolateAngleTest();
  rv += linearInterpolateMapTest();
  rv += bilinearInterpolateTest();
  rv += nearestNeighborInterpolateTest();

  std::cout << "InterpolationTest " << ((rv == 0) ? "Passed" : "Failed") << std::endl;

  return rv;
}
