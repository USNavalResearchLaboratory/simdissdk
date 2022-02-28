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
