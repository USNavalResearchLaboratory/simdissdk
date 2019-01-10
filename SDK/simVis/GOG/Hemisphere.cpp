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
#include "osgEarthAnnotation/AnnotationUtils"
#include "osgEarthAnnotation/LocalGeometryNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simNotify/Notify.h"
#include "simVis/GOG/Hemisphere.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

GogNodeInterface* Hemisphere::deserialize(const ParsedShape& parsedShape,
                        simVis::GOG::ParserData& p,
                        const GOGNodeType&       nodeType,
                        const GOGContext&        context,
                        const GogMetaData&       metaData,
                        osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(parsedShape.doubleValue(GOG_RADIUS, 1000.0), p.units_.rangeUnits_);

  osg::Vec4f color(osgEarth::Symbology::Color::White);

  float radius_m = radius.as(osgEarth::Units::METERS);

  osg::Node* shape = osgEarth::Annotation::AnnotationUtils::createHemisphere(
    radius_m, color);
  shape->setName("GOG Hemisphere");

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::Annotation::LocalGeometryNode();
    node->setMapNode(mapNode);
    node->setPosition(p.getMapPosition());
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    osg::Quat yaw(p.localHeadingOffset_->as(osgEarth::Units::RADIANS), -osg::Vec3(0, 0, 1));
    osg::Quat pitch(p.localPitchOffset_->as(osgEarth::Units::RADIANS), osg::Vec3(1, 0, 0));
    osg::Quat roll(p.localRollOffset_->as(osgEarth::Units::RADIANS), osg::Vec3(0, 1, 0));
    node->setLocalRotation(roll * pitch * yaw);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);
  node->setName("GOG Hemisphere Position");

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
