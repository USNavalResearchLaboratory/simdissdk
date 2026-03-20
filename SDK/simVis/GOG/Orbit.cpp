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
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Math.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/ErrorHandler.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
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
