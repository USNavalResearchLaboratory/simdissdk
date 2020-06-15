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
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/GeometryCompiler"
#include "osgEarth/GeometryFactory"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace osgEarth;

namespace simVis { namespace GOG {

GogNodeInterface* Ellipse::deserialize(const ParsedShape& parsedShape,
                     simVis::GOG::ParserData& p,
                     const GOGNodeType&       nodeType,
                     const GOGContext&        context,
                     const GogMetaData&       metaData,
                     MapNode*                 mapNode)
{
  Distance majorRadius, minorRadius;

  if (parsedShape.hasValue(GOG_MAJORAXIS))
    majorRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS,
      0.5 * parsedShape.doubleValue(GOG_MAJORAXIS, 10.0)), Units::METERS);

  if (parsedShape.hasValue(GOG_MINORAXIS))
    minorRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS,
      0.5 * parsedShape.doubleValue(GOG_MINORAXIS, 5.0)), Units::METERS);

  if (parsedShape.hasValue(GOG_RADIUS))
  {
    majorRadius = Distance(p.units_.rangeUnits_.convertTo(simCore::Units::METERS,
      parsedShape.doubleValue(GOG_RADIUS, 10.0)), Units::METERS);
    minorRadius = majorRadius;
  }

  const Angle rotation(0., Units::DEGREES); // Rotation handled by parameters in GOG_ORIENT
  osgEarth::GeometryFactory gf;
  Geometry* shape = gf.createEllipse(osg::Vec3d(0, 0, 0), minorRadius, majorRadius, rotation);

  osgEarth::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    node = new osgEarth::LocalGeometryNode(shape, p.style_);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);
  node->setName("GOG Ellipse Position");

  Utils::applyLocalGeometryOffsets(*node, p, nodeType);

  GogNodeInterface* rv = new LocalGeometryNodeInterface(node, metaData);
  rv->applyToStyle(parsedShape, p.units_);

  return rv;
}

} }