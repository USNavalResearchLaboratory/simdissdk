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
#include <sstream>
#include <string>
#include <vector>
#include "simNotify/Notify.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Parser.h"

namespace
{

static const std::string FILE_VERSION = "version 2\n";

bool parseGog(std::stringstream& gog, simVis::GOG::Parser& parser, std::vector<simVis::GOG::GogMetaData>& metaData, int& rv)
{
  osgEarth::Config config;
  if (!parser.parse(gog, config, metaData))
  {
    rv += SDK_ASSERT(0); // failed to parse
    SIM_ERROR << "Parser failed to parse GOG stream " << gog.str() << "\n";
    return false;
  }
  return true;
}

// Test loading absolute and relative GOGs as attached and un-attached
int testLoadRelativeAndAbsolute()
{
  int rv = 0;

  std::string absoluteGog = FILE_VERSION +
    "start\n line\n lla 26.13568698 55.28931414 5000.\n lla \"26.0 N\" \"55.0 E\" 5000.\n end\n"
    "start\n poly\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\n end\n";

  simVis::GOG::Parser parser;
  // loaded GOG shape nodes
  simVis::GOG::Parser::OverlayNodeVector gogs;
  // follow data for attached GOGs
  std::vector<simVis::GOG::GogFollowData> followData;

  std::stringstream input;

  // Test loading absolute GOGs as un-attached
  input << absoluteGog;
  bool parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify absolute shapes were loaded as un-attached
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  // Test loading absolute GOGs as attached
  input << absoluteGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 0); // verify no absolute shapes were loaded as attached
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  std::string relativeGog = FILE_VERSION +
    "start\n line\n xyz 100 10 50\n xyz -100 44 50\n end\n"
    "start\n poly\n xyz 0 -60 100\n xyz 100 34 100\n xyz -100 -20 100\n end\n";

  // Test loading relative GOGs as attached
  input << relativeGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative shapes were loaded as attached
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  // Test loading relative GOGs as un-attached
  input << relativeGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative shapes were loaded as un-attached
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  std::string relativeAndAbsoluteGog = FILE_VERSION +
    "start\n poly\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\n end\n"
    "start\n poly\n 3d name RELATIVE_GOG\n xyz 0 -60 100\n xyz 100 34 100\n xyz -100 -20 100\n end\n";

  // Test loading GOG with relative and absolute shapes as attached
  input << relativeAndAbsoluteGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1); // verify only 1 shape was loaded as attached
  std::ostringstream os;
  gogs.front()->serializeToStream(os);
  rv += SDK_ASSERT(os.str().find("RELATIVE_GOG") != std::string::npos); // verify the loaded shape was the relative one
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  // Test loading GOG with relative and absolute shapes as un-attached
  input << relativeAndAbsoluteGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative and absolute shapes were loaded as un-attached
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");

  return rv;
}

// Test that metadata is defined correctly when parsing GOGs
int testParseMetaData()
{
  int rv = 0;

  simVis::GOG::Parser parser;
  std::vector<simVis::GOG::GogMetaData> metaData;
  std::stringstream shapes;
  size_t numGogs = 2;

  // test basic shapes that provide no metadata
  shapes << FILE_VERSION;
  shapes << "start\n line\n lla 26.13568698 55.28931414 5000.\n lla \"26.0 N\" \"55.0 E\" 5000.\n end\n";
  shapes << "start\n poly\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\nend\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure all GOGs were created
    rv += SDK_ASSERT(metaData.size() == numGogs);
    // make sure the line's meta data is empty
    rv += SDK_ASSERT(metaData.at(0).metadata.empty());
    // make sure the polygon's meta data is empty
    rv += SDK_ASSERT(metaData.at(1).metadata.empty());
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test basic shapes that provide metadata
  shapes << FILE_VERSION;
  shapes << "start\n circle\n centerlla 26 55 5000.\n radius 1000\n end\n";
  shapes << "start\n arc\n centerlla 26.1 55.2 5000.\n radius 1000\n anglestart 0\n angleend 45\n end\n";
  numGogs = 2;
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure all GOGs were created
    rv += SDK_ASSERT(metaData.size() == numGogs);
    // make sure the circle added its centerlla to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("centerlla 26 55 5000.") != std::string::npos);
    // make sure the arc added its centerlla to metadata
    rv += SDK_ASSERT(metaData.at(1).metadata.find("centerlla 26.1 55.2 5000.") != std::string::npos);
    // make sure the arc added its anglestart to metadata
    rv += SDK_ASSERT(metaData.at(1).metadata.find("anglestart 0") != std::string::npos);
    // make sure the arc added its angleend to metadata
    rv += SDK_ASSERT(metaData.at(1).metadata.find("angleend 45") != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test unattached relative with geometry in metadata
  shapes << FILE_VERSION;
  shapes << "start\n circle\n centerxyz 150 100 50\n radius 1000 \n referencepoint 26.1 55.4 10.\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the circle added its centerxyz to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("centerxyz 150 100 50") != std::string::npos);
    // make sure the circle added its radius to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("radius 1000") != std::string::npos);
    // make sure the circle added its reference point to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("referencepoint 26.1 55.4 10.") != std::string::npos);
    // make sure the reference point keyword was not added, since the point is already in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::ReferencePointKeyword) == std::string::npos);
    // make sure the relative keyword was not added, since geometry is already in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::RelativeShapeKeyword) == std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test unattached relative with geometry in metadata, but shape type is at the end
  shapes << FILE_VERSION;
  shapes << "start\n centerxyz 150 100 50\n radius 1000\n referencepoint 26.1 55.4 10.\n circle\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the circle added its centerxyz to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("centerxyz 150 100 50") != std::string::npos);
    // make sure the circle added its radius to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("radius 1000") != std::string::npos);
    // make sure the circle added its reference point to metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("referencepoint 26.1 55.4 10.") != std::string::npos);
    // make sure the reference point keyword was not added, since the point is already in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::ReferencePointKeyword) == std::string::npos);
    // make sure the relative keyword was not added, since geometry is already in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::RelativeShapeKeyword) == std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test unattached relative with no geometry in metadata
  shapes << FILE_VERSION;
  shapes << "start\n line\n xyz 150 100 50\n xyz 100 100 50\n referencepoint 26.1 55.4 10.\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the geometry was not stored in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("referencepoint") == std::string::npos);
    // make sure the geometry was not stored in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("xyz") == std::string::npos);
    // make sure the reference point keyword was added
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::ReferencePointKeyword) != std::string::npos);
    // make sure the relative keyword was added
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::RelativeShapeKeyword) != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test unattached relative with no geometry in metadata, but shape type is at the end
  shapes << FILE_VERSION;
  shapes << "start\n xyz 150 100 50\n xyz 100 100 50\n referencepoint 26.1 55.4 10.\n line\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the geometry was not stored in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("referencepoint") == std::string::npos);
    // make sure the geometry was not stored in metadata
    rv += SDK_ASSERT(metaData.at(0).metadata.find("xyz") == std::string::npos);
    // make sure the reference point keyword was added
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::ReferencePointKeyword) != std::string::npos);
    // make sure the relative keyword was added
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::RelativeShapeKeyword) != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test unattached relative followed by absolute followed by relative, all with no geometry in metadata
  shapes << FILE_VERSION;
  shapes << "start\n line\n xyz 150 100 50\n xyz 100 100 50\n referencepoint 26.1 55.4 10.\n end\n";
  shapes << "start\n line\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n end\n";
  shapes << "start\n line\n xyz 150 10 10\n xyz 100 10 10\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the geometry was not stored in metadata for unattached relative shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("referencepoint") == std::string::npos);
    // make sure the geometry was not stored in metadata for unattached relative shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("xyz") == std::string::npos);
    // make sure the reference point keyword was added for unattached relative shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::ReferencePointKeyword) != std::string::npos);
    // make sure the relative keyword was added for relative shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find(simVis::GOG::RelativeShapeKeyword) != std::string::npos);
    // make sure the geometry was not stored in metadata for absolute shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("lla") == std::string::npos);
    // make sure the reference point keyword was not added for absolute shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find(simVis::GOG::ReferencePointKeyword) == std::string::npos);
    // make sure the relative keyword was not added for absolute shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find(simVis::GOG::RelativeShapeKeyword) == std::string::npos);
    // make sure the geometry was not stored in metadata for relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find("xyz") == std::string::npos);
    // make sure the reference point keyword was added for relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find(simVis::GOG::ReferencePointKeyword) != std::string::npos);
    // make sure the relative keyword was added for relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find(simVis::GOG::RelativeShapeKeyword) != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();


  // test with geometry in metadata, altitude and range units specified
  shapes << FILE_VERSION;
  shapes << "start\n centerlla 22.1 54.5 50.\n radius 1000\n circle\n altitudeunits feet\n rangeunits meters\n end\n";
  shapes << "start\n centerxyz 150 100 50\n radius 1000\n circle\n altitudeunits feet\n rangeunits meters\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the altitude units is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("altitudeunits feet") != std::string::npos);
    // make sure the range units is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("rangeunits meters") != std::string::npos);
    // make sure the altitude units is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("altitudeunits feet") != std::string::npos);
    // make sure the range units is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("rangeunits meters") != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test with no geometry in metadata, altitude and range units specified
  shapes << FILE_VERSION;
  shapes << "start\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n line\n altitudeunits feet\n rangeunits meters\n end\n";
  shapes << "start\n xyz 150 100 50\n xyz 100 100 50\n line\n altitudeunits feet\n rangeunits meters\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the altitude units is not in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("altitudeunits") == std::string::npos);
    // make sure the range units is not in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("rangeunits") == std::string::npos);
    // make sure the altitude units is not in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("altitudeunits") == std::string::npos);
    // make sure the range units is not in the metadata the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("rangeunits") == std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test basic annotations
  shapes << FILE_VERSION;
  shapes << "start\n Annotation An Absolute Label\n lla 25.6 54.2 0.\n altitudeunits meters\n end\n";
  shapes << "start\n Annotation A Relative Label\n xyz 150 100 50\n altitudeunits feet\n end\n";
  shapes << "start\n referencepoint 22.3 44.3 4.\n xyz 150 100 50\n Annotation A Relative Label\n end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the geometry is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("lla 25.6 54.2 0.") != std::string::npos);
    // make sure the altitude units is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("altitudeunits meters") != std::string::npos);
    // make sure the geometry is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("xyz 150 100 50") != std::string::npos);
    // make sure the altitude units is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("altitudeunits feet") != std::string::npos);
    // make sure the geometry is in the metadata for the unattached relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find("xyz 150 100 50") != std::string::npos);
    // make sure the reference point is in the metadata for the unattached relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find("referencepoint 22.3 44.3 4.") != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  // test annotations special case, with multiple annotations defined in a single start/end block
  shapes << FILE_VERSION;
  shapes << "start\n";
  shapes << "Annotation An Absolute Label\n lla 25.6 54.2 0.\n altitudeunits meters\n";
  shapes << "Annotation A Relative Label\n xyz 150 100 50\n altitudeunits feet\n";
  shapes << "Annotation A Relative Label\n referencepoint 22.3 44.3 4.\n xyz 150 100 50\n";
  shapes << "end\n";
  if (parseGog(shapes, parser, metaData, rv))
  {
    // make sure the geometry is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("lla 25.6 54.2 0.") != std::string::npos);
    // make sure the altitude units is in the metadata for the absolute shape
    rv += SDK_ASSERT(metaData.at(0).metadata.find("altitudeunits meters") != std::string::npos);
    // make sure the geometry is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("xyz 150 100 50") != std::string::npos);
    // make sure the altitude units is in the metadata for the relative shape
    rv += SDK_ASSERT(metaData.at(1).metadata.find("altitudeunits feet") != std::string::npos);
    // make sure the geometry is in the metadata for the unattached relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find("xyz 150 100 50") != std::string::npos);
    // make sure the reference point is in the metadata for the unattached relative shape
    rv += SDK_ASSERT(metaData.at(2).metadata.find("referencepoint 22.3 44.3 4.") != std::string::npos);
  }
  shapes.clear();
  shapes.str("");
  metaData.clear();

  return rv;
}

}

int GogTest(int argc, char* argv[])
{
  int rv = 0;

  // Check the SIMDIS SDK version
  simCore::checkVersionThrow();

  // Run tests
  rv += testLoadRelativeAndAbsolute();
  rv += testParseMetaData();

  // Shut down protobuf lib for valgrind testing
  google::protobuf::ShutdownProtobufLibrary();

  return rv;
}


