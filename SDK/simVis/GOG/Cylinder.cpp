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
#include "osgEarthAnnotation/AnnotationUtils"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "osgEarthSymbology/GeometryFactory"
#include "osgEarthFeatures/GeometryCompiler"
#include "simCore/Common/Common.h"
#include "simNotify/Notify.h"
#include "simVis/GOG/Cylinder.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

namespace simVis { namespace GOG {

GogNodeInterface* Cylinder::deserialize(const ParsedShape& parsedShape,
                      simVis::GOG::ParserData& p,
                      const GOGNodeType&       nodeType,
                      const GOGContext&        context,
                      const GogMetaData&       metaData,
                      MapNode*                 mapNode)
{
  Distance radius(parsedShape.doubleValue("radius", 1000.), p.units_.rangeUnits_);
  Angle    rotation(parsedShape.doubleValue("rotation", 0.0), p.units_.angleUnits_);
  Distance height(parsedShape.doubleValue("height", 1000.), p.units_.altitudeUnits_);
  Angle    start(parsedShape.doubleValue("anglestart", 0.0), p.units_.angleUnits_);
  Angle    end = start;
  if (parsedShape.hasValue("angledeg"))
    end = start + Angle(parsedShape.doubleValue("angledeg", 90.0), Units::DEGREES);
  else if (parsedShape.hasValue("angleend"))
    end = Angle(parsedShape.doubleValue("angleend", 0.0), p.units_.angleUnits_);

  osgEarth::Symbology::GeometryFactory gf;
  osg::ref_ptr<Geometry> tgeom = start == end ? (Geometry*)new Ring() : (Geometry*)new LineString();
  osg::ref_ptr<Geometry> shape;

  if (parsedShape.hasValue("majoraxis"))
  {
    radius = Distance(0.5 * parsedShape.doubleValue("majoraxis", 2000.0), p.units_.rangeUnits_);
    if (parsedShape.hasValue("minoraxis"))
    {
      Distance minorRadius = Distance(0.5 * parsedShape.doubleValue("minoraxis", 2000.0), p.units_.rangeUnits_);
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
    if (!parsedShape.hasValue("filled"))
      style.getOrCreateSymbol<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = false;

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
    if (!parsedShape.hasValue("filled"))
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
    if (!parsedShape.hasValue("filled"))
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
