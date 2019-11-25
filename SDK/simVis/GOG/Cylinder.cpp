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
#include "osg/FrontFace"
#include "osgEarth/MapNode"
#include "osgEarth/AnnotationUtils"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/Common/Common.h"
#include "simNotify/Notify.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace {

/** Returns the result of simCore::angFix2PI() on angle: [0,M_TWOPI) */
const Angle& angFix2PI(Angle angle)
{
  return Angle(simCore::angFix2PI(angle.as(Units::RADIANS)), Units::RADIANS);
}

/** Returns the result of fmod() on angle: (-denom,+denom) */
const Angle& fmod(Angle angle, double denom = M_TWOPI)
{
  return Angle(::fmod(angle.as(Units::RADIANS), denom), Units::RADIANS);
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
    end = start + fmod(sweep);
  }
  else if (parsedShape.hasValue(GOG_ANGLEEND))
  {
    end = Angle(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, parsedShape.doubleValue(GOG_ANGLEEND, 0.0)), Units::DEGREES);
    // angFix2PI() forces end between [0,360).  Since start is in the same range, we'll
    // never cross 0 with the osgEarth drawing algorithm.
    end = angFix2PI(end);
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
  LocalGeometryNode* sideNode = NULL;
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
  LocalGeometryNode* topCapNode = NULL;
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
  LocalGeometryNode* bottomCapNode = NULL;
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

  CylinderNodeInterface* rv = NULL;
  if (sideNode && topCapNode && bottomCapNode)
  {
    rv = new CylinderNodeInterface(g.get(), sideNode, topCapNode, bottomCapNode, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }
  return rv;
}

}}
