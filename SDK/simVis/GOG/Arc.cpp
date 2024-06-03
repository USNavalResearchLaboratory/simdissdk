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
#include "osgEarth/MapNode"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/Feature"
#include "osgEarth/GeometryCompiler"
#include "osgEarth/GeometryFactory"
#include "simNotify/Notify.h"
#include "simCore/Common/Common.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/Constants.h"
#include "simVis/GOG/Arc.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace
{

/// create an arc with inner and outer radius
Geometry* createDonut(const osg::Vec3d& center,
                           const Distance&   radius,
                           const Distance&   innerRadius,
                           const Angle&      start,
                           const Angle&      end,
                           Geometry*         geomToUse)
{
  Geometry* geom = (geomToUse ? geomToUse : new LineString());

  double rM = radius.as(Units::METERS);
  double irM = innerRadius.as(Units::METERS);
  // can't draw a donut if outer radius is <= 0 or inner radius is < 0
  if (rM <= 0. || irM < 0.)
    return geom;

  // automatically calculate number of segments
  double segLen = rM / 8.0;
  double circumference = 2 * osg::PI * rM;
  unsigned int numSegments = static_cast<unsigned int>(ceil(circumference / segLen));

  unsigned int numInnerSegments = 1;
  if (irM > 0.)
  {
    double innerSegLen = irM / 8.0;
    double innerCircumference = 2 * osg::PI * irM;
    numInnerSegments = static_cast<unsigned int>(ceil(innerCircumference / innerSegLen));
  }

  double startRad = std::min(start.as(Units::RADIANS), end.as(Units::RADIANS));
  double endRad   = std::max(start.as(Units::RADIANS), end.as(Units::RADIANS));

  if (endRad == startRad)
    endRad += 2 * osg::PI;

  double span      = endRad - startRad;
  double step      = span / static_cast<double>(numSegments);
  double innerStep = span / static_cast<double>(numInnerSegments);

  // calculate outer points
  for (unsigned int i = 0; i <= numSegments; ++i)
  {
    double angle = startRad + step * i;
    double x, y;
    x = center.x() + sin(angle) * rM;
    y = center.y() + cos(angle) * rM;
    geom->push_back(osg::Vec3d(x, y, center.z()));
  }

  // calculate inner points
  for (int i = numInnerSegments; i >= 0; --i)
  {
    double angle = startRad + innerStep * i;
    double x, y;
    x = center.x() + sin(angle) * irM;
    y = center.y() + cos(angle) * irM;
    geom->push_back(osg::Vec3d(x, y, center.z()));
  }

  // add first point again as last, to close the shape
  geom->push_back(geom->front());

  return geom;
}

// create an elliptical arc with inner and outer radius. Note that inner radius is circular,  not elliptical, following SIMDIS 9 convention
Geometry* createEllipticalDonut(const osg::Vec3d& center,
                                     const Distance&   radiusMajor,
                                     const Distance&   radiusMinor,
                                     const Distance&   innerRadius,
                                     const Angle&      rotationAngle,
                                     const Angle&      start,
                                     const Angle&      end,
                                     Geometry*         geomToUse)
{
  Geometry* geom = (geomToUse ? geomToUse : new LineString());

  double irM = innerRadius.as(Units::METERS);

  // can't draw a donut if inner radius is 0
  if (irM <= 0)
    return geom;

  // automatically calculate number of segments
  double ravgM = 0.5*(radiusMajor.as(Units::METERS) + radiusMinor.as(Units::METERS));
  // can't draw a donut if our radius is 0
  if (ravgM <= 0.)
    return geom;
  double segLen = ravgM / 8.0;
  double circumference = 2 * osg::PI * ravgM;
  unsigned int numSegments = static_cast<unsigned>(::ceil(circumference / segLen));

  double innerSegLen = irM / 8.0;
  double innerCircumference = 2 * osg::PI * irM;
  unsigned int numInnerSegments = static_cast<unsigned>(::ceil(innerCircumference / innerSegLen));

  double startRad = std::min(start.as(Units::RADIANS), end.as(Units::RADIANS));// - osg::PI_2;
  double endRad   = std::max(start.as(Units::RADIANS), end.as(Units::RADIANS));// - osg::PI_2;

  if (endRad == startRad)
    endRad += 2*osg::PI;

  double span      = endRad - startRad;
  double step      = span / static_cast<double>(numSegments);
  double innerStep = span / static_cast<double>(numInnerSegments);

  double a = radiusMajor.as(Units::METERS);
  double b = radiusMinor.as(Units::METERS);
  double g = rotationAngle.as(Units::RADIANS);
  double sing = sin(g);
  double cosg = cos(g);

  // calculate outer points
  for (unsigned int i = 0; i <= numSegments; i++)
  {
    double angle = startRad + step * i;
    double cost = cos(angle), sint = sin(angle);
    double x = center.x() + a*sint*cosg + b*cost*sing;
    double y = center.y() + b*cost*cosg - a*sint*sing;
    geom->push_back(osg::Vec3d(x, y, center.z()));
  }

  // calculate inner points
  for (int i = numInnerSegments; i >= 0; i--)
  {
    double angle = startRad + innerStep * i;
    double x, y;
    x = center.x() + sin(angle)*irM;
    y = center.y() + cos(angle)*irM;
    geom->push_back(osg::Vec3d(x, y, center.z()));
  }

  // add first point again as last, to close the shape
  geom->push_back(geom->front());

  return geom;
}

Geometry* createArcShape(const osg::Vec3d& center,
                          const Distance&   radius,
                          const Angle&      start,
                          const Angle&      end,
                          bool              drawDonut,
                          const Distance&   innerRadius,
                          bool              drawPie,
                          Geometry*         geomToUse,
                          osgEarth::GeometryFactory&  gf)
{
  if (drawDonut)
    return createDonut(center, radius, innerRadius, start, end, geomToUse);
  else
    return gf.createArc(center, radius, start, end, 0u, geomToUse, drawPie);
}

Geometry* createEllipticalArc(const osg::Vec3d& center,
                          const Distance&   radiusMajor,
                          const Distance&   radiusMinor,
                          const Angle&      rotationAngle,
                          const Angle&      start,
                          const Angle&      end,
                          bool              drawDonut,
                          const Distance&   innerRadius,
                          bool              drawPie,
                          Geometry*         geomToUse,
                          osgEarth::GeometryFactory&  gf)
{
  if (drawDonut)
    return createEllipticalDonut(center, radiusMajor, radiusMinor, innerRadius, rotationAngle, start, end, geomToUse);
  else
    return gf.createEllipticalArc(center, radiusMajor, radiusMinor, rotationAngle, start, end, 0u, geomToUse, drawPie);
}

/** Returns the result of simCore::angFix2PI() on angle: [0,M_TWOPI) */
Angle angFix2PI_(Angle angle)
{
  return Angle(simCore::angFix2PI(angle.as(Units::RADIANS)), Units::RADIANS);
}

}

namespace simVis { namespace GOG {

GogNodeInterface* Arc::deserialize(const ParsedShape& parsedShape, simVis::GOG::ParserData& p, const GOGNodeType& nodeType, const GOGContext& context, const GogMetaData& metaData, MapNode* mapNode)
{
  bool hasInnerRadius = false;
  Distance iRadius;

  if (parsedShape.hasValue(GOG_INNERRADIUS))
  {
    iRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_INNERRADIUS, 0.)), Units::METERS);
    if (iRadius.as(Units::METERS) > 0)
      hasInnerRadius = true;
  }

  Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.)), Units::METERS);
  Angle    rotation(0., Units::DEGREES); // Rotation handled by parameters in GOG_ORIENT
  Angle    start(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLESTART, 0.0)), Units::DEGREES);
  // angFix() the start between 0,360.  osgEarth takes the direct path between two angles
  // when drawing the arc.  Two angles (start+end) between [0,360) means no crossing 0
  start = angFix2PI_(start);
  Angle    end = start;
  const size_t lineNumber = parsedShape.lineNumber();

  // Check for the angledeg (sweep) version of arc, which can cross 0 degrees
  if (parsedShape.hasValue(GOG_ANGLEDEG))
  {
    Angle sweep = Angle(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLEDEG, 0.0)), Units::DEGREES);

    // Print a warning on invalid spread values (0 is invalid, >360 is warning)
    const double sweepRadians = sweep.as(Units::RADIANS);

    // If the sweep is 0, then clear out the radius to draw nothing.  Else a angledeg
    // of 0 will end up drawing a circle incorrectly (note sweep of 360 is fine).
    // Because of this, we use areEqual, NOT areAnglesEqual().
    if (simCore::areEqual(sweepRadians, 0.0))
    {
      radius = Distance(0, Units::METERS);
      context.errorHandler_->printError(lineNumber, "Arc AngleDeg cannot be 0");
    }
    else if (sweepRadians > M_TWOPI || sweepRadians < -M_TWOPI)
    {
      context.errorHandler_->printWarning(lineNumber, "Arc AngleDeg larger than 360 detected");
    }

    // Use fmod to keep the correct sign for correct sweep angle
    end = start + Angle(::fmod(sweep.as(Units::RADIANS), M_TWOPI), Units::RADIANS);
  }
  else if (parsedShape.hasValue(GOG_ANGLEEND))
  {
    end = Angle(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLEEND, 0.0)), Units::DEGREES);
    // angFix2PI() forces end between [0,360).  Since start is in the same range, we'll
    // never cross 0 with the osgEarth drawing algorithm.
    end = angFix2PI_(end);

    // If the end and start are the same value, return nullptr to draw nothing.  Cannot
    // use the angleend command to draw circles (use angledeg instead)
    if (simCore::areAnglesEqual(start.as(Units::RADIANS), end.as(Units::RADIANS)))
    {
      context.errorHandler_->printError(lineNumber, "Arc AngleEnd cannot be same value as AngleStart");
      return nullptr;
    }
  }

  // whether to include the center point in the geometry.
  bool filled = p.style_.has<PolygonSymbol>();
  osgEarth::GeometryFactory gf;
  osg::ref_ptr<osgEarth::Geometry> outlineShape = new LineString();
  osg::ref_ptr<osgEarth::Geometry> filledShape = new osgEarth::Polygon();

  if (parsedShape.hasValue(GOG_MAJORAXIS))
  {
    radius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, 0.5 * parsedShape.doubleValue(GOG_MAJORAXIS, 2000.0)), Units::METERS);
    if (parsedShape.hasValue(GOG_MINORAXIS))
    {
      Distance minorRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, 0.5 * parsedShape.doubleValue(GOG_MINORAXIS, 2000.0)), Units::METERS);
      outlineShape = createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
      filledShape = createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, hasInnerRadius, iRadius, true, filledShape.get(), gf);
    }
    else
    {
      outlineShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
      filledShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, hasInnerRadius, iRadius, true, filledShape.get(), gf);
    }
  }
  else
  {
    outlineShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
    filledShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, hasInnerRadius, iRadius, true, filledShape.get(), gf);
  }

  osgEarth::LocalGeometryNode* shapeNode = nullptr;
  osgEarth::LocalGeometryNode* fillNode = nullptr;
  osg::Group* g = new osg::Group();

  // remove the polygon symbol for the shape, since it should only exist in the fillNode
  Style shapeStyle(p.style_);
  shapeStyle.remove<PolygonSymbol>();
  // remove the line symbol for the fill node
  Style fillStyle(p.style_);
  fillStyle.remove<LineSymbol>();

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
    {
      Utils::configureStyleForClipping(shapeStyle);
      Utils::configureStyleForClipping(fillStyle);
    }

    shapeNode = new osgEarth::LocalGeometryNode(outlineShape.get(), shapeStyle);
    shapeNode->setMapNode(mapNode);

    fillNode = new osgEarth::LocalGeometryNode(filledShape.get(), fillStyle);
    fillNode->setMapNode(mapNode);
  }
  else
  {
    shapeNode = new HostedLocalGeometryNode(outlineShape.get(), shapeStyle);
    fillNode = new HostedLocalGeometryNode(filledShape.get(), fillStyle);
  }

  shapeNode->setName("Arc Outline Node");
  fillNode->setName("Arc Fill Node");

  Utils::applyLocalGeometryOffsets(*shapeNode, p, nodeType);
  // only bother with fill node if we have the shape

  Utils::applyLocalGeometryOffsets(*fillNode, p, nodeType);
  // show the filled node only if filled
  fillNode->setNodeMask(filled ? simVis::DISPLAY_MASK_GOG : simVis::DISPLAY_MASK_NONE);
  g->addChild(fillNode);

  g->addChild(shapeNode);

  GogNodeInterface* rv = new ArcNodeInterface(g, shapeNode, fillNode, metaData);
  rv->applyToStyle(parsedShape, p.units_);

  return rv;
}

GogNodeInterface* Arc::createArc(const simCore::GOG::Arc& arc, const std::string& filename, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radiusM = 0.;
  arc.getRadius(radiusM);
  Distance radius(radiusM, Units::METERS);
  Angle rotation(0., Units::DEGREES); // Rotation handled in setShapePositionOffsets()
  double angleStart = 0.;
  arc.getAngleStart(angleStart);
  Angle start(angleStart * simCore::RAD2DEG, Units::DEGREES);

  double sweepRad = 0.;
  arc.getAngleSweep(sweepRad);
  if (simCore::areEqual(sweepRad, 0.))
  {
    SIM_ERROR << "GOG: Cannot create Arc with sweep of 0" << (!filename.empty() ? " in " + filename : "") << "\n";
    return nullptr;
  }
  Angle sweep = Angle(sweepRad * simCore::RAD2DEG, Units::DEGREES);
  // Use fmod to keep the correct sign for correct sweep angle
  Angle end = start + Angle(::fmod(sweep.as(Units::RADIANS), M_TWOPI), Units::RADIANS);

  double iRadiusM = 0.;
  bool hasInnerRadius = (arc.getInnerRadius(iRadiusM) == 0);
  Distance iRadius(iRadiusM, Units::METERS);

  // whether to include the center point in the geometry.
  bool filled = false;
  arc.getIsFilled(filled);
  osgEarth::GeometryFactory gf;
  osg::ref_ptr<osgEarth::Geometry> outlineShape = new LineString();
  osg::ref_ptr<osgEarth::Geometry> filledShape = new osgEarth::Polygon();

  double majorAxis = 0.;
  if (arc.getMajorAxis(majorAxis) == 0)
  {
    radius = Distance(0.5 * majorAxis, Units::METERS);
    double minorAxis = 0.;
    if (arc.getMinorAxis(minorAxis) == 0)
    {
      Distance minorRadius = Distance(0.5 * minorAxis, Units::METERS);
      outlineShape = createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
      filledShape = createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, hasInnerRadius, iRadius, true, filledShape.get(), gf);
    }
    else
    {
      outlineShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start, end, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
      filledShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start, end, hasInnerRadius, iRadius, true, filledShape.get(), gf);
    }
  }
  else
  {
    outlineShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start, end, hasInnerRadius, iRadius, false, outlineShape.get(), gf);
    filledShape = createArcShape(osg::Vec3d(0, 0, 0), radius, start, end, hasInnerRadius, iRadius, true, filledShape.get(), gf);
  }

  osgEarth::LocalGeometryNode* shapeNode = nullptr;
  osgEarth::LocalGeometryNode* fillNode = nullptr;
  osg::Group* g = new osg::Group();

  // remove the polygon symbol for the shape, since it should only exist in the fillNode
  Style shapeStyle;
  shapeStyle.remove<PolygonSymbol>();
  // remove the line symbol for the fill node
  Style fillStyle;
  fillStyle.remove<LineSymbol>();

  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(arc))
    {
      Utils::configureStyleForClipping(shapeStyle);
      Utils::configureStyleForClipping(fillStyle);
    }

    shapeNode = new osgEarth::LocalGeometryNode(outlineShape.get(), shapeStyle);
    shapeNode->setMapNode(mapNode);

    fillNode = new osgEarth::LocalGeometryNode(filledShape.get(), fillStyle);
    fillNode->setMapNode(mapNode);
  }
  else
  {
    shapeNode = new HostedLocalGeometryNode(outlineShape.get(), shapeStyle);
    fillNode = new HostedLocalGeometryNode(filledShape.get(), fillStyle);
  }
  shapeNode->setName("Arc Outline Node");
  fillNode->setName("Arc Fill Node");

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (arc.getCenterPosition(center) != 0 && !attached)
    center = refPoint;
  LoaderUtils::setShapePositionOffsets(*shapeNode, arc, center, refPoint, attached, false);
  LoaderUtils::setShapePositionOffsets(*fillNode, arc, center, refPoint, attached, false);
  // show the filled node only if filled
  fillNode->setNodeMask(filled ? simVis::DISPLAY_MASK_GOG : simVis::DISPLAY_MASK_NONE);
  g->addChild(fillNode);
  g->addChild(shapeNode);

  GogMetaData metaData;
  return new ArcNodeInterface(g, shapeNode, fillNode, metaData);
}

} }
