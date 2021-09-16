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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
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
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

#undef LC
#define LC "[GOG::PointSet] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Points::deserialize(const ParsedShape& parsedShape,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  p.parseGeometry<osgEarth::PointSet>(parsedShape);

  // Extruded points are not supported in osgEarth; replace with line segments
  const bool isExtruded = parsedShape.boolValue(GOG_EXTRUDE, false);
  if (isExtruded)
  {
    // Note that an extrudeHeight of 0 means to extrude to ground
    const Distance extrudeHeight(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_EXTRUDE_HEIGHT, 0.0)), Units::METERS);
    recreateAsLineSegs_(p, extrudeHeight.as(Units::METERS));

    // Impersonate LINESEGS instead of points
    GogMetaData newMetaData(metaData);
    newMetaData.shape = simVis::GOG::GOG_LINESEGS;

    return deserializeImpl_(parsedShape, p, nodeType, context, newMetaData, mapNode);
  }

  return deserializeImpl_(parsedShape, p, nodeType, context, metaData, mapNode);
}

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
      feature->setName("GOG Points Feature");

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

void Points::recreateAsLineSegs_(simVis::GOG::ParserData& p, double extrudeHeight) const
{
  MultiGeometry* m = new MultiGeometry();

  for (size_t i = 0; i < p.geom_->size(); ++i)
  {
    osg::Vec3d pt = (*p.geom_)[i];
    Geometry* seg = new LineString(2);
    seg->push_back(pt);

    // If extrude height is 0, extrude to ground instead of to a relative altitude
    if (extrudeHeight == 0)
      pt.z() = 0.0;
    else
      pt.z() += extrudeHeight;

    seg->push_back(pt);
    m->add(seg);
  }
  p.geom_ = m;
}

GogNodeInterface* Points::deserializeImpl_(const ParsedShape& parsedShape,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    MapNode*                 mapNode)
{
  GogNodeInterface* rv = nullptr;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    if (p.hasAbsoluteGeometry())
    {
      Feature* feature = new Feature(p.geom_.get(), p.srs_.get(), p.style_);
      feature->setName("GOG Points Feature");
      if (p.geoInterp_.isSet())
        feature->geoInterp() = p.geoInterp_.value();
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      rv = new FeatureNodeInterface(featureNode, metaData);
      featureNode->setName("GOG Points");
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(p.geom_.get(), p.style_);
      node->setMapNode(mapNode);
      Utils::applyLocalGeometryOffsets(*node, p, nodeType, p.geom_->size() == 1);
      rv = new LocalGeometryNodeInterface(node, metaData);
      node->setName("GOG Points");
    }
  }
  else // if ( nodeType == GOGNODE_HOSTED )
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(p.geom_.get(), p.style_);
    // note no offset to apply for points, since each point inherently defines its own offsets when hosted
    rv = new LocalGeometryNodeInterface(node, metaData);
    node->setName("GOG Points");
  }

  if (rv)
    rv->applyToStyle(parsedShape, p.units_);

  return rv;
}

} }
