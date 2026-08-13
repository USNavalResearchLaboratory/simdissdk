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
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "simCore/Common/Common.h"
#include "simVis/GOG/Circle.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"
#include "simNotify/Notify.h"

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Circle::createCircle(const simCore::GOG::Circle& circle, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  const double radiusMeters = circle.getRadius().value_or(simCore::Units().convertTo(simCore::Units::METERS, 1000.));
  Distance radius(radiusMeters, Units::METERS);

  osgEarth::GeometryFactory gf;
  Geometry* shape = gf.createCircle(osg::Vec3d(0, 0, 0), radius);

  osgEarth::LocalGeometryNode* node = nullptr;
  osgEarth::Style style;

  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(circle))
      Utils::configureStyleForClipping(style);

    node = new osgEarth::LocalGeometryNode(shape, style);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, style);
  node->setName("GOG Circle Position");

  // use the ref point as the center if no center defined by the shape
  std::optional<simCore::Vec3> center = circle.getCenterPosition();
  if (!center.has_value() && !attached)
    center = refPoint;
  else if (!center.has_value())
    center = simCore::Vec3();

  LoaderUtils::setShapePositionOffsets(*node, circle, *center, refPoint, attached, false);
  GogMetaData metaData;
  return new LocalGeometryNodeInterface(node, metaData);
}

} }
