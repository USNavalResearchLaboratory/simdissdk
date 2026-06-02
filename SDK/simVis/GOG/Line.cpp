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
#include "simVis/GOG/Line.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/Utils.h"
#include "simVis/Constants.h"

#undef LC
#define LC "[GOG::Line] "

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Line::createLine(const simCore::GOG::Line& line, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  osgEarth::LineString* geom = new osgEarth::LineString();
  LoaderUtils::setPoints(line.points(), line.isRelative(), *geom);

  GogNodeInterface* rv = nullptr;
  GogMetaData metaData;
  osgEarth::Style style;

  if (!attached)
  {
    // Try to prevent terrain z-fighting.
    if (LoaderUtils::geometryRequiresClipping(line))
      Utils::configureStyleForClipping(style);

    if (!line.isRelative())
    {
      std::string vdatum;
      line.getVerticalDatum(vdatum);
      osg::ref_ptr<osgEarth::SpatialReference> srs = LoaderUtils::getSrs(vdatum);
      Feature* feature = new Feature(geom, srs.get(), style);
      FeatureNode* node = new FeatureNode(feature);
      node->setMapNode(mapNode);
      rv = new FeatureNodeInterface(node, metaData);
      node->setName("GOG Line");
    }
    else
    {
      LocalGeometryNode* node = new LocalGeometryNode(geom, style);
      node->setMapNode(mapNode);
      // pass in 0 xyz center offsets since the geometry's points define xyz offsets
      LoaderUtils::setShapePositionOffsets(*node, line, simCore::Vec3(), refPoint, attached, false);
      rv = new LocalGeometryNodeInterface(node, metaData);
      node->setName("GOG Line");
    }
  }
  else
  {
    LocalGeometryNode* node = new HostedLocalGeometryNode(geom, style);
    // pass in 0 xyz center offsets since the geometry's points define xyz offsets
    LoaderUtils::setShapePositionOffsets(*node, line, simCore::Vec3(), refPoint, attached, false);
    rv = new LocalGeometryNodeInterface(node, metaData);
    node->setName("GOG Line");
  }

  return rv;
}

} }
