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

/// define the text outline style
enum class TextOutline {
  TO_NONE = 0,
  TO_THIN,
  TO_THICK
};

/// define OSG text backdrop types
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

/// define OSG text backdrop implementation
enum class BackdropImplementation {
  BDI_POLYGON_OFFSET = 0,
  BDI_NO_DEPTH_BUFFER,
  BDI_DEPTH_RANGE,
  BDI_STENCIL_BUFFER,
  BDI_DELAYED_DEPTH_WRITES,
};

/// define OSG text alignment
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
  UNITS_KILOFEET = 32        ///< Kilofeet constant flag = 1000 feet)
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
  SIMDATA_DECLARE_FIELD(xLat_, xlat, bool);
  SIMDATA_DECLARE_FIELD(yLon_, ylon, bool);
  SIMDATA_DECLARE_FIELD(zAlt_, zalt, bool);
  SIMDATA_DECLARE_FIELD(genericData_, genericdata, bool);
  SIMDATA_DECLARE_FIELD(categoryData_, categorydata, bool);
  SIMDATA_DECLARE_FIELD(yaw_, yaw, bool);
  SIMDATA_DECLARE_FIELD(pitch_, pitch, bool);
  SIMDATA_DECLARE_FIELD(roll_, roll, bool);
  SIMDATA_DECLARE_FIELD(course_, course, bool);
  SIMDATA_DECLARE_FIELD(flightPathElevation_, flightpathelevation, bool);
  SIMDATA_DECLARE_FIELD(displayVX_, displayvx, bool);
  SIMDATA_DECLARE_FIELD(displayVY_, displayvy, bool);
  SIMDATA_DECLARE_FIELD(displayVZ_, displayvz, bool);
  SIMDATA_DECLARE_FIELD(speed_, speed, bool);
  SIMDATA_DECLARE_FIELD(mach_, mach, bool);
  SIMDATA_DECLARE_FIELD(angleOfAttack_, angleofattack, bool);
  SIMDATA_DECLARE_FIELD(sideSlip_, sideslip, bool);
  SIMDATA_DECLARE_FIELD(totalAngleOfAttack_, totalangleofattack, bool);
  SIMDATA_DECLARE_FIELD(solarAzimuth_, solarazimuth, bool);
  SIMDATA_DECLARE_FIELD(solarElevation_, solarelevation, bool);
  SIMDATA_DECLARE_FIELD(solarIlluminance_, solarilluminance, bool);
  SIMDATA_DECLARE_FIELD(lunarAzimuth_, lunarazimuth, bool);
  SIMDATA_DECLARE_FIELD(lunarElevation_, lunarelevation, bool);
  SIMDATA_DECLARE_FIELD(lunarIlluminance_, lunarilluminance, bool);
  SIMDATA_DECLARE_FIELD(late_, late, bool);
  SIMDATA_DECLARE_FIELD(useLabelCode_, uselabelcode, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(labelCode_, labelcode, std::string);
};

class SDKDATA_EXPORT LabelPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LabelPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(displayFields_, displayfields, simData::DisplayFields);
  SIMDATA_DECLARE_SUBFIELD_LIST(legendDisplayFields_, legenddisplayfields, simData::DisplayFields);
  SIMDATA_DECLARE_SUBFIELD_LIST(hoverDisplayFields_, hoverdisplayfields, simData::DisplayFields);
  SIMDATA_DECLARE_SUBFIELD_LIST(hookDisplayFields_, hookdisplayfields, simData::DisplayFields);
  SIMDATA_DECLARE_FIELD(draw_, draw, bool);
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
  SIMDATA_DECLARE_FIELD(priority_, priority, double);
  SIMDATA_DECLARE_FIELD(applyHeightAboveTerrain_, applyheightaboveterrain, bool);
  SIMDATA_DECLARE_FIELD(applyRoll_, applyroll, bool);
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  SIMDATA_DECLARE_FIELD(verticalDatum_, verticaldatum, simData::VerticalDatum);
  SIMDATA_DECLARE_FIELD(magneticVariance_, magneticvariance, simData::MagneticVariance);
  SIMDATA_DECLARE_FIELD(distanceUnits_, distanceunits, simData::DistanceUnits);
  SIMDATA_DECLARE_FIELD(angleUnits_, angleunits, simData::AngleUnits);
  SIMDATA_DECLARE_FIELD(speedUnits_, speedunits, simData::SpeedUnits);
  SIMDATA_DECLARE_FIELD(precision_, precision, int32_t);
  SIMDATA_DECLARE_FIELD(nameLength_, namelength, int32_t);
  SIMDATA_DECLARE_FIELD(geodeticUnits_, geodeticunits, simData::GeodeticUnits);
  SIMDATA_DECLARE_FIELD(altitudeUnits_, altitudeunits, simData::DistanceUnits);
  SIMDATA_DECLARE_FIELD(distancePrecision_, distanceprecision, int32_t);
  SIMDATA_DECLARE_FIELD(anglePrecision_, angleprecision, int32_t);
  SIMDATA_DECLARE_FIELD(speedPrecision_, speedprecision, int32_t);
  SIMDATA_DECLARE_FIELD(geodeticPrecision_, geodeticprecision, int32_t);
  SIMDATA_DECLARE_FIELD(altitudePrecision_, altitudeprecision, int32_t);
  SIMDATA_DECLARE_FIELD(timePrecision_, timeprecision, int32_t);

public:
  enum class UseValue {
    ACTUAL_VALUE = 0, ///< labels use actual data value
    DISPLAY_VALUE = 1 ///< labels use display value, which may include adjustments like clamping and offsets
  };

  SIMDATA_DECLARE_FIELD(useValues_, usevalues, UseValue);
};

class SDKDATA_EXPORT SpeedRing : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(SpeedRing);
  SIMDATA_DECLARE_FIELD(useFixedTime_, usefixedtime, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(fixedTime_, fixedtime, std::string);
  SIMDATA_DECLARE_FIELD(timeFormat_, timeformat, simData::ElapsedTimeFormat);
  SIMDATA_DECLARE_FIELD(radius_, radius, double);
  SIMDATA_DECLARE_FIELD(usePlatformSpeed_, useplatformspeed, bool);
  SIMDATA_DECLARE_FIELD(speedToUse_, speedtouse, double);
  SIMDATA_DECLARE_FIELD(displayTime_, displaytime, bool);
  SIMDATA_DECLARE_FIELD(speedUnits_, speedunits, simData::SpeedUnits);
};

class SDKDATA_EXPORT GridSettings : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(GridSettings);
  SIMDATA_DECLARE_FIELD(numDivisions_, numdivisions, uint32_t);
  SIMDATA_DECLARE_FIELD(numSubDivisions_, numsubdivisions, uint32_t);
  SIMDATA_DECLARE_FIELD(sectorAngle_, sectorangle, double);
};

class SDKDATA_EXPORT Position : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(Position);
  SIMDATA_DECLARE_FIELD(x_, x, double);
  SIMDATA_DECLARE_FIELD(y_, y, double);
  SIMDATA_DECLARE_FIELD(z_, z, double);
};

class SDKDATA_EXPORT BodyOrientation : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(BodyOrientation);
  SIMDATA_DECLARE_FIELD(yaw_, yaw, double);
  SIMDATA_DECLARE_FIELD(pitch_, pitch, double);
  SIMDATA_DECLARE_FIELD(roll_, roll, double);
};

class SDKDATA_EXPORT LocalGridPrefs : public FieldList
{
public:
  /// shape of local grid
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
  SIMDATA_DECLARE_SUBFIELD_LIST(gridPositionOffset_, gridpositionoffset, simData::Position);
  SIMDATA_DECLARE_SUBFIELD_LIST(gridOrientationOffset_, gridorientationoffset, simData::BodyOrientation);
  SIMDATA_DECLARE_FIELD(gridType_, gridtype, simData::LocalGridPrefs::Type);
  SIMDATA_DECLARE_FIELD(gridLabelDraw_, gridlabeldraw, bool);
  SIMDATA_DECLARE_FIELD(gridLabelColor_, gridlabelcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(gridLabelTextOutline_, gridlabeltextoutline, simData::TextOutline);
  SIMDATA_DECLARE_FIELD(gridLabelOutlineColor_, gridlabeloutlinecolor, uint32_t);
  SIMDATA_DECLARE_FIELD_CONST_REF(gridLabelFontName_, gridlabelfontname, std::string);
  SIMDATA_DECLARE_FIELD(gridLabelFontSize_, gridlabelfontsize, uint32_t);
  SIMDATA_DECLARE_FIELD(gridLabelPrecision_, gridlabelprecision, int32_t);
  SIMDATA_DECLARE_FIELD(drawGrid_, drawgrid, bool);
  SIMDATA_DECLARE_FIELD(gridColor_, gridcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(size_, size, double);
  SIMDATA_DECLARE_FIELD(positionOffsetUnits_, positionoffsetunits, simData::DistanceUnits);
  SIMDATA_DECLARE_FIELD(followYaw_, followyaw, bool);
  SIMDATA_DECLARE_FIELD(followPitch_, followpitch, bool);
  SIMDATA_DECLARE_FIELD(followRoll_, followroll, bool);
  SIMDATA_DECLARE_FIELD(sizeUnits_, sizeunits, simData::DistanceUnits);
};

class SDKDATA_EXPORT CommonPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CommonPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(labelPrefs_, labelprefs, LabelPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(localGrid_, localgrid, LocalGridPrefs);
  SIMDATA_DECLARE_FIELD(dataDraw_, datadraw, bool);
  SIMDATA_DECLARE_FIELD(draw_, draw, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(name_, name, std::string);
  SIMDATA_DECLARE_FIELD(useAlias_, usealias, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(alias_, alias, std::string);
  SIMDATA_DECLARE_FIELD(color_, color, uint32_t);
  SIMDATA_DECLARE_FIELD(useOverrideColor_, useoverridecolor, bool);
  SIMDATA_DECLARE_FIELD(overrideColor_, overridecolor, uint32_t);
  SIMDATA_DECLARE_FIELD(dataLimitTime_, datalimittime, double);
  SIMDATA_DECLARE_FIELD(dataLimitPoints_, datalimitpoints, uint32_t);
  SIMDATA_DECLARE_FIELD(includeInLegend_, includeinlegend, bool);
  SIMDATA_DECLARE_VECTOR_FIELD(acceptProjectorIds_, acceptprojectorids, uint64_t);
};

}

#endif
