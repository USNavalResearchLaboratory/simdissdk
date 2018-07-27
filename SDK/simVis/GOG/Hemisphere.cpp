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
#include "simVis/GOG/Utils.h"

using namespace simVis::GOG;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;

GogNodeInterface* Hemisphere::deserialize(const osgEarth::Config&  conf,
                        simVis::GOG::ParserData& p,
                        const GOGNodeType&       nodeType,
                        const GOGContext&        context,
                        const GogMetaData&       metaData,
                        MapNode*                 mapNode)
{
  Distance radius(conf.value("radius", 1000.0), p.units_.rangeUnits_);

  osg::Vec4f color(Color::White);

  float radius_m = radius.as(Units::METERS);

  osg::Node* shape = osgEarth::Annotation::AnnotationUtils::createHemisphere(
    radius_m, color);

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::Annotation::LocalGeometryNode();
    node->setMapNode(mapNode);
    node->setPosition(p.getMapPosition());
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    osg::Quat yaw(p.localHeadingOffset_->as(Units::RADIANS), -osg::Vec3(0, 0, 1));
    osg::Quat pitch(p.localPitchOffset_->as(Units::RADIANS), osg::Vec3(1, 0, 0));
    osg::Quat roll(p.localRollOffset_->as(Units::RADIANS), osg::Vec3(0, 1, 0));
    node->setLocalRotation(roll * pitch * yaw);
  }
  else
  {
    node = new HostedLocalGeometryNode(shape, p.style_);
  }

  GogNodeInterface* rv = NULL;
  if (node)
  {
    node->setLocalOffset(p.getLTPOffset());
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyConfigToStyle(conf, p.units_);
  }
  return rv;
}
