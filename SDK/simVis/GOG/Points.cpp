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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarthSymbology/Geometry"
#include "osgEarthSymbology/GeometryFactory"
#include "osgEarthFeatures/GeometryCompiler"
#include "osgEarthAnnotation/FeatureNode"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "simVis/GOG/Points.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/Utils.h"


#define LC "[GOG::PointSet] "

using namespace simVis::GOG;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GogNodeInterface* Points::deserialize(const osgEarth::Config&  conf,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  p.parseGeometry<osgEarth::Symbology::PointSet>(conf);

  // Extruded points are not supported in osgEarth; replace with line segments
  const bool isExtruded = conf.value("extrude", false);
  if (isExtruded)
  {
    // Note that an extrudeHeight of 0 means to extrude to ground
    const Distance extrudeHeight(conf.value<double>("extrudeheight", 0.0), p.units_.altitudeUnits_);
    recreateAsLineSegs_(p, extrudeHeight.as(Units::METERS));

    // Impersonate LINESEGS instead of points
    GogMetaData newMetaData(metaData);
    newMetaData.shape = simVis::GOG::GOG_LINESEGS;

    return deserializeImpl_(conf, p, nodeType, context, newMetaData, mapNode);
  }

  return deserializeImpl_(conf, p, nodeType, context, metaData, mapNode);
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

GogNodeInterface* Points::deserializeImpl_(const osgEarth::Config&  conf,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    MapNode*                 mapNode)
{
  GogNodeInterface* rv = NULL;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    if (p.hasAbsoluteGeometry())
    {
      Feature* feature = new Feature(p.geom_.get(), p.srs_.get(), p.style_);
      if (p.geoInterp_.isSet())
        feature->geoInterp() = p.geoInterp_.value();
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      rv = new FeatureNodeInterface(featureNode, metaData);
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(p.geom_.get(), p.style_);
      node->setMapNode(mapNode);
      Utils::applyLocalGeometryOffsets(*node, p, nodeType, p.geom_->size() == 1);
      rv = new LocalGeometryNodeInterface(node, metaData);
    }
  }
  else // if ( nodeType == GOGNODE_HOSTED )
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(p.geom_.get(), p.style_);
    // note no offset to apply for points, since each point inherently defines its own offsets when hosted
    rv = new LocalGeometryNodeInterface(node, metaData);
  }

  if (rv)
    rv->applyConfigToStyle(conf, p.units_);

  return rv;
}
