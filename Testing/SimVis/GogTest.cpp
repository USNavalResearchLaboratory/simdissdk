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
#include <sstream>
#include <string>
#include <vector>
#include "gdal.h"
#include "osgEarth/FeatureNode"
#include "osgEarth/GeoPositionNode"
#include "osgEarth/LabelNode"
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
#include "simCore/String/Utils.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Loader.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/Headless.h"
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

// returns the first GOG shape parsed from the specified string, using the simCore::GOG::Parser
simVis::GOG::GogNodeInterfacePtr parseGogFileWithCore(bool attached, const std::string& gog, int& rv)
{
  std::stringstream input;
  input << gog;
  simCore::GOG::Parser parser;
  simVis::GOG::Loader loader(parser);
  simVis::GOG::Loader::GogNodeVector gogs;
  loader.loadGogs(input, "", attached, gogs);

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

/** Searches for a PNG file in a couple of environment variables, for image overlay testing */
std::string getValidPngFile()
{
  if (getenv("SIMDIS_SDK_FILE_PATH"))
  {
    const std::string& rv = simCore::getEnvVar("SIMDIS_SDK_FILE_PATH") + "/textures/reticle.png";
    std::ifstream ifs(rv.c_str(), std::ios::binary);
    if (ifs.good())
      return rv;
  }
  if (getenv("SIMDIS_DIR"))
  {
    const std::string& rv = simCore::getEnvVar("SIMDIS_DIR") + "/data/textures/app/compass.png";
    std::ifstream ifs(rv.c_str(), std::ios::binary);
    if (ifs.good())
      return rv;
  }
  return "";
}

int testShapes()
{
  int rv = 0;
  // test points
  std::string pointGogFile = FILE_VERSION +
    "start\n points\n lla 24.1 44.3 0.\n lla \"26.0 N\" \"55.0 E\" 8.\n pointsize 24\n 3d name point 1\n altitudeunits m\n" +
    TEXT_ATTRIBUTES + " end\n"; // add some invalid items

  simVis::GOG::GogNodeInterfacePtr pointGog = parseGogFileWithCore(false, pointGogFile, rv);
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

  simVis::GOG::GogNodeInterfacePtr pointRelGog = parseGogFileWithCore(true, pointRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr lineGog = parseGogFileWithCore(false, lineGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr lineRelGog = parseGogFileWithCore(true, lineRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr polyGog = parseGogFileWithCore(false, polyGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr polyRelGog = parseGogFileWithCore(true, polyRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr circleGog = parseGogFileWithCore(false, circleGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr circleRelGog = parseGogFileWithCore(true, circleRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr arcGog = parseGogFileWithCore(false, arcGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr arcRelGog = parseGogFileWithCore(true, arcRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr cylGog = parseGogFileWithCore(false, cylGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr cylRelGog = parseGogFileWithCore(true, cylRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr ellipseGog = parseGogFileWithCore(false, ellipseGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr ellipseRelGog = parseGogFileWithCore(true, ellipseRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr sphereGog = parseGogFileWithCore(false, sphereGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr sphereRelGog = parseGogFileWithCore(true, sphereRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr hemisphereGog = parseGogFileWithCore(false, hemisphereGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr hemisphereRelGog = parseGogFileWithCore(true, hemisphereRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr ellipsoidGog = parseGogFileWithCore(false, ellipsoidGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr ellipsoidRelGog = parseGogFileWithCore(true, ellipsoidRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr coneGog = parseGogFileWithCore(false, coneGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr coneRelGog = parseGogFileWithCore(true, coneRelGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr orbitGog = parseGogFileWithCore(false, orbitGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr orbitRelGog = parseGogFileWithCore(true, orbitRelGogFile, rv);

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

  // test image overlay (there is no relative, only absolute). Image File will fail if it cannot
  // load the filename provided. Try to use a file from SIMDIS_SDK_FILE_PATH if set, or SIMDIS_DIR
  // if that's set.
  const std::string& validIcon = getValidPngFile();
  if (!validIcon.empty())
  {
    std::string imageOverlayGogFile = FILE_VERSION +
      "start\n imageoverlay 25.1 25.3 55.4 55.6 0\n imagefile " + validIcon + "\n 3d name Image Overlay\n opacity 0.75\n end\n";
    simVis::GOG::GogNodeInterfacePtr imageOverlayGog = parseGogFileWithCore(false, imageOverlayGogFile, rv);
    rv += SDK_ASSERT(imageOverlayGog != nullptr);
    if (imageOverlayGog)
    {
      rv += SDK_ASSERT(imageOverlayGog->shape() == simVis::GOG::GOG_IMAGEOVERLAY);
      rv += SDK_ASSERT(imageOverlayGog->getDraw());
      rv += SDK_ASSERT(imageOverlayGog->osgNode()->getName() == "Image Overlay");
      float opacity = 1.f;
      rv += SDK_ASSERT(imageOverlayGog->getOpacity(opacity) == 0);
      rv += SDK_ASSERT(simCore::areEqual(opacity, 0.75));
      // Do not bother testing getPosition(), it does not get updated until first pass of render
    }
  }

  // test annotation
  std::string annotationGogFile = FILE_VERSION +
    "start\n annotation label 1\n centerlla 25.2 53.2 0.\n linecolor hex 0xffff00ff\n" + TEXT_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr annotationGog = parseGogFileWithCore(false, annotationGogFile, rv);
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
  simVis::GOG::GogNodeInterfacePtr annotationRelGog = parseGogFileWithCore(true, annotationRelGogFile, rv);
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

  // test annotation text special characters
  std::string annotationTextGogFile = FILE_VERSION +
    "start\n annotation label_1\\nnext line\n centerlla 25.2 53.2 0.\nend\n";
  simVis::GOG::GogNodeInterfacePtr annotationTextGog = parseGogFileWithCore(false, annotationTextGogFile, rv);
  rv += SDK_ASSERT(annotationTextGog != nullptr);
  if (annotationTextGog)
  {
    rv += SDK_ASSERT(annotationTextGog->shape() == simVis::GOG::GOG_ANNOTATION);
    rv += SDK_ASSERT(annotationTextGog->getDraw());

    osgEarth::LabelNode* label = dynamic_cast<osgEarth::LabelNode*>(annotationTextGog->osgNode());
    rv += SDK_ASSERT(label);
    if (label)
      rv += SDK_ASSERT(label->getText() == "label 1\nnext line");
  }

  // test lat lon alt box
  const std::string llabGogFile = FILE_VERSION +
    "start\n latlonaltbox 26.13568698 26.130 55.27931357 55.270 0. 1000\n 3d name llab 1\n" + LINE_ATTRIBUTES + FILL_ATTRIBUTES + "end\n";
  simVis::GOG::GogNodeInterfacePtr llabGog = parseGogFileWithCore(false, llabGogFile, rv);
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
  loader.loadGogs(gogStr, "", false, gogs);
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
  loader.loadGogs(annoGog, "", false, gogs);
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

    gog->setTextOutline(osg::Vec4f(1.0, 1.0, 0, 0), simData::TextOutline::TO_THICK);
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
  loader.loadGogs(llabGog, "", false, gogs);
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
    shape << "start\n arc\n anglestart 0\n angleend 0\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test same start/end angles, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleend 360\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test same start/end angles, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 45\n angleend 405\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test 0 sweep, should not create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg 0\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg 360\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }

  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 0\n angleDeg -360\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  // test 360 sweep, should create GOG
  {
    std::stringstream shape;
    shape << "start\n arc\n anglestart 52.5\n angleDeg -360\n end\n";
    loader.loadGogs(shape, "", false, gogs);
  }
  rv += SDK_ASSERT(!gogs.empty());
  gogs.clear();

  return rv;
}

}

int GogTest(int argc, char* argv[])
{
  if (simVis::isHeadless())
  {
    std::cerr << "Headless display detected; aborting test.\n";
    return 0;
  }
  int rv = 0;

  // osgEarth uses std::atexit() to clean up the Registry, which makes calls into
  // GDAL. To resolve leaks with GDAL without introducing errors, we need to install
  // an at-exit handler for destroying GDAL BEFORE osgEarth's registry adds theirs,
  // so that they execute in the correct order.
  std::atexit(GDALDestroy);

  // Check the SIMDIS SDK version
  simCore::checkVersionThrow();

  // Run tests
  rv += testShapes();
  rv += testDynamicEdits();
  rv += testArcSweep();

  // Need to destroy simVis Registry for valgrind testing
  simVis::Registry::destroy();
  // Need to destroy GDAL, even with the atexit(), to avoid a race condition. It appears
  // safe to destroy it more than once.
  GDALDestroy();

  return rv;
}
