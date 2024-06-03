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
#include <vector>
#include <string>
#include <iomanip>
#include "simCore/Time/Constants.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"

namespace
{
//===========================================================================
class TestCase
{
  public:
    int UniqueID_;
    bool CheckPosition_;
    bool CheckEul_;
    bool CheckVelocity_;
    bool CheckAcc_;
    simCore::Vec3 InputPosition_;
    simCore::Vec3 InputEul_;
    simCore::Vec3 InputVelocity_;
    simCore::Vec3 InputAcc_;
    simCore::CoordinateSystem InputCoordinateSystem_;
    simCore::CoordinateSystem OutputCoordinateSystem_;
    simCore::Vec3 CorrectOutputPosition_;
    simCore::Vec3 CorrectOutputEul_;
    simCore::Vec3 CorrectOutputVelocity_;
    simCore::Vec3 CorrectOutputAcc_;

    TestCase(int uniqueID, simCore::CoordinateSystem inputCoordinateSystem, simCore::CoordinateSystem outputCoordinateSystem)
      : UniqueID_(uniqueID),
        CheckPosition_(false),
        CheckEul_(false),
        CheckVelocity_(false),
        CheckAcc_(false),
        InputCoordinateSystem_(inputCoordinateSystem),
        OutputCoordinateSystem_(outputCoordinateSystem)
    {
      InputPosition_.zero();
      InputEul_.zero();
      InputVelocity_.zero();
      InputAcc_.zero();
      CorrectOutputPosition_.zero();
      CorrectOutputEul_.zero();
      CorrectOutputVelocity_.zero();
      CorrectOutputAcc_.zero();
    }

    void SetInputPosition(double a, double b, double c)       {CheckPosition_ = true; InputPosition_.set(a, b, c);}
    void SetInputPositionLLADeg(double a, double b, double c) {CheckPosition_ = true; InputPosition_.set((a * simCore::DEG2RAD), (b * simCore::DEG2RAD), c);}
    void SetInputEul(double a, double b, double c)            {CheckEul_ = true;      InputEul_.set((a * simCore::DEG2RAD), (b * simCore::DEG2RAD), (c * simCore::DEG2RAD));}
    void SetInputVelocity(double a, double b, double c)       {CheckVelocity_ = true; InputVelocity_.set(a, b, c);}
    void SetInputAcc(double a, double b, double c)            {CheckAcc_ = true;      InputAcc_.set(a, b, c);}
    void SetCorrectOutputPosition(double a, double b, double c)       {CorrectOutputPosition_.set(a, b, c);}
    void SetCorrectOutputPositionLLADeg(double a, double b, double c) {CorrectOutputPosition_.set((a * simCore::DEG2RAD), (b * simCore::DEG2RAD), c);}
    void SetCorrectOutputEul(double a, double b, double c)            {CorrectOutputEul_.set((a * simCore::DEG2RAD), (b * simCore::DEG2RAD), (c * simCore::DEG2RAD));}
    void SetCorrectOutputVelocity(double a, double b, double c)       {CorrectOutputVelocity_.set(a, b, c);}
    void SetCorrectOutputAcc(double a, double b, double c)            {CorrectOutputAcc_.set(a, b, c);}
};

typedef std::vector<TestCase*> vTestCases;

//===========================================================================
static void createTestCases(vTestCases* testCases);

//===========================================================================
static bool almostEqual(const simCore::Vec3& value1, const simCore::Vec3& value2, double epsilon, double epsilon2)
{
  if (!simCore::areEqual(value1[0], value2[0], epsilon))
  {
    std::cerr.precision(16);
    std::cerr << std::endl << value1[0] << " " << value2[0] << " value[0] failed" << std::endl;
    std::cerr << "del: " << value1[0] - value2[0] << std::endl;
    return false;
  }
  if (!simCore::areEqual(value1[1], value2[1], epsilon))
  {
    std::cerr.precision(16);
    std::cerr << std::endl << value1[1] << " " << value2[1] << " value[1] failed" << std::endl;
    std::cerr << "del: " << value1[1] - value2[1] << std::endl;
    return false;
  }
  // use 2nd epsilon to handle altitude values for geodetic test cases
  if (!simCore::areEqual(value1[2], value2[2], epsilon2))
  {
    std::cerr.precision(16);
    std::cerr << std::endl << value1[2] << " " << value2[2] << " value[2] failed" << std::endl;
    std::cerr << "del: " << value1[2] - value2[2] << std::endl;
    return false;
  }
  return true;
}

//===========================================================================
static bool almostEqualPos(const simCore::Coordinate& cv1, const simCore::Coordinate& cv2, double epsilon = 1e-5)
{
  return simCore::v3AreEqual(cv1.position(), cv2.position(), epsilon);
}

//===========================================================================
static bool almostEqualCoord(const simCore::Coordinate& cv1, const simCore::Coordinate& cv2, double epsilon = 1e-5, double epsilon2 = 1e-5)
{
  if (!almostEqual(cv1.position(), cv2.position(), epsilon, epsilon2))
  {
    std::cerr << "Failed position" << std::endl;
    return false;
  }
  if (!simCore::v3AreEqual(cv1.velocity(), cv2.velocity(), epsilon))
  {
    std::cerr << "Failed velocity" << std::endl;
    return false;
  }
  if (!simCore::v3AreEqual(cv1.orientation(), cv2.orientation(), epsilon))
  {
    std::cerr << "Failed orientation" << std::endl;
    return false;
  }
  return true;
}

//===========================================================================
static int checkValues(int uniqueID, const char* whichTest,
                       const simCore::Vec3& result, const simCore::Vec3& correctValue, double epsilon)
{
  if (!simCore::v3AreEqual(result, correctValue, epsilon))
  {
    std::cerr << "Test Failure:  UniqueID(" << uniqueID << "):\n" << std::fixed << std::setprecision(7)
      << "  result values  = (" << result[0] << ", " << result[1] << ", " << result[2] << ")\n"
      << "  correct values = (" << correctValue[0] << ", " << correctValue[1] << ", " << correctValue[2] << ")\n";
    return 1;
  }
  return 0;
}

// calculate ecef->lla->ecef, comparing expected values of ecef and lla
int ecefLlaEcef(const simCore::CoordinateConverter& cc, const simCore::Coordinate& ecef, const simCore::Coordinate& lla, double epsilon = 1e-5, double epsilon2 = 1e-5)
{
  int rv = 0;
  simCore::Coordinate llaFromEcef;
  cc.convert(ecef, llaFromEcef, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEcef.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(lla, llaFromEcef, epsilon, epsilon2));

  simCore::Coordinate ecefFromLla;
  cc.convert(llaFromEcef, ecefFromLla, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromLla.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecef, ecefFromLla, epsilon, epsilon2));
  return rv;
}

//===========================================================================
// Note on putting this data in a file:
//
// Doing so would add one more step (file input) that could create errors
// in the test results.
//
static void createTestCases(vTestCases* testCases)
{
  if (testCases == nullptr)
    return;
  TestCase* tempTestCase;

  // ECEF to LLA --------------------------------------------------------------
  // Position:      near Africa
  // Orientation:   heading north
  // Velocity:      north at 10
  // Acceleration:  north at 10
  tempTestCase = new TestCase(1, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(6378137.0, 0.0, 0.0); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(0.0, 0.0, 0.0); // LLA
  tempTestCase->SetInputEul(0.0, -90.0, 0.0);   // ECEF
  tempTestCase->SetCorrectOutputEul(0.0,     0.0, 0.0); // LLA
  tempTestCase->SetInputVelocity(0.0,  0.0, 10.0); // ECEF
  tempTestCase->SetCorrectOutputVelocity(0.0, 10.0,  0.0); // LLA
  tempTestCase->SetInputAcc(0.0, 0.0,  10.0); // ECEF
  tempTestCase->SetCorrectOutputAcc(0.0, 10.0, 0.0);  // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:      near Africa at altitude of 10000
  // Orientation:   heading south
  // Velocity:      south at 10
  // Acceleration:  south at 10
  tempTestCase = new TestCase(2, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(6388137.0, 0.0,     0.0); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(0.0, 0.0, 10000.0); // LLA
  tempTestCase->SetInputEul(0.0, 90.0, 180.0); // ECEF
  tempTestCase->SetCorrectOutputEul(180.0,   0.0, 0.0); // LLA
  tempTestCase->SetInputVelocity(0.0,   0.0, -10.0);  // ECEF
  tempTestCase->SetCorrectOutputVelocity(0.0, -10.0,   0.0);  // LLA
  tempTestCase->SetInputAcc(0.0,   0.0, -10.0); // ECEF
  tempTestCase->SetCorrectOutputAcc(0.0, -10.0,   0.0); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  North Pole
  tempTestCase = new TestCase(3, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, simCore::WGS_B); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(90.0, 0.0,         0.0); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  South Pole
  tempTestCase = new TestCase(4, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, -simCore::WGS_B); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-90.0, 0.0,         0.0); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  South Pole, 2.1 km above surface
  tempTestCase = new TestCase(5, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, -6378099.7802647511); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-90.0, 0.0,       21347.46602); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position: NGA GoldData 6.3, WGS84, rectangular line 85, geodetic line 86
  tempTestCase = new TestCase(6, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(3921315.206497, -3921315.206497, -3180373.735384); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg( -30.0, -45.0, 20000.0); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position: NGA GoldData 6.3, WGS84, rectangular line 167, geodetic line 168
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(7, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(3921315.206497, 3921315.206497, 3180373.735384); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg( 30.0, 45.0, 20000.0); // LLA
  testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position: NGA GoldData 6.3, WGS84, rectangular line 460, geodetic line 461
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(8, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(4595548.289592, 0.0, 4408161.078281); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg( 44.0, 0.0, 100.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 203, geodetic line 204
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(9, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(-2267765.401388, -2267765.401388, 5517797.642014); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(60.0, -135.0, 20000.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 271, geodetic line 272
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(10, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(2650785.323332, 0.0, 6865553.346493); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(69.0, 0.0, 1000000.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 271, geodetic line 272
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(11, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(2650785.323332, 0.0, 6865553.346493); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(69.0, 0.0, 1000000.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 272, geodetic line 273
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(12, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(2039152.983916, 0.0, 7070200.396837); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(74.0, 0.0, 1000000.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 427, geodetic line 428
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(13, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(6375072.400269, 0.0, -110532.124771); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-1.0, 0.0, -2100.0); // LLA
  testCases->push_back(tempTestCase);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 37, geodetic line 38
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  tempTestCase = new TestCase(14, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(2260694.333577, -2260694.333577, -5500477.133939); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-60.0, -45.0, 0.0); // LLA
  testCases->push_back(tempTestCase);

  // NGA GoldData does not test near-polar latitudes with non-zero longitudes - this means testing does not verify calculation of longitude for such points.
  // test cases 15-22 use our LLA-ECEF conversion to produce ECEF positions.

  tempTestCase = new TestCase(15, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(11167.8655243, 194.935817837, 6356842.566957016475); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.9, 1.0, 100.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(16, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(111.678713082785, 1.949359187960, 6356852.313270461746); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99900000, 1.0, 100.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(17, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(1.116787131430, 0.019493591890, 6356852.314245189540); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99999000, 1.0, 100.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(18, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(.111678712501, .001949359178, 6356852.314245189540); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99999900, 1.0, 100.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(19, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(11185.141643517663, 195.237373618911, 6366742.551878457889); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.9, 1.0, 10000.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(20, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(111.851474362336, 1.952374747311, 6366752.313268953934); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99900000, 1.0, 10000.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(21, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(1.118514744226, 0.019523747484, 6366752.314245093614); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99999000, 1.0, 10000.0); // LLA
  testCases->push_back(tempTestCase);

  tempTestCase = new TestCase(22, simCore::COORD_SYS_ECEF, simCore::COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.111851473780, 0.001952374737, 6366752.314245189540); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(89.99999900, 1.0, 10000.0); // LLA
  testCases->push_back(tempTestCase);
}

//===========================================================================
int testGtp()
{
  int rv = 0;
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(49.3, -123.9666667, -16.77);
  cc.setTangentPlaneOffsets(-14063.024, 5641.235, 13.145999 * simCore::DEG2RAD);

  // double dLlaPos[] = { 49.3368930371 * simCore::DEG2RAD, -124.073426963 * simCore::DEG2RAD, -10.7299084021 };
  // source of the gtp and lla data is not documented;
  // these values may represent the expected conversion, based on some version of our own code.
  // as of 03/2018, dLlaPos.alt now represents the expected conversion from dGtpPos
  double dLlaPos[] = { 49.3368930371 * simCore::DEG2RAD, -124.073426963 * simCore::DEG2RAD, -10.735150311142206};
  double dLlaVel[] = {9.804, -5.375, 0.002};
  double llaSpeed = sqrt(simCore::square(dLlaVel[0]) + simCore::square(dLlaVel[1]) + simCore::square(dLlaVel[2]));
  double dLlaAcc[] = {2.343, -1.438, 0.003};
  double llaAccMag = sqrt(simCore::square(dLlaAcc[0]) + simCore::square(dLlaAcc[1]) + simCore::square(dLlaAcc[2]));
  simCore::Coordinate llaPos(simCore::COORD_SYS_LLA, simCore::Vec3(dLlaPos));
  llaPos.setVelocity(simCore::Vec3(dLlaVel));
  llaPos.setAcceleration(simCore::Vec3(dLlaAcc));

  double dGtpPos[] = {6487.4, -58.7639, 0.0};
  double dGtpVel[] = {10, 5, 0};
  double gtpSpeed = sqrt(simCore::square(dGtpVel[0]) + simCore::square(dGtpVel[1]) + simCore::square(dGtpVel[2]));
  double dGtpAcc[] = {1.438, -2.343, 0.005};
  double gtpAccMag = sqrt(simCore::square(dGtpAcc[0]) + simCore::square(dGtpAcc[1]) + simCore::square(dGtpAcc[2]));
  simCore::Coordinate gtpPos(simCore::COORD_SYS_GTP, simCore::Vec3(dGtpPos));
  gtpPos.setVelocity(simCore::Vec3(dGtpVel));
  gtpPos.setAcceleration(simCore::Vec3(dGtpAcc));

  // Make sure the speeds and acceleration magnitude are the same to start off with
  rv += SDK_ASSERT(simCore::areEqual(gtpSpeed, llaSpeed, 0.001));
  rv += SDK_ASSERT(simCore::areEqual(gtpAccMag, llaAccMag, 0.001));

  simCore::Coordinate gtpFromLla;
  cc.convert(llaPos, gtpFromLla, simCore::COORD_SYS_GTP);
  double gtpSpeedFromLla = sqrt(simCore::square(gtpFromLla.velocity()[0]) + simCore::square(gtpFromLla.velocity()[1]) + simCore::square(gtpFromLla.velocity()[2]));
  double gtpAccMagFromLla = sqrt(simCore::square(gtpFromLla.acceleration()[0]) + simCore::square(gtpFromLla.acceleration()[1]) + simCore::square(gtpFromLla.acceleration()[2]));
  rv += SDK_ASSERT(gtpFromLla.coordinateSystem() == simCore::COORD_SYS_GTP);
  rv += SDK_ASSERT(almostEqualPos(gtpFromLla, gtpPos, 0.005));
  rv += SDK_ASSERT(simCore::areEqual(gtpSpeedFromLla, llaSpeed, 0.001));
  rv += SDK_ASSERT(simCore::areEqual(gtpAccMagFromLla, llaAccMag, 0.001));

  simCore::Coordinate gtpFromGtp;
  cc.convert(gtpPos, gtpFromGtp, simCore::COORD_SYS_GTP);
  rv += SDK_ASSERT(gtpFromGtp.coordinateSystem() == simCore::COORD_SYS_GTP);
  rv += SDK_ASSERT(almostEqualPos(gtpFromGtp, gtpPos, 0.001));
  rv += SDK_ASSERT(almostEqual(gtpFromGtp.velocity(), simCore::Vec3(dGtpVel), 0.001, 0.001));
  rv += SDK_ASSERT(almostEqual(gtpFromGtp.acceleration(), simCore::Vec3(dGtpAcc), 0.001, 0.001));

  simCore::Coordinate llaFromGtp;
  cc.convert(gtpPos, llaFromGtp, simCore::COORD_SYS_LLA);
  double llaSpeedFromGtp = sqrt(simCore::square(llaFromGtp.velocity()[0]) + simCore::square(llaFromGtp.velocity()[1]) + simCore::square(llaFromGtp.velocity()[2]));
  double llaAccMagFromGtp = sqrt(simCore::square(llaFromGtp.acceleration()[0]) + simCore::square(llaFromGtp.acceleration()[1]) + simCore::square(llaFromGtp.acceleration()[2]));
  rv += SDK_ASSERT(llaFromGtp.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualPos(llaFromGtp, llaPos, 0.001));
  rv += SDK_ASSERT(simCore::areEqual(llaSpeedFromGtp, gtpSpeed, 0.001));
  rv += SDK_ASSERT(simCore::areEqual(llaAccMagFromGtp, gtpAccMag, 0.001));

  simCore::Coordinate llaFromLla;
  cc.convert(llaPos, llaFromLla, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromLla.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualPos(llaFromLla, llaPos, 0.001));
  rv += SDK_ASSERT(almostEqual(llaFromLla.velocity(), simCore::Vec3(dLlaVel), 0.001, 0.001));
  rv += SDK_ASSERT(almostEqual(llaFromLla.acceleration(), simCore::Vec3(dLlaAcc), 0.001, 0.001));

  std::cout << std::endl << "GTP test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

//===========================================================================
int testCC()
{
  int rv = 0;
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(-2.95192266, 4.50036968, 0.);
  const double elapsedEciTime = 17893481.467999998 - 1.69576726;

  const double dLlaPos[] = {-2.95192266 * simCore::DEG2RAD, 4.50036968 * simCore::DEG2RAD, 995807.83470784};
  simCore::Coordinate llaPos(simCore::COORD_SYS_LLA, simCore::Vec3(dLlaPos), elapsedEciTime);
  llaPos.setOrientation(355.92127 * simCore::DEG2RAD, -1.63579 * simCore::DEG2RAD, 0. * simCore::DEG2RAD);
  llaPos.setVelocity(-523.79150391, 7345.51757813, -210.30409241);

  const double dEcefPos[] = {7341511.73022153, 577837.16567499, -377547.31600009};
  simCore::Coordinate ecefPos(simCore::COORD_SYS_ECEF, simCore::Vec3(dEcefPos), elapsedEciTime);
  ecefPos.setOrientation(292.30864247 * simCore::DEG2RAD, -85.71737838 * simCore::DEG2RAD, 72.02839783 * simCore::DEG2RAD);
  ecefPos.setVelocity(208.83509767, -508.9744036, 7346.6010372);

  // Position: NGA GoldData 6.3, WGS84, rectangular line 460, geodetic line 461
  // http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
  double dLlaPos3[] = {44.0 * simCore::DEG2RAD, 0.0 * simCore::DEG2RAD, 100.0};
  const simCore::Coordinate llaPos3(simCore::COORD_SYS_LLA, simCore::Vec3(dLlaPos3));

  // Position: NGA GoldData 6.3, WGS84, rectangular line 460, geodetic line 461
  double dEcefPos3[] = {4595548.289592, 0.0, 4408161.078281};
  const simCore::Coordinate ecefPos3(simCore::COORD_SYS_ECEF, simCore::Vec3(dEcefPos3));

  double dXeastPos[] = {0., 0., 995807.83470784};
  simCore::Coordinate xEastPos(simCore::COORD_SYS_XEAST, simCore::Vec3(dXeastPos), elapsedEciTime);
  xEastPos.setOrientation(355.92127 * simCore::DEG2RAD, -1.63579 * simCore::DEG2RAD, 0. * simCore::DEG2RAD);
  xEastPos.setVelocity(-523.79150391, 7345.51757813, -210.30409241);

  const double dEciPos[] = {-3137060.76019948, -6662622.61139202, -377547.31600009};
  simCore::Coordinate eciPos(simCore::COORD_SYS_ECI, simCore::Vec3(dEciPos), elapsedEciTime);
  eciPos.setOrientation(3.0123530924664998, -1.4960504789088027, 1.2571326970698389);
  eciPos.setVelocity(-59.71753316, -157.85427509, 7346.6010372);

  double dEnuPos[] = {0., 0., 995807.83470784};
  simCore::Coordinate enuPos(simCore::COORD_SYS_ENU, simCore::Vec3(dEnuPos), elapsedEciTime);
  enuPos.setOrientation(355.92127 * simCore::DEG2RAD, -1.63579 * simCore::DEG2RAD, 0. * simCore::DEG2RAD);
  enuPos.setVelocity(-523.79150391, 7345.51757813, -210.30409241);

  double dNedPos[] = {0., 0., -995807.83470784};
  simCore::Coordinate nedPos(simCore::COORD_SYS_NED, simCore::Vec3(dNedPos), elapsedEciTime);
  nedPos.setOrientation(355.92127 * simCore::DEG2RAD, -1.63579 * simCore::DEG2RAD, 0. * simCore::DEG2RAD);
  nedPos.setVelocity(7345.51757813, -523.79150391, 210.30409241);

  double dNwuPos[] = {0., 0., 995807.83470784};
  simCore::Coordinate nwuPos(simCore::COORD_SYS_NWU, simCore::Vec3(dNwuPos), elapsedEciTime);
  nwuPos.setOrientation(355.92127 * simCore::DEG2RAD, -1.63579 * simCore::DEG2RAD, 0. * simCore::DEG2RAD);
  nwuPos.setVelocity(7345.51757813, 523.79150391, -210.30409241);

  // Convert to ECI
  simCore::Coordinate eciFromLla;
  cc.convert(llaPos, eciFromLla, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromLla.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromLla, eciPos));

  simCore::Coordinate eciFromXeast;
  cc.convert(xEastPos, eciFromXeast, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromXeast.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromXeast, eciPos));

  simCore::Coordinate eciFromEcef;
  cc.convert(ecefPos, eciFromEcef, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromEcef.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromEcef, eciPos));

  simCore::Coordinate eciFromEnu;
  cc.convert(enuPos, eciFromEnu, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromEnu.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromEnu, eciPos));

  simCore::Coordinate eciFromNed;
  cc.convert(nedPos, eciFromNed, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromNed.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromNed, eciPos));

  simCore::Coordinate eciFromNwu;
  cc.convert(nwuPos, eciFromNwu, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromNwu.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromNwu, eciPos));

  // Convert from ECI
  simCore::Coordinate llaFromEci;
  cc.convert(eciPos, llaFromEci, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEci.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromEci, llaPos));

  simCore::Coordinate xEastFromEci;
  cc.convert(eciPos, xEastFromEci, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromEci.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromEci, xEastPos));

  simCore::Coordinate ecefFromEci;
  cc.convert(eciPos, ecefFromEci, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromEci.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromEci, ecefPos));

  simCore::Coordinate enuFromEci;
  cc.convert(eciPos, enuFromEci, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromEci.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromEci, enuPos));

  simCore::Coordinate nedFromEci;
  cc.convert(eciPos, nedFromEci, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromEci.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromEci, nedPos));

  simCore::Coordinate nwuFromEci;
  cc.convert(eciPos, nwuFromEci, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromEci.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromEci, nwuPos));

  // Convert from LLA
  simCore::Coordinate xEastFromLla;
  cc.convert(llaPos, xEastFromLla, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromLla.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromLla, xEastPos));

  simCore::Coordinate ecefFromLla;
  cc.convert(llaPos, ecefFromLla, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromLla.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromLla, ecefPos));

  simCore::Coordinate ecefFromLla3;
  cc.convert(llaPos3, ecefFromLla3, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromLla3.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromLla3, ecefPos3));

  simCore::Coordinate enuFromLla;
  cc.convert(llaPos, enuFromLla, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromLla.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromLla, enuPos));

  simCore::Coordinate nedFromLla;
  cc.convert(llaPos, nedFromLla, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromLla.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromLla, nedPos));

  simCore::Coordinate nwuFromLla;
  cc.convert(llaPos, nwuFromLla, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromLla.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromLla, nwuPos));

  // Convert from ECEF
  simCore::Coordinate xEastFromEcef;
  cc.convert(ecefPos, xEastFromEcef, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromEcef.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromEcef, xEastPos));

  simCore::Coordinate llaFromEcef;
  cc.convert(ecefPos, llaFromEcef, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEcef.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromEcef, llaPos));

  simCore::Coordinate llaFromEcef3;
  cc.convert(ecefPos3, llaFromEcef3, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEcef3.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromEcef3, llaPos3));// , 1e-5, 5e-3));

  simCore::Coordinate enuFromEcef;
  cc.convert(ecefPos, enuFromEcef, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromEcef.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromEcef, enuPos));

  simCore::Coordinate nedFromEcef;
  cc.convert(ecefPos, nedFromEcef, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromEcef.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromEcef, nedPos));

  simCore::Coordinate nwuFromEcef;
  cc.convert(ecefPos, nwuFromEcef, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromEcef.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromEcef, nwuPos));

  // Convert from LLA -> ECEF -> LLA -> ECEF, using NGA Gold Data
  simCore::Coordinate ecefFromLla1;
  cc.convert(llaPos3, ecefFromLla1, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromLla1.coordinateSystem() == simCore::COORD_SYS_ECEF);

  simCore::Coordinate llaFromEcef2;
  cc.convert(ecefFromLla1, llaFromEcef2, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEcef2.coordinateSystem() == simCore::COORD_SYS_LLA);

  cc.convert(llaFromEcef2, ecefFromLla1, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromLla1.coordinateSystem() == simCore::COORD_SYS_ECEF);

  cc.convert(ecefFromLla1, llaFromEcef2, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEcef2.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromEcef2, llaPos3));// , 1e-5, 1e-2));


  // Convert from X-East
  simCore::Coordinate ecefFromXeast;
  cc.convert(xEastPos, ecefFromXeast, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromXeast.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromXeast, ecefPos));

  simCore::Coordinate llaFromXeast;
  cc.convert(xEastPos, llaFromXeast, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromXeast.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromXeast, llaPos));

  simCore::Coordinate enuFromXeast;
  cc.convert(xEastPos, enuFromXeast, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromXeast.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromXeast, enuPos));

  simCore::Coordinate nedFromXeast;
  cc.convert(xEastPos, nedFromXeast, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromXeast.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromXeast, nedPos));

  simCore::Coordinate nwuFromXeast;
  cc.convert(xEastPos, nwuFromXeast, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromXeast.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromXeast, nwuPos));

  // Convert from ENU
  simCore::Coordinate ecefFromEnu;
  cc.convert(enuPos, ecefFromEnu, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromEnu.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromEnu, ecefPos));

  simCore::Coordinate llaFromEnu;
  cc.convert(enuPos, llaFromEnu, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEnu.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromEnu, llaPos));

  simCore::Coordinate xEastFromEnu;
  cc.convert(enuPos, xEastFromEnu, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromEnu.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromEnu, xEastPos));

  simCore::Coordinate nedFromEnu;
  cc.convert(enuPos, nedFromEnu, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromEnu.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromEnu, nedPos));

  simCore::Coordinate nwuFromEnu;
  cc.convert(enuPos, nwuFromEnu, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromEnu.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromEnu, nwuPos));

  // Convert from NED
  simCore::Coordinate ecefFromNed;
  cc.convert(nedPos, ecefFromNed, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromNed.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromNed, ecefPos));

  simCore::Coordinate llaFromNed;
  cc.convert(nedPos, llaFromNed, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromNed.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromNed, llaPos));

  simCore::Coordinate xEastFromNed;
  cc.convert(nedPos, xEastFromNed, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromNed.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromNed, xEastPos));

  simCore::Coordinate enuFromNed;
  cc.convert(nedPos, enuFromNed, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromNed.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromNed, enuPos));

  simCore::Coordinate nwuFromNed;
  cc.convert(nedPos, nwuFromNed, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(nwuFromNed.coordinateSystem() == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(nwuFromNed, nwuPos));

  // Convert from NWU
  simCore::Coordinate ecefFromNwu;
  cc.convert(nwuPos, ecefFromNwu, simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(ecefFromNwu.coordinateSystem() == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(almostEqualCoord(ecefFromNwu, ecefPos));

  simCore::Coordinate llaFromNwu;
  cc.convert(nwuPos, llaFromNwu, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromNwu.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqualCoord(llaFromNwu, llaPos));

  simCore::Coordinate xEastFromNwu;
  cc.convert(nwuPos, xEastFromNwu, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(xEastFromNwu.coordinateSystem() == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(xEastFromNwu, xEastPos));

  simCore::Coordinate enuFromNwu;
  cc.convert(nwuPos, enuFromNwu, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(enuFromNwu.coordinateSystem() == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(enuFromNwu, enuPos));

  simCore::Coordinate nedFromNwu;
  cc.convert(nwuPos, nedFromNwu, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(nedFromNwu.coordinateSystem() == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(nedFromNwu, nedPos));


  // more tests of ECI implementation:
  // at zero time, conversion to ECI gives us original ECEF position and orientation, but different velocity
  llaPos.setElapsedEciTime(0.0);
  cc.convert(llaPos, eciFromLla, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromLla.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqual(eciFromLla.position(), ecefPos.position(), 1e-5, 1e-5));
  rv += SDK_ASSERT(almostEqual(eciFromLla.orientation(), ecefPos.orientation(), 1e-5, 1e-5));

  // test negative elapsedEciTime
  // this eci position is 20 seconds of earth rotation different than dEcefPos
  llaPos.setElapsedEciTime(-20.0);
  const double dEciPosNegElapsed[] = { 7342346.6532643940, 567129.52516404376, -377547.31600008655 };
  simCore::Coordinate eciPosNegElapsed(simCore::COORD_SYS_ECI, simCore::Vec3(dEciPosNegElapsed), -20.0);
  eciPosNegElapsed.setOrientation(5.1002898201157798, -1.4960504789036126, 1.2571326969871650);
  eciPosNegElapsed.setVelocity(166.73683784194071, 26.133940306514194, 7346.6010372092842);
  cc.convert(llaPos, eciFromLla, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromLla.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqualCoord(eciFromLla, eciPosNegElapsed));

  std::cout << std::endl << "Coordinate converter test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

int testEcefLlaCenterOfEarth()
{
  int rv = 0;
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(-2.95192266, 4.50036968, 0.);

  {
    // ecef 000 converts to lla center of earth, from north pole, and converts back to ecef 000.
    const simCore::Vec3 ecef(0.0, 0.0, 0.0);
    const simCore::Vec3 lla(M_PI_2, 0., -simCore::WGS_B);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla));
  }

  // data from SIM-13615; points near center-of-earth
  // test verifies that two-iteration calculation of lla from ecef for such points
  // produce acceptable results to the extent that ecef-> lla generally matches lla->ecef
  {
    // {-4105.1847617285403, 0.0000000000000000, -1099.9809416857129 }
    const simCore::Vec3 ecef(-4105.1847617285403, 0.0000000000000000, -1099.9809416857129);
    // lla obtained by calculating from ecef using two-iterations
    const simCore::Vec3 lla(0.028506163559486097, 3.1415926535897931, -6374047.4916362716);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla), 2e-3, 6e-2);
  }
  {
    // {-4225.9254900146734, 0.0000000000000000, 1132.3333223235286}
    const simCore::Vec3 ecef(-4225.9254900146734, 0.0000000000000000, 1132.3333223235286);
    // lla obtained by calculating from ecef using two-iterations
    const simCore::Vec3 lla(-0.029437050317393673, 3.1415926535897931, -6373927.7387804883);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla), 2e-3, 6e-2);
  }
  {
    // ecef {6732.9272022769273, -4177.0642522461094, -1965.4578560947064}
    // single iteration conv to lla:
    // lla {0.040228923517666403, -0.55527991376127517, -6370264.5317185409}
    // two iteration:
    // lla(0.056567845937660635, -0.55527991376127517, -6370269.1599203711);

    simCore::Vec3 ecef(6732.9272022769273, -4177.0642522461094, -1965.4578560947064);
    // lla obtained by calculating from ecef using two-iterations
    simCore::Vec3 lla(0.056567845937660635, -0.55527991376127517, -6370269.1599203711);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla), 5e-3, 1e-1);
  }
  {
    // {-4105.1847617285403, 1099.9809416857131, 2.5137006891796735e-13}
    const simCore::Vec3 ecef(-4105.1847617285403, 1099.9809416857131, 2.5137006891796735e-13);
    // lla obtained by calculating from ecef using two-iterations
    const simCore::Vec3 lla(-6.5379787960762509e-18, 2.8797932657906440, -6373886.9999999944);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla), 2e-3, 6e-2);
  }
  {
    // {-4105.1849537113685, -1099.9802251957922, 2.5137008067352520e-13}
    const simCore::Vec3 ecef(-4105.1849537113685, -1099.9802251957922, 2.5137008067352520e-13);
    // lla obtained by calculating from ecef using two-iterations
    const simCore::Vec3 lla(-6.5379791018309832e-18, -2.8797934403235690, -6373886.9999999944);
    rv += ecefLlaEcef(cc, simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef),
      simCore::Coordinate(simCore::COORD_SYS_LLA, lla), 2e-3, 6e-2);
  }
  return rv;
}

//===========================================================================
int testExternalDataECI()
{
  int rv = 0;
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(0., 0., 0.);
  const double ft2m = 0.3048;
  const double elapsedEciTime = 2.54571904E+01;

  const double dLlaPos[] = {2.20829071E+01 * simCore::DEG2RAD, -1.59794751E+02 * simCore::DEG2RAD, 1.41305717E+04 * ft2m};
  simCore::Coordinate llaPos(simCore::COORD_SYS_LLA, simCore::Vec3(dLlaPos), elapsedEciTime);

  const double dEciPos[] = {-1.82057147E+07 * ft2m, -6.73869347E+06 * ft2m, 7.82329851E+06 * ft2m};
  simCore::Coordinate eciPos(simCore::COORD_SYS_ECI, simCore::Vec3(dEciPos), elapsedEciTime);

  simCore::Coordinate eciFromLla;
  cc.convert(llaPos, eciFromLla, simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(eciFromLla.coordinateSystem() == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(almostEqual(eciFromLla.position(), eciPos.position(), 0.15, 0.15));

  simCore::Coordinate llaFromEci;
  cc.convert(eciPos, llaFromEci, simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(llaFromEci.coordinateSystem() == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(almostEqual(llaFromEci.position(), llaPos.position(), 1e-5, 0.15));

  std::cout << std::endl << "External ECI data test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

int testGtpRotation()
{
  // sin(45 degrees)
  static const double SIN45 = 0.70710678118654752440084436210485;
  // 45 degrees in radians
  static const double D45_RAD = 45 * simCore::DEG2RAD;

  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(0., 0., 0.);
  int rv = 0;

  // xEastPos is 1 unit north, looking north, moving north
  const simCore::Coordinate xEastPos(simCore::COORD_SYS_XEAST, simCore::Vec3(0, 1, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 1, 0));

  // Validate that 0,0,0 does nothing
  simCore::Coordinate gtpPos;
  cc.setTangentPlaneOffsets(0, 0, 0.0);
  cc.convert(xEastPos, gtpPos, simCore::COORD_SYS_GTP);
  rv += SDK_ASSERT(almostEqualCoord(xEastPos, gtpPos));

  // Validate the position offsets; note that TP offsets
  cc.setTangentPlaneOffsets(1, 3, 0.0);
  cc.convert(xEastPos, gtpPos, simCore::COORD_SYS_GTP);
  // For X: 0 is -1 from reference 1
  // For Y: 1 is -2 from reference 3
  simCore::Coordinate expectation(simCore::COORD_SYS_GTP, simCore::Vec3(-1, -2, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 1, 0));
  rv += SDK_ASSERT(almostEqualCoord(gtpPos, expectation));

  // GTP defines the reference frame with a 45 degree rotation; validate
  cc.setTangentPlaneOffsets(0, 0, D45_RAD);
  cc.convert(xEastPos, gtpPos, simCore::COORD_SYS_GTP);
  // For position: position is now left and up, relative to the TP's rotation
  // For angle: north is now -45 degrees from the GTP reference angle (315)
  expectation = simCore::Coordinate(simCore::COORD_SYS_GTP, simCore::Vec3(-SIN45, SIN45, 0), simCore::Vec3(315*simCore::DEG2RAD, 0, 0), simCore::Vec3(-SIN45, SIN45, 0));
  rv += SDK_ASSERT(almostEqualCoord(gtpPos, expectation));

  // Should be able to reverse all the math and get failures now

  // Start with the simple case
  simCore::Coordinate reversed;
  cc.setTangentPlaneOffsets(0, 0, 0.0);
  gtpPos = simCore::Coordinate(simCore::COORD_SYS_GTP, simCore::Vec3(0, 1, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 1, 0));
  cc.convert(gtpPos, reversed, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(reversed, xEastPos));

  // Test the position offsets
  cc.setTangentPlaneOffsets(1, 3, 0.0);
  gtpPos = simCore::Coordinate(simCore::COORD_SYS_GTP, simCore::Vec3(-1, -2, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 1, 0));
  cc.convert(gtpPos, reversed, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(reversed, xEastPos));

  // Test the rotated values
  cc.setTangentPlaneOffsets(0, 0, D45_RAD);
  gtpPos = simCore::Coordinate(simCore::COORD_SYS_GTP, simCore::Vec3(-SIN45, SIN45, 0), simCore::Vec3(315*simCore::DEG2RAD, 0, 0), simCore::Vec3(-SIN45, SIN45, 0));
  cc.convert(gtpPos, reversed, simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(almostEqualCoord(reversed, xEastPos));

  std::cout << std::endl << "GTP rotation test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

// Test scaled flat Earth (NWU, ENU, NED) systems at the pole, JIRA issue SIMDIS-2285
int testScaledFlatEarthPole()
{
  // A reference origin at or near either pole results in a degenerate case
  // for the scaled flat Earth systems that prevents any conversion of values
  // in the Y axis / longitude due to the cosine term for calculating the
  // radius at the reference latitude. As one moves closer to the poles,
  // there is less "space" in which to perform the scaled conversion.
  int rv = 0;

  // test north pole origin
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(90., 0., 0.);
  simCore::Coordinate ecefPos;
  simCore::Coordinate llaPos;
  simCore::Coordinate sfePos;

  // X axis position data from JIRA issue SIMDIS-2285
  const simCore::Coordinate xPos(simCore::COORD_SYS_NWU, simCore::Vec3(5556, 0, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(5.144, 0, 0));
  ecefPos.setCoordinateSystem(simCore::COORD_SYS_ECEF);
  llaPos.setCoordinateSystem(simCore::COORD_SYS_LLA);
  sfePos.setCoordinateSystem(simCore::COORD_SYS_NWU);

  // Validate conversion to and from ECEF fails as expected due to degenerate origin at pole
  if (cc.convert(xPos, ecefPos, simCore::COORD_SYS_ECEF) == 0)
    rv++;
  if (cc.convert(ecefPos, sfePos, simCore::COORD_SYS_NWU) == 0)
    rv++;

  // Validate conversion to and from LLA fails as expected due to degenerate origin at pole
  if (cc.convert(xPos, llaPos, simCore::COORD_SYS_LLA) == 0)
    rv++;
  if (cc.convert(llaPos, sfePos, simCore::COORD_SYS_NWU) == 0)
    rv++;

  // test south pole origin
  simCore::CoordinateConverter ccSouthPole;
  ccSouthPole.setReferenceOriginDegrees(-90., 0., 0.);

  // Y axis data from JIRA issue SIMDIS-2285
  const simCore::Coordinate yPos(simCore::COORD_SYS_NWU, simCore::Vec3(0, 5556, 0), simCore::Vec3(0, 0, 0), simCore::Vec3(5.144, 0, 0));

  // Validate conversions to and from ECEF fails as expected due to degenerate origin at pole
  if (ccSouthPole.convert(yPos, ecefPos, simCore::COORD_SYS_ECEF) == 0)
    rv++;
  if (ccSouthPole.convert(ecefPos, sfePos, simCore::COORD_SYS_NWU) == 0)
    rv++;

  // Validate conversions to and from LLA fails as expected due to degenerate origin at pole
  if (ccSouthPole.convert(yPos, llaPos, simCore::COORD_SYS_LLA) == 0)
    rv++;
  if (ccSouthPole.convert(llaPos, sfePos, simCore::COORD_SYS_NWU) == 0)
    rv++;

  std::cout << std::endl << "Scaled Flat Earth at Pole test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

// Test conversions to/from scaled flat Earth (NWU, ENU, NED) systems
int testScaledFlatEarth()
{
  // Data based on TestData testOffset.asi example
  int rv = 0;
  simCore::CoordinateConverter cc;
  cc.setReferenceOriginDegrees(22.119439197, -159.91949881, 0.);
  simCore::Coordinate enuPos;
  simCore::Coordinate llaPos;
  simCore::Coordinate sfePos;

  // Platform 10X
  const simCore::Coordinate pos10x(simCore::COORD_SYS_NED, simCore::Vec3(-200, 0, -100), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10x, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10x, sfePos));

  // Platform 10Y
  const simCore::Coordinate pos10y(simCore::COORD_SYS_NED, simCore::Vec3(0, 0, -100), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10y, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10y, sfePos));

  // Platform 10Z
  const simCore::Coordinate pos10z(simCore::COORD_SYS_NED, simCore::Vec3(200, 0, -100), simCore::Vec3(0, 0, 0), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10z, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10z, sfePos));

  // Platform 10X 45y
  const simCore::Coordinate pos10x45y(simCore::COORD_SYS_NED, simCore::Vec3(-200, 200, -100), simCore::Vec3(simCore::DEG2RAD*45, 0, 0), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10x45y, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10x45y, sfePos));

  // Platform 10X 45y 45p
  const simCore::Coordinate pos10x45y45p(simCore::COORD_SYS_NED, simCore::Vec3(-200, 400, -100), simCore::Vec3(simCore::DEG2RAD*45, simCore::DEG2RAD*45, 0), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10x45y45p, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10x45y45p, sfePos));

  // Platform 10X 45y 45p 45r
  const simCore::Coordinate pos10x45y45p45r(simCore::COORD_SYS_NED, simCore::Vec3(200, 600, -100), simCore::Vec3(simCore::DEG2RAD*45, simCore::DEG2RAD*45, simCore::DEG2RAD*45), simCore::Vec3(0, 0, 0));

  // Validate conversion to and from LLA works
  rv += cc.convert(pos10x45y45p45r, llaPos, simCore::COORD_SYS_LLA);
  rv += cc.convert(llaPos, sfePos, simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(almostEqualCoord(pos10x45y45p45r, sfePos));


  // Data from Case1.asi
  simCore::CoordinateConverter ccCase1;
  ccCase1.setReferenceOriginDegrees(40.664165, -72.664444, 0.);

  const simCore::Coordinate pos11(simCore::COORD_SYS_ENU, simCore::Vec3(-1432.22570801, -1904.70373535, 4172.20019531), simCore::Vec3(simCore::DEG2RAD*67.99902, 0, 0), simCore::Vec3(152.01904297, 61.42270279, 0.));
  // Validate conversion to and from LLA works
  rv += ccCase1.convert(pos11, llaPos, simCore::COORD_SYS_LLA);
  rv += ccCase1.convert(llaPos, sfePos, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(pos11, sfePos));

  const simCore::Coordinate pos12(simCore::COORD_SYS_ENU, simCore::Vec3(-109.4115982, -380.52468872, 1128.29882813), simCore::Vec3(simCore::DEG2RAD*199.70001, simCore::DEG2RAD*69.48, simCore::DEG2RAD*167.84), simCore::Vec3(-75.21276093, -210.0610199, 596.12738037));
  // Validate conversion to and from LLA works
  rv += ccCase1.convert(pos12, llaPos, simCore::COORD_SYS_LLA);
  rv += ccCase1.convert(llaPos, sfePos, simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(almostEqualCoord(pos12, sfePos));


  // Data from UpdateData AQM asi
  simCore::CoordinateConverter ccAqm;
  ccAqm.setReferenceOriginDegrees(26, 161, 0.);

  const simCore::Coordinate posa(simCore::COORD_SYS_NWU, simCore::Vec3(-186947.0635, -38850.85735, 43550.5247), simCore::Vec3(2.316869806, 0.364446305, 0), simCore::Vec3(-1171.866943, 0.000000, -453.903870));
  // Validate conversion to and from LLA works
  rv += ccAqm.convert(posa, llaPos, simCore::COORD_SYS_LLA);
  rv += ccAqm.convert(llaPos, sfePos, simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(almostEqualCoord(posa, sfePos));


  // SIM-11596, data fabricated to demonstrate issue
  simCore::CoordinateConverter ccsim11596;
  ccsim11596.setReferenceOriginDegrees(45, 161, 0.);
  const simCore::Coordinate sim11596(simCore::COORD_SYS_LLA, simCore::Vec3(-45.1 * simCore::DEG2RAD, -162. * simCore::DEG2RAD, 100.));
  // Validate conversion from and back to LLA works
  rv += ccsim11596.convert(sim11596, enuPos, simCore::COORD_SYS_ENU);
  rv += ccsim11596.convert(enuPos, sfePos, simCore::COORD_SYS_LLA);

  // position-only comparison
  rv += SDK_ASSERT(simCore::areAnglesEqual(sim11596.lat(), sfePos.lat()));
  rv += SDK_ASSERT(simCore::areAnglesEqual(sim11596.lon(), sfePos.lon()));
  rv += SDK_ASSERT(simCore::areEqual(sim11596.alt(), sfePos.alt()));


  std::cout << std::endl << "Scaled Flat Earth test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

int testStringFunctions()
{
  int rv = 0;

  // To-string testing
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_NED) == "Topo_NED");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_NWU) == "Topo_NWU");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_ENU) == "Topo_ENU");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_LLA) == "LLA_DD");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_ECEF) == "ECEF_WGS84");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_ECI) == "ECI_WGS84");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_XEAST) == "TangentPlane_XEast");
  rv += SDK_ASSERT(simCore::coordinateSystemToString(simCore::COORD_SYS_GTP) == "TangentPlane_Generic");

  // From-string testing
  simCore::CoordinateSystem coordSys;
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("Topo_NED", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_NED);
  // Test capitalization
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("topo_ned", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_NED);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("TOPO_NED", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_NED);

  rv += SDK_ASSERT(simCore::coordinateSystemFromString("Topo_NWU", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_NWU);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("Topo_ENU", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_ENU);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("LLA_DD", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("ECEF_WGS84", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_ECEF);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("ECI_WGS84", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_ECI);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("TangentPlane_XEast", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_XEAST);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("TangentPlane_Generic", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_GTP);

  // Test the oddball LLA legacy strings
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("LLA_DMD", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_LLA);
  rv += SDK_ASSERT(simCore::coordinateSystemFromString("LLA_DMS", coordSys) == 0);
  rv += SDK_ASSERT(coordSys == simCore::COORD_SYS_LLA);

  std::cout << std::endl << "String Functions test case: ";
  std::cout << (rv==0 ? "PASSED" : "FAILED") << std::endl;
  return rv;
}

}

//===========================================================================
int CoordConvertLibTest(int _argc_, char *_argv_[])
{
  simCore::checkVersionThrow();
  vTestCases testCases;
  vTestCases::iterator testCaseIterator;
  simCore::CoordinateConverter coordConvertor;
  double outputPosition[3];
  double outputEul[3];
  double outputVelocity[3];
  double outputAcc[3];
  int rv = 0;
  coordConvertor.setReferenceOrigin(0, 0, 0);
  createTestCases(&testCases);
  for (testCaseIterator = testCases.begin();
       testCaseIterator != testCases.end();
       ++testCaseIterator)
  {
    TestCase* temp = *testCaseIterator;
    if (temp != nullptr)
    {
      simCore::Coordinate inTSPI(temp->InputCoordinateSystem_,
          temp->InputPosition_, temp->InputEul_, temp->InputVelocity_, temp->InputAcc_);
      simCore::Coordinate outTSPI;
      coordConvertor.convert(inTSPI, outTSPI, temp->OutputCoordinateSystem_);
      outTSPI.position().toD3(outputPosition);
      outTSPI.orientation().toD3(outputEul);
      outTSPI.velocity().toD3(outputVelocity);
      outTSPI.acceleration().toD3(outputAcc);
//                                    inEul,      &outEul,
//                                    inVel,      &outVel,
//                                    inAcc,      &outAcc);
      std::cout.precision(12);
      std::cout << "\ncaseNumber: " << temp->UniqueID_ << std::endl;
      std::cout << "OutputCoordinateSystem: " << (int)temp->OutputCoordinateSystem_ << std::endl;
      if (temp->InputCoordinateSystem_ == simCore::COORD_SYS_LLA)
        std::cout << "InputPos: " << simCore::RAD2DEG*temp->InputPosition_[0] << " " << simCore::RAD2DEG*temp->InputPosition_[1] << " " << temp->InputPosition_[2] << std::endl;
      else
        std::cout << "InputPos:  " << temp->InputPosition_[0] << " " << temp->InputPosition_[1] << " " << temp->InputPosition_[2] << std::endl;
      if (temp->OutputCoordinateSystem_ == simCore::COORD_SYS_LLA)
        std::cout << "outputPos: " << simCore::RAD2DEG*outputPosition[0] << " " << simCore::RAD2DEG*outputPosition[1] << " " << outputPosition[2] << std::endl;
      else
        std::cout << "outputPos: " << outputPosition[0] << " " << outputPosition[1] << " " << outputPosition[2] << std::endl;
      std::cout << "InputEul:  " << temp->InputEul_[0] << " " << temp->InputEul_[1] << " " << temp->InputEul_[2] << std::endl;
      std::cout << "outputEul: " << outputEul[0] << " " << outputEul[1] << " " << outputEul[2] << std::endl;
      // position check
      if (temp->CheckPosition_)
        rv += checkValues(temp->UniqueID_, "position", simCore::Vec3(outputPosition), temp->CorrectOutputPosition_, 8e-7);
      // orientation check
      if (temp->CheckEul_)
        rv += checkValues(temp->UniqueID_, "orientation", simCore::Vec3(outputEul), temp->CorrectOutputEul_, 1e-4);
      // velocity check
      if (temp->CheckVelocity_)
        rv += checkValues(temp->UniqueID_, "velocity", simCore::Vec3(outputVelocity), temp->CorrectOutputVelocity_, 1e-4);
      // acceleration check
      if (temp->CheckAcc_)
        rv += checkValues(temp->UniqueID_, "acceleration", simCore::Vec3(outputAcc), temp->CorrectOutputAcc_, 1e-4);
    }
    delete *testCaseIterator;
  }
  rv += testGtp();
  rv += testCC();
  rv += testEcefLlaCenterOfEarth();
  rv += testExternalDataECI();
  rv += testGtpRotation();
  rv += testScaledFlatEarthPole();
  rv += testScaledFlatEarth();
  rv += testStringFunctions();
  return rv;
}
