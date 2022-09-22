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
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/GogToGeoFence.h"

namespace
{
  std::string okGog = "start\n"               // valid example
                      "poly\n"
                      "3d name Named GOG\n"
                      "ll 34 -121\n"
                      "ll 32 -93\n"
                      "ll 47 -94\n"
                      "ll 45 -122\n"
                      "ll 34 -121\n"
                      "end\n";

  std::string tabDelimiterGog = "start\n"        // tabs as delimiters
                                "poly\n"
                                "ll\t34\t-121\n"
                                "ll\t32\t-93\n"
                                "ll\t47\t-94\n"
                                "ll\t45\t-122\n"
                                "ll\t34\t-121\n"
                                "end\n";

  std::string randomCaseGog = "stARt\n"        // random case use throughout
                              "POLY\n"
                              "ll 34 -121\n"
                              "LL 32 -93\n"
                              "lL 47 -94\n"
                              "ll 45 -122\n"
                              "LL 34 -121\n"
                              "End\n";

  std::string llaCommentGog = "start\n"          // has altitude and comments
                              "poly\n"
                              "ll 29 -156 30000\n"
                              "ll 27 -158 30000\n"
                              "ll 25 -156 30000\n"
                              "ll 27 -154 30000\n"
                              "ll 29 -156 30000\n"
                              "end\n"
                              "comment text about a subject\n"
                              "start\n"
                              "poly\n"
                              "ll 39 -166\n"
                              "ll 37 -168 // important comment\n"
                              "ll 35 -166 # something something\n"
                              "ll 37 -164\n"
                              "ll 39 -166\n"
                              "end\n";

  std::string lineGog = "start\n"   // valid line gog
                        "line\n"
                        "ll 11 -144\n"
                        "ll -6 -144\n"
                        "ll  2 -129\n"
                        "ll  9 -132\n"
                        "ll 11 -144\n"
                        "end\n";

  std::string invalidLatGog = "start\n"
                              "poly\n"
                              "3d name Invalid Lat GOG\n"
                              "ll xx -121\n"          // invalid latitude value
                              "ll 32 -93\n"
                              "ll 47 -94\n"
                              "ll 45 -122\n"
                              "ll 34 -121\n"
                              "end\n";

  std::string invalidLonGog = "start\n"
                              "poly\n"
                              "3d name Invalid Lon GOG\n"
                              "ll 34 xx\n"          // invalid longitude value
                              "ll 32 -93\n"
                              "ll 47 -94\n"
                              "ll 45 -122\n"
                              "ll 34 -121\n"
                              "end\n";

  std::string invalidAltGog = "start\n"
                              "poly\n"
                              "3d name Invalid Alt GOG\n"
                              "ll 29 -156 xxxxx\n"  // invalid altitude value
                              "ll 27 -158 30000\n"
                              "ll 25 -156 30000\n"
                              "ll 27 -154 30000\n"
                              "ll 29 -156 30000\n"
                              "end\n";

  std::string tooFewArgsGog = "start\n"
                              "poly\n"
                              "3d name Too Few Arguments GOG\n"
                              "ll 29\n"  // too few arguments
                              "ll 27 -158 30000\n"
                              "ll 25 -156 30000\n"
                              "ll 27 -154 30000\n"
                              "ll 29 -156 30000\n"
                              "end\n";

  std::string invalidStartGog = "start\n"
                                "poly\n"
                                "ll 34 -121\n"
                                "ll 32 -93\n"
                                "start\n"          // invalid start
                                "ll 47 -94\n"
                                "ll 45 -122\n"
                                "ll 34 -121\n"
                                "end\n";

  std::string invalidKeywordGog = "start\n"
                                  "circle\n"         // bad keyword
                                  "ll 34 -121\n"
                                  "ll 32 -93\n"
                                  "ll 47 -94\n"
                                  "ll 45 -122\n"
                                  "ll 34 -121\n"
                                  "end\n";

  std::string noStartGog = "poly\n"           // no start block
                           "ll 34 -121\n"
                           "ll 32 -93\n"
                           "ll 47 -94\n"
                           "ll 45 -122\n"
                           "ll 34 -121\n"
                           "end\n";

  std::string endBeforeStartGog = "end\n"           // end before start
                                  "start\n"
                                  "poly\n"
                                  "ll 34 -121\n"
                                  "ll 32 -93\n"
                                  "ll 47 -94\n"
                                  "ll 45 -122\n"
                                  "ll 34 -121\n"
                                  "end\n";

  std::string notAGog = "invalid file\n";  // not a gog

  std::string tunnelGog = "start\n"
                          "poly\n"
                          "ll 43 -146\n"
                          "ll 25 -160\n"
                          "ll  7 -169\n"
                          "ll -6 -173\n"
                          "ll -8 -169\n"
                          "ll  5 -160\n"
                          "ll 27 -149\n"
                          "ll 41 -142\n"
                          "ll 43 -146\n"
                          "end\n";

  std::string validGog = "start\n"
                         "poly\n"
                         "ll 11 -144\n"
                         "ll -6 -144\n"
                         "ll  2 -129\n"
                         "ll  9 -132\n"
                         "ll 11 -144\n"
                         "end\n";

  std::string invalidGog = "start\n"
                           "poly\n"
                           "3d name Invalid GOG\n"
                           "ll 11 -144\n"
                           "ll -6 -144\n"
                           "ll  2 -129\n"
                           "ll  2 -139\n" // makes concave
                           "ll 11 -144\n"
                           "end\n";

  std::string offGog = "start\n"          // will only create one fence
                       "off\n"
                       "poly\n"
                       "ll 11 -144\n"
                       "ll -6 -144\n"
                       "ll  2 -129\n"
                       "ll  9 -132\n"
                       "ll 11 -144\n"
                       "end\n"
                       "start\n"
                       "poly\n"
                       "ll 11 -144\n"
                       "ll -6 -144\n"
                       "ll  2 -129\n"
                       "ll  9 -132\n"
                       "ll 11 -144\n"
                       "end\n";

  std::string threeGog = "start\n"          // creates 3 convex fences
                         "poly\n"
                         "ll 29 -156\n"
                         "ll 27 -158\n"
                         "ll 25 -156\n"
                         "ll 27 -154\n"
                         "ll 29 -156\n"
                         "end\n"
                         "\n"
                         "start\n"
                         "poly\n"
                         "ll 39 -166\n"
                         "ll 37 -168\n"
                         "ll 35 -166\n"
                         "ll 37 -164\n"
                         "ll 39 -166\n"
                         "end\n"
                         "start\n"
                         "poly\n"
                         "ll 19 -146\n"
                         "ll 17 -148\n"
                         "ll 15 -146\n"
                         "ll 17 -144\n"
                         "ll 19 -146\n"
                         "end\n";

  std::string clockwiseGog = "start\n"    // drawn in clockwise order, making it concave by default
                             "poly\n"
                             "ll 32.9 -120.7\n"
                             "ll 32.6 -120.3\n"
                             "ll 32.3 -120.7\n"
                             "ll 32.7 -121.1\n"
                             "ll 32.9 -120.7\n"
                             "end\n";

  std::string openPolyGog = "start\n"   // poly not closed
                            "poly\n"
                            "ll 11 -144\n"
                            "ll -6 -144\n"
                            "ll  2 -129\n"
                            "ll  9 -132\n"
                            "end\n";

  std::string openLineGog = "start\n"   // line not closed
                            "line\n"
                            "ll 11 -144\n"
                            "ll -6 -144\n"
                            "ll  2 -129\n"
                            "ll  9 -132\n"
                            "end\n";

  std::string clockwiseConcaveGog = "start\n"    // drawn in clockwise order and concave
                                    "poly\n"
                                    "ll 32.9 -120.7\n"
                                    "ll 32.6 -120.3\n"
                                    "ll 32.6 -120.6\n" // makes concave
                                    "ll 32.3 -120.7\n"
                                    "ll 32.7 -121.1\n"
                                    "ll 32.9 -120.7\n"
                                    "end\n";
}

int testGogSyntax()
{
  int rv = 0;

  simCore::GogToGeoFence g;

  std::stringstream is(okGog);
  rv += SDK_ASSERT(g.parse(is) == 0);
  g.clear();

  is.clear();
  is.str(tabDelimiterGog);
  rv += SDK_ASSERT(g.parse(is) == 0);
  g.clear();

  is.clear();
  is.str(randomCaseGog);
  rv += SDK_ASSERT(g.parse(is) == 0);
  g.clear();

  is.clear();
  is.str(llaCommentGog);
  rv += SDK_ASSERT(g.parse(is) == 0);
  g.clear();

  is.clear();
  is.str(lineGog);
  rv += SDK_ASSERT(g.parse(is) == 0);

  is.clear();
  is.str(invalidLatGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(invalidLonGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(invalidAltGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(tooFewArgsGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(invalidStartGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(invalidKeywordGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(noStartGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(endBeforeStartGog);
  rv += SDK_ASSERT(g.parse(is) == 1);
  g.clear();

  is.clear();
  is.str(notAGog);
  rv += SDK_ASSERT(g.parse(is) == 1);

  return rv;
}

int testContains()
{
  int rv = 0;

  simCore::GogToGeoFence g;
  std::stringstream is(tunnelGog);
  g.parse(is);
  simCore::GogToGeoFence::GeoFenceVec fences;
  g.getFences(fences);
  simCore::GeoFence tunnel(*(fences[0]));

  rv += SDK_ASSERT(tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(12.1517 * simCore::DEG2RAD, -161.7467 * simCore::DEG2RAD, 0.0))));  // inside
  rv += SDK_ASSERT(tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(14.1646 * simCore::DEG2RAD, -155.7947 * simCore::DEG2RAD, 0.0))));  // barely inside
  rv += SDK_ASSERT(tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(2.8143 * simCore::DEG2RAD, -170.2252 * simCore::DEG2RAD, 0.0))));   // barely inside
  rv += SDK_ASSERT(!tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(1.7753 * simCore::DEG2RAD, -151.8232 * simCore::DEG2RAD, 0.0))));  // outside
  rv += SDK_ASSERT(!tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(19.2367 * simCore::DEG2RAD, -168.6341 * simCore::DEG2RAD, 0.0)))); // outside
  rv += SDK_ASSERT(!tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(28.9251 * simCore::DEG2RAD, -148.1085 * simCore::DEG2RAD, 0.0)))); // barely outside
  rv += SDK_ASSERT(!tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(20.4452 * simCore::DEG2RAD, -162.4872 * simCore::DEG2RAD, 0.0)))); // barely outside
  rv += SDK_ASSERT(!tunnel.contains(simCore::Coordinate(simCore::COORD_SYS_LLA, simCore::Vec3(-1.233 * simCore::DEG2RAD, 32.8981 * simCore::DEG2RAD, 0.0))));    // other side of the world

  return rv;
}

int testValidity()
{
  int rv = 0;

  simCore::GogToGeoFence g;
  std::stringstream is(validGog);
  g.parse(is);
  simCore::GogToGeoFence::GeoFenceVec fences;
  g.getFences(fences);

  rv += SDK_ASSERT(fences[0]->valid());

  g.clear();
  is.clear();
  fences.clear();

  is.str(clockwiseGog);
  g.parse(is);
  g.getFences(fences);

  rv += SDK_ASSERT(fences[0]->valid());

  g.clear();
  is.clear();
  fences.clear();

  is.str(openPolyGog);
  g.parse(is);
  g.getFences(fences);

  rv += SDK_ASSERT(fences[0]->valid());

  g.clear();
  is.clear();
  fences.clear();

  is.str(openLineGog);
  g.parse(is);
  g.getFences(fences);

  rv += SDK_ASSERT(fences.empty());

  g.clear();
  is.clear();
  fences.clear();

  is.str(clockwiseConcaveGog);
  g.parse(is);
  g.getFences(fences);

  rv += SDK_ASSERT(fences.empty());

  g.clear();
  is.clear();
  fences.clear();

  is.str(invalidGog);
  g.parse(is);
  g.getFences(fences);

  rv += SDK_ASSERT(fences.empty());

  return rv;
}

int testOff()
{
  int rv = 0;

  simCore::GogToGeoFence g;
  std::stringstream is(offGog);
  g.parse(is);
  std::vector<simCore::Vec3String> coordinatesVec;
  simCore::GogToGeoFence::GeoFenceVec fences;
  g.getCoordinatesVec(coordinatesVec);
  g.getFences(fences);

  rv += SDK_ASSERT(fences.size() == 1);
  rv += SDK_ASSERT(coordinatesVec.size() == 1);

  return rv;
}

int testMultiple()
{
  int rv = 0;

  simCore::GogToGeoFence g;
  std::stringstream is(threeGog);
  g.parse(is);
  simCore::GogToGeoFence::GeoFenceVec fences;
  g.getFences(fences);

  rv += SDK_ASSERT(fences.size() == 3);

  return rv;
}

int GogToGeoFenceTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testGogSyntax() == 0);
  rv += SDK_ASSERT(testContains() == 0);
  rv += SDK_ASSERT(testValidity() == 0);
  rv += SDK_ASSERT(testOff() == 0);
  rv += SDK_ASSERT(testMultiple() == 0);

  return rv;
}
