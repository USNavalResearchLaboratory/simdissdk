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
#ifndef SIMCORE_GOG_GOGSHAPE_H
#define SIMCORE_GOG_GOGSHAPE_H

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "simCore/Calc/Vec3.h"
#include "simCore/Common/Common.h"
#include "simCore/Common/Optional.h"
#include "simCore/GOG/GogUtils.h"
#include "simCore/Time/TimeClass.h"

/**
* Class hierarchy for GOG shapes, only leaf nodes can be directly instantiated
* GogShape
*   Annotation
*   ImageOverlay
*   OutlinedShape:
*     Point
*     FillableShape:
*       LatLonAltBox
*       PointBasedShape:
*         Line
*         LineSegs
*         Polygon
*       CircularShape:
*         Circle
*         Sphere
*         Hemisphere
*         Orbit
*         CircularHeightShape:
*           Cone
*           Ellipsoid
*         EllipticalShape:
*           Arc
*           Ellipse
*           Cylinder
*/

namespace simCore { namespace GOG {

/// Defines special behavior pertaining to a shape's altitude
enum class AltitudeMode
{
  NONE = 0,
  CLAMP_TO_GROUND,
  RELATIVE_TO_GROUND,
  EXTRUDE
};

/// Shape being represented
enum class ShapeType
{
  UNKNOWN = 0,
  ANNOTATION,
  POINTS,
  LINE,
  LINESEGS,
  POLYGON,
  ARC,
  CIRCLE,
  ELLIPSE,
  ELLIPSOID,
  CYLINDER,
  SPHERE,
  HEMISPHERE,
  LATLONALTBOX,
  CONE,
  IMAGEOVERLAY,
  ORBIT
};

// Define's a GOG color's RGBA values, 0-255
struct SDKCORE_EXPORT Color
{
  int red;
  int green;
  int blue;
  int alpha;

  Color(int redIn, int greenIn, int blueIn, int alphaIn) : red(redIn), green(greenIn), blue(blueIn), alpha(alphaIn) {};
  Color() : Color(255, 0, 0, 255) {}
  bool operator==(const Color& rhs) const { return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha; }
  // serialize out in GOG format, 0xAABBGGRR
  std::string serialize() const;
};

/// Defines how the line stipple is drawn for a FillableShape
enum class LineStyle
{
  SOLID = 0,
  DASHED,
  DOTTED
};

/// Calculation to use when applying tessellation for PointBasedShape
enum class TessellationStyle
{
  NONE = 0,
  RHUMBLINE,
  GREAT_CIRCLE
};

/// Thickness style of the text outline for an Annotation
enum class OutlineThickness
{
  NONE = 0,
  THIN,
  THICK
};

/// Base class for the GOG shapes, containing common fields that apply to all shapes
class SDKCORE_EXPORT GogShape
{
public:

  virtual ~GogShape();

  /// Return true if this is a relative shape, which means all positions are xyz referenced in meters, otherwise positions are lla in radians
  bool isRelative() const;
  /// Set if shape is relative or absolute
  void setRelative(bool relative);

  /// Defines the shape type implementation
  virtual ShapeType shapeType() const = 0;

  /**
  * Get user friendly display name of shape; if value is not set, default name is returned
  * @return 0 if value was set, non-zero otherwise
  */
  int getName(std::string& name) const;
  /// Set user friendly display name of the shape
  void setName(const std::string& gogName);

  /// Draw state of the shape
  int getIsDrawn(bool& draw) const;
  void setDrawn(bool draw);

  /**
  * Get flag indicating if depth buffer is active for the shape; if value is not set, default value is returned
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsDepthBufferActive(bool& depthBuffer) const;
  /// Set depth buffer active for the shape
  void setDepthBufferActive(bool depthBuffer);

  /**
  * Get altitude offset in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getAltitudeOffset(double& altitudeOffset) const;
  /// Altitude offset to apply to shape's position, in meters; won't accept EXTRUDE if canExtrude_ is false
  void setAltitudeOffset(double altOffsetMeters);

  /**
  * Get mode that determines special behavior with regards to shape's altitude values; if value is not set, default value is returned
  * @return 0 if value was set, non-zero otherwise
  */
  int getAltitudeMode(AltitudeMode& mode) const;
  /// Set mode that determines special behavior with regards to shape's altitude values
  void setAltitudeMode(AltitudeMode mode);

  /**
  * Get the shape's extrusion height in meters, only applies if altitude mode is extrude; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getExtrudeHeight(double& height) const;
  /// Set the shape's height in meters
  void setExtrudeHeight(double heightMeters);

  /**
  * Get reference position, lla in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getReferencePosition(simCore::Vec3& refPos) const;
  /// Set reference position for relative shapes
  void setReferencePosition(const simCore::Vec3& refPos);
  /// Clear out the current reference position
  void clearReferencePosition();

  /**
  * Get the scalar adjustment values for shape's the xyz components; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getScale(simCore::Vec3& scale) const;
  /// Set the scalar adjustment values for shape's xyz components
  void setScale(const simCore::Vec3& scale);

  /**
  * Get flag indicating if shape's yaw component is locked to a reference orientation; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsFollowingYaw(bool& follow) const;
  /// Set flag indicating if shape's yaw component is locked to a reference orientation; no effect if canFollow_ is false
  void setFollowYaw(bool follow);

  /**
  * Get flag indicating if shape's pitch component is locked to a reference orientation; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsFollowingPitch(bool& follow) const;
  /// Set flag indicating if shape's pitch component is locked to a reference orientation; no effect if canFollow_ is false
  void setFollowPitch(bool follow);

  /**
  * Get flag indicating if shape's roll component is locked to a reference orientation; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsFollowingRoll(bool& follow) const;
  /// Set flag indicating if shape's roll component is locked to a reference orientation; no effect if canFollow_ is false
  void setFollowRoll(bool follow);

  /**
  * Get yaw angle offset from reference orientation in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getYawOffset(double& offset) const;
  /// Set yaw angle offset from reference orientation in radians
  void setYawOffset(double offsetRad);

  /**
  * Get pitch angle offset from reference orientation in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getPitchOffset(double& offset) const;
  /// Set pitch angle offset from reference orientation in radians
  void setPitchOffset(double offsetRad);

  /**
  * Get roll angle offset from reference orientation in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getRollOffset(double& offset) const;
  /// Set roll angle offset from reference orientation in radians
  void setRollOffset(double offsetRad);

  /**
  * Get the vertical datum string; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getVerticalDatum(std::string& verticalDatum) const;
  /// Set the vertical datum string
  void setVerticalDatum(const std::string& verticalDatum);

  /**
  * Get the start time; if value is not set, infinite time stamp is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getStartTime(simCore::TimeStamp& startTime) const;
  /// Set the start time
  void setStartTime(const simCore::TimeStamp& startTime);
  /// Clears the start time, setting it to an invalid state.
  void clearStartTime();

  /**
  * Get the end time; if value is not set, infinite time stamp is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getEndTime(simCore::TimeStamp& startTime) const;
  /// Set the end time
  void setEndTime(const simCore::TimeStamp& startTime);
  /// Clears the end time, setting it to an invalid state.
  void clearEndTime();

  /// Comments associated with the shape
  const std::vector<std::string>& comments() const;
  void addComment(const std::string& commment);

  /// Return the line number associated with this shape in the original GOG file
  size_t lineNumber() const;
  /// Set the line number of the shape in the original GOG file
  void setLineNumber(size_t lineNumber);

  /// Convert a shape type enum to a user friendly string
  static std::string shapeTypeToString(ShapeType shapeType);
  /// Convert a string representation of shape type to its equivalent enum
  static ShapeType stringToShapeType(const std::string& shapeType);

  /// Set the original units of the shape for use when serializing the shape
  void setOriginalUnits(const UnitsState& units);
  /// Retrieve the original units specified for this shape
  UnitsState originalUnits() const;

  /// Serialize the shape to the specified stream
  void serializeToStream(std::ostream& gogOutputStream) const;

protected:
  GogShape();
  /// Set if shape supports extrusion
  void setCanExtrude_(bool canExtrude);
  /// Set if shape's orientation can be locked to a reference orientation
  void setCanFollow_(bool canFollow);
  /// Set if the shape will serialize out its name as a separate line item using '3d name'
  void setSerializeName_(bool serializeName);
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const = 0;
  /// Helper method to serialize a list of positions into lla or xyz, depending on relative state
  void serializePoints_(const std::vector<simCore::Vec3>& points, std::ostream& gogOutputStream) const;

  UnitsState originalUnits_; ///< store original units for serialization

private:
  bool canExtrude_; ///< Indicates if shape supports extrusion
  bool canFollow_; ///< Indicates if shape's orientation can be locked to a reference orientation
  bool relative_; ///< Indicates if shape is relative coordinates (xyz meters) or absolute coordinates (lla radians)
  bool serializeName_; ///< Indicates if shape will serialize out its name as a separate line item using '3d name'
  size_t lineNumber_; ///< Location in original GOG file

  std::optional<std::string> name_; ///< Display name
  std::optional<bool> draw_; ///< Draw state
  std::optional<double> altitudeOffset_; ///< offset for altitude values, meters
  std::optional<bool> depthBuffer_; ///< Depth buffer active state
  std::optional<AltitudeMode> altitudeMode_; ///< Defines special behavior for altitude
  std::optional<double> extrudeHeight_; ///< Extrusion height if extruded, meters
  std::optional<simCore::Vec3> referencePosition_; ///< Reference position, only valid for relative shapes, lla radians
  std::optional<simCore::Vec3> scale_; ///< Scalar adjustments for the shape's xyz components

  std::optional<bool> followYaw_; ///< Yaw component locked to a reference orientation
  std::optional<bool> followPitch_; ///< Pitch component locked to a reference orientation
  std::optional<bool> followRoll_; ///< Roll component locked to a reference orientation
  std::optional<double> yawOffset_; ///< Angle offset for yaw component, radians
  std::optional<double> pitchOffset_; ///< Angle offset for pitch component, radians
  std::optional<double> rollOffset_; ///< Angle offset for roll component, radians

  std::optional<simCore::TimeStamp> startTime_; ///< Time to start displaying shape
  std::optional<simCore::TimeStamp> endTime_; ///< Time to stop displaying shape

  std::optional<std::string> verticalDatum_; ///< String that represents vertical datum, e.g. wgs84
  std::vector<std::string> comments_; ///< Comment strings for the shape
};

/// Shape that supports outlined state
class SDKCORE_EXPORT OutlinedShape : public GogShape
{
public:
  /**
  * Get outlined state flag; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsOutlined(bool& outlined) const;
  /// Set the shape's outlined state flag
  void setOutlined(bool outlined);

protected:
  OutlinedShape();
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:
  std::optional<bool> outlined_; ///< outlined state of the shape
};

/// Point shape implementation
class SDKCORE_EXPORT Points : public OutlinedShape
{
public:
  explicit Points(bool relative);

  virtual ShapeType shapeType() const;

  /// Get the positions of points in the shape; in lla radians if absolute or xyz meters if relative
  const std::vector<simCore::Vec3>& points() const;
  /// Add a point position; in lla radians if absolute or xyz meters if relative
  void addPoint(const simCore::Vec3& point);
  /// Clear all stored positions
  void clearPoints();

  /**
  * Get point size for all points in the shape in pixels; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getPointSize(int& size) const;
  /// Set the shape's point size in pixels
  void setPointSize(int pointSizePixels);

  /**
  * Get the shape's color; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getColor(Color& color) const;
  /// Set the shape's color
  void setColor(const Color& gogColor);

protected:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

public:
  std::optional<int> pointSize_; ///< pixels
  std::optional<Color> color_;
  std::vector<simCore::Vec3> points_; ///< lla radians if absolute, xyz meters if relative
};

/// Shape that supports lined and filled attributes
class SDKCORE_EXPORT FillableShape : public OutlinedShape
{
public:
  /**
  * Get the line width in pixels; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getLineWidth(int& lineWidth) const;
  /// Set the shape's line width in pixels
  void setLineWidth(int widthPixels);

  /**
  * Get the line color; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getLineColor(Color& color) const;
  /// Set the shape's line color
  void setLineColor(const Color& color);

  /**
  * Get the line style; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getLineStyle(LineStyle& style) const;
  /// Set the shape's line style
  void setLineStyle(LineStyle style);

  /**
  * Get the filled state; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIsFilled(bool& filled) const;
  /// Set the shape's filled state
  void setFilled(bool filled);

  /**
  * Get the fill color; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getFillColor(Color& color) const;
  /// Set the shape's fill color
  void setFillColor(const Color& color);

protected:
  FillableShape();
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:

  std::optional<int> lineWidth_; ///< pixels
  std::optional<Color> lineColor_;
  std::optional<LineStyle> lineStyle_;
  std::optional<bool> filled_; ///< filled state of the shape
  std::optional<Color> fillColor_;
};

/// Shape that is defined by point positions and supports tessellation
class SDKCORE_EXPORT PointBasedShape : public FillableShape
{
public:
  /// Get the positions of points in the shape; in lla radians if absolute or xyz meters if relative
  const std::vector<simCore::Vec3>& points() const;
  /// Add a point position; in lla radians if absolute or xyz meters if relative
  void addPoint(const simCore::Vec3& point);
  /// Clear all stored positions
  void clearPoints();

  /**
  * Get the shape's tessellation style; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getTessellation(TessellationStyle& tessellation) const;
  /// Set the shape's tessellation style
  void setTesssellation(TessellationStyle tessellation);

protected:
  explicit PointBasedShape(bool relative);
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:
  std::optional<TessellationStyle> tessellation_; ///< defines calculation used for tessellation
  std::vector<simCore::Vec3> points_; ///< lla radians if absolute, xyz meters if relative
};

/// Line shape implementation
class SDKCORE_EXPORT Line : public PointBasedShape
{
public:
  explicit Line(bool relative);

  virtual ShapeType shapeType() const;
};

/// Line segments shape implementation; point pairs represent lines, in the order they are added to the shape
class SDKCORE_EXPORT LineSegs : public PointBasedShape
{
public:
  explicit LineSegs(bool relative);

  virtual ShapeType shapeType() const;
};

/// Polygon shape implementation
class SDKCORE_EXPORT Polygon : public PointBasedShape
{
public:
  explicit Polygon(bool relative);

  virtual ShapeType shapeType() const;
};

/// Shape that supports a radius and center position
class SDKCORE_EXPORT CircularShape : public FillableShape
{
public:
  /**
  * Get the shape's center position in lla radians if absolute or xyz meters if relative; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getCenterPosition(simCore::Vec3& centerPosition) const;
  /// Set the shape's center position; in lla radians if absolute, xyz meters if relative
  void setCenterPosition(const simCore::Vec3& centerPosition);

  /**
  * Get the shape's radius in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getRadius(double& radius) const;
  /// Set the shape's radius in meters
  void setRadius(double radiusMeters);

protected:
  CircularShape();
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:
  std::optional<simCore::Vec3> center_; ///< lla radians if absolute, xyz meters if relative
  std::optional<double> radius_; ///< meters
};

/// Circle shape implementation
class SDKCORE_EXPORT Circle : public CircularShape
{
public:
  explicit Circle(bool relative);

  virtual ShapeType shapeType() const;
};

/// 3D Sphere shape implementation
class SDKCORE_EXPORT Sphere : public CircularShape
{
public:
  explicit Sphere(bool relative);

  virtual ShapeType shapeType() const;
};

/// 3D Hemisphere shape implementation
class SDKCORE_EXPORT Hemisphere : public CircularShape
{
public:
  explicit Hemisphere(bool relative);

  virtual ShapeType shapeType() const;
};

/// Orbit shape implementation
class SDKCORE_EXPORT Orbit : public CircularShape
{
public:
  explicit Orbit(bool relative);

  virtual ShapeType shapeType() const;

  /// Get the orbit's second center position; in lla radians if absolute, xyz meters if relative
  simCore::Vec3 centerPosition2() const;
  /// Set the orbit's second center position, ignores z value in favor of z from first center position;  in lla radians if absolute, xyz meters if relative
  void setCenterPosition2(const simCore::Vec3& center2);

  /**
   * Helper function to create XYZ (meters) orbit geometry from the specified parameters.
   * @param azimuthRad Azimuth from the first center position to second center position. If the orbit is
   *   in LLA (absolute), simCore::sodanoInverse() can help with this. If in relative, atan(xd / yd) can
   *   provide this value. This is in radians.
   * @param lengthM Length from one center position to another, in meters. For absolute LLA coordinates,
   *   simCore::sodanoInverse() can also provide this. For relative coordinates, this is equivalent to
   *   the distance formula (sqrt(xd*xd + yd*yd)).
   * @param radiusM Radius of the orbit ends in meters. Must be > 0.
   * @param altitudeM Altitude of the output relative coordinates. The resulting coordinates will have this value.
   * @param segmentLenM Length of orbit end segments, in meters. A good typical value is radiusM / 8.
   * @param xyz Output vector that will hold the XYZ points defining the orbit shape relative to the origin.
   *   This results in a closed shape, i.e. xyz.front() == xyz.back()
   */
  static void createOrbitShape(double azimuthRad, double lengthM, double radiusM, double altitudeM, double segmentLenM, std::vector<simCore::Vec3>& xyz);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  simCore::Vec3 center2_; ///< lla radians if absolute, xyz meters if relative
};

/// Shape implementation that supports major and minor axis as well as a start angle and sweep
class SDKCORE_EXPORT EllipticalShape : public CircularShape
{
public:
  /**
  * Get the shape's start angle in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getAngleStart(double& angle) const;
  /// Set the start angle in radians
  void setAngleStart(double angleStartRad);

  /**
  * Get the shape's angle sweep in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getAngleSweep(double& angle) const;
  /// Set the shape's angle sweep in radians
  void setAngleSweep(double angleSweepRad);

  /**
  * Get the shape's major axis in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getMajorAxis(double& axis) const;
  /// Set the shape's major axis in meters
  void setMajorAxis(double majorAxisMeters);

  /**
  * Get the shape's major axis in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getMinorAxis(double& axis) const;
  /// Set the shape's minor axis in meters
  void setMinorAxis(double minorAxisMeters);

protected:
  EllipticalShape();
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:
  std::optional<double> angleStart_; ///< radians
  std::optional<double> angleSweep_; ///< radians
  std::optional<double> majorAxis_; ///< meters
  std::optional<double> minorAxis_; ///< meters
};

/// Arc shape implementation, supports elliptical arcs
class SDKCORE_EXPORT Arc : public EllipticalShape
{
public:
  explicit Arc(bool relative);

  virtual ShapeType shapeType() const;

  /**
 * Get the shape's inner radius in meters; if value is not set, default value is returned.
 * @return 0 if value was set, non-zero otherwise
 */
  int getInnerRadius(double& innerRadius) const;
  /// Set the shape's inner radius in meters
  void setInnerRadius(double innerRadius);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  std::optional<double> innerRadius_; ///< meters

};

/// Ellipse shape implementation
class SDKCORE_EXPORT Ellipse : public EllipticalShape
{
public:
  explicit Ellipse(bool relative);

  virtual ShapeType shapeType() const;
};

/// Cylinder shape implementation, supports elliptical cylinders and wedges
class SDKCORE_EXPORT Cylinder : public EllipticalShape
{
public:
  explicit Cylinder(bool relative);

  virtual ShapeType shapeType() const;

  /**
  * Get the shape's height in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getHeight(double& height) const;
  /// Set the shape's height in meters
  void setHeight(double heightMeters);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  std::optional<double> height_; ///< meters
};

/// Shape that supports a height as well as center position and radius
class SDKCORE_EXPORT CircularHeightShape : public CircularShape
{
public:
  /**
  * Get the shape's height in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getHeight(double& height) const;
  /// Set the shape's height in meters
  void setHeight(double heightMeters);

protected:
  CircularHeightShape();
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

private:
  std::optional<double> height_; ///< meters
};

/// Cone shape implementation
class SDKCORE_EXPORT Cone : public CircularHeightShape
{
public:
  explicit Cone(bool relative);

  virtual ShapeType shapeType() const;
};

/// 3D Ellipsoid shape implementation
class SDKCORE_EXPORT Ellipsoid : public CircularHeightShape
{
public:
  explicit Ellipsoid(bool relative);

  virtual ShapeType shapeType() const;

  /**
  * Get the shape's major axis in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getMajorAxis(double& axis) const;
  /// Set the shape's major axis in meters
  void setMajorAxis(double majorAxisMeters);

  /**
  * Get the shape's major axis in meters; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getMinorAxis(double& axis) const;
  /// Set the shape's minor axis in meters
  void setMinorAxis(double minorAxisMeters);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  std::optional<double> majorAxis_; ///< meters
  std::optional<double> minorAxis_; ///< meters
};

/// Annotation implementation, a text label that optionally includes an icon
class SDKCORE_EXPORT Annotation : public GogShape
{
public:
  explicit Annotation(bool relative);

  virtual ShapeType shapeType() const;

  /// Get the display text of the annotation
  std::string text() const;
  /// Set the display text of the annotation
  void setText(const std::string& text);

  /**
  * Get the shape's position; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getPosition(simCore::Vec3& position) const;
  /// Set the label's position; in lla radians if absolute, xyz meters if relative
  void setPosition(const simCore::Vec3& position);

  /**
  * Get the font filename; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getFontName(std::string& fontName) const;
  /// Set the font filename
  void setFontName(const std::string& fontName);

  /**
  * Get the text point size; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getTextSize(int& size) const;
  /// Set the text point size
  void setTextSize(int textPointSize);

  /**
  * Get the text color; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getTextColor(Color& color) const;
  /// Set the text color
  void setTextColor(const Color& color);

  /**
  * Get the text outline color; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getOutlineColor(Color& color) const;
  /// Set the text outline color
  void setOutlineColor(const Color& color);

  /**
  * Get the text outline thickness style; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getOutlineThickness(OutlineThickness& thickness) const;
  /// Set the text outline thickness style
  void setOutlineThickness(OutlineThickness thickness);

  /**
  * Get the image file to display; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getImageFile(std::string& imageFile) const;
  /// Set the image file to display
  void setImageFile(const std::string& imageFile);

  /**
  * Get the text deconfliction priority value; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getPriority(double& priority) const;
  /// Set the text deconfliction prority value
  void setPriority(double priority);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  std::string text_; ///< display text
  std::optional<simCore::Vec3> position_; ///< lla radians if absolute, xyz meters if relative
  std::optional<std::string> fontName_; ///< font filename
  std::optional<int> textSize_; ///< text point size
  std::optional<Color> textColor_;
  std::optional<Color> outlineColor_;
  std::optional<OutlineThickness> outlineThickness_; ///< thickness style of text outline
  std::optional<std::string> imageFile_; ///< image filename
  std::optional<double> priority_; ///< priority of the annotation text display
};

/// A parallel 3D or 2D box
class SDKCORE_EXPORT LatLonAltBox : public FillableShape
{
public:
  LatLonAltBox();

  virtual ShapeType shapeType() const;

  /// Box north corner latitude in radians
  double north() const;
  void setNorth(double northRad);

  /// Box south corner latitude in radians
  double south() const;
  void setSouth(double southRad);

  /// Box east corner longitude in radians
  double east() const;
  void setEast(double eastRad);

  /// Box west corner longitude in radians
  double west() const;
  void setWest(double westRad);

  /// Altitude of the bottom of the box in meters
  double altitude() const;
  void setAltitude(double altitudeMeters);

  /// Box optional height in meters
  int getHeight(double& height) const;
  void setHeight(double heightMeters);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  double north_; ///< north corner latitude, radians
  double south_; ///< south corner latitude, radians
  double east_; ///< east corner latitude, radians
  double west_; ///< west corner latitude, radians
  double altitude_; ///< altitude of the box bottom, meters
  std::optional<double> height_; ///< height of the box above the altitude, meters
};

/// Image overlay implementation, displays an image file within a specified bounding box
class SDKCORE_EXPORT ImageOverlay : public GogShape
{
public:
  ImageOverlay();

  virtual ShapeType shapeType() const;

  /// Box north corner latitude in radians
  double north() const;
  void setNorth(double northRad);

  /// Box south corner latitude in radians
  double south() const;
  void setSouth(double southRad);

  /// Box east corner longitude in radians
  double east() const;
  void setEast(double eastRad);

  /// Box west corner longitude in radians
  double west() const;
  void setWest(double westRad);

  /// Rotation of the image offset counterclockwise from true north (matching KML GroundOverlay), in radians
  double getRotation() const;
  void setRotation(double rotation);

  /// image filename
  std::string imageFile() const;
  void setImageFile(const std::string& imageFile);

  /// opacity value for image (0.0 transparent, 1.0 opaque)
  int getOpacity(double& opacity) const;
  void setOpacity(double opacity);

private:
  /// Serialize the shape's specific implementation attributes to the stream
  virtual void serializeToStream_(std::ostream& gogOutputStream) const;

  double north_; ///< north corner latitude, radians
  double south_; ///< south corner latitude, radians
  double east_; ///< east corner latitude, radians
  double west_; ///< west corner latitude, radians
  double rotation_; ///< rotation angle from true north, radians
  std::string imageFile_; ///< image file to display
  std::optional<double> opacity_; ///< 0.0 (transparent) to 1.0 (opaque)
};

/// Define a shared ptr
typedef std::shared_ptr<GogShape> GogShapePtr;

}}

#endif /* SIMCORE_GOG_GOGSHAPE_H */
