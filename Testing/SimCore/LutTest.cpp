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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/LUT/InterpTable.h"

namespace
{
// helper function
void initAndPopulate(simCore::LUT::LUT2<float>& lut, const std::vector<float>& x, const std::vector<float>& y, const std::vector<float>& val)
{
  lut.initialize(static_cast<double>(*x.begin()), static_cast<double>(*x.rbegin()), x.size(), static_cast<double>(*y.begin()), static_cast<double>(*y.rbegin()), y.size(), 0.f);
  for (auto xiter = x.begin(); xiter != x.end(); ++xiter)
  {
    for (auto yiter = y.begin(); yiter != y.end(); ++yiter)
    {
      const auto xindex = std::distance(x.begin(), xiter);
      const auto yindex = std::distance(y.begin(), yiter);
      lut(xindex, yindex) = val[(xindex * lut.numY()) + yindex];
    }
  }
}

int lutInterpolateTest()
{
  int rv = 0;

  {
  simCore::LUT::LUT2<float> lut2; //5 vectors of 4
  std::vector<float> x{1,2,3,4,5};
  std::vector<float> y{10,20,30,40};
  // internally, 5 vectors, each containing one 4 value vector
  std::vector<float> val{100,200,300,400, 500,600,700,800, 900,1000,1100,1200, 1300,1400,1500,1600, 1700,1800,1900,2000};
  initAndPopulate(lut2, x, y, val);
  rv += SDK_ASSERT(lut2(0, 1) == 200.f);
  rv += SDK_ASSERT(lut2(1, 0) == 500.f);
  rv += SDK_ASSERT(lut2(2, 3) == 1200.f);
  BilinearInterpolate<float> bil;
  auto interpVal = simCore::LUT::interpolate(lut2, 1.f, 15.f, bil);
  rv += SDK_ASSERT(interpVal == 150.f);
  interpVal = simCore::LUT::interpolate(lut2, 1.5f, 10.f, bil);
  rv += SDK_ASSERT(interpVal == 300.f);
  }

  // simple interpolation for a 2x2 LUT2 
  {
    simCore::LUT::LUT2<float> lut2;
    std::vector<float> x{1,2};
    std::vector<float> y{10,20};
    std::vector<float> val{ 100,200, 300,600 };
    initAndPopulate(lut2, x, y, val);
    BilinearInterpolate<float> bil;
    auto interpVal = simCore::LUT::interpolate(lut2, 1., 12., bil);
    rv += SDK_ASSERT(interpVal == 120.f);
    interpVal = simCore::LUT::interpolate(lut2, 1.02, 12., bil);
    rv += SDK_ASSERT(interpVal == 124.800003f);
    interpVal = simCore::LUT::interpolate(lut2, 1.1, 12., bil);
    rv += SDK_ASSERT(interpVal == 144.f);
    interpVal = simCore::LUT::interpolate(lut2, 1.11, 12., bil);
    rv += SDK_ASSERT(interpVal == 146.399994f);
  }

  // interpolation with noData for a 2x2 LUT2 (similar to the LUT above)
  {
    simCore::LUT::LUT2<float> lut2;
    std::vector<float> x{ 1,2 };
    std::vector<float> y{ 10,20 };
    std::vector<float> val{ 100,-99, -99,600 };
    initAndPopulate(lut2, x, y, val);
    lut2.setNoDataValue(-99.f);
    BilinearInterpolate<float> bil;
    auto optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1., 12., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && optionalInterpVal == 100.f);
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1.02, 12., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && optionalInterpVal == 102.f);
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1.1, 12., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && optionalInterpVal == 110.f);
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1.11, 12., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && optionalInterpVal == 200.f);
  }

  // interpolation with noData for a 5x4 LUT2
  {
    simCore::LUT::LUT2<float> lut2;
    std::vector<float> x{ 1,2,3,4,5 };
    std::vector<float> y{ 10,20,30,40 };
    std::vector<float> val{
      //      y= 10, 20, 30, 40      
      /* x=1 */ 100,-99,300,400,
      /* x=2 */ -99,600,700,800,
      /* x=3 */ -99,-99,1100,1200,
      /* x=4 */1300,1400,1500,1600,
      /* x=5 */1700,1800,1900,2000 };

    initAndPopulate(lut2, x, y, val);
    lut2.setNoDataValue(-99.f);
    rv += SDK_ASSERT(lut2(0, 1) == -99.f);
    rv += SDK_ASSERT(lut2(1, 2) == 700.f);
    BilinearInterpolate<float> bil;
    auto interpVal = simCore::LUT::interpolate(lut2, 1., 15., bil);
    rv += SDK_ASSERT(interpVal == 0.5f); // this is not a reasonable value, due to blindly interpolating with noData val

    // 4 good vals
    auto optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 4.5, 15., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && *optionalInterpVal == 1550.f);

    // 3 good vals, one noData val.
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1.5, 25., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && *optionalInterpVal == 550);

    // 2 noData val
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1.5, 15., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && *optionalInterpVal == 350.f);

    // 2 noData val, but x arg constrains interpolation to only x=1 values
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 1., 15., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && *optionalInterpVal == 100.f);


    // 3 noData; outside of closeness criterion, can select the one data val of the 4 (600)
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 2., 12., bil);
    rv += SDK_ASSERT(optionalInterpVal.has_value() && *optionalInterpVal == 600.f);
    // 3 noData, but y=11. constrains selection of a val from higher y
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 2., 11., bil);
    rv += SDK_ASSERT(!optionalInterpVal.has_value());
    // 3 noData, but X constrains selection of a val from lower x
    optionalInterpVal = simCore::LUT::interpolateWithNoDataValue(lut2, 2.9, 15., bil);
    rv += SDK_ASSERT(!optionalInterpVal.has_value());
  }
  return rv;
}

}

int LutTest(int argc, char* argv[])
{
  int rv = 0;
  rv += lutInterpolateTest();
  return rv;
}
