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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/FrontFace"
#include "osgEarth/GeometryFactory"
#include "osgEarth/GeometryCompiler"
#include "osgEarth/FeatureNode"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/GOG/GogShape.h"
#include "simVis/GOG/GogNodeInterface.h"
#include "simVis/GOG/LoaderUtils.h"
#include "simVis/GOG/ParsedShape.h"
#include "simVis/GOG/Utils.h"
#include "simVis/GOG/LatLonAltBox.h"

using namespace osgEarth;

namespace simVis { namespace GOG {

/** Ring that provides a noop for the rewind() method; used to fix box winding */
class NoRewindRing : public osgEarth::Ring
{
public:
  NoRewindRing()
   : Ring()
  {
  }
  explicit NoRewindRing(const NoRewindRing& rhs)
   : Ring(rhs)
  {
  }
  explicit NoRewindRing(const Vec3dVector* toCopy)
   : Ring(toCopy)
  {
  }

  virtual void rewind(Orientation ori)
  {
    // Noop: Do not automatically rewind
  }

  virtual Geometry* cloneAs(const Geometry::Type& newType) const
  {
    if (newType == TYPE_LINESTRING)
      return Ring::cloneAs(newType);
    // Always return a NoRewindRing instead of a Ring
    return new NoRewindRing(&asVector());
  }
};

///////////////////////////////////////////////////////////////////////

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


  // Make sure min <= max
  if (minLat > maxLat)
    std::swap(minLat, maxLat);
  if (minLon > maxLon)
    std::swap(minLon, maxLon);
  if (minAlt > maxAlt)
    std::swap(minAlt, maxAlt);

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
    style.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = true;

    // geometry for bottom/left/back
    MultiGeometry* linesBottom = new MultiGeometry();
    Geometry* bottom = linesBottom->add(new NoRewindRing());
    bottom->push_back(minPoint.vec3d());
    bottom->push_back(minPoint.x(), maxPoint.y(), minPoint.z());
    bottom->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
    bottom->push_back(maxPoint.x(), minPoint.y(), minPoint.z());

    // geometry for top/right/front
    MultiGeometry* linesTop = new MultiGeometry();
    Geometry* top = linesTop->add(new NoRewindRing());
    top->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
    top->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
    top->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
    top->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
    osgEarth::Geometry::Orientation ori2 = top->getOrientation();

    // Top and bottom are required for proper display above and below.  Sides are not required for 0-height.
    if (maxPoint.z() > minPoint.z())
    {
      Geometry* left = linesBottom->add(new NoRewindRing());
      left->push_back(minPoint.x(), minPoint.y(), minPoint.z());
      left->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
      left->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
      left->push_back(minPoint.x(), maxPoint.y(), minPoint.z());

      Geometry* right = linesTop->add(new NoRewindRing());
      right->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
      right->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
      right->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
      right->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());

      Geometry* back = linesBottom->add(new NoRewindRing());
      back->push_back(minPoint.x(), maxPoint.y(), minPoint.z());
      back->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
      back->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
      back->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());

      Geometry* front = linesTop->add(new NoRewindRing());
      front->push_back(minPoint.x(), minPoint.y(), minPoint.z());
      front->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
      front->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
      front->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
    }

    // An unfilled LLB should be drawn as lines, so remove any conflicting symbology
    if (!parsedShape.hasValue(GOG_FILLED))
      style.remove<PolygonSymbol>();

    Feature* featureBottom = new Feature(linesBottom, p.srs_.get(), style);
    featureBottom->setName("GOG LatLonAltBox Feature Bottom");
    osgEarth::FeatureNode* nodeBottom = new osgEarth::FeatureNode(featureBottom);
    nodeBottom->setName("GOG LatLonAltBox Bottom");
    nodeBottom->setMapNode(mapNode);
    // Set the node facing to clockwise, to solve winding issue with osgEarth
    nodeBottom->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);

    Feature* featureTop = new Feature(linesTop, p.srs_.get(), style);
    featureTop->setName("GOG LatLonAltBox Feature Top");
    osgEarth::FeatureNode* nodeTop = new osgEarth::FeatureNode(featureTop);
    nodeTop->setName("GOG LatLonAltBox Top");
    nodeTop->setMapNode(mapNode);

    osg::Group* parent = new osg::Group();
    parent->addChild(nodeBottom);
    parent->addChild(nodeTop);

    GogNodeInterface* rv = new LatLonAltBoxInterface(parent, nodeTop, nodeBottom, metaData);
    rv->applyToStyle(parsedShape, p.units_);
    return rv;
  }
  // no "hosted" version of this GOG.
  return nullptr;
}

GogNodeInterface* LatLonAltBox::createLatLonAltBox(const simCore::GOG::LatLonAltBox& llab, bool attached, const simCore::Vec3& refPoint, osgEarth::MapNode* mapNode)
{
  // no "hosted" version of this GOG.
  if (attached)
    return nullptr;

  osgEarth::Angle minLat(llab.south() * simCore::RAD2DEG, Units::DEGREES);
  osgEarth::Angle maxLat(llab.north() * simCore::RAD2DEG, Units::DEGREES);
  osgEarth::Angle minLon(llab.west() * simCore::RAD2DEG, Units::DEGREES);
  osgEarth::Angle maxLon(llab.east() * simCore::RAD2DEG, Units::DEGREES);
  osgEarth::Distance minAlt(llab.altitude(), Units::METERS);

  double height = 0.;
  llab.getHeight(height);
  osgEarth::Distance maxAlt(height + llab.altitude(), Units::METERS);


  // Make sure min <= max
  if (minLat > maxLat)
    std::swap(minLat, maxLat);
  if (minLon > maxLon)
    std::swap(minLon, maxLon);
  if (minAlt > maxAlt)
    std::swap(minAlt, maxAlt);

  Style style;

  std::string vdatum;
  llab.getVerticalDatum(vdatum);
  osgEarth::SpatialReference* srs = LoaderUtils::getSrs(vdatum);

  GeoPoint minPoint(
    srs,
    minLon.as(Units::DEGREES),
    minLat.as(Units::DEGREES),
    minAlt.as(Units::METERS),
    ALTMODE_ABSOLUTE);

  GeoPoint maxPoint(
    srs,
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
  style.getOrCreateSymbol<osgEarth::RenderSymbol>()->backfaceCulling() = true;

  // geometry for bottom/left/back
  MultiGeometry* linesBottom = new MultiGeometry();
  Geometry* bottom = linesBottom->add(new NoRewindRing());
  bottom->push_back(minPoint.vec3d());
  bottom->push_back(minPoint.x(), maxPoint.y(), minPoint.z());
  bottom->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
  bottom->push_back(maxPoint.x(), minPoint.y(), minPoint.z());

  // geometry for top/right/front
  MultiGeometry* linesTop = new MultiGeometry();
  Geometry* top = linesTop->add(new NoRewindRing());
  top->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
  top->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
  top->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
  top->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
  osgEarth::Geometry::Orientation ori2 = top->getOrientation();

  // Top and bottom are required for proper display above and below.  Sides are not required for 0-height.
  if (maxPoint.z() > minPoint.z())
  {
    Geometry* left = linesBottom->add(new NoRewindRing());
    left->push_back(minPoint.x(), minPoint.y(), minPoint.z());
    left->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
    left->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
    left->push_back(minPoint.x(), maxPoint.y(), minPoint.z());

    Geometry* right = linesTop->add(new NoRewindRing());
    right->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
    right->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());
    right->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
    right->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());

    Geometry* back = linesBottom->add(new NoRewindRing());
    back->push_back(minPoint.x(), maxPoint.y(), minPoint.z());
    back->push_back(minPoint.x(), maxPoint.y(), maxPoint.z());
    back->push_back(maxPoint.x(), maxPoint.y(), maxPoint.z());
    back->push_back(maxPoint.x(), maxPoint.y(), minPoint.z());

    Geometry* front = linesTop->add(new NoRewindRing());
    front->push_back(minPoint.x(), minPoint.y(), minPoint.z());
    front->push_back(maxPoint.x(), minPoint.y(), minPoint.z());
    front->push_back(maxPoint.x(), minPoint.y(), maxPoint.z());
    front->push_back(minPoint.x(), minPoint.y(), maxPoint.z());
  }

  // An unfilled LLB should be drawn as lines, so remove any conflicting symbology
  bool filled = false;
  llab.getIsFilled(filled);
  if (!filled)
    style.remove<PolygonSymbol>();

  Feature* featureBottom = new Feature(linesBottom, srs, style);
  featureBottom->setName("GOG LatLonAltBox Feature Bottom");
  osgEarth::FeatureNode* nodeBottom = new osgEarth::FeatureNode(featureBottom);
  nodeBottom->setName("GOG LatLonAltBox Bottom");
  nodeBottom->setMapNode(mapNode);
  // Set the node facing to clockwise, to solve winding issue with osgEarth
  nodeBottom->getOrCreateStateSet()->setAttributeAndModes(new osg::FrontFace(osg::FrontFace::CLOCKWISE), osg::StateAttribute::ON);

  Feature* featureTop = new Feature(linesTop, srs, style);
  featureTop->setName("GOG LatLonAltBox Feature Top");
  osgEarth::FeatureNode* nodeTop = new osgEarth::FeatureNode(featureTop);
  nodeTop->setName("GOG LatLonAltBox Top");
  nodeTop->setMapNode(mapNode);

  osg::Group* parent = new osg::Group();
  parent->addChild(nodeBottom);
  parent->addChild(nodeTop);

  GogMetaData metaData;
  return new LatLonAltBoxInterface(parent, nodeTop, nodeBottom, metaData);
}

} }
