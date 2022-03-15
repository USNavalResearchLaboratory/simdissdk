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
#include <iomanip>

#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"

namespace {

static const unsigned int PRECISION = 16;

static bool vectorsAreEqual(const double* v, const double* u, const size_t size, const double epsilon=1e-6)
{
  for (size_t i = 0; i < size; ++i)
  {
    if (!simCore::areEqual(v[i], u[i], epsilon))
      return false;
  }
  return true;
}


//===========================================================================
int testRint()
{
  std::cerr << "Testing simCore::rint ================================================= " << std::endl;
  int rv = 0;

  rv += SDK_ASSERT(simCore::rint(-1.7) == -2.);
  rv += SDK_ASSERT(simCore::rint(-1.5) == -2.);
  rv += SDK_ASSERT(simCore::rint(-0.5) == 0.);
  rv += SDK_ASSERT(simCore::rint(-0.2) == 0.);
  rv += SDK_ASSERT(simCore::rint(0.) == 0.);
  rv += SDK_ASSERT(simCore::rint(0.5) == 0.);
  rv += SDK_ASSERT(simCore::rint(1.5) == 2.);
  rv += SDK_ASSERT(simCore::rint(1.7) == 2.);
  rv += SDK_ASSERT(simCore::rint(2.5) == 2.);

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;
  return rv;
}

int testRound()
{
  std::cerr << "Testing simCore::round ================================================ " << std::endl;
  int rv = 0;

  rv += SDK_ASSERT(simCore::round(0.) == 0.);
  rv += SDK_ASSERT(simCore::round(0.1) == 0.);
  rv += SDK_ASSERT(simCore::round(-0.1) == 0.);
  rv += SDK_ASSERT(simCore::round(0.5) == 1.);
  rv += SDK_ASSERT(simCore::round(-0.5) == -1.);
  rv += SDK_ASSERT(simCore::round(1.4) == 1.);
  rv += SDK_ASSERT(simCore::round(-1.4) == -1.);
  rv += SDK_ASSERT(simCore::round(1.6) == 2.);
  rv += SDK_ASSERT(simCore::round(-1.6) == -2.);
  rv += SDK_ASSERT(simCore::round(2.5) == 3.);
  rv += SDK_ASSERT(simCore::round(-2.5) == -3.);

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;
  return rv;
}

int testAreAngleEqual()
{
  std::cerr << "Testing simCore::areAnglesEqual ======================================= " << std::endl;
  int rv = 0;
  double eps = 1e-6;

  // Test zero case
  rv += SDK_ASSERT(simCore::areAnglesEqual(0.0, 0.0));

  // Test typical values
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI_2, M_PI_2));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI_2, -M_PI_2));
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI_4, M_PI_4));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI_4, -M_PI_4));

  // Test typical values slightly off
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI_2-eps/4.0, M_PI_2+eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI_2-eps/4.0, -M_PI_2+eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI_4+eps/4.0, M_PI_4-eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI_4+eps/4.0, -M_PI_4-eps/4.0, eps));

  // Test 0 and 360 values
  rv += SDK_ASSERT(simCore::areAnglesEqual(2.0*M_PI, 2.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-2.0*M_PI, 2.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(2.0*M_PI, -2.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-2.0*M_PI, -2.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(2.0*M_PI, 0.0));
  rv += SDK_ASSERT(simCore::areAnglesEqual(0.0, 2.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-2.0*M_PI, 0.0));
  rv += SDK_ASSERT(simCore::areAnglesEqual(0.0, -2.0*M_PI));

  // Test 180 and -180
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI, M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI, M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI, -M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI, -M_PI));

  // Test 180 and -180 with slightly off values
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI-eps/4.0, M_PI+eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI+eps/4.0, M_PI-eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(M_PI-eps/4.0, -M_PI+eps/4.0, eps));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-M_PI+eps/4.0, -M_PI-eps/4.0, eps));

  // Test multiples of 180 and -180
  rv += SDK_ASSERT(simCore::areAnglesEqual(5.0*M_PI, 3.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-5.0*M_PI, 3.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(5.0*M_PI, -3.0*M_PI));
  rv += SDK_ASSERT(simCore::areAnglesEqual(-5.0*M_PI, -3.0*M_PI));

  // Test failures
  rv += SDK_ASSERT(!simCore::areAnglesEqual(0.0, M_PI_4));
  rv += SDK_ASSERT(!simCore::areAnglesEqual(0.0, 1.1*eps, eps));
  rv += SDK_ASSERT(!simCore::areAnglesEqual(0.0, -1.1*eps, eps));
  rv += SDK_ASSERT(!simCore::areAnglesEqual(M_PI_4, M_PI_4+1.1*eps, eps));
  rv += SDK_ASSERT(!simCore::areAnglesEqual(M_PI_4, M_PI_4-1.1*eps, eps));

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;
  return rv;
}

//===========================================================================
int dQNormTest(const double input[4], const double expected[4])
{
  double output[4] = {0};
  simCore::dQNorm(input, output);
  if (vectorsAreEqual(output, expected, 4))
    return 0;
  std::cerr << " FAILURE - Input Quaternion: [" << std::setprecision(PRECISION) << input[0] << ", " << input[1] << ", " << input[2] << ", " << input[3] << "]" << std::endl
            << "           Expected: [" << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3] << "]" << std::endl
            << "           Actual:   [" << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << "]" << std::endl
            << "           Diff:     [" << fabs(expected[0] - output[0]) << ", " << fabs(expected[1] - output[1]) << ", " << fabs(expected[2] - output[2]) << ", " << fabs(expected[3] - output[3]) <<"]" << std::endl;
  return 1;
}

//===========================================================================
int runQuaternionNormalTest()
{
  int rv = 0;
  std::cerr << "Testing simCore::dQNorm =============================================== " << std::endl;
  // Expected outcomes are based values from Scientific Python 2.9.2 Quaternion.normalized() on 12/12/13
  // http://dirac.cnrs-orleans.fr/ScientificPython/
  // >>> import Scientific.Geometry.Quaternion
  // >>> import math
  // >>> a = Scientific.Geometry.Quaternion.Quaternion(math.pi, 0, 0, 0)
  // >>> a.normalized()
  // Quaternion([1.0, 0.0, 0.0, 0.0])
  //
  // NOTE: Quaternion.normalized() fails for an input of (0, 0, 0, 0), our code catches the divide by zero
  // case and sets the return value to (0, 0, 0, 0)

                           // {         ------Quaternion------        ,          ------Expected Norm------        }
  double testParams[22][8] = { {       0,        0,        0,        0,         0,         0,         0,         0},
                               {    M_PI,        0,        0,        0,         1,         0,         0,         0},
                               {       0,     M_PI,        0,        0,         0,         1,         0,         0},
                               {       0,        0,     M_PI,        0,         0,         0,         1,         0},
                               {       0,        0,        0,     M_PI,         0,         0,         0,         1},  //5
                               {    M_PI,     M_PI,        0,        0,  0.707106,  0.707106,         0,         0},
                               {    M_PI,        0,     M_PI,        0,  0.707106,         0,  0.707106,         0},
                               {    M_PI,        0,        0,     M_PI,  0.707106,         0,         0,  0.707106},
                               {       0,     M_PI,     M_PI,        0,         0,  0.707106,  0.707106,         0},
                               {       0,     M_PI,        0,     M_PI,         0,  0.707106,         0,  0.707106},  //10
                               {       0,        0,     M_PI,     M_PI,         0,         0,  0.707106,  0.707106},
                               {    M_PI,     M_PI,     M_PI,        0,   0.57735,   0.57735,   0.57735,         0},
                               {    M_PI,     M_PI,        0,     M_PI,   0.57735,   0.57735,         0,   0.57735},
                               {    M_PI,        0,     M_PI,     M_PI,   0.57735,         0,   0.57735,   0.57735},
                               {       0,     M_PI,     M_PI,     M_PI,         0,   0.57735,   0.57735,   0.57735},  //15
                               {    M_PI,     M_PI,     M_PI,     M_PI,       0.5,       0.5,       0.5,       0.5},
                               { 4.11172, -5.71628,  6.07437,  4.15629,  0.403663, -0.561189,  0.596344,  0.408039},
                               {-3.40719,  8.78395, -1.02755, -2.00114, -0.351748,  0.906829, -0.106081, -0.206591},
                               {-2.54365, -1.65958,  2.21721,  3.26366, -0.510861, -0.333306,  0.445299,  0.655466},
                               {-2.97032,  1.58287,  8.02363, -9.11293, -0.235745,  0.125627,  0.636811, -0.723265},  //20
                               { 2.32196, -9.06369, -3.95736, -1.16816,  0.227068, -0.886353, -0.386997, -0.114236},
                               {-1.78372,  1.73589, -2.16737, -6.85712,  -0.23439,  0.228105, -0.284804, -0.901063} };

  size_t testCaseCount = sizeof(testParams) / sizeof(testParams[0]);
  size_t numParams = sizeof(testParams[0]) / sizeof(testParams[0][0]);
  for (size_t i = 0; i < testCaseCount; ++i)
  {
    double *val = testParams[i];
    rv += SDK_ASSERT(dQNormTest(val, val+4) == 0);
    for (size_t j = 0; j < numParams; ++j)
      testParams[i][j] *= -1;
    rv += SDK_ASSERT(dQNormTest(val, val+4) == 0);
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}


//===========================================================================
int dQMultTest(const double input1[4], const double input2[4], const double expected[4])
{
  double output[4] = {0};
  double q1[4] = {0};
  double q2[4] = {0};
  simCore::dQNorm(input1, q1);
  simCore::dQNorm(input2, q2);
  simCore::dQMult(q1, q2, output);
  if (vectorsAreEqual(output, expected, 4))
    return 0;
  std::cerr << " FAILURE - Input Quaternion1: [" << std::setprecision(PRECISION) << q1[0] << ", " << q1[1] << ", " << q1[2] << ", " << q1[3] << "]" << std::endl
            << "           Input Quaternion1: [" << q2[0] << ", " << q2[1] << ", " << q2[2] << ", " << q2[3] << "]" << std::endl
            << "           Expected: [" << expected[0] << ", " << expected[1] << ", " << expected[2] << ", " << expected[3] << "]" << std::endl
            << "           Actual:   [" << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << "]" << std::endl
            << "           Diff:     [" << fabs(expected[0] - output[0]) << ", " << fabs(expected[1] - output[1]) << ", " << fabs(expected[2] - output[2]) << ", " << fabs(expected[3] - output[3]) <<"]" << std::endl;
  return 1;
}

//===========================================================================
int runQuaternionMultiplicationTest()
{
  int rv = 0;
  std::cerr << "Testing simCore::dQMult =============================================== " << std::endl;
  // Expected outcomes are based values from Scientific Python 2.9.2 Quaternion multiplication on 12/12/13
  // http://dirac.cnrs-orleans.fr/ScientificPython/
  // >>> import Scientific.Geometry.Quaternion
  // >>> import math
  // >>> q1 = Scientific.Geometry.Quaternion.Quaternion(math.pi, 0, 0, 0).normalized()
  // >>> q2 = Scientific.Geometry.Quaternion.Quaternion(0, math.pi, 0, 0).normalized()
  // >>> print q1*q2
  // Quaternion([0.0, 1.0, 0.0, 0.0])
  //

                           // {         ------Quaternion 1------        ,         ------Quaternion 2------          ,          ------Expected Norm------        }
  double testParams[][12] = { {       M_PI,        0,        0,        0,         0,      M_PI,         0,         0,         0,         1,         0,         0},
                              {       M_PI,        0,        0,        0,      M_PI,         0,         0,         0,         1,         0,         0,         0},
                              {          0,        0,     M_PI,        0,         0,         0,         0,      M_PI,         0,         1,         0,         0},
                              {          0,     M_PI,        0,        0,         0,         0,         0,      M_PI,         0,         0,        -1,         0},
                              {       M_PI,     M_PI,        0,        0,      M_PI,         0,         0,         0,         0.70710678118654757,         0.70710678118654757,         0,         0},
                              {       M_PI,        0,     M_PI,        0,      0.70710678118654757,         0,         0.70710678118654757,         0,         0,         0,         1,         0},
                              {       1,           2,        3,        4,         5,         6,         7,         8,         -0.83045479853739956, 0.16609095970747995, 0.41522739926869978, 0.33218191941495989},
                              {0.40366342466166283, -0.56118927386226936, 0.59634435147871567, 0.40803903361294608, -0.51086107911939904, -0.33330640209343754, 0.44529958651320856, 0.65546630608724388, -0.92627164172279719, 0.36133017057546934, 0.10694067531623935, 0.005004551424758974}};

  size_t testCaseCount = sizeof(testParams) / sizeof(testParams[0]);
  for (size_t i = 0; i < testCaseCount; ++i)
  {
    double *val = testParams[i];
    rv += SDK_ASSERT(dQMultTest(val, val+4, val+8) == 0);
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

//===========================================================================
static int d3QtoEulerTest(const double input[4], const double expected[3])
{
  double output[3] = {0};
  simCore::Vec3 outVec;
  simCore::d3QtoEuler(input, outVec);
  outVec.toD3(output);
  if (vectorsAreEqual(output, expected, 3, 2e-6))
    return 0;
  std::cerr << " FAILURE - Input Quaternion: [" << std::setprecision(PRECISION) << input[0] << ", " << input[1] << ", " << input[2] << ", " << input[3] << "]" << std::endl
            << "           Expected: [" << expected[0] << ", " << expected[1] << ", " << expected[2] << "]" << std::endl
            << "           Actual:   [" << output[0] << ", " << output[1] << ", " << output[2] << "]" << std::endl
            << "           Diff:     [" << fabs(expected[0] - output[0]) << ", " << fabs(expected[1] - output[1]) << ", " << fabs(expected[2] - output[2]) << "]" << std::endl;
  return 1;
}

//===========================================================================
int d3EulertoQTest(const double input[3], const double expected[4])
{
  double output[4] = {0};
  simCore::d3EulertoQ(simCore::Vec3(input), output);
  if (vectorsAreEqual(output, expected, 4))
    return 0;
  std::cerr << " FAILURE - Input Euler: [" << std::setprecision(PRECISION) << input[0] << ", " << input[1] << ", " << input[2] << "]" << std::endl
            << "           Expected: [" << expected[0] << ", " << expected[1] << ", " << expected[2] <<", " << expected[3] << "]" << std::endl
            << "           Actual:   [" << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << "]" << std::endl
            << "           Diff:     [" << fabs(expected[0] - output[0]) << ", " << fabs(expected[1] - output[1]) << ", " << fabs(expected[2] - output[2]) << ", " << fabs(expected[3] - output[3]) <<"]" << std::endl;
  return 1;
}

//===========================================================================
int runD3QtoFromEulerTest()
{
  int rv = 0;
  std::cerr << "Testing simCore: d3QtoEuler and d3EulertoQ ============================ " << std::endl;
  // Expected outcomes were taken from MATLAB, Note, the values must be normalized
                             //{        ------Quaternion------        ,  -------Expected Angles (deg)------  }
  double testParams[][7] = {{1,         0,         0,         0,          0,  0,  0},
                            {0.923880,  0.382683,  0,         0,          0,  0, 45},
                            {0.923880,  0,         0.382683,  0,          0, 45,  0},
                            {0.923880,  0,         0,         0.382683,  45,  0,  0},
                            {0.853553,  0.353553,  0.353553, -0.146447,   0, 45, 45},
                            {0.853553,  0.353553,  0.146447,  0.353553,  45,  0, 45},
                            {0.853553, -0.146447,  0.353553,  0.353553,  45, 45,  0},
                            {0.844623,  0.191342,  0.461940,  0.191342,  45, 45, 45} };

  size_t testCaseCount = sizeof(testParams) / sizeof(testParams[0]);

  // Verify quaternions are normalized
  for (size_t ii = 0; ii < testCaseCount; ii++)
  {
    double mag = sqrt(simCore::square(testParams[ii][0]) + simCore::square(testParams[ii][1]) +simCore::square(testParams[ii][2]) + simCore::square(testParams[ii][3]));
    rv += SDK_ASSERT(simCore::areEqual(1.0, mag));
  }

  // convert to radians
  for (size_t ii = 0; ii < testCaseCount; ii++)
  {
    for (size_t jj = 0; jj < 3; jj++)
    {
      testParams[ii][jj+4] *= simCore::DEG2RAD;
    }
  }

  for (size_t ii = 0; ii < testCaseCount; ++ii)
  {
    double *val = testParams[ii];
    rv += SDK_ASSERT(d3EulertoQTest(val+4, val) == 0);
    if (ii == 4)
      val[4] = 360.0 * simCore::DEG2RAD;    // Band-aid fix; rounding errors shifts the value to other side zero
    rv += SDK_ASSERT(d3QtoEulerTest(val, val+4) == 0);
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

int runD3MMmult()
{
  int rv = 0;

  std::cerr << "Testing simCore::d3MMmult ============================================= " << std::endl;

  double mat1[][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  double mat2[][3] = {{0.1, 0.2, 0.3}, {0.4, 0.5, 0.6}, {0.7, 0.8, 0.9}};
  double expected[][3] = {{3.0, 3.6, 4.2}, {6.6, 8.1, 9.6}, {10.2, 12.6, 15.0}};  // From MATLAB

  double output[3][3];
  simCore::d3MMmult(mat1, mat2, output);
  for (size_t ii = 0; ii < 3; ii++)
  {
    rv += SDK_ASSERT(vectorsAreEqual(output[ii], expected[ii], 3));
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

int runD3MMTmult()
{
  int rv = 0;

  std::cerr << "Testing simCore::d3MMTmult ============================================ " << std::endl;

  double mat1[][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  double mat2[][3] = {{0.1, 0.2, 0.3}, {0.4, 0.5, 0.6}, {0.7, 0.8, 0.9}};
  double expected[][3] = {{1.4, 3.2, 5.0}, {3.2, 7.7, 12.2}, {5.0, 12.2, 19.4}};  // From MATLAB

  double output[3][3];
  simCore::d3MMTmult(mat1, mat2, output);
  for (size_t ii = 0; ii < 3; ii++)
  {
    rv += SDK_ASSERT(vectorsAreEqual(output[ii], expected[ii], 3));
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

int runD3DCMtoFromEuler()
{
  int rv = 0;

  std::cerr << "Testing simCore::d3EulertoDCM and d3DCMtoEuler ======================== " << std::endl;
  simCore::Vec3 ea(37.0 * simCore::DEG2RAD, 13.0 * simCore::DEG2RAD, 7.0 * simCore::DEG2RAD);
  double dcm[3][3] = {{ 0.778167, 0.586391, -0.224951}, // From MATLAB
                      {-0.575435, 0.809181,  0.118746},
                      { 0.251658, 0.037041,  0.967107}};
  simCore::Vec3 eaOutput;
  double dcmOutput[3][3];

  simCore::d3EulertoDCM(ea, dcmOutput);
  for (size_t ii = 0; ii < 3; ii++)
  {
    rv += SDK_ASSERT(vectorsAreEqual(dcmOutput[ii], dcm[ii], 3));
  }

  simCore::d3DCMtoEuler(dcm, eaOutput);
  rv += SDK_ASSERT(simCore::v3AreEqual(ea, eaOutput));
  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

int runV3SphtoRec()
{
  int rv = 0;

  std::cerr << "Testing simCore::v3SphtoRec =========================================== " << std::endl;
  double input[][3] =    {{10.0,   0.0,  0.0},
                          {10.0,  45.0,  0.0},
                          {10.0,  90.0,  0.0},
                          {10.0, 180.0,  0.0},
                          {10.0, 270.0,  0.0},
                          {10.0, 360.0,  0.0},
                          {10.0,   0.0, 45.0},
                          {10.0,  45.0, 45.0},
                          {10.0,  90.0, 45.0},
                          {10.0, 180.0, 45.0},
                          {10.0, 270.0, 45.0},
                          {10.0, 360.0, 45.0}};
  double expected[][3] = {{0.0,         10.0,         0.0},   // Excel
                          {  7.071067812,  7.071067812, 0.0},
                          { 10.0,          0.0,         0.0},
                          {  0.0,        -10.0,         0.0},
                          {-10.0,          0.0,         0.0},
                          {  0.0,          10.0,        0.0},
                          {  0.0,          7.071067812, 7.071067812},
                          {  5.0,          5.0,         7.071067812},
                          {  7.071067812,  0.0,         7.071067812},
                          {  0.0,         -7.071067812, 7.071067812},
                          { -7.071067812,  0.0,         7.071067812},
                          {  0.0,          7.071067812, 7.071067812}};


  for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); i++)
  {
    simCore::Vec3 output;
    // Convert to radians
    input[i][1] *= simCore::DEG2RAD;
    input[i][2] *= simCore::DEG2RAD;
    simCore::v3SphtoRec(simCore::Vec3(input[i]), output);
    rv += SDK_ASSERT(simCore::v3AreEqual(output, simCore::Vec3(expected[i])));
    // verify that the expected values make sense
    rv += SDK_ASSERT(simCore::areEqual(input[i][0], simCore::v3Length(output)));
  }

  std::cerr << ((rv == 0) ? "PASS" : "FAILED") << std::endl;

  return rv;
}

int testIsFinite()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::isFinite(simCore::Vec3(0.0, 0.0, 0.0)));
  rv += SDK_ASSERT(simCore::isFinite(simCore::Vec3(1.0, 2.0, 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::infinity(), 2.0, 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, std::numeric_limits<double>::infinity(), 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, 2.0, std::numeric_limits<double>::infinity())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::infinity(), 2.0, std::numeric_limits<double>::infinity())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity())));

  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::quiet_NaN(), 2.0, 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, std::numeric_limits<double>::quiet_NaN(), 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, 2.0, std::numeric_limits<double>::quiet_NaN())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), 3.0)));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::quiet_NaN(), 2.0, std::numeric_limits<double>::quiet_NaN())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(1.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN())));
  rv += SDK_ASSERT(!simCore::isFinite(simCore::Vec3(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN())));

  return rv;
}

int testSciValue(double value, double mantissa, int exponent)
{
  int rvExp = 0;
  const double rvMant = simCore::toScientific(value, &rvExp);
  if (rvExp != exponent)
    return 1;
  return simCore::areEqual(rvMant, mantissa) ? 0 : 2;
}

int testToScientific()
{
  int rv = 0;

  rv += SDK_ASSERT(0 == testSciValue(1000.0, 1.0, 3));
  rv += SDK_ASSERT(0 == testSciValue(100.0, 1.0, 2));
  rv += SDK_ASSERT(0 == testSciValue(99.99, 9.999, 1));
  rv += SDK_ASSERT(0 == testSciValue(10.0, 1.0, 1));
  rv += SDK_ASSERT(0 == testSciValue(1.0, 1.0, 0));
  rv += SDK_ASSERT(0 == testSciValue(0.1, 1.0, -1));
  rv += SDK_ASSERT(0 == testSciValue(0.01, 1.0, -2));
  rv += SDK_ASSERT(0 == testSciValue(0.001, 1.0, -3));

  rv += SDK_ASSERT(0 == testSciValue(80.0, 8.0, 1));
  rv += SDK_ASSERT(0 == testSciValue(8.0, 8.0, 0));
  rv += SDK_ASSERT(0 == testSciValue(0.8, 8.0, -1));
  rv += SDK_ASSERT(0 == testSciValue(0.08, 8.0, -2));
  rv += SDK_ASSERT(0 == testSciValue(0.008, 8.0, -3));

  rv += SDK_ASSERT(0 == testSciValue(-1000.0, -1.0, 3));
  rv += SDK_ASSERT(0 == testSciValue(-100.0, -1.0, 2));
  rv += SDK_ASSERT(0 == testSciValue(-99.99, -9.999, 1));
  rv += SDK_ASSERT(0 == testSciValue(-10.0, -1.0, 1));
  rv += SDK_ASSERT(0 == testSciValue(-1.0, -1.0, 0));
  rv += SDK_ASSERT(0 == testSciValue(-0.1, -1.0, -1));
  rv += SDK_ASSERT(0 == testSciValue(-0.01, -1.0, -2));
  rv += SDK_ASSERT(0 == testSciValue(-0.001, -1.0, -3));

  rv += SDK_ASSERT(0 == testSciValue(-80.0, -8.0, 1));
  rv += SDK_ASSERT(0 == testSciValue(-8.0, -8.0, 0));
  rv += SDK_ASSERT(0 == testSciValue(-0.8, -8.0, -1));
  rv += SDK_ASSERT(0 == testSciValue(-0.08, -8.0, -2));
  rv += SDK_ASSERT(0 == testSciValue(-0.008, -8.0, -3));

  rv += SDK_ASSERT(0 == testSciValue(-1.0, -1.0, 0));
  rv += SDK_ASSERT(0 == testSciValue(0.0, 0.0, 0));
  return rv;
}

int testGuessStepSize()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10, 2), 0.01));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9, 2), 0.01));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10, 1), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9, 1), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9, 0), 1.0));

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(100, 2), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 99, 2), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(100, 1), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 99, 1), 0.1));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(100, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 99, 0), 1.0));

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(180, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(179, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(180, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(179, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(180, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(179, 0), 1.0));

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(360, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(359, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(360, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(359, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(360, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(359, 0), 1.0));

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(1000, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 999, 2), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(1000, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 999, 1), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(1000, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 999, 0), 1.0));

  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10000, 2), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9999, 2), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10000, 1), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9999, 1), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize(10000, 0), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(simCore::guessStepSize( 9999, 0), 10.0));

  return rv;
}

int testPowerOfTenSignificance()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.0, 0) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.0, 5) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.0, -5) == 0);

  // Positive significance: 2

  // Test positive significance of 2.  For example, 12340.0 is -3, because 2 digits
  // of significance on 12340 is the "12" part, so "12340 * 10^-3 == 12.340"
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(12340.0, 2) == -3);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1234.0, 2) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(123.4, 2) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(12.34, 2) == 0);
  // Note, 1.234 is within the power of ten significance for 10^0 for 2 digits of precision
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.234, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.1234, 2) == 2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.01234, 2) == 3);

  // Test positive significance of 3
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(12340.0, 3) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1234.0, 3) == -1);
  // Note the following 3 values are within significance for 10^3
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(123.4, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(12.34, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.234, 3) == 0);
  // ... and here we jump back out into 10^3, 10^4, etc
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.1234, 3) == 3);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.01234, 3) == 4);

  // Test negative values
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-12340.0, 3) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1234.0, 3) == -1);
  // Note the following 3 values are within significance for 10^3
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-123.4, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-12.34, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1.234, 3) == 0);
  // ... and here we jump back out into 10^3, 10^4, etc
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-0.1234, 3) == 3);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-0.01234, 3) == 4);

  // Edge cases for 2 significance
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10001.0, 2) == -3);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10000.0, 2) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(9999.9, 2) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1000.1, 2) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1000.0, 2) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(999.99, 2) == -1);

  // 0 significance
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(123.4, 0) == -3);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(12.34, 0) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.234, 0) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.1234, 0) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(0.01234, 0) == 1);

  // Test +1 and -1, which should return 0 for 10^0
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.0, 0) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.0, 1) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(1.0, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1.0, 0) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1.0, 1) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-1.0, 3) == 0);

  // Edge cases for 10 and -10
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10.0, 0) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10.0, 1) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(10.0, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-10.0, 0) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-10.0, 1) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-10.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-10.0, 3) == 0);

  // Edge cases for 100 and -100
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(100.0, 0) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(100.0, 1) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(100.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(100.0, 3) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-100.0, 0) == -2);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-100.0, 1) == -1);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-100.0, 2) == 0);
  rv += SDK_ASSERT(simCore::getPowerOfTenForSignificance(-100.0, 3) == 0);

  return rv;
}

int helpTestRoundRanges(double minValue, double maxValue, double expectedOutMin, double expectedOutMax)
{
  double tmin = minValue;
  double tmax = maxValue;
  simCore::roundRanges(minValue, maxValue);
  const int rv = (simCore::areEqual(expectedOutMin, minValue) && simCore::areEqual(expectedOutMax, maxValue)) ? 0 : 1;
  if (rv != 0)
    std::cout << "Failed Result: " << minValue << " to " << maxValue << "\n";
  return rv;
}

int testRoundRanges()
{
  int rv = 0;

  // Various "regular" input, including example from the documentation
  rv += SDK_ASSERT(helpTestRoundRanges(1.5, 19.7, 1.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.5, 19.7, 0.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.005, 19.7, 0.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-0.005, 19.7, -1.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-1.5, 19.7, -2.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-8.5, 19.7, -9.0, 20.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-9.5, 19.7, -10.0, 20.0) == 0);

  // Use 1970, 4 digits of significance.  Note the range is ~2000, and rounding
  // uses 2 digits, so expected minimum resolution is 100, thus 0 and -100 minimums
  rv += SDK_ASSERT(helpTestRoundRanges(1.5, 1970, 0.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.5, 1970, 0.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.005, 1970, 0.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-0.005, 1970, -100.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-1.5, 1970, -100.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-8.5, 1970, -100.0, 2000.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(-9.5, 1970, -100.0, 2000.0) == 0);

  // Repeat the tests with swapped min/max values
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, 1.5, 20.0, 1.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, 0.5, 20.0, 0.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, 0.005, 20.0, 0.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, -0.005, 20.0, -1.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, -1.5, 20.0, -2.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, -8.5, 20.0, -9.0) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(19.7, -9.5, 20.0, -10.0) == 0);

  // Test smaller and larger values
  rv += SDK_ASSERT(helpTestRoundRanges(0.015, 0.197, 0.01, 0.20) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.00015, 0.00197, 0.0001, 0.0020) == 0);
  rv += SDK_ASSERT(helpTestRoundRanges(0.000015, 0.000197, 0.00001, 0.00020) == 0);

  return rv;
}

int testBetween()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::isBetween(5, 0, 10));
  rv += SDK_ASSERT(simCore::isBetween(5, 10, 0));
  rv += SDK_ASSERT(simCore::isBetween(5, -10, 10));
  rv += SDK_ASSERT(simCore::isBetween(5, 10, -10));
  rv += SDK_ASSERT(simCore::isBetween(5, 0, std::numeric_limits<int>::max()));
  rv += SDK_ASSERT(simCore::isBetween(5, std::numeric_limits<int>::max(), 0));
  rv += SDK_ASSERT(simCore::isBetween(5, 5, 5));

  rv += SDK_ASSERT(!simCore::isBetween(-5, 0, 10));
  rv += SDK_ASSERT(!simCore::isBetween(-5, 10, 0));
  rv += SDK_ASSERT(!simCore::isBetween(15, 0, 10));
  rv += SDK_ASSERT(!simCore::isBetween(15, 10, 0));
  rv += SDK_ASSERT(!simCore::isBetween(-15, -10, 10));
  rv += SDK_ASSERT(!simCore::isBetween(-15, 10, -10));
  rv += SDK_ASSERT(!simCore::isBetween(25, -10, 10));
  rv += SDK_ASSERT(!simCore::isBetween(25, 10, -10));
  rv += SDK_ASSERT(!simCore::isBetween(-5, 0, std::numeric_limits<int>::max()));
  rv += SDK_ASSERT(!simCore::isBetween(-5, std::numeric_limits<int>::max(), 0));
  rv += SDK_ASSERT(!simCore::isBetween(6, 5, 5));

  rv += SDK_ASSERT(simCore::isBetween(5., 0., 10.));
  rv += SDK_ASSERT(simCore::isBetween(5., 10., 0.));
  rv += SDK_ASSERT(simCore::isBetween(5., -10., 10.));
  rv += SDK_ASSERT(simCore::isBetween(5., 10., -10.));
  rv += SDK_ASSERT(simCore::isBetween(5., 0., std::numeric_limits<double>::max()));
  rv += SDK_ASSERT(simCore::isBetween(5., std::numeric_limits<double>::max(), 0.));
  rv += SDK_ASSERT(simCore::isBetween(5., 5., 5.));

  rv += SDK_ASSERT(!simCore::isBetween(-5., 0., 10.));
  rv += SDK_ASSERT(!simCore::isBetween(-5., 10., 0.));
  rv += SDK_ASSERT(!simCore::isBetween(15., -10., 10.));
  rv += SDK_ASSERT(!simCore::isBetween(15., 10., -10.));
  rv += SDK_ASSERT(!simCore::isBetween(-5., 0., std::numeric_limits<double>::max()));
  rv += SDK_ASSERT(!simCore::isBetween(-5., std::numeric_limits<double>::max(), 0.));
  rv += SDK_ASSERT(!simCore::isBetween(6., 5., 5.));

  return rv;
}

int testClamp()
{
  int rv = 0;

  rv += SDK_ASSERT(5 == simCore::clamp(5, 0, 10));
  rv += SDK_ASSERT(5 == simCore::clamp(5, 10, 0));
  rv += SDK_ASSERT(5 == simCore::clamp(5, -10, 10));
  rv += SDK_ASSERT(5 == simCore::clamp(5, 10, -10));
  rv += SDK_ASSERT(5 == simCore::clamp(5, 0, std::numeric_limits<int>::max()));
  rv += SDK_ASSERT(5 == simCore::clamp(5, std::numeric_limits<int>::max(), 0));
  rv += SDK_ASSERT(5 == simCore::clamp(5, 5, 5));

  rv += SDK_ASSERT(0 == simCore::clamp(-5, 0, 10));
  rv += SDK_ASSERT(0 == simCore::clamp(-5, 10, 0));
  rv += SDK_ASSERT(10 == simCore::clamp(15, 0, 10));
  rv += SDK_ASSERT(10 == simCore::clamp(15, 10, 0));
  rv += SDK_ASSERT(-10 == simCore::clamp(-15, -10, 10));
  rv += SDK_ASSERT(-10 == simCore::clamp(-15, 10, -10));
  rv += SDK_ASSERT(10 == simCore::clamp(25, -10, 10));
  rv += SDK_ASSERT(10 == simCore::clamp(25, 10, -10));
  rv += SDK_ASSERT(0 == simCore::clamp(-5, 0, std::numeric_limits<int>::max()));
  rv += SDK_ASSERT(0 == simCore::clamp(-5, std::numeric_limits<int>::max(), 0));
  rv += SDK_ASSERT(6 == simCore::clamp(5, 6, 6));
  rv += SDK_ASSERT(6 == simCore::clamp(7, 6, 6));

  rv += SDK_ASSERT(5. == simCore::clamp(5., 0., 10.));
  rv += SDK_ASSERT(5. == simCore::clamp(5., 10., 0.));
  rv += SDK_ASSERT(5. == simCore::clamp(5., -10., 10.));
  rv += SDK_ASSERT(5. == simCore::clamp(5., 10., -10.));
  rv += SDK_ASSERT(5. == simCore::clamp(5., 0., std::numeric_limits<double>::max()));
  rv += SDK_ASSERT(5. == simCore::clamp(5., std::numeric_limits<double>::max(), 0.));
  rv += SDK_ASSERT(5. == simCore::clamp(5., 5., 5.));

  rv += SDK_ASSERT(0. == simCore::clamp(-5., 0., 10.));
  rv += SDK_ASSERT(0. == simCore::clamp(-5., 10., 0.));
  rv += SDK_ASSERT(10. == simCore::clamp(15., -10., 10.));
  rv += SDK_ASSERT(10. == simCore::clamp(15., 10., -10.));
  rv += SDK_ASSERT(0. == simCore::clamp(-5., 0., std::numeric_limits<double>::max()));
  rv += SDK_ASSERT(0. == simCore::clamp(-5., std::numeric_limits<double>::max(), 0.));
  rv += SDK_ASSERT(6. == simCore::clamp(5., 6., 6.));
  rv += SDK_ASSERT(6. == simCore::clamp(7., 6., 6.));

  return rv;
}

}

int MathTest(int argc, char* argv[])
{
  int rv = 0;

  rv += testRint();
  rv += testRound();
  rv += testAreAngleEqual();
  rv += testIsFinite();
  rv += runQuaternionNormalTest();
  rv += runQuaternionMultiplicationTest();
  rv += runD3QtoFromEulerTest();
  rv += runD3MMmult();
  rv += runD3MMTmult();
  rv += runD3DCMtoFromEuler();
  rv += runV3SphtoRec();
  rv += testToScientific();
  rv += testGuessStepSize();
  rv += testPowerOfTenSignificance();
  rv += testRoundRanges();
  rv += testBetween();
  rv += testClamp();

  return rv;
}
