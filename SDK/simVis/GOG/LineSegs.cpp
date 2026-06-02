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
#include "osgEarth/FeatureNode"
#include "osgEarth/LocalGeometryNode"
#include "simCore/Calc/Angle.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"

#undef LC
#define LC "[GOG::LineSegs] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* LineSegs::createLineSegs(const simCore::GOG::LineSegs& lineSegs, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  MultiGeometry* multiGeom = new MultiGeometry();
  const std::vector<simCore::Vec3>& points = lineSegs.points();
  // add the line seg points in pairs
  for (auto i = points.begin(); i != points.end(); /* two places in the loop*/)
  {
    osg::Vec3d point1;
    if (lineSegs.isRelative())
      point1 = osg::Vec3d(i->x(), i->y(), i->z());
    else
      point1 = osg::Vec3d(i->lon() * simCore::RAD2DEG, i->lat() * simCore::RAD2DEG, i->alt());
    ++i; //<< increment
    if (i != points.end())
    {
      osg::Vec3d point2;
      if (lineSegs.isRelative())
        point2 = osg::Vec3d(i->x(), i->y(), i->z());
      else
        point2 = osg::Vec3d(i->lon() * simCore::RAD2DEG, i->lat() * simCore::RAD2DEG, i->alt());
      // Avoid adding the same point twice
      if (point1 != point2)
      {
        Geometry* seg = new LineString(2);
        seg->push_back(point1);
        seg->push_back(point2);
        multiGeom->add(seg);
      }
      ++i; //<< increment
    }
  }

  osgEarth::Style style;
  GogMetaData metaData;
  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(lineSegs))
      Utils::configureStyleForClipping(style);

    if (!lineSegs.isRelative())
    {
      std::string vdatum;
      lineSegs.getVerticalDatum(vdatum);
      osgEarth::SpatialReference* srs = LoaderUtils::getSrs(vdatum);
      Feature* feature = new Feature(multiGeom, srs, style);
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      featureNode->setName("GOG LineSegs");
      return new FeatureNodeInterface(featureNode, metaData);
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(multiGeom, style);
      node->setMapNode(mapNode);
      // pass in 0 xyz center offsets since the geometry's points define xyz offsets
      LoaderUtils::setShapePositionOffsets(*node, lineSegs, simCore::Vec3(), refPoint, attached, false);
      node->setName("GOG LineSegs");
      return new LocalGeometryNodeInterface(node, metaData);
    }
  }
  LocalGeometryNode* node = new HostedLocalGeometryNode(multiGeom, style);
  // pass in 0 xyz center offsets since the geometry's points define xyz offsets
  LoaderUtils::setShapePositionOffsets(*node, lineSegs, simCore::Vec3(), refPoint, attached, false);
  node->setName("GOG LineSegs");
  return new LocalGeometryNodeInterface(node, metaData);
}

} }
