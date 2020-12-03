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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
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
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

#undef LC
#define LC "[GOG::Polygon] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* simVis::GOG::Polygon::deserialize(const ParsedShape& parsedShape,
                                  simVis::GOG::ParserData& p,
                                  const GOGNodeType&       nodeType,
                                  const GOGContext&        context,
                                  const GogMetaData&       metaData,
                                  osgEarth::MapNode*       mapNode)
{
  p.parseGeometry<osgEarth::Polygon>(parsedShape);
  GogNodeInterface* rv = nullptr;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    // force non-zero crease angle for extruded tessellated polygon, we want to only draw posts at actual vertices
    if (p.style_.has<osgEarth::LineSymbol>() &&
      p.style_.getSymbol<osgEarth::LineSymbol>()->tessellation() > 0 &&
      p.style_.has<osgEarth::ExtrusionSymbol>() &&
      !p.style_.getSymbol<osgEarth::LineSymbol>()->creaseAngle().isSet())
    {
      p.style_.getSymbol<osgEarth::LineSymbol>()->creaseAngle() = 1.0f;
    }

    if (p.hasAbsoluteGeometry())
    {
      Feature* feature = new Feature(p.geom_.get(), p.srs_.get(), p.style_);
      feature->setName("GOG Polygon Feature");
      if (p.geoInterp_.isSet())
        feature->geoInterp() = p.geoInterp_.value();
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      rv = new FeatureNodeInterface(featureNode, metaData);
      featureNode->setName("GOG Polygon");
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(p.geom_.get(), p.style_);
      node->setMapNode(mapNode);
      Utils::applyLocalGeometryOffsets(*node, p, nodeType);
      rv = new LocalGeometryNodeInterface(node, metaData);
      node->setName("GOG Polygon");
    }
  }
  else // if ( nodeType == GOGNODE_HOSTED )
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(p.geom_.get(), p.style_);
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new LocalGeometryNodeInterface(node, metaData);
    node->setName("GOG Polygon");
  }

  if (rv)
    rv->applyToStyle(parsedShape, p.units_);

  return rv;
}

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
      feature->setName("GOG Polygon Feature");
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
