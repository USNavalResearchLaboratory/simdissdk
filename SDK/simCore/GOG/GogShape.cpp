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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iomanip>
#include <cassert>
#include <sstream>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Units.h"
#include "simCore/GOG/GogShape.h"

namespace simCore { namespace GOG {

std::string Color::serialize() const
{
  std::ostringstream os;
  uint32_t colorVal = (alpha << 24) + (blue << 16) + (green << 8) + red;
  os << "0x" << std::hex << std::setfill('0') << std::setw(8) << colorVal;
  return os.str();
}


GogShape::GogShape()
  : canExtrude_(false),
    canFollow_(false),
    relative_(false),
    serializeName_(true)
{}

GogShape::~GogShape()
{}

bool GogShape::isRelative() const
{
  return relative_;
}

int GogShape::getName(std::string& name) const
{
  name = name_.value_or(shapeTypeToString(shapeType()));
  return (name_.has_value() ? 0 : 1);
}

void GogShape::setName(const std::string& gogName)
{
  name_ = gogName;
}

int GogShape::getIsDrawn(bool& draw) const
{
  draw = draw_.value_or(true);
  return (draw_.has_value() ? 0 : 1);
}

void GogShape::setDrawn(bool draw)
{
  draw_ = draw;
}

int GogShape::getIsDepthBufferActive(bool& depthBuffer) const
{
  depthBuffer = depthBuffer_.value_or(false);
  return (depthBuffer_.has_value() ? 0 : 1);
}

void GogShape::setDepthBufferActive(bool depthBuffer)
{
  depthBuffer_ = depthBuffer;
}

int GogShape::getAltitudeOffset(double& altitudeOffset) const
{
  altitudeOffset = altitudeOffset_.value_or(0.);
  return (altitudeOffset_.has_value() ? 0 : 1);
}

void GogShape::setAltitudeOffset(double altOffsetMeters)
{
  altitudeOffset_ = altOffsetMeters;
}

int GogShape::getAltitudeMode(AltitudeMode& mode) const
{
  mode = altitudeMode_.value_or(AltitudeMode::NONE);
  return (altitudeMode_.has_value() ? 0 : 1);
}

void GogShape::setAltitudeMode(AltitudeMode mode)
{
  if (mode == AltitudeMode::EXTRUDE && !canExtrude_)
    return;
  altitudeMode_ = mode;
}

int GogShape::getExtrudeHeight(double& height) const
{
  height = extrudeHeight_.value_or(0.);
  return (extrudeHeight_.has_value() ? 0 : 1);
}

void GogShape::setExtrudeHeight(double heightMeters)
{
  extrudeHeight_ = heightMeters;
}

int GogShape::getReferencePosition(simCore::Vec3& refPos) const
{
  refPos = referencePosition_.value_or(simCore::Vec3());
  return (referencePosition_.has_value() ? 0 : 1);
}

void GogShape::setReferencePosition(const simCore::Vec3& refPos)
{
  // reference position is only valid for relative shapes
  if (!relative_)
    return;
  referencePosition_ = refPos;
}

int GogShape::getScale(simCore::Vec3& scale) const
{
  scale = scale_.value_or(simCore::Vec3(1., 1., 1.));
  return (scale_.has_value() ? 0 : 1);
}

void GogShape::setScale(const simCore::Vec3& scale)
{
  scale_ = scale;
}

int GogShape::getIsFollowingYaw(bool& follow) const
{
  follow = followYaw_.value_or(false);
  return (followYaw_.has_value() ? 0 : 1);
}

void GogShape::setFollowYaw(bool follow)
{
  if (canFollow_)
    followYaw_ = follow;
}

int GogShape::getIsFollowingPitch(bool& follow) const
{
  follow = followPitch_.value_or(false);
  return (followPitch_.has_value() ? 0 : 1);
}

void GogShape::setFollowPitch(bool follow)
{
  if (canFollow_)
    followPitch_ = follow;
}

int GogShape::getIsFollowingRoll(bool& follow) const
{
  follow = followRoll_.value_or(false);
  return (followRoll_.has_value() ? 0 : 1);
}

void GogShape::setFollowRoll(bool follow)
{
  if (canFollow_)
    followRoll_ = follow;
}

int GogShape::getYawOffset(double& offset) const
{
  offset = yawOffset_.value_or(0.);
  return (yawOffset_.has_value() ? 0 : 1);
}

void GogShape::setYawOffset(double offset)
{
  if (canFollow_)
    yawOffset_ = offset;
}

int GogShape::getPitchOffset(double& offset) const
{
  offset = pitchOffset_.value_or(0.);
  return (pitchOffset_.has_value() ? 0 : 1);
}

void GogShape::setPitchOffset(double offset)
{
  if (canFollow_)
    pitchOffset_ = offset;
}

int GogShape::getRollOffset(double& offset) const
{
  offset = rollOffset_.value_or(0.);
  return (rollOffset_.has_value() ? 0 : 1);
}

void GogShape::setRollOffset(double offset)
{
  if (canFollow_)
    rollOffset_ = offset;
}

const std::vector<std::string>& GogShape::comments() const
{
  return comments_;
}

int GogShape::getVerticalDatum(std::string& verticalDatum) const
{
  verticalDatum = verticalDatum_.value_or("wgs84");
  return (verticalDatum_.has_value() ? 0 : 1);
}

void GogShape::setVerticalDatum(const std::string& verticalDatum)
{
  verticalDatum_ = verticalDatum;
}

void GogShape::addComment(const std::string& comment)
{
  comments_.push_back(comment);
}

std::string GogShape::shapeTypeToString(ShapeType shapeType)
{
  switch (shapeType)
  {
  case ShapeType::ANNOTATION:
    return "annotation";
  case ShapeType::CIRCLE:
    return "circle";
  case ShapeType::ELLIPSE:
    return "ellipse";
  case ShapeType::ELLIPSOID:
    return "ellipsoid";
  case ShapeType::ARC:
    return "arc";
  case ShapeType::CYLINDER:
    return "cylinder";
  case ShapeType::HEMISPHERE:
    return "hemisphere";
  case ShapeType::SPHERE:
    return "sphere";
  case ShapeType::POINTS:
    return "points";
  case ShapeType::LINE:
    return "line";
  case ShapeType::POLYGON:
    return "polygon";
  case ShapeType::LINESEGS:
    return "linesegs";
  case ShapeType::LATLONALTBOX:
    return "latlonaltbox";
  case ShapeType::CONE:
    return "cone";
  case ShapeType::IMAGEOVERLAY:
    return "imageoverlay";
  case ShapeType::ORBIT:
    return "orbit";
  case ShapeType::UNKNOWN:
    break;
  }
  return "";
}

ShapeType GogShape::stringToShapeType(const std::string& shapeType)
{
  if (shapeType == "annotation")
    return ShapeType::ANNOTATION;
  if (shapeType == "circle")
    return ShapeType::CIRCLE;
  if (shapeType == "ellipse")
    return ShapeType::ELLIPSE;
  if (shapeType == "arc")
    return ShapeType::ARC;
  if (shapeType == "cylinder")
    return ShapeType::CYLINDER;
  if (shapeType == "hemisphere")
    return ShapeType::HEMISPHERE;
  if (shapeType == "sphere")
    return ShapeType::SPHERE;
  if (shapeType == "ellipsoid")
    return ShapeType::ELLIPSOID;
  if (shapeType == "points")
    return ShapeType::POINTS;
  if (shapeType == "line")
    return ShapeType::LINE;
  if (shapeType == "poly")
    return ShapeType::POLYGON;
  if (shapeType == "polygon")
    return ShapeType::POLYGON;
  if (shapeType == "linesegs")
    return ShapeType::LINESEGS;
  if (shapeType == "latlonaltbox")
    return ShapeType::LATLONALTBOX;
  if (shapeType == "cone")
    return ShapeType::CONE;
  if (shapeType == "imageoverlay")
    return ShapeType::IMAGEOVERLAY;
  if (shapeType == "orbit")
    return ShapeType::ORBIT;
  return ShapeType::UNKNOWN;
}

void GogShape::setOriginalUnits(const UnitsState& units)
{
  originalUnits_ = units;
}

void GogShape::serializeToStream(std::ostream& gogOutputStream) const
{
  gogOutputStream << "start\n";
  // comments should be serialized first
  for (std::string comment : comments_)
    gogOutputStream << comment << "\n";

  // first call implementation methods
  serializeToStream_(gogOutputStream);

  if (serializeName_ && name_.has_value())
    gogOutputStream << "3d name " << name_.value_or("") << "\n";

  // serialize out draw state only if it's specifically set to 'off'
  if (draw_.has_value() && !draw_.value_or(true))
    gogOutputStream << "off\n";

  simCore::Units altUnits(simCore::Units::METERS);

  if (altitudeOffset_.has_value())
    gogOutputStream << "3d offsetalt " << altUnits.convertTo(originalUnits_.altitudeUnits(), altitudeOffset_.value_or(0.)) << "\n";

  if (depthBuffer_.has_value())
    gogOutputStream << "depthbuffer " << (depthBuffer_.value_or(true) ? "true" : "false") << "\n";

  if (altitudeMode_.has_value())
  {
    std::string altitudeModeStr = "altitudemode ";
    switch (altitudeMode_.value_or(AltitudeMode::NONE))
    {
    case AltitudeMode::NONE:
      gogOutputStream << altitudeModeStr << "none\n";
      break;
    case AltitudeMode::CLAMP_TO_GROUND:
      gogOutputStream << altitudeModeStr << "clamptoground\n";
      break;
    case AltitudeMode::RELATIVE_TO_GROUND:
      gogOutputStream << altitudeModeStr << "relativetoground\n";
      break;
    case AltitudeMode::EXTRUDE:
      altitudeModeStr = "extrude true";
      if (extrudeHeight_.has_value())
        gogOutputStream << altitudeModeStr << " " << altUnits.convertTo(originalUnits_.altitudeUnits(), extrudeHeight_.value_or(0.)) << "\n";
      else
        gogOutputStream << altitudeModeStr << "\n";
      break;
    }
  }

  if (referencePosition_.has_value())
  {
    simCore::Vec3 ref = referencePosition_.value_or(simCore::Vec3());
    gogOutputStream << "ref " << ref.lat() * simCore::RAD2DEG << " " << ref.lon() * simCore::RAD2DEG << " " << altUnits.convertTo(originalUnits_.altitudeUnits(), ref.alt()) << "\n";
  }

  if (scale_.has_value())
  {
    simCore::Vec3 scale = scale_.value_or(simCore::Vec3());
    gogOutputStream << "scale " << scale.x() << " " << scale.y() << " " << scale.z() << "\n";
  }

  if (verticalDatum_.has_value())
    gogOutputStream << "verticaldatum " << verticalDatum_.value_or("") << "\n";

  if (originalUnits_.hasAltitudeUnits())
    gogOutputStream << "altitudeunits " << originalUnits_.altitudeUnits().abbreviation() << "\n";

  if (originalUnits_.hasAngleUnits())
    gogOutputStream << "angleunits " << originalUnits_.angleUnits().abbreviation() << "\n";

  if (originalUnits_.hasRangeUnits())
    gogOutputStream << "rangeunits " << originalUnits_.rangeUnits().abbreviation() << "\n";

  // follow data can be presented in multiple ways (3d follow, orient,  rotate, and the 3d offsetcourse, offsetpitch, offsetroll values)
  // serialize out using the 3d follow, which provides the most well defined values
  std::string followComponents;
  if (followYaw_.value_or(false))
    followComponents += "c";
  if (followPitch_.value_or(false))
    followComponents += "p";
  if (followRoll_.value_or(false))
    followComponents += "r";
  if (!followComponents.empty())
    gogOutputStream << "3d follow " << followComponents << "\n";

  if (yawOffset_.has_value())
    gogOutputStream << "3d offsetcourse " << simCore::Units::RADIANS.convertTo(originalUnits_.angleUnits(), yawOffset_.value_or(0.)) << "\n";
  if (pitchOffset_.has_value())
    gogOutputStream << "3d offsetpitch " << simCore::Units::RADIANS.convertTo(originalUnits_.angleUnits(), pitchOffset_.value_or(0.)) << "\n";
  if (rollOffset_.has_value())
    gogOutputStream << "3d offsetroll " << simCore::Units::RADIANS.convertTo(originalUnits_.angleUnits(), rollOffset_.value_or(0.)) << "\n";

  gogOutputStream << "end\n";
}

void GogShape::serializePoints_(const std::vector<simCore::Vec3>& points, std::ostream& gogOutputStream) const
{
  simCore::Units distanceUnits(simCore::Units::METERS);
  for (simCore::Vec3 point : points)
  {
    if (isRelative())
      gogOutputStream << "xyz " << distanceUnits.convertTo(originalUnits_.rangeUnits(), point.x()) << " "
      << distanceUnits.convertTo(originalUnits_.rangeUnits(), point.y()) << " "
      << distanceUnits.convertTo(originalUnits_.altitudeUnits(), point.z()) << "\n";
    else
      gogOutputStream << "lla " << point.lat() * simCore::RAD2DEG << " " << point.lon() * simCore::RAD2DEG << " "
      << distanceUnits.convertTo(originalUnits_.altitudeUnits(), point.alt()) << "\n";
  }
}

void GogShape::setCanExtrude_(bool canExtrude)
{
  canExtrude_ = canExtrude;
}

void GogShape::setCanFollow_(bool canFollow)
{
  canFollow_ = canFollow;
}

void GogShape::setRelative_(bool relative)
{
  relative_ = relative;
}

void GogShape::setSerializeName_(bool serializeName)
{
  serializeName_ = serializeName;
}

OutlinedShape::OutlinedShape()
  : GogShape()
{}

int OutlinedShape::getIsOutlined(bool& outlined) const
{
  outlined = outlined_.value_or(true);
  return (outlined_.has_value() ? 0 : 1);
}

void OutlinedShape::setOutlined(bool outlined)
{
  outlined_ = outlined;
}

void OutlinedShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  if (outlined_.has_value())
    gogOutputStream << "outline " << (outlined_.value_or(true) ? "true" : "false") << "\n";
}

Points::Points(bool relative)
  : OutlinedShape()
{
  setCanExtrude_(false);
  setCanFollow_(relative);
  setRelative_(relative);
}

ShapeType Points::shapeType() const
{
  return ShapeType::POINTS;
}

const std::vector<simCore::Vec3>& Points::points() const
{
  return points_;
}

void Points::addPoint(const simCore::Vec3& point)
{
  points_.push_back(point);
}

int Points::getPointSize(int& size) const
{
  size = pointSize_.value_or(1);
  return (pointSize_.has_value() ? 0 : 1);
}

void Points::setPointSize(int pointSizePixels)
{
  pointSize_ = pointSizePixels;
}

int Points::getColor(Color& color) const
{
  color = color_.value_or(Color());
  return (color_.has_value() ? 0 : 1);
}

void Points::setColor(const Color& gogColor)
{
  color_ = gogColor;
}

void Points::serializeToStream_(std::ostream& gogOutputStream) const
{
  // points serialize shape type as a separate line item
  gogOutputStream << GogShape::shapeTypeToString(shapeType()) << "\n";

  serializePoints_(points_, gogOutputStream);

  if (pointSize_.has_value())
    gogOutputStream << "pointsize " << pointSize_.value_or(0) << "\n";

  if (color_.has_value())
    gogOutputStream << "linecolor hex " << color_->serialize() << "\n";

  OutlinedShape::serializeToStream_(gogOutputStream);
}

FillableShape::FillableShape()
  : OutlinedShape()
{
}

int FillableShape::getLineWidth(int& lineWidth) const
{
  lineWidth = lineWidth_.value_or(1);
  return (lineWidth_.has_value() ? 0 : 1);
}
void FillableShape::setLineWidth(int widthPixels)
{
  lineWidth_ = widthPixels;
}

int FillableShape::getLineColor(Color& color) const
{
  color = lineColor_.value_or(Color());
  return (lineColor_.has_value() ? 0 : 1);
}

void FillableShape::setLineColor(const Color& color)
{
  lineColor_ = color;
}

int FillableShape::getLineStyle(LineStyle& style) const
{
  style = lineStyle_.value_or(LineStyle::SOLID);
  return (lineStyle_.has_value() ? 0 : 1);
}
void FillableShape::setLineStyle(LineStyle style)
{
  lineStyle_ = style;
}

int FillableShape::getIsFilled(bool& filled) const
{
  filled = filled_.value_or(false);
  return (filled_.has_value() ? 0 : 1);
}

void FillableShape::setFilled(bool filled)
{
  filled_ = filled;
}

int FillableShape::getFillColor(Color& color) const
{
  color = fillColor_.value_or(Color());
  return (fillColor_.has_value() ? 0 : 1);
}

void FillableShape::setFillColor(const Color& color)
{
  fillColor_ = color;
}

void FillableShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  if (lineWidth_.has_value())
    gogOutputStream << "linewidth " << lineWidth_.value_or(0) << "\n";
  if (lineColor_.has_value())
    gogOutputStream << "linecolor hex " << lineColor_->serialize() << "\n";
  if (lineStyle_.has_value())
  {
    std::string lineStyle = "solid";
    switch (lineStyle_.value_or(LineStyle::SOLID))
    {
    case LineStyle::SOLID:
      break;
    case LineStyle::DASHED:
      lineStyle = "dashed";
      break;
    case LineStyle::DOTTED:
      lineStyle = "dotted";
      break;
    }
    gogOutputStream << "linestyle " << lineStyle << "\n";
  }
  if (filled_.value_or(false))
    gogOutputStream << "filled\n";
  if (fillColor_.has_value())
    gogOutputStream << "fillcolor hex " << fillColor_->serialize() << "\n";
  OutlinedShape::serializeToStream_(gogOutputStream);
}

PointBasedShape::PointBasedShape(bool relative)
  : FillableShape()
{
  setCanExtrude_(true);
  setCanFollow_(relative);
  setRelative_(relative);
}

const std::vector<simCore::Vec3>& PointBasedShape::points() const
{
  return points_;
}

void PointBasedShape::addPoint(const simCore::Vec3& point)
{
  points_.push_back(point);
}

int PointBasedShape::getTessellation(TessellationStyle& tessellation) const
{
  tessellation = tessellation_.value_or(TessellationStyle::NONE);
  return (tessellation_.has_value() ? 0 : 1);
}

void PointBasedShape::setTesssellation(TessellationStyle tessellation)
{
  tessellation_ = tessellation;
}

void PointBasedShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  // point based shapes serialize shape type as a separate line item
  gogOutputStream << GogShape::shapeTypeToString(shapeType()) << "\n";
  serializePoints_(points_, gogOutputStream);

  if (tessellation_.has_value())
  {
    std::string lineProjection;
    bool tessellate = true;
    switch (tessellation_.value_or(TessellationStyle::NONE))
    {
    case TessellationStyle::NONE:
      tessellate = false;
      break;
    case TessellationStyle::GREAT_CIRCLE:
      lineProjection = "greatcircle";
      break;
    case TessellationStyle::RHUMBLINE:
      lineProjection = "rhumbline";
      break;
    }
    gogOutputStream << "tessellate " << (tessellate ? "true" : "false") << "\n";
    if (!lineProjection.empty())
      gogOutputStream << "lineprojection " << lineProjection << "\n";
  }
  FillableShape::serializeToStream_(gogOutputStream);
}

Line::Line(bool relative)
  : PointBasedShape(relative)
{
}

ShapeType Line::shapeType() const
{
  return ShapeType::LINE;
}

LineSegs::LineSegs(bool relative)
  : PointBasedShape(relative)
{
}

ShapeType LineSegs::shapeType() const
{
  return ShapeType::LINESEGS;
}

Polygon::Polygon(bool relative)
  : PointBasedShape(relative)
{
}

ShapeType Polygon::shapeType() const
{
  return ShapeType::POLYGON;
}


CircularShape::CircularShape()
  : FillableShape()
{
  setCanFollow_(true);
}

int CircularShape::getCenterPosition(simCore::Vec3& centerPosition) const
{
  centerPosition = center_.value_or(simCore::Vec3());
  return (center_.has_value() ? 0 : 1);
}

void CircularShape::setCenterPosition(const simCore::Vec3& centerPosition)
{
  center_ = centerPosition;
}

int CircularShape::getRadius(double& radius) const
{
  // default radius is 1000 unitless
  radius = radius_.value_or(originalUnits_.rangeUnits().convertTo(simCore::Units::METERS, 1000.));
  return (radius_.has_value() ? 0 : 1);
}

void CircularShape::setRadius(double radiusMeters)
{
  radius_ = radiusMeters;
}

void CircularShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  // circular shapes serialize shape type as a separate line item
  gogOutputStream << GogShape::shapeTypeToString(shapeType()) << "\n";

  simCore::Units distanceUnits(simCore::Units::METERS);
  if (center_.has_value())
  {
    simCore::Vec3 center = center_.value_or(simCore::Vec3());
    if (isRelative())
      gogOutputStream << "centerxyz " << distanceUnits.convertTo(originalUnits_.rangeUnits(), center.x()) << " " << distanceUnits.convertTo(originalUnits_.rangeUnits(), center.y()) << " "
      << distanceUnits.convertTo(originalUnits_.altitudeUnits(), center.z()) << "\n";
    else
      gogOutputStream << "centerlla " << center.lat() * simCore::RAD2DEG << " " << center.lon() * simCore::RAD2DEG << " " << distanceUnits.convertTo(originalUnits_.altitudeUnits(), center.alt()) << "\n";
  }

  if (radius_.has_value())
    gogOutputStream << "radius " << distanceUnits.convertTo(originalUnits_.rangeUnits(), radius_.value_or(0.)) << "\n";

  FillableShape::serializeToStream_(gogOutputStream);
}

Circle::Circle(bool relative)
  : CircularShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

ShapeType Circle::shapeType() const
{
  return ShapeType::CIRCLE;
}

Sphere::Sphere(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

ShapeType Sphere::shapeType() const
{
  return ShapeType::SPHERE;
}

Hemisphere::Hemisphere(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

ShapeType Hemisphere::shapeType() const
{
  return ShapeType::HEMISPHERE;
}

Orbit::Orbit(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

ShapeType Orbit::shapeType() const
{
  return ShapeType::ORBIT;
}

simCore::Vec3 Orbit::centerPosition2() const
{
  return center2_;
}

void Orbit::setCenterPosition2(const simCore::Vec3& center2)
{
  center2_ = center2;
  // always use z from center position
  simCore::Vec3 center1;
  getCenterPosition(center1);
  center2_.setZ(center1.z());
}

void Orbit::serializeToStream_(std::ostream& gogOutputStream) const
{
  CircularShape::serializeToStream_(gogOutputStream);
  simCore::Units distanceUnits(simCore::Units::METERS);
  if (isRelative())
    gogOutputStream << "centerxy2 " << distanceUnits.convertTo(originalUnits_.rangeUnits(), center2_.x()) << " " << distanceUnits.convertTo(originalUnits_.rangeUnits(), center2_.y())  << "\n";
  else
    gogOutputStream << "centerll2 " << center2_.lat() * simCore::RAD2DEG << " " << center2_.lon() * simCore::RAD2DEG << "\n";
}

EllipticalShape::EllipticalShape()
  : CircularShape()
{
}

int EllipticalShape::getAngleStart(double& angle) const
{
  angle = angleStart_.value_or(0.);
  return (angleStart_.has_value() ? 0 : 1);
}

void EllipticalShape::setAngleStart(double angleStartRad)
{
  angleStart_ = angleStartRad;
}

int EllipticalShape::getAngleSweep(double& angle) const
{
  angle = angleSweep_.value_or(0.);
  return (angleSweep_.has_value() ? 0 : 1);
}

void EllipticalShape::setAngleSweep(double angleSweepRad)
{
  angleSweep_ = angleSweepRad;
}

int EllipticalShape::getMajorAxis(double& axis) const
{
  axis = majorAxis_.value_or(0.);
  return (majorAxis_.has_value() ? 0 : 1);
}

void EllipticalShape::setMajorAxis(double majorAxisMeters)
{
  majorAxis_ = majorAxisMeters;
}

int EllipticalShape::getMinorAxis(double& axis) const
{
  axis = minorAxis_.value_or(0.);
  return (minorAxis_.has_value() ? 0 : 1);
}

void EllipticalShape::setMinorAxis(double minorAxisMeters)
{
  minorAxis_ = minorAxisMeters;
}

void EllipticalShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  CircularShape::serializeToStream_(gogOutputStream);

  simCore::Units angleUnits(simCore::Units::RADIANS);
  if (angleStart_.has_value())
    gogOutputStream << "anglestart " << angleUnits.convertTo(originalUnits_.angleUnits(), angleStart_.value_or(0.)) << "\n";
  if (angleSweep_.has_value())
    gogOutputStream << "angledeg " << angleUnits.convertTo(originalUnits_.angleUnits(), angleSweep_.value_or(0.)) << "\n";
  simCore::Units distanceUnits(simCore::Units::METERS);
  if (majorAxis_.has_value())
    gogOutputStream << "majoraxis " << distanceUnits.convertTo(originalUnits_.rangeUnits(), majorAxis_.value_or(0.)) << "\n";
  if (minorAxis_.has_value())
    gogOutputStream << "minoraxis " << distanceUnits.convertTo(originalUnits_.rangeUnits(), minorAxis_.value_or(0.)) << "\n";
}

Arc::Arc(bool relative)
  : EllipticalShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

ShapeType Arc::shapeType() const
{
  return ShapeType::ARC;
}

Ellipse::Ellipse(bool relative)
  : EllipticalShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

ShapeType Ellipse::shapeType() const
{
  return ShapeType::ELLIPSE;
}

Cylinder::Cylinder(bool relative)
  : EllipticalShape()
{
  setCanExtrude_(false);
  setCanFollow_(true);
  setRelative_(relative);
}

ShapeType Cylinder::shapeType() const
{
  return ShapeType::CYLINDER;
}

int Cylinder::getHeight(double& height) const
{
  // default height is 1000 unitless
  height = height_.value_or(originalUnits_.altitudeUnits().convertTo(simCore::Units::METERS, 1000.));
  return (height_.has_value() ? 0 : 1);
}

void Cylinder::setHeight(double height)
{
  height_ = height;
}

void Cylinder::serializeToStream_(std::ostream& gogOutputStream) const
{
  EllipticalShape::serializeToStream_(gogOutputStream);
  if (height_.has_value())
    gogOutputStream << "height " << simCore::Units::METERS.convertTo(originalUnits_.altitudeUnits(), height_.value_or(0.)) << "\n";
}

CircularHeightShape::CircularHeightShape()
  : CircularShape()
{}

int CircularHeightShape::getHeight(double& height) const
{
  // default height is 1000 unitless
  height = height_.value_or(originalUnits_.altitudeUnits().convertTo(simCore::Units::METERS, 1000.));
  return (height_.has_value() ? 0 : 1);
}

void CircularHeightShape::setHeight(double heightMeters)
{
  height_ = heightMeters;
}

void CircularHeightShape::serializeToStream_(std::ostream& gogOutputStream) const
{
  CircularShape::serializeToStream_(gogOutputStream);
  if (height_.has_value())
    gogOutputStream << "height " << simCore::Units::METERS.convertTo(originalUnits_.altitudeUnits(), height_.value_or(0.)) << "\n";
}

Cone::Cone(bool relative)
  : CircularHeightShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

ShapeType Cone::shapeType() const
{
  return ShapeType::CONE;
}

Ellipsoid::Ellipsoid(bool relative)
  : CircularHeightShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

ShapeType Ellipsoid::shapeType() const
{
  return ShapeType::ELLIPSOID;
}

int Ellipsoid::getMajorAxis(double& axis) const
{
  axis = majorAxis_.value_or(1000.);
  return (majorAxis_.has_value() ? 0 : 1);
}

void Ellipsoid::setMajorAxis(double majorAxisMeters)
{
  majorAxis_ = majorAxisMeters;
}

int Ellipsoid::getMinorAxis(double& axis) const
{
  axis = minorAxis_.value_or(1000.);
  return (minorAxis_.has_value() ? 0 : 1);
}

void Ellipsoid::setMinorAxis(double minorAxisMeters)
{
  minorAxis_ = minorAxisMeters;
}

void Ellipsoid::serializeToStream_(std::ostream& gogOutputStream) const
{
  CircularHeightShape::serializeToStream_(gogOutputStream);
  simCore::Units distanceUnits(simCore::Units::METERS);
  if (majorAxis_.has_value())
    gogOutputStream << "majoraxis " << distanceUnits.convertTo(originalUnits_.rangeUnits(), majorAxis_.value_or(0.)) << "\n";
  if (minorAxis_.has_value())
    gogOutputStream << "minoraxis " << distanceUnits.convertTo(originalUnits_.rangeUnits(), minorAxis_.value_or(0.)) << "\n";
}

Annotation::Annotation(bool relative)
  : GogShape()
{
  setCanExtrude_(false);
  setCanFollow_(false);
  setRelative_(relative);
  setSerializeName_(false);
}

ShapeType Annotation::shapeType() const
{
  return ShapeType::ANNOTATION;
}

int Annotation::getPosition(simCore::Vec3& position) const
{
  position = position_.value_or(simCore::Vec3());
  return (position_.has_value() ? 0 : 1);
}

void Annotation::setPosition(const simCore::Vec3& position)
{
  position_ = position;
}

std::string Annotation::text() const
{
  return text_;
}

void Annotation::setText(const std::string& text)
{
  text_ = text;
}

int Annotation::getFontName(std::string& fontName) const
{
  fontName = fontName_.value_or("arial.ttf");
  return (fontName_.has_value() ? 0 : 1);
}

void Annotation::setFontName(const std::string& fontName)
{
  fontName_ = fontName;
}

int Annotation::getTextSize(int& textSize) const
{
  textSize = textSize_.value_or(15);
  return (textSize_.has_value() ? 0 : 1);
}

void Annotation::setTextSize(int textPointSize)
{
  textSize_ = textPointSize;
}

int Annotation::getTextColor(Color& color) const
{
  color = textColor_.value_or(Color());
  return (textColor_.has_value() ? 0 : 1);
}

void Annotation::setTextColor(const Color& color)
{
  textColor_ = color;
}

int Annotation::getOutlineColor(Color& color) const
{
  color = outlineColor_.value_or(Color(0, 0, 0, 255));
  return (outlineColor_.has_value() ? 0 : 1);
}

void Annotation::setOutlineColor(const Color& color)
{
  outlineColor_ = color;
}

int Annotation::getOutlineThickness(OutlineThickness& thickness) const
{
  thickness = outlineThickness_.value_or(OutlineThickness::THIN);
  return (outlineThickness_.has_value() ? 0 : 1);
}

void Annotation::setOutlineThickness(OutlineThickness thickness)
{
  outlineThickness_ = thickness;
}

int Annotation::getIconFile(std::string& iconFile) const
{
  iconFile = iconFile_.value_or("");
  return (iconFile_.has_value() ? 0 : 1);
}

void Annotation::setIconFile(const std::string& iconFile)
{
  iconFile_ = iconFile;
}

int Annotation::getPriority(double& priority) const
{
  priority = priority_.value_or(100.);
  return (priority_.has_value() ? 0 : 1);
}

void Annotation::setPriority(double priority)
{
  priority_ = priority;
}

void Annotation::serializeToStream_(std::ostream& gogOutputStream) const
{
  std::string name;
  getName(name);
  gogOutputStream << shapeTypeToString(shapeType()) << " " << name << "\n";

  if (position_.has_value())
  {
    std::vector<simCore::Vec3> positionVec;
    positionVec.push_back(*position_);
    // annotation serializes out position as lla or xyz
    serializePoints_(positionVec, gogOutputStream);
  }

  if (fontName_.has_value())
    gogOutputStream << "fontname " << fontName_.value_or("") << "\n";
  if (textSize_.has_value())
    gogOutputStream << "fontsize " << textSize_.value_or(0) << "\n";
  if (textColor_.has_value())
    gogOutputStream << "linecolor hex " << textColor_->serialize() << "\n";
  if (outlineColor_.has_value())
    gogOutputStream << "textoutlinecolor hex " << outlineColor_->serialize() << "\n";
  if (outlineThickness_.has_value())
  {
    std::string thicknessStr = "none";
    OutlineThickness thickness = OutlineThickness::NONE;
    switch (outlineThickness_.value_or(thickness))
    {
    case OutlineThickness::NONE:
      break;
    case OutlineThickness::THIN:
      thicknessStr = "thin";
      break;
    case OutlineThickness::THICK:
      thicknessStr = "thick";
    }
    gogOutputStream << "textoutlinethickness " << thicknessStr << "\n";
  }
  if (priority_.has_value())
    gogOutputStream << "priority " << priority_.value_or(0) << "\n";
}

LatLonAltBox::LatLonAltBox()
  : FillableShape(),
    north_(0.),
    south_(0.),
    east_(0.),
    west_(0.),
    altitude_(0.)
{
  setCanExtrude_(false);
  setCanFollow_(false);
  setRelative_(false);
}

ShapeType LatLonAltBox::shapeType() const
{
  return ShapeType::LATLONALTBOX;
}

double LatLonAltBox::north() const
{
  return north_;
}

void LatLonAltBox::setNorth(double northRad)
{
  north_ = northRad;
}

double LatLonAltBox::south() const
{
  return south_;
}

void LatLonAltBox::setSouth(double southRad)
{
  south_ = southRad;
}

double LatLonAltBox::east() const
{
  return east_;
}

void LatLonAltBox::setEast(double eastRad)
{
  east_ = eastRad;
}

double LatLonAltBox::west() const
{
  return west_;
}

void LatLonAltBox::setWest(double westRad)
{
  west_ = westRad;
}

double LatLonAltBox::altitude() const
{
  return altitude_;
}

void LatLonAltBox::setAltitude(double altitudeMeters)
{
  altitude_ = altitudeMeters;
}

int LatLonAltBox::getHeight(double& height) const
{
  height = height_.value_or(0.);
  return (height_.has_value() ? 0 : 1);
}

void LatLonAltBox::setHeight(double heightMeters)
{
  height_ = heightMeters;
}

void LatLonAltBox::serializeToStream_(std::ostream& gogOutputStream) const
{
  gogOutputStream << GogShape::shapeTypeToString(shapeType()) << " " << (north_ * simCore::RAD2DEG) << " " << (south_ * simCore::RAD2DEG) << " "
    << (west_ * simCore::RAD2DEG) << " " << (east_ * simCore::RAD2DEG) << " " << simCore::Units::METERS.convertTo(originalUnits_.altitudeUnits(), altitude_);
  if (height_.has_value())
    gogOutputStream << " " << simCore::Units::METERS.convertTo(originalUnits_.altitudeUnits(), altitude_ + height_.value_or(0));
  gogOutputStream << "\n";
  FillableShape::serializeToStream_(gogOutputStream);
}

ImageOverlay::ImageOverlay()
  : GogShape(),
    north_(0.),
    south_(0.),
    east_(0.),
    west_(0.),
    rotation_(0.)
{
  setCanExtrude_(false);
  setCanFollow_(false);
  setRelative_(false);
}

ShapeType ImageOverlay::shapeType() const
{
  return ShapeType::IMAGEOVERLAY;
}

double ImageOverlay::north() const
{
  return north_;
}

void ImageOverlay::setNorth(double northRad)
{
  north_ = northRad;
}

double ImageOverlay::south() const
{
  return south_;
}

void ImageOverlay::setSouth(double southRad)
{
  south_ = southRad;
}

double ImageOverlay::east() const
{
  return east_;
}

void ImageOverlay::setEast(double eastRad)
{
  east_ = eastRad;
}

double ImageOverlay::west() const
{
  return west_;
}

void ImageOverlay::setWest(double westRad)
{
  west_ = westRad;
}

double ImageOverlay::getRotation() const
{
  return rotation_;
}

void ImageOverlay::setRotation(double rotationRad)
{
  rotation_ = rotationRad;
}

std::string ImageOverlay::imageFile() const
{
  return imageFile_;
}

void ImageOverlay::setImageFile(const std::string& imageFile)
{
  imageFile_ = imageFile;
}

void ImageOverlay::serializeToStream_(std::ostream& gogOutputStream) const
{
  // no-op, serialization is handled by serializing comments in GogShape base
}

}}
