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
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
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
#if OSGEARTH_SOVERSION < 181
    style.getOrCreate<ExtrusionSymbol>()->height() = heightM;
#else
    style.getOrCreate<ExtrusionSymbol>()->height() = osgEarth::Distance(heightM, osgEarth::Units::METERS);
#endif

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
    topCapNode->getPositionAttitudeTransform()->setPivotPoint(osg::Vec3(0, 0, -heightM));

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
