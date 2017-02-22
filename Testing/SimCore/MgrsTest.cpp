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
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Mgrs.h"

namespace
{

int mgrsToLla()
{
  int rv = 0;
  std::string err;
  double lat;
  double lon;
  // 0,0
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NAA6602100000", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 0 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 0 * simCore::DEG2RAD));

  // Random point northwest quadrant
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("10SGA3487998613", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 32.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -120.5 * simCore::DEG2RAD));

  // Near dateline 1
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("60CWA8071262770", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -76 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 179.99 * simCore::DEG2RAD));

  // Near dateline 2; note leading 0
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("01NAE6798353800", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 4.1 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -179.99 * simCore::DEG2RAD));

  // Near dateline 3; note lack of leading 0
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("1NAE6798353800", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 4.1 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -179.99 * simCore::DEG2RAD));

  // Near dateline 3; note lack of leading 0
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("02Q MD 0000", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 16.27876350 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -171.93592645 * simCore::DEG2RAD));

  // Near 84 degrees north, the bound of MGRS/UTM
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("23XNJ0904399764", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 80.1625335 * simCore::DEG2RAD));
  // Note that the longitude value here is slightly off due to lack of precision
  // in MGRS coordinates.  This value matches what was given in the online converter
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -44.5258899 * simCore::DEG2RAD, 1e-5));

  // Near 80 degrees south, the bound of MGRS/UTM
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("33CWM1974418352", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -79.999 * simCore::DEG2RAD));
  // Again note lack of precision near poles
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 16.01846201 * simCore::DEG2RAD, 1e-5));

  // Test one more near the middle that isn't exactly 0,0
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA2173455318", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 0.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 0.5 * simCore::DEG2RAD));

  // Near north pole
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("YZG9922199208", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 89.99 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -44.5258892 * simCore::DEG2RAD));

  // Another near the north pole
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("YZD9418566906", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 87.0 * simCore::DEG2RAD));
  // Note that the longitude value here is slightly off due to the MGRS standard of truncating
  // instead of rounding. The value matches what was given in the online converter, however
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -1.0 * simCore::DEG2RAD, 1e-5));

  // Rounding up does improve accuracy by a degree of magnitude
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("YZD9418666907", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -1.0 * simCore::DEG2RAD, 1e-6));

  // Near south pole
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("BAN0030601067", lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -89.99 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 16.0021174 * simCore::DEG2RAD));

  // Try invalid positions...

  // Chop off the last digit in most recent one (makes it lopsided)
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA217345531", lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();

  // Make sure that more than 10 position digits still will convert
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("ZBA217345531800", lat, lon, &err) == 0);

  // Chop off another and it should work -- same with chopping off all the easting/northing
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA21734553", lat, lon, &err) == 0);
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA", lat, lon, &err) == 0);
  // But adding one should fail
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA2", lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // And adding 2 will succeed
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA21", lat, lon, &err) == 0);

  // White box test on the pointer
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA2", lat, lon, &err) != 0);
  rv += SDK_ASSERT(simCore::Mgrs::convertMgrsToGeodetic("31NBA2", lat, lon, NULL) != 0);
  return rv;
}

int upsToLla()
{
  int rv = 0;
  std::string err;
  double lat;
  double lon;

  // Test the north and south poles
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 2000000.0, 2000000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -90.0 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 0.0 *simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 2000000.0, 2000000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 90.0 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 0.0 * simCore::DEG2RAD));

  // Test some points just past the UTM limits
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 900000, 1900000, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -80.0752462 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -95.1944289 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 2000000.0, 1000000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -81.0106632645 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 180.0 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 2000000.0, 3000000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -81.0106632645 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 0.0 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 2786184.0, 2786184.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -80.01 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 45.0 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 2550000.0, 2150000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -84.8684706 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, 74.7448813 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 1950000, 1950000, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -89.3631098 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -135.0 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_SOUTH, 1000000.0, 2000000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, -81.0106632645 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -90.0 * simCore::DEG2RAD));

  // Same as previous point but in north hemisphere. Should be the same latitude but positive, and the
  // longitude should be rotated by 180 degrees
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 2550000.0, 2150000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 84.8684706 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, (180.0 - 74.7448813) * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 1860000.0, 1870000.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 88.2793246 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -47.1210964 * simCore::DEG2RAD));

  // Just outside the UTM range, but still well within UPS
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 1403500.0, 1703500.0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::areAnglesEqual(lat, 84.0054010 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areAnglesEqual(lon, -63.5695812 * simCore::DEG2RAD));

  // Values outside the range of UPS
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 4000001, 0, lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 0, 4000001, lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, -1, 2000000, lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 2000000, -1, lat, lon, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Test the four corners of the UPS range.
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 0, 0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 4000000, 0, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 0, 4000000, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());
  rv += SDK_ASSERT(simCore::Mgrs::convertUpsToGeodetic(simCore::Mgrs::UPS_NORTH, 4000000, 4000000, lat, lon, &err) == 0);
  rv += SDK_ASSERT(err.empty());

  return rv;
}

int divide()
{
  int rv = 0;
  int zone;
  std::string letters;
  double easting;
  double northing;
  std::string err;

  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("31NAA6602100000", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 31);
  rv += SDK_ASSERT(letters == "NAA");
  rv += SDK_ASSERT(easting == 66021);
  rv += SDK_ASSERT(northing == 00000);

  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("10SGA3487998613", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 10);
  rv += SDK_ASSERT(letters == "SGA");
  rv += SDK_ASSERT(easting == 34879);
  rv += SDK_ASSERT(northing == 98613);

  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("60CWA8071262770", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 60);
  rv += SDK_ASSERT(letters == "CWA");
  rv += SDK_ASSERT(easting == 80712);
  rv += SDK_ASSERT(northing == 62770);

  // Note leading 0
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("01NAE6798353800", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 1);
  rv += SDK_ASSERT(letters == "NAE");
  rv += SDK_ASSERT(easting == 67983);
  rv += SDK_ASSERT(northing == 53800);

  // Note lack of leading 0
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("1NAE6798353800", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 1);
  rv += SDK_ASSERT(letters == "NAE");
  rv += SDK_ASSERT(easting == 67983);
  rv += SDK_ASSERT(northing == 53800);

  // Polar region
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("00YZG9922199208", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 0);
  rv += SDK_ASSERT(letters == "YZG");
  rv += SDK_ASSERT(easting == 99221);
  rv += SDK_ASSERT(northing == 99208);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("YZG9922199208", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 0);
  rv += SDK_ASSERT(letters == "YZG");
  rv += SDK_ASSERT(easting == 99221);
  rv += SDK_ASSERT(northing == 99208);

  // Other polar region
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("00BAN0030601067", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 0);
  rv += SDK_ASSERT(letters == "BAN");
  rv += SDK_ASSERT(easting == 306);
  rv += SDK_ASSERT(northing == 1067);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("BAN0030601067", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 0);
  rv += SDK_ASSERT(letters == "BAN");
  rv += SDK_ASSERT(easting == 306);
  rv += SDK_ASSERT(northing == 1067);

  // Divide up another point, with decreasing accuracy
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ1234567890", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ12345678", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ123456", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(zone == 4);
  rv += SDK_ASSERT(letters == "QFJ");
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ1234", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(northing == 34000);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ12", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(easting == 0);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ123456789012", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(easting == 12345.6);
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ12345678901234", zone, letters, easting, northing, &err) == 0);
  rv += SDK_ASSERT(easting == 12345.67);

  // Throw in invalid positions...
  // Odd digits
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ123456789", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ1234567", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ12345", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ123", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ1", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // GZD too short
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("FJ12345678", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // 11 digits
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("4QFJ12345678901", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // No zone
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("1234567890", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Too big of a zone
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("123QFJ456890", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Empty string
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Single character string
  rv += SDK_ASSERT(simCore::Mgrs::breakMgrsString("A", zone, letters, easting, northing, &err) != 0);
  rv += SDK_ASSERT(!err.empty());
  err.clear();

  return rv;
}

}

int MgrsTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(mgrsToLla() == 0);
  rv += SDK_ASSERT(upsToLla() == 0);
  rv += SDK_ASSERT(divide() == 0);
  return rv;
}
