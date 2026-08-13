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
#include "osgEarth/GeometryCompiler"
#include "osgEarth/GeometryFactory"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Ellipse::createEllipse(const simCore::GOG::Ellipse& ellipse, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  Distance majorRadius, minorRadius;

  double majorAxis = 0.;
  if (ellipse.getMajorAxis(majorAxis) == 0)
    majorRadius = Distance(0.5 * majorAxis, Units::METERS);

  double minorAxis = 0.;
  if (ellipse.getMinorAxis(minorAxis) == 0)
    minorRadius = Distance(0.5 * minorAxis, Units::METERS);

  const std::optional<double> radius = ellipse.getRadius();
  if (radius.has_value())
  {
    majorRadius = Distance(radius.value(), Units::METERS);
    minorRadius = majorRadius;
  }

  const Angle rotation(0., Units::DEGREES); // Rotation handled in setShapePositionOffsets()
  osgEarth::GeometryFactory gf;
  Geometry* shape = gf.createEllipse(osg::Vec3d(0, 0, 0), minorRadius, majorRadius, rotation);

  osgEarth::LocalGeometryNode* node = nullptr;

  osgEarth::Style style;
  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(ellipse))
      Utils::configureStyleForClipping(style);

    node = new osgEarth::LocalGeometryNode(shape, style);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, style);
  node->setName("GOG Ellipse Position");

  // use the ref point as the center if no center defined by the shape
  std::optional<simCore::Vec3> center = ellipse.getCenterPosition();
  if (!center.has_value() && !attached)
    center = refPoint;
  else if (!center.has_value())
    center = simCore::Vec3();

  LoaderUtils::setShapePositionOffsets(*node, ellipse, *center, refPoint, attached, false);
  GogMetaData metaData;
  return new LocalGeometryNodeInterface(node, metaData);
}

} }
