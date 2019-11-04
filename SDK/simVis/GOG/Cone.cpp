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
#include "osgEarth/LocalGeometryNode"
#include "simCore/Calc/MathConstants.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/HostedLocalGeometryNode.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Cone.h"

namespace simVis { namespace GOG {

GogNodeInterface* Cone::deserialize(const ParsedShape& parsedShape,
                    simVis::GOG::ParserData& p,
                    const GOGNodeType&       nodeType,
                    const GOGContext&        context,
                    const GogMetaData&       metaData,
                    osgEarth::MapNode*       mapNode)
{
  osgEarth::Distance radius(p.units_.rangeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_RADIUS, 1000.)), osgEarth::Units::METERS);
  osgEarth::Distance height(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_HEIGHT, 1000.)), osgEarth::Units::METERS);

  // Set up geometry
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
  geom->setName("simVis::GOG::Cone Geometry");
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  geom->setVertexArray(verts.get());
  // Create and bind color array
  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
  geom->setColorArray(colors.get());
  colors->push_back(osg::Vec4f(osgEarth::Color::White));

  /** Number of points in cone's cap */
  const int CAP_RESOLUTION = 32;
  const double radiusM = radius.as(osgEarth::Units::METERS);
  const double heightM = height.as(osgEarth::Units::METERS);
  const osg::Vec3 tip(0, 0, 0);
  for (int i = 0; i < CAP_RESOLUTION; i++)
  {
    /** Converts the CAP_RESOLUTION to points on a circle, in range [0, 2PI) */
    const double angle = i * M_TWOPI / CAP_RESOLUTION;
    const double sine = radiusM * sin(angle);
    const double cosine = radiusM * cos(angle);
    verts->push_back(osg::Vec3(sine, cosine, heightM));
    verts->push_back(tip);
  }

  // Repeat the first vertex to close the shape
  verts->push_back(*(verts->begin()));
  geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, verts->size()));

  osgEarth::LocalGeometryNode* node = NULL;
  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    node = new osgEarth::LocalGeometryNode();
    node->getPositionAttitudeTransform()->addChild(geom.get());
    node->setStyle(p.style_);
    node->setMapNode(mapNode);
  }
  else
    node = new HostedLocalGeometryNode(geom.get(), p.style_);

  node->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
  node->setName("simVis::GOG::Cone");

  Utils::applyLocalGeometryOffsets(*node, p, nodeType);
  GogNodeInterface* rv = new ConeNodeInterface(node, metaData);
  rv->applyToStyle(parsedShape, p.units_);
  rv->setFilledState(true); // always filled
  return rv;
}

} }
