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
using namespace osgEarth::Symbology;


GogNodeInterface* Sphere::deserialize(const osgEarth::Config&  conf,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    MapNode*                 mapNode)
{
  Distance radius(conf.value("radius", 1000.0), p.units_.rangeUnits_);

  osg::Vec4f color(Color::White);

  float radius_m = radius.as(Units::METERS);

  osg::Node* shape = osgEarth::Annotation::AnnotationUtils::createSphere(
    radius_m, color);

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::Annotation::LocalGeometryNode(mapNode, shape, p.style_);
    node->setPosition(p.getMapPosition());
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
