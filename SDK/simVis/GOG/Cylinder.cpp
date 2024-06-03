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
#include "osg/FrontFace"
#include "osgEarth/MapNode"
#include "osgEarth/AnnotationUtils"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Common/Common.h"
#include "simCore/GOG/GogShape.h"
#include "simNotify/Notify.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace {

/** Returns the result of simCore::angFix2PI() on angle: [0,M_TWOPI) */
Angle angFix2PI(Angle angle)
{
  return Angle(simCore::angFix2PI(angle.as(Units::RADIANS)), Units::RADIANS);
}

}

namespace simVis { namespace GOG {

GogNodeInterface* Cylinder::deserialize(const ParsedShape& parsedShape,
                      simVis::GOG::ParserData& p,
                      const GOGNodeType&       nodeType,
                      const GOGContext&        context,
                      const GogMetaData&       metaData,
                      MapNode*                 mapNode)
{
  Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.)), Units::METERS);
  Angle    rotation(0., Units::DEGREES); // Rotation handled by parameters in GOG_ORIENT
  Distance height(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_HEIGHT, 1000.)), Units::METERS);
  Angle    start(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLESTART, 0.)), Units::DEGREES);
  // angFix() the start between 0,360.  osgEarth takes the direct path between two angles
  // when drawing the arc.  Two angles (start+end) between [0,360) means no crossing 0
  start = angFix2PI(start);

  Angle    end = start;
  const size_t lineNumber = parsedShape.lineNumber();
  if (parsedShape.hasValue(GOG_ANGLEDEG))
  {
    Angle sweep(parsedShape.doubleValue(GOG_ANGLEDEG, 90.0), Units::DEGREES);
    const double sweepRadians = sweep.as(Units::RADIANS);

    // If the sweep is 0, then clear out the radius to draw nothing.  Else an angledeg
    // of 0 will end up drawing a circle incorrectly (note sweep of 360 is fine).
    // Because of this, we use areEqual, NOT areAnglesEqual().
    if (simCore::areEqual(sweepRadians, 0.0))
    {
      radius = Distance(0, Units::METERS);
      context.errorHandler_->printError(lineNumber, "Cylinder AngleDeg cannot be 0");
    }
    else if (sweepRadians > M_TWOPI || sweepRadians < -M_TWOPI)
    {
      context.errorHandler_->printWarning(lineNumber, "Cylinder AngleDeg larger than 360 detected");
    }

    // Use fmod to keep the correct sign for correct sweep angle
    end = start + Angle(::fmod(sweep.as(Units::RADIANS), M_TWOPI), Units::RADIANS);
  }
  else if (parsedShape.hasValue(GOG_ANGLEEND))
  {
    end = Angle(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLEEND, 0.0)), Units::DEGREES);
    // angFix2PI() forces end between [0,360).  Since start is in the same range, we'll
    // never cross 0 with the osgEarth drawing algorithm.
    end = angFix2PI(end);

    // If the end and start are the same value, return nullptr to draw nothing.  Cannot
    // use the angleend command to draw circles (use angledeg instead)
    if (simCore::areAnglesEqual(start.as(Units::RADIANS), end.as(Units::RADIANS)))
    {
      context.errorHandler_->printError(lineNumber, "Cylinder AngleEnd cannot be same value as AngleStart");
      return nullptr;
    }
  }

  osgEarth::GeometryFactory gf;
  osg::ref_ptr<Geometry> tgeom = simCore::areAnglesEqual(start.as(Units::RADIANS), end.as(Units::RADIANS)) ? dynamic_cast<Geometry*>(new Ring()) : dynamic_cast<Geometry*>(new LineString());
  osg::ref_ptr<Geometry> shape;

  if (parsedShape.hasValue(GOG_MAJORAXIS))
  {
    radius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, 0.5 * parsedShape.doubleValue(GOG_MAJORAXIS, 2000.0)), Units::METERS);
    if (parsedShape.hasValue(GOG_MINORAXIS))
    {
      Distance minorRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, 0.5 * parsedShape.doubleValue(GOG_MINORAXIS, 2000.0)), Units::METERS);
      shape = gf.createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, 0u, tgeom.get(), true);
    }
    else
      shape = gf.createArc(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, 0u, tgeom.get(), true);
  }
  else
    shape = gf.createArc(osg::Vec3d(0, 0, 0), radius, start + rotation, end + rotation, 0u, tgeom.get(), true);

  osg::ref_ptr<osg::Group> g = new osg::Group();
  float heightValue = height.as(Units::METERS);

  // first the extruded side shape:
  LocalGeometryNode* sideNode = nullptr;
  {
    Style style(p.style_);
    style.getOrCreate<ExtrusionSymbol>()->height() = heightValue;

    // remove a line symbol so we don't stripe the sides
    style.remove<LineSymbol>();

    // Need to turn backface culling off for unfilled cylinders so the sides are visible
    if (!parsedShape.hasValue(GOG_FILLED))
      style.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = false;

    if (nodeType == GOGNODE_GEOGRAPHIC)
    {
      sideNode = new LocalGeometryNode(shape.get(), style);
      sideNode->setMapNode(mapNode);
    }
    else
    {
      sideNode = new HostedLocalGeometryNode(shape.get(), style);
    }
    sideNode->setName("Cylinder Side");
    Utils::applyLocalGeometryOffsets(*sideNode, p, nodeType);
    g->addChild(sideNode);
  }

  // next the top cap:
  LocalGeometryNode* topCapNode = nullptr;
  {
    Style style(p.style_);

    // remove the extrusion symbol
    style.remove<ExtrusionSymbol>();
    if (!parsedShape.hasValue(GOG_FILLED))
      style.remove<PolygonSymbol>();

    if (nodeType == GOGNODE_GEOGRAPHIC)
    {
      topCapNode = new LocalGeometryNode(shape.get(), style);
      topCapNode->setMapNode(mapNode);
    }
    else
    {
      topCapNode = new HostedLocalGeometryNode(shape.get(), style);
    }
    topCapNode->setName("Cylinder Top");

    // apply a local offset to get the cap node to the correct height
    osg::Vec3d localOffset = p.getLTPOffset();
    localOffset[2] += heightValue;
    Utils::applyLocalGeometryOffsets(*topCapNode, p, nodeType);
    // offset the top cap node's altitude by the height
    if (nodeType == GOGNODE_GEOGRAPHIC)
    {
      osgEarth::GeoPoint pos = topCapNode->getPosition();
      pos.alt() += heightValue;
      topCapNode->setPosition(pos);
    }
    else
    {
      osg::Vec3d pos = topCapNode->getPositionAttitudeTransform()->getPosition();
      pos.z() += heightValue;
      topCapNode->getPositionAttitudeTransform()->setPosition(pos);
    }

    g->addChild(topCapNode);
  }

  // next the bottom cap:
  LocalGeometryNode* bottomCapNode = nullptr;
  {
    Style style(p.style_);

    // remove the extrusion symbol
    style.remove<ExtrusionSymbol>();
    if (!parsedShape.hasValue(GOG_FILLED))
      style.remove<PolygonSymbol>();

    if (nodeType == GOGNODE_GEOGRAPHIC)
    {
      bottomCapNode = new LocalGeometryNode(shape.get(), style);
      bottomCapNode->setMapNode(mapNode);
    }
    else
    {
      bottomCapNode = new HostedLocalGeometryNode(shape.get(), style);
    }
    bottomCapNode->setName("Cylinder Bottom");
    Utils::applyLocalGeometryOffsets(*bottomCapNode, p, nodeType);

    // Set the frontface on bottom to clockwise, since we cannot easily rewind vertices
    bottomCapNode->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);
    g->addChild(bottomCapNode);
  }

  CylinderNodeInterface* rv = nullptr;
  if (sideNode && topCapNode && bottomCapNode)
  {
    rv = new CylinderNodeInterface(g.get(), sideNode, topCapNode, bottomCapNode, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }
  return rv;
}

GogNodeInterface* Cylinder::createCylinder(const simCore::GOG::Cylinder& cyl, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radiusM = 0.;
  cyl.getRadius(radiusM);
  Distance radius(radiusM, Units::METERS);
  Angle rotation(0., Units::DEGREES); // Rotation handled in setShapePositionOffsets()
  double heightM = 0.;
  cyl.getHeight(heightM);
  double angleStart = 0;
  cyl.getAngleStart(angleStart);
  Angle start(angleStart * simCore::RAD2DEG, Units::DEGREES);

  // default to a full circle if no angle sweep specified
  Angle end(360., Units::DEGREES);
  double sweepRad = 0.;
  if (cyl.getAngleSweep(sweepRad) == 0)
  {
    if (simCore::areEqual(sweepRad, 0.))
      radius = Distance(0, Units::METERS);
    Angle sweep = Angle(sweepRad * simCore::RAD2DEG, Units::DEGREES);
    // Use fmod to keep the correct sign for correct sweep angle
    end = start + Angle(::fmod(sweep.as(Units::RADIANS), M_TWOPI), Units::RADIANS);
  }
  osgEarth::GeometryFactory gf;
  osg::ref_ptr<Geometry> tgeom = simCore::areAnglesEqual(start.as(Units::RADIANS), end.as(Units::RADIANS)) ? dynamic_cast<Geometry*>(new Ring()) : dynamic_cast<Geometry*>(new LineString());
  osg::ref_ptr<Geometry> shape;

  double majorAxis = 0.;
  bool elliptical = false;
  if (cyl.getMajorAxis(majorAxis) == 0)
  {
    radius = Distance(0.5 * majorAxis, Units::METERS);
    double minorAxis = 0.;
    if (cyl.getMinorAxis(minorAxis) == 0)
    {
      elliptical = true;
      Distance minorRadius = Distance(0.5 * minorAxis, Units::METERS);
      shape = gf.createEllipticalArc(osg::Vec3d(0, 0, 0), radius, minorRadius, rotation, start, end, 0u, tgeom.get(), true);
    }
    else
      shape = gf.createArc(osg::Vec3d(0, 0, 0), radius, start, end, 0u, tgeom.get(), true);
  }
  else
    shape = gf.createArc(osg::Vec3d(0, 0, 0), radius, start, end, 0u, tgeom.get(), true);

  osg::ref_ptr<osg::Group> g = new osg::Group();
  bool filled = false;
  cyl.getIsFilled(filled);

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (cyl.getCenterPosition(center) != 0 && !attached)
    center = refPoint;

  // first the extruded side shape:
  LocalGeometryNode* sideNode = nullptr;
  {
    Style style;
    style.getOrCreate<ExtrusionSymbol>()->height() = heightM;

    // remove a line symbol so we don't stripe the sides
    style.remove<LineSymbol>();

    // Need to turn backface culling off for unfilled cylinders so the sides are visible
    if (!filled)
      style.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = false;

    if (!attached)
    {
      sideNode = new LocalGeometryNode(shape.get(), style);
      sideNode->setMapNode(mapNode);
    }
    else
      sideNode = new HostedLocalGeometryNode(shape.get(), style);

    // Set the node facing to clockwise, to solve winding issue with osgEarth for elliptical arcs
    if (elliptical)
      sideNode->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);

    sideNode->setName("Cylinder Side");
    LoaderUtils::setShapePositionOffsets(*sideNode, cyl, center, refPoint, attached, false);
    g->addChild(sideNode);
  }

  // next the top cap:
  LocalGeometryNode* topCapNode = nullptr;
  {
    Style style;

    // remove the extrusion symbol
    style.remove<ExtrusionSymbol>();
    if (!filled)
      style.remove<PolygonSymbol>();

    if (!attached)
    {
      topCapNode = new LocalGeometryNode(shape.get(), style);
      topCapNode->setMapNode(mapNode);
    }
    else
      topCapNode = new HostedLocalGeometryNode(shape.get(), style);
    topCapNode->setName("Cylinder Top");

    LoaderUtils::setShapePositionOffsets(*topCapNode, cyl, center, refPoint, attached, false);
    // offset the top cap node's altitude by the height
    if (!attached)
    {
      osgEarth::GeoPoint pos = topCapNode->getPosition();
      pos.alt() += heightM;
      topCapNode->setPosition(pos);
    }
    else
    {
      osg::Vec3d pos = topCapNode->getPositionAttitudeTransform()->getPosition();
      pos.z() += heightM;
      topCapNode->getPositionAttitudeTransform()->setPosition(pos);
    }

    g->addChild(topCapNode);
  }

  // next the bottom cap:
  LocalGeometryNode* bottomCapNode = nullptr;
  {
    Style style;

    // remove the extrusion symbol
    style.remove<ExtrusionSymbol>();
    if (!filled)
      style.remove<PolygonSymbol>();

    if (!attached)
    {
      bottomCapNode = new LocalGeometryNode(shape.get(), style);
      bottomCapNode->setMapNode(mapNode);
    }
    else
      bottomCapNode = new HostedLocalGeometryNode(shape.get(), style);
    bottomCapNode->setName("Cylinder Bottom");
    LoaderUtils::setShapePositionOffsets(*bottomCapNode, cyl, center, refPoint, attached, false);

    // Set the frontface on bottom to clockwise, since we cannot easily rewind vertices
    bottomCapNode->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);
    g->addChild(bottomCapNode);
  }

  CylinderNodeInterface* rv = nullptr;
  GogMetaData metaData;
  if (sideNode && topCapNode && bottomCapNode)
    rv = new CylinderNodeInterface(g.get(), sideNode, topCapNode, bottomCapNode, metaData);
  return rv;
}

}}
