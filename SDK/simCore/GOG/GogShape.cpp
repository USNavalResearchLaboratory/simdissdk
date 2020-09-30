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
#include <cassert>
#include "simCore/Calc/Angle.h"
#include "simCore/GOG/GogShape.h"

namespace simCore { namespace GOG {

// GOG default reference origin: a location off the Pacific Missile Range Facility "BARSTUR Center"
static const simCore::Vec3 BSTUR(simCore::DEG2RAD * 22.1194392, simCore::DEG2RAD * -159.9194988, 0.0);

GogShape::GogShape()
  : canExtrude_(false),
    canFollow_(false),
    relative_(false)
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

int GogShape::getReferencePosition(simCore::Vec3& refPos) const
{
  refPos = referencePosition_.value_or(BSTUR);
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
  if (!canFollow_)
    return;
  followYaw_ = follow;
}

int GogShape::getIsFollowingPitch(bool& follow) const
{
  follow = followPitch_.value_or(false);
  return (followPitch_.has_value() ? 0 : 1);
}

void GogShape::setFollowPitch(bool follow)
{
  if (!canFollow_)
    return;
  followPitch_ = follow;
}

int GogShape::getIsFollowingRoll(bool& follow) const
{
  follow = followRoll_.value_or(false);
  return (followRoll_.has_value() ? 0 : 1);
}

void GogShape::setFollowRoll(bool follow)
{
  if (!canFollow_)
    return;
  followRoll_ = follow;
}

int GogShape::getYawOffset(double& offset) const
{
  offset = yawOffset_.value_or(0.);
  return (yawOffset_.has_value() ? 0 : 1);
}

void GogShape::setYawOffset(double offset)
{
  yawOffset_ = offset;
}

int GogShape::getPitchOffset(double& offset) const
{
  offset = pitchOffset_.value_or(0.);
  return (pitchOffset_.has_value() ? 0 : 1);
}

void GogShape::setPitchOffset(double offset)
{
  pitchOffset_ = offset;
}

int GogShape::getRollOffset(double& offset) const
{
  offset = rollOffset_.value_or(0.);
  return (rollOffset_.has_value() ? 0 : 1);
}

void GogShape::setRollOffset(double offset)
{
  rollOffset_ = offset;
}

const std::vector<std::string>& GogShape::comments() const
{
  return comments_;
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

GogShape::ShapeType GogShape::stringToShapeType(const std::string& shapeType)
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

Point::Point(bool relative)
  : OutlinedShape()
{
  setCanExtrude_(true);
  setCanFollow_(relative);
  setRelative_(relative);
}

GogShape::ShapeType Point::shapeType() const
{
  return ShapeType::POINTS;
}

const std::vector<simCore::Vec3>& Point::points() const
{
  return points_;
}

void Point::addPoint(const simCore::Vec3& point)
{
  points_.push_back(point);
}

int Point::getPointSize(int& size) const
{
  size = pointSize_.value_or(1);
  return (pointSize_.has_value() ? 0 : 1);
}

void Point::setPointSize(int pointSizePixels)
{
  pointSize_ = pointSizePixels;
}

int Point::getColor(Color& color) const
{
  color = color_.value_or(Color());
  return (color_.has_value() ? 0 : 1);
}

void Point::setColor(Color& gogColor)
{
  color_ = gogColor;
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

Line::Line(bool relative)
  : PointBasedShape(relative)
{
}

GogShape::ShapeType Line::shapeType() const
{
  return ShapeType::LINE;
}

LineSegs::LineSegs(bool relative)
  : PointBasedShape(relative)
{
}

GogShape::ShapeType LineSegs::shapeType() const
{
  return ShapeType::LINESEGS;
}

Polygon::Polygon(bool relative)
  : PointBasedShape(relative)
{
}

GogShape::ShapeType Polygon::shapeType() const
{
  return ShapeType::POLYGON;
}


CircularShape::CircularShape()
  : FillableShape()
{
  setCanFollow_(true);
}

simCore::Vec3 CircularShape::centerPosition() const
{
  return center_;
}

void CircularShape::setCenterPosition(const simCore::Vec3& centerPosition)
{
  center_ = centerPosition;
}

int CircularShape::getRadius(double& radius) const
{
  radius = radius_.value_or(500.);
  return (radius_.has_value() ? 0 : 1);
}

void CircularShape::setRadius(double radiusMeters)
{
  radius_ = radiusMeters;
}

Circle::Circle(bool relative)
  : CircularShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

GogShape::ShapeType Circle::shapeType() const
{
  return ShapeType::CIRCLE;
}

Sphere::Sphere(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

GogShape::ShapeType Sphere::shapeType() const
{
  return ShapeType::SPHERE;
}

HemiSphere::HemiSphere(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

GogShape::ShapeType HemiSphere::shapeType() const
{
  return ShapeType::HEMISPHERE;
}

Orbit::Orbit(bool relative)
  : CircularShape()
{
  setCanExtrude_(false);
  setRelative_(relative);
}

GogShape::ShapeType Orbit::shapeType() const
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
  center2_.setZ(centerPosition().z());
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

Arc::Arc(bool relative)
  : EllipticalShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

GogShape::ShapeType Arc::shapeType() const
{
  return ShapeType::ARC;
}

Ellipse::Ellipse(bool relative)
  : EllipticalShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

GogShape::ShapeType Ellipse::shapeType() const
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

GogShape::ShapeType Cylinder::shapeType() const
{
  return ShapeType::CYLINDER;
}

int Cylinder::getHeight(double& height) const
{
  height = height_.value_or(500);
  return (height_.has_value() ? 0 : 1);
}

void Cylinder::setHeight(double height)
{
  height_ = height;
}

CircularHeightShape::CircularHeightShape()
  : CircularShape()
{}

int CircularHeightShape::getHeight(double& height) const
{
  height = height_.value_or(500.);
  return (height_.has_value() ? 0 : 1);
}

void CircularHeightShape::setHeight(double heightMeters)
{
  height_ = heightMeters;
}

Cone::Cone(bool relative)
  : CircularHeightShape()
{
  setCanExtrude_(true);
  setRelative_(relative);
}

GogShape::ShapeType Cone::shapeType() const
{
  return ShapeType::CONE;
}

Ellipsoid::Ellipsoid(bool relative)
  : CircularHeightShape(),
    majorAxis_(0.),
    minorAxis_(0.)
{
  setCanExtrude_(false);
  setRelative_(relative);
}

GogShape::ShapeType Ellipsoid::shapeType() const
{
  return ShapeType::ELLIPSOID;
}

double Ellipsoid::majorAxis() const
{
  return majorAxis_;
}

void Ellipsoid::setMajorAxis(double majorAxisMeters)
{
  majorAxis_ = majorAxisMeters;
}

double Ellipsoid::minorAxis() const
{
  return minorAxis_;
}

void Ellipsoid::setMinorAxis(double minorAxisMeters)
{
  minorAxis_ = minorAxisMeters;
}

Annotation::Annotation(bool relative)
  : GogShape()
{
  setCanExtrude_(false);
  setCanFollow_(false);
  setRelative_(relative);
}

GogShape::ShapeType Annotation::shapeType() const
{
  return ShapeType::ANNOTATION;
}

simCore::Vec3 Annotation::position() const
{
  return position_;
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
  color = outlineColor_.value_or(Color());
  return (outlineColor_.has_value() ? 0 : 1);
}

void Annotation::setOutlineColor(const Color& color)
{
  outlineColor_ = color;
}

int Annotation::getOutlineThickness(OutlineThickness& thickness) const
{
  thickness = outlineThickness_.value_or(OutlineThickness::NONE);
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

LatLonAltBox::LatLonAltBox()
  : FillableShape(),
    north_(0.),
    south_(0.),
    east_(0.),
    west_(0.),
    height_(0.)
{
  setCanExtrude_(false);
  setCanFollow_(false);
  setRelative_(false);
}

GogShape::ShapeType LatLonAltBox::shapeType() const
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

double LatLonAltBox::height() const
{
  return height_;
}

void LatLonAltBox::setHeight(double heightMeters)
{
  height_ = heightMeters;
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

GogShape::ShapeType ImageOverlay::shapeType() const
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

void ImageOverlay::setImageFIle(const std::string& imageFile)
{
  imageFile_ = imageFile;
}

}}
