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
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/AnnotationUtils"
#include "osg/CullFace"
#include "simNotify/Notify.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Sphere.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

GogNodeInterface* Sphere::deserialize(const ParsedShape& parsedShape,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.0)), osgEarth::Units::METERS);

  osg::Vec4f color(osgEarth::Color::White);

  float radius_m = radius.as(osgEarth::Units::METERS);

  // cannot create a sphere with no radius
  if (radius_m <= 0.f)
  {
    SIM_WARN << "Cannot create sphere with no radius\n";
    return nullptr;
  }
  osg::Node* shape = osgEarth::AnnotationUtils::createSphere(
    radius_m, color);
  shape->setName("GOG Sphere");

  osgEarth::LocalGeometryNode* node = nullptr;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
  {
    node = new HostedLocalGeometryNode(shape, p.style_);
  }
  node->setName("GOG Sphere Position");

  GogNodeInterface* rv = nullptr;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }
  return rv;
}

GogNodeInterface* Sphere::createSphere(const simCore::GOG::Sphere& sphere, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radiusM;
  sphere.getRadius(radiusM);

  // cannot create a sphere with no radius
  if (radiusM <= 0.)
  {
    SIM_WARN << "Cannot create sphere with no radius\n";
    return nullptr;
  }

  osg::Vec4f color(osgEarth::Color::White);
  osg::Node* shape = osgEarth::AnnotationUtils::createSphere(radiusM, color);
  shape->setName("GOG Sphere");

  osgEarth::LocalGeometryNode* node = nullptr;
  if (!attached)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setMapNode(mapNode);
  }
  else
  {
    osgEarth::Style style;
    node = new HostedLocalGeometryNode(shape, style);
  }
  node->setName("GOG Sphere Position");

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (sphere.getCenterPosition(center) != 0 && !attached)
    center = refPoint;
  LoaderUtils::setShapePositionOffsets(*node, sphere, center, refPoint, attached, false);
  GogMetaData metaData;
  return new SphericalNodeInterface(node, metaData);
}

} }
