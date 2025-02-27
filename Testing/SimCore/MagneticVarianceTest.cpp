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
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/MagneticVariance.h"

namespace
{
  int calculateMagneticVarianceTest()
  {
    simCore::WorldMagneticModel wmm;
    simCore::Vec3 lla;
    double varianceRad;

    // Based on TestValues2015v2.pdf from NOAA
    int rv = 0;
    int funcRv = 0;
    double alt = 0.;
    int ordinalDay = 0;
    int year = 2015;

    // Declination (variance) values pulled from column 11: D (Deg)
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -3.9, 0.1) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.55, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.81, 0.01) && funcRv == 0);

    alt = 100. * 1000.; // PDF value in km
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -4.32, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.54, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.22, 0.01) && funcRv == 0);

    // 2017.5 values
    ordinalDay = 183;
    year = 2017;
    alt = 0.;
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -2.59, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.37, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.59, 0.01) && funcRv == 0);

    alt = 100. * 1000.; // PDF value in km
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -3.01, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.36, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.01, 0.01) && funcRv == 0);

    /////////////////////////////////
    // Based on WMM2020testvalues.pdf
    // Released Dec 10, 2019
    year = 2020;
    ordinalDay = 0;
    alt = 0.;

    // Declination (variance) values pulled from column 11: D (Deg)
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -1.28, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.16, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.36, 0.01) && funcRv == 0);

    alt = 100. * 1000.; // PDF value in km
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -1.70, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.16, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 68.78, 0.01) && funcRv == 0);

    // 2022.5 values
    ordinalDay = 183;
    year = 2022;
    alt = 0.;
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 0.01, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -0.06, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 69.13, 0.01) && funcRv == 0);

    alt = 100. * 1000.; // PDF value in km
    lla.set(80.*simCore::DEG2RAD, 0., alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -0.41, 0.01) && funcRv == 0);

    lla.set(0., 120.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, -0.05, 0.01) && funcRv == 0);

    lla.set(-80.*simCore::DEG2RAD, 240.*simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad*simCore::RAD2DEG, 68.55, 0.01) && funcRv == 0);


    /////////////////////////////////
    // Based on WMM2025testvalues.pdf
    // Released Dec, 2024
    year = 2025;
    ordinalDay = 0;

    // Declination (variance) values pulled from column 5: declination (deg)
    alt = 28. * 1000.; // PDF value in km
    lla.set(89. * simCore::DEG2RAD, -121. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -99.77, 0.01) && funcRv == 0);

    alt = 48. * 1000.; // PDF value in km
    lla.set(80. * simCore::DEG2RAD, -96. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -29.91, 0.01) && funcRv == 0);

    alt = 54. * 1000.; // PDF value in km
    lla.set(82. * simCore::DEG2RAD, 87. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 54.89, 0.01) && funcRv == 0);

    alt = 65. * 1000.; // PDF value in km
    lla.set(43. * simCore::DEG2RAD, 93. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 0.5, 0.01) && funcRv == 0);

    alt = 51. * 1000.; // PDF value in km
    lla.set(-33. * simCore::DEG2RAD, 109. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -5.49, 0.01) && funcRv == 0);

    alt = 39. * 1000.; // PDF value in km
    lla.set(-59. * simCore::DEG2RAD, -8. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -15.75, 0.01) && funcRv == 0);

    alt = 3. * 1000.; // PDF value in km
    lla.set(-50. * simCore::DEG2RAD, -103. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 27.96, 0.01) && funcRv == 0);

    alt = 94. * 1000.; // PDF value in km
    lla.set(-29. * simCore::DEG2RAD, -110. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 15.74, 0.01) && funcRv == 0);

    alt = 66. * 1000.; // PDF value in km
    lla.set(14. * simCore::DEG2RAD, 143. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -0.19, 0.01) && funcRv == 0);

    alt = 18. * 1000.; // PDF value in km
    lla.set(0., 21. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 1.29, 0.01) && funcRv == 0);

    // 2027.5 values
    ordinalDay = 183;
    year = 2027;
    alt = 0.;

    lla.set(-13. * simCore::DEG2RAD, -59. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -17.49, 0.01) && funcRv == 0);

    alt = 8. * 1000.; // PDF value in km
    lla.set(62. * simCore::DEG2RAD, 53. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 19.39, 0.01) && funcRv == 0);

    alt = 77. * 1000.; // PDF value in km
    lla.set(-68. * simCore::DEG2RAD, -7. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -16.19, 0.01) && funcRv == 0);

    alt = 98. * 1000.; // PDF value in km
    lla.set(-5. * simCore::DEG2RAD, 159. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 7.79, 0.01) && funcRv == 0);

    alt = 34. * 1000.; // PDF value in km
    lla.set(-29. * simCore::DEG2RAD, -107. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 15.64, 0.01) && funcRv == 0);

    alt = 60. * 1000.; // PDF value in km
    lla.set(27. * simCore::DEG2RAD, 65. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 1.85, 0.01) && funcRv == 0);

    alt = 73. * 1000.; // PDF value in km
    lla.set(-72. * simCore::DEG2RAD, 95. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -102.64, 0.01) && funcRv == 0);

    alt = 96. * 1000.; // PDF value in km
    lla.set(-46. * simCore::DEG2RAD, -85. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 17.93, 0.01) && funcRv == 0);

    alt = 16. * 1000.; // PDF value in km
    lla.set(66. * simCore::DEG2RAD, -178. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, 0.37, 0.01) && funcRv == 0);

    alt = 72. * 1000.; // PDF value in km
    lla.set(-87. * simCore::DEG2RAD, 38. * simCore::DEG2RAD, alt);
    funcRv = wmm.calculateMagneticVariance(lla, ordinalDay, year, varianceRad);
    rv += SDK_ASSERT(simCore::areAnglesEqual(varianceRad * simCore::RAD2DEG, -65.44, 0.01) && funcRv == 0);

    return rv;
  }

}

int MagneticVarianceTest(int argc, char* argv[])
{
  int rv = 0;

  rv += calculateMagneticVarianceTest();

  std::cout << "MagneticVarianceTest " << ((rv == 0) ? "Passed" : "Failed") << std::endl;

  return rv;
}
