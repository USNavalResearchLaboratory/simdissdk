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
// Gold Data v6.2 For Software Testing
// http://earth-info.nga.mil/GandG/coordsys/Conversion_Software/index.html
//
// While GEOTRANS is the NGA recommended and supported software package for
// Datum Transformations and Coordinate Conversions, there are occasions when
// something else is requested or proposed, and needs to be tested. For such
// occasions, software developers are invited to download the following ZIP
// collection of test files. These are files created by NGA geodesists working
// independently of GEOTRANS. The files are simulated data (not measurements)
// and provide a higher accuracy standard for software performance than does
// GEOTRANS as of 3/25/2009 for the algorithms treated. Also, in some cases,
// they allow input values that GEOTRANS does not. Therefore, if developers
// have not carefully defined the domain of valid inputs and implemented the
// corresponding input checking, these tests are likely to reveal the lack
// thereof. The files "Instructions.doc" and "Release_Notes.doc", found in
// the ZIP, contain further information. This is an ongoing project, and more
// tests will be added in later releases. Comments are welcome at the e-mail
// address below.
// Point of Contact: Coordinate Systems Analysis Team
// phone (314) 676-9124, DSN 846-9124
// coordsys@nga.mil
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/UtfUtils.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace simCore;
using namespace std;

static const std::string STR_FMT_WHITE_SPACE = " \n\r\t";
static const double CU_DEG2RAD  = 0.017453292519943295;

//===========================================================================
bool getStrippedLine(istream& is,
                            std::string& str)
{
  if (!std::getline(is, str))
  {
    return false;
  }

  str.erase(str.find_last_not_of(STR_FMT_WHITE_SPACE) + 1);

  return true;
}

template <class T>
inline void StringTokenizer(T *t, const std::string &str,
   const std::string &delimiters = " \t\n\r", bool erase = true)
{
  if (erase) t->erase(t->begin(), t->end());
  // Skip delimiters at beginning.
  std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
  while (std::string::npos != pos || std::string::npos != lastPos)
  {
    // Found a token, add it to the STL container.
    t->push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

//===========================================================================
int compareVec(vector<Coordinate> &inVec,
               vector<Coordinate> &outVec,
               bool lla,
               double epsilonZ)
{
  if (inVec.size() != outVec.size())
  {
    std::cout << "ERROR: Vector sizes mismatch " << inVec.size() << " and "
              << outVec.size() << std::endl;
    return 1;
  }

  // Gold data appears approximate, epsilon should be 1 m precision
  // For geodetic latitude and longitude, the epsilon is approximated
  // based on the following:
  // At sea level, one minute of angle (around the equator or a meridian)
  // equals about one nautical mile (1852 m)
  // Epsilon for a radian value: PI/180/60/1852 = 1.57e-7
  double epsilon = (lla) ? 1.57e-7 : epsilonZ;
  cout.precision(12);

  Vec3 inPos;
  Vec3 outPos;
  bool rv = false;
  for (size_t i = 0; i < inVec.size(); ++i)
  {
    bool failed = false; // print failed message only the first time

     inPos =  inVec[i].position();
    outPos = outVec[i].position();

    if (!areEqual(inPos[0], outPos[0], epsilon))
    {
      cout << "Conversion failed, line #: " << i+1 << endl;
      failed = true;
      rv = true;
      cout << " X: " << inPos[0] << " " << outPos[0] << " " << inPos[0] - outPos[0] << endl;
    }
    if (!areEqual(inPos[1], outPos[1], epsilon))
    {
      if (!failed) cout << "Conversion failed, line #: " << i+1 << endl;
      failed = true;
      rv = true;
      cout << " Y: " << inPos[1] << " " << outPos[1] << " " << inPos[1] - outPos[1] << endl;
    }
    if (!areEqual(inPos[2], outPos[2], epsilonZ))
    {
      if (!failed) cout << "Conversion failed, line #: " << i+1 << endl;
      rv = true;
      cout << " Z: " << inPos[2] << " " << outPos[2] << " " << inPos[2] - outPos[2] << endl;
    }
  }

  if (!rv)
  {
    cout << "Test Passed\n" << endl;
  }

  return rv ? 1 : 0;
}

//===========================================================================
int loadGoldData(const std::string &fname, vector<Coordinate> &inVec,
  CoordinateSystem cs)
{
  const bool lla = cs == COORD_SYS_LLA;

  ifstream inFile(simCore::streamFixUtf8(fname));
  std::string st;
  std::vector<std::string> tmpvec;
  double pos[3];

  if (!inFile)
  {
    cerr << "ERROR!  Could Not Open " << fname << " For Reading" << endl;
    return -1;
  }
  else
  {
    std::cout << "Loading NGA Gold Data file " << fname << endl;
  }

  while (getStrippedLine(inFile, st))
  {
    StringTokenizer(&tmpvec, st, ",");
    if (tmpvec.size() == 3)
    {
      pos[0] = lla ? atof(tmpvec[0].c_str()) * CU_DEG2RAD : atof(tmpvec[0].c_str());
      pos[1] = lla ? atof(tmpvec[1].c_str()) * CU_DEG2RAD : atof(tmpvec[1].c_str());
      pos[2] = atof(tmpvec[2].c_str());
      inVec.push_back(Coordinate(cs, Vec3(pos[0], pos[1], pos[2])));
    }
  }

  inFile.close();
  return 0;
}

//===========================================================================
int GoldDataCoordConvertTest(int _argc_, char *_argv_[])
{
  static const char *const inFiles[COORD_SYS_MAX-1] = {
    "out1.dat",
    "out2.dat",
    "out3.dat",
    "geodetic.dat",
    "geocentric.dat",
    "out10.dat",
    "tan_plane_0_0_0.dat",
    "out9.dat"
  };

  vector<Coordinate> inData[COORD_SYS_MAX-1];
  for (unsigned sys = COORD_SYS_NONE+1; sys < COORD_SYS_MAX; ++sys)
  {
    int rv = loadGoldData(inFiles[sys-1], inData[sys-1], CoordinateSystem(sys));
    if (rv != 0)
    {
      cout << "Failed to load file " << inFiles[sys-1] << endl;
      return -1;
    }
  }

  CoordinateConverter coordConvertor;
  coordConvertor.setReferenceOrigin(0, 0, 0);

  int rv = 0;
  vector<Coordinate> outputVec;
  for (unsigned sys1 = COORD_SYS_NONE+1; sys1 < COORD_SYS_MAX; ++sys1)
  for (unsigned sys2 = COORD_SYS_NONE+1; sys2 < COORD_SYS_MAX; ++sys2)
  {
    if (sys1 == sys2) continue;
    outputVec.clear();

    cout << endl;
    cout << "====================================================" << endl;
    cout << sys1 << " to " << sys2 << " Test Using NGA Gold Data v6.2" << endl;
    cout << "====================================================" << endl;

    for (vector<Coordinate>::iterator iter = inData[sys1-1].begin();
        iter != inData[sys1-1].end();
        ++iter)
    {
      Coordinate outCoord;
      coordConvertor.convert(*iter, &outCoord, CoordinateSystem(sys2));
      outputVec.push_back(outCoord);
    }
    rv += compareVec(inData[sys2-1], outputVec, sys2 == COORD_SYS_LLA, .9);
  }

  return rv;
}

int main(int argc, char **argv)
{
  int rv = GoldDataCoordConvertTest(argc, argv);
  if (rv)
    cerr << "Test failed" << std::endl;
  return rv;
}

