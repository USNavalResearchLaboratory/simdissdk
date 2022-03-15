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
#include "osgEarth/LocalGeometryNode"
#include "simCore/Calc/MathConstants.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Cone.h"

namespace simVis { namespace GOG {

GogNodeInterface* Cone::deserialize(const ParsedShape& parsedShape,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.)), osgEarth::Units::METERS);
  osgEarth::Distance height(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_HEIGHT, 1000.)), osgEarth::Units::METERS);

  // Set up geometry
  osg::ref_ptr<osg::Geometry> coneGeom = new osg::Geometry;
  coneGeom->setName("simVis::GOG::Cone Geometry");
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> coneVerts = new osg::Vec3Array();
  coneGeom->setVertexArray(coneVerts.get());
  // Create and bind color array
  osg::ref_ptr<osg::Vec4Array> coneColors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  coneGeom->setColorArray(coneColors.get());
  coneColors->push_back(osg::Vec4f(osgEarth::Color::White));

  osg::ref_ptr<osg::Geometry> capGeom = new osg::Geometry;
  capGeom->setName("simVis::GOG::Cone Cap Geometry");
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> capVerts = new osg::Vec3Array();
  capGeom->setVertexArray(capVerts.get());
  // Bind color array
  capGeom->setColorArray(coneColors.get());

  // Number of points in cone's cap
  const int CAP_RESOLUTION = 32;
  const double radiusM = radius.as(osgEarth::Units::METERS);
  const double heightM = height.as(osgEarth::Units::METERS);
  const osg::Vec3 tip(0, 0, 0);
  const osg::Vec3 capCenter(0, 0, heightM);
  for (int i = 0; i < CAP_RESOLUTION; i++)
  {
    // Converts the CAP_RESOLUTION to points on a circle, in range [0, 2PI)
    const double angle = i * M_TWOPI / CAP_RESOLUTION;
    const double sine = radiusM * sin(angle);
    const double cosine = radiusM * cos(angle);
    // Cone vertices and cap vertices need to be wound opposite from each other
    // to ensure the cone faces draw outward and the cap faces draw upward
    coneVerts->push_back(osg::Vec3(cosine, sine, heightM));
    coneVerts->push_back(tip);
    capVerts->push_back(osg::Vec3(sine, cosine, heightM));
    capVerts->push_back(capCenter);
  }

  // Repeat the first vertex to close the shape
  coneVerts->push_back(*(coneVerts->begin()));
  capVerts->push_back(*(capVerts->begin()));
  coneGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, coneVerts->size()));
  capGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, capVerts->size()));

  osgEarth::LocalGeometryNode* node = nullptr;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(coneGeom.get());
    node->getPositionAttitudeTransform()->addChild(capGeom.get());
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
  {
    node = new HostedLocalGeometryNode(coneGeom.get(), p.style_);
    node->getPositionAttitudeTransform()->addChild(capGeom.get());
  }

  node->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
  node->setName("simVis::GOG::Cone");

  Utils::applyLocalGeometryOffsets(*node, p, nodeType);
  GogNodeInterface* rv = new ConeNodeInterface(node, metaData);
  rv->applyToStyle(parsedShape, p.units_);
  rv->setFilledState(true); // always filled
  return rv;
}

GogNodeInterface* Cone::createCone(const simCore::GOG::Cone& cone, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radiusM = 0.;
  cone.getRadius(radiusM);
  double heightM = 0.;
  cone.getHeight(heightM);

  // Set up geometry
  osg::ref_ptr<osg::Geometry> coneGeom = new osg::Geometry;
  coneGeom->setName("simVis::GOG::Cone Geometry");
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> coneVerts = new osg::Vec3Array();
  coneGeom->setVertexArray(coneVerts.get());
  // Create and bind color array
  osg::ref_ptr<osg::Vec4Array> coneColors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  coneGeom->setColorArray(coneColors.get());
  coneColors->push_back(osg::Vec4f(osgEarth::Color::White));

  osg::ref_ptr<osg::Geometry> capGeom = new osg::Geometry;
  capGeom->setName("simVis::GOG::Cone Cap Geometry");
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> capVerts = new osg::Vec3Array();
  capGeom->setVertexArray(capVerts.get());
  // Bind color array
  capGeom->setColorArray(coneColors.get());

  // Number of points in cone's cap
  const int CAP_RESOLUTION = 32;
  const osg::Vec3 tip(0, 0, 0);
  const osg::Vec3 capCenter(0, 0, heightM);
  for (int i = 0; i < CAP_RESOLUTION; i++)
  {
    // Converts the CAP_RESOLUTION to points on a circle, in range [0, 2PI)
    const double angle = i * M_TWOPI / CAP_RESOLUTION;
    const double sine = radiusM * sin(angle);
    const double cosine = radiusM * cos(angle);
    // Cone vertices and cap vertices need to be wound opposite from each other
    // to ensure the cone faces draw outward and the cap faces draw upward
    coneVerts->push_back(osg::Vec3(cosine, sine, heightM));
    coneVerts->push_back(tip);
    capVerts->push_back(osg::Vec3(sine, cosine, heightM));
    capVerts->push_back(capCenter);
  }

  // Repeat the first vertex to close the shape
  coneVerts->push_back(*(coneVerts->begin()));
  capVerts->push_back(*(capVerts->begin()));
  coneGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, coneVerts->size()));
  capGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, capVerts->size()));

  osgEarth::LocalGeometryNode* node = nullptr;
  if (!attached)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(coneGeom.get());
    node->getPositionAttitudeTransform()->addChild(capGeom.get());
    node->setMapNode(mapNode);
  }
  else
  {
    osgEarth::Style style;
    node = new HostedLocalGeometryNode(coneGeom.get(), style);
    node->getPositionAttitudeTransform()->addChild(capGeom.get());
  }

  node->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
  node->setName("simVis::GOG::Cone");

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (cone.getCenterPosition(center) != 0 && !attached)
    center = refPoint;

  LoaderUtils::setShapePositionOffsets(*node, cone, center, refPoint, attached, false);
  GogMetaData metaData;
  GogNodeInterface* rv = new ConeNodeInterface(node, metaData);
  rv->setFilledState(true); // always filled
  return rv;
}

} }
