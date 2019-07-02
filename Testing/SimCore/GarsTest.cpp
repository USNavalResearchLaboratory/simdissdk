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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Gars.h"
#include "simCore/Calc/Math.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Format.h"

namespace
{

  // Gold data for these tests generated using random values that were then verified using a
  // calculator found online ( http://www.earthpoint.us/Convert.aspx ) and by turning on the
  // GARS grid and using the mouse pointer and SIMDIS's cursor position readout.

int llaToGars()
{
  int rv = 0;

  std::string gars;
  // Test random geodetic coordinates
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(22.791517 * simCore::DEG2RAD, -178.815690 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "003KK19");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(25.617852 * simCore::DEG2RAD, 178.875637 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "718KR45");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(88.255585 * simCore::DEG2RAD, 33.624625 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "428QW18");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(-86.800432 * simCore::DEG2RAD, 14.190311 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "389AG33");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(-16.353404 * simCore::DEG2RAD, 82.484571 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "525GD46");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(25.093151 * simCore::DEG2RAD, 5.766738 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "372KQ44");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(33.369778 * simCore::DEG2RAD, -48.507968 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "263LG26");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(30.319639 * simCore::DEG2RAD, -88.597160 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "183LA28");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(0., 0., gars) == 0);
  rv += SDK_ASSERT(gars == "361HN37");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(0., -180. * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "001HN37");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(0., 180. * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "001HN37");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(0., 179.999999 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "720HN49");

  // Test all 36 cells within a single GARS 5-character cell
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.455386 * simCore::DEG2RAD, -37.958948 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE11");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.453063 * simCore::DEG2RAD, -37.869747 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE12");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.453695 * simCore::DEG2RAD, -37.793930 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE13");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.367717 * simCore::DEG2RAD, -37.955651 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE14");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.371987 * simCore::DEG2RAD, -37.883062 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE15");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.385842 * simCore::DEG2RAD, -37.772261 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE16");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.298674 * simCore::DEG2RAD, -37.942197 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE17");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.297526 * simCore::DEG2RAD, -37.881729 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE18");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.296365 * simCore::DEG2RAD, -37.791347 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE19");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.451888 * simCore::DEG2RAD, -37.702184 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE21");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.455439 * simCore::DEG2RAD, -37.611710 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE22");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.451156 * simCore::DEG2RAD, -37.535896 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE23");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.376821 * simCore::DEG2RAD, -37.687564 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE24");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.373782 * simCore::DEG2RAD, -37.624523 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE25");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.373096 * simCore::DEG2RAD, -37.539192 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE26");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.291556 * simCore::DEG2RAD, -37.706697 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE27");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.299914 * simCore::DEG2RAD, -37.616312 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE28");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.290225 * simCore::DEG2RAD, -37.538036 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE29");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.196570 * simCore::DEG2RAD, -37.958615 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE31");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.205654 * simCore::DEG2RAD, -37.873387 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE32");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.209237 * simCore::DEG2RAD, -37.795137 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE33");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.118430 * simCore::DEG2RAD, -37.953405 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE34");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.120915 * simCore::DEG2RAD, -37.873937 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE35");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.126361 * simCore::DEG2RAD, -37.808455 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE36");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.034941 * simCore::DEG2RAD, -37.966632 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE37");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.048852 * simCore::DEG2RAD, -37.875143 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE38");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.047101 * simCore::DEG2RAD, -37.784904 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE39");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.214651 * simCore::DEG2RAD, -37.706715 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE41");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.207417 * simCore::DEG2RAD, -37.630382 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE42");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.207348 * simCore::DEG2RAD, -37.545142 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE43");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.134183 * simCore::DEG2RAD, -37.709907 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE44");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.137755 * simCore::DEG2RAD, -37.625340 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE45");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.127464 * simCore::DEG2RAD, -37.531249 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE46");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.046506 * simCore::DEG2RAD, -37.703564 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE47");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.039867 * simCore::DEG2RAD, -37.619687 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE48");
  rv += SDK_ASSERT(simCore::Gars::convertGeodeticToGars(20.047015 * simCore::DEG2RAD, -37.549780 * simCore::DEG2RAD, gars) == 0);
  rv += SDK_ASSERT(gars == "285KE49");

  return rv;
}

int garsToLla()
{
  int rv = 0;

  double lat;
  double lon;

  // Test various fully qualified GARS coordinates
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("003KK19", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 22.75 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -178.833333 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("718KR45", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 25.5833333 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, 178.833333 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("428QW18", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 88.25 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, 33.5833333 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("389AG33", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, -86.833333 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, 14.1666667 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("525GD46", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, -16.4166667 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, 82.4166667 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("372KQ44", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 25.0833333 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, 5.75 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("263LG26", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 33.333333 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -48.583333 * simCore::DEG2RAD));

  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("183LA28", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 30.25 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -88.666667 * simCore::DEG2RAD));

  // Test the same GARS coordinate at increasing levels of detail -- should return identical coordinates
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("322NV", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 63.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -19.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("322NV3", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 63.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -19.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("322NV37", lat, lon) == 0);
  rv += SDK_ASSERT(simCore::areEqual(lat, 63.5 * simCore::DEG2RAD));
  rv += SDK_ASSERT(simCore::areEqual(lon, -19.5 * simCore::DEG2RAD));

  // Test invalid GARS coordinates
  std::string err;
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("wrong", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Too short
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("100A", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Too long
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("100AA110", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid longitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("000AA11", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid longitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("721AA11", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid latitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001RA11", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid latitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001A811", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid latitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AI11", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid latitudinal band
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AO11", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid 15 minute quadrant
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AAA1", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid 15 minute quadrant
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AA01", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid 5 minute key
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AA1A", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();
  // Invalid 5 minute key
  rv += SDK_ASSERT(simCore::Gars::convertGarsToGeodetic("001AA10", lat, lon, &err) == 1);
  rv += SDK_ASSERT(!err.empty());
  err.clear();

  return rv;
}

}

int GarsTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(llaToGars() == 0);
  rv += SDK_ASSERT(garsToLla() == 0);
  return rv;
}

