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
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

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
  osg::ref_ptr<osg::Node> shape = simVis::createHemisphere(
    radius_m, color);
  shape->setName("GOG Hemisphere");

  osgEarth::LocalGeometryNode* node = nullptr;
  osgEarth::Style style;
  if (!attached)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape.get());
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape.get(), style);
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
