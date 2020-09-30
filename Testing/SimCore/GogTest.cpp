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
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Units.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/GOG/GogShape.h"
#include "simCore/GOG/Parser.h"

namespace {

// outlined shape optional field in GOG format
static const std::string OUTLINED_FIELD = "outline true\n";
// fillable shape optional fields in GOG format
static const std::string FILLABLE_FIELDS = OUTLINED_FIELD + "linewidth 4\n linecolor green\n linestyle dashed\n filled\n fillcolor yellow\n";
// circular shape optional fields in GOG format (in meters for testing)
static const std::string CIRCULAR_FIELDS = FILLABLE_FIELDS + " radius 1000.\n rangeunits m\n";
// point based shape optional field in GOG format
static const std::string POINTBASED_FIELDS = FILLABLE_FIELDS + " tessellate true\n lineprojection greatcircle\n";

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
  parser.parse(missingEnd, shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  // test file with missing start fails to create shape
  std::stringstream missingStart;
  missingStart << "circle\n end\n";
  parser.parse(missingStart, shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  // test file with multiple keywords between start/end fails to create shape
  std::stringstream uncertainShape;
  uncertainShape << "start\n circle\n line\n centerlla 25.1 58.2 0.\n end\n";
  parser.parse(uncertainShape, shapes);
  rv += SDK_ASSERT(shapes.empty());
  shapes.clear();

  return rv;
}

// test that the specified circular gog string parses to the specified object, and that its center position matches the specified centerPos, and that radius was not set
template <typename T>
int testMinimalCircularShape(const std::string& gog, const simCore::Vec3& centerPos)
{
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  int rv = 0;

  std::stringstream gogStr;
  gogStr << gog;
  parser.parse(gogStr, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    T* shape = dynamic_cast<T*>(shapes.front().get());
    rv += SDK_ASSERT(shape != nullptr);
    if (shape)
    {
      const simCore::Vec3& center = shape->centerPosition();
      rv += SDK_ASSERT(comparePositions(center, centerPos));
      double radius = 0.;
      // verify radius wasn't set
      rv += SDK_ASSERT(shape->getRadius(radius) == 1);
      // verify default value was returned
      rv += SDK_ASSERT(radius == 500.);
    }
  }
  shapes.clear();
  return rv;
}

// test that the specified point based gog string parses to the specified object, and that its positions match the specified positions and that tessellation was not set
template <typename T>
int testMinimalPointBasedShape(const std::string& gog, const std::vector<simCore::Vec3>& positions)
{
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;
  int rv = 0;

  std::stringstream gogStr;
  gogStr << gog;
  parser.parse(gogStr, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    T* shape = dynamic_cast<T*>(shapes.front().get());
    rv += SDK_ASSERT(shape != nullptr);
    if (shape)
    {
      const std::vector<simCore::Vec3>& positionsOut = shape->points();
      rv += SDK_ASSERT(positions.size() == positionsOut.size());
      rv += SDK_ASSERT(comparePositionVectors(positions, positionsOut));

      // verify that tessellation has not been set
      simCore::GOG::PointBasedShape::TessellationStyle style = simCore::GOG::PointBasedShape::TessellationStyle::NONE;
      rv += SDK_ASSERT(shape->getTessellation(style) == 1);
      rv += SDK_ASSERT(style == simCore::GOG::PointBasedShape::TessellationStyle::NONE);
    }
  }
  shapes.clear();
  return rv;
}

// test shapes with only minimum required fields set
int testMinimalShapes()
{
  int rv = 0;

  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test circle
  rv += testMinimalCircularShape<simCore::GOG::Circle>("start\n circle\n centerlla 25.1 58.2 0.\n end\n", simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  // test sphere
  rv += testMinimalCircularShape<simCore::GOG::Sphere>("start\n sphere\n centerlla 25.1 58.2 0.\n end\n", simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  // test hemisphere
  rv += testMinimalCircularShape<simCore::GOG::Hemisphere>("start\n hemisphere\n centerlla 25.1 58.2 0.\n end\n", simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));

  // test orbit
  std::stringstream orbitGog;
  orbitGog << "start\n orbit\n centerlla 24.4 43.2 0.0\n centerll2 24.1 43.5\n end\n";
  parser.parse(orbitGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Orbit* orbit = dynamic_cast<simCore::GOG::Orbit*>(shapes.front().get());
    rv += SDK_ASSERT(orbit != nullptr);
    if (orbit)
    {
      const simCore::Vec3& center = orbit->centerPosition();
      rv += SDK_ASSERT(comparePositions(center, simCore::Vec3(24.4 * simCore::DEG2RAD, 43.2 * simCore::DEG2RAD, 0.)));
      const simCore::Vec3& center2 = orbit->centerPosition2();
      rv += SDK_ASSERT(comparePositions(center2, simCore::Vec3(24.1 * simCore::DEG2RAD, 43.5 * simCore::DEG2RAD, 0.)));

      double radius = 0.;
      // verify radius wasn't set
      rv += SDK_ASSERT(orbit->getRadius(radius) == 1);
      // verify default value was returned
      rv += SDK_ASSERT(radius == 500.);
    }
  }
  shapes.clear();

  // test line
  std::vector<simCore::Vec3> linePoints;
  linePoints.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  linePoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, 0.));
  rv += testMinimalPointBasedShape<simCore::GOG::Line>("start\n line\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n end\n", linePoints);

  // test linesegs
  std::vector<simCore::Vec3> lineSegPoints;
  lineSegPoints.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  lineSegPoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, 0.));
  rv += testMinimalPointBasedShape<simCore::GOG::LineSegs>("start\n linesegs\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n end\n", lineSegPoints);

  // test polygon
  std::vector<simCore::Vec3> polyPoints;
  polyPoints.push_back(simCore::Vec3(25.1 * simCore::DEG2RAD, 58.2 * simCore::DEG2RAD, 0.));
  polyPoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 58.3 * simCore::DEG2RAD, 0.));
  polyPoints.push_back(simCore::Vec3(26.2 * simCore::DEG2RAD, 57.9 * simCore::DEG2RAD, 0.));
  rv += testMinimalPointBasedShape<simCore::GOG::Polygon>("start\n poly\n lla 25.1 58.2 0.\n lla 26.2 58.3 0.\n lla 26.2 57.9 0.\n end\n", polyPoints);

  return rv;
}

// test that the shape's optional field matches the pre-defined test fields from OUTLINED_FIELD
int testOutlinedField(const simCore::GOG::OutlinedShape* shape)
{
  int rv = 0;

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

  simCore::GOG::FillableShape::LineStyle style = simCore::GOG::FillableShape::LineStyle::SOLID;
  rv += SDK_ASSERT(shape->getLineStyle(style) == 0);
  rv += SDK_ASSERT(style == simCore::GOG::FillableShape::LineStyle::DASHED);

  simCore::GOG::GogShape::Color lineColor;
  rv += SDK_ASSERT(shape->getLineColor(lineColor) == 0);
  rv += SDK_ASSERT(lineColor == simCore::GOG::GogShape::Color(0, 255, 0, 255));

  bool filled = false;
  rv += SDK_ASSERT(shape->getIsFilled(filled) == 0);
  rv += SDK_ASSERT(filled);

  simCore::GOG::GogShape::Color fillColor;
  rv += SDK_ASSERT(shape->getFillColor(fillColor) == 0);
  rv += SDK_ASSERT(fillColor == simCore::GOG::GogShape::Color(255, 255, 0, 255));

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
  simCore::GOG::PointBasedShape::TessellationStyle style = simCore::GOG::PointBasedShape::TessellationStyle::NONE;
  rv += SDK_ASSERT(shape->getTessellation(style) == 0);
  rv += SDK_ASSERT(style == simCore::GOG::PointBasedShape::TessellationStyle::GREAT_CIRCLE);
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
  parser.parse(gogStr, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    ClassT* shape = dynamic_cast<ClassT*>(shapes.front().get());
    rv += SDK_ASSERT(shape);
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

  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

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

  return rv;
}

// test shapes that have required fields to ensure they are not created if required field is missing
int testIncompleteShapes()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test circle (requires center point)
  std::stringstream circleGogIncomplete;
  circleGogIncomplete << "start\n circle\n end\n";
  parser.parse(circleGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test sphere (requires center point)
  std::stringstream sphereGogIncomplete;
  sphereGogIncomplete << "start\n sphere\n end\n";
  parser.parse(sphereGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test hemisphere (requires center point)
  std::stringstream hemiGogIncomplete;
  hemiGogIncomplete << "start\n hemisphere\n end\n";
  parser.parse(hemiGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test orbit (requires center point)
  std::stringstream orbitCtr2GogIncomplete;
  orbitCtr2GogIncomplete << "start\n orbit\n centerll2 23.4 45.2\n end\n";
  parser.parse(orbitCtr2GogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test orbit (requires center point 2)
  std::stringstream orbitCtrGogIncomplete;
  orbitCtrGogIncomplete << "start\n orbit\n centerll 23.4 45.2\n end\n";
  parser.parse(orbitCtrGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test line (requires 2 points minimum)
  std::stringstream lineGogIncomplete;
  lineGogIncomplete << "start\n line\n lla 25.1 58.2 0.\n end\n";
  parser.parse(lineGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test line segs (requires 2 points minimum)
  std::stringstream lineSegsGogIncomplete;
  lineSegsGogIncomplete << "start\n linesegs\n lla 25.1 58.2 0.\n end\n";
  parser.parse(lineSegsGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test polygon (requires 3 points minimum)
  std::stringstream polyGogIncomplete;
  polyGogIncomplete << "start\n poly\n lla 25.1 58.2 0.\n lla 25.1 58.3 0.\n end\n";
  parser.parse(polyGogIncomplete, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test annotation (requires position)
  std::stringstream annoTextGog;
  annoTextGog << "start\n annotation label 1\n end\n";
  parser.parse(annoTextGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  // test annotation (requires text)
  std::stringstream annoCtrGog;
  annoCtrGog << "start\n annotation\n centerlla 24.2 43.3 0.\n end\n";
  parser.parse(annoCtrGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 0);
  shapes.clear();

  return rv;
}

// test all the annotation fields and nested annotations special case
int testAnnotation()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test annotation with only required fields set
  std::stringstream annoMinimalGog;
  annoMinimalGog << "start\n annotation label 1\n centerll 24.5 54.6\n end\n";
  parser.parse(annoMinimalGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(shapes.front().get());
    rv += SDK_ASSERT(anno != nullptr);
    if (anno)
    {
      rv += SDK_ASSERT(anno->text() == "label 1");
      rv += SDK_ASSERT(comparePositions(anno->position(), simCore::Vec3(24.5 * simCore::DEG2RAD, 54.6 * simCore::DEG2RAD, 0.)));
      // make sure optional fields were not set
      std::string fontName;
      rv += SDK_ASSERT(anno->getFontName(fontName) == 1);
      int textSize = 0;
      rv += SDK_ASSERT(anno->getTextSize(textSize) == 1);
      simCore::GOG::GogShape::Color textColor;
      rv += SDK_ASSERT(anno->getTextColor(textColor) == 1);
      simCore::GOG::GogShape::Color outlineColor;
      rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) == 1);
      simCore::GOG::Annotation::OutlineThickness thickness = simCore::GOG::Annotation::OutlineThickness::NONE;
      rv += SDK_ASSERT(anno->getOutlineThickness(thickness) == 1);
    }
  }
  shapes.clear();

  // test full annotation
  std::stringstream annoGog;
  annoGog << "start\n annotation label 1\n centerll 24.5 54.6\n fontname georgia.ttf\n fontsize 24\n linecolor hex 0xa0ffa0ff\n textoutlinethickness thin\n textoutlinecolor blue\n end\n";
  parser.parse(annoGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Annotation* anno = dynamic_cast<simCore::GOG::Annotation*>(shapes.front().get());
    rv += SDK_ASSERT(anno != nullptr);
    if (anno)
    {
      rv += SDK_ASSERT(comparePositions(anno->position(), simCore::Vec3(24.5 * simCore::DEG2RAD, 54.6 * simCore::DEG2RAD, 0.)));
      rv += SDK_ASSERT(anno->text() == "label 1");
      std::string fontName;
      rv += SDK_ASSERT(anno->getFontName(fontName) == 0);
      rv += SDK_ASSERT(fontName.find("georgia.ttf") != std::string::npos);
      int textSize = 0;
      rv += SDK_ASSERT(anno->getTextSize(textSize) == 0);
      rv += SDK_ASSERT(textSize == 24);
      simCore::GOG::GogShape::Color textColor;
      rv += SDK_ASSERT(anno->getTextColor(textColor) == 0);
      rv += SDK_ASSERT(textColor == simCore::GOG::GogShape::Color(255, 160, 255, 160));
      simCore::GOG::GogShape::Color outlineColor;
      rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) == 0);
      rv += SDK_ASSERT(outlineColor == simCore::GOG::GogShape::Color(0, 0, 255, 255));
      simCore::GOG::Annotation::OutlineThickness thickness = simCore::GOG::Annotation::OutlineThickness::NONE;
      rv += SDK_ASSERT(anno->getOutlineThickness(thickness) == 0);
      rv += SDK_ASSERT(thickness == simCore::GOG::Annotation::OutlineThickness::THIN);
    }
  }
  shapes.clear();

  // test nested annotations
  std::stringstream annoNestedGog;
  annoNestedGog << "start\n annotation label 0\n centerll 24.5 54.6\n fontname georgia.ttf\n fontsize 24\n linecolor hex 0xa0ffa0ff\n textoutlinethickness thin\n textoutlinecolor blue\n"
    << "annotation label 1\n centerll 24.7 54.3\n annotation label 2\n centerll 23.4 55.4\n end\n";
  parser.parse(annoNestedGog, shapes);
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
        rv += SDK_ASSERT(comparePositions(anno->position(), positions[textId]));
        std::ostringstream os;
        os << "label " << textId++;
        rv += SDK_ASSERT(anno->text() == os.str());

        std::string fontName;
        rv += SDK_ASSERT(anno->getFontName(fontName) == 0);
        rv += SDK_ASSERT(fontName.find("georgia.ttf") != std::string::npos);
        int textSize = 0;
        rv += SDK_ASSERT(anno->getTextSize(textSize) == 0);
        rv += SDK_ASSERT(textSize == 24);
        simCore::GOG::GogShape::Color textColor;
        rv += SDK_ASSERT(anno->getTextColor(textColor) == 0);
        rv += SDK_ASSERT(textColor == simCore::GOG::GogShape::Color(255, 160, 255, 160));
        simCore::GOG::GogShape::Color outlineColor;
        rv += SDK_ASSERT(anno->getOutlineColor(outlineColor) == 0);
        rv += SDK_ASSERT(outlineColor == simCore::GOG::GogShape::Color(0, 0, 255, 255));
        simCore::GOG::Annotation::OutlineThickness thickness = simCore::GOG::Annotation::OutlineThickness::NONE;
        rv += SDK_ASSERT(anno->getOutlineThickness(thickness) == 0);
        rv += SDK_ASSERT(thickness == simCore::GOG::Annotation::OutlineThickness::THIN);
      }
    }
  }
  shapes.clear();

  return rv;
}

int testUnits()
{
  int rv = 0;
  simCore::GOG::Parser parser;
  std::vector<simCore::GOG::GogShapePtr> shapes;

  // test circle range units default to yards and altitude units default to feet
  std::stringstream circleGog;
  circleGog << "start\n circle\n centerlla 25.1 58.2 12.\n radius 100\n end\n";
  parser.parse(circleGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Circle* circle = dynamic_cast<simCore::GOG::Circle*>(shapes.front().get());
    rv += SDK_ASSERT(circle != nullptr);
    if (circle)
    {
      const simCore::Vec3& center = circle->centerPosition();
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
  parser.parse(circleDefinedGog, shapes);
  rv += SDK_ASSERT(shapes.size() == 1);
  if (!shapes.empty())
  {
    simCore::GOG::Circle* circle = dynamic_cast<simCore::GOG::Circle*>(shapes.front().get());
    rv += SDK_ASSERT(circle != nullptr);
    if (circle)
    {
      const simCore::Vec3& center = circle->centerPosition();
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
  parser.parse(lineGog, shapes);
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
  parser.parse(lineDefinedGog, shapes);
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

  return rv;
}

