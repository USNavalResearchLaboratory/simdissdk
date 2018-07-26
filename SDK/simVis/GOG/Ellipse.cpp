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
#include "osgEarthFeatures/GeometryCompiler"
#include "osgEarthSymbology/GeometryFactory"
#include "simVis/GOG/Ellipse.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/Utils.h"


using namespace simVis::GOG;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;

GogNodeInterface* Ellipse::deserialize(const osgEarth::Config&  conf,
                     simVis::GOG::ParserData& p,
                     const GOGNodeType&       nodeType,
                     const GOGContext&        context,
                     const GogMetaData&       metaData,
                     MapNode*                 mapNode)
{
  Distance majorRadius, minorRadius;

  if (conf.hasValue("majoraxis"))
    majorRadius = Distance(0.5*conf.value<double>("majoraxis", 10.0), p.units_.rangeUnits_);

  if (conf.hasValue("minoraxis"))
    minorRadius = Distance(0.5*conf.value<double>("minoraxis", 5.0), p.units_.rangeUnits_);

  if (conf.hasValue("radius"))
  {
    majorRadius = Distance(conf.value<double>("radius", 10.0), p.units_.rangeUnits_);
    minorRadius = majorRadius;
  }

  Angle rotation(conf.value<double>("rotation", 0.0), p.units_.angleUnits_);

  GeometryFactory gf;
  Geometry* shape = gf.createEllipse(osg::Vec3d(0, 0, 0), minorRadius, majorRadius, rotation);

  osgEarth::Annotation::LocalGeometryNode* node = NULL;

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    // Try to prevent terrain z-fighting.
    if (p.geometryRequiresClipping())
      Utils::configureStyleForClipping(p.style_);

    node = new osgEarth::Annotation::LocalGeometryNode(shape, p.style_);
    node->setMapNode(mapNode);
    Utils::applyLocalGeometryOffsets(*node, p);
  }
  else
  {
    node = new HostedLocalGeometryNode(shape, p.style_);
    node->setLocalOffset(p.getLTPOffset());
  }

  GogNodeInterface* rv = NULL;
  if (node)
  {
    rv = new LocalGeometryNodeInterface(node, metaData);
    rv->applyConfigToStyle(conf, p.units_);
  }
  return rv;
}
