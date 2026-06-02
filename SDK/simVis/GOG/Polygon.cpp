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
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/Polygon.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"

#undef LC
#define LC "[GOG::Polygon] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Polygon::createPolygon(const simCore::GOG::Polygon& polygon, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  osgEarth::Geometry* geom = new osgEarth::Polygon();
  LoaderUtils::setPoints(polygon.points(), polygon.isRelative(), *geom);

  osgEarth::Style style;
  GogMetaData metaData;
  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(polygon))
      Utils::configureStyleForClipping(style);

    if (!polygon.isRelative())
    {
      std::string vdatum;
      polygon.getVerticalDatum(vdatum);
      osgEarth::SpatialReference* srs = LoaderUtils::getSrs(vdatum);
      Feature* feature = new Feature(geom, srs, style);
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      featureNode->setName("GOG Polygon");
      return new FeatureNodeInterface(featureNode, metaData);
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(geom, style);
      node->setMapNode(mapNode);
      node->setName("GOG Polygon");
      LoaderUtils::setShapePositionOffsets(*node, polygon, simCore::Vec3(), refPoint, attached, false);
      return new LocalGeometryNodeInterface(node, metaData);
    }
  }

  LocalGeometryNode* node = new HostedLocalGeometryNode(geom, style);
  LoaderUtils::setShapePositionOffsets(*node, polygon, simCore::Vec3(), refPoint, attached, false);
  node->setName("GOG Polygon");
  return new LocalGeometryNodeInterface(node, metaData);
}

} }
