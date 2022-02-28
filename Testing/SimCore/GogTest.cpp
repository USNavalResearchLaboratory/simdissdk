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

#include <sstream>
#include <vector>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/GOG/GogShape.h"
#include "simCore/GOG/Parser.h"

namespace {

// base gog shape optional fields in GOG format (set alt units to meters for testing), not testing  extrude height here
static const std::string BASE_FIELDS = "altitudeunits m\n 3d name my favorite shape\n off\n depthbuffer true\n 3d offsetalt 120.\n ref 24.5 55.6 10.\n altitudemode relativetoground\n scale 2. 1.3 .5\n orient 45. 10. 5.\n verticaldatum egm1984\nstarttime \"001 1970 00:00:00.00000\"\nendtime \"001 1970 01:00:00.00000\"\n";
// outlined shape optional field in GOG format
static const std::string OUTLINED_FIELD = BASE_FIELDS + "outline true\n";
// fillable shape optional fields in GOG format
static const std::string FILLABLE_FIELDS = OUTLINED_FIELD + "linewidth 4\n linecolor green\n linestyle dashed\n filled\n fillcolor yellow\n";
// circular shape optional fields in GOG format (in meters for testing)
static const std::string CIRCULAR_FIELDS = FILLABLE_FIELDS + " radius 1000.\n rangeunits m\n";
// point based shape optional field in GOG format
static const std::string POINTBASED_FIELDS = FILLABLE_FIELDS + " tessellate true\n lineprojection greatcircle\n";
// elliptical shape optional fields in GOG format
static const std::string ELLIPTICAL_FIELDS = CIRCULAR_FIELDS + " anglestart 10.\n angledeg 45.\n majoraxis 100.\n minoraxis 250.\n";
// arc shape optional fields in GOG format
static const std::string ARC_FIELDS = ELLIPTICAL_FIELDS + " innerradius 50\n";
// height field in GOG format
static const std::string HEIGHT_FIELD = "height 180.\n";
// points shape optional fields in GOG format
static const std::string POINTS_FIELDS = OUTLINED_FIELD + " pointsize 5\n linecolor magenta\n";
// annotation optional fields in GOG format
static const std::string ANNOTATION_FIELDS = "fontname georgia.ttf\n fontsize 24\n linecolor hex 0x0affa0ff\n textoutlinethickness thin\n textoutlinecolor blue\n imagefile icon.png\n priority 10.\n";

// return true if the specified positions are equal
bool comparePositions(const simCore::Vec3& pos1, const simCore::Vec3& pos2)
{
  return simCore::areEqual(pos1.x(), pos2.x()) && simCore::areEqual(pos1.y(), pos2.y()) && simCore::areEqual(pos1.z(), pos2.z());
}

// return true if all the positions in pos2 are in pos1
bool comparePositionVectors(const std::vector<simCore::Vec3>& pos1, const std::vector<simCore::Vec3>& pos2)
{
  size_t found = 0;
  for (simCore::Vec3 position : pos1)
  {
    for (simCore::Vec3 position2 : pos2)
    {
      if (comparePositions(position, position2))
        found++;
    }
  }
  return (found == pos1.size());
}

// test basic GOG format syntax checking
int testGeneralSyntax()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test file with missing end fails to create shape
  std::stringstream missingEnd;
  missingEnd << "start\n circle\n";
  parser.parse(missingEnd, "", shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  // test file with missing start fails to create shape
  std::stringstream missingStart;
  missingStart << "circle\n end\n";
  parser.parse(missingStart, "", shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  // test file with multiple keywords between start/end fails to create shape
  std::stringstream uncertainShape;
  uncertainShape << "start\n circle\n line\n centerlla 25.1 58.2 0.\n end\n";
  parser.parse(uncertainShape, "", shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  // test mixed case creates shapes
  std::stringstream mixedCaseCircle;
  mixedCaseCircle << "start\n CirCle\n centerLL 25.1 58.2\n END\n ";
  parser.parse(mixedCaseCircle, "", shapes);
  rv += SDK_ASSERT(!shapes.empty());
  if (!shapes.empty())
    rv += SDK_ASSERT(shapes.front()->shapeType() == simCore::GOG::ShapeType::CIRCLE);
  shapes.clear();
  std::stringstream mixedCaseLine;
  mixedCaseLine << "StarT\n LINE\n ll 22.2 23.2\n LL 22.5 25.2\nenD\n";
  parser.parse(mixedCaseLine, "", shapes);
  rv += SDK_ASSERT(!shapes.empty());
  if (!shapes.empty())
    rv += SDK_ASSERT(shapes.front()->shapeType() == simCore::GOG::ShapeType::LINE);
  shapes.clear();

  // test shapes with nested start still creates shape
  std::stringstream nestedStartShape;
  nestedStartShape << "start\ncircle\ncenterll 1 1\nstart\nend\n";
  parser.parse(nestedStartShape, "", shapes);
  rv += SDK_ASSERT(!shapes.empty());
  if (!shapes.empty())
    rv += SDK_ASSERT(shapes.front()->shapeType() == simCore::GOG::ShapeType::CIRCLE);
  shapes.clear();

  // test leading end statement still creates shape
  std::stringstream leadingEndShape;
  leadingEndShape << "end\nstart\ncircle\ncenterll 1 1\nend\n";
  parser.parse(leadingEndShape, "", shapes);
  rv += SDK_ASSERT(!shapes.empty());
  if (!shapes.empty())
    rv += SDK_ASSERT(shapes.front()->shapeType() == simCore::GOG::ShapeType::CIRCLE);
  shapes.clear();

  // test that unknown shape followed by actual shape still creates actual shape
  std::stringstream unknownShape;
  unknownShape << "start\nblahblah\ncenterll 2 2\nend\nstart\ncircle\ncenterll 1 1\nend\n";
  parser.parse(unknownShape, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
    rv += SDK_ASSERT(shapes.front()->shapeType() == simCore::GOG::ShapeType::CIRCLE);
  shapes.clear();

  return rv;
}

int testBaseOptionalFieldsNotSet(const simCore::GOG::GogShape* shape)
{
  int rv = 0;
  std::string name;
  rv += SDK_ASSERT(shape->getName(name) != 0);
  bool draw = false;
  rv += SDK_ASSERT(shape->getIsDrawn(draw) != 0);
  rv += SDK_ASSERT(draw);
  bool depthBuffer = true;
  rv += SDK_ASSERT(shape->getIsDepthBufferActive(depthBuffer) != 0);
  rv += SDK_ASSERT(!depthBuffer);
  double altOffset = 10.;
  rv += SDK_ASSERT(shape->getAltitudeOffset(altOffset) != 0);
  rv += SDK_ASSERT(altOffset == 0.);
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::CLAMP_TO_GROUND;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) != 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::NONE);
  simCore::Vec3 refPos(25., 25., 25.);
  rv += SDK_ASSERT(shape->getReferencePosition(refPos) != 0);
  rv += SDK_ASSERT(refPos == simCore::Vec3());
  simCore::Vec3 scalar(10., 10., 10.);
  rv += SDK_ASSERT(shape->getScale(scalar) != 0);
  rv += SDK_ASSERT(scalar == simCore::Vec3(1., 1., 1.));
  bool followYaw = true;
  rv += SDK_ASSERT(shape->getIsFollowingYaw(followYaw) != 0);
  rv += SDK_ASSERT(!followYaw);
  bool followPitch = true;
  rv += SDK_ASSERT(shape->getIsFollowingPitch(followPitch) != 0);
  rv += SDK_ASSERT(!followPitch);
  bool followRoll = true;
  rv += SDK_ASSERT(shape->getIsFollowingRoll(followRoll) != 0);
  rv += SDK_ASSERT(!followRoll);
  double yawOffset = 10.;
  rv += SDK_ASSERT(shape->getYawOffset(yawOffset) != 0);
  rv += SDK_ASSERT(yawOffset == 0.);
  double pitchOffset = 10.;
  rv += SDK_ASSERT(shape->getPitchOffset(pitchOffset) != 0);
  rv += SDK_ASSERT(pitchOffset == 0.);
  double rollOffset = 10.;
  rv += SDK_ASSERT(shape->getPitchOffset(rollOffset) != 0);
  rv += SDK_ASSERT(rollOffset == 0.);
  double extrudeHeight = 10.;
  rv += SDK_ASSERT(shape->getExtrudeHeight(extrudeHeight) != 0);
  rv += SDK_ASSERT(extrudeHeight == 0.);
  std::string vdatum = "?";
  rv += SDK_ASSERT(shape->getVerticalDatum(vdatum) != 0);
  rv += SDK_ASSERT(vdatum == "wgs84");
  simCore::TimeStamp stamp;
  rv += SDK_ASSERT(shape->getStartTime(stamp) != 0);
  rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  rv += SDK_ASSERT(shape->getEndTime(stamp) != 0);
  rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  return rv;

}

// test outlined shape's optional field is not set and returns default value
int testOutlinedOptionalFieldNotSet(const simCore::GOG::OutlinedShape* shape)
{
  int rv = testBaseOptionalFieldsNotSet(shape);
  bool outlined = false;
  rv += SDK_ASSERT(shape->getIsOutlined(outlined) != 0);
  rv += SDK_ASSERT(outlined);
  return rv;
}

// test that fillable shape's optional fields are not set and return default values
int testFillableOptionalFieldsNotSet(const simCore::GOG::FillableShape* shape)
{
  int rv = testOutlinedOptionalFieldNotSet(shape);
  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth) != 0);
  rv += SDK_ASSERT(lineWidth == 1);
  simCore::GOG::LineStyle style = simCore::GOG::LineStyle::DASHED;
  rv += SDK_ASSERT(shape->getLineStyle(style) != 0);
  rv += SDK_ASSERT(style == simCore::GOG::LineStyle::SOLID);
  simCore::GOG::Color color(0, 255, 255, 0);
  rv += SDK_ASSERT(shape->getLineColor(color) != 0);
  rv += SDK_ASSERT(color == simCore::GOG::Color());
  bool filled = true;
  rv += SDK_ASSERT(shape->getIsFilled(filled) != 0);
  rv += SDK_ASSERT(!filled);
  simCore::GOG::Color fillColor(0, 255, 255, 0);
  rv += SDK_ASSERT(shape->getFillColor(fillColor) != 0);
  rv += SDK_ASSERT(fillColor == simCore::GOG::Color());
  return rv;
}

// test the circular shape's position is set, and the optional fields are not
auto testCircularShapeMinimalFieldsFunc = [](const simCore::GOG::CircularShape* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testFillableOptionalFieldsNotSet(shape);
  simCore::Vec3 center;
  rv += SDK_ASSERT(shape->getCenterPosition(center) == 0);
  rv += SDK_ASSERT(comparePositions(center, positions.front()));
  double radius = 0.;
  // verify radius wasn't set
  rv += SDK_ASSERT(shape->getRadius(radius) == 1);

  return rv;
};

// test the orbit shape's position fields are set, and the optional fields are not
auto testOrbitShapeMinimalFieldsFunc = [](const simCore::GOG::Orbit* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  // dev error, require 2 positions to test orbit
  assert(positions.size() == 2);
  int rv = testCircularShapeMinimalFieldsFunc(shape, positions);
  if (positions.size() > 1)
  {
    const simCore::Vec3& center2 = shape->centerPosition2();
    rv += SDK_ASSERT(comparePositions(center2, positions[1]));
  }
  return rv;
};

// test the cone shape's required fields are set, and the optional fields are not
auto testCircularHeightShapeMinimalFieldsFunc = [](const simCore::GOG::CircularHeightShape* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testCircularShapeMinimalFieldsFunc(shape, positions);
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) != 0);
  return rv;
};

// test the ellipsoid shape's required fields are set, and the optional fields are not
auto testEllipsoidShapeMinimalFieldsFunc = [](const simCore::GOG::Ellipsoid* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testCircularHeightShapeMinimalFieldsFunc(shape, positions);
  double majorAxis = 0.;
  rv += SDK_ASSERT(shape->getMajorAxis(majorAxis) != 0);
  rv += SDK_ASSERT(majorAxis == 1000.);
  double minorAxis = 0.;
  rv += SDK_ASSERT(shape->getMinorAxis(minorAxis) != 0);
  rv += SDK_ASSERT(minorAxis == 1000.);
  return rv;
};

// test elliptical shape's required fields are set, and the optional fields are not
auto testEllipticalShapeMinimalFieldsFunc = [](const simCore::GOG::EllipticalShape* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testCircularShapeMinimalFieldsFunc(shape, positions);
  double angleStart = 10.;
  rv += SDK_ASSERT(shape->getAngleStart(angleStart) != 0);
  rv += SDK_ASSERT(angleStart == 0.);
  double angleSweep = 10.;
  rv += SDK_ASSERT(shape->getAngleSweep(angleSweep) != 0);
  rv += SDK_ASSERT(angleSweep == 0.);
  double majorAxis = 10.;
  rv += SDK_ASSERT(shape->getMajorAxis(majorAxis) != 0);
  rv += SDK_ASSERT(majorAxis == 0.);
  double minorAxis = 10.;
  rv += SDK_ASSERT(shape->getMinorAxis(minorAxis) != 0);
  rv += SDK_ASSERT(minorAxis == 0.);
  return rv;
};

// test the point based shape's required fields are set, and the optional fields are not
auto testPointBasedShapeMinimalFieldsFunc = [](const simCore::GOG::PointBasedShape* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testFillableOptionalFieldsNotSet(shape);
  const std::vector<simCore::Vec3>& positionsOut = shape->points();
  rv += SDK_ASSERT(positions.size() == positionsOut.size());
  rv += SDK_ASSERT(comparePositionVectors(positions, positionsOut));

  // verify that tessellation has not been set
  simCore::GOG::TessellationStyle style = simCore::GOG::TessellationStyle::NONE;
  rv += SDK_ASSERT(shape->getTessellation(style) != 0);
  rv += SDK_ASSERT(style == simCore::GOG::TessellationStyle::NONE);
  return rv;
};

// test the points shape's required fields are set, and the optional fields are not
auto testPointsShapeMinimalFieldsFunc = [](const simCore::GOG::Points* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = testOutlinedOptionalFieldNotSet(shape);
  const std::vector<simCore::Vec3>& positionsOut = shape->points();
  rv += SDK_ASSERT(positions.size() == positionsOut.size());
  rv += SDK_ASSERT(comparePositionVectors(positions, positionsOut));

  int pointSize = 0;
  rv += SDK_ASSERT(shape->getPointSize(pointSize) != 0);
  rv += SDK_ASSERT(pointSize == 1);
  simCore::GOG::Color color(0, 255, 255, 0);
  rv += SDK_ASSERT(shape->getColor(color) != 0);
  rv += SDK_ASSERT(color == simCore::GOG::Color());

  return rv;
};

// test the latlonaltbox shape's required fields are set, and the optional fields are not
auto testLatLonAltBoxMinimalFieldsFunc = [](const simCore::GOG::LatLonAltBox* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  // dev error, need two points to test latlonaltbox
  assert(positions.size() == 2);
  int rv = testFillableOptionalFieldsNotSet(shape);
  rv += SDK_ASSERT(simCore::areEqual(shape->north(), positions[0].lat()));
  rv += SDK_ASSERT(simCore::areEqual(shape->south(), positions[1].lat()));
  rv += SDK_ASSERT(simCore::areEqual(shape->east(), positions[0].lon()));
  rv += SDK_ASSERT(simCore::areEqual(shape->west(), positions[1].lon()));
  rv += SDK_ASSERT(shape->altitude() == positions[0].alt());
  double height = 10.;
  rv += SDK_ASSERT(shape->getHeight(height) != 0);
  rv += SDK_ASSERT(height == 0.);
  return rv;
};

// test the image overlay shape's required fields are set, and the optional fields are not
auto testImageOverlayMinimalFieldsFunc = [](const simCore::GOG::ImageOverlay* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  // dev error, need two points to test image overlay
  assert(positions.size() == 2);
  int rv = 0;
  rv += SDK_ASSERT(simCore::areEqual(shape->north(), positions[0].lat()));
  rv += SDK_ASSERT(simCore::areEqual(shape->south(), positions[1].lat()));
  rv += SDK_ASSERT(simCore::areEqual(shape->east(), positions[0].lon()));
  rv += SDK_ASSERT(simCore::areEqual(shape->west(), positions[1].lon()));
  rv += SDK_ASSERT(shape->imageFile() == "image.png");
  rv += SDK_ASSERT(shape->getRotation() == (32.0 * simCore::DEG2RAD));
  return rv;
};

// test that the specified gog string parses to the specified object, and calls the specified function with the shape and positions
template <typename ClassT, typename FunctionT>
int testShapePositionsFunction(const std::string& gog, const FunctionT& func, const std::vector<simCore::Vec3>& positions)
{
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  int rv = 0;

  std::stringstream gogStr;
  gogStr << gog;
  parser.parse(gogStr, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    ClassT* shape = dynamic_cast<ClassT*>(shapes.front().get());
    rv += SDK_ASSERT(shape != nullptr);
    if (shape)
      rv += func(shape, positions);
  }
  shapes.clear();
  return rv;
}

// test shapes with only minimum required fields set
int testMinimalShapes()
{
  int rv = 0;

  // ABSOLUTE

  // test circle
  std::vector<simCore::Vec3> centerPoint;
  centerPoint.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Circle>("start\n circle\n centerlla 25.1 58.2 0.\n end\n", testCircularShapeMinimalFieldsFunc, centerPoint);
  // test sphere
  rv += testShapePositionsFunction<simCore::GOG::Sphere>("start\n sphere\n centerlla 25.1 58.2 0.\n end\n", testCircularShapeMinimalFieldsFunc, centerPoint);
  // test hemisphere
  rv += testShapePositionsFunction<simCore::GOG::Hemisphere>("start\n hemisphere\n centerlla 25.1 58.2 0.\n end\n", testCircularShapeMinimalFieldsFunc, centerPoint);
  // test ellipsoid
  rv += testShapePositionsFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n centerlla 25.1 58.2 0.\n end\n", testEllipsoidShapeMinimalFieldsFunc, centerPoint);
  // test arc
  rv += testShapePositionsFunction<simCore::GOG::Arc>("start\n arc\n centerlla 25.1 58.2 0.\n end\n", testEllipticalShapeMinimalFieldsFunc, centerPoint);
  // test ellipse
  rv += testShapePositionsFunction<simCore::GOG::Ellipse>("start\n ellipse\n centerlla 25.1 58.2 0.\n end\n", testEllipticalShapeMinimalFieldsFunc, centerPoint);
  // test cylinder
  rv += testShapePositionsFunction<simCore::GOG::Cylinder>("start\n cylinder\n centerlla 25.1 58.2 0.\n end\n", testEllipticalShapeMinimalFieldsFunc, centerPoint);
  // test cone
  rv += testShapePositionsFunction<simCore::GOG::Cone>("start\n cone\n centerlla 25.1 58.2 0.\n end\n", testCircularHeightShapeMinimalFieldsFunc, centerPoint);

  // test orbit
  std::vector<simCore::Vec3> orbitCtrs;
  orbitCtrs.push_back(simCore::Vec3(24.4 * simCore::DEG2RAD, 43.2 * simCore::DEG2RAD, 0.));
  orbitCtrs.push_back(simCore::Vec3(24.1 * simCore::DEG2RAD, 43.5 * simCore::DEG2RAD, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Orbit>("start\n orbit\n centerlla 24.4 43.2 0.0\n centerll2 24.1 43.5\n end\n", testOrbitShapeMinimalFieldsFunc, orbitCtrs);

  // test line
  std::vector<simCore::Vec3> linePoints;
  linePoints.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  linePoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Line>("start\n line\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n end\n", testPointBasedShapeMinimalFieldsFunc, linePoints);
  // test linesegs
  rv += testShapePositionsFunction<simCore::GOG::LineSegs>("start\n linesegs\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n end\n", testPointBasedShapeMinimalFieldsFunc, linePoints);
  // test polygon
  linePoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 57.9 * simCore::DEG2RAD, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Polygon>("start\n poly\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n lla 26.2 57.9 0.\n end\n", testPointBasedShapeMinimalFieldsFunc, linePoints);
  // test points
  rv += testShapePositionsFunction<simCore::GOG::Points>("start\n points\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n lla 26.2 57.9 0.\n end\n", testPointsShapeMinimalFieldsFunc, linePoints);

  // test lla box
  std::vector<simCore::Vec3> llabPoints;
  llabPoints.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 55.6 * simCore::DEG2RAD, 100.));
  llabPoints.push_back(simCore::Vec3(25.3 * simCore::DEG2RAD, 55.4 * simCore::DEG2RAD, 100.));
  rv += testShapePositionsFunction<simCore::GOG::LatLonAltBox>("start\n latlonaltbox 25.1 25.3 55.4 55.6 100.\naltitudeunits m\n end\n", testLatLonAltBoxMinimalFieldsFunc, llabPoints);

  // test image overlay
  rv += testShapePositionsFunction<simCore::GOG::ImageOverlay>("start\n imageoverlay 25.1 25.3 55.4 55.6 32.\n imagefile image.png\n end\n", testImageOverlayMinimalFieldsFunc, llabPoints);

  // RELATIVE

  // test circle
  std::vector<simCore::Vec3> xyzPoint;
  xyzPoint.push_back(simCore::Vec3(15.2, 20., 10.));
  rv += testShapePositionsFunction<simCore::GOG::Circle>("start\n circle\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testCircularShapeMinimalFieldsFunc, xyzPoint);
  // test sphere
  rv += testShapePositionsFunction<simCore::GOG::Sphere>("start\n sphere\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testCircularShapeMinimalFieldsFunc, xyzPoint);
  // test hemisphere
  rv += testShapePositionsFunction<simCore::GOG::Hemisphere>("start\n hemisphere\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testCircularShapeMinimalFieldsFunc, xyzPoint);
  // test ellipsoid
  rv += testShapePositionsFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testEllipsoidShapeMinimalFieldsFunc, xyzPoint);
  // test arc
  rv += testShapePositionsFunction<simCore::GOG::Arc>("start\n arc\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testEllipticalShapeMinimalFieldsFunc, xyzPoint);
  // test ellipse
  rv += testShapePositionsFunction<simCore::GOG::Ellipse>("start\n ellipse\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testEllipticalShapeMinimalFieldsFunc, xyzPoint);
  // test cylinder
  rv += testShapePositionsFunction<simCore::GOG::Cylinder>("start\n cylinder\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testEllipticalShapeMinimalFieldsFunc, xyzPoint);
  // test cone
  rv += testShapePositionsFunction<simCore::GOG::Cone>("start\n cone\n centerxyz 15.2 20. 10.\n rangeunits m\n altitudeunits m\n end\n", testCircularHeightShapeMinimalFieldsFunc, xyzPoint);

  // test orbit
  std::vector<simCore::Vec3> orbitXyzCtrs;
  orbitXyzCtrs.push_back(simCore::Vec3(24.4, 43.2, 0.));
  orbitXyzCtrs.push_back(simCore::Vec3(24.1, 43.5, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Orbit>("start\n orbit\n centerxyz 24.4 43.2 0.0\n centerxy2 24.1 43.5\n rangeunits m\n altitudeunits m\n end\n", testOrbitShapeMinimalFieldsFunc, orbitXyzCtrs);

  // test line
  std::vector<simCore::Vec3> lineXyzPoints;
  lineXyzPoints.push_back(simCore::Vec3(10., 10., 10.));
  lineXyzPoints.push_back(simCore::Vec3(100.0, -500., 10.));
  rv += testShapePositionsFunction<simCore::GOG::Line>("start\n line\n xyz 10. 10. 10.\n xyz 100. -500. 10.\n  rangeunits m\n altitudeunits m\n end\n", testPointBasedShapeMinimalFieldsFunc, lineXyzPoints);
  // test linesegs
  rv += testShapePositionsFunction<simCore::GOG::LineSegs>("start\n linesegs\n xyz 10. 10. 10.\n xyz 100. -500. 10.\n  rangeunits m\n altitudeunits m\n end\n", testPointBasedShapeMinimalFieldsFunc, lineXyzPoints);
  // test polygon
  lineXyzPoints.push_back(simCore::Vec3(-500., 50., 0.));
  rv += testShapePositionsFunction<simCore::GOG::Polygon>("start\n poly\n xyz 10. 10. 10.\n xyz 100. -500. 10.\n xyz -500. 50. 0.\n  rangeunits m\n altitudeunits m\n end\n", testPointBasedShapeMinimalFieldsFunc, lineXyzPoints);
  // test points
  rv += testShapePositionsFunction<simCore::GOG::Points>("start\n points\n xyz 10. 10. 10.\n xyz 100. -500. 10.\n xyz -500. 50. 0.\n  rangeunits m\n altitudeunits m\n end\n", testPointsShapeMinimalFieldsFunc, lineXyzPoints);

  return rv;
}

// some shape types can not follow, others implement follow depending on relative state
bool canFollow(simCore::GOG::ShapeType type, bool relative)
{
  switch (type)
  {
  case simCore::GOG::ShapeType::ANNOTATION:
  case simCore::GOG::ShapeType::LATLONALTBOX:
  case simCore::GOG::ShapeType::IMAGEOVERLAY:
    return false;
  case simCore::GOG::ShapeType::LINE:
  case simCore::GOG::ShapeType::LINESEGS:
  case simCore::GOG::ShapeType::POLYGON:
  case simCore::GOG::ShapeType::POINTS:
    return relative;
  default:
    return true;
  }
}

// test that all follow components are set, and that offsets are 45., 10., and 5. respectively
auto testFollowFunc = [](const simCore::GOG::GogShape* shape) -> int
{
  int rv = 0;
  bool followYaw = false;
  rv += SDK_ASSERT(shape->getIsFollowingYaw(followYaw) == 0);
  rv += SDK_ASSERT(followYaw);
  bool followPitch = false;
  rv += SDK_ASSERT(shape->getIsFollowingPitch(followPitch) == 0);
  rv += SDK_ASSERT(followPitch);
  bool followRoll = false;
  rv += SDK_ASSERT(shape->getIsFollowingRoll(followRoll) == 0);
  rv += SDK_ASSERT(followRoll);
  double yawOffset = 0.;
  rv += SDK_ASSERT(shape->getYawOffset(yawOffset) == 0);
  rv += SDK_ASSERT(simCore::areEqual(yawOffset * simCore::RAD2DEG, 45.));
  double pitchOffset = 0.;
  rv += SDK_ASSERT(shape->getPitchOffset(pitchOffset) == 0);
  rv += SDK_ASSERT(simCore::areEqual(pitchOffset * simCore::RAD2DEG, 10.));
  double rollOffset = 0.;
  rv += SDK_ASSERT(shape->getRollOffset(rollOffset) == 0);
  rv += SDK_ASSERT(simCore::areEqual(rollOffset * simCore::RAD2DEG, 5.));
  return rv;
};

// test the shape's optional fields match the pre-defined test fields from BASE_FIELDS
int testBaseOptionalFields(const simCore::GOG::GogShape* shape)
{
  int rv = 0;
  std::string name;
  rv += SDK_ASSERT(shape->getName(name) == 0);
  rv += SDK_ASSERT(name == "my favorite shape");
  bool draw = true;
  rv += SDK_ASSERT(shape->getIsDrawn(draw) == 0);
  rv += SDK_ASSERT(!draw);
  bool depthBuffer = false;
  rv += SDK_ASSERT(shape->getIsDepthBufferActive(depthBuffer) == 0);
  rv += SDK_ASSERT(depthBuffer);
  double altOffset = 0.;
  rv += SDK_ASSERT(shape->getAltitudeOffset(altOffset) == 0);
  rv += SDK_ASSERT(altOffset == 120.);
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) == 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::RELATIVE_TO_GROUND);
  simCore::Vec3 scale;
  rv += SDK_ASSERT(shape->getScale(scale) == 0);
  rv += SDK_ASSERT(scale == simCore::Vec3(2., 1.3, 0.5));
  std::string vdatum;
  rv += SDK_ASSERT(shape->getVerticalDatum(vdatum) == 0);
  rv += SDK_ASSERT(vdatum == "egm1984");
  simCore::TimeStamp stamp;
  rv += SDK_ASSERT(shape->getStartTime(stamp) == 0);
  rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 0));
  rv += SDK_ASSERT(shape->getEndTime(stamp) == 0);
  rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 3600));

  // test reference point if relative
  if (shape->isRelative())
  {
    simCore::Vec3 refLla;
    rv += SDK_ASSERT(shape->getReferencePosition(refLla) == 0);
    rv += SDK_ASSERT(refLla == simCore::Vec3(24.5 * simCore::DEG2RAD, 55.6 * simCore::DEG2RAD, 10.));
  }

  // only certain shapes can follow
  if (canFollow(shape->shapeType(), shape->isRelative()))
  {
    rv += testFollowFunc(shape);
  }
  return rv;
}

// test that the shape's optional field matches the pre-defined test fields from OUTLINED_FIELD
int testOutlinedField(const simCore::GOG::OutlinedShape* shape)
{
  int rv = testBaseOptionalFields(shape);

  bool outlined = false;
  rv += SDK_ASSERT(shape->getIsOutlined(outlined) == 0);
  rv += SDK_ASSERT(outlined);

  return rv;
}

// test that the shape's optional fields match the pre-defined test fields from FILLABLE_FIELDS
auto testFillableShapeOptionalFieldsFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = testOutlinedField(shape);

  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth) == 0);
  rv += SDK_ASSERT(lineWidth == 4);

  simCore::GOG::LineStyle style = simCore::GOG::LineStyle::SOLID;
  rv += SDK_ASSERT(shape->getLineStyle(style) == 0);
  rv += SDK_ASSERT(style == simCore::GOG::LineStyle::DASHED);

  simCore::GOG::Color lineColor;
  rv += SDK_ASSERT(shape->getLineColor(lineColor) == 0);
  rv += SDK_ASSERT(lineColor == simCore::GOG::Color(0, 255, 0, 255));

  bool filled = false;
  rv += SDK_ASSERT(shape->getIsFilled(filled) == 0);
  rv += SDK_ASSERT(filled);

  simCore::GOG::Color fillColor;
  rv += SDK_ASSERT(shape->getFillColor(fillColor) == 0);
  rv += SDK_ASSERT(fillColor == simCore::GOG::Color(255, 255, 0, 255));

  return rv;
};

// test that the shape's optional fields match the pre-defined test fields from CIRCULAR_FIELDS
auto testCircularShapeOptionalFieldsFunc = [](const simCore::GOG::CircularShape* shape) -> int
{
  int rv = testFillableShapeOptionalFieldsFunc(shape);
  double radius = 0.;
  rv += SDK_ASSERT(shape->getRadius(radius) == 0);
  rv += SDK_ASSERT(radius == 1000.);
  return rv;
};

// test the shape's optional fields match the pre-defined test fields from POINTBASED_FIELDS
auto testPointBasedShapeOptionalFieldsFunc = [](const simCore::GOG::PointBasedShape* shape) -> int
{
  int rv = testFillableShapeOptionalFieldsFunc(shape);
  simCore::GOG::TessellationStyle style = simCore::GOG::TessellationStyle::NONE;
  rv += SDK_ASSERT(shape->getTessellation(style) == 0);
  rv += SDK_ASSERT(style == simCore::GOG::TessellationStyle::GREAT_CIRCLE);
  return rv;
};

// test the point shape's optional fields match the pre-defined test fields from POINTS_FIELDS
auto testPointsOptionalFieldsFunc = [](const simCore::GOG::Points* shape) -> int
{
  int rv = testOutlinedField(shape);

  int pointSize = 0;
  rv += SDK_ASSERT(shape->getPointSize(pointSize) == 0);
  rv += SDK_ASSERT(pointSize == 5);
  simCore::GOG::Color color;
  rv += SDK_ASSERT(shape->getColor(color) == 0);
  rv += SDK_ASSERT(color == simCore::GOG::Color(192, 0, 192, 255));

  return rv;
};

// test the elliptical shape's optional fields match the pre-defined test fields from ELLIPTICAL_FIELDS
auto testEllipticalShapeOptionalFieldsFunc = [](const simCore::GOG::EllipticalShape* shape) -> int
{
  int rv = testCircularShapeOptionalFieldsFunc(shape);
  double angleStart = 0.;
  rv += SDK_ASSERT(shape->getAngleStart(angleStart) == 0);
  rv += SDK_ASSERT(simCore::areEqual(angleStart * simCore::RAD2DEG, 10.));
  double angleSweep = 0.;
  rv += SDK_ASSERT(shape->getAngleSweep(angleSweep) == 0);
  rv += SDK_ASSERT(simCore::areEqual(angleSweep * simCore::RAD2DEG, 45.));
  double majorAxis = 0.;
  rv += SDK_ASSERT(shape->getMajorAxis(majorAxis) == 0);
  rv += SDK_ASSERT(majorAxis == 100.);
  double minorAxis = 0.;
  rv += SDK_ASSERT(shape->getMinorAxis(minorAxis) == 0);
  rv += SDK_ASSERT(minorAxis == 250.);
  return rv;
};

// test the arc shape's optional fields match the pre-defined test fields from ARC_FIELDS
auto testArcShapeOptionalFieldsFunc = [](const simCore::GOG::Arc* shape) -> int
{
  int rv = testEllipticalShapeOptionalFieldsFunc(shape);
  double innerRadius = 0.;
  rv += SDK_ASSERT(shape->getInnerRadius(innerRadius) == 0);
  rv += SDK_ASSERT(simCore::areEqual(innerRadius, 50.));
  return rv;
};

// test the cylinder shape's optional height field matches the pre-defined test field from HEIGHT_FIELD
auto testCircularHeightShapeOptionalFieldsFunc = [](const simCore::GOG::CircularHeightShape* shape) -> int
{
  int rv = testCircularShapeOptionalFieldsFunc(shape);
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) == 0);
  rv += SDK_ASSERT(height == 180.);
  return rv;
};

// test the cylinder shape's optional height field matches the pre-defined test field from HEIGHT_FIELD
auto testCylinderShapeOptionalFieldsFunc = [](const simCore::GOG::Cylinder* shape) -> int
{
  int rv = testEllipticalShapeOptionalFieldsFunc(shape);
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) == 0);
  rv += SDK_ASSERT(height == 180.);
  return rv;
};

// test the ellipsoid shape's optional fields match the pre-defined test field from ELLIPTICAL_FIELDS (ignores anglestart and angledeg)
auto testEllipsoidShapeOptionalFieldsFunc = [](const simCore::GOG::Ellipsoid* shape) -> int
{
  int rv = testCircularHeightShapeOptionalFieldsFunc(shape);
  double majorAxis = 0.;
  rv += SDK_ASSERT(shape->getMajorAxis(majorAxis) == 0);
  rv += SDK_ASSERT(majorAxis == 100.);
  double minorAxis = 0.;
  rv += SDK_ASSERT(shape->getMinorAxis(minorAxis) == 0);
  rv += SDK_ASSERT(minorAxis == 250.);
  return rv;
};

// test that the specified gog string parses to the specified object, and that its optional fields match the pre-defined test fields
template <typename ClassT, typename FunctionT>
int testShapeFunction(const std::string& gog, const FunctionT& func)
{
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  int rv = 0;

  std::stringstream gogStr;
  gogStr << gog;
  parser.parse(gogStr, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    ClassT* shape = dynamic_cast<ClassT*>(shapes.front().get());
    rv += SDK_ASSERT(shape != nullptr);
    if (shape)
      rv += func(shape);
  }
  shapes.clear();
  return rv;
}

// test shapes with all fields set
int testShapesOptionalFields()
{
  int rv = 0;

  // test circle
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n centerlla 24.4 43.2 0.0\n" + CIRCULAR_FIELDS + " end\n",  testCircularShapeOptionalFieldsFunc);
  // test sphere
  rv += testShapeFunction<simCore::GOG::Sphere>("start\n sphere\n centerlla 24.4 43.2 0.0\n" + CIRCULAR_FIELDS + " end\n", testCircularShapeOptionalFieldsFunc);
  // test hemisphere
  rv += testShapeFunction<simCore::GOG::Hemisphere>("start\n hemisphere\n centerlla 24.4 43.2 0.0\n" + CIRCULAR_FIELDS + " end\n", testCircularShapeOptionalFieldsFunc);
  // test orbit
  rv += testShapeFunction<simCore::GOG::Orbit>("start\n orbit\n centerlla 24.4 43.2 0.0\n centerll2 24.1 43.5\n" + CIRCULAR_FIELDS + " end\n", testCircularShapeOptionalFieldsFunc);

  // test line
  rv += testShapeFunction<simCore::GOG::Line>("start\n line\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n" + POINTBASED_FIELDS + " end\n", testPointBasedShapeOptionalFieldsFunc);
  // test linesegs
  rv += testShapeFunction<simCore::GOG::LineSegs>("start\n linesegs\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n" + POINTBASED_FIELDS + " end\n", testPointBasedShapeOptionalFieldsFunc);
  // test polygon
  rv += testShapeFunction<simCore::GOG::Polygon>("start\n poly\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n lla 26.2 57.9 0.\n" + POINTBASED_FIELDS + " end\n", testPointBasedShapeOptionalFieldsFunc);
  // test points
  rv += testShapeFunction<simCore::GOG::Points>("start\n points\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n" + POINTS_FIELDS + " end\n", testPointsOptionalFieldsFunc);

  // test arc
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n centerlla 24.4 43.2 0.0\n" + ARC_FIELDS + "end\n", testArcShapeOptionalFieldsFunc);
  // test ellipse
  rv += testShapeFunction<simCore::GOG::Ellipse>("start\n ellipse\n centerlla 24.4 43.2 0.0\n" + ELLIPTICAL_FIELDS + "end\n", testEllipticalShapeOptionalFieldsFunc);
  // test cylinder
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n centerlla 24.4 43.2 0.0\n" + ELLIPTICAL_FIELDS + HEIGHT_FIELD + "end\n", testCylinderShapeOptionalFieldsFunc);
  // test ellipsoid (note that anglestart and angleend are ignored)
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n centerlla 24.4 43.2 0.0\n" + ELLIPTICAL_FIELDS + HEIGHT_FIELD + "end\n", testEllipsoidShapeOptionalFieldsFunc);
  // test cone
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n centerlla 24.4 43.2 0.0\n" + ELLIPTICAL_FIELDS + HEIGHT_FIELD + "end\n", testCircularHeightShapeOptionalFieldsFunc);

  // test arc with angleend
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n centerlla 24.4 43.2 0.0\n" + CIRCULAR_FIELDS + "angleStart 10.\n angleend 55.\n majoraxis 100.\n minoraxis 250.\n end\n", testEllipticalShapeOptionalFieldsFunc);
  // test arc with angleend, cannot cross 0
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n centerlla 24.4 43.2 0.0\n" + CIRCULAR_FIELDS + "angleStart 10.\n angleend -305.\n majoraxis 100.\n minoraxis 250.\n end\n", testEllipticalShapeOptionalFieldsFunc);

  return rv;
}

// test shapes that have required fields to ensure they are not created if required field is missing
int testIncompleteShapes()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test line (requires 2 points minimum)
  std::stringstream lineGogIncomplete;
  lineGogIncomplete << "start\n line\n lla 25.1 58.2 0.\n end\n";
  parser.parse(lineGogIncomplete, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test line segs (requires 2 points minimum)
  std::stringstream lineSegsGogIncomplete;
  lineSegsGogIncomplete << "start\n linesegs\n lla 25.1 58.2 0.\n end\n";
  parser.parse(lineSegsGogIncomplete, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test polygon (requires 3 points minimum)
  std::stringstream polyGogIncomplete;
  polyGogIncomplete << "start\n poly\n lla 25.1 58.2 0.\n lla 25.1 58.3 0.\n end\n";
  parser.parse(polyGogIncomplete, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test annotation (requires text)
  std::stringstream annoCtrGog;
  annoCtrGog << "start\n annotation\n centerlla 24.2 43.3 0.\n end\n";
  parser.parse(annoCtrGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test points (requires 1 point minimum)
  std::stringstream pointsGog;
  pointsGog << "start\n points\n end\n";
  parser.parse(pointsGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test orbit (requires 2 center points)
  std::stringstream orbitCtr1Gog;
  orbitCtr1Gog << "start\n orbit\n centerll 1 2\n end\n";
  parser.parse(orbitCtr1Gog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test orbit (requires 2 center points)
  std::stringstream orbitCtr2Gog;
  orbitCtr2Gog << "start\n orbit\n centerll2 1 2\n end\n";
  parser.parse(orbitCtr2Gog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  return rv;
}

// test the annotation center
auto testAnnotationCenterFunc = [](const simCore::GOG::Annotation* shape, const std::vector<simCore::Vec3>& positions) -> int
{
  int rv = 0;
  // dev error, require 1 position to test annotation
  assert(!positions.empty());
  if (!positions.empty())
  {
    simCore::Vec3 position;
    rv += SDK_ASSERT(shape->getPosition(position) == 0);
    rv += SDK_ASSERT(comparePositions(position, positions[0]));
  }
  return rv;
};

// test all the annotation fields and nested annotations special case
int testAnnotation()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test annotation with only required fields set
  std::stringstream annoMinimalGog;
  annoMinimalGog << "start\n annotation label 1\n centerll 24.5 54.6\n end\n";
  parser.parse(annoMinimalGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(shapes.front().get());
    rv += SDK_ASSERT(anno != nullptr);
    if (anno)
    {
      rv += SDK_ASSERT(anno->text() == "label 1");
      simCore::Vec3 position;
      rv += SDK_ASSERT(anno->getPosition(position) == 0);
      rv += SDK_ASSERT(comparePositions(position, simCore::Vec3(24.5 * simCore::DEG2RAD, 54.6 * simCore::DEG2RAD, 0.)));
      // make sure optional fields were not set
      std::string fontName;
      rv += SDK_ASSERT(anno->getFontName(fontName) != 0);
      int textSize = 0;
      rv += SDK_ASSERT(anno->getTextSize(textSize) != 0);
      rv += SDK_ASSERT(textSize == 15);
      simCore::GOG::Color textColor(0,255,255,0);
      rv += SDK_ASSERT(anno->getTextColor(textColor) != 0);
      rv += SDK_ASSERT(textColor == simCore::GOG::Color());
      simCore::GOG::Color outlineColor(0,255,255,0);
      rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) != 0);
      rv += SDK_ASSERT(outlineColor == simCore::GOG::Color(0,0,0,255));
      simCore::GOG::OutlineThickness thickness = simCore::GOG::OutlineThickness::THICK;
      rv += SDK_ASSERT(anno->getOutlineThickness(thickness) != 0);
      rv += SDK_ASSERT(thickness == simCore::GOG::OutlineThickness::THIN);
      std::string iconFile = "someFile";
      rv += SDK_ASSERT(anno->getImageFile(iconFile) != 0);
      rv += SDK_ASSERT(iconFile.empty());
      double priority = 0.;
      rv += SDK_ASSERT(anno->getPriority(priority) != 0);
      rv += SDK_ASSERT(priority == 100.);
    }
  }
  shapes.clear();

  // test full annotation
  std::stringstream annoGog;
  annoGog << "start\n annotation label 1\n centerll 24.5 54.6\n fontname georgia.ttf\n fontsize 24\n linecolor hex 0xa0ffa0ff\n textoutlinethickness thin\n textoutlinecolor blue\n imagefile icon.png\n priority 10.\n end\n";
  parser.parse(annoGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(shapes.front().get());
    rv += SDK_ASSERT(anno != nullptr);
    if (anno)
    {
      simCore::Vec3 position;
      rv += SDK_ASSERT(anno->getPosition(position) == 0);
      rv += SDK_ASSERT(comparePositions(position, simCore::Vec3(24.5 * simCore::DEG2RAD, 54.6 * simCore::DEG2RAD, 0.)));
      rv += SDK_ASSERT(anno->text() == "label 1");
      std::string fontName;
      rv += SDK_ASSERT(anno->getFontName(fontName) == 0);
      rv += SDK_ASSERT(fontName.find("georgia.ttf") != std::string::npos);
      int textSize = 0;
      rv += SDK_ASSERT(anno->getTextSize(textSize) == 0);
      rv += SDK_ASSERT(textSize == 24);
      simCore::GOG::Color textColor;
      rv += SDK_ASSERT(anno->getTextColor(textColor) == 0);
      rv += SDK_ASSERT(textColor == simCore::GOG::Color(255, 160, 255, 160));
      simCore::GOG::Color outlineColor;
      rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) == 0);
      rv += SDK_ASSERT(outlineColor == simCore::GOG::Color(0, 0, 255, 255));
      simCore::GOG::OutlineThickness thickness = simCore::GOG::OutlineThickness::NONE;
      rv += SDK_ASSERT(anno->getOutlineThickness(thickness) == 0);
      rv += SDK_ASSERT(thickness == simCore::GOG::OutlineThickness::THIN);
      std::string iconFile;
      rv += SDK_ASSERT(anno->getImageFile(iconFile) == 0);
      rv += SDK_ASSERT(iconFile == "icon.png");
      double priority = 0.;
      rv += SDK_ASSERT(anno->getPriority(priority) == 0);
      rv += SDK_ASSERT(priority == 10.);
    }
  }
  shapes.clear();

  // test nested annotations
  std::stringstream annoNestedGog;
  annoNestedGog << "start\n annotation label 0\n centerll 24.5 54.6\n fontname georgia.ttf\n fontsize 24\n linecolor hex 0xa0ffa0ff\n textoutlinethickness thin\n textoutlinecolor blue\n priority 10.\n"
    << "annotation label 1\n centerll 24.7 54.3\n annotation label 2\n centerll 23.4 55.4\n end\n";
  parser.parse(annoNestedGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 3);
  if (!shapes.empty())
  {
    int textId = 0;
    std::vector<simCore::Vec3> positions;
    positions.push_back(simCore::Vec3(24.5 * simCore::DEG2RAD, 54.6 * simCore::DEG2RAD, 0.));
    positions.push_back(simCore::Vec3(24.7 * simCore::DEG2RAD, 54.3 * simCore::DEG2RAD, 0.));
    positions.push_back(simCore::Vec3(23.4* simCore::DEG2RAD, 55.4 * simCore::DEG2RAD, 0.));

    // check that all 3 annotations have the same attributes, since they should all match the first annotation fields found
    for (simCore::GOG::GogShapePtr gogPtr : shapes)
    {
      simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(gogPtr.get());
      rv += SDK_ASSERT(anno != nullptr);
      if (anno)
      {
        simCore::Vec3 position;
        rv += SDK_ASSERT(anno->getPosition(position) == 0);
        rv += SDK_ASSERT(comparePositions(position, positions[textId]));
        std::ostringstream os;
        os << "label " << textId++;
        rv += SDK_ASSERT(anno->text() == os.str());
        rv += SDK_ASSERT(!anno->isRelative());

        std::string fontName;
        rv += SDK_ASSERT(anno->getFontName(fontName) == 0);
        rv += SDK_ASSERT(fontName.find("georgia.ttf") != std::string::npos);
        int textSize = 0;
        rv += SDK_ASSERT(anno->getTextSize(textSize) == 0);
        rv += SDK_ASSERT(textSize == 24);
        simCore::GOG::Color textColor;
        rv += SDK_ASSERT(anno->getTextColor(textColor) == 0);
        rv += SDK_ASSERT(textColor == simCore::GOG::Color(255, 160, 255, 160));
        simCore::GOG::Color outlineColor;
        rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) == 0);
        rv += SDK_ASSERT(outlineColor == simCore::GOG::Color(0, 0, 255, 255));
        simCore::GOG::OutlineThickness thickness = simCore::GOG::OutlineThickness::NONE;
        rv += SDK_ASSERT(anno->getOutlineThickness(thickness) == 0);
        rv += SDK_ASSERT(thickness == simCore::GOG::OutlineThickness::THIN);
        double priority = 0.;
        rv += SDK_ASSERT(anno->getPriority(priority) == 0);
        rv += SDK_ASSERT(priority == 10.);
      }
    }
  }
  shapes.clear();

  // test nested valid and invalid annotations
  std::stringstream annoNestedInvalidGog;
  annoNestedInvalidGog << "start\n annotation label 0\n centerll 24.5 54.6\n"
    << "annotation\n centerll 24.7 54.3\n annotation label 1\n centerll 23.4 55.4\n end\n";
  parser.parse(annoNestedInvalidGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 2);
  if (!shapes.empty())
  {
    int textId = 0;
    for (simCore::GOG::GogShapePtr gogPtr : shapes)
    {
      simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(gogPtr.get());
      rv += SDK_ASSERT(anno != nullptr);
      if (anno)
      {
        std::ostringstream os;
        os << "label " << textId++;
        rv += SDK_ASSERT(anno->text() == os.str());
      }
    }
  }
  shapes.clear();

  // test invalid annotation with a valid shape, ensure valid shape is parsed
  std::stringstream annoInvalidGog;
  annoInvalidGog << "start\n annotation \n centerll 24.5 54.6\n end\n start\ncircle\n centerll 1 1\n end\n";
  parser.parse(annoInvalidGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
    rv += SDK_ASSERT(dynamic_cast<simCore::GOG::Circle*>(shapes.front().get()) != nullptr);
  shapes.clear();

  // test annotation position parsing, which supports multiple options (centerlla, centerll, lla, ll, centerxyz, centerxy, xyz, xy)
  std::vector<simCore::Vec3> annoAbsoluteCtr;
  annoAbsoluteCtr.push_back(simCore::Vec3(24.4 * simCore::DEG2RAD, 43.2 * simCore::DEG2RAD, 250.));
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n centerlla 24.4 43.2 250.\n altitudeunits m\n  end\n", testAnnotationCenterFunc, annoAbsoluteCtr);
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n lla 24.4 43.2 250.\n altitudeunits m\n  end\n", testAnnotationCenterFunc, annoAbsoluteCtr);
  annoAbsoluteCtr.clear();
  annoAbsoluteCtr.push_back(simCore::Vec3(24.4 * simCore::DEG2RAD, 43.2 * simCore::DEG2RAD, 0.));
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n centerll 24.4 43.2\n altitudeunits m\n  end\n", testAnnotationCenterFunc, annoAbsoluteCtr);
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n ll 24.4 43.2\n altitudeunits m\n  end\n", testAnnotationCenterFunc, annoAbsoluteCtr);
  std::vector<simCore::Vec3> annoRelCtr;
  annoRelCtr.push_back(simCore::Vec3(20., 35., 10.));
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n centerxyz 20. 35. 10.\n rangeunits m\n altitudeunits m\n end\n", testAnnotationCenterFunc, annoRelCtr);
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n xyz 20. 35. 10.\n rangeunits m\n altitudeunits m\n end\n", testAnnotationCenterFunc, annoRelCtr);
  annoRelCtr.clear();
  annoRelCtr.push_back(simCore::Vec3(20., 35., 0.));
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n centerxy 20. 35.\n rangeunits m\n altitudeunits m\n end\n", testAnnotationCenterFunc, annoRelCtr);
  rv += testShapePositionsFunction<simCore::GOG::Annotation>("start\n annotation label 1\n xy 20. 35.\n rangeunits m\n altitudeunits m\n end\n", testAnnotationCenterFunc, annoRelCtr);

  // test annotation text special characters
  std::stringstream annoTextGog;
  annoTextGog << "start\n annotation label_1\\nnext line\n centerll 24.5 54.6\n end\n";
  parser.parse(annoTextGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(shapes.front().get());
    rv += SDK_ASSERT(anno != nullptr);
    if (anno)
      rv += SDK_ASSERT(anno->text() == "label 1\nnext line");
  }

  return rv;
}


// test that the arc's angles are startAngle = 5 deg and sweep = 47 deg
auto testArcAnglesFunc = [](const simCore::GOG::Arc* shape) -> int
{
  int rv = 0;
  double startAngle = 0.;
  rv += SDK_ASSERT(shape->getAngleStart(startAngle) == 0);
  rv += SDK_ASSERT(simCore::areEqual(startAngle * simCore::RAD2DEG, 5.));
  double sweep = 0.;
  rv += SDK_ASSERT(shape->getAngleSweep(sweep) == 0);
  rv += SDK_ASSERT(simCore::areEqual(sweep * simCore::RAD2DEG, 47.));
  return rv;
};

// test that the circular height shape's height is 350 m
auto testCircularHeightFunc = [](const simCore::GOG::CircularHeightShape* shape) -> int
{
  int rv = 0;
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) == 0);
  rv += SDK_ASSERT(height == 350);
  return rv;
};

// test that the cylinder shape's height is 350 m
auto testCylinderHeightFunc = [](const simCore::GOG::Cylinder* shape) -> int
{
  int rv = 0;
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) == 0);
  rv += SDK_ASSERT(height == 350);
  return rv;
};

// test that the circle's radius is 2000 m
auto testCircularRadiusFunc = [](const simCore::GOG::CircularShape* shape) -> int
{
  int rv = 0;
  double radius = 0.;
  rv += SDK_ASSERT(shape->getRadius(radius) == 0);
  rv += SDK_ASSERT(simCore::areEqual(radius, 2000.));
  return rv;
};

// test that the elliptical shape's angle start is M_PI_2
auto testEllipticalAngleStartFunc = [](const simCore::GOG::EllipticalShape* shape) -> int
{
  int rv = 0;
  double angleStart = 0.;
  rv += SDK_ASSERT(shape->getAngleStart(angleStart) == 0);
  rv += SDK_ASSERT(simCore::areEqual(angleStart, M_PI_2));
  return rv;
};

int testUnits()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test circle range units default to yards and altitude units default to feet
  std::stringstream circleGog;
  circleGog << "start\n circle\n centerlla 25.1 58.2 12.\n radius 100\n end\n";
  parser.parse(circleGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Circle* circle = dynamic_cast<simCore::GOG::Circle*>(shapes.front().get());
    rv += SDK_ASSERT(circle != nullptr);
    if (circle)
    {
      simCore::Vec3 center;
      rv += SDK_ASSERT(circle->getCenterPosition(center) == 0);
      simCore::Units altMeters(simCore::Units::METERS);
      simCore::Units altFeet(simCore::Units::FEET);
      // verify output in meters matches input in feet
      rv += SDK_ASSERT(comparePositions(center, simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, altFeet.convertTo(altMeters, 12.))));

      double radius = 0.;
      // verify output in meters matches input in feet
      rv += SDK_ASSERT(circle->getRadius(radius) == 0);
      simCore::Units altYards(simCore::Units::YARDS);
      rv += SDK_ASSERT(simCore::areEqual(radius, altYards.convertTo(altMeters, 100.)));
    }
  }
  shapes.clear();

  // test circle with defined range and altitude units
  std::stringstream circleDefinedGog;
  circleDefinedGog << "start\n circle\n centerlla 25.1 58.2 10.\n radius 10\n rangeunits km\n altitudeunits m\n end\n";
  parser.parse(circleDefinedGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Circle* circle = dynamic_cast<simCore::GOG::Circle*>(shapes.front().get());
    rv += SDK_ASSERT(circle != nullptr);
    if (circle)
    {
      simCore::Vec3 center;
      rv += SDK_ASSERT(circle->getCenterPosition(center) == 0);
      // verify output in meters matches input in meters
      rv += SDK_ASSERT(comparePositions(center, simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 10.)));

      double radius = 0.;
      // verify radius is 10 km
      rv += SDK_ASSERT(circle->getRadius(radius) == 0);
      rv += SDK_ASSERT(simCore::areEqual(radius, 10000.));
    }
  }
  shapes.clear();

  // test line altitude units default to feet
  std::stringstream lineGog;
  lineGog << "start\n line\n lla 25.1 58.2 20.\n lla 26.2 58.3 12.\n end\n";
  parser.parse(lineGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Line* line = dynamic_cast<simCore::GOG::Line*>(shapes.front().get());
    rv += SDK_ASSERT(line != nullptr);
    if (line)
    {
      const std::vector<simCore::Vec3>& positions = line->points();
      rv += SDK_ASSERT(positions.size() == 2);
      std::vector<simCore::Vec3> input;
      simCore::Units altMeters(simCore::Units::METERS);
      simCore::Units altFeet(simCore::Units::FEET);
      input.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, altFeet.convertTo(altMeters, 20.)));
      input.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, altFeet.convertTo(altMeters, 12.)));
      rv += SDK_ASSERT(comparePositionVectors(input, positions));
    }
  }
  shapes.clear();

  // test line with defined altitude units
  std::stringstream lineDefinedGog;
  lineDefinedGog << "start\n line\n lla 25.1 58.2 1.4\n lla 26.2 58.3 2.\n altitudeunits kf\n end\n";
  parser.parse(lineDefinedGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Line* line = dynamic_cast<simCore::GOG::Line*>(shapes.front().get());
    rv += SDK_ASSERT(line != nullptr);
    if (line)
    {
      const std::vector<simCore::Vec3>& positions = line->points();
      rv += SDK_ASSERT(positions.size() == 2);
      std::vector<simCore::Vec3> input;
      simCore::Units altMeters(simCore::Units::METERS);
      simCore::Units altKf(simCore::Units::KILOFEET);
      input.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, altKf.convertTo(altMeters, 1.4)));
      input.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, altKf.convertTo(altMeters, 2.)));
      rv += SDK_ASSERT(comparePositionVectors(input, positions));
    }
  }
  shapes.clear();

  // test arc angle units default to degrees
  std::stringstream arcGog;
  arcGog << "start\n arc\n centerlla 25.1 58.2 12.\n anglestart 5.\n angledeg 100.\n end\n";
  parser.parse(arcGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Arc* arc = dynamic_cast<simCore::GOG::Arc*>(shapes.front().get());
    rv += SDK_ASSERT(arc != nullptr);
    if (arc)
    {
      double angleStart = 0.;
      arc->getAngleStart(angleStart);
      // verify output in radians matches input in degrees
      rv += SDK_ASSERT(simCore::areEqual(angleStart * simCore::RAD2DEG, 5.));

      double angleSweep = 0.;
      arc->getAngleSweep(angleSweep);
      rv += SDK_ASSERT(simCore::areEqual(angleSweep * simCore::RAD2DEG, 100.));
    }
  }
  shapes.clear();

  // test arc with defined angle units
  std::stringstream arcDefinedGog;
  arcDefinedGog << "start\n arc\n centerlla 25.1 58.2 12.\n anglestart 0.1253\n angledeg 1.5\n angleunits rad\n end\n";
  parser.parse(arcDefinedGog, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Arc* arc = dynamic_cast<simCore::GOG::Arc*>(shapes.front().get());
    rv += SDK_ASSERT(arc != nullptr);
    if (arc)
    {
      double angleStart = 0.;
      arc->getAngleStart(angleStart);
      rv += SDK_ASSERT(simCore::areEqual(angleStart, 0.1253));

      double angleSweep = 0.;
      arc->getAngleSweep(angleSweep);
      rv += SDK_ASSERT(simCore::areEqual(angleSweep, 1.5));
    }
  }
  shapes.clear();

  // test arc with degrees specified angle units, since degree is a non-standard units name
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n centerlla 25.1 58.2 12.\n anglestart 5.\n angledeg 47.\n angleunits degree\n end\n", testArcAnglesFunc);

  // test height units work for circular height shape
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n centerlla 25 34 0\n centerll2 23 23 0\n height 350\n altitudeunits m\n end\n", testCircularHeightFunc);
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n centerlla 25 34 0\n centerll2 23 23 0\n height .35\n altitudeunits km\n end\n", testCircularHeightFunc);
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n centerlla 25 34 0\n height 350\n altitudeunits m\n end\n", testCircularHeightFunc);
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n centerlla 25 34 0\n height .35\n altitudeunits km\n end\n", testCircularHeightFunc);
  // test height units work for cylinder
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n centerlla 25 34 0\n height 350\n altitudeunits m\n end\n", testCylinderHeightFunc);
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n centerlla 25 34 0\n height .35\n altitudeunits km\n end\n", testCylinderHeightFunc);

  // test all range units (use 2000 meters radius)
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2000000\n rangeunits   mm  \n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2000000\n rangeunits millimeters\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 200000\n rangeunits cm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 200000\n rangeunits centimeters\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 78740.16058032128\n rangeunits in\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 78740.16058032128\n rangeunits inches\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 6561.680005304462\n rangeunits ft\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 6561.680005304462\n rangeunits feet\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2187.226668434821\n rangeunits yd\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2187.226668434821\n rangeunits yards\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2000\n rangeunits m\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2000\n rangeunits meters\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1093.61333421741\n rangeunits fm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1093.61333421741\n rangeunits fathoms\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 6.561680005304462\n rangeunits kf\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 6.561680005304462\n rangeunits kilofeet\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2.187226600000268\n rangeunits kyd\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2.187226600000268\n rangeunits kiloyards\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2\n rangeunits km\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2\n rangeunits kilometers\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits sm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits mi\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits miles\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.079913606911447\n rangeunits nm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.079913606911447\n rangeunits  nautical miles \n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.093613199999827\n rangeunits dm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.093613199999827\n rangeunits data miles\n end\n", testCircularRadiusFunc);

  // test all angle units (use 90 degrees)
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 90\n angleunits deg\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 90\n angleunits degree\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 90\n angleunits degree\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 1.57079632679489661923\n angleunits rad\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 1.57079632679489661923\n angleunits radians\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 2.467401100272329\n angleunits bam\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 2.467401100272329\n angleunits binary angle measurement\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 1599.999999999\n angleunits mil\n end\n", testEllipticalAngleStartFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n anglestart 1599.999999999\n angleunits angular mil\n end\n", testEllipticalAngleStartFunc);

  // test units with different case
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2\n rangeunits km\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2\n rangeunits Km\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 2\n rangeunits KM\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits sm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits Sm\n end\n", testCircularRadiusFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n radius 1.242742384474668\n rangeunits SM\n end\n", testCircularRadiusFunc);

  return rv;
}

// test that altitude mode is extrude and extrude height is 250.
auto testExtrudeFunc = [](const simCore::GOG::GogShape* shape) -> int
{
  int rv = 0;
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) == 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::EXTRUDE);
  double extrudeHeight = 0.;
  rv += SDK_ASSERT(shape->getExtrudeHeight(extrudeHeight) == 0);
  rv += SDK_ASSERT(extrudeHeight == 250.);
  return rv;
};

// test that altitude mode is clamp to ground
auto testClampFunc = [](const simCore::GOG::GogShape* shape) -> int
{
  int rv = 0;
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) == 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::CLAMP_TO_GROUND);
  return rv;
};

// test that altitude mode is not set
auto testAltModeUnsetFunc = [](const simCore::GOG::GogShape* shape) -> int
{
  int rv = 0;
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::EXTRUDE;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) != 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::NONE);
  return rv;
};
// test that all the altitude mode options work
int testAltitudeModes()
{
  int rv = 0;

  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n centerlla 24.4 43.2 0.\n extrude true 250.\n altitudeunits m\n end\n", testExtrudeFunc);
  rv += testShapeFunction<simCore::GOG::Line>("start\n line\n lla 24.4 43.2 0.\n lla 24.3 43.1 0.\n extrude true 250.\n altitudeunits m\n end\n", testExtrudeFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n centerlla 24.4 43.2 0.\n altitudemode clamptoground\n end\n", testClampFunc);
  rv += testShapeFunction<simCore::GOG::Line>("start\n line\n lla 24.4 43.2 0.\n lla 24.3 43.1 0.\n altitudemode clamptoground\n end\n", testClampFunc);
  // relative to ground is already tested in testShapesOptionalFields()

  // test shapes that don't support extrude ensure extrude is ignored
  rv += testShapeFunction<simCore::GOG::Points>("start\n points\n lla 24.4 43.2 0.\n lla 24.3 43.1 0.\n extrude true\n end\n", testAltModeUnsetFunc);
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n extrude true\n end\n", testAltModeUnsetFunc);
  return rv;
}


// test the different ways to define follow values
int testFollow()
{
  int rv = 0;
  // test absolute circle
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n centerlla 24.4 43.2 0.\n 3d follow cpr\n 3d offsetcourse 45.\n 3d offsetpitch 10.\n 3d offsetroll 5.\n end\n", testFollowFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n centerlla 24.4 43.2 0.\n rotate\n 3d offsetcourse 45.\n 3d offsetpitch 10.\n 3d offsetroll 5.\n end\n", testFollowFunc);
  // test relative line
  rv += testShapeFunction<simCore::GOG::Line>("start\n line\n xy 12. 23.\n xy 13. 24.\n 3d follow cpr\n 3d offsetcourse 45.\n 3d offsetpitch 10.\n 3d offsetroll 5.\n end\n", testFollowFunc);
  rv += testShapeFunction<simCore::GOG::Line>("start\n line\n xy 12. 23.\n xy 13. 24.\n rotate\n 3d offsetcourse 45.\n 3d offsetpitch 10.\n 3d offsetroll 5.\n end\n", testFollowFunc);
  // orient is already tested in testShapesOptionalFields()
  return rv;
}

// test that the circular shape has no center defined
auto testCircularNoCenterFunc = [](const simCore::GOG::CircularShape* shape) -> int
{
  int rv = 0;
  simCore::Vec3 center;
  rv += SDK_ASSERT(shape->getCenterPosition(center) != 0);
  return rv;
};

// test that the annotation shape has no position defined
auto testAnnotationNoPositionFunc = [](const simCore::GOG::Annotation* shape) -> int
{
  int rv = 0;
  simCore::Vec3 pos;
  rv += SDK_ASSERT(shape->getPosition(pos) != 0);
  return rv;
};

// test shapes that are centered will still parse if no center defined
int testCenteredDefault()
{
  int rv = 0;
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Sphere>("start\n sphere\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Hemisphere>("start\n hemisphere\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Ellipse>("start\n ellipse\n end\n", testCircularNoCenterFunc);
  rv += testShapeFunction<simCore::GOG::Annotation>("start\n annotation some name\n end\n", testAnnotationNoPositionFunc);

  return rv;
}

// test that the circular shape has default radius of 1000 yards
auto testCircularRadiusDefaultFunc = [](const simCore::GOG::CircularShape* shape) -> int
{
  int rv = 0;
  double radius = 0.;
  rv += SDK_ASSERT(shape->getRadius(radius) != 0);
  rv += SDK_ASSERT(simCore::areEqual(radius, 914.39997));
  return rv;
};

// test that the circular height shape has default height of 1000 feet
auto testCircularHeightDefaultHeightFunc = [](const simCore::GOG::CircularHeightShape* shape) -> int
{
  int rv = 0;
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) != 0);
  rv += SDK_ASSERT(simCore::areEqual(height, 304.79999));
  return rv;
};

// test that the cylinder shape has default height of 1000 feet
auto testCylinderDefaultHeightFunc = [](const simCore::GOG::Cylinder* shape) -> int
{
  int rv = 0;
  double height = 0.;
  rv += SDK_ASSERT(shape->getHeight(height) != 0);
  rv += SDK_ASSERT(simCore::areEqual(height, 304.79999));
  return rv;
};

// test the default height and radius values, which default to 1000, yards for radius and feet for height
int testDefaults()
{
  int rv = 0;
  // test default radius
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Arc>("start\n arc\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Ellipse>("start\n ellipse\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Sphere>("start\n sphere\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Hemisphere>("start\n hemisphere\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Orbit>("start\n orbit\n centerxyz 0 0 0\n centerxy2 10 10\n end\n", testCircularRadiusDefaultFunc);

  // test default height
  rv += testShapeFunction<simCore::GOG::Cone>("start\n cone\n end\n", testCircularHeightDefaultHeightFunc);
  rv += testShapeFunction<simCore::GOG::Ellipsoid>("start\n ellipsoid\n end\n", testCircularHeightDefaultHeightFunc);
  rv += testShapeFunction<simCore::GOG::Cylinder>("start\n cylinder\n end\n", testCylinderDefaultHeightFunc);

  return rv;
}

// tests parsing 1 shape, then serializing it back out, verifies that all the items in serializeItems are in the shape
template <typename ClassT>
int testSerializeShape(const std::string& gog, const std::vector<std::string>& serializedItems)
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  std::stringstream gogStr;
  gogStr << gog;
  parser.parse(gogStr, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    std::stringstream os;
    shapes.front()->serializeToStream(os);
    std::string serialized = os.str();
    // verify all the expected items are present in the serialized shape
    for (std::string item : serializedItems)
    {
      rv += SDK_ASSERT(serialized.find(item) != std::string::npos);
      if (serialized.find(item) == std::string::npos)
        std::cerr << "Failed to serialize : " << item << "\n";
    }

    // now that all items from serializedItems have been found in the serialized gog, verify that they both contain the same number of items
    std::vector<std::string> lines;
    simCore::escapeTokenize(lines, serialized, true, "\n");
    rv += SDK_ASSERT(lines.size() == serializedItems.size());

    if (rv > 0)
      std::cerr << serialized << "\n";

    // now parse the serialization back in to verify it produces a valid shape of the expected type
    shapes.clear();
    parser.parse(os, "", shapes);
    {
      rv += SDK_ASSERT(shapes.size() == 1);
      if (!shapes.empty())
      {
        rv += SDK_ASSERT(dynamic_cast<ClassT*>(shapes.front().get()) != nullptr);
      }
    }
  }
  return rv;
}

int testSerialization()
{
  int rv = 0;

  // serialized items that match those in BASE_FIELDS (excluding name)
  std::vector<std::string> baseItemsNoName;
  baseItemsNoName.push_back("altitudeunits m\n");
  baseItemsNoName.push_back("off\n");
  baseItemsNoName.push_back("depthbuffer true\n");
  baseItemsNoName.push_back("3d offsetalt 120\n");
  baseItemsNoName.push_back("altitudemode relativetoground\n");
  baseItemsNoName.push_back("scale 2 1.3 0.5\n");
  baseItemsNoName.push_back("verticaldatum egm1984\n");
  baseItemsNoName.push_back("starttime \"001 1970 00:00:00.00000\"");
  baseItemsNoName.push_back("endtime \"001 1970 01:00:00.00000\"");
  baseItemsNoName.push_back("start\n");
  baseItemsNoName.push_back("end\n");

  // all base items including the 3d name
  std::vector<std::string> baseItems = baseItemsNoName;
  baseItems.push_back("3d name my favorite shape\n");

  // follow items are in a separate list, since not all shapes support follow
  std::vector<std::string> baseFollowItems;
  baseFollowItems.push_back("3d follow cpr\n");
  baseFollowItems.push_back("3d offsetcourse 45\n");
  baseFollowItems.push_back("3d offsetpitch 10\n");
  baseFollowItems.push_back("3d offsetroll 5\n");

  // serialized item in OUTLINE_FIELD
  std::string outlineItem ="outline true";

  // serialized items that match those in FILLABLE_FIELDS
  std::vector<std::string> fillableItems;
  fillableItems.push_back(outlineItem);
  fillableItems.push_back("linewidth 4\n");
  fillableItems.push_back("linecolor hex 0xff00ff00\n");
  fillableItems.push_back("linestyle dashed\n");
  fillableItems.push_back("filled");
  fillableItems.push_back("fillcolor hex 0xff00ffff");

  // serialized items that match those in CIRCULAR_FIELDS; all circular shapes support follow
  std::vector<std::string> circularItems;
  circularItems.insert(circularItems.end(), baseItems.begin(), baseItems.end());
  circularItems.insert(circularItems.end(), baseFollowItems.begin(), baseFollowItems.end());
  circularItems.insert(circularItems.end(), fillableItems.begin(), fillableItems.end());
  circularItems.push_back("radius 1000\n");
  circularItems.push_back("rangeunits m\n");

  // serialized items that match those in ELLIPTICAL_FIELDS
  std::vector<std::string> ellipticalItems = circularItems;
  ellipticalItems.push_back("anglestart 10\n");
  ellipticalItems.push_back("angledeg 45\n");
  ellipticalItems.push_back("majoraxis 100\n");
  ellipticalItems.push_back("minoraxis 250\n");

  // serialized items that match those in POINTBASED_FIELDS; note that point based shapes will not always support follow
  std::vector<std::string> pointBasedItems;
  pointBasedItems.insert(pointBasedItems.end(), baseItems.begin(), baseItems.end());
  pointBasedItems.insert(pointBasedItems.end(), fillableItems.begin(), fillableItems.end());
  pointBasedItems.push_back("tessellate true\n");
  pointBasedItems.push_back("lineprojection greatcircle\n");

  // serialized items that match those in POINTS_FIELDS
  std::vector<std::string> pointItems = baseItems;
  pointItems.push_back(outlineItem);
  pointItems.push_back("pointsize 5\n");
  pointItems.push_back("linecolor hex 0xffc000c0\n");

  // serialized items that match those in ANNOTATION_FIELDS + BASE_FIELDS (excluding the 3d name, which is part of the annotation shape type line)
  std::vector<std::string> annotationItems = baseItemsNoName;
  annotationItems.push_back("annotation my favorite shape\n");
  annotationItems.push_back("fontname georgia.ttf\n");
  annotationItems.push_back("fontsize 24\n");
  annotationItems.push_back("linecolor hex 0x0affa0ff\n");
  annotationItems.push_back("textoutlinethickness thin\n");
  annotationItems.push_back("textoutlinecolor hex 0xffff0000\n");
  annotationItems.push_back("imagefile icon.png\n");
  annotationItems.push_back("priority 10\n");


  // define common center point for centered items
  const std::string centerLla = "centerlla 24.5 158.7 12\n";

  // test basic serialization for absolute shapes
  {
    std::vector<std::string> circleItems = circularItems;
    circleItems.push_back("circle\n");
    circleItems.push_back(centerLla);
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n " + centerLla + CIRCULAR_FIELDS + " end\n", circleItems);
  }
  {
    std::vector<std::string> arcItems = ellipticalItems;
    arcItems.push_back("arc\n");
    arcItems.push_back(centerLla);
    arcItems.push_back("innerradius 50\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\n arc\n " + centerLla + ARC_FIELDS + " end\n", arcItems);
  }
  {
    std::vector<std::string> ellipseItems = ellipticalItems;
    ellipseItems.push_back("ellipse\n");
    ellipseItems.push_back(centerLla);
    rv += testSerializeShape<simCore::GOG::Ellipse>("start\n ellipse\n " + centerLla + ELLIPTICAL_FIELDS + " end\n", ellipseItems);
  }
  {
    std::vector<std::string> cylinderItems = ellipticalItems;
    cylinderItems.push_back("cylinder\n");
    cylinderItems.push_back(centerLla);
    cylinderItems.push_back("height 1800");
    rv += testSerializeShape<simCore::GOG::Cylinder>("start\n cylinder\n height 1800\n" + centerLla + ELLIPTICAL_FIELDS + "end\n", cylinderItems);
  }
  {
    std::vector<std::string> sphereItems = circularItems;
    sphereItems.push_back("sphere\n");
    sphereItems.push_back(centerLla);
    rv += testSerializeShape<simCore::GOG::Sphere>("start\n sphere\n" + centerLla + CIRCULAR_FIELDS + "end\n", sphereItems);
  }
  {
    std::vector<std::string> hemisphereItems = circularItems;
    hemisphereItems.push_back("hemisphere\n");
    hemisphereItems.push_back(centerLla);
    rv += testSerializeShape<simCore::GOG::Hemisphere>("start\n hemisphere\n" + centerLla + CIRCULAR_FIELDS + "end\n", hemisphereItems);
  }
  {
    std::vector<std::string> coneItems = circularItems;
    coneItems.push_back("cone\n");
    coneItems.push_back(centerLla);
    coneItems.push_back("height 1800");
    rv += testSerializeShape<simCore::GOG::Cone>("start\n cone\n height 1800\n" + centerLla + CIRCULAR_FIELDS + "end\n", coneItems);
  }
  {
    std::vector<std::string> orbitItems = circularItems;
    orbitItems.push_back("orbit\n");
    orbitItems.push_back(centerLla);
    orbitItems.push_back("centerll2 25.6 159.7\n");
    rv += testSerializeShape<simCore::GOG::Orbit>("start\n orbit\n centerll2 25.6 159.7\n" + centerLla + CIRCULAR_FIELDS + "end\n", orbitItems);
  }
  {
    std::vector<std::string> ellipsoidItems = circularItems;
    ellipsoidItems.push_back("ellipsoid\n");
    ellipsoidItems.push_back(centerLla);
    ellipsoidItems.push_back("height 1800");
    ellipsoidItems.push_back("majoraxis 2000");
    ellipsoidItems.push_back("minoraxis 1500");
    rv += testSerializeShape<simCore::GOG::Ellipsoid>("start\n ellipsoid\n height 1800\n majoraxis 2000\n minoraxis 1500\n" + centerLla + CIRCULAR_FIELDS + "end\n", ellipsoidItems);
  }

  // use for testing shapes with multiple points
  std::string positionsStr = "lla 24.5 158.1 10\n lla 24.6 158.2 10\n lla 24.7 158.3 10\n lla 24.8 158.4 10\n";
  std::vector<std::string> positions;
  positions.push_back("lla 24.5 158.1 10\n");
  positions.push_back("lla 24.6 158.2 10\n");
  positions.push_back("lla 24.7 158.3 10\n");
  positions.push_back("lla 24.8 158.4 10\n");

  {
    std::vector<std::string> pointItemsAbs = pointItems;
    pointItemsAbs.push_back("points\n");
    pointItemsAbs.insert(pointItemsAbs.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Points>("start\n points\n" + positionsStr + POINTS_FIELDS + "end\n", pointItemsAbs);
  }
  {
    std::vector<std::string> lineItems = pointBasedItems;
    lineItems.push_back("line\n");
    lineItems.insert(lineItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Line>("start\n line\n" + positionsStr + POINTBASED_FIELDS + "end\n", lineItems);
  }
  {
    std::vector<std::string> lineSegItems = pointBasedItems;
    lineSegItems.push_back("linesegs\n");
    lineSegItems.insert(lineSegItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::LineSegs>("start\n linesegs\n" + positionsStr + POINTBASED_FIELDS + "end\n", lineSegItems);
  }
  {
    std::vector<std::string> polyItems = pointBasedItems;
    polyItems.push_back("polygon\n");
    polyItems.insert(polyItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Polygon>("start\n poly\n" + positionsStr + POINTBASED_FIELDS + "end\n", polyItems);
  }
  {
    std::vector<std::string> annotationAbsItems = annotationItems;
    annotationAbsItems.push_back("lla 24.5 54.6 0\n");
    // note the difference in input vs output position format
    rv += testSerializeShape<simCore::GOG::Annotation>("start\n annotation my favorite shape\n centerll 24.5 54.6\n" + ANNOTATION_FIELDS + BASE_FIELDS + "end\n", annotationAbsItems);
  }
  {
    std::vector<std::string> llabItems = baseItems;
    llabItems.insert(llabItems.end(), fillableItems.begin(), fillableItems.end());
    llabItems.push_back("latlonaltbox 25.1 25.2 130.1 130.2 1000 1000\n");
    rv += testSerializeShape<simCore::GOG::LatLonAltBox>("start\n latlonaltbox 25.1 25.2 130.1 130.2 1000 1000\n" + FILLABLE_FIELDS + "end\n", llabItems);
  }
  {
    std::vector<std::string> imageOverlayItems = baseItems;
    imageOverlayItems.push_back("imageoverlay 25.1 25.3 55.4 55.6 0\n");
    imageOverlayItems.push_back("imagefile image.png\n");
    rv += testSerializeShape<simCore::GOG::ImageOverlay>("start\n imageoverlay 25.1 25.3 55.4 55.6 0\n imagefile image.png\n" + BASE_FIELDS + "end\n", imageOverlayItems);
    // Note that 1.0 writes as "1" in text output
    imageOverlayItems.push_back("opacity 1\n");
    // Tests the opacity keyword
    rv += testSerializeShape<simCore::GOG::ImageOverlay>("start\n imageoverlay 25.1 25.3 55.4 55.6 0\n imagefile image.png\n" + BASE_FIELDS + "opacity 1.0\nend\n", imageOverlayItems);
    // Tests clamping on opacity
    rv += testSerializeShape<simCore::GOG::ImageOverlay>("start\n imageoverlay 25.1 25.3 55.4 55.6 0\n imagefile image.png\n" + BASE_FIELDS + "opacity 3.5\nend\n", imageOverlayItems);
    // Tests non-1.0 opacity
    imageOverlayItems.back() = "opacity 0.5\n";
    rv += testSerializeShape<simCore::GOG::ImageOverlay>("start\n imageoverlay 25.1 25.3 55.4 55.6 0\n imagefile image.png\n" + BASE_FIELDS + "opacity 0.5\nend\n", imageOverlayItems);
  }

  // relative shapes

  // define common center xyz for centered items
  const std::string centerXyz = "centerxyz 100 1000 10\n";
  // add the ref item for the relative shapes
  circularItems.push_back("ref 24.5 55.6 10\n");
  ellipticalItems.push_back("ref 24.5 55.6 10\n");

  // test basic serialization for relative shapes
  {
    std::vector<std::string> circleItems = circularItems;
    circleItems.push_back("circle\n");
    circleItems.push_back(centerXyz);
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n " + centerXyz + CIRCULAR_FIELDS + " end\n", circleItems);
  }
  {
    std::vector<std::string> arcItems = ellipticalItems;
    arcItems.push_back("arc\n");
    arcItems.push_back(centerXyz);
    rv += testSerializeShape<simCore::GOG::Arc>("start\n arc\n " + centerXyz + ELLIPTICAL_FIELDS + " end\n", arcItems);
  }
  {
    std::vector<std::string> ellipseItems = ellipticalItems;
    ellipseItems.push_back("ellipse\n");
    ellipseItems.push_back(centerXyz);
    rv += testSerializeShape<simCore::GOG::Ellipse>("start\n ellipse\n " + centerXyz + ELLIPTICAL_FIELDS + " end\n", ellipseItems);
  }
  {
    std::vector<std::string> cylinderItems = ellipticalItems;
    cylinderItems.push_back("cylinder\n");
    cylinderItems.push_back(centerXyz);
    cylinderItems.push_back("height 1800");
    rv += testSerializeShape<simCore::GOG::Cylinder>("start\n cylinder\n height 1800\n" + centerXyz + ELLIPTICAL_FIELDS + "end\n", cylinderItems);
  }
  {
    std::vector<std::string> sphereItems = circularItems;
    sphereItems.push_back("sphere\n");
    sphereItems.push_back(centerXyz);
    rv += testSerializeShape<simCore::GOG::Sphere>("start\n sphere\n" + centerXyz + CIRCULAR_FIELDS + "end\n", sphereItems);
  }
  {
    std::vector<std::string> hemisphereItems = circularItems;
    hemisphereItems.push_back("hemisphere\n");
    hemisphereItems.push_back(centerXyz);
    rv += testSerializeShape<simCore::GOG::Hemisphere>("start\n hemisphere\n" + centerXyz + CIRCULAR_FIELDS + "end\n", hemisphereItems);
  }
  {
    std::vector<std::string> coneItems = circularItems;
    coneItems.push_back("cone\n");
    coneItems.push_back(centerXyz);
    coneItems.push_back("height 1800");
    rv += testSerializeShape<simCore::GOG::Cone>("start\n cone\n height 1800\n" + centerXyz + CIRCULAR_FIELDS + "end\n", coneItems);
  }
  {
    std::vector<std::string> orbitItems = circularItems;
    orbitItems.push_back("orbit\n");
    orbitItems.push_back(centerXyz);
    orbitItems.push_back("centerxy2 2000 150\n");
    rv += testSerializeShape<simCore::GOG::Orbit>("start\n orbit\n centerxy2 2000 150\n" + centerXyz + CIRCULAR_FIELDS + "end\n", orbitItems);
  }
  {
    std::vector<std::string> ellipsoidItems = circularItems;
    ellipsoidItems.push_back("ellipsoid\n");
    ellipsoidItems.push_back(centerXyz);
    ellipsoidItems.push_back("height 1800");
    ellipsoidItems.push_back("majoraxis 2000");
    ellipsoidItems.push_back("minoraxis 1500");
    rv += testSerializeShape<simCore::GOG::Ellipsoid>("start\n ellipsoid\n height 1800\n majoraxis 2000\n minoraxis 1500\n" + centerXyz + CIRCULAR_FIELDS + "end\n", ellipsoidItems);
  }

  // use for testing shapes with multiple points
  positionsStr = "xyz 100 150 10\n xyz -100 150 10\n xyz 150 200 10\n xyz -150 340 10\n";
  positions.clear();
  positions.push_back("xyz 100 150 10\n");
  positions.push_back("xyz -100 150 10\n");
  positions.push_back("xyz 150 200 10\n");
  positions.push_back("xyz -150 340 10\n");

  // add follow items which are valid for relative shapes
  pointBasedItems.insert(pointBasedItems.end(), baseFollowItems.begin(), baseFollowItems.end());
  pointItems.insert(pointItems.end(), baseFollowItems.begin(), baseFollowItems.end());
  // add the ref item for the relative shapes
  pointBasedItems.push_back("ref 24.5 55.6 10\n");
  pointItems.push_back("ref 24.5 55.6 10\n");
  annotationItems.push_back("ref 24.5 55.6 10\n");

  {
    std::vector<std::string> pointItemsAbs = pointItems;
    pointItemsAbs.push_back("points\n");
    pointItemsAbs.insert(pointItemsAbs.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Points>("start\n points\n" + positionsStr + POINTS_FIELDS + "end\n", pointItemsAbs);
  }
  {
    std::vector<std::string> lineItems = pointBasedItems;
    lineItems.push_back("line\n");
    lineItems.insert(lineItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Line>("start\n line\n" + positionsStr + POINTBASED_FIELDS + "end\n", lineItems);
  }
  {
    std::vector<std::string> lineSegItems = pointBasedItems;
    lineSegItems.push_back("linesegs\n");
    lineSegItems.insert(lineSegItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::LineSegs>("start\n linesegs\n" + positionsStr + POINTBASED_FIELDS + "end\n", lineSegItems);
  }
  {
    std::vector<std::string> polyItems = pointBasedItems;
    polyItems.push_back("polygon\n");
    polyItems.insert(polyItems.end(), positions.begin(), positions.end());
    rv += testSerializeShape<simCore::GOG::Polygon>("start\n poly\n" + positionsStr + POINTBASED_FIELDS + "end\n", polyItems);
  }
  {
    std::vector<std::string> annotationRelItems = annotationItems;
    annotationRelItems.push_back("xyz 150 250 0\n");
    // note the difference in input vs output position format
    rv += testSerializeShape<simCore::GOG::Annotation>("start\n annotation my favorite shape\n centerxy 150 250\n" + ANNOTATION_FIELDS + BASE_FIELDS + "end\n", annotationRelItems);
  }

  // no relative latlonaltbox or image overlay

  // test serializing values with multiple input options

  // test radius/diameter
  std::vector<std::string> radiusItems;
  radiusItems.push_back("start\n");
  radiusItems.push_back("end\n");
  radiusItems.push_back("circle\n");
  radiusItems.push_back("radius 2500\n");
  rv += testSerializeShape<simCore::GOG::Circle>("start\ncircle\nradius 2500\nend\n", radiusItems);
  rv += testSerializeShape<simCore::GOG::Circle>("start\ncircle\ndiameter 5000\nend\n", radiusItems);

  // test orient, rotate, 3d follow
  {
    std::vector<std::string> followItems = baseFollowItems;
    followItems.push_back("start\n");
    followItems.push_back("end\n");
    followItems.push_back("arc\n");
    followItems.push_back("centerxyz 0 0 0");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterxy 0 0\n orient 45 10 5\n end\n", followItems);
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterxy 0 0\n rotate\n 3d offsetcourse 45\n 3d offsetpitch 10\n 3d offsetroll 5\n end\n", followItems);
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterxy 0 0\n 3d follow cpr\n 3d offsetcourse 45\n 3d offsetpitch 10\n 3d offsetroll 5\n end\n", followItems);
  }
  // test specific follow components
  std::vector<std::string> arcShapeItems;
  arcShapeItems.push_back("start\n");
  arcShapeItems.push_back("end\n");
  arcShapeItems.push_back("arc\n");
  arcShapeItems.push_back("centerlla 24.1 134.5 0");
  {
    std::vector<std::string> followItems = arcShapeItems;
    followItems.push_back("3d follow c\n");
    followItems.push_back("3d offsetcourse 45\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterll 24.1 134.5\n orient 45\n end\n", followItems);
  }
  {
    std::vector<std::string> followItems = arcShapeItems;
    followItems.push_back("3d follow cp\n");
    followItems.push_back("3d offsetcourse 45\n");
    followItems.push_back("3d offsetpitch 10\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterll 24.1 134.5\n orient 45 10\n end\n", followItems);
  }
  {
    std::vector<std::string> followItems = arcShapeItems;
    followItems.push_back("3d follow c\n");
    followItems.push_back("3d offsetcourse 45\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterll 24.1 134.5\n 3d follow c\n 3d offsetcourse 45\n end\n", followItems);
  }
  {
    std::vector<std::string> followItems = arcShapeItems;
    followItems.push_back("3d follow p\n");
    followItems.push_back("3d offsetpitch 10\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterll 24.1 134.5\n 3d follow p\n 3d offsetpitch 10\n end\n", followItems);
  }
  {
    std::vector<std::string> followItems = arcShapeItems;
    followItems.push_back("3d follow p\n");
    followItems.push_back("3d offsetroll 5\n");
    rv += testSerializeShape<simCore::GOG::Arc>("start\narc\ncenterll 24.1 134.5\n 3d follow p\n 3d offsetroll 5\n end\n", followItems);
  }

  // test semimajoraxis/majoraxis, semiminoraxis/minoraxis
  {
    std::vector<std::string> ellipseItems;
    ellipseItems.push_back("start\n");
    ellipseItems.push_back("end\n");
    ellipseItems.push_back("ellipse\n");
    ellipseItems.push_back("majoraxis 350\n");
    ellipseItems.push_back("minoraxis 220\n");
    rv += testSerializeShape<simCore::GOG::Ellipse>("start\n ellipse\n majoraxis 350\n minoraxis 220\n end\n", ellipseItems);
    rv += testSerializeShape<simCore::GOG::Ellipse>("start\n ellipse\n semimajoraxis 175\n semiminoraxis 110\n end\n", ellipseItems);
  }

  // test altitude modes
  std::vector<std::string> simpleCircleItems;
  simpleCircleItems.push_back("start\n");
  simpleCircleItems.push_back("end\n");
  simpleCircleItems.push_back("circle\n");
  {
    std::vector<std::string> altModeItems = simpleCircleItems;
    altModeItems.push_back("altitudemode clamptoground\n");
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n altitudemode clamptoground\n end\n", altModeItems);
  }
  {
    std::vector<std::string> altModeItems = simpleCircleItems;
    altModeItems.push_back("altitudemode relativetoground\n");
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n altitudemode relativetoground\n end\n", altModeItems);
  }
  {
    std::vector<std::string> altModeItems = simpleCircleItems;
    altModeItems.push_back("extrude true\n");
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n altitudemode extrude\n end\n", altModeItems);
  }
  {
    std::vector<std::string> altModeItems = simpleCircleItems;
    altModeItems.push_back("extrude true\n");
    rv += testSerializeShape<simCore::GOG::Circle>("start\n circle\n extrude true\n end\n", altModeItems);
  }

  return rv;
}

int testColors()
{
  int rv = 0;

  std::vector<std::string> colorStringsInput;
  colorStringsInput.push_back("linecolor hex 0\n"); // test 0
  colorStringsInput.push_back("linecolor color2\n"); // test custom defined color
  colorStringsInput.push_back("linecolor blue\n"); // test defined color
  colorStringsInput.push_back("linecolor hex 01ff0aff\n"); // test different hex format
  colorStringsInput.push_back("linecolor invalidColorString\n"); // test invalid color, should default to red
  colorStringsInput.push_back("linecolor blahblah 0x00ff00ff\n"); // test that for 3 tokens, 2nd is ignored and 3rd is the color value
  colorStringsInput.push_back("linecolor whatever 148f320c\n"); // test that for 3 tokens, 2nd is ignored and 3rd is the color value for hex without leading '0x'
  colorStringsInput.push_back("linecolor white 2576980479\n"); // test unsigned int instead of hex format (note more than 8 digits, can't be hex)

  std::vector<std::string> colorStringsOutput;
  colorStringsOutput.push_back("linecolor hex 0x00000000\n");
  colorStringsOutput.push_back("linecolor hex 0x0000ac02\n");
  colorStringsOutput.push_back("linecolor hex 0xffff0000\n");
  colorStringsOutput.push_back("linecolor hex 0x01ff0aff\n");
  colorStringsOutput.push_back("linecolor hex 0xff0000ff\n");
  colorStringsOutput.push_back("linecolor hex 0x00ff00ff\n");
  colorStringsOutput.push_back("linecolor hex 0x148f320c\n");
  colorStringsOutput.push_back("linecolor hex 0x999999ff\n");

  simCore::GOG::Parser parser;
  // set a custom defined color
  parser.addOverwriteColor("color2", "0000ac02");
  std::vector<simCore::GOG::GogShapePtr> shapes;
  std::stringstream gogStr;
  for (std::string colorStr : colorStringsInput)
    gogStr << "start\n circle\n" << colorStr << "end\n";
  parser.parse(gogStr, "", shapes);
  rv += SDK_ASSERT(shapes.size() == colorStringsInput.size());

  std::vector<simCore::GOG::Color> colors;
  colors.push_back(simCore::GOG::Color(0, 0, 0, 0));
  colors.push_back(simCore::GOG::Color(2, 172, 0, 0));
  colors.push_back(simCore::GOG::Color(0, 0, 255, 255));
  colors.push_back(simCore::GOG::Color(255, 10, 255, 1));
  colors.push_back(simCore::GOG::Color(255, 0, 0, 255));
  colors.push_back(simCore::GOG::Color(255, 0, 255, 0));
  colors.push_back(simCore::GOG::Color(12, 50, 143, 20));
  colors.push_back(simCore::GOG::Color(255, 153, 153, 153));

  int i = 0;
  for (simCore::GOG::GogShapePtr shape : shapes)
  {
    simCore::GOG::Circle* circle = dynamic_cast<simCore::GOG::Circle*>(shape.get());
    rv += SDK_ASSERT(circle != nullptr);
    if (circle != nullptr)
    {
      simCore::GOG::Color color;
      circle->getLineColor(color);
      rv += SDK_ASSERT(color == colors[i]);
      std::stringstream os;
      circle->serializeToStream(os);
      std::string serialized = os.str();
      rv += SDK_ASSERT(serialized.find(colorStringsOutput[i]) != std::string::npos);
    }
    i++;
  }

  return rv;
}


// test that filled is true, outlined is false, and extrude is true
auto testBooleansFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = 0;
  bool filled;
  rv += SDK_ASSERT(shape->getIsFilled(filled) == 0);
  rv += SDK_ASSERT(filled);
  bool outlined;
  rv += SDK_ASSERT(shape->getIsOutlined(outlined) == 0);
  rv += SDK_ASSERT(!outlined);
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
  rv += SDK_ASSERT(shape->getAltitudeMode(mode) == 0);
  rv += SDK_ASSERT(mode == simCore::GOG::AltitudeMode::EXTRUDE);
  return rv;
};

// test the different boolean strings
int testBooleanInputs()
{
  int rv = 0;
  // filled will always be true, since it only requires the presence of the single keyword
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\nfilled false\n outline false\n extrude true\n end\n", testCircularRadiusDefaultFunc);
  // outline will be false, since any string that does not explicitly indicate true is parsed as false
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n filled no\n outline unknown\n extrude true\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n filled\n outline 0\n extrude 1\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n filled\n outline off\n extrude on\n end\n", testCircularRadiusDefaultFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\n circle\n filled\n outline no\n extrude yes\n end\n", testCircularRadiusDefaultFunc);
  return rv;
}


auto testPointSizeFunc = [](const simCore::GOG::Points* shape) -> int
{
  int rv = 0;
  int pointSize = 0;
  rv += SDK_ASSERT(shape->getPointSize(pointSize) == 0);
  rv += SDK_ASSERT(pointSize == 3);
  return rv;
};

auto testLineWidthFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = 0;
  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth) == 0);
  rv += SDK_ASSERT(lineWidth == 3);
  return rv;
};

auto testTextSizeFunc = [](const simCore::GOG::Annotation* shape) -> int
{
  int rv = 0;
  int textSize = 0;
  rv += SDK_ASSERT(shape->getTextSize(textSize) == 0);
  rv += SDK_ASSERT(textSize == 3);
  return rv;
};

int testDoublesToInts()
{
  int rv = 0;

  rv += testShapeFunction<simCore::GOG::Points>("start\npoints\nlla 1 1 1\n pointsize 2.5\nend\n", testPointSizeFunc);
  rv += testShapeFunction<simCore::GOG::FillableShape>("start\ncircle\ncenterlla 1 1 1\n linewidth 2.5\nend\n", testLineWidthFunc);
  rv += testShapeFunction<simCore::GOG::Annotation>("start\nannotation test\ncenterlla 1 1 1\n fontsize 2.5\nend\n", testTextSizeFunc);

  return rv;
}

auto testLineWidthThinFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = 0;
  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth)== 0);
  rv += SDK_ASSERT(lineWidth == 1);
  return rv;
};

auto testLineWidthMediumFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = 0;
  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth) == 0);
  rv += SDK_ASSERT(lineWidth == 2);
  return rv;
};

auto testLineWidthThickFunc = [](const simCore::GOG::FillableShape* shape) -> int
{
  int rv = 0;
  int lineWidth = 0;
  rv += SDK_ASSERT(shape->getLineWidth(lineWidth) == 0);
  rv += SDK_ASSERT(lineWidth == 4);
  return rv;
};

int testLineWidthStrings()
{
  int rv = 0;
  rv += testShapeFunction<simCore::GOG::Circle>("start\ncircle\ncenterll 1 1\nlinewidth thin\nend\n", testLineWidthThinFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\ncircle\ncenterll 1 1\nlinewidth med\nend\n", testLineWidthMediumFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\ncircle\ncenterll 1 1\nlinewidth medium\nend\n", testLineWidthMediumFunc);
  rv += testShapeFunction<simCore::GOG::Circle>("start\ncircle\ncenterll 1 1\nlinewidth thick\nend\n", testLineWidthThickFunc);
  return rv;
}

int testTimeStrings()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  simCore::TimeStamp stamp;

  // Test neither start nor end
  std::stringstream noTime;
  noTime << "start\nannotation test\ncenterll 30 30\nend\n";
  parser.parse(noTime, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
    rv += SDK_ASSERT(shape->getEndTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  }
  shapes.clear();

  // Test start time without end time
  std::stringstream startOnly;
  startOnly << "start\nannotation test\ncenterll 30 30\nstarttime \"001 1970 00:00:00.00000\"\nend\n";
  parser.parse(startOnly, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) == 0);
    rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 0));
    rv += SDK_ASSERT(shape->getEndTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  }
  shapes.clear();

  // Test end time without start time
  std::stringstream endOnly;
  endOnly << "start\nannotation test\ncenterll 30 30\nendtime \"001 1970 00:00:00.00000\"\nend\n";
  parser.parse(endOnly, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
    rv += SDK_ASSERT(shape->getEndTime(stamp) == 0);
    rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 0));
  }
  shapes.clear();

  // Test both start and end time
  std::stringstream startAndEnd;
  startAndEnd << "start\nannotation test\ncenterll 30 30\nstarttime \"001 1970 00:00:00.00000\"\nendtime \"001 1970 01:00:00.00000\"\nend\n";
  parser.parse(startAndEnd, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) == 0);
    rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 0));
    rv += SDK_ASSERT(shape->getEndTime(stamp) == 0);
    rv += SDK_ASSERT(stamp == simCore::TimeStamp(1970, 3600));
  }
  shapes.clear();

  // Test start time later than end time
  std::stringstream reversed;
  reversed << "start\nannotation test\ncenterll 30 30\nstarttime \"001 1970 01:00:00.00000\"\nendtime \"001 1970 00:00:00.00000\"\nend\n";
  parser.parse(reversed, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
    rv += SDK_ASSERT(shape->getEndTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  }
  shapes.clear();

  // Test that a time format lacking a year is not accepted
  std::stringstream unreferenced;
  unreferenced << "start\nannotation test\ncenterll 30 30\nstarttime \"00:00:00\"\nendtime \"01:00:00\"\nend\n";
  parser.parse(unreferenced, "", shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    auto shape = shapes.front();
    rv += SDK_ASSERT(shape->getStartTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
    rv += SDK_ASSERT(shape->getEndTime(stamp) != 0);
    rv += SDK_ASSERT(stamp == simCore::INFINITE_TIME_STAMP);
  }
  shapes.clear();

  return rv;
}

int testReferencePositionField()
{
  int rv = 0;

  simCore::GOG::Line line(false);
  simCore::Vec3 refPoint(1, 2, 3);
  line.setReferencePosition(refPoint);
  simCore::Vec3 setRefPoint;
  // ref position should not be set when shape is absolute
  rv += SDK_ASSERT(line.getReferencePosition(setRefPoint) != 0);
  rv += SDK_ASSERT(setRefPoint != refPoint);
  line.setRelative(true);
  line.setReferencePosition(refPoint);
  // ref position should now be set successfully since shape is relative
  rv += SDK_ASSERT(line.getReferencePosition(setRefPoint) == 0);
  rv += SDK_ASSERT(setRefPoint == refPoint);
  line.clearReferencePosition();
  // clearing ref position should work, even when relative
  rv += SDK_ASSERT(line.getReferencePosition(setRefPoint) != 0);
  rv += SDK_ASSERT(setRefPoint != refPoint);

  line.setReferencePosition(refPoint);
  // verify ref position set successfully again
  rv += SDK_ASSERT(line.getReferencePosition(setRefPoint) == 0);
  rv += SDK_ASSERT(setRefPoint == refPoint);
  line.setRelative(false);
  // ref position should be cleared when shape was set to absolute
  rv += SDK_ASSERT(line.getReferencePosition(setRefPoint) != 0);
  rv += SDK_ASSERT(setRefPoint != refPoint);

  return rv;
}

}

int GogTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;

  rv += testGeneralSyntax();
  rv += testMinimalShapes();
  rv += testIncompleteShapes();
  rv += testShapesOptionalFields();
  rv += testAnnotation();
  rv += testUnits();
  rv += testAltitudeModes();
  rv += testFollow();
  rv += testCenteredDefault();
  rv += testDefaults();
  rv += testSerialization();
  rv += testColors();
  rv += testBooleanInputs();
  rv += testDoublesToInts();
  rv += testLineWidthStrings();
  rv += testTimeStrings();
  rv += testReferencePositionField();

  return rv;
}

