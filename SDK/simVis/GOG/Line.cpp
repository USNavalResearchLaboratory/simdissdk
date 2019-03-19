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
#include "simVis/GOG/Line.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Constants.h"

#define LC "[GOG::Line] "

using namespace simVis::GOG;
using namespace osgEarth::Features;
using namespace osgEarth::Annotation;

GogNodeInterface* Line::deserialize(const ParsedShape& parsedShape,
                  simVis::GOG::ParserData& p,
                  const GOGNodeType&       nodeType,
                  const GOGContext&        context,
                  const GogMetaData&       metaData,
                  osgEarth::MapNode*       mapNode)
{
  p.parseGeometry<LineString>(parsedShape);

  GogNodeInterface* rv = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    // force non-zero crease angle for extruded tesselated line, we want to only draw posts at actual vertices
    if (p.style_.has<osgEarth::Symbology::LineSymbol>() &&
      p.style_.getSymbol<osgEarth::Symbology::LineSymbol>()->tessellation() > 0 &&
      p.style_.has<osgEarth::Symbology::ExtrusionSymbol>() &&
      !p.style_.getSymbol<osgEarth::Symbology::LineSymbol>()->creaseAngle().isSet())
    {
      p.style_.getSymbol<osgEarth::Symbology::LineSymbol>()->creaseAngle() = 1.0f;
    }

    if (p.hasAbsoluteGeometry())
    {
      Feature* feature = new Feature(p.geom_.get(), p.srs_.get(), p.style_);
      feature->setName("GOG Line Feature");
      if (p.geoInterp_.isSet())
      {
        feature->geoInterp() = p.geoInterp_.value();
      }
      FeatureNode* node = new FeatureNode(feature);
      node->setMapNode(mapNode);
      rv = new FeatureNodeInterface(node, metaData);
      node->setName("GOG Line");
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(p.geom_.get(), p.style_);
      node->setMapNode(mapNode);
      Utils::applyLocalGeometryOffsets(*node, p, nodeType);
      rv = new LocalGeometryNodeInterface(node, metaData);
      node->setName("GOG Line");
    }
  }
  else // if ( nodeType == GOGNODE_HOSTED )
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(p.geom_.get(), p.style_);
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new LocalGeometryNodeInterface(node, metaData);
    node->setName("GOG Line");
  }

  if (rv)
    rv->applyToStyle(parsedShape, p.units_);

  return rv;
}
