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
#include "osgEarthAnnotation/LocalGeometryNode"
#include "osgEarthAnnotation/AnnotationUtils"
#include "osg/CullFace"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/Sphere.h"
#include "simVis/GOG/Utils.h"

using namespace simVis::GOG;


GogNodeInterface* Sphere::deserialize(const osgEarth::Config&  conf,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(conf.value("radius", 1000.0), p.units_.rangeUnits_);

  osg::Vec4f color(osgEarth::Symbology::Color::White);

  float radius_m = radius.as(osgEarth::Units::METERS);

  osg::Node* shape = osgEarth::Annotation::AnnotationUtils::createSphere(
    radius_m, color);

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::Annotation::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
  {
    node = new HostedLocalGeometryNode(shape, p.style_);
  }

  GogNodeInterface* rv = NULL;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyConfigToStyle(conf, p.units_);
  }
  return rv;
}
