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
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/UtfUtils.h"

namespace {

//----------------------------------------------------------------------------
bool almostEqual(double value1, double value2, double epsilon=1e-4)
{
  if (!simCore::areEqual(value1, value2, epsilon))
  {
    std::cerr << "FAILURE" << std::endl;
    std::cerr << std::setprecision(16) << "    " << value1 << " != " << value2 << " delta: " << value1 - value2 << std::endl;
    return false;
  }
  return true;
}

int testCalculateRelAzEl(double *fromLla, double *fromOri, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateRelAzEl +++++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  double azim;
  double elev;
  double compositeAngle;

  simCore::calculateRelAzEl(simCore::Vec3(fromLla), simCore::Vec3(fromOri), simCore::Vec3(to), &azim, &elev, &compositeAngle, earth, &coordConvert);

  if (almostEqual(azim, result[0]) &&
    almostEqual(elev, result[1]) &&
    almostEqual(compositeAngle, result[2]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateAbsAzEl(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateAbsAzEl +++++++++++++ ";

  double azim;
  double elev;
  double compositeAngle;

  simCore::calculateAbsAzEl(simCore::Vec3(from[0], from[1], from[2]), simCore::Vec3(to), &azim, &elev, &compositeAngle, earth, &coordConvert);

  if (almostEqual(azim, result[0]) &&
    almostEqual(elev, result[1]) &&
    almostEqual(compositeAngle, result[2]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateSlant(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateSlant +++++++++++++++ ";

  const double slant = simCore::calculateSlant(simCore::Vec3(from[0], from[1], from[2]), simCore::Vec3(to), earth, &coordConvert);
  if (almostEqual(slant, result[0]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateGroundDist(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateGroundDist ++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const double groundDist = simCore::calculateGroundDist(simCore::Vec3(from), simCore::Vec3(to), earth, &coordConvert);
  if (almostEqual(groundDist, result[0]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateAltitude(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateAltitude ++++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const double altitude = simCore::calculateAltitude(simCore::Vec3(from[0], from[1], from[2]),
    simCore::Vec3(to[0], to[1], to[2]), earth, &coordConvert);
  if (almostEqual(altitude, result[0]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateDRCRDownValue(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateDRCRDownValue +++++++ ";

  double downRng;
  double crossRng;
  double downValue;

  simCore::calculateDRCRDownValue(simCore::Vec3(from), from[3], simCore::Vec3(to), earth, &coordConvert, &downRng, &crossRng, &downValue);

  if (almostEqual(downRng, result[0]) &&
    almostEqual(crossRng, result[1]) &&
    almostEqual(downValue, result[2]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateGeodesicDRCR(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateGeodesicDRCR ++++++++ ";

  double downRng;
  double crossRng;

  simCore::calculateGeodesicDRCR(simCore::Vec3(from), from[3], simCore::Vec3(to), &downRng, &crossRng);

  if (almostEqual(downRng, result[0], 1.3) &&
    almostEqual(crossRng, result[1]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateTotalVelocity(double *from, double *to, double deltaTime, simCore::EarthModelCalculations earth, double *result)
{
  std::cerr << "calculateTotalVelocity +++++++ ";

  simCore::Vec3 velVec;
  simCore::calculateVelFromGeodeticPos(simCore::Vec3(from), simCore::Vec3(to), deltaTime, velVec);
  if (almostEqual(velVec[0], result[0]) &&
    almostEqual(velVec[1], result[1]) &&
    almostEqual(velVec[2], result[2]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateClosingVelocity(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateClosingVelocity +++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const double velocity = simCore::calculateClosingVelocity(simCore::Vec3(from), simCore::Vec3(to), earth, &coordConvert, simCore::Vec3(&from[6]), simCore::Vec3(&to[6]));
  if (almostEqual(velocity, result[0]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

int testCalculateVelocityDelta(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateVelocityDelta +++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const double velocity = simCore::calculateVelocityDelta(simCore::Vec3(from), simCore::Vec3(to), earth, &coordConvert, simCore::Vec3(&from[6]), simCore::Vec3(&to[6]));
  if (almostEqual(velocity, result[0]))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

//===========================================================================
int testCalculateAspectAngle(double *from, double *to, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double *result)
{
  std::cerr << "calculateAspectAngle +++++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const double aspectAngle = simCore::calculateAspectAngle(simCore::Vec3(from), simCore::Vec3(to), simCore::Vec3(&to[3]));
  if (almostEqual(aspectAngle, result[0], 0.001))
  {
    std::cerr << "successful" << std::endl;
    return 0;
  }
  return 1;
}

//===========================================================================
int testPositionInGate(double *from, double *to, double *gate, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double* result)
{
  std::cerr << "positionInGate +++++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const int inGate = simCore::positionInGate(simCore::Vec3(from), simCore::Vec3(to), gate[0], gate[1], gate[2], gate[3], gate[4], gate[5], earth, coordConvert) ? 1 : 0;
  if (inGate == static_cast<int>(result[0]))
  {
    std::cerr << "successful\n";
    return 0;
  }
  std::cerr << "failed\n";
  return 1;
}

//===========================================================================
int testLaserInGate(double *from, double *to, double *gate, double *laser, simCore::EarthModelCalculations earth, simCore::CoordinateConverter coordConvert, double* result)
{
  std::cerr << "laserInGate +++++++++++++ ";

  if (earth == simCore::PERFECT_SPHERE)
  {
    std::cerr << "calculation not valid for Earth Model" << std::endl;
    return 1;
  }

  const int inGate = simCore::laserInGate(simCore::Vec3(from), simCore::Vec3(to), gate[0], gate[1], gate[2], gate[3], gate[4], gate[5], laser[0], laser[1], laser[2], earth, coordConvert) ? 1 : 0;
  if (inGate == static_cast<int>(result[0]))
  {
    std::cerr << "successful\n";
    return 0;
  }
  std::cerr << "failed\n";
  return 1;
}

//===========================================================================
void printInstructions()
{
  static bool seenInstructions = false;
  if (!seenInstructions)
  {
    seenInstructions = true;
    std::cout << "Input File Format:" << std::endl << std::endl;
    std::cout << "[Calculation][CoordinateSystem] [ReferenceOrigin]" << std::endl;
    std::cout << "[Arg1] [Arg2] ... [ArgN]" << std::endl;
    std::cout << "[ExpectedResult1] [ExpectedResult2] ... [ExpectedResultN]" << std::endl << std::endl;
  }
}

/////////////////////////////////////////////////////////////////////////////
//Takes an input file with the following format:
//
//[Calculation][CoordinateSystem] [ReferenceOrigin]
//[Arg1] [Arg2] ... [ArgN]
//[ExpectedResult1] [ExpectedResult2] ... [ExpectedResultN]
//
//Where CoordinateSystem = WGS84|FlatEarth|PerfectSphere|TangentPlaneWGS84
//
//Following calculations are available:
//
//_______________________________________________________________________
//|Calculation          | Input Arguments      | # Results             |
//-----------------------------------------------------------------------
//| Slant               | from[3] to[3]        | 1                     |
//| AbsAzEl             | from[3] to[3]        | 3 [Az, El, Composite] |
//| RelAzEl             | from[6] to[3]        | 3 [Az, El, Composite] |
//| AspectAngle         | from[3] to[6]        | 1                     |
//| Altitude            | from[3] to[3]        | 1                     |
//| GroundDist          | from[3] to[3]        | 1                     |
//| GeodesicDRCR        | from[6] to[3]        | 2 [DR, CR]            |
//| VelocityDelta       | from[9] to[9]        | 1                     |
//| TotalVelocity       | from[9] to[9]        | 3 [Velocity Vector]   |
//| DRCRDownValue       | from[6] to[3]        | 3 [DR, CR, DownValue] |
//| ClosingVelocity     | from[9] to[9]        | 1                     |
//| PositionInGate      | from[3] to[3] gate[6]| 1                     |
//| LaserInGate         | from[3] to[3] gate[6] laser[3] | 1           |
//
//
/////////////////////////////////////////////////////////////////////////////

int readNextTest(std::istream& fd, bool& doneReading)
{
  std::string test;
  if (fd >> test)
    doneReading = false;
  else
  {
    doneReading = true;
    // Not a failure
    return 0;
  }

  // Check for comments
  if (test.compare(0, 1, "#") == 0)
    return 0;

  int rv = 0;
  simCore::EarthModelCalculations earth = simCore::PERFECT_SPHERE;
  // set coordinate system / reference frame
  if (test.compare(simCore::sdkMax((int)test.length() - 17, 0), test.length() - 1, "TangentPlaneWGS84") == 0)
  {
    if (earth != simCore::TANGENT_PLANE_WGS_84)
    {
      std::cerr << "Earth Model: TangentPlaneWGS84" << std::endl;
      earth = simCore::TANGENT_PLANE_WGS_84;
    }
  }
  else if (test.compare(simCore::sdkMax((int)test.length() - 5, 0), test.length() - 1, "WGS84") == 0)
  {
    if (earth != simCore::WGS_84)
    {
      std::cerr << "Earth Model: WGS84" << std::endl;
      earth = simCore::WGS_84;
    }
  }
  else if (test.compare(simCore::sdkMax((int)test.length() - 9, 0), test.length() - 1, "FlatEarth") == 0)
  {
    if (earth != simCore::FLAT_EARTH)
    {
      std::cerr << "Earth Model: FlatEarth" << std::endl;
      earth = simCore::FLAT_EARTH;
    }
  }
  else if (test.compare(simCore::sdkMax((int)test.length() - 13, 0), test.length() - 1, "PerfectSphere") == 0)
  {
    if (earth != simCore::PERFECT_SPHERE)
    {
      std::cerr << "Earth Model: PerfectSphere" << std::endl;
      earth = simCore::PERFECT_SPHERE;
    }
  }
  else
  {
    std::cout << "Incorrect input file format:" << " missing valid coordinate system / reference frame" << std::endl;
    printInstructions();
    rv++;
  }

  std::cerr << "  ";

  // get and set Reference Origin
  double refOrigin[3] = { 0.0 };
  fd >> refOrigin[0] >> refOrigin[1] >> refOrigin[2];
  simCore::CoordinateConverter coordConvert;
  coordConvert.setReferenceOrigin(refOrigin[0], refOrigin[1], refOrigin[2]);

  double from[9] = { 0.0 };
  double to[9] = { 0.0 };
  double result[3] = { 0.0 };

  // set to/from and run relevant test
  if (test.compare(0, 5, "Slant") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> to[0] >> to[1] >> to[2];
    fd >> result[0];
    rv += testCalculateSlant(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 7, "AbsAzEl") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> to[0] >> to[1] >> to[2];
    fd >> result[0] >> result[1] >> result[2];
    rv += testCalculateAbsAzEl(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 7, "RelAzEl") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5];
    fd >> to[0] >> to[1] >> to[2];
    fd >> result[0] >> result[1] >> result[2];
    rv += testCalculateRelAzEl(from, from + 3, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 11, "AspectAngle") == 0)
  {
    fd >> from[0] >> from[1] >> from[2];
    fd >> to[0] >> to[1] >> to[2] >> to[3] >> to[4] >> to[5];
    fd >> result[0];
    rv += testCalculateAspectAngle(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 8, "Altitude") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> to[0] >> to[1] >> to[2];
    fd >> result[0];
    rv += testCalculateAltitude(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 10, "GroundDist") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> to[0] >> to[1] >> to[2];
    fd >> result[0];
    rv += testCalculateGroundDist(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 12, "GeodesicDRCR") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5];
    fd >> to[0] >> to[1] >> to[2];
    fd >> result[0] >> result[1];
    rv += testCalculateGeodesicDRCR(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 13, "VelocityDelta") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5] >> from[6] >> from[7] >> from[8];
    fd >> to[0] >> to[1] >> to[2] >> to[3] >> to[4] >> to[5] >> to[6] >> to[7] >> to[8];
    fd >> result[0];
    rv += testCalculateVelocityDelta(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 13, "TotalVelocity") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5] >> from[6] >> from[7] >> from[8];
    fd >> to[0] >> to[1] >> to[2] >> to[3] >> to[4] >> to[5] >> to[6] >> to[7] >> to[8];
    double time = 0.0;
    fd >> time;
    fd >> result[0] >> result[1] >> result[2];
    rv += testCalculateTotalVelocity(from, to, time, earth, result);
  }
  else if (test.compare(0, 13, "DRCRDownValue") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5];
    fd >> to[0] >> to[1] >> to[2];
    fd >> result[0] >> result[1] >> result[2];
    rv += testCalculateDRCRDownValue(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 15, "ClosingVelocity") == 0)
  {
    fd >> from[0] >> from[1] >> from[2] >> from[3] >> from[4] >> from[5] >> from[6] >> from[7] >> from[8];
    fd >> to[0] >> to[1] >> to[2] >> to[3] >> to[4] >> to[5] >> to[6] >> to[7] >> to[8];
    fd >> result[0];
    rv += testCalculateClosingVelocity(from, to, earth, coordConvert, result);
  }
  else if (test.compare(0, 14, "PositionInGate") == 0)
  {
    double gate[6] = { 0.0 };
    fd >> from[0] >> from[1] >> from[2];
    fd >> to[0] >> to[1] >> to[2];
    fd >> gate[0] >> gate[1] >> gate[2] >> gate[3] >> gate[4] >> gate[5];
    fd >> result[0];
    rv += testPositionInGate(from, to, gate, earth, coordConvert, result);
  }
  else if (test.compare(0, 11, "LaserInGate") == 0)
  {
    double gate[6] = { 0.0 };
    double laser[3] = { 0.0 };
    fd >> from[0] >> from[1] >> from[2];
    fd >> to[0] >> to[1] >> to[2];
    fd >> gate[0] >> gate[1] >> gate[2] >> gate[3] >> gate[4] >> gate[5];
    fd >> laser[0] >> laser[1] >> laser[2];
    fd >> result[0];
    rv += testLaserInGate(from, to, gate, laser, earth, coordConvert, result);
  }
  else
  {
    std::cout << "Command not valid: " << test;
    printInstructions();
    rv++;
  }
  return rv;
}

int calculateLibTestFile(const std::string& filename)
{
  // Read the test data
  std::ifstream fd(simCore::streamFixUtf8(filename));
  if (!fd)
  {
    std::cout << "Error opening file " << filename << std::endl;
    return 1;
  }

  // Loop through each group in the file, reading the next test
  int rv = 0;
  bool doneReading = false;
  while (!doneReading)
    rv += readNextTest(fd, doneReading);

  return rv;
}

}

//===========================================================================
int CalculateLibTest(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " [filename]" << std::endl;
    return -1;
  }

  int rv = 0;
  rv += calculateLibTestFile(argv[1]);
  return rv;
}
