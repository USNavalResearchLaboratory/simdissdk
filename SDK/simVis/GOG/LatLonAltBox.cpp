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
#include "osgEarthAnnotation/FeatureNode"
#include "simVis/GOG/LatLonAltBox.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"

using namespace simVis::GOG;
using namespace osgEarth::Features;

GogNodeInterface* LatLonAltBox::deserialize(const ParsedShape& parsedShape,
                          simVis::GOG::ParserData& p,
                          const GOGNodeType&       nodeType,
                          const GOGContext&        context,
                          const GogMetaData&       metaData,
                          osgEarth::MapNode*       mapNode)
{
  osgEarth::Angle    minLat(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_S), 0.0)), Units::DEGREES);
  osgEarth::Angle    maxLat(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_N), 1.0)), Units::DEGREES);
  osgEarth::Angle    minLon(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_W), 0.0)), Units::DEGREES);
  osgEarth::Angle    maxLon(p.units_.angleUnits_.convertTo(simCore::Units::DEGREES, p.parseAngle(parsedShape.stringValue(GOG_LLABOX_E), 1.0)), Units::DEGREES);
  osgEarth::Distance minAlt(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_LLABOX_MINALT, 0.0)), Units::METERS);
  osgEarth::Distance maxAlt(p.units_.altitudeUnits_.convertTo(simCore::Units::METERS, parsedShape.doubleValue(GOG_LLABOX_MAXALT, 1000.0)), Units::METERS);

  if (nodeType == GOGNODE_GEOGRAPHIC)
  {
    Style style(p.style_);

    GeoPoint minPoint(
      p.srs_.get(),
      minLon.as(Units::DEGREES),
      minLat.as(Units::DEGREES),
      minAlt.as(Units::METERS),
      ALTMODE_ABSOLUTE);

    GeoPoint maxPoint(
      p.srs_.get(),
      maxLon.as(Units::DEGREES),
      maxLat.as(Units::DEGREES),
      maxAlt.as(Units::METERS),
      ALTMODE_ABSOLUTE);

    // for a 3D box, we need to adjust clamp-to-terrain mode so that
    // the upper part of the box does not get smashed down. Use relative
    // mode instead, with the lower box at zero.
    if (maxPoint.z() > minPoint.z())
    {
      AltitudeSymbol* alt = style.get<AltitudeSymbol>();
      if (alt)
      {
        if (alt->clamping().isSetTo(AltitudeSymbol::CLAMP_TO_TERRAIN))
        {
          alt->clamping() = AltitudeSymbol::CLAMP_RELATIVE_TO_TERRAIN;
          maxPoint.z() = maxPoint.z() - minPoint.z();
          minPoint.z() = 0.0;
        }
      }
    }

    // Turn on backface culling.  Lines will still be visible through the polygons if the polygon is
    // semi-transparent.  If you always want to see lines then remove this line.
    style.getOrCreateSymbol<osgEarth::Symbology::RenderSymbol>()->backfaceCulling() = true;

    // result geometry:
    MultiGeometry* lines = new MultiGeometry();

    Geometry* bottom = lines->add(new Ring());
    bottom->push_back(minPoint.vec3d());
    bottom->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
    bottom->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
    bottom->push_back(minPoint.x(), maxPoint.y(), minPoint.z());

    if (maxPoint.z() > minPoint.z())
    {
      Geometry* left = lines->add(new Ring());
      left->push_back(minPoint.x(), minPoint.y(), minPoint.z());
      left->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
      left->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
      left->push_back(minPoint.x(), maxPoint.y(), minPoint.z());

      Geometry* right = lines->add(new Ring());
      right->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
      right->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
      right->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
      right->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());

      Geometry* back = lines->add(new Ring());
      back->push_back(minPoint.x(), minPoint.y(), minPoint.z());
      back->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
      back->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
      back->push_back(minPoint.x(), minPoint.y(), maxPoint.z());

      Geometry* front = lines->add(new Ring());
      front->push_back(minPoint.x(), maxPoint.y(), minPoint.z());
      front->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
      front->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
      front->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());

      Geometry* top = lines->add(new Ring());
      top->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
      top->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
      top->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
      top->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
    }

    // An unfilled LLB should be drawn as lines, so remove any conflicting symbology
    if (!parsedShape.hasValue(GOG_FILLED))
      style.remove<PolygonSymbol>();

    Feature* feature = new Feature(lines, mapNode->getMapSRS(), style);
    feature->setName("GOG LatLonAltBox Feature");
    osgEarth::Annotation::FeatureNode* node = new osgEarth::Annotation::FeatureNode(feature);
    node->setName("GOG LatLonAltBox");
    node->setMapNode(mapNode);
    GogNodeInterface* rv = new FeatureNodeInterface(node, metaData);
    rv->applyToStyle(parsedShape, p.units_);
    return rv;
  }
  // no "hosted" version of this GOG.
  return NULL;
}
