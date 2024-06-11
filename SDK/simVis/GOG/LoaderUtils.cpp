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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#include <cassert>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/String/Format.h"
#include "osgEarth/Geometry"
#include "osgEarth/LocalGeometryNode"
#include "osgEarth/NodeUtils"
#include "osgEarth/SpatialReference"
#include "simVis/GOG/LoaderUtils.h"

namespace simVis { namespace GOG {

bool LoaderUtils::isShape3D(const simCore::GOG::GogShape& shape)
{
  if (shape.shapeType() == simCore::GOG::ShapeType::SPHERE || shape.shapeType() == simCore::GOG::ShapeType::HEMISPHERE)
    return true;
  if (shape.shapeType() == simCore::GOG::ShapeType::CYLINDER)
  {
    const simCore::GOG::Cylinder* cyl = dynamic_cast<const simCore::GOG::Cylinder*>(&shape);
    if (cyl)
    {
      double height = 0.;
      cyl->getHeight(height);
      return height > 0.;
    }
    else
      assert(0); // dev error, parser provided incorrect object for type
  }
  else if (shape.shapeType() == simCore::GOG::ShapeType::ELLIPSOID || shape.shapeType() == simCore::GOG::ShapeType::CONE)
  {
    const simCore::GOG::CircularHeightShape* heightShape = dynamic_cast<const simCore::GOG::CircularHeightShape*>(&shape);
    if (heightShape)
    {
      double height = 0.;
      heightShape->getHeight(height);
      return height > 0.;
    }
    else
      assert(0); // dev error, parser provided incorrect object for type
  }
  else if (shape.shapeType() == simCore::GOG::ShapeType::LATLONALTBOX)
  {
    const simCore::GOG::LatLonAltBox* llab = dynamic_cast<const simCore::GOG::LatLonAltBox*>(&shape);
    if (llab)
    {
      double height = 0.;
      llab->getHeight(height);
      return height > 0.;
    }
    else
      assert(0); // dev error, parser provided incorrect object for type
  }
  return 0;
}

bool LoaderUtils::geometryRequiresClipping(const simCore::GOG::GogShape& shape)
{
  // check all the conditions under which geometry would render on the ground
  // and potentially cause z-fighting.

  // non-relative terrain clamping: yes.
  simCore::GOG::AltitudeMode mode = simCore::GOG::AltitudeMode::NONE;
  shape.getAltitudeMode(mode);
  if (mode == simCore::GOG::AltitudeMode::CLAMP_TO_GROUND)
    return true;

  // 3D geometry? Never clip. But in the case of 2D geometry, we need to check whether
  // it is absolute or relative.
  bool geomIs3D = isShape3D(shape);
  if (geomIs3D)
    return false;

  // 2D and absolute? yes, clip.
  if (!shape.isRelative())
    return true;

  simCore::Vec3 referencePoint;
  // 2D, relative to a map position, and Z=0 on the map position? Clip.
  if (shape.getReferencePosition(referencePoint) == 0 && referencePoint.alt() == 0.)
  {
    const simCore::GOG::CircularShape* circular = dynamic_cast<const simCore::GOG::CircularShape*>(&shape);
    if (circular)
    {
      simCore::Vec3 center;
      circular->getCenterPosition(center);
      return (circular && center.z() > 0.);
    }
  }

  // Out of things to check. No clip.
  return false;
}

osgEarth::GeoPoint LoaderUtils::getShapeGeoPosition(const simCore::GOG::GogShape& shape, const simCore::Vec3& centerPoint, const simCore::Vec3& refPoint, bool ignoreOffset)
{
  std::string vdatum;
  shape.getVerticalDatum(vdatum);
  osg::ref_ptr<osgEarth::SpatialReference> srs = getSrs(vdatum);

  simCore::Vec3 refLla;
  // handle relative un-attached
  if (shape.isRelative())
  {
    // use shape's reference position first, otherwise use the default ref point
    if (shape.getReferencePosition(refLla) != 0)
      refLla = refPoint;

    osg::Vec3d xyz(centerPoint.x(), centerPoint.y(), centerPoint.z());
    // if ignoring offset or center xyz has no offsets, set the ref point as the center
    if (ignoreOffset || (xyz.x() == 0 && xyz.y() == 0 && xyz.z() == 0))
      return osgEarth::GeoPoint(srs.get(), osg::Vec3d(refLla.lon() * simCore::RAD2DEG, refLla.lat() * simCore::RAD2DEG, refLla.alt()), osgEarth::ALTMODE_ABSOLUTE);
    else
    {
      // apply any xyz offset to the map position ref point
      simCore::CoordinateConverter cc;
      cc.setReferenceOrigin(refLla.lat(), refLla.lon(), refLla.alt());
      simCore::Coordinate coord(simCore::COORD_SYS_XEAST, simCore::Vec3(xyz.x(), xyz.y(), xyz.z()));
      simCore::Coordinate outCoord;
      cc.convert(coord, outCoord, simCore::COORD_SYS_LLA);

      xyz.y() = outCoord.lat() * simCore::RAD2DEG;
      xyz.x() = outCoord.lon() * simCore::RAD2DEG;
      xyz.z() = outCoord.alt();
      return osgEarth::GeoPoint(srs.get(), xyz, osgEarth::ALTMODE_ABSOLUTE);
    }
  }
  // just use the provided center point
  return osgEarth::GeoPoint(srs.get(), osg::Vec3d(centerPoint.lon() * simCore::RAD2DEG, centerPoint.lat() * simCore::RAD2DEG, centerPoint.alt()), osgEarth::ALTMODE_ABSOLUTE);
}

void LoaderUtils::setShapePositionOffsets(osgEarth::LocalGeometryNode& node, const simCore::GOG::GogShape& shape, const simCore::Vec3& centerPoint, const simCore::Vec3& refPoint, bool attached, bool ignoreOffset)
{
  if (attached)
  {
    // if this is an attached node, it will need to set any offsets in the attitude transform's position, since it's position is ignored
    osg::PositionAttitudeTransform* trans = node.getPositionAttitudeTransform();
    if (trans != nullptr)
      trans->setPosition(osg::Vec3d(centerPoint.x(), centerPoint.y(), centerPoint.z()));
    return;
  }

  // if this is an absolute node, set position directly on the node; note that un-attached relative GOGs are treated as absolute here
  osgEarth::GeoPoint center = getShapeGeoPosition(shape, centerPoint, refPoint, ignoreOffset);
  node.setPosition(center);
}

void LoaderUtils::setScale(const simCore::GOG::GogShape& shape, osg::Node* node)
{
  simCore::Vec3 scale;
  if (shape.getScale(scale) != 0)
    return;
  osgEarth::AnnotationNode* anno = osgEarth::findTopMostNodeOfType<osgEarth::AnnotationNode>(node);
  if (!anno)
    return;

  if (dynamic_cast<osgEarth::GeoPositionNode*>(anno))
  {
    osgEarth::GeoPositionNode* local = static_cast<osgEarth::GeoPositionNode*>(anno);
    local->setScale(osg::Vec3f(static_cast<float>(scale.x()), static_cast<float>(scale.y()), static_cast<float>(scale.z())));
  }
}
osgEarth::SpatialReference* LoaderUtils::getSrs(const std::string vdatum)
{
  if (simCore::caseCompare(vdatum, "egm1984") == 0 || simCore::caseCompare(vdatum, "egm84") == 0)
    return osgEarth::SpatialReference::create("wgs84", "egm84");
  else if (simCore::caseCompare(vdatum, "egm1996") == 0 || simCore::caseCompare(vdatum, "egm96") == 0)
    return osgEarth::SpatialReference::create("wgs84", "egm96");
  else if (simCore::caseCompare(vdatum, "egm2008") == 0 || simCore::caseCompare(vdatum, "egm08") == 0)
    return osgEarth::SpatialReference::create("wgs84", "egm2008");
  return osgEarth::SpatialReference::create("wgs84");
}

osg::Vec4f LoaderUtils::convertToOsgColor(const simCore::GOG::Color& color)
{
  return osg::Vec4f(static_cast<float>(color.red) / 255.0, static_cast<float>(color.green) / 255.0, static_cast<float>(color.blue) / 255.0, static_cast<float>(color.alpha) / 255.0);
}

AltitudeMode LoaderUtils::convertToVisAltitudeMode(simCore::GOG::AltitudeMode mode)
{
  switch (mode)
  {
  case simCore::GOG::AltitudeMode::NONE:
    break;
  case simCore::GOG::AltitudeMode::CLAMP_TO_GROUND:
    return ALTITUDE_GROUND_CLAMPED;
  case simCore::GOG::AltitudeMode::RELATIVE_TO_GROUND:
    return ALTITUDE_GROUND_RELATIVE;
  case simCore::GOG::AltitudeMode::EXTRUDE:
    return ALTITUDE_EXTRUDE;
  }
  return ALTITUDE_NONE;
}
Utils::LineStyle LoaderUtils::convertToVisLineStyle(simCore::GOG::LineStyle lineStyle)
{
  switch (lineStyle)
  {
  case simCore::GOG::LineStyle::SOLID:
    break;
  case simCore::GOG::LineStyle::DASHED:
    return Utils::LINE_DASHED;
  case simCore::GOG::LineStyle::DOTTED:
    return Utils::LINE_DOTTED;
  }
  return Utils::LINE_SOLID;
}

void LoaderUtils::setPoints(const std::vector<simCore::Vec3>& points, bool relative, osgEarth::Geometry& geom)
{
  if (relative)
  {
    for (const auto& xyz : points)
    {
      const osg::Vec3d point(xyz.x(), xyz.y(), xyz.z());
      // Avoid adding the same point twice
      if (geom.empty() || (*geom.rbegin()) != point)
        geom.push_back(point);
    }
    return;
  }
  for (const auto& lla : points)
  {
    const osg::Vec3d point(lla.lon() * simCore::RAD2DEG, lla.lat() * simCore::RAD2DEG, lla.alt());

    // Avoid adding the same point twice
    if (geom.empty() || (*geom.rbegin()) != point)
      geom.push_back(point);
  }
}

GogShape LoaderUtils::convertToVisShapeType(simCore::GOG::ShapeType type)
{
  switch (type)
  {
  case simCore::GOG::ShapeType::UNKNOWN:
    break;
  case simCore::GOG::ShapeType::ANNOTATION:
    return GOG_ANNOTATION;
  case simCore::GOG::ShapeType::ARC:
    return GOG_ARC;
  case simCore::GOG::ShapeType::CIRCLE:
    return GOG_CIRCLE;
  case simCore::GOG::ShapeType::CONE:
    return GOG_CONE;
  case simCore::GOG::ShapeType::CYLINDER:
    return GOG_CYLINDER;
  case simCore::GOG::ShapeType::ELLIPSE:
    return GOG_ELLIPSE;
  case simCore::GOG::ShapeType::ELLIPSOID:
    return GOG_ELLIPSOID;
  case simCore::GOG::ShapeType::HEMISPHERE:
    return GOG_HEMISPHERE;
  case simCore::GOG::ShapeType::IMAGEOVERLAY:
    return GOG_IMAGEOVERLAY;
  case simCore::GOG::ShapeType::LATLONALTBOX:
    return GOG_LATLONALTBOX;
  case simCore::GOG::ShapeType::LINE:
    return GOG_LINE;
  case simCore::GOG::ShapeType::LINESEGS:
    return GOG_LINESEGS;
  case simCore::GOG::ShapeType::ORBIT:
    return GOG_ORBIT;
  case simCore::GOG::ShapeType::POINTS:
    return GOG_POINTS;
  case simCore::GOG::ShapeType::POLYGON:
    return GOG_POLYGON;
  case simCore::GOG::ShapeType::SPHERE:
    return GOG_SPHERE;
  }
  return GOG_UNKNOWN;
}

TessellationStyle LoaderUtils::convertToVisTessellation(simCore::GOG::TessellationStyle style)
{
  switch (style)
  {
  case simCore::GOG::TessellationStyle::NONE:
    break;
  case simCore::GOG::TessellationStyle::GREAT_CIRCLE:
    return TESSELLATE_GREAT_CIRCLE_PROJECTION;
  case simCore::GOG::TessellationStyle::RHUMBLINE:
    return TESSELLATE_RHUMBLINE;
  }
  return TESSELLATE_NONE;
}
simData::TextOutline LoaderUtils::convertToVisOutlineThickness(simCore::GOG::OutlineThickness thickness)
{
  switch (thickness)
  {
  case simCore::GOG::OutlineThickness::NONE:
    break;
  case simCore::GOG::OutlineThickness::THIN:
    return simData::TO_THIN;
  case simCore::GOG::OutlineThickness::THICK:
    return simData::TO_THICK;
  }
  return simData::TO_NONE;
}

simCore::GOG::AltitudeMode LoaderUtils::convertToCoreAltitudeMode(AltitudeMode mode)
{
  switch (mode)
  {
  case ALTITUDE_NONE:
    break;
  case ALTITUDE_GROUND_CLAMPED:
    return simCore::GOG::AltitudeMode::CLAMP_TO_GROUND;
  case ALTITUDE_GROUND_RELATIVE:
    return simCore::GOG::AltitudeMode::RELATIVE_TO_GROUND;
  case ALTITUDE_EXTRUDE:
    return simCore::GOG::AltitudeMode::EXTRUDE;
  }
  return simCore::GOG::AltitudeMode::NONE;
}

simCore::GOG::Color LoaderUtils::convertToCoreColor(const osg::Vec4f& color)
{
  return simCore::GOG::Color(color.r() * 255, color.g() * 255, color.b() * 255, color.a() * 255);
}

simCore::GOG::LineStyle LoaderUtils::convertToCoreLineStyle(Utils::LineStyle style)
{
  switch (style)
  {
  case Utils::LINE_SOLID:
    break;
  case Utils::LINE_DASHED:
    return simCore::GOG::LineStyle::DASHED;
  case Utils::LINE_DOTTED:
    return simCore::GOG::LineStyle::DOTTED;
  }
  return simCore::GOG::LineStyle::SOLID;
}

simCore::GOG::TessellationStyle LoaderUtils::convertToCoreTessellation(TessellationStyle style)
{
  switch (style)
  {
  case TESSELLATE_NONE:
    break;
  case TESSELLATE_GREAT_CIRCLE_PROJECTION:
    return simCore::GOG::TessellationStyle::GREAT_CIRCLE;
  case TESSELLATE_RHUMBLINE:
    return simCore::GOG::TessellationStyle::RHUMBLINE;
  }
  return simCore::GOG::TessellationStyle::NONE;
}

simCore::GOG::OutlineThickness LoaderUtils::convertToCoreOutlineThickness(simData::TextOutline thickness)
{
  switch (thickness)
  {
  case simData::TO_NONE:
    break;
  case simData::TO_THIN:
    return simCore::GOG::OutlineThickness::THIN;
  case simData::TO_THICK:
    return simCore::GOG::OutlineThickness::THICK;
  }
  return simCore::GOG::OutlineThickness::NONE;
}

} }
