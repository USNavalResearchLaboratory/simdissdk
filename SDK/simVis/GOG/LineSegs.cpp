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
#include "osgEarthSymbology/GeometryFactory"
#include "osgEarthFeatures/GeometryCompiler"
#include "osgEarthAnnotation/FeatureNode"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "simVis/GOG/LineSegs.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/Utils.h"

#define LC "[GOG::LineSegs] "

using namespace simVis::GOG;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GogNodeInterface* LineSegs::deserialize(const osgEarth::Config&  conf,
                      simVis::GOG::ParserData& p,
                      const GOGNodeType&       nodeType,
                      const GOGContext&        context,
                      const GogMetaData&       metaData,
                      osgEarth::MapNode*       mapNode)
{
  osg::ref_ptr<Geometry> temp = new Geometry();
  p.parseLineSegmentPoints(conf, p.units_, temp.get(), p.geomIsLLA_);

  MultiGeometry* m = new MultiGeometry();

  for (unsigned int i = 0; i < temp->size();)
  {
    Geometry* seg = new LineString(2);
    seg->push_back((*temp)[i++]);
    if (i < temp->size())
      seg->push_back((*temp)[i++]);
    m->add(seg);
  }
  p.geom_ = m;

  GogNodeInterface* rv = NULL;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    if (p.hasAbsoluteGeometry())
    {
      Feature* feature = new Feature(p.geom_.get(), p.srs_.get(), p.style_);
      feature->setName("GOG LineSegs Feature");
      FeatureNode* featureNode = new FeatureNode(feature);
      featureNode->setMapNode(mapNode);
      rv = new FeatureNodeInterface(featureNode, metaData);
      featureNode->setName("GOG LineSegs");
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(p.geom_.get(), p.style_);
      node->setMapNode(mapNode);
      Utils::applyLocalGeometryOffsets(*node, p, nodeType);
      rv = new LocalGeometryNodeInterface(node, metaData);
      node->setName("GOG LineSegs");
    }
  }
  else
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(p.geom_.get(), p.style_);
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new LocalGeometryNodeInterface(node, metaData);
    node->setName("GOG LineSegs");
  }
  if (rv)
    rv->applyConfigToStyle(conf, p.units_);
  return rv;
}
