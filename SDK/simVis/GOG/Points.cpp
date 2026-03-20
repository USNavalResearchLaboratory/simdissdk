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
#include "osgEarth/Geometry"
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "osgEarth/FeatureNode"
#include "osgEarth/LocalGeometryNode"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"

#undef LC
#define LC "[GOG::PointSet] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Points::createPoints(const simCore::GOG::Points& points, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  osgEarth::Geometry* geom = new osgEarth::PointSet();
  LoaderUtils::setPoints(points.points(), points.isRelative(), *geom);

  osgEarth::Style style;
  GogMetaData metaData;
  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(points))
      Utils::configureStyleForClipping(style);

    if (!points.isRelative())
    {
      std::string vdatum;
      points.getVerticalDatum(vdatum);
      osgEarth::SpatialReference* srs = LoaderUtils::getSrs(vdatum);
      Feature* feature = new Feature(geom, srs, style);

      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      featureNode->setName("GOG Points");
      return new FeatureNodeInterface(featureNode, metaData);
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(geom, style);
      node->setMapNode(mapNode);
      LoaderUtils::setShapePositionOffsets(*node, points, simCore::Vec3(), refPoint, attached, true);
      node->setName("GOG Points");
      return new LocalGeometryNodeInterface(node, metaData);
    }
  }

  LocalGeometryNode* node = new HostedLocalGeometryNode(geom, style);
  // note no offset to apply for points, since each point inherently defines its own offsets when hosted
  node->setName("GOG Points");
  return new LocalGeometryNodeInterface(node, metaData);
}

} }
