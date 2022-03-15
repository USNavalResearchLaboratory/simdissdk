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
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/GOG/Orbit.h"


namespace {

// generate an orbit geometry from specified parameters, azimuth in radians, others in meters
osgEarth::Geometry* createOrbitGeometry(double azimuthRad, double lengthM, double radiusM, double altitudeM)
{
  if (radiusM <= 0)
    return nullptr;
  std::vector<simCore::Vec3> xyzVec;
  simCore::GOG::Orbit::createOrbitShape(azimuthRad, lengthM, radiusM, altitudeM, radiusM / 8., xyzVec);
  osgEarth::Geometry* geom = new osgEarth::LineString();
  for (const auto& xyz : xyzVec)
    geom->push_back(osg::Vec3d(xyz.x(), xyz.y(), xyz.z()));
  geom->rewind(osgEarth::Geometry::ORIENTATION_CCW);
  return geom;
}

}

namespace simVis { namespace GOG {

GogNodeInterface* Orbit::deserialize(
  const ParsedShape& parsedShape,
  simVis::GOG::ParserData& p,
  const GOGNodeType& nodeType,
  const GOGContext& context,
  const GogMetaData& metaData,
  osgEarth::MapNode* mapNode)
{
  double radius = p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.));
  const size_t lineNumber = parsedShape.lineNumber();

  if (radius <= 0)
  {
    context.errorHandler_->printError(lineNumber, "Orbit must have a valid radius");
    return nullptr;
  }

  osgEarth::LocalGeometryNode* node = nullptr;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    if (!parsedShape.hasValue(GOG_CENTERLL) || !parsedShape.hasValue(GOG_CENTERLL2))
    {
      context.errorHandler_->printError(lineNumber, "Orbit must have both center points, [centerll,centerlla,centerlatlon] and centerll2");
      return nullptr;
    }

    // if center points are not set, the hasValue() checks above were implemented incorrectly
    assert(p.centerLLA_.isSet());
    assert(p.centerLLA2_.isSet());

    const osg::Vec3d& ctr1 = p.centerLLA_.get();
    const osg::Vec3d& ctr2 = p.centerLLA2_.get();

    // find azimuth and length of orbit
    double azimuth = 0.;
    double length = simCore::sodanoInverse(ctr1.y() * simCore::DEG2RAD, ctr1.x() * simCore::DEG2RAD, ctr1.z(),
      ctr2.y() * simCore::DEG2RAD, ctr2.x() * simCore::DEG2RAD, &azimuth, nullptr);
    osgEarth::Geometry* geom = createOrbitGeometry(azimuth, length, radius, ctr1.z());

    osgEarth::Style style(p.style_);
    node = new osgEarth::LocalGeometryNode(geom, style);
    node->setMapNode(mapNode);

  }
  else
  {
    if (!parsedShape.hasValue(GOG_CENTERXY) || !parsedShape.hasValue(GOG_CENTERXY2))
    {
      context.errorHandler_->printError(lineNumber, "Orbit relative must have both center points, [centerxy,centerxyz] and centerxy2");
      return nullptr;
    }
    // if center points are not set, the hasValue() checks above were implemented incorrectly
    assert(p.centerXYZ_.isSet());
    assert(p.centerXYZ2_.isSet());

    const osg::Vec3d ctr1 = p.centerXYZ_.get();
    const osg::Vec3d ctr2 = p.centerXYZ2_.get();

    double xLen = ctr1.x() - ctr2.x();
    double yLen = ctr1.y() - ctr2.y();
    double length = sqrt((xLen * xLen) + (yLen * yLen));
    double azimuth = atan(xLen / yLen);

    if (yLen > 0)
      azimuth += M_PI;

    osgEarth::Geometry* geom = createOrbitGeometry(simCore::angFix2PI(azimuth), length, radius, ctr1.z());
    osgEarth::Style style(p.style_);
    node = new HostedLocalGeometryNode(geom, style);
  }

  GogNodeInterface* rv = nullptr;
  if (node)
  {
    node->setName("Orbit");
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new LocalGeometryNodeInterface(node, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }

  return rv;
 }

GogNodeInterface* Orbit::createOrbit(const simCore::GOG::Orbit& orbit, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  double radius = 0.;
  orbit.getRadius(radius);

  simCore::Vec3 center1;
  orbit.getCenterPosition(center1);
  simCore::Vec3 center2 = orbit.centerPosition2();

  osgEarth::LocalGeometryNode* node = nullptr;
  osgEarth::Style style;
  if (!orbit.isRelative())
  {
    // find azimuth and length of orbit
    double azimuth = 0.;
    double length = simCore::sodanoInverse(center1.x(), center1.y(), center1.z(),
      center2.x(), center2.y(), &azimuth, nullptr);
    osgEarth::Geometry* geom = createOrbitGeometry(azimuth, length, radius, 0.); // Pass in 0 altitude, relative to host center1

    node = new osgEarth::LocalGeometryNode(geom, style);
    node->setMapNode(mapNode);
  }
  else
  {
    double xLen = center1.x() - center2.x();
    double yLen = center1.y() - center2.y();

    double length = 0.;
    if (xLen != 0. || yLen != 0.)
      length = sqrt((xLen * xLen) + (yLen * yLen));

    double azimuth = M_PI_2;
    if (yLen != 0.)
      azimuth = atan(xLen / yLen);
    else if (xLen > 0.)
      azimuth = M_PI_2 * 3;

    if (yLen > 0.)
      azimuth += M_PI;
    osgEarth::Geometry* geom = createOrbitGeometry(simCore::angFix2PI(azimuth), length, radius, 0.); // Pass in 0 altitude, relative to host center1
    if (attached)
      node = new HostedLocalGeometryNode(geom, style);
    else
    {
      node = new osgEarth::LocalGeometryNode(geom, style);
      node->setMapNode(mapNode);
    }
  }

  node->setName("Orbit");
  LoaderUtils::setShapePositionOffsets(*node, orbit, center1, refPoint, attached, false);
  GogMetaData metaData;
  return new LocalGeometryNodeInterface(node, metaData);
}

}}
