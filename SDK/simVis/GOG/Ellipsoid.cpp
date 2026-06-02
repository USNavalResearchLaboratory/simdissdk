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
#include "osg/CullFace"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/Types.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

GogNodeInterface* Ellipsoid::createEllipsoid(const simCore::GOG::Ellipsoid& ellipsoid, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  // all the ways to set the radii
  double majorAxis = 0.;
  ellipsoid.getMajorAxis(majorAxis);
  double minorAxis = 0.;
  ellipsoid.getMinorAxis(minorAxis);
  double height = 0.;
  bool heightSet = (ellipsoid.getHeight(height) == 0);

  osgEarth::Distance y_diam(minorAxis, osgEarth::Units::METERS);
  osgEarth::Distance x_diam(majorAxis, osgEarth::Units::METERS);
  osgEarth::Distance z_diam(height, osgEarth::Units::METERS);

  double radius = 0.;
  if (ellipsoid.getRadius(radius) == 0)
  {
    x_diam.set(radius * 2, osgEarth::Units::METERS);
    y_diam = x_diam;
    // use radius for height if it wasn't set, or if it was set to 0
    if (!heightSet || z_diam == 0.0)
      z_diam = x_diam;
  }

  osg::Vec4f color(simVis::Color::White);

  float x_radius_m = x_diam.as(osgEarth::Units::METERS) / 2.0;
  float y_radius_m = y_diam.as(osgEarth::Units::METERS) / 2.0;
  float z_radius_m = z_diam.as(osgEarth::Units::METERS) / 2.0;

  osg::ref_ptr<osg::Node> shape = simVis::createEllipsoid(
    y_radius_m, x_radius_m, z_radius_m, color);  // y, x, z order to match SIMDIS 9
  shape->setName("GOG Ellipsoid");

  osgEarth::LocalGeometryNode* node = nullptr;
  osgEarth::Style style;
  if (!attached)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape.get());
    node->setStyle(style);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape.get(), style);
  node->setName("GOG Ellipsoid Position");

  // use the ref point as the center if no center defined by the shape
  simCore::Vec3 center;
  if (ellipsoid.getCenterPosition(center) != 0 && !attached)
    center = refPoint;

  LoaderUtils::setShapePositionOffsets(*node, ellipsoid, center, refPoint, attached, false);
  GogMetaData metaData;
  return new SphericalNodeInterface(node, metaData);
}

} }
