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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "osgEarth/AnnotationUtils"
#include "osgEarth/LocalGeometryNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogShape.h"
#include "simNotify/Notify.h"
#include "simVis/GOG/Hemisphere.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

GogNodeInterface* Hemisphere::deserialize(const ParsedShape& parsedShape,
                        simVis::GOG::ParserData& p,
                        const GOGNodeType&       nodeType,
                        const GOGContext&        context,
                        const GogMetaData&       metaData,
                        osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.0)), osgEarth::Units::METERS);

  osg::Vec4f color(osgEarth::Color::White);

  float radius_m = radius.as(osgEarth::Units::METERS);

  // cannot create a hemisphere with no radius
  if (radius_m <= 0.f)
  {
    SIM_WARN << "Cannot create hemisphere with no radius\n";
    return nullptr;
  }
  osg::Node* shape = osgEarth::AnnotationUtils::createHemisphere(
    radius_m, color);
  shape->setName("GOG Hemisphere");

  osgEarth::LocalGeometryNode* node = nullptr;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::LocalGeometryNode();
    node->setMapNode(mapNode);
    node->setPosition(p.getMapPosition());
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    osg::Quat yaw(p.localHeadingOffset_->as(osgEarth::Units::RADIANS), -osg::Vec3(0, 0, 1));
    osg::Quat pitch(p.localPitchOffset_->as(osgEarth::Units::RADIANS), osg::Vec3(1, 0, 0));
    osg::Quat roll(p.localRollOffset_->as(osgEarth::Units::RADIANS), osg::Vec3(0, 1, 0));
    node->setLocalRotation(roll * pitch * yaw);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);
  node->setName("GOG Hemisphere Position");

  GogNodeInterface* rv = nullptr;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }
  return rv;
}

GogNodeInterface* Hemisphere::createHemisphere(const simCore::GOG::Hemisphere& hemi, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radiusM = 0.;
  hemi.getRadius(radiusM);
  osgEarth::Distance radius(radiusM, osgEarth::Units::METERS);

  osg::Vec4f color(osgEarth::Color::White);

  float radius_m = radius.as(osgEarth::Units::METERS);

  // cannot create a hemisphere with no radius
  if (radius_m <= 0.f)
  {
    SIM_WARN << "Cannot create hemisphere with no radius\n";
    return nullptr;
  }
  osg::Node* shape = osgEarth::AnnotationUtils::createHemisphere(
    radius_m, color);
  shape->setName("GOG Hemisphere");

  osgEarth::LocalGeometryNode* node = nullptr;
  osgEarth::Style style;
  if (!attached)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, style);
  node->setName("GOG Hemisphere Position");

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (hemi.getCenterPosition(center) != 0 && !attached)
    center = refPoint;
  LoaderUtils::setShapePositionOffsets(*node, hemi, center, refPoint, attached, false);
  GogMetaData metaData;
  return new SphericalNodeInterface(node, metaData);
}

} }
