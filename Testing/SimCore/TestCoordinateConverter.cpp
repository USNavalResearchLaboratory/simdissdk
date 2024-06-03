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
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <cmath>

using namespace simCore;

typedef std::vector<class TestCase*> vTestCases;

static void createTestCases(vTestCases* testCases);

//-----------------------------------------------------------------------------
static const double CU_DEG2RAD  = 0.017453292519943295;   /**< Multiply degree, to convert to radians */
static const double CU_RAD2DEG  = 57.295779513082323;     /**< Multiply radians to convert to degrees */

//-----------------------------------------------------------------------------
class TestCase
{
  public:
    int UniqueID_;

    bool CheckPosition_;
    bool CheckEul_;
    bool CheckVelocity_;
    bool CheckAcc_;

    Vec3 InputPosition_;
    Vec3 InputEul_;
    Vec3 InputVelocity_;
    Vec3 InputAcc_;

    CoordinateSystem InputCoordinateSystem_;
    CoordinateSystem OutputCoordinateSystem_;

    Vec3 CorrectOutputPosition_;
    Vec3 CorrectOutputEul_;
    Vec3 CorrectOutputVelocity_;
    Vec3 CorrectOutputAcc_;

    TestCase(int uniqueID, CoordinateSystem inputCoordinateSystem, CoordinateSystem outputCoordinateSystem)
      : UniqueID_(uniqueID),
        CheckPosition_(false),
        CheckEul_(false),
        CheckVelocity_(false),
        CheckAcc_(false),
        InputCoordinateSystem_(inputCoordinateSystem),
        OutputCoordinateSystem_(outputCoordinateSystem)
    {
    }

    void SetInputPosition(double a, double b, double c)       {CheckPosition_ = true; InputPosition_.set(a, b, c);}
    void SetInputPositionLLADeg(double a, double b, double c) {CheckPosition_ = true; InputPosition_.set((a * CU_DEG2RAD), (b * CU_DEG2RAD), c);}
    void SetInputEul(double a, double b, double c)            {CheckEul_ = true;      InputEul_.set((a * CU_DEG2RAD), (b * CU_DEG2RAD), (c * CU_DEG2RAD));}
    void SetInputVelocity(double a, double b, double c)       {CheckVelocity_ = true; InputVelocity_.set(a, b, c);}
    void SetInputAcc(double a, double b, double c)            {CheckAcc_ = true;      InputAcc_.set(a, b, c);}

    void SetCorrectOutputPosition(double a, double b, double c)       {CorrectOutputPosition_.set(a, b, c);}
    void SetCorrectOutputPositionLLADeg(double a, double b, double c) {CorrectOutputPosition_.set((a * CU_DEG2RAD), (b * CU_DEG2RAD), c);}
    void SetCorrectOutputEul(double a, double b, double c)            {CorrectOutputEul_.set((a * CU_DEG2RAD), (b * CU_DEG2RAD), (c * CU_DEG2RAD));}
    void SetCorrectOutputVelocity(double a, double b, double c)       {CorrectOutputVelocity_.set(a, b, c);}
    void SetCorrectOutputAcc(double a, double b, double c)            {CorrectOutputAcc_.set(a, b, c);}
};

//-----------------------------------------------------------------------------
static bool almostEqual(const Vec3& value1, const Vec3& value2, double epsilon)
{
  if (!areEqual(value1[0], value2[0], epsilon))
    return false;

  if (!areEqual(value1[1], value2[1], epsilon))
    return false;

  if (!areEqual(value1[2], value2[2], epsilon))
    return false;

  return true;
}

static int checkValues(int uniqueID, const char* whichTest,
                       const Vec3& result, const Vec3& correctValue, double epsilon)
{
  std::string errorString;

  if (!almostEqual(result, correctValue, epsilon))
  {
    std::ostringstream tempString;
    tempString << whichTest << std::endl;
    tempString << "  result values  = ("
               << result[0] << ", "
               << result[1] << ", "
               << result[2] << ")\n  correct values = (";
    tempString << correctValue[0] << ", ";
    tempString << correctValue[1] << ", ";
    tempString << correctValue[2] << ")";

    if (errorString.empty())
    {
      errorString = tempString.str();
    }
    else
    {
      errorString.append(", ");
      errorString.append(tempString.str());
    }
  }

  if (!errorString.empty())
  {
    std::cerr << "Test Failure:  UniqueID(" << uniqueID << "):  " << errorString.c_str() << "\n";
    return 1; // fail
  }

  return 0; // success
}

//-----------------------------------------------------------------------------
// drive the test
int CoordConvertLibTest(int _argc_, char *_argv_[])
{
  vTestCases testCases;
  createTestCases(&testCases);

  CoordinateConverter coordConvertor;
  coordConvertor.setReferenceOrigin(0, 0, 0);

  // avoid loop overhead
  Vec3 outputPosition;
  Vec3 outputEul;
  Vec3 outputVelocity;
  Vec3 outputAcc;

  int rv = 0; // return code, live out from loop
  for (vTestCases::iterator testCaseIterator = testCases.begin();
      testCaseIterator != testCases.end();
      ++testCaseIterator)
  {
    TestCase* temp = *testCaseIterator;
    if (!temp)
      continue;

    Coordinate inTSPI(temp->InputCoordinateSystem_, temp->InputPosition_, temp->InputEul_, temp->InputVelocity_, temp->InputAcc_);
    Coordinate outTSPI;
    coordConvertor.convert(inTSPI, &outTSPI, temp->OutputCoordinateSystem_);

    outputPosition = outTSPI.position();
    outputEul      = outTSPI.orientation();
    outputVelocity = outTSPI.velocity();
    outputAcc      = outTSPI.acceleration();

    int errCode = 0;
    // position check
    if (temp->CheckPosition_)
      errCode += checkValues(temp->UniqueID_, "position", outputPosition, temp->CorrectOutputPosition_, 1);

    // orientation check
    if (temp->CheckEul_)
      errCode += checkValues(temp->UniqueID_, "orientation", outputEul, temp->CorrectOutputEul_, 1e-4);

    // velocity check
    if (temp->CheckVelocity_)
      errCode += checkValues(temp->UniqueID_, "velocity", outputVelocity, temp->CorrectOutputVelocity_, 1e-4);

    // acceleration check
    if (temp->CheckAcc_)
      errCode += checkValues(temp->UniqueID_, "acceleration", outputAcc, temp->CorrectOutputAcc_, 1e-4);

    rv += errCode;

    if (errCode)
    {
      std::cout.precision(12);

      std::cout << std::endl << "caseNumber: " << temp->UniqueID_ << std::endl;
      std::cout << "OutputCoordinateSystem: " << (int)temp->OutputCoordinateSystem_ << std::endl;

      std::cout << "InputPos: ";
      if (temp->InputCoordinateSystem_ == COORD_SYS_LLA)
        std::cout << CU_RAD2DEG*temp->InputPosition_[0] << " " << CU_RAD2DEG*temp->InputPosition_[1] << " " << temp->InputPosition_[2] << std::endl;
      else
        std::cout << temp->InputPosition_[0] << " " << temp->InputPosition_[1] << " " << temp->InputPosition_[2] << std::endl;

      std::cout << "outputPos: ";
      if (temp->OutputCoordinateSystem_ == COORD_SYS_LLA)
        std::cout << CU_RAD2DEG*outputPosition[0] << " " << CU_RAD2DEG*outputPosition[1] << " " << outputPosition[2] << std::endl;
      else
        std::cout << outputPosition[0] << " " << outputPosition[1] << " " << outputPosition[2] << std::endl;

      std::cout << "InputEul:  " << temp->InputEul_[0] << " " << temp->InputEul_[1] << " " << temp->InputEul_[2] << std::endl;
      std::cout << "outputEul: " << outputEul[0] << " " << outputEul[1] << " " << outputEul[2] << std::endl;
    }
  }

  return rv;
}

int main(int argc, char **argv)
{
  int rv = CoordConvertLibTest(argc, argv);
  std::cout << std::endl << "Coordinate Converter test: ";
  if (rv != 0)
    std::cout << "failed.  Error code " << rv << std::endl;
  else
    std::cout << "passed." << std::endl;

  std::cout << std::endl;

  return 0;
}

//-----------------------------------------------------------------------------
//NOTE: (on putting this data in a file)
// Doing so would add one more step (file input) that could create errors
// in the test results.
//
static void createTestCases(vTestCases* testCases)
{
  if (!testCases)
    return;

  TestCase *tempTestCase;

//   // LLA to ECEF --------------------------------------------------------------
//   // Position:      near Africa
//   // Orientation:   heading north
//   // Velocity:      north at 10
//   // Acceleration:  north at 10
//   tempTestCase = new TestCase(1, eCOORD_SYS_LLA, eCOORD_SYS_ECEF);
//   tempTestCase->SetInputPositionLLADeg        (0.0, 0.0, 0.0); // LLA
//   tempTestCase->SetCorrectOutputPosition(6378137.0, 0.0, 0.0); // ECEF
//   tempTestCase->SetInputEul          (0.0,   0.0, 0.0); // LLA
//   tempTestCase->SetCorrectOutputEul(0.0, -90.0, 0.0);   // ECEF
//   tempTestCase->SetInputVelocity        (0.0, 10.0,  0.0);  // LLA
//   tempTestCase->SetCorrectOutputVelocity(0.0,  0.0, 10.0);  // ECEF
//   tempTestCase->SetInputAcc        (0.0, 10.0,  0.0); // LLA
//   tempTestCase->SetCorrectOutputAcc(0.0,  0.0, 10.0); // ECEF
//   testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:      near Africa
  // Orientation:   heading north
  // Velocity:      north at 10
  // Acceleration:  north at 10
  tempTestCase = new TestCase(2, COORD_SYS_ECEF, COORD_SYS_LLA);
  tempTestCase->SetInputPosition(6378137.0, 0.0, 0.0); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(0.0, 0.0, 0.0); // LLA
  tempTestCase->SetInputEul(0.0, -90.0, 0.0);   // ECEF
  tempTestCase->SetCorrectOutputEul(0.0,     0.0, 0.0); // LLA
  tempTestCase->SetInputVelocity(0.0,  0.0, 10.0); // ECEF
  tempTestCase->SetCorrectOutputVelocity(0.0, 10.0,  0.0); // LLA
  tempTestCase->SetInputAcc(0.0, 0.0,  10.0); // ECEF
  tempTestCase->SetCorrectOutputAcc(0.0, 10.0, 0.0);  // LLA
  testCases->push_back(tempTestCase);

//   // LLA to ECEF --------------------------------------------------------------
//   // Position:      near Africa at altitude of 10000
//   // Orientation:   heading south
//   // Velocity:      south at 10
//   // Acceleration:  south at 10
//   tempTestCase = new TestCase(3, eCOORD_SYS_LLA, eCOORD_SYS_ECEF);
//   tempTestCase->SetInputPositionLLADeg        (0.0, 0.0, 10000.0); // LLA
//   tempTestCase->SetCorrectOutputPosition(6388137.0, 0.0,     0.0); // ECEF
//   tempTestCase->SetInputEul        (180.0,   0.0, 0.0);  // LLA
//   tempTestCase->SetCorrectOutputEul  (0.0, 90.0, 180.0); // ECEF
//   // WARNING: Result could be instead, 270, 90, 90.  Both are equivalent
//   tempTestCase->SetInputVelocity        (0.0, -10.0,   0.0);  // LLA
//   tempTestCase->SetCorrectOutputVelocity(0.0,   0.0, -10.0);  // ECEF
//   tempTestCase->SetInputAcc        (0.0, -10.0,   0.0); // LLA
//   tempTestCase->SetCorrectOutputAcc(0.0,   0.0, -10.0); // ECEF
//   testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:      near Africa at altitude of 10000
  // Orientation:   heading south
  // Velocity:      south at 10
  // Acceleration:  south at 10
  tempTestCase = new TestCase(4, COORD_SYS_ECEF, COORD_SYS_LLA);
  tempTestCase->SetInputPosition(6388137.0, 0.0,     0.0); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(0.0, 0.0, 10000.0); // LLA
  tempTestCase->SetInputEul(0.0, 90.0, 180.0); // ECEF
  tempTestCase->SetCorrectOutputEul(180.0,   0.0, 0.0); // LLA
  tempTestCase->SetInputVelocity(0.0,   0.0, -10.0);  // ECEF
  tempTestCase->SetCorrectOutputVelocity(0.0, -10.0,   0.0);  // LLA
  tempTestCase->SetInputAcc(0.0,   0.0, -10.0); // ECEF
  tempTestCase->SetCorrectOutputAcc(0.0, -10.0,   0.0); // LLA
  testCases->push_back(tempTestCase);

//   // LLA to ECEF --------------------------------------------------------------
//   // Position:  North Pole
//   tempTestCase = new TestCase(5, eCOORD_SYS_LLA, eCOORD_SYS_ECEF);
//   tempTestCase->SetInputPositionLLADeg (90.0, 0.0,         0.0); // LLA
//   tempTestCase->SetCorrectOutputPosition(0.0, 0.0, 6356752.314); // ECEF
//   testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  North Pole
  tempTestCase = new TestCase(6, COORD_SYS_ECEF, COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, 6356752.314); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(90.0, 0.0,         0.0); // LLA
  testCases->push_back(tempTestCase);

//   // LLA to ECEF --------------------------------------------------------------
//   // Position:  South Pole
//   tempTestCase = new TestCase(7, eCOORD_SYS_LLA, eCOORD_SYS_ECEF);
//   tempTestCase->SetInputPositionLLADeg (-90.0, 0.0,         0.0); // LLA
//   tempTestCase->SetCorrectOutputPosition(0.0, 0.0, -6356752.314); // ECEF
//   testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  South Pole
  tempTestCase = new TestCase(8, COORD_SYS_ECEF, COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, -6356752.314); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-90.0, 0.0,         0.0); // LLA
  testCases->push_back(tempTestCase);

//   // LLA to ECEF --------------------------------------------------------------
//   // Position:  South Pole, 2.1 km above surface
//   tempTestCase = new TestCase(9, eCOORD_SYS_LLA, eCOORD_SYS_ECEF);
//   tempTestCase->SetInputPositionLLADeg (-90.0, 0.0,        21347.46602); // LLA
//   tempTestCase->SetCorrectOutputPosition(0.0, 0.0, -6378099.7802647511); // ECEF
//   testCases->push_back(tempTestCase);

  // ECEF to LLA --------------------------------------------------------------
  // Position:  South Pole, 2.1 km above surface
  tempTestCase = new TestCase(10, COORD_SYS_ECEF, COORD_SYS_LLA);
  tempTestCase->SetInputPosition(0.0, 0.0, -6378099.7802647511); // ECEF
  tempTestCase->SetCorrectOutputPositionLLADeg(-90.0, 0.0,       21347.46602); // LLA
  testCases->push_back(tempTestCase);

//   // LLA to TP --------------------------------------------------------------
//   // Position:  Dabob Bay
//   tempTestCase = new TestCase(11, eCOORD_SYS_LLA, eCOORD_SYS_TP);
//   tempTestCase->SetInputPositionLLADeg(47.79073138611, -122.8215004611, 0.0); // LLA
//   tempTestCase->SetCorrectOutputPosition (0.0, 0.0, 0.0); // TP
//   testCases->push_back(tempTestCase);

//   // TP to LLA --------------------------------------------------------------
//   // Position:  Dabob Bay
//   tempTestCase = new TestCase(12, eCOORD_SYS_TP, eCOORD_SYS_LLA);
//   tempTestCase->SetInputPosition (0.0, 0.0, 0.0); // TP
//   tempTestCase->SetCorrectOutputPositionLLADeg(47.79073138611, -122.8215004611, 0.0); // LLA
//   testCases->push_back(tempTestCase);

//   // LLA to TP --------------------------------------------------------------
//   // Position:  Dabob Bay
//   tempTestCase = new TestCase(13, eCOORD_SYS_LLA, eCOORD_SYS_TP);
//   tempTestCase->SetInputPositionLLADeg(47.0, -122.0, 0.0); // LLA
//   tempTestCase->SetCorrectOutputPosition(73634.787, 78431.019, 0.0); // TP
//   testCases->push_back(tempTestCase);

//   // TP to LLA --------------------------------------------------------------
//   // Position:  Dabob Bay
//   tempTestCase = new TestCase(14, eCOORD_SYS_TP, eCOORD_SYS_LLA);
//   tempTestCase->SetCorrectOutputPositionLLADeg(47.0, -122.0, 0.0); // LLA
//   tempTestCase->SetInputPosition(73634.787, 78431.019, 0.0); // TP
//   testCases->push_back(tempTestCase);
}

// int main(int _argc_, char *_argv_[])
// {
//   double llaPosition[3] = {(0.0 * CU_DEG2RAD), (0.0 * CU_DEG2RAD), (0.0 * CU_DEG2RAD)};
//   double llaEul[3] = {(0.0 * CU_DEG2RAD), (0.0 * CU_DEG2RAD), (0.0 * CU_DEG2RAD)};
//   double llaVelocity[3] = {0.0, 0.0, 0.0};
//   double llaAcc[3] = {0.0, 0.0, 0.0};

//   double ecefPosition[3];
//   double ecefEul[3];
//   double ecefVelocity[3];
//   double ecefAcc[3];

//   CoordConvert coordConvertor;

//   coordConvertor.SetReferenceOrigin(0, 0, 0);

//   cerr << "-------------------------------------------------------\n";
//   cerr << "llaPosition[0] = " << llaPosition[0] << "\n";
//   cerr << "llaPosition[1] = " << llaPosition[1] << "\n";
//   cerr << "llaPosition[2] = " << llaPosition[2] << "\n\n";
//   cerr << "llaEul[0] = " << llaEul[0] * CU_RAD2DEG << "\n";
//   cerr << "llaEul[1] = " << llaEul[1] * CU_RAD2DEG << "\n";
//   cerr << "llaEul[2] = " << llaEul[2] * CU_RAD2DEG << "\n\n";
//   cerr << "llaVelocity[0] = " << llaVelocity[0] << "\n";
//   cerr << "llaVelocity[1] = " << llaVelocity[1] << "\n";
//   cerr << "llaVelocity[2] = " << llaVelocity[2] << "\n\n";
//   cerr << "llaAcc[0] = " << llaAcc[0] << "\n";
//   cerr << "llaAcc[1] = " << llaAcc[1] << "\n";
//   cerr << "llaAcc[2] = " << llaAcc[2] << "\n\n";

//   coordConvertor.ConvertPosOri(llaPosition,  eCOORD_SYS_LLA,
//                                ecefPosition, eCOORD_SYS_ECEF,
//                                llaEul,       ecefEul,
//                                llaVelocity,  ecefVelocity,
//                                llaAcc,       ecefAcc);

//   cerr << "-------------------------------------------------------\n";
//   cerr << "ecefPosition[0] = " << ecefPosition[0] << "\n";
//   cerr << "ecefPosition[1] = " << ecefPosition[1] << "\n";
//   cerr << "ecefPosition[2] = " << ecefPosition[2] << "\n\n";
//   cerr << "ecefEul[0] = " << ecefEul[0] * CU_RAD2DEG << "\n";
//   cerr << "ecefEul[1] = " << ecefEul[1] * CU_RAD2DEG << "\n";
//   cerr << "ecefEul[2] = " << ecefEul[2] * CU_RAD2DEG << "\n\n";
//   cerr << "ecefVelocity[0] = " << ecefVelocity[0] << "\n";
//   cerr << "ecefVelocity[1] = " << ecefVelocity[1] << "\n";
//   cerr << "ecefVelocity[2] = " << ecefVelocity[2] << "\n\n";
//   cerr << "ecefAcc[0] = " << ecefAcc[0] << "\n";
//   cerr << "ecefAcc[1] = " << ecefAcc[1] << "\n";
//   cerr << "ecefAcc[2] = " << ecefAcc[2] << "\n\n";

//   return 0;
// }

