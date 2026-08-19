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
#ifndef SIMDATA_COMMON_PREFERENCES_H
#define SIMDATA_COMMON_PREFERENCES_H

#include <memory>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"
#include "simData/DataTypeBasics.h"

namespace simData
{

/**
 * CommonPreferences.h, EntityPreferences.h, and DataTypeProperties.h define the underlying data format for SIMDIS SDK's simData module.
 *
 * Entity types include platforms, beams, gates, lasers, projectors, line of bearing (LOB) groups, and Custom Rendering
 * Entities are generally broken up into 3 components:
 *
 *   Properties - Settings for an entity, such as ID that are mostly unchanging
 *   Preferences - Changeable settings for an entity, including PrefsTool-like settings;
 *                 can be time stamped by using Commands
 *   Commands - A time stamp and a (likely) sparse list of preferences to apply at the time
 *
 * Unless otherwise specified, all units are in metric: radians for angles, meters for distance,
 * and meters per second for speed values.
 *
 * Notes on Draw / dataDraw flags: Draw is a user preference. This is equivalent to the Draw setting
 * in PrefsTool. The dataDraw flag is a data setting that is managed through the data provider and
 * is similar to the On/Off settings in SIMDIS 9. The dataDraw flag is hierarchical -- a gate is not
 * drawn if its beam's dataDraw is off, and even if the gate's dataDraw and draw flags are true. The draw flag is not
 * hierarchical.  An entity is only drawn if its dataDraw is on, its host is active, and its draw flag is on.
 * The dataDraw flag is not set for, and is completely ignored by Platforms.
 * Beams and gates still require that their platform host be active, but this is signaled by other means than the dataDraw flag.
 */

/// Define the text outline style
enum class TextOutline {
  TO_NONE = 0,
  TO_THIN,
  TO_THICK
};

/// Define OSG text backdrop types
enum class BackdropType {
  BDT_SHADOW_BOTTOM_RIGHT = 0,
  BDT_SHADOW_CENTER_RIGHT,
  BDT_SHADOW_TOP_RIGHT,
  BDT_SHADOW_BOTTOM_CENTER,
  BDT_SHADOW_TOP_CENTER,
  BDT_SHADOW_BOTTOM_LEFT,
  BDT_SHADOW_CENTER_LEFT,
  BDT_SHADOW_TOP_LEFT,
  BDT_OUTLINE,
  BDT_NONE
};

/// Define OSG text backdrop implementation
enum class BackdropImplementation {
  BDI_POLYGON_OFFSET = 0,
  BDI_NO_DEPTH_BUFFER,
  BDI_DEPTH_RANGE,
  BDI_STENCIL_BUFFER,
  BDI_DELAYED_DEPTH_WRITES,
};

/// Define OSG text alignment
enum class TextAlignment {
  ALIGN_LEFT_TOP = 0,
  ALIGN_LEFT_CENTER,
  ALIGN_LEFT_BOTTOM,
  ALIGN_CENTER_TOP,
  ALIGN_CENTER_CENTER,
  ALIGN_CENTER_BOTTOM,
  ALIGN_RIGHT_TOP,
  ALIGN_RIGHT_CENTER,
  ALIGN_RIGHT_BOTTOM
};

/// Elapsed time formats supported in the SDK.  Elapsed time is a relative measure of time from some epoch.
enum class ElapsedTimeFormat {
  ELAPSED_SECONDS = 1,  ///< SS.sssss
  ELAPSED_MINUTES = 2,  ///< MM:SS.sssss
  ELAPSED_HOURS = 3     ///< HH:MM:SS.sssss
};

/// Units for angular measurement
enum class AngleUnits {
  UNITS_RADIANS = 10,                  ///< Radians flag
  UNITS_DEGREES = 11,                  ///< Degrees flag
  UNITS_DEGREES_MINUTES = 12,          ///< DM format
  UNITS_DEGREES_MINUTES_SECONDS = 13,  ///< DMS format
  UNITS_UTM = 14,                      ///< UTM flag
  UNITS_BAM = 15,                      ///< BAM (Binary Angular Measure) flag
  UNITS_MIL = 16,                      ///< Angular Mil (NATO variant) flag
  UNITS_MILLIRADIANS = 17              ///< Milliradians (0.001 radians) flag
};

/// Units for geodetic angular measurement
enum class GeodeticUnits {
  GEODETIC_DEGREES = 11,                  ///< Degrees flag
  GEODETIC_DEGREES_MINUTES = 12,          ///< DM format
  GEODETIC_DEGREES_MINUTES_SECONDS = 13   ///< DMS format
};

/// Units for distance measurement
enum class DistanceUnits {
  UNITS_METERS = 20,         ///< Meters constant flag
  UNITS_KILOMETERS = 21,     ///< Kilometers constant flag
  UNITS_YARDS = 22,          ///< Yards constant flag
  UNITS_MILES = 23,          ///< Statute Miles constant flag
  UNITS_FEET = 24,           ///< Feet constant flag
  UNITS_INCHES = 25,         ///< Inches constant flag
  UNITS_NAUTICAL_MILES = 26, ///< Nautical Miles constant flag
  UNITS_CENTIMETERS = 27,    ///< Centimeters constant flag
  UNITS_MILLIMETERS = 28,    ///< Millimeters constant flag
  UNITS_KILOYARDS = 29,      ///< Kiloyards constant flag
  UNITS_DATAMILES = 30,      ///< Data Miles constant flag
  UNITS_FATHOMS = 31,        ///< Fathoms constant flag
  UNITS_KILOFEET = 32        ///< Kilofeet constant flag (equal to 1000 feet)
};


/// Units for speed measurement.
enum class SpeedUnits {
  UNITS_METERS_PER_SECOND = 40,     ///< m/s constant flag
  UNITS_KILOMETERS_PER_HOUR = 41,   ///< km/h constant flag
  UNITS_KNOTS = 42,                 ///< knots constant flag
  UNITS_MILES_PER_HOUR = 43,        ///< mph constant flag
  UNITS_FEET_PER_SECOND = 44,       ///< ft/sec constant flag
  // Note: Index 45 is reserved and not for public use.
  UNITS_KILOMETERS_PER_SECOND = 46, ///< km/s constant flag
  UNITS_DATAMILES_PER_HOUR = 47,    ///< Data miles per hour constant flag
  UNITS_YARDS_PER_SECOND = 48       ///< Yds per second constant flag
};


/// Enumeration of antenna/rcs polarizations
enum class Polarity {
  POL_UNKNOWN = 0,     ///< Unknown polarity
  POL_HORIZONTAL = 1,  ///< Horizontal RCV and XMT polarity
  POL_VERTICAL = 2,    ///< Vertical RCV and XMT polarity
  POL_CIRCULAR = 3,    ///< Circular RCV and XMT polarity
  POL_HORZVERT = 4,    ///< Horizontal RCV and vertical XMT polarity
  POL_VERTHORZ = 5,    ///< Vertical RCV and horizontal XMT polarity
  POL_LEFTCIRC = 6,    ///< Left circular RCV and XMT polarity
  POL_RIGHTCIRC = 7,   ///< Right circular RCV and XMT polarity
  POL_LINEAR = 8       ///< Linear RCV and XMT polarity
};

/// Polygon draw style for models; can draw filled (solid), lines (wireframe), or points
enum class ModelDrawMode {
  MDM_SOLID = 0, ///< polygons are filled by surface textures
  MDM_WIRE = 1, ///< model is shown as a wireframe (polygons are lines)
  MDM_POINTS = 2  ///< a cloud of points (no lines between the points of the polygons)
};

/**
 * Settings for how to orient an image icon relative to the eye.  This option is unused unless an
 * image icon (PNG, JPG, BMP, etc.) is loaded for the platform model.
 */
enum class IconRotation {

  /**
   * Billboard Pointing Up -- orient the icon so that "up is up" and the full icon always faces viewer
   */
  IR_2D_UP = 0,

  /**
   * Billboard with Yaw -- Orient icon based on yaw of platform it represents.  Icon will always be oriented
   * so that it faces the viewer, but can rotate about the center to reflect platform yaw.
   */
  IR_2D_YAW = 1,

  /**
   * Follow Platform -- orient icon using the yaw, pitch, and roll of the platform it represents.
   */
  IR_3D_YPR = 2,

  /**
   * Flat Oriented North -- Orient the icon flat (zero pitch) with respect to the Earth's surface.  The
   * orientation of the icon will always be facing North.
   */
  IR_3D_NORTH = 3,

  /**
   * Flat with Yaw -- Orient the icon flat (zero pitch) with respect to the Earth's surface.  Orientation
   * of the icon will be rotated based on the yaw of the platform it represents.
   */
  IR_3D_YAW = 4
};

class SDKDATA_EXPORT DisplayFields : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(DisplayFields);
  /// Toggle display of X or Latitude value in the label (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(xLat_, xlat, bool);
  /// Toggle display of Y or Longitude value in the label (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(yLon_, ylon, bool);
  /// Toggle display of Z or Altitude value in the label (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(zAlt_, zalt, bool);
  /// Toggle all generic data
  SIMDATA_DECLARE_FIELD(genericData_, genericdata, bool);
  /// Toggle all category data on and off
  SIMDATA_DECLARE_FIELD(categoryData_, categorydata, bool);
  /// Toggle Platform Yaw or Psi (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(yaw_, yaw, bool);
  /// Toggle Platform Pitch or Theta (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(pitch_, pitch, bool);
  /// Toggle Platform Roll or Phi (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(roll_, roll, bool);
  /// Toggle course on/off for platform (velocity-based azimuth)
  SIMDATA_DECLARE_FIELD(course_, course, bool);
  /// Toggle flight path angle on/off for platform (velocity-based elevation)
  SIMDATA_DECLARE_FIELD(flightPathElevation_, flightpathelevation, bool);
  /// Toggle velocity X component (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(displayVX_, displayvx, bool);
  /// Toggle velocity Y component (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(displayVY_, displayvy, bool);
  /// Toggle velocity Z component (based on coordinateSystem)
  SIMDATA_DECLARE_FIELD(displayVZ_, displayvz, bool);
  /// Toggle speed component
  SIMDATA_DECLARE_FIELD(speed_, speed, bool);
  /// Toggle mach component
  SIMDATA_DECLARE_FIELD(mach_, mach, bool);
  /// Toggle angle of attack (alpha) component; roll affects calculation if applyRoll set
  SIMDATA_DECLARE_FIELD(angleOfAttack_, angleofattack, bool);
  /// Toggle side slip (beta) component; roll affects calculation if applyRoll is set
  SIMDATA_DECLARE_FIELD(sideSlip_, sideslip, bool);
  /// Toggle total angle of attack
  SIMDATA_DECLARE_FIELD(totalAngleOfAttack_, totalangleofattack, bool);
  /// Toggle azimuth angle to sun
  SIMDATA_DECLARE_FIELD(solarAzimuth_, solarazimuth, bool);
  /// Toggle elevation angle to the sun
  SIMDATA_DECLARE_FIELD(solarElevation_, solarelevation, bool);
  /// Toggle solar illuminance value
  SIMDATA_DECLARE_FIELD(solarIlluminance_, solarilluminance, bool);
  /// Toggle azimuth angle to moon
  SIMDATA_DECLARE_FIELD(lunarAzimuth_, lunarazimuth, bool);
  /// Toggle elevation angle to the moon
  SIMDATA_DECLARE_FIELD(lunarElevation_, lunarelevation, bool);
  /// Toggle illuminance value from moon
  SIMDATA_DECLARE_FIELD(lunarIlluminance_, lunarilluminance, bool);
  /// Toggle late value which is seconds since last point
  SIMDATA_DECLARE_FIELD(late_, late, bool);
  /// Flag to use embedded scripting to generate the label text
  SIMDATA_DECLARE_FIELD(useLabelCode_, uselabelcode, bool);
  /// Embedded scripting code to generate the label
  SIMDATA_DECLARE_FIELD_CONST_REF(labelCode_, labelcode, std::string);
};

/// Preferences which all labels share
class SDKDATA_EXPORT LabelPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LabelPrefs);
  /// Display contents of the label
  SIMDATA_DECLARE_SUBFIELD_LIST(displayFields_, displayfields, simData::DisplayFields);
  /// Display contents of the legend
  SIMDATA_DECLARE_SUBFIELD_LIST(legendDisplayFields_, legenddisplayfields, simData::DisplayFields);
  /// Display contents of the mouse-over/hover text popup
  SIMDATA_DECLARE_SUBFIELD_LIST(hoverDisplayFields_, hoverdisplayfields, simData::DisplayFields);
  /// Display contents of the hook window
  SIMDATA_DECLARE_SUBFIELD_LIST(hookDisplayFields_, hookdisplayfields, simData::DisplayFields);
  /// Toggles display of the label
  SIMDATA_DECLARE_FIELD(draw_, draw, bool);
  /// Label color
  SIMDATA_DECLARE_FIELD(color_, color, uint32_t);
  SIMDATA_DECLARE_FIELD(textOutline_, textoutline, simData::TextOutline);
  SIMDATA_DECLARE_FIELD(outlineColor_, outlinecolor, uint32_t);
  SIMDATA_DECLARE_FIELD(backdropType_, backdroptype, simData::BackdropType);
  SIMDATA_DECLARE_FIELD(backdropImplementation_, backdropimplementation, simData::BackdropImplementation);
  SIMDATA_DECLARE_FIELD_CONST_REF(overlayFontName_, overlayfontname, std::string);
  SIMDATA_DECLARE_FIELD(overlayFontPointSize_, overlayfontpointsize, uint32_t);
  SIMDATA_DECLARE_FIELD(offsetX_, offsetx, int32_t);
  SIMDATA_DECLARE_FIELD(offsetY_, offsety, int32_t);
  SIMDATA_DECLARE_FIELD(alignment_, alignment, simData::TextAlignment);
  /// Positive high values have precedence over positive low values; Negative values are always shown
  SIMDATA_DECLARE_FIELD(priority_, priority, double);
  /// Toggle height above terrain for LLA altitude
  SIMDATA_DECLARE_FIELD(applyHeightAboveTerrain_, applyheightaboveterrain, bool);
  /// Toggle use of roll in AoA and Side slip calculations
  SIMDATA_DECLARE_FIELD(applyRoll_, applyroll, bool);
  /// Earth model to use for label calculations
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  /// Vertical datum to apply to altitude values for display
  SIMDATA_DECLARE_FIELD(verticalDatum_, verticaldatum, simData::VerticalDatum);
  /// Magnetic variance to apply to horizontal angle calculations for display
  SIMDATA_DECLARE_FIELD(magneticVariance_, magneticvariance, simData::MagneticVariance);
  /// Distance units for distance based label values
  SIMDATA_DECLARE_FIELD(distanceUnits_, distanceunits, simData::DistanceUnits);
  /// Angle units for angle based label values
  SIMDATA_DECLARE_FIELD(angleUnits_, angleunits, simData::AngleUnits);
  /// Speed units are only used in platform calculations
  SIMDATA_DECLARE_FIELD(speedUnits_, speedunits, simData::SpeedUnits);
  /// number of digits of precision in numerical displays but can be overridden by the fields below
  SIMDATA_DECLARE_FIELD(precision_, precision, int32_t);
  /// Maximum length of the name (character limit)
  SIMDATA_DECLARE_FIELD(nameLength_, namelength, int32_t);
  /// Units for display LLA angles, Degrees, Degrees-Minutes, Degrees-Minutes-Seconds
  SIMDATA_DECLARE_FIELD(geodeticUnits_, geodeticunits, simData::GeodeticUnits);
  /// Units for displaying altitude
  SIMDATA_DECLARE_FIELD(altitudeUnits_, altitudeunits, simData::DistanceUnits);
  /// If set overrides the precision field and sets the precision for distance
  SIMDATA_DECLARE_FIELD(distancePrecision_, distanceprecision, int32_t);
  /// If set overrides the precision field and sets the precision for angle
  SIMDATA_DECLARE_FIELD(anglePrecision_, angleprecision, int32_t);
  /// If set overrides the precision field and sets the precision for speed
  SIMDATA_DECLARE_FIELD(speedPrecision_, speedprecision, int32_t);
  /// If set overrides the precision field and sets the precision for geodetic
  SIMDATA_DECLARE_FIELD(geodeticPrecision_, geodeticprecision, int32_t);
  /// If set overrides the precision field and sets the precision for altitude
  SIMDATA_DECLARE_FIELD(altitudePrecision_, altitudeprecision, int32_t);
  /// If set overrides the precision field and sets the precision for time
  SIMDATA_DECLARE_FIELD(timePrecision_, timeprecision, int32_t);

public:
  enum class UseValue {
    ACTUAL_VALUE = 0, ///< labels use actual data value
    DISPLAY_VALUE = 1 ///< labels use display value, which may include adjustments like clamping and offsets
  };

  /// Indicates which value to use in labels
  SIMDATA_DECLARE_FIELD(useValues_, usevalues, UseValue);
};

/// Settings for SPEED_RINGS mode
class SDKDATA_EXPORT SpeedRing : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(SpeedRing);
  /// Use fixedTime below, instead of using radius (which is a relative time)
  SIMDATA_DECLARE_FIELD(useFixedTime_, usefixedtime, bool);
  /// This ordinal-format time specifies speed rings extent for speed rings
  SIMDATA_DECLARE_FIELD_CONST_REF(fixedTime_, fixedtime, std::string);
  /// User-specified units for the speed rings radius value and for speed ring labels
  SIMDATA_DECLARE_FIELD(timeFormat_, timeformat, simData::ElapsedTimeFormat);
  /// If !useFixedTime, this value specifies speed rings extent for speed rings, in units specified by timeFormat above
  SIMDATA_DECLARE_FIELD(radius_, radius, double);
  /// Use platform's speed for speed rings calculations
  SIMDATA_DECLARE_FIELD(usePlatformSpeed_, useplatformspeed, bool);
  /// If !usePlatformSpeed, use this user-specified speed for speed rings calculations
  SIMDATA_DECLARE_FIELD(speedToUse_, speedtouse, double);
  /// When set, speed rings labels display time, rather than distance
  SIMDATA_DECLARE_FIELD(displayTime_, displaytime, bool);
  /// User-specified units for speedToUse, determines conversion of speedToUse to m/s
  SIMDATA_DECLARE_FIELD(speedUnits_, speedunits, simData::SpeedUnits);
};

/// Grid division settings (all modes)
class SDKDATA_EXPORT GridSettings : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(GridSettings);
  /// Bold, thick lines spaced evenly out to grid size
  SIMDATA_DECLARE_FIELD(numDivisions_, numdivisions, uint32_t);
  /// Lighter lines placed in between divisions
  SIMDATA_DECLARE_FIELD(numSubDivisions_, numsubdivisions, uint32_t);
  /// For polar plots and speed rings
  SIMDATA_DECLARE_FIELD(sectorAngle_, sectorangle, double);
};

/// Position in 3D space
class SDKDATA_EXPORT Position : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(Position);
  SIMDATA_DECLARE_FIELD(x_, x, double);
  SIMDATA_DECLARE_FIELD(y_, y, double);
  SIMDATA_DECLARE_FIELD(z_, z, double);
};

/// Alignment of a body in 3D space, angles in radians; body centric
class SDKDATA_EXPORT BodyOrientation : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(BodyOrientation);
  SIMDATA_DECLARE_FIELD(yaw_, yaw, double);
  SIMDATA_DECLARE_FIELD(pitch_, pitch, double);
  SIMDATA_DECLARE_FIELD(roll_, roll, double);
};

/// A grid attached to an entity
class SDKDATA_EXPORT LocalGridPrefs : public FieldList
{
public:
  /// Shape of local grid
  enum class Type {
    CARTESIAN = 1,   ///< a square grid
    POLAR = 2,       ///< circles drawn at each range subdivision, with lines sectoring
    RANGE_RINGS = 3, ///< polar, with no sector lines
    SPEED_RINGS = 4, ///< polar rings, where the size is based on speed * time
    SPEED_LINE = 5   ///< a line drawn out to speed * time
  };

  SIMDATA_DECLARE_METHODS(LocalGridPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(speedRing_, speedring, simData::SpeedRing);
  SIMDATA_DECLARE_SUBFIELD_LIST(gridSettings_, gridsettings, simData::GridSettings);
  /// x/y/z offset in units as described below
  SIMDATA_DECLARE_SUBFIELD_LIST(gridPositionOffset_, gridpositionoffset, simData::Position);
  /// Yaw/pitch/roll offset in radians
  SIMDATA_DECLARE_SUBFIELD_LIST(gridOrientationOffset_, gridorientationoffset, simData::BodyOrientation);
  /// Draw mode
  SIMDATA_DECLARE_FIELD(gridType_, gridtype, simData::LocalGridPrefs::Type);
  /// Is the grid label shown in the display
  SIMDATA_DECLARE_FIELD(gridLabelDraw_, gridlabeldraw, bool);
  /// Color of the grid label text (default: yellow)
  SIMDATA_DECLARE_FIELD(gridLabelColor_, gridlabelcolor, uint32_t);
  /// Style of the grid label text outline
  SIMDATA_DECLARE_FIELD(gridLabelTextOutline_, gridlabeltextoutline, simData::TextOutline);
  /// Color of the grid label text outline (default: black)
  SIMDATA_DECLARE_FIELD(gridLabelOutlineColor_, gridlabeloutlinecolor, uint32_t);
  /// Font file name of the grid label text
  SIMDATA_DECLARE_FIELD_CONST_REF(gridLabelFontName_, gridlabelfontname, std::string);
  /// Font point size of the grid label text
  SIMDATA_DECLARE_FIELD(gridLabelFontSize_, gridlabelfontsize, uint32_t);
  /// Precision to use for grid label text
  SIMDATA_DECLARE_FIELD(gridLabelPrecision_, gridlabelprecision, int32_t);
  /// Is the grid shown in the display
  SIMDATA_DECLARE_FIELD(drawGrid_, drawgrid, bool);
  /// Color for lines (text has its own color settings)(default: yellow)
  SIMDATA_DECLARE_FIELD(gridColor_, gridcolor, uint32_t);
  /// Total grid size (full diameter of the local grid, in sizeUnits)
  SIMDATA_DECLARE_FIELD(size_, size, double);
  SIMDATA_DECLARE_FIELD(positionOffsetUnits_, positionoffsetunits, simData::DistanceUnits);
  SIMDATA_DECLARE_FIELD(followYaw_, followyaw, bool);
  SIMDATA_DECLARE_FIELD(followPitch_, followpitch, bool);
  SIMDATA_DECLARE_FIELD(followRoll_, followroll, bool);
  /// Units of the local grid size value, and of grid labels
  SIMDATA_DECLARE_FIELD(sizeUnits_, sizeunits, simData::DistanceUnits);
};

/// Preferences shared by all entity types
class SDKDATA_EXPORT CommonPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CommonPrefs);
  /// Points to label preferences, includes alias settings
  SIMDATA_DECLARE_SUBFIELD_LIST(labelPrefs_, labelprefs, LabelPrefs);
  /// Controls the local grid
  SIMDATA_DECLARE_SUBFIELD_LIST(localGrid_, localgrid, LocalGridPrefs);
  /**
   * Indicates that the entity is marked for drawing in the data stream.  This is intended as an internal
   * flag that is supposed to match up with items like a BeamOnOffCmd.  A user may alter the "draw" flag
   * to turn an object on or off, but if the data overrides the draw from the user, the item is still not
   * drawn.  In other words, an entity that is off in data cannot be forced to draw by the end user.
   */
  SIMDATA_DECLARE_FIELD(dataDraw_, datadraw, bool);
  /**
   * User-controlled preference for drawing or not drawing an entity.  An entity is only drawn if this flag
   * is turned on, and the dataDraw flag is also enabled.  This is a more-or-less public flag that corresponds
   * to the Draw flag in Super Form.
   */
  SIMDATA_DECLARE_FIELD(draw_, draw, bool);
  /// Entity name string
  SIMDATA_DECLARE_FIELD_CONST_REF(name_, name, std::string);
  /// Indicates whether alias should be used over proper name
  SIMDATA_DECLARE_FIELD(useAlias_, usealias, bool);
  /// Optional secondary name that can be swapped in and out
  SIMDATA_DECLARE_FIELD_CONST_REF(alias_, alias, std::string);
  /// Data-driven color for the entity
  SIMDATA_DECLARE_FIELD(color_, color, uint32_t);
  /// Override the normal coloration
  SIMDATA_DECLARE_FIELD(useOverrideColor_, useoverridecolor, bool);
  /// Override color when useOverrideColor is true
  SIMDATA_DECLARE_FIELD(overrideColor_, overridecolor, uint32_t);
  /// Entity-specific data limiting value in seconds of history; <= 0 indicates no limiting applied
  SIMDATA_DECLARE_FIELD(dataLimitTime_, datalimittime, double);
  /// Entity-specific data limiting value in number of data points; == 0 indicates no limiting applied
  SIMDATA_DECLARE_FIELD(dataLimitPoints_, datalimitpoints, uint32_t);
  /// Control Legend display; this may get replaced by more detailed control of legends
  SIMDATA_DECLARE_FIELD(includeInLegend_, includeinlegend, bool);
  /// Current accepted projector IDs; may contain up to 4 projector IDs
  SIMDATA_DECLARE_VECTOR_FIELD(acceptProjectorIds_, acceptprojectorids, uint64_t);
};

}

#endif
