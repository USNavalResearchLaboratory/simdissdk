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

#include <cassert>
#include <sstream>
#include "simCore/Calc/Angle.h"
#include "simCore/String/Format.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Gars.h"

namespace {
  /** Letters used to specify latitude portion of GARS coordinate. I and O are intentionally not used. */
  static const std::string LAT_LETTERS = "ABCDEFGHJKLMNPQRSTUVWXYZ";
  /** Total number of valid letters used in specifying latitudinal band. */
  static const int NUM_LAT_LETTERS = 24;
  /** Valid latitudinal band specifiers range from AA to QZ, making Q (index 14) the last valid primary letter. */
  static const int MAX_PRIMARY_LAT_IDX = 14;
  /** Number of latitudinal degrees per primary letter. */
  static const double DEG_PER_PRIMARY_LETTER = 12.0;
  /** Number of latitudinal degrees per secondary letter. */
  static const double DEG_PER_SECONDARY_LETTER = 0.5;
}

namespace simCore
{

bool Gars::isValidGars(const std::string& gars, std::string* err, int* lonBand, int* latPimaryIdx, int* latSecondaryIdx, int* quad15, int* key5)
{
  // Verify length of GARS coordinate
  if (gars.size() < 5 || gars.size() > 7)
  {
    if (err)
      *err = "Invalid GARS coordinate length (valid range is [5, 7])";
    return false;
  }

  // Find longitude using first three characters
  const std::string lonBandStr = gars.substr(0, 3);
  int lonBandInt;
  if (!simCore::isValidNumber(lonBandStr, lonBandInt, false))
  {
    if (err)
      *err = "Longitudinal band not a valid number";
    return false;
  }

  if (lonBandInt < 1 || lonBandInt > 720)
  {
    if (err)
      *err = "Longitudal band out of range (valid range is [001, 720])";
    return false;
  }

  // Find latitude band using next two characters
  std::string latBandStr = gars.substr(3, 2);
  latBandStr = simCore::upperCase(latBandStr);
  const size_t latPrimaryIndex = LAT_LETTERS.find(latBandStr[0]);
  const size_t latSecondaryIndex = LAT_LETTERS.find(latBandStr[1]);
  if (latPrimaryIndex == std::string::npos || latPrimaryIndex > MAX_PRIMARY_LAT_IDX ||
    latSecondaryIndex == std::string::npos)
  {
    if (err)
      *err = "Invalid letters given for latitudinal band (valid range is AA-QZ)";
    return false;
  }

  if (gars.size() > 5)
  {
    // Adjust the coordinates based on the given 15 minute quadrant
    std::string quad15str = gars.substr(5, 1);
    int quad15Int;
    if (!simCore::isValidNumber(quad15str, quad15Int, false))
    {
      if (err)
        *err = "15 minute quadrant is not a valid number";
      return false;
    }
    if (quad15Int < 1 || quad15Int > 4)
    {
      if (err)
        *err = "Invalid number given for 15 minute quadrant (valid range is [1-4])";
      return false;
    }

    if (gars.size() > 6)
    {
      // Adjust the coordinates based on the given 5 minute key
      std::string key5str = gars.substr(6, 1);
      int key5Int;
      if (!simCore::isValidNumber(key5str, key5Int, false))
      {
        if (err)
          *err = "5 minute key is not a valid number";
        return false;
      }
      if (key5Int < 1 || key5Int > 9)
      {
        if (err)
          *err = "Invalid number given for 5 minute key (valid range is [1-9])";
        return false;
      }
      // Assign value only on success
      if (key5)
        *key5 = key5Int;
    }
    // Assign value only on success
    if (quad15)
      *quad15 = quad15Int;
  }

  // Assign values only on success
  if (lonBand)
    *lonBand = lonBandInt;
  if (latPimaryIdx)
    *latPimaryIdx = static_cast<int>(latPrimaryIndex);
  if (latSecondaryIdx)
    *latSecondaryIdx = static_cast<int>(latSecondaryIndex);

  return true;
}

int Gars::convertGarsToGeodetic(const std::string& gars, double& latRad, double& lonRad, std::string* err)
{
  latRad = 0.;
  lonRad = 0.;
  int lonBand;
  int latPrimaryIndex;
  int latSecondaryIndex;
  int quad15;
  int key5;

  if (!Gars::isValidGars(gars, err, &lonBand, &latPrimaryIndex, &latSecondaryIndex, &quad15, &key5))
    return 1; // Error was set by isValidGars()

  double lat = 0.;
  double lon = 0.;

  // Convert from lonBand integer to longitude value
  lon = (lonBand - 360 - 1) * 0.5;

  // Start latitude at -90
  lat = -90.0;
  // Move it up 12 degrees per primary letter
  lat += (latPrimaryIndex * DEG_PER_PRIMARY_LETTER);
  // Move it up 0.5 degrees per secondary letter
  lat += (latSecondaryIndex * DEG_PER_SECONDARY_LETTER);

  if (gars.size() > 5)
  {
    // Quadrants 1 and 2 are 0.25 degrees north of the cell's origin
    if (quad15 < 3)
      lat += 0.25;
    // Quadrants 2 and 4 are 0.25 degrees east of the cell's origin
    if (quad15 % 2 == 0)
      lon += 0.25;

    if (gars.size() > 6)
    {
      // Determine the column specified by the key number, move longitude east accordingly
      int x5 = ((key5 - 1) % 3);
      lon += (x5 / 12.);
      // Determine the row specified by the key number, move latitude north accordingly
      int y5 = 2 - ((key5 - 1) / 3);
      lat += (y5 / 12.);
    }
  }

  // Lat and lon are currently in degrees, convert to radians
  latRad = lat * simCore::DEG2RAD;
  lonRad = lon * simCore::DEG2RAD;

  return 0;
}

int Gars::convertGeodeticToGars(double latRad, double lonRad, std::string& gars, Level level, std::string* err)
{
  // Conversion algorithm below adapted from osgEarthUtil/GARSGraticule.cpp getGARSLabel()

  // Input values are in radians but the algorithm works in degrees, so convert immediately
  double lat = latRad * simCore::RAD2DEG;
  double lon = lonRad * simCore::RAD2DEG;

  // Fix the input values
  lon = simCore::angFix180(lon);
  // Manually fix +180 to -180 to ensure correct conversion
  if (lon == 180.0)
    lon = -180.0;
  lat = simCore::angFix90(lat);

  // Find the longitudinal band number
  const int lonBand = static_cast<int>(floor((lon + 180.0) * 2.));

  // Format the longitude portion of the GARS coordinate
  std::stringstream buf;
  if (lonBand < 9)
    buf << "00";
  else if (lonBand < 99)
    buf << "0";
  buf << (lonBand + 1);

  // Find the latitudinal band number
  const int latBand = static_cast<int>(floor((lat + 90.0) * 2.));
  // Convert the band number to a two letter specification
  const int latPrimaryIndex = latBand / NUM_LAT_LETTERS;
  if (latPrimaryIndex > MAX_PRIMARY_LAT_IDX)
  {
    assert(0); // Should not be possible to calculate a primary index greater than "Q"
    if (err)
      *err = "Internal error";
    return 1;
  }
  const int latSecondaryIndex = latBand - (latPrimaryIndex * NUM_LAT_LETTERS);
  if (latPrimaryIndex < 0 || latPrimaryIndex >= NUM_LAT_LETTERS ||
    latSecondaryIndex < 0 || latSecondaryIndex >= NUM_LAT_LETTERS)
  {
    assert(0); // Calculated indices should not be out of range
    if (err)
      *err = "Internal error";
    return 1;
  }
  // Format the latitude portion of the GARS coordinate
  buf << LAT_LETTERS[latPrimaryIndex] << LAT_LETTERS[latSecondaryIndex];

  if (level == GARS_15 || level == GARS_5)
  {
    // Determine the 15 minute quadrant value [1, 4]
    const int x15Cell = static_cast<int>(floor(fmod(lon + 180.0, 0.5) * 4.));
    const int y15Cell = static_cast<int>(floor(fmod(lat + 90.0, 0.5)  * 4.));
    const int y15CellInverted = 2 - y15Cell - 1;
    // Format the 15 minute quadrant
    const int quad15 = x15Cell + y15CellInverted * 2 + 1;
    assert(quad15 >= 1 && quad15 <= 4); // Quadrant number should always fall in [1, 4] range
    buf << quad15;

    if (level == GARS_5)
    {
      // Determine the 5 minute key value [1, 9]
      const int x5Cell = static_cast<int>(floor((lon + 180.0 - (lonBand * 0.5 + x15Cell * 0.25)) * 12.));
      const int y5Cell = static_cast<int>(floor((lat + 90.0 - (latBand * 0.5 + y15Cell * 0.25))  * 12.));
      const int y5CellInverted = 3 - y5Cell - 1;
      // Format the 5 minute key
      const int key5 = x5Cell + y5CellInverted * 3 + 1;
      assert(key5 >= 1 && key5 <= 9); // Key number should always fall in [1, 9] range
      buf << key5;
    }
  }


  gars = buf.str();
  return 0;
}

}
