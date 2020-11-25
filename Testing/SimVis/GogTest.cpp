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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <sstream>
#include <string>
#include <vector>
#include "osgEarth/FeatureNode"
#include "osgEarth/GeoPositionNode"
#include "osgEarth/LocalGeometryNode"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/GOG/Parser.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Loader.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Parser.h"
#include "simVis/Registry.h"

namespace
{

static const std::string FILE_VERSION = "version 2\n";

// GOG file line attributes that differ from the default
static const std::string LINE_ATTRIBUTES = "linewidth 5\n linestyle dotted\n linecolor hex 0xffff00ff\n";
// GOG file fill attributes that differ from the default
static const std::string FILL_ATTRIBUTES = "filled true\n fillcolor hex 0xff00ffff\n";
// GOG file annotation text attributes that differ from the default
static const std::string TEXT_ATTRIBUTES = "fontsize 32\n fontname georgia.ttf\n";

bool parseGog(std::stringstream& gog, simVis::GOG::Parser& parser, std::vector<simVis::GOG::GogMetaData>& metaData, int& rv)
{
  std::vector<simVis::GOG::ParsedShape> configs;
  if (!parser.parse(gog, configs, metaData))
  {
    rv += SDK_ASSERT(0); // failed to parse
    SIM_ERROR << "Parser failed to parse GOG stream " << gog.str() << "\n";
    return false;
  }
  return true;
}

void clearItems(simVis::GOG::Parser::OverlayNodeVector& gogs, std::vector<simVis::GOG::GogFollowData>& followData, std::stringstream& input)
{
  for (auto iter = gogs.begin(); iter != gogs.end(); ++iter)
  {
    delete *iter;
  }
  gogs.clear();
  followData.clear();
  input.clear();
  input.str("");
}

// returns the first GOG shape parsed from the specified string
simVis::GOG::GogNodeInterfacePtr parseGogFile(simVis::GOG::Parser& parser, simVis::GOG::GOGNodeType type, const std::string& gog, int& rv)
{
  // loaded GOG shape nodes
  simVis::GOG::Parser::OverlayNodeVector gogs;
  // follow data for attached GOGs
  std::vector<simVis::GOG::GogFollowData> followData;
  std::stringstream input;
  input << gog;
  bool parsedGog = parser.loadGOGs(input, type, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked

  simVis::GOG::GogNodeInterfacePtr gogPtr;

  // don't have to delete the pointer in the gogs vector, since the shared_ptr takes ownership
  if (!gogs.empty())
    gogPtr.reset(gogs.front());
  return gogPtr;
}

// returns the first GOG shape parsed from the specified string, using the simCore::GOG::Parser
simVis::GOG::GogNodeInterfacePtr parseGogFileWithCore(bool attached, const std::string& gog, int& rv)
{
  std::stringstream input;
  input << gog;
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);
  simVis::GOG::Loader::GogNodeVector gogs;
  loader.loadGogs(input, attached, gogs);

  simVis::GOG::GogNodeInterfacePtr gogPtr;
  // don't have to delete the pointer in the gogs vector, since the shared_ptr takes ownership
  if (!gogs.empty())
    gogPtr = gogs.front();
  return gogPtr;
}

// tests line state values are consistent with those defined in the LINE_ATTRIBUTES string
int testLineState(simVis::GOG::GogNodeInterfacePtr gog)
{
  int rv = 0;
  bool outlined = false;
  osg::Vec4f lineColor;
  simVis::GOG::Utils::LineStyle lineStyle = simVis::GOG::Utils::LINE_SOLID;
  int lineWidth = 0;
  rv += SDK_ASSERT(gog->getLineState(outlined, lineColor, lineStyle, lineWidth) == 0);
  rv += SDK_ASSERT(outlined);
  rv += SDK_ASSERT(lineColor == osg::Vec4f(1.0, 0, 1.0, 1.0));
  rv += SDK_ASSERT(lineStyle == simVis::GOG::Utils::LINE_DOTTED);
  rv += SDK_ASSERT(lineWidth == 5);
  return rv;
}

// tests fill state values are consistent with those defined in the FILL_ATTRIBUTES string
int testFillState(simVis::GOG::GogNodeInterfacePtr gog)
{
  int rv = 0;
  bool filled = false;
  osg::Vec4f fillColor;
  rv += SDK_ASSERT(gog->getFilledState(filled, fillColor) == 0);
  rv += SDK_ASSERT(filled);
  rv += SDK_ASSERT(fillColor == osg::Vec4f(1.0, 1.0, 0, 1.0));
  return rv;
}

// return true if the specified positions are equal
bool comparePositions(const osg::Vec3d& pos1, const osg::Vec3d& pos2)
{
  return simCore::areEqual(pos1.x(), pos2.x()) && simCore::areEqual(pos1.y(), pos2.y()) && simCore::areEqual(pos1.z(), pos2.z());
}

// test that the specified gog is a FeatureNode, and that it contains the specified points
int testFeatureGeometry(osg::Node* gog, const std::vector<osg::Vec3d>& points)
{
  int rv = 0;
  osgEarth::FeatureNode* gogNode = dynamic_cast<osgEarth::FeatureNode*>(gog);
  rv += SDK_ASSERT(gogNode != nullptr);
  if (rv)
    return rv;
  size_t numPoints = 0;

  const osgEarth::Geometry* geom = gogNode->getFeature()->getGeometry();
  for (osg::Vec3d geomPoint : *geom)
  {
    for (osg::Vec3d point : points)
    {
      if (comparePositions(point, geomPoint))
        numPoints++;
    }
  }
  // make sure all points were found in the Geometry
  rv += SDK_ASSERT(numPoints == points.size());
  return rv;
}

// test that the specified gog is a LocalGeometryNode, and that it contains the specified points
int testLocalGeometry(osg::Node* gog, const std::vector<osg::Vec3d>& points)
{
  int rv = 0;

  osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(gog);
  rv += SDK_ASSERT(gogNode != nullptr);
  if (rv)
    return rv;

  size_t numPoints = 0;
  const osgEarth::Geometry* geom = gogNode->getGeometry();
  for (osg::Vec3d geomPoint : *geom)
  {
    for (osg::Vec3d point : points)
    {
      if (comparePositions(point, geomPoint))
        numPoints++;
    }
  }
  // make sure all points were found in the Geometry
  rv += SDK_ASSERT(numPoints == points.size());
  return rv;
}

int testShapes(bool useCore)
{
  int rv = 0;
  simVis::GOG::Parser parser;
  // test points
  std::string pointGogFile = FILE_VERSION +
    "start\n points\n lla 24.1 44.3 0.\n lla \"26.0 N\" \"55.0 E\" 8.\n pointsize 24\n 3d name point 1\n altitudeunits m\n" +
    TEXT_ATTRIBUTES + " end\n"; // add some invalid items

  simVis::GOG::GogNodeInterfacePtr pointGog = (useCore ? parseGogFileWithCore(false, pointGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, pointGogFile, rv));
  // test the the point parsed correctly
  rv += SDK_ASSERT(pointGog != nullptr);
  if (pointGog)
  {
    rv += SDK_ASSERT(pointGog->shape() == simVis::GOG::GOG_POINTS);
    rv += SDK_ASSERT(pointGog->getDraw());
    rv += SDK_ASSERT(pointGog->osgNode()->getName() == "point 1");

    int pointSize = 0;
    rv += SDK_ASSERT(pointGog->getPointSize(pointSize) == 0);
    rv += SDK_ASSERT(pointSize == 24);

    // test that point doesn't have invalid attributes
    std::string font;
    int fontSize;
    osg::Vec4f fontColor;
    rv += SDK_ASSERT(pointGog->getFont(font, fontSize, fontColor) != 0);

    // test geometry, osgEarth positions are (lon,lat,alt)
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(44.3, 24.1, 0.0));
    points.push_back(osg::Vec3d(55.0, 26.0, 8.0));
    rv += testFeatureGeometry(pointGog->osgNode(), points);
  }

  // test relative point
  std::string pointRelGogFile = FILE_VERSION +
    "start\n points\n xyz 100 200 0\n xyz -100 9 0\n pointsize 24\n 3d name point relative 1\n rangeunits m\n end\n";

  simVis::GOG::GogNodeInterfacePtr pointRelGog = (useCore ? parseGogFileWithCore(true, pointRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, pointRelGogFile, rv));
  // test the the point parsed correctly
  rv += SDK_ASSERT(pointRelGog != nullptr);
  if (pointRelGog)
  {
    rv += SDK_ASSERT(pointRelGog->shape() == simVis::GOG::GOG_POINTS);
    rv += SDK_ASSERT(pointRelGog->getDraw());
    rv += SDK_ASSERT(pointRelGog->osgNode()->getName() == "point relative 1");

    int pointSize = 0;
    rv += SDK_ASSERT(pointRelGog->getPointSize(pointSize) == 0);
    rv += SDK_ASSERT(pointSize == 24);


    // test geometry
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(100.0, 200.0, 0.0));
    points.push_back(osg::Vec3d(-100.0, 9, 0.0));
    rv += testLocalGeometry(pointRelGog->osgNode(), points);
  }

  // test line
  std::string lineGogFile = FILE_VERSION +
    "start\n line\n lla 26.13568698 55.28 5000.\n lla \"26.0 N\" \"55.0 E\" 5000.\n" + LINE_ATTRIBUTES + "3d name line 1\n altitudeunits m\n" +
    TEXT_ATTRIBUTES + // add some invalid items
    "end\n";
  simVis::GOG::GogNodeInterfacePtr lineGog = (useCore ? parseGogFileWithCore(false, lineGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, lineGogFile, rv));
  rv += SDK_ASSERT(lineGog != nullptr);
  if (lineGog)
  {
    rv += SDK_ASSERT(lineGog->shape() == simVis::GOG::GOG_LINE);
    rv += SDK_ASSERT(lineGog->getDraw());
    rv += SDK_ASSERT(lineGog->osgNode()->getName() == "line 1");
    rv += testLineState(lineGog);

    // test that line doesn't have invalid attributes
    std::string font;
    int fontSize;
    osg::Vec4f fontColor;
    rv += SDK_ASSERT(lineGog->getFont(font, fontSize, fontColor) != 0);

    // test geometry, osgEarth positions are (lon,lat,alt)
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(55.28, 26.13568698, 5000.0));
    points.push_back(osg::Vec3d(55.0, 26.0, 5000.0));
    rv += testFeatureGeometry(lineGog->osgNode(), points);
  }

  // test relative line
  std::string lineRelGogFile = FILE_VERSION +
    "start\n line\n xyz 500 500 0\n xyz -500 50 0\n rangeunits m\n" + LINE_ATTRIBUTES + "3d name line relative 1\n" +
    TEXT_ATTRIBUTES + // add some invalid items
    "end\n";
  simVis::GOG::GogNodeInterfacePtr lineRelGog = (useCore ? parseGogFileWithCore(true, lineRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, lineRelGogFile, rv));
  rv += SDK_ASSERT(lineRelGog != nullptr);
  if (lineRelGog)
  {
    rv += SDK_ASSERT(lineRelGog->shape() == simVis::GOG::GOG_LINE);
    rv += SDK_ASSERT(lineRelGog->getDraw());
    rv += SDK_ASSERT(lineRelGog->osgNode()->getName() == "line relative 1");
    rv += testLineState(lineRelGog);

    // test geometry
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(500.0, 500.0, 0.0));
    points.push_back(osg::Vec3d(-500.0, 50.0, 0.0));
    rv += testLocalGeometry(lineRelGog->osgNode(), points);
  }

  // test polygon
  std::string polyGogFile = FILE_VERSION +
    "start\n poly\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\n 3d name poly 1\n altitudeunits m\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr polyGog = (useCore ? parseGogFileWithCore(false, polyGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, polyGogFile, rv));
  rv += SDK_ASSERT(polyGog != nullptr);
  if (polyGog)
  {
    rv += SDK_ASSERT(polyGog->shape() == simVis::GOG::GOG_POLYGON);
    rv += SDK_ASSERT(polyGog->getDraw());
    rv += SDK_ASSERT(polyGog->osgNode()->getName() == "poly 1");
    rv += testLineState(polyGog);
    rv += testFillState(polyGog);

    // test geometry, osgEarth positions are (lon,lat,alt)
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(53.2, 25.2, 10.0));
    points.push_back(osg::Vec3d(54.1, 22.3, 10.0));
    points.push_back(osg::Vec3d(53.8, 24.1, 10.0));
    rv += testFeatureGeometry(polyGog->osgNode(), points);
  }

  // test relative polygon
  std::string polyRelGogFile = FILE_VERSION +
    "start\n poly\n xyz 100 200 0\n xyz -100 100 0\n xyz -100 -200 0\n 3d name poly relative 1\n rangeunits m\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr polyRelGog = (useCore ? parseGogFileWithCore(true, polyRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, polyRelGogFile, rv));
  rv += SDK_ASSERT(polyRelGog != nullptr);
  if (polyRelGog)
  {
    rv += SDK_ASSERT(polyRelGog->shape() == simVis::GOG::GOG_POLYGON);
    rv += SDK_ASSERT(polyRelGog->getDraw());
    rv += SDK_ASSERT(polyRelGog->osgNode()->getName() == "poly relative 1");
    rv += testLineState(polyRelGog);
    rv += testFillState(polyRelGog);

    // test geometry
    std::vector<osg::Vec3d> points;
    points.push_back(osg::Vec3d(100.0, 200.0, 0.0));
    points.push_back(osg::Vec3d(-100.0, 100.0, 0.0));
    points.push_back(osg::Vec3d(-100.0, -200., 0.0));
    rv += testLocalGeometry(polyRelGog->osgNode(), points);
  }

  // test circle
  std::string circleGogFile = FILE_VERSION +
    "start\n circle\n centerlla 25.2 53.2 0.\n radius 500\n linewidth 6\n 3d name circle 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr circleGog = (useCore ? parseGogFileWithCore(false, circleGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, circleGogFile, rv));
  rv += SDK_ASSERT(circleGog != nullptr);
  if (circleGog)
  {
    rv += SDK_ASSERT(circleGog->shape() == simVis::GOG::GOG_CIRCLE);
    rv += SDK_ASSERT(circleGog->getDraw());
    rv += SDK_ASSERT(circleGog->osgNode()->getName() == "circle 1");
    rv += testLineState(circleGog);
    rv += testFillState(circleGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(circleGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 25.2, 0.0)));
  }

  // test relative circle
  std::string circleRelGogFile = FILE_VERSION +
    "start\n circle\n centerxyz 0 0 0\n radius 500\n linewidth 6\n 3d name circle relative 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr circleRelGog = (useCore ? parseGogFileWithCore(true, circleRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, circleRelGogFile, rv));
  rv += SDK_ASSERT(circleRelGog != nullptr);
  if (circleRelGog)
  {
    rv += SDK_ASSERT(circleRelGog->shape() == simVis::GOG::GOG_CIRCLE);
    rv += SDK_ASSERT(circleRelGog->getDraw());
    rv += SDK_ASSERT(circleRelGog->osgNode()->getName() == "circle relative 1");
    rv += testLineState(circleRelGog);
    rv += testFillState(circleRelGog);

    // test center point
    osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(circleRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(0.0, 0.0, 0.0)));
  }

  // test arc
  std::string arcGogFile = FILE_VERSION +
    "start\n arc\n centerlla 25.2 53.2 0.\n radius 500\n anglestart 44.3\n angledeg 36.7\n 3d name arc 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr arcGog = (useCore ? parseGogFileWithCore(false, arcGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, arcGogFile, rv));
  rv += SDK_ASSERT(arcGog != nullptr);
  if (arcGog)
  {
    rv += SDK_ASSERT(arcGog->shape() == simVis::GOG::GOG_ARC);
    rv += SDK_ASSERT(arcGog->getDraw());
    rv += SDK_ASSERT(arcGog->osgNode()->getName() == "arc 1");
    rv += testLineState(arcGog);
    rv += testFillState(arcGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(arcGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(ctrPoint == osg::Vec3d(53.2, 25.2, 0.0));
  }

  // test relative arc
  std::string arcRelGogFile = FILE_VERSION +
    "start\n arc\n centerxyz 500 500 0\n radius 500\n anglestart 44.3\n angledeg 36.7\n 3d name arc relative 1\n rangeunits m\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr arcRelGog = (useCore ? parseGogFileWithCore(true, arcRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, arcRelGogFile, rv));
  rv += SDK_ASSERT(arcRelGog != nullptr);
  if (arcRelGog)
  {
    rv += SDK_ASSERT(arcRelGog->shape() == simVis::GOG::GOG_ARC);
    rv += SDK_ASSERT(arcRelGog->getDraw());
    rv += SDK_ASSERT(arcRelGog->osgNode()->getName() == "arc relative 1");
    rv += testLineState(arcRelGog);
    rv += testFillState(arcRelGog);

    // test center point
    osg::Group* groupNode = dynamic_cast<osg::Group*>(arcRelGog->osgNode());
    // arcs have 2 nodes: outline and fill
    rv += SDK_ASSERT(groupNode->getNumChildren() == 2);
    if (groupNode)
    {
      // Arc is made up of multiple LocalGeometryNodes
      osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(groupNode->getChild(0));
      rv += SDK_ASSERT(gogNode != nullptr);
      if (gogNode)
        rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(500.0, 500.0, 0.0)));
    }
  }

  // test cylinder
  std::string cylGogFile = FILE_VERSION +
    "start\n cylinder\n centerlla 25.2 53.2 0.\n radius 500\n height 340\n anglestart 44.3\n angledeg 36.7\n 3d name cyl 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr cylGog = (useCore ? parseGogFileWithCore(false, cylGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, cylGogFile, rv));
  rv += SDK_ASSERT(cylGog != nullptr);
  if (cylGog)
  {
    rv += SDK_ASSERT(cylGog->shape() == simVis::GOG::GOG_CYLINDER);
    rv += SDK_ASSERT(cylGog->getDraw());
    rv += SDK_ASSERT(cylGog->osgNode()->getName() == "cyl 1");
    rv += testLineState(cylGog);
    rv += testFillState(cylGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(arcGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 25.2, 0.0)));
  }

  // test relative cylinder
  std::string cylRelGogFile = FILE_VERSION +
    "start\n cylinder\n centerxyz 500 -200 10\n radius 500\n height 340\n anglestart 44.3\n angledeg 36.7\n 3d name cyl relative 1\n rangeunits m\n altitudeunits m\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr cylRelGog = (useCore ? parseGogFileWithCore(true, cylRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, cylRelGogFile, rv));
  rv += SDK_ASSERT(cylRelGog != nullptr);
  if (cylRelGog)
  {
    rv += SDK_ASSERT(cylRelGog->shape() == simVis::GOG::GOG_CYLINDER);
    rv += SDK_ASSERT(cylRelGog->getDraw());
    rv += SDK_ASSERT(cylRelGog->osgNode()->getName() == "cyl relative 1");
    rv += testLineState(cylRelGog);
    rv += testFillState(cylRelGog);

    // test center point
    osg::Group* groupNode = dynamic_cast<osg::Group*>(cylRelGog->osgNode());
    // cylinders have 3 nodes: top, side, bottom
    rv += SDK_ASSERT(groupNode->getNumChildren() == 3);
    if (groupNode)
    {
      // cylinder is made up of multiple LocalGeometryNodes
      osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(groupNode->getChild(0));
      rv += SDK_ASSERT(gogNode != nullptr);
      if (gogNode)
        rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(500.0, -200.0, 10.0)));
    }
  }

  // test ellipse
  std::string ellipseGogFile = FILE_VERSION +
    "start\n ellipse\n centerlla 25.2 53.2 10.\n majoraxis 500\n minoraxis 200\n 3d name ellipse 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr ellipseGog = (useCore ? parseGogFileWithCore(false, ellipseGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, ellipseGogFile, rv));
  rv += SDK_ASSERT(ellipseGog != nullptr);
  if (ellipseGog)
  {
    rv += SDK_ASSERT(ellipseGog->shape() == simVis::GOG::GOG_ELLIPSE);
    rv += SDK_ASSERT(ellipseGog->getDraw());
    rv += SDK_ASSERT(ellipseGog->osgNode()->getName() == "ellipse 1");
    rv += testLineState(ellipseGog);
    rv += testFillState(ellipseGog);
  }

  // test relative ellipse
  std::string ellipseRelGogFile = FILE_VERSION +
    "start\n ellipse\n centerxyz 0 0 0\n majoraxis 500\n minoraxis 200\n 3d name ellipse relative 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr ellipseRelGog = (useCore ? parseGogFileWithCore(true, ellipseRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, ellipseRelGogFile, rv));
  rv += SDK_ASSERT(ellipseRelGog != nullptr);
  if (ellipseRelGog)
  {
    rv += SDK_ASSERT(ellipseRelGog->shape() == simVis::GOG::GOG_ELLIPSE);
    // draw defaults to on
    rv += SDK_ASSERT(ellipseRelGog->getDraw());
    rv += SDK_ASSERT(ellipseRelGog->osgNode()->getName() == "ellipse relative 1");
    rv += testLineState(ellipseRelGog);
    rv += testFillState(ellipseRelGog);
  }

  // test sphere
  std::string sphereGogFile = FILE_VERSION +
    "start\n sphere\n centerlla 25.2 53.4 0.\n radius 200\n 3d name sphere 1\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr sphereGog = (useCore ? parseGogFileWithCore(false, sphereGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, sphereGogFile, rv));
  rv += SDK_ASSERT(sphereGog != nullptr);
  if (sphereGog)
  {
    rv += SDK_ASSERT(sphereGog->shape() == simVis::GOG::GOG_SPHERE);
    rv += SDK_ASSERT(sphereGog->getDraw());
    rv += SDK_ASSERT(sphereGog->osgNode()->getName() == "sphere 1");
    rv += testFillState(sphereGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(sphereGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.4, 25.2, 0.0)));
  }

  // test relative sphere
  std::string sphereRelGogFile = FILE_VERSION +
    "start\n sphere\n centerxyz 5 0 0\n radius 200\n 3d name sphere relative 1\n rangeunits m\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr sphereRelGog = (useCore ? parseGogFileWithCore(true, sphereRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, sphereRelGogFile, rv));
  rv += SDK_ASSERT(sphereRelGog != nullptr);
  if (sphereRelGog)
  {
    rv += SDK_ASSERT(sphereRelGog->shape() == simVis::GOG::GOG_SPHERE);
    rv += SDK_ASSERT(sphereRelGog->getDraw());
    rv += SDK_ASSERT(sphereRelGog->osgNode()->getName() == "sphere relative 1");
    rv += testFillState(sphereRelGog);

    // test center point
    osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(sphereRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(5.0, 0.0, 0.0)));
  }

  // test hemisphere
  std::string hemisphereGogFile = FILE_VERSION +
    "start\n hemisphere\n centerlla 26.2 53.2 0.\n radius 200\n 3d name hemisphere 1\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr hemisphereGog = (useCore ? parseGogFileWithCore(false, hemisphereGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, hemisphereGogFile, rv));
  rv += SDK_ASSERT(hemisphereGog != nullptr);
  if (hemisphereGog)
  {
    rv += SDK_ASSERT(hemisphereGog->shape() == simVis::GOG::GOG_HEMISPHERE);
    rv += SDK_ASSERT(hemisphereGog->getDraw());
    rv += SDK_ASSERT(hemisphereGog->osgNode()->getName() == "hemisphere 1");
    rv += testFillState(hemisphereGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(hemisphereGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 26.2, 0.0)));
  }

  // test relative hemisphere
  std::string hemisphereRelGogFile = FILE_VERSION +
    "start\n hemisphere\n centerxyz 0 50 0\n radius 200\n 3d name hemisphere relative 1\n rangeunits m\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr hemisphereRelGog = (useCore ? parseGogFileWithCore(true, hemisphereRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, hemisphereRelGogFile, rv));
  rv += SDK_ASSERT(hemisphereRelGog != nullptr);
  if (hemisphereRelGog)
  {
    rv += SDK_ASSERT(hemisphereRelGog->shape() == simVis::GOG::GOG_HEMISPHERE);
    rv += SDK_ASSERT(hemisphereRelGog->getDraw());
    rv += SDK_ASSERT(hemisphereRelGog->osgNode()->getName() == "hemisphere relative 1");
    rv += testFillState(hemisphereRelGog);

    // test center point
    osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(hemisphereRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(0.0, 50.0, 0.0)));
  }

  // test ellipsoid
  std::string ellipsoidGogFile = FILE_VERSION +
    "start\n ellipsoid\n centerlla 25.2 53.2 10.\n majoraxis 500\n minoraxis 200\n height 300\n 3d name ellipsoid 1\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr ellipsoidGog = (useCore ? parseGogFileWithCore(false, ellipsoidGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, ellipsoidGogFile, rv));
  rv += SDK_ASSERT(ellipsoidGog != nullptr);
  if (ellipsoidGog)
  {
    rv += SDK_ASSERT(ellipsoidGog->shape() == simVis::GOG::GOG_ELLIPSOID);
    rv += SDK_ASSERT(ellipsoidGog->getDraw());
    rv += SDK_ASSERT(ellipsoidGog->osgNode()->getName() == "ellipsoid 1");
    rv += testFillState(ellipsoidGog);
  }

  // test relative ellipsoid
  std::string ellipsoidRelGogFile = FILE_VERSION +
    "start\n ellipsoid\n centerxyz 0 0 10.\n majoraxis 500\n minoraxis 200\n height 300\n 3d name ellipsoid relative 1\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr ellipsoidRelGog = (useCore ? parseGogFileWithCore(true, ellipsoidRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, ellipsoidRelGogFile, rv));
  rv += SDK_ASSERT(ellipsoidRelGog != nullptr);
  if (ellipsoidRelGog)
  {
    rv += SDK_ASSERT(ellipsoidRelGog->shape() == simVis::GOG::GOG_ELLIPSOID);
    rv += SDK_ASSERT(ellipsoidRelGog->getDraw());
    rv += SDK_ASSERT(ellipsoidRelGog->osgNode()->getName() == "ellipsoid relative 1");
    rv += testFillState(ellipsoidRelGog);
  }

  // test cone
  std::string coneGogFile = FILE_VERSION +
    "start\n cone\n centerlla 25.8 53.2 0.\n radius 500\n height 340\n 3d name cone 1\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr coneGog = (useCore ? parseGogFileWithCore(false, coneGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, coneGogFile, rv));
  rv += SDK_ASSERT(coneGog != nullptr);
  if (coneGog)
  {
    rv += SDK_ASSERT(coneGog->shape() == simVis::GOG::GOG_CONE);
    rv += SDK_ASSERT(coneGog->getDraw());
    rv += SDK_ASSERT(coneGog->osgNode()->getName() == "cone 1");
    rv += testFillState(coneGog);

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(coneGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 25.8, 0.0)));
  }

  // test relative cone
  std::string coneRelGogFile = FILE_VERSION +
    "start\n cone\n centerxyz 500 -200 0\n radius 500\n height 340\n 3d name cone relative 1\n rangeunits m\n" + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr coneRelGog = (useCore ? parseGogFileWithCore(true, coneRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, coneRelGogFile, rv));
  rv += SDK_ASSERT(coneRelGog != nullptr);
  if (coneRelGog)
  {
    rv += SDK_ASSERT(coneRelGog->shape() == simVis::GOG::GOG_CONE);
    rv += SDK_ASSERT(coneRelGog->getDraw());
    rv += SDK_ASSERT(coneRelGog->osgNode()->getName() == "cone relative 1");
    rv += testFillState(coneRelGog);

    // test center point
    osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(coneRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(500.0, -200.0, 0.0)));
  }

  // test orbit
  std::string orbitGogFile = FILE_VERSION +
    "start\n orbit\n centerlla 25.2 53.2 0.\n centerll2 26. 54.3\n radius 500\n 3d name orbit 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr orbitGog = (useCore ? parseGogFileWithCore(false, orbitGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, orbitGogFile, rv));
  rv += SDK_ASSERT(orbitGog != nullptr);
  if (orbitGog)
  {
    rv += SDK_ASSERT(orbitGog->shape() == simVis::GOG::GOG_ORBIT);
    rv += SDK_ASSERT(orbitGog->getDraw());
    rv += SDK_ASSERT(orbitGog->osgNode()->getName() == "orbit 1");
    rv += testLineState(orbitGog);
    rv += testFillState(orbitGog);

    // test center point, orbit uses centerlla as center
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(orbitGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 25.2, 0.0)));
  }

  // test relative orbit
  std::string orbitRelGogFile = FILE_VERSION +
    "start\n orbit\n centerxyz 500 -200 0\n centerxy2 600 200 radius 500\n 3d name orbit relative 1\n rangeunits m\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr orbitRelGog = (useCore ? parseGogFileWithCore(true, orbitRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, orbitRelGogFile, rv));

  rv += SDK_ASSERT(orbitRelGog != nullptr);
  if (orbitRelGog)
  {
    rv += SDK_ASSERT(orbitRelGog->shape() == simVis::GOG::GOG_ORBIT);
    rv += SDK_ASSERT(orbitRelGog->getDraw());
    rv += SDK_ASSERT(orbitRelGog->osgNode()->getName() == "orbit relative 1");
    rv += testLineState(orbitRelGog);
    rv += testFillState(orbitRelGog);

    // test center point
    osgEarth::LocalGeometryNode* gogNode = dynamic_cast<osgEarth::LocalGeometryNode*>(orbitRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(500.0, -200.0, 0.0)));
  }

  // test annotation
  std::string annotationGogFile = FILE_VERSION +
    "start\n annotation label 1\n centerlla 25.2 53.2 0.\n linecolor hex 0xffff00ff\n" + TEXT_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr annotationGog = (useCore ? parseGogFileWithCore(false, annotationGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, annotationGogFile, rv));
  rv += SDK_ASSERT(annotationGog != nullptr);
  if (annotationGog)
  {
    rv += SDK_ASSERT(annotationGog->shape() == simVis::GOG::GOG_ANNOTATION);
    rv += SDK_ASSERT(annotationGog->getDraw());
    rv += SDK_ASSERT(annotationGog->osgNode()->getName() == "label 1");

    std::string font;
    int fontSize = 0;
    osg::Vec4f fontColor;
    rv += SDK_ASSERT(annotationGog->getFont(font, fontSize, fontColor) == 0);
    rv += SDK_ASSERT(fontSize == 32);
// test font file on windows only, since linux fonts are too unreliable
#ifdef WIN32
    rv += SDK_ASSERT(font.find("georgia.ttf") != std::string::npos);
#endif
    rv += SDK_ASSERT(fontColor == osg::Vec4f(1.0, 0, 1.0, 1.0));

    // test center point
    osg::Vec3d ctrPoint;
    rv += SDK_ASSERT(annotationGog->getPosition(ctrPoint) == 0);
    rv += SDK_ASSERT(comparePositions(ctrPoint, osg::Vec3d(53.2, 25.2, 0.0)));
  }

  // test relative annotation
  std::string annotationRelGogFile = FILE_VERSION +
    "start\n annotation label relative 1\n centerxyz 10 0 0.\n linecolor hex 0xffff00ff\n rangeunits m\n" + TEXT_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr annotationRelGog = (useCore ? parseGogFileWithCore(true, annotationRelGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_HOSTED, annotationRelGogFile, rv));
  rv += SDK_ASSERT(annotationRelGog != nullptr);
  if (annotationRelGog)
  {
    rv += SDK_ASSERT(annotationRelGog->shape() == simVis::GOG::GOG_ANNOTATION);
    rv += SDK_ASSERT(annotationRelGog->getDraw());
    rv += SDK_ASSERT(annotationRelGog->osgNode()->getName() == "label relative 1");

    std::string font;
    int fontSize = 0;
    osg::Vec4f fontColor;
    rv += SDK_ASSERT(annotationRelGog->getFont(font, fontSize, fontColor) == 0);
    rv += SDK_ASSERT(fontSize == 32);
// test font file on windows only, since linux fonts are too unreliable
#ifdef WIN32
    rv += SDK_ASSERT(font.find("georgia.ttf") != std::string::npos);
#endif
    rv += SDK_ASSERT(fontColor == osg::Vec4f(1.0, 0, 1.0, 1.0));

    // test center point
    osgEarth::GeoPositionNode* gogNode = dynamic_cast<osgEarth::GeoPositionNode*>(annotationRelGog->osgNode());
    rv += SDK_ASSERT(gogNode != nullptr);
    if (gogNode)
      rv += SDK_ASSERT(comparePositions(gogNode->getLocalOffset(), osg::Vec3d(10.0, 0.0, 0.0)));
  }

  // test special case of nested annotations
  std::string annotationNestedGogFile = FILE_VERSION +
    "start\n annotation label 1\n centerxyz 0 0 0.\n linecolor hex 0xffff00ff\n" + TEXT_ATTRIBUTES
    + "annotation label 2\n centerxyz 100 10 0\n annotation label 3\n centerxyz 10 200 0\nend\n";
  simVis::GOG::Parser::OverlayNodeVector gogs;
  std::vector<simVis::GOG::GogFollowData> followData;
  std::stringstream input;
  input << annotationNestedGogFile;
  bool parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 3); // verify it found all the nested annotations
  // all the annotations should have the same attributes as the first one found
  for (simVis::GOG::GogNodeInterface* gog : gogs)
  {
    std::string font;
    int fontSize = 0;
    osg::Vec4f fontColor;
    rv += SDK_ASSERT(annotationRelGog->getFont(font, fontSize, fontColor) == 0);
    rv += SDK_ASSERT(fontSize == 32);
// test font file on windows only, since linux fonts are too unreliable
#ifdef WIN32
    rv += SDK_ASSERT(font.find("georgia.ttf") != std::string::npos);
#endif
    rv += SDK_ASSERT(fontColor == osg::Vec4f(1.0, 0, 1.0, 1.0));
  }
  clearItems(gogs, followData, input);

  // test lat lon alt box
  const std::string llabGogFile = FILE_VERSION +
    "start\n latlonaltbox 26.13568698 26.130 55.27931357 55.270 0. 1000\n 3d name llab 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr llabGog = (useCore ? parseGogFileWithCore(false, llabGogFile, rv) : parseGogFile(parser, simVis::GOG::GOGNODE_GEOGRAPHIC, llabGogFile, rv));
  rv += SDK_ASSERT(llabGog != nullptr);
  if (llabGog)
  {
    rv += SDK_ASSERT(llabGog->shape() == simVis::GOG::GOG_LATLONALTBOX);
    rv += SDK_ASSERT(llabGog->getDraw());
    rv += SDK_ASSERT(llabGog->osgNode()->getName() == "llab 1");
    rv += testLineState(llabGog);
    rv += testFillState(llabGog);

    // test geometry, osgEarth positions are (lon,lat,alt)
    osg::Group* groupNode = dynamic_cast<osg::Group*>(llabGog->osgNode());
    // LatLonAltBox has 2 nodes, front/right/top and back/left/bottom
    rv += SDK_ASSERT(groupNode->getNumChildren() == 2);
    if (groupNode)
    {
      // first child node is the back/left/bottom
      osgEarth::FeatureNode* gogNode = dynamic_cast<osgEarth::FeatureNode*>(groupNode->getChild(0));
      rv += SDK_ASSERT(gogNode != nullptr);
      if (gogNode)
      {
        std::vector<osg::Vec3d> points;
        points.push_back(osg::Vec3d(55.27931357, 26.13568698, 0.));
        points.push_back(osg::Vec3d(55.27, 26.13568698, 0.));
        points.push_back(osg::Vec3d(55.27931357, 26.13, 0.));
        points.push_back(osg::Vec3d(55.27, 26.13, 0.));

        size_t numPoints = 0;
        osgEarth::Geometry* geom = gogNode->getFeature()->getGeometry();
        // LatLonAltBox is created with MultiGeometry, so need geometry iterator
        osgEarth::GeometryIterator iter(geom, false);
        size_t altIndex = 0;
        while (iter.hasMore())
        {
          osgEarth::Geometry* part = iter.next();
          for (size_t i = 0; i < part->size(); ++i)
          {
            for (auto pointIter = points.begin(); pointIter != points.end(); ++pointIter)
            {
              if (comparePositions(*pointIter, (*part)[i]))
                numPoints++;
            }
          }
        }
        // check that all points were in the Geometry, some duplication exists in the multiple Geometries
        rv += SDK_ASSERT(numPoints >= points.size());
      }
    }
  }

  return rv;
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
  clearItems(gogs, followData, input);

  // Test loading absolute GOGs as attached
  input << absoluteGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 0); // verify no absolute shapes were loaded as attached
  clearItems(gogs, followData, input);

  std::string relativeGog = FILE_VERSION +
    "start\n line\n xyz 100 10 50\n xyz -100 44 50\n end\n"
    "start\n poly\n xyz 0 -60 100\n xyz 100 34 100\n xyz -100 -20 100\n end\n";

  // Test loading relative GOGs as attached
  input << relativeGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative shapes were loaded as attached
  clearItems(gogs, followData, input);

  // Test loading relative GOGs as un-attached
  input << relativeGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative shapes were loaded as un-attached
  clearItems(gogs, followData, input);

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
  clearItems(gogs, followData, input);

  // Test loading GOG with relative and absolute shapes as un-attached
  input << relativeAndAbsoluteGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 2); // verify relative and absolute shapes were loaded as un-attached
  clearItems(gogs, followData, input);

  // create a relative shape with relative points first, then some absolute points
  std::string relativeShapeWithAbsolutePoints = FILE_VERSION +
    "start\n poly\nxyz 1 1 1\nxyz 1 -1 1\nxyz -1 1 -1\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\n end\n";

  // Test loading GOG shape with relative and absolute points, relative first, as attached. Should work, since relative points are first, so shape should be read as relative
  input << relativeShapeWithAbsolutePoints;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1); // verify only 1 shape was loaded as attached
  std::ostringstream os2;
  gogs.front()->serializeToStream(os2);
  rv += SDK_ASSERT(os2.str().find("lla") == std::string::npos); // verify the shape was loaded as relative, since those were the first points found
  clearItems(gogs, followData, input);

  // create an absolute shape with absolute points first, then some relative points
  std::string absoluteShapeWithRelativePoints = FILE_VERSION +
    "start\n poly\n lla 25.2 53.2 10.\n lla 22.3 54.1 10.\n lla 24.1 53.8 10.\nxyz 1 1 1\nxyz 1 -1 1\nxyz -1 1 -1\nend\n";

  // Test loading GOG shape with relative and absolute points, absolute first, as absolute. Should succeed, since absolute points are first, so shape should be read as absolute
  input << absoluteShapeWithRelativePoints;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1); // verify the shape loaded
  std::ostringstream os3;
  gogs.front()->serializeToStream(os3);
  rv += SDK_ASSERT(os3.str().find("xyz") == std::string::npos); // verify the shape was not loaded as relative
  clearItems(gogs, followData, input);

  // Test loading GOG shape with relative and absolute points, absolute first, as relative. Should fail, since absolute points are first, so shape should be read as absolute
  input << absoluteShapeWithRelativePoints;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_HOSTED, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 0); // verify the shape failed to load
  clearItems(gogs, followData, input);

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


bool findTokenValue(std::stringstream& ss, const std::string& token, const std::string& value)
{
  std::vector<std::string> tokens;
  std::string line;
  while (simCore::getStrippedLine(ss, line))
  {
    simCore::quoteTokenizer(tokens, line);
    line = simCore::lowerCase(line);
    // 3d is a special case that represents a GOG keyword made up of multiple tokens
    if (tokens[0] == "3d")
    {
      if (tokens.size() < 3)
        return false;
      if ((tokens[0] + " " + tokens[1]) == token)
        return tokens[2] == value;
    }
    else if (tokens[0] == token)
    {
      if (tokens.size() < 1)
        return false;
      return tokens[1] == value;
    }
  }
  return false;
}

int testAltitudeUnits()
{
  int rv = 0;

  simVis::GOG::Parser parser;
  // loaded GOG shape nodes
  simVis::GOG::Parser::OverlayNodeVector gogs;
  // follow data for attached GOGs
  std::vector<simVis::GOG::GogFollowData> followData;

  std::stringstream input;

  // Test loading line with altitude units specified
  std::string lineGog = FILE_VERSION +
    "start\n line\n lla 22.1 - 159.7 2\n lla 22.1 - 159.3 2\n 3d offsetalt 2\n altitudeUnits km\n end\n";
  input << lineGog;
  bool parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    double altOffset = 0.;
    rv += SDK_ASSERT(gogs.front()->getAltOffset(altOffset) == 0);
    rv += SDK_ASSERT(altOffset == 2000); // value is in meters, verify it still matches 2 km

    std::stringstream serializedLine;
    gogs.front()->serializeToStream(serializedLine);
    // line should always serialize out altitude units as meters, despite input
    rv += SDK_ASSERT(findTokenValue(serializedLine, "altitudeunits", "meters"));
  }
  clearItems(gogs, followData, input);

  // Test loading circle with altitude units specified
  std::string circleGog = FILE_VERSION +
    "start\n circle\n centerlla 22.1 - 159.7 2\n radius 200\n 3d offsetalt 2\n altitudeUnits km\n end\n";
  input << circleGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    double altOffset = 0.;
    rv += SDK_ASSERT(gogs.front()->getAltOffset(altOffset) == 0);
    rv += SDK_ASSERT(altOffset == 2000); // value is in meters, verify it still matches 2 km

    std::stringstream serializedCircle;
    gogs.front()->serializeToStream(serializedCircle);
    // circle should serialize out to the same units it went in as
    rv += SDK_ASSERT(findTokenValue(serializedCircle, "altitudeunits", "km"));
  }
  clearItems(gogs, followData, input);

  // Test loading LatLonAltBox with no altitude units specified, altitude units default to ft
  std::string llabGog = FILE_VERSION +
    "start\n LatLonAltBox 21.945 22.0 -159.454 -159.41 1. 4.\n 3d offsetalt 2000\n  end\n";
  input << llabGog;
  parsedGog = parser.loadGOGs(input, simVis::GOG::GOGNODE_GEOGRAPHIC, gogs, followData);
  rv += SDK_ASSERT(parsedGog); // verify parsing worked
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    double altOffset = 0.;
    rv += SDK_ASSERT(gogs.front()->getAltOffset(altOffset) == 0);
    simCore::Units altMeters(simCore::Units::METERS);
    simCore::Units altFeet(simCore::Units::FEET);
    // alt offset is in meters, verify it is equivalent to 2000 ft
    rv += SDK_ASSERT(simCore::areEqual(altOffset, altFeet.convertTo(altMeters, 2000)));

    // now update altitude offset to 3000 ft
    gogs.front()->setAltOffset(altFeet.convertTo(altMeters, 3000));

    std::stringstream serializedLlab;
    gogs.front()->serializeToStream(serializedLlab);
    // verify that altitude offset serializes out as 3000 ft
    rv += SDK_ASSERT(findTokenValue(serializedLlab, "3d offsetalt", "3000"));
  }
  clearItems(gogs, followData, input);

  return rv;
}

// test that all items are in the serialized string, and that the string contains no more than the number of items
int testItemsInSerialization(const std::string& serialized, const std::vector<std::string>& items)
{
  int rv = 0;
  // verify all the expected items are present in the serialized shape
  for (std::string item : items)
  {
    rv += SDK_ASSERT(serialized.find(item) != std::string::npos);
    if (serialized.find(item) == std::string::npos)
      std::cerr << "Failed to serialize : " << item << "\n";
  }

  // now that all items from serializedItems have been found in the serialized gog, verify that they both contain the same number of items
  std::vector<std::string> lines;
  simCore::escapeTokenize(lines, serialized, true, "\n");
  rv += SDK_ASSERT(lines.size() == items.size());

  if (rv > 0)
    std::cerr << serialized << "\n";
  return rv;
}

// test the line fields for the GOG defined by the gog and shapeItems
int testLineDynamicEdits(simVis::GOG::GogNodeInterface& gog, std::vector<std::string>& shapeItems)
{
  gog.setLineStyle(simVis::GOG::Utils::LINE_DOTTED);
  gog.setLineColor(osg::Vec4f(1.0, 1.0, 0, 1.0));
  gog.setLineWidth(5);
  gog.setOutlineState(false);

  shapeItems.push_back("linestyle dotted\n");
  shapeItems.push_back("linecolor hex 0xff00ffff\n");
  shapeItems.push_back("linewidth 5\n");
  shapeItems.push_back("outline false\n");

  std::ostringstream os;
  gog.serializeToStream(os);
  return testItemsInSerialization(os.str(), shapeItems);
}

int testFillDynamicEdits(simVis::GOG::GogNodeInterface& gog, std::vector<std::string>& shapeItems)
{
  gog.setFilledState(true);
  gog.setFillColor(osg::Vec4f(0, 1.0, 1.0, 1.0));

  shapeItems.push_back("filled\n");
  shapeItems.push_back("fillcolor hex 0xffffff00\n");

  std::ostringstream os;
  gog.serializeToStream(os);
  return testItemsInSerialization(os.str(), shapeItems);
}

// test the general fields for the GOG defined by the gog and shapeItems
int testGeneralDynamicEdits(simVis::GOG::GogNodeInterface& gog, std::vector<std::string>& shapeItems)
{
  gog.setAltitudeMode(simVis::GOG::ALTITUDE_GROUND_CLAMPED);
  gog.setAltOffset(250);
  gog.setDepthBuffer(true);
  gog.setDrawState(false);

  shapeItems.push_back("altitudemode clamptoground\n");
  // note altitude units are in feet
  shapeItems.push_back("3d offsetalt 820.21\n");
  shapeItems.push_back("depthbuffer true\n");
  shapeItems.push_back("off\n");

  std::ostringstream os;
  gog.serializeToStream(os);
  return testItemsInSerialization(os.str(), shapeItems);
}

// test the follow fields for the GOG defined by the gog and shapeItems
int testFollowDynamicEdits(simVis::GOG::GogNodeInterface& gog, std::vector<std::string>& shapeItems)
{
  gog.setFollowYaw(true);
  gog.setFollowRoll(true);
  gog.setYawOffset(45.1 * simCore::DEG2RAD);
  gog.setPitchOffset(0.31 * simCore::DEG2RAD);
  gog.setRollOffset(22.3 * simCore::DEG2RAD);

  shapeItems.push_back("3d follow cr\n");
  shapeItems.push_back("3d offsetcourse 45.1\n");
  shapeItems.push_back("3d offsetpitch 0.31\n");
  shapeItems.push_back("3d offsetroll 22.3\n");

  std::ostringstream os;
  gog.serializeToStream(os);
  return testItemsInSerialization(os.str(), shapeItems);
}

// test a basic shape that supports fill fields, and alternately supports line fields
int testBasicGog(std::vector<std::string>& shapeItems, bool testLined, bool testFollow)
{
  int rv = 0;
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);
  simVis::GOG::Loader::GogNodeVector gogs;

  std::stringstream gogStr;
  gogStr << "start\n";
  for (std::string item : shapeItems)
    gogStr << item;
  gogStr << "end\n";
  shapeItems.push_back("start\n");
  shapeItems.push_back("end\n");
  loader.loadGogs(gogStr, false, gogs);
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    simVis::GOG::GogNodeInterfacePtr gog = gogs.front();

    // first check that serialization doesn't contain anything extra
    std::stringstream blank;
    gog->serializeToStream(blank);
    rv += testItemsInSerialization(blank.str(), shapeItems);
    if (testLined)
      rv += testLineDynamicEdits(*gog.get(), shapeItems);
    rv += testFillDynamicEdits(*gog.get(), shapeItems);
    rv += testGeneralDynamicEdits(*gog.get(), shapeItems);
    if (testFollow)
      rv += testFollowDynamicEdits(*gog.get(), shapeItems);
  }
  return rv;
}

// test that changes to the GOG are reflected in the serialized output
int testDynamicEdits()
{
  int rv = 0;
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("circle\n");
    rv += testBasicGog(shapeItems, true, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("arc\n");
    shapeItems.push_back("anglestart 0\n");
    shapeItems.push_back("angledeg 1\n");
    rv += testBasicGog(shapeItems, true, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("ellipse\n");
    rv += testBasicGog(shapeItems, true, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("cylinder\n");
    rv += testBasicGog(shapeItems, true, true);
  }

  std::vector<std::string> pointItems;
  pointItems.push_back("lla 23.1 123 0\n");
  pointItems.push_back("lla 23.2 123.1 0\n");
  pointItems.push_back("lla 23.3 123 0\n");
  pointItems.push_back("lla 23.4 123.4 0\n");

  {
    std::vector<std::string> shapeItems = pointItems;
    shapeItems.push_back("line\n");
    rv += testBasicGog(shapeItems, true, false);
  }
  {
    std::vector<std::string> shapeItems = pointItems;
    shapeItems.push_back("linesegs\n");
    rv += testBasicGog(shapeItems, true, false);
  }
  {
    std::vector<std::string> shapeItems = pointItems;
    shapeItems.push_back("polygon\n");
    rv += testBasicGog(shapeItems, true, false);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("sphere\n");
    rv += testBasicGog(shapeItems, false, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("hemisphere\n");
    rv += testBasicGog(shapeItems, false, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("ellipsoid\n");
    rv += testBasicGog(shapeItems, false, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("cone\n");
    rv += testBasicGog(shapeItems, false, true);
  }
  {
    std::vector<std::string> shapeItems;
    shapeItems.push_back("orbit\n");
    shapeItems.push_back("centerlla 24.2 45.2 0\n");
    shapeItems.push_back("centerll2 24.3 45.1\n");
    rv += testBasicGog(shapeItems, false, true);
  }

  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);
  simVis::GOG::Loader::GogNodeVector gogs;

  // test annotation
  std::vector<std::string> annoItems;
  annoItems.push_back("start\n");
  annoItems.push_back("end\n");
  annoItems.push_back("annotation some text\n");
  std::stringstream annoGog;
  annoGog << "start\n annotation some text\n end\n";
  loader.loadGogs(annoGog, false, gogs);
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    simVis::GOG::GogNodeInterfacePtr gog = gogs.front();

    // first check that serialization doesn't contain anything extra
    std::stringstream blank;
    gog->serializeToStream(blank);
    rv += testItemsInSerialization(blank.str(), annoItems);

    gog->setFont("georgia.ttf", 32, osg::Vec4f(.6, 1., 0., 1.));
    annoItems.push_back("fontname georgia.ttf\n");
    annoItems.push_back("fontsize 32\n");
    annoItems.push_back("linecolor hex 0xff00ff99\n");
    std::stringstream annoGog1;
    gog->serializeToStream(annoGog1);
    rv += testItemsInSerialization(annoGog1.str(), annoItems);

    gog->setTextOutline(osg::Vec4f(1.0, 1.0, 0, 0), simData::TO_THICK);
    annoItems.push_back("textoutlinecolor hex 0x0000ffff\n");
    annoItems.push_back("textoutlinethickness thick\n");
    std::stringstream annoGog2;
    gog->serializeToStream(annoGog2);
    rv += testItemsInSerialization(annoGog2.str(), annoItems);

    rv += testGeneralDynamicEdits(*gog.get(), annoItems);
  }
  gogs.clear();

  // test latlonaltbox
  std::vector<std::string> llabItems;
  llabItems.push_back("start\n");
  llabItems.push_back("end\n");
  llabItems.push_back("latlonaltbox 24.2 23.4 55.6 55.2 0\n");
  std::stringstream llabGog;
  llabGog << "start\n latlonaltbox 24.2 23.4 55.6 55.2 0\n end\n";
  loader.loadGogs(llabGog, false, gogs);
  rv += SDK_ASSERT(gogs.size() == 1);
  if (!gogs.empty())
  {
    simVis::GOG::GogNodeInterfacePtr gog = gogs.front();

    // first check that serialization doesn't contain anything extra
    std::stringstream blank;
    gog->serializeToStream(blank);
    rv += testItemsInSerialization(blank.str(), llabItems);
    rv += testLineDynamicEdits(*gog.get(), llabItems);
    rv += testFillDynamicEdits(*gog.get(), llabItems);
    rv += testGeneralDynamicEdits(*gog.get(), llabItems);
  }
  gogs.clear();


  return rv;
}

// test different ways to define arcs, to verify arcs with 0 sweep are not created
int testArcSweep()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);
  simVis::GOG::Loader::GogNodeVector gogs;

  // test same start/end angles, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n anglend 0\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test same start/end angles, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n anglend 360\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test same start/end angles, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 45\n anglend 405\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test 0 sweep, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg 0\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg 360\n end\n";
    loader.loadGogs(shape, false, gogs);
  }

  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg -360\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 52.5\n angleDeg -360\n end\n";
    loader.loadGogs(shape, false, gogs);
  }
  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  return rv;
}

}

int GogTest(int argc, char* argv[])
{
  int rv = 0;

  // Check the SIMDIS SDK version
  simCore::checkVersionThrow();

  // Run tests
  rv += testShapes(false);
  rv += testShapes(true);
  rv += testLoadRelativeAndAbsolute();
  rv += testParseMetaData();
  rv += testAltitudeUnits();
  rv += testDynamicEdits();
  rv += testArcSweep();

  // Shut down protobuf lib for valgrind testing
  google::protobuf::ShutdownProtobufLibrary();
  // Need to destroy simVis Registry for valgrind testing
  simVis::Registry::destroy();

  return rv;
}
