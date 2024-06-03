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

/*
 * Note: Several functions have been repurposed from software provided by the
 *       White Sands Missile Range (WSMR) and from the GEOTRANS library.
 *       Source code for GEOTRANS can be found here : http ://earth-info.nga.mil/GandG/geotrans/
 *       GEOTRANS license can be found here : http ://earth-info.nga.mil/GandG/geotrans/docs/MSP_GeoTrans_Terms_of_Use.pdf
 */

#include <cmath>
#include <sstream>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Mgrs.h"

namespace simCore
{

int Mgrs::convertMgrsToGeodetic(const std::string& mgrs, double& lat, double& lon, std::string* err)
{
  int zone;
  std::string gzdLetters;
  double easting;
  double northing;
  bool northPole;

  if (breakMgrsString(mgrs, zone, gzdLetters, easting, northing, err) != 0)
  {
    return 1;
  }
  // A zone of 0 means the grid zone letter is A/B/Y/Z and thus should be converted to UPS.
  if (zone == 0)
  {
    double upsEasting;
    double upsNorthing;
    if (convertMgrsToUps(gzdLetters, easting, northing, northPole, upsEasting, upsNorthing, err) != 0)
      return 1;
    if (convertUpsToGeodetic(northPole, upsEasting, upsNorthing, lat, lon, err) != 0)
      return 1;
  }
  // Everything else should be converted through UTM
  else
  {
    double utmEasting;
    double utmNorthing;
    if (convertMgrsToUtm(zone, gzdLetters, easting, northing, northPole, utmEasting, utmNorthing, err) != 0)
      return 1;
    if (convertUtmToGeodetic(zone, northPole, utmEasting, utmNorthing, lat, lon, err) != 0)
      return 1;
  }

  return 0;
}

int Mgrs::breakMgrsString(const std::string& mgrs, int& zone, std::string& gzdLetters, double& easting, double& northing, std::string* err)
{
  // Remove any whitespace and surrounding quotes from the string
  std::string mgrsString = StringUtils::substitute(removeQuotes(mgrs), " ", "");

  size_t letterStart = mgrsString.find_first_not_of("0123456789");
  if (letterStart == std::string::npos)
  {
    if (err)
      *err = "Invalid MGRS string: Missing grid zone designator.";
    return 1;
  }
  std::string utmZone = mgrsString.substr(0, letterStart);

  if (!isValidNumber(utmZone, zone, false))
  {
    // Check to see if zone number wasn't provided and the coordinate is at one of the polar zones.
    if (mgrsString[0] == 'A' || mgrsString[0] == 'B' || mgrsString[0] == 'Y' || mgrsString[0] == 'Z')
      zone = 0;
    else
    {
      if (err)
        *err = "Invalid MGRS string: expected zone number.";
      return 1;
    }
  }
  if (zone > 60)
  {
    if (err)
      *err = "Invalid MGRS string: zone number out of range (0-60).";
    return 1;
  }

  // Will end on the index of the last letter, so increment by 1 to get the index of the first position digit
  size_t positionStart = mgrsString.find_last_not_of("0123456789") + 1;
  std::string zoneLetters = mgrsString.substr(letterStart, positionStart - letterStart);
  if (zoneLetters.size() != 3)
  {
    if (err)
    {
      if (zoneLetters.size() > 3)
        *err = "Invalid MGRS string: GZD or Grid Sqare ID is too large.";
      else
        *err = "Invalid MGRS string: GZD or Grid Sqare ID missing.";
    }
    return 1;
  }
  for (int i = 0; i < 3; i++)
  {
    zoneLetters[i] = toupper(zoneLetters[i]);
    if (!isalpha(zoneLetters[i]) || zoneLetters[i] == 'I' || zoneLetters[i] == 'O')
    {
      if (err)
        *err = "Invalid MGRS string: Invalid character found.";
      return 1;
    }
  }
  gzdLetters = zoneLetters;

  std::string position = mgrsString.substr(positionStart, std::string::npos);
  // Make sure the numerical location has an even number of digits and is less than 10 digits total
  if (position.size() == 0)
  {
    easting = 0;
    northing = 0;
  }
  else if ((position.size() & 1) == 0)
  {
    size_t numDigitsInPosition = position.size() / 2;
    if (!isValidNumber(position.substr(0, numDigitsInPosition), easting, false))
    {
      if (err)
        *err = "Invalid MGRS string: Numeric easting location is not a valid number.";
      return 1;
    }
    if (!isValidNumber(position.substr(numDigitsInPosition, std::string::npos), northing, false))
    {
      if (err)
        *err = "Invalid MGRS string: Numeric northing location is not a valid number.";
      return 1;
    }
    // multiply the position values until they are 5 digits long (i.e. range of 0 - 99,999)
    if (numDigitsInPosition < 5)
    {
      for (unsigned int i = 0; i < 5 - numDigitsInPosition; ++i)
      {
        easting *= 10;
        northing *= 10;
      }
    }
    // If more than 5 digits, we have sub-meter precision and need to divide it down to less than 100,000
    else if (numDigitsInPosition > 5)
    {
      for (unsigned int i = 0; i < numDigitsInPosition - 5; ++i)
      {
        easting /= 10;
        northing /= 10;
      }
    }
  }
  else
  {
    if (err)
      *err = "Invalid MGRS string: Numeric easting and northing location are different length.";
    return 1;
  }

  return 0;
}

int Mgrs::convertMgrsToUtm(int zone, const std::string& gzdLetters, double mgrsEasting, double mgrsNorthing,
  bool &northPole, double& utmEasting, double& utmNorthing, std::string* err)
{
  const double ONEHT = 100000.;
  const double TWOMIL = 2000000.;

  if (zone < 1 || zone > 60)
  {
    if (err)
      *err = "Invalid MGRS coordinate: Zone is not in range 1-60";
    return 1;
  }
  if (gzdLetters.size() != 3)
  {
    if (err)
      *err = "Invalid MGRS coordinate: GZD is invalid.";
    return 1;
  }
  if (mgrsEasting > ONEHT)
  {
    if (err)
      *err = "Invalid MGRS coordinate: Easting is out of range.";
    return 1;
  }
  if (mgrsNorthing > ONEHT)
  {
    if (err)
      *err = "Invalid MGRS coordinate: Northing is out of range.";
    return 1;
  }

  // Exception case for Svalbard
  if ((gzdLetters[0] == 'X') && ((zone == 32) || (zone == 34) || (zone == 36)))
  {
    if (err)
      *err = "Invalid MGRS coordinate: Zones 32X, 34X, and 36X do not exist.";
    return 1;
  }
  // Exception case for Norway
  if ((gzdLetters[0] == 'V') && (zone == 31) && (gzdLetters[1] > 'D'))
  {
    if (err)
      *err = "Invalid MGRS coordinate: Zone 31V must have grid column letter D or lower.";
    return 1;
  }
  // Make sure the grid row letter is in the correct range
  if (gzdLetters[2] > 'V')
  {
    if (err)
      *err = "Invalid MGRS coordinate: Grid row letter is out of range.";
    return 1;
  }

  char columnLetterLowValue;
  char columnLetterHighValue;
  double patternOffset;
  getGridValues_(zone, columnLetterLowValue, columnLetterHighValue, patternOffset);

  // Check that the second letter of the MGRS string is within the range of valid second letter values
  // and check that the third letter is valid
  if ((gzdLetters[1] < columnLetterLowValue) || (gzdLetters[1] > columnLetterHighValue))
  {
    if (err)
      *err = "Invalid MGRS coordinate: Grid column letter is out of range.";
    return 1;
  }

  double gridEasting = (gzdLetters[1] - columnLetterLowValue + 1) * ONEHT;
  if ((columnLetterLowValue == 'J') && (gzdLetters[1] > 'O'))
    gridEasting -= ONEHT;

  // The equivalent northing to the MGRS grid value should be 100,000m times the row letter value, minus 'I' and 'O'.
  double rowLetterNorthing = (gzdLetters[2] - 'A') * ONEHT;
  if (gzdLetters[2] > 'O')
    rowLetterNorthing -= ONEHT;
  if (gzdLetters[2] > 'I')
    rowLetterNorthing -= ONEHT;

  double minNorthing;
  double northingOffset;
  if (getLatitudeBandMinNorthing_(gzdLetters[0], minNorthing, northingOffset) != 0)
  {
    if (err)
      *err = "Invalid MGRS coordinate: Latitude band letter is invalid.";
    return 1;
  }

  double gridNorthing = rowLetterNorthing - patternOffset;
  if (gridNorthing < 0)
    gridNorthing += TWOMIL;

  gridNorthing += northingOffset;

  if (gridNorthing < minNorthing)
    gridNorthing += TWOMIL;

  utmEasting = gridEasting + mgrsEasting;
  utmNorthing = gridNorthing + mgrsNorthing;

  // Latitude bands of 'N' and lower are in the southern hemisphere.
  northPole = !(gzdLetters[0] < 'N');

  return 0;
}

int Mgrs::convertUtmToGeodetic(int zone, bool northPole, double easting, double northing, double& lat, double& lon, std::string* err)
{
  // Standard scale factor for UTM
  const double scaleFactor = 0.9996;

  if (zone < 1 || zone > 60)
  {
    if (err)
      *err = "Invalid UTM coordinate: Zone is not in range 1-60.";
    return 1;
  }
  // some basic range checking.
  if (easting > 1000000 || easting < 0)
  {
    if (err)
      *err = "Invalid UTM coordinate: Easting is not within expected range.";
    return 1;
  }
  if (northing > 10000000 || northing < 0)
  {
    if (err)
      *err = "Invalid UTM coordinate: Northing is not within expected range.";
    return 1;
  }

  // If in the southern hemisphere, subtract the standard false northing value of 10 million that is added to avoid negative values.
  if (!northPole)
    northing -= 10000000;

  const double n1 = WGS_F / (2.0 - WGS_F);
  const double n2 = pow(n1, 2);
  const double n3 = pow(n1, 3);
  const double n4 = pow(n1, 4);

  const double r = WGS_A * (1.0 - n1) * (1.0 - n2) * (1.0 + 9.0*n2 / 4.0 + 225.0*n4 / 64.0);
  const double omega = northing / (scaleFactor * r);

  const double cosP1 = cos(omega);
  const double cos2P1 = cosP1  * cosP1;
  const double cos4P1 = cos2P1 * cos2P1;
  const double cos6P1 = cos4P1 * cos2P1;

  const double v2 = 3.0*n1 / 2.0 - 27.0*n3 / 32.0;
  const double v4 = 21.0*n2 / 16.0 - 55.0*n4 / 32.0;
  const double v6 = 151.0*n3 / 96.0;
  const double v8 = 1097.0*n4 / 512.0;

  const double V0 = 2.0*(v2 - 2.0*v4 + 3.0*v6 - 4.0*v8);
  const double V2 = 8.0*(v4 - 4.0*v6 + 10.0*v8);
  const double V4 = 32.0*(v6 - 6.0*v8);
  const double V6 = 128.0*(v8);

  const double phif = omega + sin(omega)*cosP1*(V0 + V2 * cos2P1 + V4 * cos4P1 + V6 * cos6P1);

  const double tf = tan(phif);
  const double tf2 = tf * tf;
  const double tf4 = tf2 * tf2;
  const double tf6 = tf4 * tf2;

  const double etaf2 = WGS_EP2 * cos(phif) * cos(phif);
  const double etaf4 = etaf2 * etaf2;

  const double B2 = -0.5 * tf * (1.0 + etaf2);
  const double B3 = -1.0 / 6.0 * (1.0 + 2.0 * tf2 + etaf2);
  const double B4 = -1.0 / 12.0 * (5.0 + 3.0 * tf2 + etaf2*(1.0 - 9.0 * tf2) - 4.0 * etaf4);
  const double B5 = 1.0 / 120.0 * (5.0 + 28.0 * tf2 + 24.0 * tf4 + etaf2*(6.0 + 8.0 * tf2));
  const double B6 = 1.0 / 360.0 * (61.0 + 90.0 * tf2 + 45.0 * tf4 + etaf2*(46.0 - 252.0 * tf2 - 90.0*tf4));
  const double B7 = -1.0 / 5040.0 * (61.0 + 662.0 * tf2 + 1320.0 * tf4 + 720.0 * tf6);

  const double Q = (easting - 500000.0) * sqrt(1.0 - WGS_ESQ * sin(phif) * sin(phif)) / (scaleFactor * WGS_A);
  const double Q2 = Q * Q;

  lat = phif + B2 * Q2 * (1.0 + Q2 * (B4 + B6 * Q2));
  lon = (6 * zone - 183) * DEG2RAD + Q*(1.0 + Q2 * (B3 + Q2 * (B5 + B7 * Q2))) / cos(phif);

  if (lat > M_PI_2 || lat < -M_PI_2 || lon > M_PI || lon < -M_PI)
  {
    if (err)
      *err = "UTM to geodetic conversion resulted in position outside valid range.";
    return 1;
  }
  return 0;
}

int Mgrs::convertMgrsToUps(const std::string& gzdLetters, double mgrsEasting, double mgrsNorthing,
  bool& northPole, double& upsEasting, double& upsNorthing, std::string* err)
{
  const UPS_Constants UPS_Constant_Table[4] =
  {
    { 'J', 'Z', 'Z', 800000.0, 800000.0 },   // Latitude band A
    { 'A', 'R', 'Z', 2000000.0, 800000.0 },  // Latitude band B
    { 'J', 'Z', 'P', 800000.0, 1300000.0 },  // Latitude band Y
    { 'A', 'J', 'P', 2000000.0, 1300000.0 }  // Latitude band Z
  };

  if (gzdLetters.size() != 3)
  {
    if (err)
      *err = "Invalid UPS coordinate: GZD string must be 3 characters.";
    return 1;
  }

  int upsIndex;
  if ((gzdLetters[0] == 'Y') || (gzdLetters[0] == 'Z'))
  {
    northPole = true;
    // The indices for 'Y' and 'Z' are at 2 and 3, so subtract 'Y' - 2 == 'W'
    upsIndex = gzdLetters[0] - 'W';
  }
  else if ((gzdLetters[0] == 'A') || (gzdLetters[0] == 'B'))
  {
    northPole = false;
    upsIndex = gzdLetters[0] - 'A';
  }
  else
  {
    if (err)
      *err = "Invalid UPS coordinate: First letter of GZD must be A, B, Y, or Z.";
    return 1;
  }

  char gridColumnLowValue = UPS_Constant_Table[upsIndex].gridColumnLowValue;
  char gridColumnHighValue = UPS_Constant_Table[upsIndex].gridColumnHighValue;
  char gridRowHighValue = UPS_Constant_Table[upsIndex].gridRowHighValue;
  double falseEasting = UPS_Constant_Table[upsIndex].falseEasting;
  double falseNorthing = UPS_Constant_Table[upsIndex].falseNorthing;

  // Check that the grid column letter of the MGRS string is within the range of valid second letter values.
  if ((gzdLetters[1] < gridColumnLowValue) || (gzdLetters[1] > gridColumnHighValue) ||
    ((gzdLetters[1] == 'D') || (gzdLetters[1] == 'E') ||
    (gzdLetters[1] == 'M') || (gzdLetters[1] == 'N') ||
    (gzdLetters[1] == 'V') || (gzdLetters[1] == 'W')))
  {
    if (err)
      *err = "Grid column letter is not valid for provided GZD.";
    return 1;
  }
  // Check that the grid row letter is valid.
  if (gzdLetters[2] > gridRowHighValue)
  {
    if (err)
      *err = "Grid row letter is outside of the range of possible values.";
    return 1;
  }

  // Northing for 100,000 meter grid square
  double gridNorthing = (gzdLetters[2] - 'A') * 100000.0 + falseNorthing;
  if (gzdLetters[2] > 'I')
    gridNorthing = gridNorthing - 100000.0;

  if (gzdLetters[2] > 'O')
    gridNorthing = gridNorthing - 100000.0;

  // Easting for 100,000 meter grid square
  double gridEasting = (gzdLetters[1] - gridColumnLowValue) * 100000.0 + falseEasting;
  if (gridColumnLowValue != 'A')
  {
    if (gzdLetters[1] > 'L')
      gridEasting = gridEasting - 300000.0;

    if (gzdLetters[1] > 'U')
      gridEasting = gridEasting - 200000.0;
  }
  else
  {
    if (gzdLetters[1] > 'C')
      gridEasting = gridEasting - 200000.0;

    if (gzdLetters[1] > 'I')
      gridEasting = gridEasting - 100000.0;

    if (gzdLetters[1] > 'L')
      gridEasting = gridEasting - 300000.0;
  }

  upsEasting = gridEasting + mgrsEasting;
  upsNorthing = gridNorthing + mgrsNorthing;

  return 0;
}

// Equation adapted from GeographicLib version 1.49, PolarStereographic::Reverse()
// https://geographiclib.sourceforge.io/html/PolarStereographic_8cpp_source.html
int Mgrs::convertUpsToGeodetic(bool northPole, double falseEasting, double falseNorthing, double& lat, double& lon, std::string* err)
{
  // False position offset for easting and northing values
  const double falsePosOffset = 2000000.0;
  // Maximum easting and northing value based on a false value of 2000000.0 and a delta value of 2000000.0
  const double maxFalseValue = 4000000.0;
  // Check that easting and northing are not out of range.
  if (falseEasting > maxFalseValue || falseEasting < 0)
  {
    if (err)
    {
      std::stringstream eStr;
      eStr.precision(10);
      eStr << "Easting (" << falseEasting << ") is not within the range of UPS: [4000000, 0].";
      *err = eStr.str();
    }
    return 1;
  }
  if (falseNorthing > maxFalseValue || falseNorthing < 0)
  {
    if (err)
    {
      std::stringstream eStr;
      eStr.precision(10);
      eStr << "Northing (" << falseNorthing << ") is not within the range of UPS: [4000000, 0].";
      *err = eStr.str();
    }
    return 1;
  }

  // Back out the false offset values, algorithm expects easting and northing of points (m) from the
  // center of projection (true means north, false means south).
  const double y = falseNorthing - falsePosOffset;
  const double x = falseEasting - falsePosOffset;

  // Handle special case at the top of the pole where longitude is undefined.
  // GeographicLib computes a value of M_PI for longitude in this condition,
  // we will force longitude to zero to match the prior implementation.
  if (x == 0. && y == 0.)
  {
    lat = (northPole ? 1. : -1.) * M_PI_2;
    lon = 0.;
    return 0;
  }

  const double rho = hypot_(x, y);
  // 7.9130711166184124404360762089019e-8
  // constant based on 1. / ((2. * 0.994 * WGS_A) / ((1 - WGS_F) * exp(WGS_E * atanh(WGS_E))))
  // where 0.994 is the UPS central scale factor
  const double t = rho != 0 ? (rho * 7.9130711166184124404360762089019e-8) : square(std::numeric_limits<double>::epsilon());
  const double taup = (1. / t - t) / 2.;
  const double tau = tauf_(taup);
  lat = (northPole ? 1. : -1.) * atan2(tau, 1.);
  lon = atan2(x, (northPole ? -y : y));
  return 0;
}

void Mgrs::getGridValues_(int zone, char& columnLetterLowValue, char& columnLetterHighValue, double& patternOffset)
{
  // The zones' lowest and highest column letters repeat every 3 zones.
  long setNumber = zone % 3;

  if (setNumber == 1)
  {
    columnLetterLowValue = 'A';
    columnLetterHighValue = 'H';
  }
  else if (setNumber == 2)
  {
    columnLetterLowValue = 'J';
    columnLetterHighValue = 'R';
  }
  else if (setNumber == 0)
  {
    columnLetterLowValue = 'S';
    columnLetterHighValue = 'Z';
  }

  // Account for the offset applied on every other grid row letter
  if ((zone & 1) ==  0)
    patternOffset = 500000.0;
  else
    patternOffset = 0.0;
}

int Mgrs::getLatitudeBandMinNorthing_(char bandLetter, double& minNorthing, double& northingOffset)
{
  const Latitude_Band latitudeBandTable[26] =
  {
    // Letters A, B, I, O, Y, and Z are invalid but are added here for error checking and to simplify indexing
    { -1, -1 },               // LETTER A
    { -1, -1},                // LETTER B
    { 1100000.0, 0.0 },       // LETTER C
    { 2000000.0, 2000000.0 }, // LETTER D
    { 2800000.0, 2000000.0 }, // LETTER E
    { 3700000.0, 2000000.0 }, // LETTER F
    { 4600000.0, 4000000.0 }, // LETTER G
    { 5500000.0, 4000000.0 }, // LETTER H
    { -1, -1 },               // LETTER I
    { 6400000.0, 6000000.0 }, // LETTER J
    { 7300000.0, 6000000.0 }, // LETTER K
    { 8200000.0, 8000000.0 }, // LETTER L
    { 9100000.0, 8000000.0 }, // LETTER M
    { 0.0, 0.0 },             // LETTER N
    { -1, -1 },               // LETTER O
    { 800000.0, 0.0 },        // LETTER P
    { 1700000.0, 0.0 },       // LETTER Q
    { 2600000.0, 2000000.0 }, // LETTER R
    { 3500000.0, 2000000.0 }, // LETTER S
    { 4400000.0, 4000000.0 }, // LETTER T
    { 5300000.0, 4000000.0 }, // LETTER U
    { 6200000.0, 6000000.0 }, // LETTER V
    { 7000000.0, 6000000.0 }, // LETTER W
    { 7900000.0, 6000000.0 }, // LETTER X
    { -1, -1 },               // LETTER Y
    { -1, -1 }                // LETTER Z
  };

  int latIndex = static_cast<int>(bandLetter - 'A');
  if (latitudeBandTable[latIndex].minNorthing == -1 && latitudeBandTable[latIndex].northingOffset == -1)
    return 1;

  minNorthing = latitudeBandTable[latIndex].minNorthing;
  northingOffset = latitudeBandTable[latIndex].northingOffset;

  return 0;
}

double Mgrs::atanh_(double x)
{
  return (log(1 + x) - log(1 - x)) / 2;
}

double Mgrs::hypot_(double x, double y)
{
  // NOTE: CXX11 has a std::hypot (> vc11 and gcc 4.6)
  x = fabs(x);
  y = fabs(y);
  if (x < y)
    std::swap(x, y); // Now x >= y >= 0
  y /= (x ? x : 1);
  return x * sqrt(1 + y * y);
}

double Mgrs::taupf_(double tau)
{
  const double tau1 = hypot_(1., tau);
  const double eaTanhE = WGS_E * atanh_(WGS_E * (tau / tau1));
  const double sig = sinh(eaTanhE);
  return hypot_(1., sig) * tau - sig * tau1;
}

double Mgrs::tauf_(double taup)
{
  static const int numit = 5;
  static const double tol = sqrt(std::numeric_limits<double>::epsilon()) / 10.;
  // To lowest order in e^2, taup = (1 - e^2) * tau = _e2m * tau; so use
  // tau = taup/_e2m as a starting guess.  (This starting guess is the
  // geocentric latitude which, to first order in the flattening, is equal
  // to the conformal latitude.)  Only 1 iteration is needed for |lat| <
  // 3.35 deg, otherwise 2 iterations are needed.  If, instead, tau = taup
  // is used the mean number of iterations increases to 1.99 (2 iterations
  // are needed except near tau = 0).
  double tau = taup / WGS_ESQC;
  const double stol = tol * simCore::sdkMax(1., fabs(taup));
  // min iterations = 1, max iterations = 2; mean = 1.94
  for (int i = 0; i < numit; ++i)
  {
    double taupa = taupf_(tau);
    double dtau = (taup - taupa) * (1 + WGS_ESQC * (tau*tau)) / (WGS_ESQC * hypot_(1., tau) * hypot_(1., taupa));
    tau += dtau;
    if (!(fabs(dtau) >= stol))
      break;
  }
  return tau;
}

}
