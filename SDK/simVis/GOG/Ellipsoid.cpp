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
#include "osg/CullFace"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/Types.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

GogNodeInterface* Ellipsoid::deserialize(const ParsedShape& parsedShape,
                       simVis::GOG::ParserData& p,
                       const GOGNodeType&       nodeType,
                       const GOGContext&        context,
                       const GogMetaData&       metaData,
                       osgEarth::MapNode*       mapNode)
{
  // all the ways to set the radii
  osgEarth::Distance y_diam(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_MINORAXIS, 1000.0)), osgEarth::Units::METERS);
  osgEarth::Distance x_diam(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_MAJORAXIS, 1000.0)), osgEarth::Units::METERS);
  osgEarth::Distance z_diam(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_HEIGHT, 0.0)), osgEarth::Units::METERS);

  if (parsedShape.hasValue(GOG_RADIUS))
  {
    x_diam = parsedShape.doubleValue(GOG_RADIUS, 0) * 2;
    y_diam = x_diam;
    if (z_diam == 0.0)
      z_diam = x_diam;
  }

  osg::Vec4f color(simVis::Color::White);

  float x_radius_m = x_diam.as(osgEarth::Units::METERS) / 2.0;
  float y_radius_m = y_diam.as(osgEarth::Units::METERS) / 2.0;
  float z_radius_m = z_diam.as(osgEarth::Units::METERS) / 2.0;

  osg::Node* shape = osgEarth::AnnotationUtils::createEllipsoid(
    y_radius_m, x_radius_m, z_radius_m, color);  // y, x, z order to match SIMDIS 9
  shape->setName("GOG Ellipsoid");

  osgEarth::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);
  node->setName("GOG Ellipsoid Position");

  GogNodeInterface* rv = NULL;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyToStyle(parsedShape, p.units_);
  }
  return rv;
}

} }