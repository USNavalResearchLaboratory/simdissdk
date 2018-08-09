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
#include "osgEarthAnnotation/LocalGeometryNode"
#include "osgEarthAnnotation/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/GOG/Ellipsoid.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/Utils.h"

using namespace simVis::GOG;
using namespace osgEarth::Symbology;

GogNodeInterface* Ellipsoid::deserialize(const osgEarth::Config&  conf,
                       simVis::GOG::ParserData& p,
                       const GOGNodeType&       nodeType,
                       const GOGContext&        context,
                       const GogMetaData&       metaData,
                       MapNode*                 mapNode)
{
  // all the ways to set the radii
  Distance y_diam(conf.value("minoraxis", 1000.0), p.units_.rangeUnits_);
  Distance x_diam(conf.value("majoraxis", 1000.0), p.units_.rangeUnits_);
  Distance z_diam(conf.value("height", 0.0), p.units_.altitudeUnits_);

  if (conf.hasValue("radius"))
  {
    x_diam = conf.value<double>("radius", 0) * 2;
    y_diam = conf.value<double>("radius", 0) * 2;
    if (z_diam == 0.0)
      z_diam = conf.value<double>("radius", 0) * 2;
  }

  if (conf.hasValue("diameter"))
  {
    x_diam = conf.value<double>("diameter", 0);
    y_diam = conf.value<double>("diameter", 0);
    if (z_diam == 0.0)
      z_diam = conf.value<double>("diameter", 0);
  }

  if (conf.hasValue("semiminoraxis"))
  {
    y_diam = conf.value<double>("semiminoraxis", 0) * 2;
  }

  if (conf.hasValue("semimajoraxis"))
  {
    x_diam = conf.value<double>("semimajoraxis", 0) * 2;
  }

  osg::Vec4f color(Color::White);

  float x_radius_m = x_diam.as(Units::METERS) / 2.0;
  float y_radius_m = y_diam.as(Units::METERS) / 2.0;
  float z_radius_m = z_diam.as(Units::METERS) / 2.0;

  osg::Node* shape = osgEarth::Annotation::AnnotationUtils::createEllipsoid(
    y_radius_m, x_radius_m, z_radius_m, color);  // y, x, z order to match SIMDIS 9

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::Annotation::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(shape);
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(shape, p.style_);

  GogNodeInterface* rv = NULL;
  if (node)
  {
    Utils::applyLocalGeometryOffsets(*node, p, nodeType);
    rv = new SphericalNodeInterface(node, metaData);
    rv->applyConfigToStyle(conf, p.units_);
  }
  return rv;
}
