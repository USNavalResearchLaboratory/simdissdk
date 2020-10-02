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
#ifndef SIMCORE_GOG_GOGSHAPE_H
#define SIMCORE_GOG_GOGSHAPE_H

#include <memory>
#include <string>
#include <vector>
#include "simCore/Calc/Vec3.h"
#include "simCore/Common/Common.h"
#include "simCore/Common/Optional.h"

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
*       EllipticalShape:
*         Arc
*         Ellipse
*         Cylinder
*/

namespace simCore { namespace GOG {

/// Base class for the GOG shapes, containing common fields that apply to all shapes
class SDKCORE_EXPORT GogShape
{
public:

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
  struct Color
  {
    int red;
    int green;
    int blue;
    int alpha;

    Color(int redIn, int greenIn, int blueIn, int alphaIn) : red(redIn), green(greenIn), blue(blueIn), alpha(alphaIn) {};
    Color() : Color(255, 0, 0, 255) {}
    bool operator==(const Color& rhs) const { return red == rhs.red && green == rhs.green && blue == rhs.blue && alpha == rhs.alpha; }
  };

  virtual ~GogShape();

  /// Return true if this is a relative shape, which means all positions are xyz referenced in meters, otherwise positions are lla in radians
  bool isRelative() const;

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
  * Get reference position, lla in radians; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getReferencePosition(simCore::Vec3& refPos) const;
  /// Set reference position for relative shapes
  void setReferencePosition(const simCore::Vec3& refPos);

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

  /// Comments associated with the shape
  const std::vector<std::string>& comments() const;
  void addComment(const std::string& commment);

  /// Convert a shape type enum to a user friendly string
  static std::string shapeTypeToString(ShapeType shapeType);
  /// Convert a string representation of shape type to its equivalent enum
  static ShapeType stringToShapeType(const std::string& shapeType);

protected:
  GogShape();
  /// Set if shape supports extrusion
  void setCanExtrude_(bool canExtrude);
  /// Set if shape's orientation can be locked to a reference orientation
  void setCanFollow_(bool canFollow);
  /// Set if shape is relative or absolute
  void setRelative_(bool relative);

private:
  bool canExtrude_; ///< Indicates if shape supports extrusion
  bool canFollow_; ///< Indicates if shape's orientation can be locked to a reference orientation
  bool relative_; ///< Indicates if shape is relative coordinates (xyz meters) or absolute coordinates (lla radians)

  Optional<std::string> name_; ///< Display name
  Optional<bool> draw_; ///< Draw state
  Optional<double> altitudeOffset_; ///< offset for altitude values, meters
  Optional<bool> depthBuffer_; ///< Depth buffer active state
  Optional<AltitudeMode> altitudeMode_; ///< Defines special behavior for altitude
  Optional<simCore::Vec3> referencePosition_; ///< Reference position, only valid for relative shapes, lla radians
  Optional<simCore::Vec3> scale_; ///< Scalar adjustments for the shape's xyz components

  Optional<bool> followYaw_; ///< Yaw component locked to a reference orientation
  Optional<bool> followPitch_; ///< Pitch component locked to a reference orientation
  Optional<bool> followRoll_; ///< Roll component locked to a reference orientation
  Optional<double> yawOffset_; ///< Angle offset for yaw component, radians
  Optional<double> pitchOffset_; ///< Angle offset for pitch component, radians
  Optional<double> rollOffset_; ///< Angle offset for roll component, radians

  std::vector<std::string> comments_; ///< Comment strings for the shape

  // TODO: store original units for serialization
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

private:
  Optional<bool> outlined_; ///< outlined state of the shape
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
  void setColor(Color& gogColor);

public:
  Optional<int> pointSize_; ///< pixels
  Optional<Color> color_;
  std::vector<simCore::Vec3> points_; ///< lla radians if absolute, xyz meters if relative
};

/// Shape that supports lined and filled attributes
class SDKCORE_EXPORT FillableShape : public OutlinedShape
{
public:
  /// Defines how the line stipple is drawn
  enum class LineStyle
  {
    SOLID = 0,
    DASHED,
    DOTTED
  };

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

private:

  Optional<int> lineWidth_; ///< pixels
  Optional<Color> lineColor_;
  Optional<LineStyle> lineStyle_;
  Optional<bool> filled_; ///< filled state of the shape
  Optional<Color> fillColor_;
};

/// Shape that is defined by point positions and supports tessellation
class SDKCORE_EXPORT PointBasedShape : public FillableShape
{
public:
  /// Calculation to use when applying tessellation
  enum class TessellationStyle
  {
    NONE = 0,
    RHUMBLINE,
    GREAT_CIRCLE
  };

  /// Get the positions of points in the shape; in lla radians if absolute or xyz meters if relative
  const std::vector<simCore::Vec3>& points() const;
  /// Add a point position; in lla radians if absolute or xyz meters if relative
  void addPoint(const simCore::Vec3& point);

  /**
  * Get the shape's tessellation style; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getTessellation(TessellationStyle& tessellation) const;
  /// Set the shape's tessellation style
  void setTesssellation(TessellationStyle tessellation);

protected:
  explicit PointBasedShape(bool relative);

private:
  Optional<TessellationStyle> tessellation_; ///< defines calculation used for tessellation
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
  /// Get the shape's center position; in lla radians if absolute, xyz meters if relative
  simCore::Vec3 centerPosition() const;
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

private:
  simCore::Vec3 center_; ///< lla radians if absolute, xyz meters if relative
  Optional<double> radius_; ///< meters
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

private:
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

private:
  Optional<double> angleStart_; ///< radians
  Optional<double> angleSweep_; ///< radians
  Optional<double> majorAxis_; ///< meters
  Optional<double> minorAxis_; ///< meters
};

/// Arc shape implementation, supports elliptical arcs
class SDKCORE_EXPORT Arc : public EllipticalShape
{
public:
  explicit Arc(bool relative);

  virtual ShapeType shapeType() const;
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
  Optional<double> height_; ///< meters
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

private:
  Optional<double> height_; ///< meters
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
  Optional<double> majorAxis_; ///< meters
  Optional<double> minorAxis_; ///< meters
};

/// Annotation implementation, a text label that optionally includes an icon
class SDKCORE_EXPORT Annotation : public GogShape
{
public:
  /// Thickness style of the text outline
  enum class OutlineThickness
  {
    NONE = 0,
    THIN,
    THICK
  };

  explicit Annotation(bool relative);

  virtual ShapeType shapeType() const;

  /// Get the label's position; in lla radians if absolute, xyz meters if relative
  simCore::Vec3 position() const;
  /// Set the label's position; in lla radians if absolute, xyz meters if relative
  void setPosition(const simCore::Vec3& position);

  /// Get the display text of the annotation
  std::string text() const;
  /// Set the display text of the annotation
  void setText(const std::string& text);

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
  * Get the icon file to display; if value is not set, default value is returned.
  * @return 0 if value was set, non-zero otherwise
  */
  int getIconFile(std::string& iconFile) const;
  /// Set the icon file to display
  void setIconFile(const std::string& iconFile);

private:
  simCore::Vec3 position_; ///< lla radians if absolute, xyz meters if relative
  std::string text_; ///< display text
  Optional<std::string> fontName_; ///< font filename
  Optional<int> textSize_; ///< text point size
  Optional<Color> textColor_;
  Optional<Color> outlineColor_;
  Optional<OutlineThickness> outlineThickness_; ///< thickness style of text outline
  Optional<std::string> iconFile_; ///< icon filename
};

/// A parallel 3D or 2D box
class SDKCORE_EXPORT LatLonAltBox : public FillableShape
{
public:
  LatLonAltBox();

  virtual ShapeType shapeType() const;

  // Box north corner latitude in radians
  double north() const;
  void setNorth(double northRad);

  // Box south corner latitude in radians
  double south() const;
  void setSouth(double southRad);

  // Box east corner longitude in radians
  double east() const;
  void setEast(double eastRad);

  // Box west corner longitude in radians
  double west() const;
  void setWest(double westRad);

  // Altitude of the bottom of the box in meters
  double altitude() const;
  void setAltitude(double altitudeMeters);

  // Box optional height in meters
  int getHeight(double& height) const;
  void setHeight(double heightMeters);

private:
  double north_; ///< north corner latitude, radians
  double south_; ///< south corner latitude, radians
  double east_; ///< east corner latitude, radians
  double west_; ///< west corner latitude, radians
  double altitude_; ///< altitude of the box bottom, meters
  Optional<double> height_; ///< height of the box above the altitude, meters
};

/// Image overlay implementation, displays an image file within a specified bounding box
class SDKCORE_EXPORT ImageOverlay : public GogShape
{
public:
  ImageOverlay();

  virtual ShapeType shapeType() const;

  // Box north corner latitude in radians
  double north() const;
  void setNorth(double northRad);

  // Box south corner latitude in radians
  double south() const;
  void setSouth(double southRad);

  // Box east corner longitude in radians
  double east() const;
  void setEast(double eastRad);

  // Box west corner longitude in radians
  double west() const;
  void setWest(double westRad);

  // Rotation of the image offset from true north, in radians
  double getRotation() const;
  void setRotation(double rotation);

  // image filename
  std::string imageFile() const;
  void setImageFile(const std::string& imageFile);

private:
  double north_; ///< north corner latitude, radians
  double south_; ///< south corner latitude, radians
  double east_; ///< east corner latitude, radians
  double west_; ///< west corner latitude, radians
  double rotation_; ///< rotation angle from true north, radians
  std::string imageFile_; ///< image file to display
};

/// Define a shared ptr
typedef std::shared_ptr<GogShape> GogShapePtr;

}}

#endif /* SIMCORE_GOG_GOGSHAPE_H */
