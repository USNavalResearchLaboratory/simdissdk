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
#include <cmath>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Random.h"
#include "simCore/Calc/NumericalAnalysis.h"

namespace {

// If the pole is one of the locations, the angles do not work
int testSodanoDistance(double refLatDeg, double refLonDeg, double latDeg, double lonDeg, double expectedDistance, double t=0.01)
{
  int rv = 0;

  double azfwd;
  double azbck;
  double distance = simCore::sodanoInverse(refLatDeg*simCore::DEG2RAD, refLonDeg*simCore::DEG2RAD, 0.0, latDeg*simCore::DEG2RAD, lonDeg*simCore::DEG2RAD, &azfwd, &azbck);
  rv += SDK_ASSERT(simCore::areEqual(distance, expectedDistance, t));

  // Reverse the order
  double reverseAzbck;
  double reverseAzfwd;
  distance = simCore::sodanoInverse(latDeg*simCore::DEG2RAD, lonDeg*simCore::DEG2RAD, 0.0, refLatDeg*simCore::DEG2RAD, refLonDeg*simCore::DEG2RAD, &reverseAzfwd, &reverseAzbck);
  rv += SDK_ASSERT(simCore::areEqual(distance, expectedDistance, t));

  return rv;
}

// Test going to a location and back. The arguments expectedDistance and t are in meters, everything else in degrees
int testSodanoBothDirections(double refLatDeg, double refLonDeg, double latDeg, double lonDeg, double expectedDistance, double expectedAzFwd, double expectedAzBck, double t=0.01)
{
  int rv = 0;

  double azfwd;
  double azbck;
  double distance = simCore::sodanoInverse(refLatDeg*simCore::DEG2RAD, refLonDeg*simCore::DEG2RAD, 0.0, latDeg*simCore::DEG2RAD, lonDeg*simCore::DEG2RAD, &azfwd, &azbck);
  rv += SDK_ASSERT(simCore::areEqual(distance, expectedDistance, t));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azfwd, expectedAzFwd*simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azbck, expectedAzBck*simCore::DEG2RAD));

  // Reverse the order
  double reverseAzbck;
  double reverseAzfwd;
  distance = simCore::sodanoInverse(latDeg*simCore::DEG2RAD, lonDeg*simCore::DEG2RAD, 0.0, refLatDeg*simCore::DEG2RAD, refLonDeg*simCore::DEG2RAD, &reverseAzfwd, &reverseAzbck);
  rv += SDK_ASSERT(simCore::areEqual(distance, expectedDistance, t));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azfwd, reverseAzbck));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azbck, reverseAzfwd));

  double lat;
  double lon;
  double newAzbck;
  simCore::sodanoDirect(refLatDeg*simCore::DEG2RAD, refLonDeg*simCore::DEG2RAD, 0.0, distance, azfwd, &lat, &lon, &newAzbck);
  rv += SDK_ASSERT(simCore::areAnglesEqual(latDeg*simCore::DEG2RAD, lat));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lonDeg*simCore::DEG2RAD, lon));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azbck, newAzbck));

  // Reverse the order
  simCore::sodanoDirect(latDeg*simCore::DEG2RAD, lonDeg*simCore::DEG2RAD, 0.0, distance, azbck, &lat, &lon, &newAzbck);
  rv += SDK_ASSERT(simCore::areAnglesEqual(refLatDeg*simCore::DEG2RAD, lat));
  rv += SDK_ASSERT(simCore::areAnglesEqual(refLonDeg*simCore::DEG2RAD, lon));
  rv += SDK_ASSERT(simCore::areAnglesEqual(azfwd, newAzbck));

  return rv;
}


// The distance values are from MatLab.  http://mooring.ucsd.edu/software/matlab/doc/ocean/sodano.html
// The angle values are from Matlab using distance routine found in the Mapping Toolkit
int testSodano()
{
  int rv = 0;

  // Do small distances
  // Do some calculations near 0,0
  rv += testSodanoBothDirections(1.0,  1.0,  2.0,  2.0, 1.568761453140806e+005, 45.170470084159682,    2.251966487022478e+02);
  rv += testSodanoBothDirections(-1.0,  1.0, -2.0,  2.0, 1.568761453140806e+005, 1.348295299158403e+02, 3.148033512977522e+02);
  rv += testSodanoBothDirections(-1.0, -1.0, -2.0, -2.0, 1.568761453140806e+005, 2.251704700841594e+02, 45.196648702247479);
  rv += testSodanoBothDirections(1.0, -1.0,  2.0, -2.0, 1.568761453140806e+005, 3.148295299158406e+02, 1.348033512977525e+02);

  // do X
  rv += testSodanoBothDirections(1.0,  1.0, -1.0, -1.0, 3.137991284073622e+005, 2.251967673216324e+02, 45.196767321632407);
  rv += testSodanoBothDirections(-1.0, -1.0,  1.0,  1.0, 3.137991284073622e+005, 45.196767321632407,    2.251967673216324e+02);

  // Do horizontal sides of a box, in both directions
  rv += testSodanoBothDirections(1.0,  1.0,  1.0, -1.0, 2.226052877067270e+005, 2.700174541901544e+02, 89.982545809845632);
  rv += testSodanoBothDirections(1.0, -1.0,  1.0,  1.0, 2.226052877067270e+005, 89.982545809845632,    2.700174541901544e+02);
  rv += testSodanoBothDirections(-1.0,  1.0, -1.0, -1.0, 2.226052877067270e+005, 2.699825458098457e+02, 90.017454190154382);
  rv += testSodanoBothDirections(-1.0, -1.0, -1.0,  1.0, 2.226052877067270e+005, 90.017454190154382,    2.699825458098457e+02);

  // Do vertical sides of a box, in both directions
  rv += testSodanoBothDirections(-1.0,  1.0,  1.0,  1.0, 2.211487770968528e+005,   0, 180);
  rv += testSodanoBothDirections(1.0,  1.0, -1.0,  1.0, 2.211487770968528e+005, 180,   0);
  rv += testSodanoBothDirections(-1.0, -1.0,  1.0, -1.0, 2.211487770968528e+005,   0, 180);
  rv += testSodanoBothDirections(1.0, -1.0, -1.0, -1.0, 2.211487770968528e+005, 180,   0);

  // So some calculations near 180
  rv += testSodanoBothDirections(1.0,  179.0,  1.0,  178.0, 1.113026451444276e+005, 2.700087264261628e+02, 89.991273573837233);
  rv += testSodanoBothDirections(-1.0,  179.0, -1.0,  178.0, 1.113026451444276e+005, 2.699912735738373e+02, 90.008726426162781);
  rv += testSodanoBothDirections(-1.0, -179.0, -1.0, -178.0, 1.113026451444276e+005, 90.008726426162781,    2.699912735738373e+02);
  rv += testSodanoBothDirections(1.0, -179.0,  1.0, -178.0, 1.113026451444276e+005, 89.991273573837233,    2.700087264261628e+02);

  // do X
  rv += testSodanoBothDirections(1.0,  179.0, -1.0, -179.0, 3.137991284073604e+005, 1.348032326783677e+02, 3.148032326783677e+02);
  rv += testSodanoBothDirections(-1.0, -179.0,  1.0,  179.0, 3.137991284073604e+005, 3.148032326783677e+02, 1.348032326783677e+02);

  // Do horizontal sides of a box, in both directions
  rv += testSodanoBothDirections(1.0,  179.0,  1.0, -179.0, 2.226052877067245e+005, 89.982545809845632,    2.700174541901544e+02);
  rv += testSodanoBothDirections(1.0, -179.0,  1.0,  179.0, 2.226052877067245e+005, 2.700174541901544e+02, 89.982545809845632);
  rv += testSodanoBothDirections(-1.0,  179.0, -1.0, -179.0, 2.226052877067245e+005, 90.017454190154382,    2.699825458098457e+02);
  rv += testSodanoBothDirections(-1.0, -179.0, -1.0,  179.0, 2.226052877067245e+005, 2.699825458098457e+02, 90.017454190154382);

  // Do vertical sides of a box, in both directions
  rv += testSodanoBothDirections(-1.0,  179.0,  1.0,  179.0, 2.211487770968528e+005,  0,  180);
  rv += testSodanoBothDirections(1.0,  179.0, -1.0,  179.0, 2.211487770968528e+005, 180,   0);
  rv += testSodanoBothDirections(-1.0, -179.0,  1.0, -179.0, 2.211487770968528e+005,   0, 180);
  rv += testSodanoBothDirections(1.0, -179.0, -1.0, -179.0, 2.211487770968528e+005, 180,   0);

  // Walk up the latitude
  rv += testSodanoBothDirections(0.0, -1.0,  0.0, 1.0, 2.226389731952653e+005, 90, 270);
  rv += testSodanoBothDirections(10.0, -1.0, 10.0, 1.0, 2.192783841406829e+005, 89.826334608130836, 2.701736653918692e+02);
  rv += testSodanoBothDirections(20.0, -1.0, 20.0, 1.0, 2.092929216207915e+005, 89.657949005086920, 2.703420509949131e+02);
  rv += testSodanoBothDirections(30.0, -1.0, 30.0, 1.0, 1.929701037318372e+005, 89.499961727436400, 2.705000382725636e+02);
  rv += testSodanoBothDirections(40.0, -1.0, 40.0, 1.0, 1.707841246752811e+005, 89.357173936166234, 2.706428260638338e+02);
  rv += testSodanoBothDirections(50.0, -1.0, 50.0, 1.0, 1.433872296181857e+005, 89.233923328660893, 2.707660766713391e+02);
  rv += testSodanoBothDirections(60.0, -1.0, 60.0, 1.0, 1.115957494478587e+005, 89.133952575648166, 2.708660474243518e+02);
  rv += testSodanoBothDirections(70.0, -1.0, 70.0, 1.0, 7.636965581654559e+004, 89.060296209355357, 2.709397037906446e+02);
  rv += testSodanoBothDirections(80.0, -1.0, 80.0, 1.0, 3.878505979783758e+004, 89.015189231278910, 2.709848107687211e+02);
  rv += testSodanoBothDirections(89.0, -1.0, 89.0, 1.0, 3.898455437780945e+003, 89.000152273923746, 2.709998477260762e+02);

  // Test the poles; distance only since angles do not work well
  rv += testSodanoDistance(90.0, 0.0,  89.0,  0.0, 1.116938607089692e+005, 10.0);  // 10 meter tolerance
  rv += testSodanoDistance(90.0, 0.0,  89.0,  1.0, 1.116938607089692e+005, 10.0);
  rv += testSodanoDistance(90.0, 0.0,  89.0, -1.0, 1.116938607089692e+005, 10.0);
  rv += testSodanoDistance(-90.0, 0.0, -89.0,  0.0, 1.116938607089692e+005, 10.0);
  rv += testSodanoDistance(-90.0, 0.0, -89.0,  1.0, 1.116938607089692e+005, 10.0);
  rv += testSodanoDistance(-90.0, 0.0, -89.0, -1.0, 1.116938607089692e+005, 10.0);

  // Cross the poles
  rv += testSodanoBothDirections(89.0, 0.0,  89.0,  179.0, 2.233877214179392e+005, 0.500076160049554,     3.594999238399504e+02, 10.0);
  rv += testSodanoBothDirections(89.0, 0.0,  89.0,  180.0, 2.233877214179392e+005, 0.0, 0.0, 10.0);
  rv += testSodanoBothDirections(89.0, 0.0,  89.0, -179.0, 2.233877214179392e+005, 3.594999238399504e+02, 0.500076160049554, 10.0);
  rv += testSodanoBothDirections(-89.0, 0.0, -89.0,  179.0, 2.233877214179392e+005, 1.794999238399504e+02, 1.805000761600496e+02, 10.0);
  rv += testSodanoBothDirections(-89.0, 0.0, -89.0,  180.0, 2.233877214179392e+005, 180.0, 180.0, 10.0);
  rv += testSodanoBothDirections(-89.0, 0.0, -89.0, -179.0, 2.233877214179392e+005, 1.805000761600496e+02, 1.794999238399504e+02, 10.0);

  // Do some large distances
  rv += testSodanoBothDirections(10.0,  10.0,  20.0,  20.0, 1.541856393022642e+006, 42.992954888269502,    2.255972785162924e+02);
  rv += testSodanoBothDirections(-10.0,  10.0, -20.0,  20.0, 1.541856393022642e+006, 1.370070451117305e+02, 3.144027214837075e+02);
  rv += testSodanoBothDirections(-10.0, -10.0, -20.0, -20.0, 1.541856393022642e+006, 2.229929548882695e+02, 45.59727851629239);
  rv += testSodanoBothDirections(10.0, -10.0,  20.0, -20.0, 1.541856393022642e+006, 3.170070451117305e+02, 1.344027214837076e+02);

  // Do some very large distances
  rv += testSodanoBothDirections(10.0,  10.0,  20.0,  120.0, 1.167879865586178e+007, 66.007918979519317,    2.868264820630452e+02);
  rv += testSodanoBothDirections(-10.0,  10.0, -20.0,  120.0, 1.167879865586178e+007, 1.139920810204807e+02, 2.531735179369548e+02);
  rv += testSodanoBothDirections(-10.0, -10.0, -20.0, -120.0, 1.167879865586178e+007, 2.460079189795193e+02, 1.068264820630452e+02);
  rv += testSodanoBothDirections(10.0, -10.0,  20.0, -120.0, 1.167879865586178e+007, 2.939920810204807e+02, 73.173517936954809);

  return rv;
}

int testLinearSearch()
{
  int rv = 0;

  // Test y = x^2 - 3; positive solution
  simCore::LinearSearch ls(50, 1e-010);
  double err = 1.0;
  double minX = 0.5;
  double maxX = 1000.0;
  double x = (maxX + minX) / 2.0;
  simCore::NumericalSearchType type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x - 3;

  rv += SDK_ASSERT(type == simCore::SEARCH_CONVERGED);
  rv += SDK_ASSERT(simCore::areEqual(x, std::pow(3.0, 0.5)));

  // negative solution
  err = 1.0;
  maxX = -0.5;
  minX = -1000.0;
  x = (maxX + minX) / 2.0;
  type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x - 3;

  rv += SDK_ASSERT(type == simCore::SEARCH_CONVERGED);
  rv += SDK_ASSERT(simCore::areEqual(x, -1.0 * std::pow(3.0, 0.5)));

  // test no solution
  err = 1.0;
  maxX =  1.5;
  minX = -1.5;
  x = (maxX + minX) / 2.0;
  type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x - 3;

  rv += SDK_ASSERT(type == simCore::SEARCH_FAILED);

  // Test y = x^3 - 2x^2 -4x + 8, first solution
  err = 1.0;
  maxX =  1000.0;
  minX = 0.1;
  x = (maxX + minX) / 2.0;
  type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x*x - 2*x*x - 4*x + 8;

  rv += SDK_ASSERT(type == simCore::SEARCH_CONVERGED);
  rv += SDK_ASSERT(simCore::areEqual(x, 2.0, 0.000005));

  // Second solution
  err = 1.0;
  maxX =  -0.10;
  minX = -1000.0;
  x = (maxX + minX) / 2.0;
  type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x*x - 2*x*x - 4*x + 8;

  rv += SDK_ASSERT(type == simCore::SEARCH_CONVERGED);
  rv += SDK_ASSERT(simCore::areEqual(x, -2.0, 0.000005));

  // test no solution
  err = 1.0;
  maxX =  1.0;
  minX = -1.0;
  x = (maxX + minX) / 2.0;
  type = simCore::SEARCH_INIT;
  while ((type = ls.searchX(x, err, minX, maxX, 1.0e-7, type)) < simCore::SEARCH_CONVERGED)
    err = x*x*x - 2*x*x - 4*x + 8;

  rv += SDK_ASSERT(type == simCore::SEARCH_FAILED);

  return rv;
}

// Symmetrical around the equator test case
int testGeodeticRangePair(double fromLat, double fromLon, double toLat, double toLong)
{
  int rv = 0;

  simCore::Vec3 fromLla(fromLat, fromLon, 0.0);
  simCore::Vec3 toLla(toLat, toLong, 0.0);
  double downRng1 = 0.0;
  double crossRng1 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, 0.0, toLla, &downRng1, &crossRng1);

  double downRng2 = 0.0;
  double crossRng2 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, 90.0*simCore::DEG2RAD, toLla, &downRng2, &crossRng2);

  double downRng3 = 0.0;
  double crossRng3 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, 180.0*simCore::DEG2RAD, toLla, &downRng3, &crossRng3);

  double downRng4 = 0.0;
  double crossRng4 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, -90.0*simCore::DEG2RAD, toLla, &downRng4, &crossRng4);

  // Check for expected symmetry; not sure why the tolerance needs to be set so high
  rv += SDK_ASSERT(simCore::areEqual(downRng1, -crossRng2, 1.0));
  rv += SDK_ASSERT(simCore::areEqual(downRng1, -downRng3, 1.0));
  rv += SDK_ASSERT(simCore::areEqual(downRng1, crossRng4, 1.0));
  rv += SDK_ASSERT(simCore::areEqual(crossRng1, downRng2, 1.0));
  rv += SDK_ASSERT(simCore::areEqual(crossRng1, -crossRng3, 1.0));
  rv += SDK_ASSERT(simCore::areEqual(crossRng1, -downRng4, 1.0));

  // Sanity check the downrange value by comparing it to the distance to the corner
  // The values will not be exact since the surface is curved, but they will be close
  double distance = simCore::sodanoInverse(fromLla[0], fromLla[1], 0.0, toLla[0], fromLla[1], NULL, NULL);
  rv += SDK_ASSERT(simCore::areEqual(fabs(downRng1), distance, 1.0));

  // Sanity check the cross range value by comparing it to the distance to the corner
  // The values will not be exact since the surface is curved, but they will be close
  distance = simCore::sodanoInverse(fromLla[0], fromLla[1], 0.0, fromLla[0], toLla[1], NULL, NULL);
  rv += SDK_ASSERT(simCore::areEqual(fabs(crossRng1), distance, 1.0));

  // Point directly at the target
  double azfwd = 0;
  double downRngAtTarget;
  double crossRngAtTarget;
  distance = simCore::sodanoInverse(fromLla[0], fromLla[1], 0.0, toLla[0], toLla[1], &azfwd, NULL);
  simCore::calculateGeodesicDRCR(fromLla, azfwd, toLla, &downRngAtTarget, &crossRngAtTarget);
  rv += SDK_ASSERT(simCore::areEqual(distance, downRngAtTarget, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(0.0, crossRngAtTarget, 0.01));

  // Point directly away from the target
  simCore::calculateGeodesicDRCR(fromLla, azfwd+M_PI, toLla, &downRngAtTarget, &crossRngAtTarget);
  rv += SDK_ASSERT(simCore::areEqual(distance, -downRngAtTarget, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(0.0, crossRngAtTarget, 0.01));

  // Point 90 degrees clockwise away from the target
  simCore::calculateGeodesicDRCR(fromLla, azfwd+M_PI_2, toLla, &downRngAtTarget, &crossRngAtTarget);
  rv += SDK_ASSERT(simCore::areEqual(0.0, downRngAtTarget, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(distance, -crossRngAtTarget, 0.01));

  // Point 90 degrees counterclockwise away from the target
  simCore::calculateGeodesicDRCR(fromLla, azfwd-M_PI_2, toLla, &downRngAtTarget, &crossRngAtTarget);
  rv += SDK_ASSERT(simCore::areEqual(0.0, downRngAtTarget, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(distance, crossRngAtTarget, 0.01));

  return rv;
}

// A sanity check since data from a secondary source is not available.
int testGeodeticRanges()
{
  int rv = 0;
  const double delta = 0.1*simCore::DEG2RAD;

  // Test around 0.0
  rv += testGeodeticRangePair(-delta, -delta, delta, delta);
  rv += testGeodeticRangePair(delta, delta, -delta, -delta);
  rv += testGeodeticRangePair(delta, -delta, -delta, delta);


  // Test around 180.0
  rv += testGeodeticRangePair(M_PI-delta, M_PI-delta, M_PI+delta, M_PI+delta);
  rv += testGeodeticRangePair(M_PI+delta, M_PI+delta, M_PI-delta, M_PI-delta);


  // Test that NULL arguments are supported
  simCore::Vec3 fromLla(-delta, -delta, 0.0);
  simCore::Vec3 toLla(delta, delta, 0.0);
  double downRng1 = 0.0;
  double crossRng1 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, 0.0, toLla, &downRng1, &crossRng1);

  double downRng2 = 0.0;
  double crossRng2 = 0.0;
  simCore::calculateGeodesicDRCR(fromLla, 0.0, toLla, &downRng2, NULL);
  simCore::calculateGeodesicDRCR(fromLla, 0.0, toLla, NULL, &crossRng2);

  rv += SDK_ASSERT(simCore::areEqual(downRng1, downRng2));
  rv += SDK_ASSERT(simCore::areEqual(crossRng1, crossRng2));
  // 10b/5l
  {
    simCore::Vec3 fromLla(0.481701, -2.90005, 100.0);
    simCore::Vec3 toLla(0.413779, -2.82214, 99999.7);
    double yaw = -0.816289;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }
  // 10b/5l
  {
    simCore::Vec3 fromLla(0.481701, -2.90005, 100.0);
    simCore::Vec3 toLla(0.41558, -2.82416, 99999.7);
    double yaw = -0.816289;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }
  // 15b/5l
  {
    simCore::Vec3 fromLla(0.481701, -2.90005, 100.0);
    simCore::Vec3 toLla(0.422248, -2.83223, 99999.8);
    double yaw = -0.816289;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }

  // 23b/4l
  {
    simCore::Vec3 fromLla(0.48170099, -2.9000541, 100.0);
    simCore::Vec3 toLla(0.4220893, -2.8320394, 99999.701);
    double yaw = -0.81628855;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }

  // 22b/5l
  {
    simCore::Vec3 fromLla(0.50494353, -2.9193277, 0.0);
    simCore::Vec3 toLla(0.42590118, -2.8347063, 251274.08);
    double yaw = 2.3561945;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }

  // 20b/5l
  {
    simCore::Vec3 fromLla(0.48170120442637893, -2.9000543567447332, 100.0);
    simCore::Vec3   toLla(0.48172436334940283, -2.9000820096345903, 99999.740547421927);
    double yaw = -0.81628679105973367;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_CONVERGED);
  }

  // test failures
  // 1b/5l
  {
    simCore::Vec3 fromLla(0.48170099, -2.9000541, 100.0);
    simCore::Vec3 toLla(-1.07514, 2.27683, 99999.6);
    double yaw = -0.816289;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_FAILED);
  }
  // 1b/5l
  {
    simCore::Vec3 fromLla(0.481701, -2.90005, 100.0);
    simCore::Vec3 toLla(5.20804, 2.27683, 99999.6);
    double yaw = -0.816289;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_FAILED);
  }
  // 1b/5l
  {
    simCore::Vec3 fromLla(0.515814486261627, -2.8949967201533, 100000.0);
    simCore::Vec3   toLla(-0.790056859326086, 1.21029897496941, 100002.430935895);
    double yaw = -0.638914116638234;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_FAILED);
  }
  // 1b/5l
  {
    simCore::Vec3 fromLla(0.515814486261627, -2.8949967201533, 100000.0);
    simCore::Vec3   toLla(5.49505093055421, 1.20625997384174, 100002.396556861);
    double yaw = -0.638914116638234;
    double downRng1 = 0.0;
    double crossRng1 = 0.0;
    simCore::NumericalSearchType status = simCore::calculateGeodesicDRCR(fromLla, yaw, toLla, &downRng1, &crossRng1);
    rv += SDK_ASSERT(status == simCore::SEARCH_FAILED);
  }
  return rv;
}

int testCoordinateConverterReferenceOrigin()
{
  int rv = 0;

  // Test the over-ride flag
  simCore::CoordinateConverter coordConvertor;

  // Verify initial conditions
  rv += SDK_ASSERT(HUGE_VAL == coordConvertor.referenceLat());  // Cannot use simCore::areEqual
  rv += SDK_ASSERT(HUGE_VAL == coordConvertor.referenceLon());  // Cannot use simCore::areEqual
  rv += SDK_ASSERT(simCore::areEqual(0.0, coordConvertor.referenceAlt()));

  // Since first one, it should ignore the flag
  coordConvertor.setReferenceOrigin(0.1, 0.11, 1.11);
  rv += SDK_ASSERT(simCore::areEqual(0.1, coordConvertor.referenceLat()));
  rv += SDK_ASSERT(simCore::areEqual(0.11, coordConvertor.referenceLon()));
  rv += SDK_ASSERT(simCore::areEqual(1.11, coordConvertor.referenceAlt()));

  // Since the flag is true, it should accept the update
  coordConvertor.setReferenceOrigin(0.2, 0.22, 2.22);
  rv += SDK_ASSERT(simCore::areEqual(0.2, coordConvertor.referenceLat()));
  rv += SDK_ASSERT(simCore::areEqual(0.22, coordConvertor.referenceLon()));
  rv += SDK_ASSERT(simCore::areEqual(2.22, coordConvertor.referenceAlt()));

  // Use debugger to verify optimization worked
  coordConvertor.setReferenceOrigin(0.2, 0.22, 2.22);

  return rv;
}


int testClosingVelocity()
{
  int rv = 0;
  double delta = 0.0000001;
  // A rough approximation of the opposite ends of a cube.
  simCore::Vec3 fromLla(0.0, 0.0, 0.0);
  simCore::Vec3 toLla(delta, delta, simCore::WGS_A * delta);
  simCore::EarthModelCalculations model = simCore::WGS_84;
  simCore::CoordinateConverter coordConv;
  coordConv.setReferenceOrigin(0.0, 0.0, 0.0);
  simCore::Vec3 fromVel(-1.0, -1.0, -1.0);
  simCore::Vec3 toVel(1.0, 1.0, 1.0);

  double velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  double expectedVelocity = -2.0 * pow(3.0, 0.5);  // Approximate since this is flat surface value
  rv += SDK_ASSERT(simCore::areEqual(velocity, expectedVelocity, 2e-5));

  // swap directions
  fromVel.set(1.0, 1.0, 1.0);
  toVel.set(-1.0, -1.0, -1.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  rv += SDK_ASSERT(simCore::areEqual(velocity, -expectedVelocity, 2e-5));

  // Make one stationary
  fromVel.set(0.0, 0.0, 0.0);
  toVel.set(1.0, 1.0, 1.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  expectedVelocity = -pow(3.0, 0.5);
  rv += SDK_ASSERT(simCore::areEqual(velocity, expectedVelocity, 1e-4));

  // Change direction
  toVel.set(-1.0, -1.0, -1.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  rv += SDK_ASSERT(simCore::areEqual(velocity, -expectedVelocity, 1e-4));

  // Do each component, X
  toVel.set(1.0, 0.0, 0.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  expectedVelocity = -pow(0.3333333333333, 0.5);
  rv += SDK_ASSERT(simCore::areEqual(velocity, expectedVelocity, 1e-2));
  // Y
  toVel.set(0.0, 1.0, 0.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  expectedVelocity = -pow(0.3333333333333, 0.5);
  rv += SDK_ASSERT(simCore::areEqual(velocity, expectedVelocity, 1e-2));
  // Z
  toVel.set(0.0, 0.0, 1.0);
  velocity = simCore::calculateClosingVelocity(fromLla, toLla, model, &coordConv, fromVel, toVel);
  expectedVelocity = -pow(0.3333333333333, 0.5);
  rv += SDK_ASSERT(simCore::areEqual(velocity, expectedVelocity, 1e-2));

  return rv;
}

int testV3Angle()
{
  int rv = 0;
  // Values verified by MatLab

  // Test Zero vector
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, 0.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 1.0), simCore::Vec3(0.0, 0.0, 0.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 0.0, 0.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.0));

  // Test aligned vectors
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(1.0, 0.0, 0.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 0.0), simCore::Vec3(0.0, 1.0, 0.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 0.0, 1.0), simCore::Vec3(0.0, 0.0, 1.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 0.0), simCore::Vec3(1.0, 1.0, 0.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 1.0), simCore::Vec3(1.0, 0.0, 1.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 1.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(2.0, 2.0, 2.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.0));

  // Test opposite align vectors
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(-1.0, 0.0, 0.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 0.0), simCore::Vec3(0.0, -1.0, 0.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 0.0, 1.0), simCore::Vec3(0.0, 0.0, -1.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 0.0), simCore::Vec3(-1.0, -1.0, 0.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 1.0), simCore::Vec3(-1.0, 0.0, -1.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 1.0), simCore::Vec3(-1.0, -1.0, -1.0)), M_PI));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(2.0, 2.0, 2.0), simCore::Vec3(-1.0, -1.0, -1.0)), M_PI));

  // Test perpendicular axis
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(0.0, 1.0, 0.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, 1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 0.0), simCore::Vec3(0.0, 0.0, 1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(0.0, -1.0, 0.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, -1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 0.0), simCore::Vec3(0.0, 0.0, -1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(-1.0, 0.0, 0.0), simCore::Vec3(0.0, 1.0, 0.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(-1.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, 1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, -1.0, 0.0), simCore::Vec3(0.0, 0.0, 1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(-1.0, 0.0, 0.0), simCore::Vec3(0.0, -1.0, 0.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(-1.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, -1.0)), M_PI_2));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, -1.0, 0.0), simCore::Vec3(0.0, 0.0, -1.0)), M_PI_2));

  // Test various angles
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(1.0, 1.0, 0.0)), M_PI_4));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(-1.0, 1.0, 0.0)), M_PI_2+M_PI_4));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(-1.0, -1.0, 0.0)), M_PI_2+M_PI_4));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(1.0, -1.0, 0.0)), M_PI_4));

  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 0.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.955316618124509));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 0.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.955316618124509));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 0.0, 1.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.955316618124509));

  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 1.0, 0.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.615479708670387));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(1.0, 0.0, 1.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.615479708670387));
  rv += SDK_ASSERT(simCore::areEqual(simCore::v3Angle(simCore::Vec3(0.0, 1.0, 1.0), simCore::Vec3(1.0, 1.0, 1.0)), 0.615479708670387));

  return rv;
}

int testInverseCosine()
{
  int rv = 0;

  // test typical values
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(0.0), acos(0.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(0.5), acos(0.5)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(-0.5), acos(-0.5)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(1.0), acos(1.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(-1.0), acos(-1.0)));

  // test accumulated error values
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(1.000001), acos(1.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseCosine(-1.000001), acos(-1.0)));

  return rv;
}

int testInverseSine()
{
  int rv = 0;

  // test typical values
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(0.0), asin(0.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(0.5), asin(0.5)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(-0.5), asin(-0.5)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(1.0), asin(1.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(-1.0), asin(-1.0)));

  // test accumulated error values
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(1.000001), asin(1.0)));
  rv += SDK_ASSERT(simCore::areEqual(simCore::inverseSine(-1.000001), asin(-1.0)));

  return rv;
}

// Test in both directions, lat and lon in degrees, the rest in meters
int validateGeodeticEcef(double lat, double lon, double alt, double x, double y, double z)
{
  int rv = 0;

  simCore::Coordinate lla(simCore::COORD_SYS_LLA, simCore::Vec3(lat*simCore::DEG2RAD, lon*simCore::DEG2RAD, alt));
  simCore::Coordinate ecefResults;

  simCore::CoordinateConverter::convertGeodeticToEcef(lla, ecefResults);

  rv += SDK_ASSERT(simCore::areEqual(ecefResults.x(), x, 0.01));  // Within a cm
  rv += SDK_ASSERT(simCore::areEqual(ecefResults.y(), y, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(ecefResults.z(), z, 0.01));

  simCore::Coordinate ecef(simCore::COORD_SYS_ECEF, simCore::Vec3(x, y, z));
  simCore::Coordinate llaResults;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecef, llaResults);

  rv += SDK_ASSERT(simCore::areAnglesEqual(llaResults.lat(), lat*simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(llaResults.lon(), lon*simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(llaResults.alt(), alt, 0.01));

  return rv;
}

// Test a point in the first octet in all the other octets
int validateGeodeticEcefOctet(double lat, double lon, double alt, double x, double y, double z)
{
  int rv = 0;

  // Do the North
  rv += SDK_ASSERT(validateGeodeticEcef(lat, lon,       alt,  x,  y, z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(lat, lon+90.0,  alt, -y,  x, z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(lat, lon+180.0, alt, -x, -y, z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(lat, lon+270.0, alt,  y, -x, z) == 0);

  // Do the South
  rv += SDK_ASSERT(validateGeodeticEcef(-lat, lon,       alt,  x,  y, -z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(-lat, lon+90.0,  alt, -y,  x, -z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(-lat, lon+180.0, alt, -x, -y, -z) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(-lat, lon+270.0, alt,  y, -x, -z) == 0);

  return rv;
}

// Values validated by MatLab Mapping Toolbox using geodetic2ecef
int testGeodeticEcef()
{
  int rv = 0;

  // Four Corners of the world
  rv += SDK_ASSERT(validateGeodeticEcef(0.0,   0.0, 0.0,  6378137,      0.0, 0.0) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(0.0,  90.0, 0.0,      0.0,  6378137, 0.0) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(0.0, 180.0, 0.0, -6378137,      0.0, 0.0) == 0);
  rv += SDK_ASSERT(validateGeodeticEcef(0.0, 270.0, 0.0,      0.0, -6378137, 0.0) == 0);

  // North Pole
  rv += SDK_ASSERT(validateGeodeticEcef(90.0, 0.0, 0.0, 0.0, 0.0,   6.356752314245179e+06) == 0);
  // South Pole
  rv += SDK_ASSERT(validateGeodeticEcef(-90.0, 0.0, 0.0, 0.0, 0.0,  -6.356752314245179e+06) == 0);

  // Some random values tested in each octet
  rv += SDK_ASSERT(validateGeodeticEcefOctet(10.0,  10.0,    10.0, 6.186446764493323e+06, 1.090837479296760e+06, 1.100250284217138e+06) == 0);
  rv += SDK_ASSERT(validateGeodeticEcefOctet(13.0,  27.0, 12345.0, 5.548963434433493e+06, 2.827338088157875e+06, 1.428182229588835e+06) == 0);
  rv += SDK_ASSERT(validateGeodeticEcefOctet(51.0,  41.0, 54321.0, 3.061265055830413e+06, 2.661117113973628e+06, 4.975759965997723e+06) == 0);

  return rv;
}

// Test in both directions, lat and lon in degrees, the rest in meters
int validateXEastEcef(double xEcef, double yEcef, double zEcef, double lat, double lon, double alt, double xXEast, double yXEast, double zXEast)
{
  int rv = 0;

  simCore::CoordinateConverter coordConvertor;
  coordConvertor.setReferenceOriginDegrees(lat, lon, alt);

  simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(xEcef, yEcef, zEcef));
  simCore::Coordinate tpCoordResults;

  coordConvertor.convert(ecefCoord, tpCoordResults, simCore::COORD_SYS_XEAST);

  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.x(), xXEast, 0.01));  // Within one cm
  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.y(), yXEast, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.z(), zXEast, 0.01));

  simCore::Coordinate ecefCoordResults;
  simCore::Coordinate tpCoord(simCore::COORD_SYS_XEAST, simCore::Vec3(xXEast, yXEast, zXEast));

  coordConvertor.convert(tpCoord, ecefCoordResults, simCore::COORD_SYS_ECEF);

  rv += SDK_ASSERT(simCore::areEqual(ecefCoordResults.x(), xEcef, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(ecefCoordResults.y(), yEcef, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(ecefCoordResults.z(), zEcef, 0.01));

  return rv;
}

// Test a point in the first octet in all the other octets
int validateXEastEcefOctet(double xEcef, double yEcef, double zEcef, double lat, double lon, double alt, double xXEast, double yXEast, double zXEast)
{
  int rv = 0;

  // Do the North
  rv += validateXEastEcef(xEcef,  yEcef, zEcef, lat, lon,       alt, xXEast, yXEast, zXEast);
  rv += validateXEastEcef(-yEcef,  xEcef, zEcef, lat, lon+90.0,  alt, xXEast, yXEast, zXEast);
  rv += validateXEastEcef(-xEcef, -yEcef, zEcef, lat, lon+180.0, alt, xXEast, yXEast, zXEast);
  rv += validateXEastEcef(yEcef, -xEcef, zEcef, lat, lon+270.0, alt, xXEast, yXEast, zXEast);

  // Do the South
  rv += validateXEastEcef(xEcef,  yEcef, -zEcef, -lat, lon,       alt, xXEast, -yXEast, zXEast);
  rv += validateXEastEcef(-yEcef,  xEcef, -zEcef, -lat, lon+90.0,  alt, xXEast, -yXEast, zXEast);
  rv += validateXEastEcef(-xEcef, -yEcef, -zEcef, -lat, lon+180.0, alt, xXEast, -yXEast, zXEast);
  rv += validateXEastEcef(yEcef, -xEcef, -zEcef, -lat, lon+270.0, alt, xXEast, -yXEast, zXEast);

  return rv;
}

// Values validated by MatLab Mapping Toolbox using ecef2enu
int testXEastEcef()
{
  int rv = 0;

  // Four Corners of the world
  rv += validateXEastEcefOctet(6378137.0,    0.0,    0.0, 0.0, 0.0, 0.0,    0.0,    0.0,    0.0);
  rv += validateXEastEcefOctet(6378147.0,   10.0,   10.0, 0.0, 0.0, 0.0,   10.0,   10.0,   10.0);
  rv += validateXEastEcefOctet(6378237.0,  100.0,  100.0, 0.0, 0.0, 0.0,  100.0,  100.0,  100.0);
  rv += validateXEastEcefOctet(6379137.0, 1000.0, 1000.0, 0.0, 0.0, 0.0, 1000.0, 1000.0, 1000.0);

  // North Pole
  rv += validateXEastEcef(0.0,    0.0,  6356752.314245179,  90.0, 0.0, 0.0,    0.0,    0.0,     0.0);
  // South Pole
  rv += validateXEastEcef(0.0,    0.0, -6356752.314245179, -90.0, 0.0, 0.0,    0.0,    0.0,     0.0);
  // Around the poles
  rv += validateXEastEcefOctet(10.0,  10.0,  6356752.314245179,  90.0, 0.0, 0.0,   10.0,  -10.0,     0.0);
  rv += validateXEastEcefOctet(10.0, 100.0,  6356752.314245179,  90.0, 0.0, 0.0,  100.0,  -10.0,     0.0);

  // Near Pole
  rv += validateXEastEcefOctet(1.116881943557355e+05,   0.0,  6.355777626639486e+06, 89.0, 0.0, 0.0,    0.0,    0.0,     0.0);
  rv += validateXEastEcefOctet(1.116800000000000e+05,   0.0,  6.355700000000000e+06, 89.0, 0.0, 0.0,    0.0, 6.838336032185653,    -77.757827799881355);
  rv += validateXEastEcefOctet(1.116900000000000e+05,  10.0,  6.355800000000000e+06, 89.0, 0.0, 0.0,   10.0, -1.414900275649909,   22.401465780130604);

  // Typical value
  rv += validateXEastEcefOctet(3.061265055830413e+06, 2.661117113973628e+06, 4.975759965997723e+06, 51.0, 41.0, 54321.0, 0.0, 0.0, 0.0);
  rv += validateXEastEcefOctet(3.061200000000000e+06, 2.661100000000000e+06, 4.975700000000000e+06, 51.0, 41.0, 54321.0, 29.764385078137590, 9.144309629627152,  -84.566737132171426);
  rv += validateXEastEcefOctet(3.061200000000000e+06, 2.661100000000000e+06, 4.975700000000000e+06, 51.0, 41.0,     0.0, 29.764385078280611, 9.144309629529744,    5.423643326286730e+04);
  rv += validateXEastEcefOctet(3.061200000000000e+06, 2.661100000000000e+06, 4.975700000000000e+06, 51.0, 40.0,     0.0, 7.081943681147846e+04, 4.896471388384562e+02,     5.384732974386888e+04);

  return rv;
}

// Test in both directions, lat, lon, latRef, lonRef in degrees, the rest in meters
int validateXEastGeodetic(double lat, double lon, double alt, double latRef, double lonRef, double altRef, double xXEast, double yXEast, double zXEast)
{
  int rv = 0;

  simCore::CoordinateConverter coordConvertor;
  coordConvertor.setReferenceOriginDegrees(latRef, lonRef, altRef);

  simCore::Coordinate geodeticCoord(simCore::COORD_SYS_LLA, simCore::Vec3(lat*simCore::DEG2RAD, lon*simCore::DEG2RAD, alt));
  simCore::Coordinate tpCoordResults;

  coordConvertor.convert(geodeticCoord, tpCoordResults, simCore::COORD_SYS_XEAST);

  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.x(), xXEast, 0.01));  // Within one cm
  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.y(), yXEast, 0.01));
  rv += SDK_ASSERT(simCore::areEqual(tpCoordResults.z(), zXEast, 0.01));

  simCore::Coordinate geodeticCoordResults;
  simCore::Coordinate tpCoord(simCore::COORD_SYS_XEAST, simCore::Vec3(xXEast, yXEast, zXEast));

  coordConvertor.convert(tpCoord, geodeticCoordResults, simCore::COORD_SYS_LLA);

  rv += SDK_ASSERT(simCore::areAnglesEqual(geodeticCoordResults.x(), lat*simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(geodeticCoordResults.y(), lon*simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(geodeticCoordResults.z(), alt, 0.01));

  return rv;
}

// Test a point in the first octet in all the other octets
int validateXEastGeodeticOctet(double lat, double lon, double alt, double latRef, double lonRef, double altRef, double xXEast, double yXEast, double zXEast)
{
  int rv = 0;

  // Do the North
  rv += validateXEastGeodetic(lat,  lon,       alt,  latRef, lonRef,     altRef, xXEast, yXEast, zXEast);
  rv += validateXEastGeodetic(lat,  lon+90.0,  alt,  latRef, lonRef+90,  altRef, xXEast, yXEast, zXEast);
  rv += validateXEastGeodetic(lat,  lon+180.0, alt,  latRef, lonRef+180, altRef, xXEast, yXEast, zXEast);
  rv += validateXEastGeodetic(lat,  lon+270.0, alt,  latRef, lonRef+270, altRef, xXEast, yXEast, zXEast);

  // Do the South
  rv += validateXEastGeodetic(-lat,  lon,       alt, -latRef, lonRef,     altRef, xXEast, -yXEast, zXEast);
  rv += validateXEastGeodetic(-lat,  lon+90.0,  alt, -latRef, lonRef+90,  altRef, xXEast, -yXEast, zXEast);
  rv += validateXEastGeodetic(-lat,  lon+180.0, alt, -latRef, lonRef+180, altRef, xXEast, -yXEast, zXEast);
  rv += validateXEastGeodetic(-lat,  lon+270.0, alt, -latRef, lonRef+270, altRef, xXEast, -yXEast, zXEast);

  return rv;
}

// Values validated by MatLab Mapping Toolbox using geodetic2enu
int testXEastGeodetic()
{
  int rv = 0;

  // Four Corners of the world
  rv += validateXEastGeodeticOctet(0.0, 0.0,    0.0, 0.0, 0.0,   0.0,  0.0, 0.0, 0.0);
  rv += validateXEastGeodeticOctet(1.0, 0.0,    0.0, 0.0, 0.0,   0.0,  0.0, 1.105687748245666e+05, -9.649195715897885e+02);
  rv += validateXEastGeodeticOctet(0.0, 1.0,    0.0, 0.0, 0.0,   0.0,  1.113138392366761e+05, 0.0, -9.714211583000571e+02);
  rv += validateXEastGeodeticOctet(0.0, 0.0, 1000.0, 0.0, 0.0,   0.0,  0.0, 0.0, 1000.0);
  rv += validateXEastGeodeticOctet(1.0, 1.0, 1000.0, 0.0, 0.0,   0.0,  1.113144488164847e+05, 1.105862272310039e+05, -9.364983544556235e+02);
  rv += validateXEastGeodeticOctet(1.0, 1.0, 1000.0, 0.5, 0.5, 500.0,  5.565934374550242e+04, 5.529738070206125e+04, 15.875287915777108);

  //// North Pole
  rv += validateXEastGeodeticOctet(90.0, 0.0, 0.0, 90.0, 0.0, 0.0,  0.0, 0.0, 0.0);
  rv += validateXEastGeodeticOctet(89.9, 0.0, 0.0, 90.0, 0.0, 0.0,  0.0, -1.116939217060576e+04, -9.747135865154089);
  rv += validateXEastGeodeticOctet(89.9, 0.5, 0.0, 90.0, 0.0, 0.0,  97.470097272051007, -1.116896687437683e+04, -9.747135865154089);
  //// South Pole
  rv += validateXEastGeodeticOctet(-90.0, 0.0, 0.0, -90.0, 0.0, 0.0,  0.0, 0.0, 0.0);
  rv += validateXEastGeodeticOctet(-89.9, 0.0, 0.0, -90.0, 0.0, 0.0,  0.0, 1.116939217060576e+04, -9.747135865154089);
  rv += validateXEastGeodeticOctet(-89.9, 0.5, 0.0, -90.0, 0.0, 0.0, 97.470097272051007, 1.116896687437683e+04, -9.747135865154089);

  //// Near the Poles
  rv += validateXEastGeodeticOctet(89.0, 1.5, 0.0, 88.5, 1.0, 0.0,  9.746509927946082e+02,  5.585025998853827e+04, -2.437872080481182e+02);
  rv += validateXEastGeodeticOctet(88.0, 1.5, 0.0, 88.5, 1.0, 0.0,  1.948999098673805e+03, -5.583724961423888e+04, -2.438969972875477e+02);
  rv += validateXEastGeodeticOctet(88.0, 0.5, 0.0, 88.5, 1.0, 0.0, -1.948999098673804e+03, -5.583724961423886e+04, -2.438969972875482e+02);
  rv += validateXEastGeodeticOctet(89.0, 0.5, 0.0, 88.5, 1.0, 0.0, -9.746509927946084e+02,  5.585025998853825e+04, -2.437872080481177e+02);

  //// Typical value
  rv += validateXEastGeodeticOctet(22.5, 44.5, 0.0, 22.0, 45.0, 0.0, -5.144747182328344e+04,  5.545022357894816e+04, -4.497216817110384e+02);
  rv += validateXEastGeodeticOctet(21.5, 44.5, 0.0, 22.0, 45.0, 0.0, -5.180944668085703e+04, -5.527807278851989e+04, -4.511664727142634e+02);
  rv += validateXEastGeodeticOctet(21.5, 45.5, 0.0, 22.0, 45.0, 0.0,  5.180944668085703e+04, -5.527807278851989e+04, -4.511664727142634e+02);
  rv += validateXEastGeodeticOctet(22.5, 45.5, 0.0, 22.0, 45.0, 0.0,  5.144747182328344e+04,  5.545022357894816e+04, -4.497216817110384e+02);

  return rv;
}

int testCalculateGeodeticOriFromRelOri()
{
  int rv = 0;

  // Test data generated from NumPy, transformations.py and testRotation.py
  // .py files are checked into Scripts

  //Input 1: 30 15 0
  //Input 2: 30 10 0
  //Output: (yaw, pitch, roll)
  //62.3013830099
  //22.8593242282
  //8.07327530779
  simCore::Vec3 hostYpr(simCore::DEG2RAD*30, simCore::DEG2RAD*15, 0);
  simCore::Vec3 relYpr(simCore::DEG2RAD*30, simCore::DEG2RAD*10, 0);
  simCore::Vec3 ans(62.3013830099*simCore::DEG2RAD, 22.8593242282*simCore::DEG2RAD, 8.07327530779*simCore::DEG2RAD);
  simCore::Vec3 ypr;
  simCore::calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr);
  rv += (simCore::areAnglesEqual(ypr.yaw(), ans.yaw()) &&
    simCore::areAnglesEqual(ypr.pitch(), ans.pitch()) &&
    simCore::areAnglesEqual(ypr.roll(), ans.roll())) ? 0 : 1;
  SDK_ASSERT(rv == 0);

  //Input 1: 350 -10 -3
  //Input 2: 0 0 0
  //Output: (yaw, pitch, roll)
  //-10.0
  //-10.0
  //-3.0
  hostYpr.set(simCore::DEG2RAD*350, simCore::DEG2RAD*-10, simCore::DEG2RAD*-3);
  relYpr.set(0, 0, 0);
  ans.set(-10*simCore::DEG2RAD, -10*simCore::DEG2RAD, -3*simCore::DEG2RAD);
  simCore::calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr);
  rv += (simCore::areAnglesEqual(ypr.yaw(), ans.yaw()) &&
    simCore::areAnglesEqual(ypr.pitch(), ans.pitch()) &&
    simCore::areAnglesEqual(ypr.roll(), ans.roll())) ? 0 : 1;
  SDK_ASSERT(rv == 0);

  //Input 1: 180 0 0
  //Input 2: 0 0 10
  //Output: (yaw, pitch, roll)
  //180.0
  //8.8278125961e-32
  //10.0
  hostYpr.set(simCore::DEG2RAD*180, 0, 0);
  relYpr.set(0, 0, simCore::DEG2RAD*10);
  ans.set(180*simCore::DEG2RAD, 8.8278125961e-32*simCore::DEG2RAD, 10*simCore::DEG2RAD);
  simCore::calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr);
  rv += (simCore::areAnglesEqual(ypr.yaw(), ans.yaw()) &&
    simCore::areAnglesEqual(ypr.pitch(), ans.pitch()) &&
    simCore::areAnglesEqual(ypr.roll(), ans.roll())) ? 0 : 1;
  SDK_ASSERT(rv == 0);

  //Input 1: 0 0 0
  //Input 2: 0 0 0
  //Output: (yaw, pitch, roll)
  //0.0
  //-0.0
  //0.0
  hostYpr.set(0, 0, 0);
  relYpr.set(0, 0, 0);
  ans.set(0, -0, 0);
  simCore::calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr);
  rv += (simCore::areAnglesEqual(ypr.yaw(), ans.yaw()) &&
    simCore::areAnglesEqual(ypr.pitch(), ans.pitch()) &&
    simCore::areAnglesEqual(ypr.roll(), ans.roll())) ? 0 : 1;
  SDK_ASSERT(rv == 0);

  //Input 1: 1 2 3
  //Input 2: 4 5 6
  //Output: (yaw, pitch, roll)
  //5.27121717114
  //6.77845756226
  //9.15260354518
  hostYpr.set(simCore::DEG2RAD*1, simCore::DEG2RAD*2, simCore::DEG2RAD*3);
  relYpr.set(simCore::DEG2RAD*4, simCore::DEG2RAD*5, simCore::DEG2RAD*6);
  ans.set(5.27121717114*simCore::DEG2RAD, 6.77845756226*simCore::DEG2RAD, 9.15260354518*simCore::DEG2RAD);
  simCore::calculateGeodeticOriFromRelOri(hostYpr, relYpr, ypr);
  rv += (simCore::areAnglesEqual(ypr.yaw(), ans.yaw()) &&
    simCore::areAnglesEqual(ypr.pitch(), ans.pitch()) &&
    simCore::areAnglesEqual(ypr.roll(), ans.roll())) ? 0 : 1;
  SDK_ASSERT(rv == 0);

  if (rv != 0)
    std::cout << "testCalculateGeodeticOriFromRelOri fails\n";
  else
    std::cout << "testCalculateGeodeticOriFromRelOri passes\n";

  return rv;
}

int testRotateEulerAngle()
{
  int rv = 0;

  const double RAD15 = 15 * simCore::DEG2RAD;
  const double RAD30 = 30 * simCore::DEG2RAD;
  const double RAD180 = 180 * simCore::DEG2RAD;

  simCore::Vec3 hostYPR;
  simCore::Vec3 bodyAzEl;
  simCore::Vec3 trueAzElCalculated;
  simCore::Vec3 bodyAzElCalculated;
  double bodyAz = 0.0;
  double bodyEl = 0.0;
  {
    // Simple rotation left and right, starting from 0,0,0
    hostYPR.set(0, 0, 0);

    bodyAzEl.set(RAD15, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(RAD30, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD30));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(-RAD15, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), -RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(-RAD30, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), -RAD30));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));
  }
  {
    // Simple rotation left and right, starting from -180,0,0
    hostYPR.set(RAD180, 0, 0);

    bodyAzEl.set(RAD15, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD180 + RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(RAD30, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD180 + RAD30));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(-RAD15, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD180 - RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(-RAD30, 0, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD180 - RAD30));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 0));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));
  }
  {
    // Simple rotation up and down
    hostYPR.set(RAD15, 0, 0);

    bodyAzEl.set(0, RAD15, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), RAD15));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(0, RAD30, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), RAD30));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(0, -RAD15, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), -RAD15));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));

    bodyAzEl.set(0, -RAD30, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), RAD15));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), -RAD30));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));
  }

  {
    // Complex example from the Doxygen docs
    hostYPR.set(15 * simCore::DEG2RAD, 5 * simCore::DEG2RAD, -90 * simCore::DEG2RAD);
    bodyAzEl.set(0, 15 * simCore::DEG2RAD, 0);
    trueAzElCalculated = simCore::rotateEulerAngle(hostYPR, bodyAzEl);
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.x(), -0.055 * simCore::DEG2RAD, 1e-2));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.y(), 4.82922 * simCore::DEG2RAD, 1e-2));
    rv += SDK_ASSERT(simCore::areAnglesEqual(trueAzElCalculated.z(), -91.2972 * simCore::DEG2RAD, 1e-2));
    simCore::calculateRelAngToTrueAzEl(trueAzElCalculated.x(), trueAzElCalculated.y(), hostYPR, &bodyAz, &bodyEl, NULL);
    bodyAzElCalculated.set(bodyAz, bodyEl, 0.0);
    rv += SDK_ASSERT(simCore::v3AreAnglesEqual(bodyAzElCalculated, bodyAzEl));
  }
  return rv;
}

bool areLlaEqual(const simCore::Vec3& lla1, const simCore::Vec3& lla2)
{
  if (!simCore::areAnglesEqual(lla1.lat(), lla2.lat()))
    return false;

  if (!simCore::areAnglesEqual(lla1.lon(), lla2.lon()))
    return false;

  if (!simCore::areEqual(lla1.alt(), lla2.alt(), 0.01))
    return false;

  return true;
}

int testGetClosestPoint()
{
  int rv = 0;

  double distance;
  // Pick random start and end points
  simCore::Vec3 startLla(0.1 * simCore::DEG2RAD, 0.2 * simCore::DEG2RAD, 100.0);
  simCore::Vec3 endLla(0.3 * simCore::DEG2RAD, 0.4 * simCore::DEG2RAD, 200.0);
  simCore::Vec3 toLlA;
  simCore::Vec3 closestLla;

  // toLla on the line segment at the start
  toLlA.set(startLla);
  distance = getClosestPoint(startLla, endLla, toLlA, closestLla);
  rv += SDK_ASSERT(simCore::areEqual(distance, 0.0));
  rv += SDK_ASSERT(areLlaEqual(toLlA, closestLla));

  // toLla on the line segment at the end
  toLlA.set(endLla);
  distance = getClosestPoint(startLla, endLla, toLlA, closestLla);
  rv += SDK_ASSERT(simCore::areEqual(distance, 0.0));
  rv += SDK_ASSERT(areLlaEqual(toLlA, closestLla));

  // toLla on the line segment at the middle; results as of 9/30/2014
  toLlA.set((startLla.lat() + endLla.lat())/2.0, (startLla.lon() + endLla.lon())/2.0, (startLla.alt() + endLla.alt())/2.0);
  distance = getClosestPoint(startLla, endLla, toLlA, closestLla);
  rv += SDK_ASSERT(simCore::areEqual(distance, 19.364560518361056));
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(0.0034906734785969773, 0.0052359866171920941, 130.63567324914038), closestLla));

  // toLla on one corner; results as of 9/30/2014
  toLlA.set(startLla.lat(), endLla.lon(), (startLla.alt() + endLla.alt())/2.0);
  distance = getClosestPoint(startLla, endLla, toLlA, closestLla);
  rv += SDK_ASSERT(simCore::areEqual(distance, 15690.393002825369));
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(0.0035023838473604674, 0.0052476969686070362, 130.97202189546078), closestLla));

  // toLla on the other corner; results as of 9/30/2014
  toLlA.set(endLla.lat(), startLla.lon(), (startLla.alt() + endLla.alt())/2.0);
  distance = getClosestPoint(startLla, endLla, toLlA, closestLla);
  rv += SDK_ASSERT(simCore::areEqual(distance, 15690.202271225024));
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(0.0034789654358837285, 0.0052242785932742983, 130.30113441031426), closestLla));

  return rv;
}

int testCalculateGeodeticOffsetPos()
{
  int rv = 0;

  // test comparison values are from 9/27/2017 execution

  simCore::Vec3 originLLA(simCore::DEG2RAD*22, simCore::DEG2RAD*-160, 9);
  simCore::Vec3 offsetLLA;

  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, 0, 0), simCore::Vec3(10, 0, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839740119, -2.7925268032, 9.0006338218)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, 0, 0), simCore::Vec3(0, 10, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839724357, -2.7925284934, 9.0006337678)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, 0, 0), simCore::Vec3(0, 0, 10), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839724357, -2.7925268032, 19.0006259140)));

  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(simCore::DEG2RAD*45, 0, 0), simCore::Vec3(10, 0, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839735502, -2.7925256080, 9.0006337967)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, simCore::DEG2RAD*45, 0), simCore::Vec3(10, 0, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839735502, -2.7925268032, 16.0716976793)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, 0, simCore::DEG2RAD*45), simCore::Vec3(10, 0, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839740119, -2.7925268032, 9.0006338218)));

  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(simCore::DEG2RAD*45, 0, 0), simCore::Vec3(0, 10, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839735502, -2.7925279983, 9.0006337967)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, simCore::DEG2RAD*45, 0), simCore::Vec3(0, 10, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839724357, -2.7925284934, 9.0006337678)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(0, 0, simCore::DEG2RAD*45), simCore::Vec3(0, 10, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839724357, -2.7925279983, 16.0716976495)));

  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(simCore::DEG2RAD*77, simCore::DEG2RAD*-5, simCore::DEG2RAD*60), simCore::Vec3(10, 0, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839727889, -2.7925251626, 8.1290762862)));
  simCore::calculateGeodeticOffsetPos(originLLA, simCore::Vec3(simCore::DEG2RAD*77, simCore::DEG2RAD*-5, 0), simCore::Vec3(0, 10, 0), offsetLLA);
  rv += SDK_ASSERT(areLlaEqual(offsetLLA, simCore::Vec3(0.3839739715, -2.7925271834, 9.0006338200)));

  return rv;
}

int testCalculateGeodeticEndPoint()
{
  int rv = 0;

  // test comparison values are from 9/27/2017 execution

  simCore::Vec3 originLLA(simCore::DEG2RAD*22, simCore::DEG2RAD*-160, 9);
  simCore::Vec3 endPtLLA;

  simCore::calculateGeodeticEndPoint(originLLA, 0, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839740119, -2.7925268032, 9.0006338218)));
  simCore::calculateGeodeticEndPoint(originLLA, 0, 0, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.5402134989, -2.7925268032, 78305.0330870207)));

  simCore::calculateGeodeticEndPoint(originLLA, 45*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839735502, -2.7925256080, 9.0006337967)));
  simCore::calculateGeodeticEndPoint(originLLA, 45*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.4535335040, -2.7143754139, 742384.0044007823)));
  simCore::calculateGeodeticEndPoint(originLLA, 45*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.4704367810, -2.6942941087, -663075.6404297417)));

  simCore::calculateGeodeticEndPoint(originLLA, 90*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839724357, -2.7925251130, 9.0006337678)));
  simCore::calculateGeodeticEndPoint(originLLA, 90*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3819674117, -2.6853470850, 742298.1740604648)));
  simCore::calculateGeodeticEndPoint(originLLA, 90*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3808529921, -2.6589189645, -663207.0165526336)));

  simCore::calculateGeodeticEndPoint(originLLA, 135*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839713211, -2.7925256080, 9.0006337846)));
  simCore::calculateGeodeticEndPoint(originLLA, 135*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3123658956, -2.7186903787, 742394.3779521575)));
  simCore::calculateGeodeticEndPoint(originLLA, 135*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.2943057462, -2.7010093780, -663055.5087756803)));

  simCore::calculateGeodeticEndPoint(originLLA, 180*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839708595, -2.7925268032, 9.0006338041)));
  simCore::calculateGeodeticEndPoint(originLLA, 180*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.2839999972, -2.7925268032, 742487.5565864388)));
  simCore::calculateGeodeticEndPoint(originLLA, 180*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.2591349749, -2.7925268032, -662909.8589776233)));

  simCore::calculateGeodeticEndPoint(originLLA, 225*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839713211, -2.7925279983, 9.0006337846)));
  simCore::calculateGeodeticEndPoint(originLLA, 225*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3123658956, -2.8663632277, 742394.3779521566)));
  simCore::calculateGeodeticEndPoint(originLLA, 225*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.2943057462, -2.8840442284, -663055.5087756803)));

  simCore::calculateGeodeticEndPoint(originLLA, 270*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839724357, -2.7925284934, 9.0006337678)));
  simCore::calculateGeodeticEndPoint(originLLA, 270*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3819674117, -2.8997065214, 742298.1740604648)));
  simCore::calculateGeodeticEndPoint(originLLA, 270*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3808529921, -2.9261346419, -663207.0165526336)));

  simCore::calculateGeodeticEndPoint(originLLA, 315*simCore::DEG2RAD, 0, 10, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839735502, -2.7925279983, 9.0006337967)));
  simCore::calculateGeodeticEndPoint(originLLA, 315*simCore::DEG2RAD, 45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.4535335040, -2.8706781925, 742384.0044007823)));
  simCore::calculateGeodeticEndPoint(originLLA, 315*simCore::DEG2RAD, -45*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.4704367810, -2.8907594977, -663075.6404297426)));

  simCore::calculateGeodeticEndPoint(originLLA, 0, 90*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839724354, -2.7925268032, 1000009.0000245674)));
  simCore::calculateGeodeticEndPoint(originLLA, 0, -90*simCore::DEG2RAD, 1000000, endPtLLA);
  rv += SDK_ASSERT(areLlaEqual(endPtLLA, simCore::Vec3(0.3839724375, -2.7925268032, -999990.9956273828)));

  return rv;
}

int testCalculateVelOriFromPos(const simCore::Vec3& llaStart, const simCore::Vec3& llaEnd, simCore::CoordinateSystem inputSystem, simCore::CoordinateSystem outputSystem)
{
  // Figure out the truth values for velocity and FPA based on LLA start/end
  const double TIME_DELTA = 10.0; // seconds between data points
  simCore::Vec3 llaVelocity;
  simCore::calculateVelFromGeodeticPos(llaEnd, llaStart, TIME_DELTA, llaVelocity);
  simCore::Vec3 enuFpa;
  simCore::calculateFlightPathAngles(llaVelocity, enuFpa);

  // Need a CC centered on the start
  simCore::CoordinateConverter cc;
  cc.setReferenceOrigin(llaStart);

  // Get the input coordinates in the input system
  simCore::Coordinate inputStart;
  cc.convert(simCore::Coordinate(simCore::COORD_SYS_LLA, llaStart), inputStart, inputSystem);
  simCore::Coordinate inputEnd;
  cc.convert(simCore::Coordinate(simCore::COORD_SYS_LLA, llaEnd), inputEnd, inputSystem);

  int rv = 0;

  // Call the method to get the values
  simCore::Vec3 outVel;
  simCore::Vec3 outOri;
  rv += SDK_ASSERT(simCore::calculateVelOriFromPos(inputEnd.position(), inputStart.position(), TIME_DELTA, inputSystem, outVel, outOri, llaStart, outputSystem));

  // Get the output end position, in the output system
  simCore::Coordinate outputEnd;
  cc.convert(simCore::Coordinate(simCore::COORD_SYS_LLA, llaEnd), outputEnd, outputSystem);

  // Attach it to the inputEnd, and convert to LLA to compare to our values
  outputEnd.setVelocity(outVel);
  outputEnd.setOrientation(outOri);
  simCore::Coordinate outputLla;
  cc.convert(outputEnd, outputLla, simCore::COORD_SYS_LLA);
  // Now the values should match
  rv += SDK_ASSERT(simCore::v3AreEqual(outputLla.velocity(), llaVelocity, 1e-2));
  rv += SDK_ASSERT(simCore::v3AreAnglesEqual(outputLla.orientation(), enuFpa, 1e-3));
  return rv;
}

std::string toString(simCore::CoordinateSystem cs)
{
  switch (cs)
  {
  case simCore::COORD_SYS_NONE:
    return "None";
  case simCore::COORD_SYS_NED:
    return "NED";
  case simCore::COORD_SYS_NWU:
    return "NWU";
  case simCore::COORD_SYS_ENU:
    return "ENU";
  case simCore::COORD_SYS_LLA:
    return "LLA";
  case simCore::COORD_SYS_ECEF:
    return "ECEF";
  case simCore::COORD_SYS_ECI:
    return "ECI";
  case simCore::COORD_SYS_XEAST:
    return "XEAST";
  case simCore::COORD_SYS_GTP:
    return "GTP";
  case simCore::COORD_SYS_MAX:
    return "MAX";
  }
  return "";
}

int testCalculateVelOriFromPos()
{
  // First we're going to generate 3 points.  One at center, one northeast at 45 degrees, and
  // one southwest at XX degrees.  They'll be our truth points.

  // Pick a random center coordinate to use as a basis for our other coordinates
  const simCore::Coordinate llaCenter(simCore::COORD_SYS_LLA, simCore::Vec3(0.3, 0.1, 0.0));
  simCore::Vec3 v3;

  // Set up a coordinate converter so that we can get a few other nearby positions
  simCore::CoordinateConverter cc;
  cc.setReferenceOrigin(llaCenter.position());

  // Pick a point that is 45 degrees in heading from the center, make it face northeast (100m from center)
  const double sinFortyFive = sin(45 * simCore::DEG2RAD);
  const simCore::Coordinate xeTopRight(simCore::COORD_SYS_XEAST, simCore::Vec3(sinFortyFive * 100, sinFortyFive * 100, 0.0));
  // Calculate that same position in LLA and ECEF
  simCore::Coordinate llaTopRight;
  cc.convert(xeTopRight, llaTopRight, simCore::COORD_SYS_LLA);

  // Pick a point at 210 degrees with a range of 100
  const double twoTen = 210 * simCore::DEG2RAD;
  const double sinTwoTen = sin(twoTen);
  const double cosTwoTen = cos(twoTen);
  const simCore::Coordinate xeBottomLeft(simCore::COORD_SYS_XEAST, simCore::Vec3(sinTwoTen * 100, cosTwoTen * 100, 0.0));
  // Calculate that same position in LLA and ECEF
  simCore::Coordinate llaBottomLeft;
  cc.convert(xeBottomLeft, llaBottomLeft, simCore::COORD_SYS_LLA);

  // At this point we have all the truth data we need.  We want to test northeast and southwest, the following combinations:
  //   LLA -> LLA
  //   LLA -> XEAST
  //   LLA -> ECEF
  //   XEAST -> LLA
  //   XEAST -> XEAST
  //   XEAST -> ECEF
  //   ECEF -> LLA
  //   ECEF -> XEAST
  //   ECEF -> ECEF


  // Create a list of the supported coordinate systems (not ECI or GTP)
  std::vector<simCore::CoordinateSystem> coords;
  coords.push_back(simCore::COORD_SYS_NED);
  coords.push_back(simCore::COORD_SYS_NWU);
  coords.push_back(simCore::COORD_SYS_ENU);
  coords.push_back(simCore::COORD_SYS_LLA);
  coords.push_back(simCore::COORD_SYS_ECEF);
  coords.push_back(simCore::COORD_SYS_XEAST);

  // Loop through each set of coords
  int rv = 0;
  for (std::vector<simCore::CoordinateSystem>::const_iterator i1 = coords.begin(); i1 != coords.end(); ++i1)
  {
    for (std::vector<simCore::CoordinateSystem>::const_iterator i2 = coords.begin(); i2 != coords.end(); ++i2)
    {
      int rv2 = testCalculateVelOriFromPos(llaCenter.position(), llaTopRight.position(), *i1, *i2);
      if (rv2 != 0)
      {
        std::cerr << "Failed test 1: " << toString(*i1) << " to " << toString(*i2) << "\n";
        rv++;
      }
      int rv3 = testCalculateVelOriFromPos(llaCenter.position(), llaBottomLeft.position(), *i1, *i2);
      if (rv3 != 0)
      {
        std::cerr << "Failed test 2: " << toString(*i1) << " to " << toString(*i2) << "\n";
        rv++;
      }
    }
  }

  // Make sure that a time of 0.0 doesn't divide-by-zero
  simCore::Vec3 vel;
  simCore::Vec3 ori;
  rv += SDK_ASSERT(simCore::calculateVelOriFromPos(llaTopRight.position(), llaCenter.position(), 10.0, simCore::COORD_SYS_LLA, vel, ori, llaCenter.position(), simCore::COORD_SYS_LLA));
  rv += SDK_ASSERT(simCore::calculateVelOriFromPos(llaTopRight.position(), llaCenter.position(), 0.0, simCore::COORD_SYS_LLA, vel, ori, llaCenter.position(), simCore::COORD_SYS_LLA));

  return rv;
}

simCore::Vec3 toRadians(simCore::Vec3 lla)
{
  lla.setLat(lla.lat() * simCore::DEG2RAD);
  lla.setLon(lla.lon() * simCore::DEG2RAD);
  return lla;
}
simCore::Vec3 yprToRadians(simCore::Vec3 ypr)
{
  ypr.setYaw(ypr.yaw() * simCore::DEG2RAD);
  ypr.setPitch(ypr.pitch() * simCore::DEG2RAD);
  ypr.setRoll(ypr.roll() * simCore::DEG2RAD);
  return ypr;
}

int testMidPointLowRes()
{
  int rv = 0;
  bool wraps = false;
  simCore::Vec3 midpoint;

  // Simple 0,0 case
  simCore::calculateGeodeticMidPoint(simCore::Vec3(0,0,0), simCore::Vec3(0,0,0), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(0,0,0), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Different altitudes
  simCore::calculateGeodeticMidPoint(simCore::Vec3(0,0,-15), simCore::Vec3(0,0,5), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(0,0,-5), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Same lat/lon, but not 0,0
  simCore::calculateGeodeticMidPoint(simCore::Vec3(-1,1,3), simCore::Vec3(-1,1,3), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(simCore::Vec3(-1,1,3), midpoint));
  rv += SDK_ASSERT(wraps == false);

  // SW to NE near equator
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(-2, -2, 0)), toRadians(simCore::Vec3(4, 4, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(1,1,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Reverse latitudes, should be the same; NW to SE
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(4, -2, 0)), toRadians(simCore::Vec3(-2, 4, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(1,1,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);

  // Don't cross dateline, but go across the whole globe
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(-20, -176, 0)), toRadians(simCore::Vec3(40, 178, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(10,1,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Don't cross dateline, but go across the whole globe as far as possible
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(-20, -180, 0)), toRadians(simCore::Vec3(40, 180, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(10,0,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);

  // Short crossing of dateline
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(-20, 178, 0)), toRadians(simCore::Vec3(-24, -176, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(-22,-179,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);
  // Long dateline crossing -- 330 degrees wide
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(50, -50, 0)), toRadians(simCore::Vec3(56, -80, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(53,115,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);

  // Test from <-180; clear intent here is -179 as the middle
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(50, -186, 0)), toRadians(simCore::Vec3(56, -172, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(53,-179,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);
  // Test from <-180 with a result <-180
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(50, -186, 0)), toRadians(simCore::Vec3(56, -178, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(53,-182,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);

  // Test with >+180; clear intent here is -179 as the middle
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(50, 176, 0)), toRadians(simCore::Vec3(56, 182, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(53,179,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);
  // Test with >+180 with a result >+180
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(50, 178, 0)), toRadians(simCore::Vec3(-90, 186, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(-20,182,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);

  // Test case from review on code: JFK to SIN
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(40.640, -73.779, 0)), toRadians(simCore::Vec3(1.359,103.989, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(20.9995,15.105,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Reversed ordering
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(1.359,103.989, 0)), toRadians(simCore::Vec3(40.640, -73.779, 0)), false, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(20.9995,-164.895,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);

  return rv;
}

int testMidPointHighRes()
{
  int rv = 0;
  bool wraps = false;
  simCore::Vec3 midpoint;

  // Test case from review on code: JFK to SIN
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(40.640, -73.779, 0)), toRadians(simCore::Vec3(1.359,103.989, 0)), true, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(70.34117458722292, 97.02347775257729,0)), midpoint));
  rv += SDK_ASSERT(wraps == false);
  // Reversed ordering
  simCore::calculateGeodeticMidPoint(toRadians(simCore::Vec3(1.359,103.989, 0)), toRadians(simCore::Vec3(40.640, -73.779, 0)), true, midpoint, &wraps);
  rv += SDK_ASSERT(areLlaEqual(toRadians(simCore::Vec3(70.34117458722292, 97.02347775257729,0)), midpoint));
  rv += SDK_ASSERT(wraps == true);

  return rv;
}

// Completely arbitrary test of randomness, intended to make sure same
// value is not returned X times in a row.  While this could happen in
// a random set, the likelihood should be so low that it's not worth
// considering.
int testIsRandom(simCore::RandomVariable& random)
{
  double firstValue = random();
  for (int k = 0; k < 100; ++k)
  {
    if (random() != firstValue)
      return 0;
  }
  return 1;
}

int testIsRandom(simCore::DiscreteRandomVariable& random)
{
  int firstValue = random();
  for (int k = 0; k < 100; ++k)
  {
    if (random() != firstValue)
      return 0;
  }
  return 1;
}

int testRandom()
{
  int rv = 0;
  simCore::NormalVariable v1;
  rv += SDK_ASSERT(testIsRandom(v1) == 0);
  simCore::ExponentialVariable v2;
  rv += SDK_ASSERT(testIsRandom(v2) == 0);
  simCore::PoissonVariable v3;
  rv += SDK_ASSERT(testIsRandom(v3) == 0);
  v3.setMean(2.0);
  rv += SDK_ASSERT(testIsRandom(v3) == 0);
  simCore::GeometricVariable v4;
  rv += SDK_ASSERT(testIsRandom(v4) == 0);
  simCore::BinomialVariable v5;
  rv += SDK_ASSERT(testIsRandom(v5) == 0);
  return rv;
}


int testTaos_intercept()
{
  int rv = 0;

  simCore::EarthModelCalculations model = simCore::WGS_84;
  simCore::CoordinateConverter coordConv;
  coordConv.setReferenceOriginDegrees(0.1, 0.1, 10.0);

  simCore::Vec3 fromLla, toLla, fromOriLla, toOriLla;
  double az, el, cmp, aa, s, v;

  // from data\TestData\UpdateData\Taos_intercept.asi
  // time 30.5
  fromLla = simCore::Vec3(0.09999997, 0.100033, 13.45353708);
  fromOriLla = simCore::Vec3(90.00175867, 74.71705511, 0.00205688);
  toLla = simCore::Vec3(0., 0.1365198, 28064.53543379);
  toOriLla = simCore::Vec3(270., - 57.28952036, 0.0000025);
  simCore::calculateRelAzEl(toRadians(fromLla), yprToRadians(fromOriLla), toRadians(toLla), &az, &el, &cmp, model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(az * simCore::RAD2DEG, 21.550 , 1e-03));
  rv += SDK_ASSERT(simCore::areEqual(el * simCore::RAD2DEG, 6.520, 1e-03));
  aa = simCore::calculateAspectAngle(toRadians(fromLla), toRadians(toLla), yprToRadians(toOriLla));
  rv += SDK_ASSERT(simCore::areEqual(aa * simCore::RAD2DEG, 32.011, 1e-03));
  s = simCore::calculateSlant(toRadians(fromLla), toRadians(toLla), model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(s, 30434.2302335016, 1.5e-02));
  v = simCore::calculateClosingVelocity(toRadians(fromLla), toRadians(toLla), model, &coordConv, simCore::Vec3(14.90399335, - 0.00071756, 54.54880707), simCore::Vec3(-783.3883867, 0., - 1219.77459698));
  rv += SDK_ASSERT(simCore::areEqual(-v, -1281.5041259559, 2.5e-03));

  // time 37.0
  fromLla = simCore::Vec3(0.0999999, 0.1076603, 2568.066505);
  fromOriLla = simCore::Vec3(89.99832735, 70.82268784, -0.00172971);
  toLla = simCore::Vec3(0., 0.09115336, 19973.65088355);
  toOriLla = simCore::Vec3(270., - 58.55615129, 0.0000025);
  simCore::calculateRelAzEl(toRadians(fromLla), yprToRadians(fromOriLla), toRadians(toLla), &az, &el, &cmp, model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(az * simCore::RAD2DEG, 35.028, 1.5e-03));
  rv += SDK_ASSERT(simCore::areEqual(el * simCore::RAD2DEG, 21.096, 1e-03));
  aa = simCore::calculateAspectAngle(toRadians(fromLla), toRadians(toLla), yprToRadians(toOriLla));
  rv += SDK_ASSERT(simCore::areEqual(aa * simCore::RAD2DEG, 47.854, 1e-03));
  s = simCore::calculateSlant(toRadians(fromLla), toRadians(toLla), model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(s, 20713.402648426403, 1.5e-02));
  v = simCore::calculateClosingVelocity(toRadians(fromLla), toRadians(toLla), model, &coordConv, simCore::Vec3(248.37812524, 0.00020858, 714.14911139), simCore::Vec3(-774.22680037, 0., - 1266.18988254));
  rv += SDK_ASSERT(simCore::areEqual(-v, -1573.5390923747, 3e-03));

  // time 41.5
  fromLla = simCore::Vec3(0.11110779, 0.12372155, 5993.1720121);
  fromOriLla = simCore::Vec3(24.85137373, 33.97610179, -9.4387819);
  toLla = simCore::Vec3(0., 0.06027507, 14242.53942117);
  toOriLla = simCore::Vec3(270., - 59.39726959, 0.0000025);
  simCore::calculateRelAzEl(toRadians(fromLla), yprToRadians(fromOriLla), toRadians(toLla), &az, &el, &cmp, model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(az * simCore::RAD2DEG, 170.567, 1.5e-03));
  rv += SDK_ASSERT(simCore::areEqual(el * simCore::RAD2DEG, 63.874, 1e-03));
  aa = simCore::calculateAspectAngle(toRadians(fromLla), toRadians(toLla), yprToRadians(toOriLla));
  rv += SDK_ASSERT(simCore::areEqual(aa * simCore::RAD2DEG, 77.636, 1e-03));
  s = simCore::calculateSlant(toRadians(fromLla), toRadians(toLla), model, &coordConv);
  rv += SDK_ASSERT(simCore::areEqual(s, 16416.8749333886, 1.5e-02));
  v = simCore::calculateClosingVelocity(toRadians(fromLla), toRadians(toLla), model, &coordConv, simCore::Vec3(467.50701324, 597.67212996, 760.2732746), simCore::Vec3(-754.68396112, 0., - 1275.98640469)); 
  rv += SDK_ASSERT(simCore::areEqual(-v, -48.8624863969, 3e-03));

  return rv;
}

int testAoaSideslipTotalAoa()
{
  int rv = 0;

  double testParams[24][9] =  { 
    // Test data generated by the TAOS application.  NOTE: "Expected SS" values are multiplied by -1 since SIMDIS looks at the angle from the opposite perspective
    // Yaw (rad), Pitch (rad), Roll (rad), EastVel (any), NorthVel (any), UpVel (any), Expected AOA (rad), Expected SS (rad), Expected TotalAOA (rad)
    {     1.5708,     1.10174,         -0,        821.82,           0.00,     2361.06,          -0.134094,               0.0,      0.134094},
    {     1.5708,     1.11942,          0,       1586.49,           0.00,     3415.72,         -0.0165457,               0.0,     0.0165457},
    {  -0.610534,     1.17927,    1.23123,       -177.65,         253.79,      750.82,                0.0,       0.000226893,   0.000226893},
    {   -0.61104,     1.17183,    2.55434,       -234.34,         334.76,      969.44,               -0.0,       0.000122173,   0.000122173},
    {  -0.613221,     1.06195,  -0.967192,       -718.09,        1019.66,     2236.20,       -0.000191986,              -0.0,   0.000191986}, // 5
    {  -0.613954,     1.04884,   -1.68339,       -797.25,        1131.13,     2406.03,                0.0,               0.0,           0.0},
    {  -0.610499,     1.19477,   0.208253,        -97.99,         139.93,      434.25,        -0.00118682,       0.000401426,    0.00125664},
    {  -0.609713,     1.18536,   0.393799,       -119.45,         170.64,      519.99,         -0.0039619,        0.00205949,    0.00445059},
    // Test values taken from UtilsRestricted::testCalculateAngleOfAttack.
    // NOTE: Existing data only contained values for TotalAOA, so AOA and SS values are filled in based on results here
    // Yaw (rad), Pitch (rad), Roll (rad), EastVel (any), NorthVel (any),  UpVel (any), Expected AOA (rad), Expected SS (rad), Expected TotalAOA (rad)
    {          0,           0,          0,             0,              0,            0,                  0,                 0,            0}, // Should return all zeroes, since no Velocity
    {    0.49037,     3.76566,     5.4944,             0,              0,            0,                  0,                 0,            0}, // 10
    {          0,           0,          0,   16644.39016,    29208.15583, -28846.88083,           0.709169,         -0.517958,      0.85083},
    {    0.49037,     3.76566,     5.4944,   16644.39016,    29208.15583, -28846.88083,          -0.734134,          -1.89301,     1.808114},
    {    0.49037,     3.76566,     5.4944,             1,              1,           -1,          -0.504567,          -1.91646,     1.871923},
    {    3.14159,     3.14159,    3.14159,             1,              1,           -1,           0.615483,         -0.785401,      0.95532},
    {          1,           1,          1,             1,              1,           -1,           0.744443,          -1.64536,     1.625611}, // 15
    {   10.84689,     -9.9035,   -0.86838,             1,              1,           -1,          -0.156555,          -1.24436,     1.248493},
    {   10.84689,     -9.9035,   -0.86838,             0,              0,           -1,          -0.610694,          -2.16793,     2.049518},
    {   10.84689,     -9.9035,   -0.86838,             1,              0,           -1,          -0.766407,          -1.14895,     1.271375},
    {   10.84689,     -9.9035,   -0.86838,             0,              1,           -1,          0.0972667,          -1.80684,     1.805702},
    {   10.84689,     -9.9035,   -0.86838,             1,              1,            0,           0.216204,         -0.751484,     0.776078}, // 20
    {   10.84689,     -9.9035,   -0.86838,             0,              1,            0,             0.7906,          -1.38277,      1.43893},
    {   -5.60301,    10.17163,   11.52466,   43166.64503,    15583.04935, -7141.456008,           0.094936,          -2.11559,     2.112865},
    {  -10.49650,     2.62059,    -6.6306,   13531.13069,    13212.05930, -19061.39684,          -0.791297,           2.43112,     2.132763},
    {   -0.40684,   -11.84026,   -9.24658,    6985.30190,    37348.59157, -15209.51840,          -0.754879,           1.10146,     1.235094}  // 24
  };

  const double tolerance = 0.01 * simCore::DEG2RAD;
  size_t testCaseCount = sizeof(testParams) / sizeof(testParams[0]);
  for (size_t i = 0; i < testCaseCount; ++i)
  {
    double* val = testParams[i];
    const simCore::Vec3 yprVec((*val), (*(val + 1)), (*(val + 2)));
    const simCore::Vec3 enuVec(*(val + 3), *(val + 4), *(val + 5));
    double aoa;
    double ss;
    double totalAoa;
    simCore::calculateAoaSideslipTotalAoa(enuVec, yprVec, true, &aoa, &ss, &totalAoa);
    SDK_ASSERT(simCore::areAnglesEqual(aoa, *(val + 6), tolerance));
    SDK_ASSERT(simCore::areAnglesEqual(ss, *(val + 7), tolerance));
    SDK_ASSERT(simCore::areAnglesEqual(totalAoa, *(val + 8), tolerance));
  }

  return rv;
}

int testBoresightAlphaBeta()
{
  int rv = 0;                  // Test Data uses X-East coordinates in meters; Yaw, Pitch, and Roll as well as expected Azimuth, Elevation, and Composite are in degrees
                               // Before passed into calculateRelAzEl(), X-East coordinates are converted to LLA (decimal degrees) and degrees are converted into radians
                               //{    FromX,     FromY,    FromZ,     Yaw,    Pitch,    Roll,      ToX,      ToY,     ToZ,   ExpAzim,  ExpElev, ExpComAng}
  double paramsTest[15][12] =  { {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,      0.0,      0.0,     0.0,      0.00,     0.00,     0.0}, // Changing Lat & Long
                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,      0.0,   1000.0,     0.0,      0.00,     0.00,     0.0},
                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,   1000.0,   1000.0,     0.0,     45.00,     0.00,   45.00},
                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,   1000.0,      0.0,     0.0,     90.00,     0.00,   90.00},
                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,   1000.0,  -1000.0,     0.0,    135.00,     0.00,  135.00}, // #5

                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,      0.0,   1000.0,  1000.0,      0.00,    45.00,   45.00}, // Changing Alt
                                 {      0.0,       0.0,      0.0,    0.00,     0.00,    0.00,   1000.0,   1000.0,  1000.0,     45.00,    35.26,   54.73},
                                 {      0.0,       0.0,   1000.0,    0.00,     0.00,    0.00,   1000.0,      0.0,     0.0,     90.00,   -45.00,   90.00},
                                 {      0.0,    1000.0,   1000.0,    0.00,     0.00,    0.00,   1000.0,  -2000.0,     0.0,    161.56,   -17.55,  154.75},
                                 {  -3000.0,    4000.0,   3000.0,    0.00,     0.00,    0.00,  -6000.0,  -1000.0,     0.0,   -149.01,   -27.24,  139.65}, // #10

                                 {      0.0,       0.0,      0.0,    90.00,    0.00,    0.00,  -1000.0,  -1000.0,     0.0,    135.00,     0.00,  135.00}, // Changing YPR
                                 {      0.0,       0.0,      0.0,    45.00,   45.00,    0.00,      0.0,  -1000.0,  1000.0,     73.67,    58.60,   81.57},
                                 {   1000.0,   -3000.0,   5000.0,    20.00,   20.00,  -80.00,   1000.0,   1000.0,  2000.0,    -59.67,     7.27,   59.94},
                                 {   2000.0,    6000.0,  20000.0,  -120.00,  -30.00,    0.00,   1000.0,   1000.0,     0.0,    -16.47,   -49.30,   51.30},
                                 {      0.0,   -4000.0,      0.0,   160.00,   45.00,   90.00,  -3000.0,   2000.0,  5000.0,    -98.15,     5.25,   98.11}  // #15
  };

  const double tolerance = 0.01 * simCore::DEG2RAD;
  simCore::CoordinateConverter cc;
  cc.setReferenceOrigin();

  size_t testCaseCount = sizeof(paramsTest) / sizeof(paramsTest[0]);
  for (size_t i = 0; i < testCaseCount; i++)
  {
    const double* row = paramsTest[i];

    const simCore::Vec3 fromXEast(row[0], row[1], row[2]);
    simCore::Coordinate fromLla;
    cc.convert(simCore::Coordinate(simCore::COORD_SYS_XEAST, fromXEast), fromLla, simCore::COORD_SYS_LLA);

    simCore::Vec3 fromOri(row[3], row[4], row[5]);
    fromOri.scale(simCore::DEG2RAD);

    const simCore::Vec3 toXEast(row[6], row[7], row[8]);
    simCore::Coordinate toLla;
    cc.convert(simCore::Coordinate(simCore::COORD_SYS_XEAST, toXEast), toLla, simCore::COORD_SYS_LLA);

    double azim;
    double elev;
    double cmp;

    const double azimExpected = row[9] * simCore::DEG2RAD;
    const double elevExpected = row[10] * simCore::DEG2RAD;
    const double cmpExpected = row[11] * simCore::DEG2RAD;

    simCore::calculateRelAzEl(fromLla.position(), fromOri, toLla.position(), &azim, &elev, &cmp, simCore::WGS_84, NULL); // Coord Converter optional for WGS_84 models

    rv += SDK_ASSERT(simCore::areAnglesEqual(azim, azimExpected, tolerance));
    rv += SDK_ASSERT(simCore::areAnglesEqual(elev, elevExpected, tolerance));
    rv += SDK_ASSERT(simCore::areAnglesEqual(cmp, cmpExpected, tolerance));
  }

  return rv;
}

int testTangentPlane2Sphere()
{
  int rv = 0;

  // with trivial tangent plane offset(ht only), at any lla point, spherical and wgs84 are trivially comparable
  for (double lat : { 1., 10., 60.})
  {
    for (double lon : {3., 5., 13., 27., 53., 90.})
    {
      for (double alt : {30., 500., 1300., 2700., 5300., 9000.})
      {
        simCore::Vec3 refLla(lat, lon, alt);
        simCore::Vec3 tpSphereXYZ;
        simCore::geodeticToSpherical(refLla.lat(), refLla.lon(), refLla.alt(), tpSphereXYZ);

        for (double z : { 10., 100., 1000., 10000., 100000.})
        {
          simCore::Vec3 sphereXYZ;
          simCore::Vec3 tpVec(0., 0., z);
          simCore::tangentPlane2Sphere(refLla, tpVec, sphereXYZ, &tpSphereXYZ);
          const double altAboveSphere = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;
          rv += SDK_ASSERT(simCore::areEqual(z + refLla.alt(), altAboveSphere));
        }
      }
    }
  }

  simCore::Vec3 refLla(0., 0., 0.);
  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refLla.lat(), refLla.lon(), refLla.alt(), tpSphereXYZ);
  // with x,y offsets, things get interesting
  for (double z : { 0., 10., 100., 1000., 10000., 100000.})
  {
    simCore::Vec3 sphereXYZ;
    simCore::Vec3 tpVec(10000., 0., z);
    simCore::tangentPlane2Sphere(refLla, tpVec, sphereXYZ, &tpSphereXYZ);
    const double altAboveSphere = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;

    // this approximates the ht offset between the spherical earth and the ellipsoidal earth at a point dropped to the earth from the point on the tangent plane.
    const double sphereToEllipsoidOffset = altAboveSphere - (refLla.alt() + z);
    // the spherical earth is always bigger than wgs84 ellipsoid, except that they are the same size at the equator
    rv += SDK_ASSERT(sphereToEllipsoidOffset >= 0);
  }

  // verify that spherical height is constant with respect to x/y distance from an arbitrary reflla
  refLla = simCore::Vec3(10., 20., 50.);
  simCore::geodeticToSpherical(refLla.lat(), refLla.lon(), refLla.alt(), tpSphereXYZ);
  for (double i : { 10., 100., 1000., 10000., 100000.})
  {
    simCore::Vec3 sphereXYZ;

    simCore::Vec3 t1(i, 100., 100.);
    simCore::tangentPlane2Sphere(refLla, t1, sphereXYZ, &tpSphereXYZ);
    const double alt1 = v3Length(sphereXYZ);

    simCore::Vec3 t2(100., i, 100.);
    simCore::tangentPlane2Sphere(refLla, t2, sphereXYZ, &tpSphereXYZ);
    const double alt2 = v3Length(sphereXYZ);
    rv += SDK_ASSERT(simCore::areEqual(alt1, alt2));
  }
  return rv;
}

}

int CalculationTest(int argc, char* argv[])
{
  int rv = 0;

  rv += testSodano();
  rv += testLinearSearch();
  rv += testGeodeticRanges();
  rv += testCoordinateConverterReferenceOrigin();
  rv += testClosingVelocity();
  rv += testV3Angle();
  rv += testInverseCosine();
  rv += testInverseSine();
  rv += testGeodeticEcef();
  rv += testXEastEcef();
  rv += testXEastGeodetic();
  rv += testCalculateGeodeticOriFromRelOri();
  rv += testRotateEulerAngle();
  rv += testGetClosestPoint();
  rv += testCalculateGeodeticOffsetPos();
  rv += testCalculateGeodeticEndPoint();
  rv += testCalculateVelOriFromPos();
  rv += testMidPointLowRes();
  rv += testMidPointHighRes();
  rv += testRandom();
  rv += testTaos_intercept();
  rv += testAoaSideslipTotalAoa();
  rv += testBoresightAlphaBeta();
  rv += testTangentPlane2Sphere();
  return rv;
}
