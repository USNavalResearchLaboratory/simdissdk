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
#ifndef SIMDATA_ENTITY_PREFERENCES_H
#define SIMDATA_ENTITY_PREFERENCES_H

#include <memory>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"
#include "simData/CommonPreferences.h"

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

/** Custom rendering preferences */
class SDKDATA_EXPORT CustomRenderingPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CustomRenderingPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// How long, in seconds, to display a custom rendering after a data point
  SIMDATA_DECLARE_FIELD(persistence_, persistence, double);
  /**
   * The length of the history in seconds. The history is the smaller of secondsHistory and pointsHistory
   * Not all renderers support this field.
   */
  SIMDATA_DECLARE_FIELD(secondsHistory_, secondshistory, double);
  /**
   * How many data points before the current data point to display as a history.  Default to 0, which is no history.
   * Not all renderers support this field.
   */
  SIMDATA_DECLARE_FIELD(pointsHistory_, pointshistory, uint32_t);
  /// If true, show outline instead of the shape.  Not all renderers support this field.
  SIMDATA_DECLARE_FIELD(outline_, outline, bool);
  /**
   * A flag to determine if the history override color should be applied
   * Not all renderers support this field.
   */
  SIMDATA_DECLARE_FIELD(useHistoryOverrideColor_, usehistoryoverridecolor, bool);
  /**
   * The history override color
   * Not all renderers support this field.
   */
  SIMDATA_DECLARE_FIELD(historyOverrideColor_, historyoverridecolor, uint32_t);
  /// If true, show center axis inside the shape. Shape color must be semitransparent.  Not all renderers support this field.
  SIMDATA_DECLARE_FIELD(centerAxis_, centeraxis, bool);
  /// If true, show a lighted shape.  Not all renderers support this field
  SIMDATA_DECLARE_FIELD(showLighted_, showlighted, bool);
  /// if true, depth testing is active
  SIMDATA_DECLARE_FIELD(depthTest_, depthtest, bool);
};

/** Projector preferences */
class SDKDATA_EXPORT ProjectorPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ProjectorPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Projector image filename
  SIMDATA_DECLARE_FIELD_CONST_REF(rasterFile_, rasterfile, std::string);
  /// Determine if we want to show projector frustum
  SIMDATA_DECLARE_FIELD(showFrustum_, showfrustum, bool);
  /// Projector alpha for blending [0.0, 1.0]
  SIMDATA_DECLARE_FIELD(projectorAlpha_, projectoralpha, float);
  /// Determine if we should interpolate FOV
  SIMDATA_DECLARE_FIELD(interpolateProjectorFov_, interpolateprojectorfov, bool);
  /// Override the field of view Y (vertical FOV) value display if true, using overrideFovValue and ignoring projector updates
  SIMDATA_DECLARE_FIELD(overrideFov_, overridefov, bool);
  /// Field of view for projector Y (vertical FOV) value in radians, if overrideFov is true (default is ~10 degrees)
  SIMDATA_DECLARE_FIELD(overrideFovAngle_, overridefovangle, double);
  /// Override the field of view X (horizontal FOV) value display if true, using overrideFovValue and ignoring projector updates
  SIMDATA_DECLARE_FIELD(overrideHFov_, overridehfov, bool);
  /// Field of view for projector X (horizontal FOV) value in radians, if overrideFov is true (default is ~10 degrees)
  SIMDATA_DECLARE_FIELD(overrideHFovAngle_, overridehfovangle, double);
  /// Determine if the projector should use shadow mapping
  SIMDATA_DECLARE_FIELD(shadowMapping_, shadowmapping, bool);
  /// Maximum distance from projector at which to draw projected image (<=0 means no limiting)
  SIMDATA_DECLARE_FIELD(maxDrawRange_, maxdrawrange, float);
  /// Whether to project on backfacing geometry as well as front
  SIMDATA_DECLARE_FIELD(doubleSided_, doublesided, bool);
};

/** Laser preferences */
class SDKDATA_EXPORT LaserPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LaserPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Positional offset from the host platform
  SIMDATA_DECLARE_SUBFIELD_LIST(laserXyzOffset_, laserxyzoffset, Position);
  /// Range of the laser in meters
  SIMDATA_DECLARE_FIELD(maxRange_, maxrange, double);
  /// Width of the laser in pixels on the screen
  SIMDATA_DECLARE_FIELD(laserWidth_, laserwidth, int32_t);
};

/** Animated Line bending preference */
enum class AnimatedLineBend {
  /** Automatic line bending. Lines are straight unless they would intersect the earth, in which case they bend. */
  ALB_AUTO = 0,
  /** Lines are always straight. */
  ALB_STRAIGHT = 1,
  /** Lines always bend. */
  ALB_BEND = 2
};

/** LOB Group preferences */
class SDKDATA_EXPORT LobGroupPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LobGroupPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Positional offset from the host platform
  SIMDATA_DECLARE_SUBFIELD_LIST(xyzOffset_, xyzoffset, Position);
  /// Width of the LOBs in pixels on the screen
  SIMDATA_DECLARE_FIELD(lobwidth_, lobwidth, int32_t);
  /// Color of the first part of the stipple pattern; RRGGBBA
  SIMDATA_DECLARE_FIELD(color1_, color1, uint32_t);
  /// Color of the second part of the stipple pattern; RRGGBBA
  SIMDATA_DECLARE_FIELD(color2_, color2, uint32_t);
  /// GL stipple pattern of color1; 0xffff for a solid line
  SIMDATA_DECLARE_FIELD(stipple1_, stipple1, uint32_t);
  /// GL stipple pattern of color2; 0xffff for a solid line
  SIMDATA_DECLARE_FIELD(stipple2_, stipple2, uint32_t);
  /// the maximum number of seconds of history data to display
  SIMDATA_DECLARE_FIELD(maxDataSeconds_, maxdataseconds, double);
  /// the maximum number of data points to display
  SIMDATA_DECLARE_FIELD(maxDataPoints_, maxdatapoints, uint32_t);
  /// Clamp to ground if true, otherwise no clamping
  SIMDATA_DECLARE_FIELD(lobUseClampAlt_, lobuseclampalt, bool);
  /// Override the range indicated by data
  SIMDATA_DECLARE_FIELD(useRangeOverride_, userangeoverride, bool);
  /// Value to use for range (m) when overriding
  SIMDATA_DECLARE_FIELD(rangeOverrideValue_, rangeoverridevalue, double);
  /// Line bending preference
  SIMDATA_DECLARE_FIELD(bending_, bending, AnimatedLineBend);
};

/** Gate preferences */
class SDKDATA_EXPORT GatePrefs : public FieldList
{
public:
  /** Enumerations of different types of gates */
  enum class DrawMode {
    UNKNOWN = 0,
    RANGE = 1,
    GUARD = 2,
    // TARGET = 3,
    ANGLE = 4,
    RAIN = 5,
    CLUTTER = 6,
    FOOTPRINT = 7,
    SECTOR = 8,
    PUSH = 9,
    COVERAGE = 10  ///< Gate rendered as a spherical slice
    // BODY = 11,
  };

  /**  Different ways to fill the gate area */
  enum class FillPattern {
    STIPPLE = 0,
    SOLID = 1,
    ALPHA = 2,
    WIRE = 3,
    CENTROID = 4
  };

  SIMDATA_DECLARE_METHODS(GatePrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Toggle lighting (shading)
  SIMDATA_DECLARE_FIELD(gateLighting_, gatelighting, bool);
  /// Toggle alpha blending
  SIMDATA_DECLARE_FIELD(gateBlending_, gateblending, bool);
  /// Type of gate to draw
  SIMDATA_DECLARE_FIELD(gateDrawMode_, gatedrawmode, DrawMode);
  /// Specifies the way the gate volume is filled
  SIMDATA_DECLARE_FIELD(fillPattern_, fillpattern, FillPattern);
  /// Toggle drawing of the centroid
  SIMDATA_DECLARE_FIELD(drawCentroid_, drawcentroid, bool);
  /// Interpolate between data points
  SIMDATA_DECLARE_FIELD(interpolateGatePos_, interpolategatepos, bool);
  /// Azimuth offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(gateAzimuthOffset_, gateazimuthoffset, double);
  /// Elevation offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(gateElevationOffset_, gateelevationoffset, double);
  /// Roll offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(gateRollOffset_, gaterolloffset, double);
  /// Toggle drawing of the outline
  SIMDATA_DECLARE_FIELD(drawOutline_, drawoutline, bool);
  /// Color for the centroid
  SIMDATA_DECLARE_FIELD(centroidColor_, centroidcolor, uint32_t);
};

/** Antenna pattern preferences */
class SDKDATA_EXPORT AntennaPatterns : public FieldList
{
public:
  /// Antenna patterns can be read from a file or generated from an algorithm
  enum class Type {
    NONE = 0,
    FILE = 1,       ///< File based antenna pattern
    ALGORITHM = 2,  ///< Algorithmic based antenna pattern
  };

  /// Different file formats that are supported
  enum class FileFormat {
    TABLE = 6,           ///< Look-up table antenna pattern
    MONOPULSE = 7,       ///< Monopulse (sum and delta) antenna pattern
    RELATIVE_TABLE = 9,  ///< Relative look-up table antenna pattern
    BILINEAR = 10,       ///< Bilinear antenna pattern (type of interpolation)
    NSMA = 11,           ///< NSMA antenna pattern format
    EZNEC = 12,          ///< EZNEC antenna pattern format
    XFDTD = 13           ///< XFDTD antenna pattern format
  };

  /// Algorithms that can be used instead of a file
  enum class Algorithm {
    PEDESTAL = 1,  ///< Pedestal algorithm
    GAUSS = 2,     ///< Gaussian algorithm
    CSCSQ = 3,     ///< Cosecant squared algorithm
    SINXX = 4,     ///< Sin(x/x) algorithm
    OMNI = 5       ///< Omni directional algorithm
  };

  /// Enumeration of antenna volume rendering types
  enum class VolumeType {
    GAIN_AS_RANGE_SCALAR = 0,  ///< Use the gain value as if it were a range scalar
    FREE_SPACE_RANGE_LINEAR = 1, ///< Use the gain converted to linear range
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(AntennaPatterns);
  /// Type of pattern; either from a file, or from an algorithm
  SIMDATA_DECLARE_FIELD(type_, type, AntennaPatterns::Type);
  /// The following should be specified for file type antenna patterns
  SIMDATA_DECLARE_FIELD(fileFormat_, fileformat, AntennaPatterns::FileFormat);
  /// Filename of the antenna, for file-type patterns
  SIMDATA_DECLARE_FIELD_CONST_REF(fileName_, filename, std::string);
  /// The following should be specified for algorithm type antenna patterns
  SIMDATA_DECLARE_FIELD(algorithm_, algorithm, AntennaPatterns::Algorithm);
  /// Antenna volume rendering
  SIMDATA_DECLARE_FIELD(volumeType_, volumeType, AntennaPatterns::VolumeType);
};

/** Beam preferences */
class SDKDATA_EXPORT BeamPrefs : public FieldList
{
public:
  /** Beams can be drawn as a wire, as solid, or solid with wires; NOTE: different than Platform's draw mode */
  enum class DrawMode {
    WIRE = 0,
    SOLID = 1,
    WIRE_ON_SOLID = 2
  };

  /** Rendering type can draw using a pattern or as a 3 dB beam */
  enum class DrawType {
    BEAM_3DB = 0,         ///< Beam drawn using 3 dB half power points
    ANTENNA_PATTERN = 1,  ///< Beam drawn using antenna pattern
    COVERAGE = 2,         ///< Beam drawn as a spherical slice (cap only)
    LINE = 3              ///< Beam drawn as a line
  };

  /** RangeMode allows user to override beam range with a calculation */
  enum class RangeMode {
    BEAM_UPDATE = 0,         ///< Use range provided in beam update
    ONE_WAY_FREE_SPACE = 1,  ///< Use one-way-free-space calculation
    TWO_WAY_FREE_SPACE = 2  ///< Use two-way-free-space calculation
  };

  SIMDATA_DECLARE_METHODS(BeamPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Antenna pattern data for the beam
  SIMDATA_DECLARE_SUBFIELD_LIST(antennaPattern_, antennapattern, AntennaPatterns);
  /// Specify an offset relative to center of host platform
  SIMDATA_DECLARE_SUBFIELD_LIST(beamPositionOffset_, beampositionoffset, Position);
  /// Toggle lighting (shading)
  SIMDATA_DECLARE_FIELD(shaded_, shaded, bool);
  /// Toggle alpha blending
  SIMDATA_DECLARE_FIELD(blended_, blended, bool);
  /// Rendering mode: solid, wire, combined
  SIMDATA_DECLARE_FIELD(beamDrawMode_, beamdrawmode, BeamPrefs::DrawMode);
  /// Rendering scale, typically 1.0; multiplier that applies to the beam length
  SIMDATA_DECLARE_FIELD(beamScale_, beamscale, double);
  /// Rendering style, either as a full pattern or just as a 3 dB beam
  SIMDATA_DECLARE_FIELD(drawType_, drawtype, BeamPrefs::DrawType);
  /// Number of polygons in the beam's end cap resolution
  SIMDATA_DECLARE_FIELD(capResolution_, capresolution, uint32_t);
  /// Number of polygons in the beam's cone
  SIMDATA_DECLARE_FIELD(coneResolution_, coneresolution, uint32_t);
  /// Render beam as a cone instead of a spherical wedge
  SIMDATA_DECLARE_FIELD(renderCone_, rendercone, bool);
  /// Controls antenna pattern visibility based on gain (dB)
  SIMDATA_DECLARE_FIELD(sensitivity_, sensitivity, double);
  /// Main lobe antenna pattern gain (dB)
  SIMDATA_DECLARE_FIELD(gain_, gain, double);
  /// Antenna pattern angular field of view (radians)
  SIMDATA_DECLARE_FIELD(fieldOfView_, fieldofview, double);
  /// Angular step size, controlling antenna pattern's degree of detail, in degrees
  SIMDATA_DECLARE_FIELD(detail_, detail, double);
  /// Antenna transmitter peak power, in watts
  SIMDATA_DECLARE_FIELD(power_, power, double);
  /// Antenna transmitter frequency, in MHz
  SIMDATA_DECLARE_FIELD(frequency_, frequency, double);
  /// Polarization of the antenna
  SIMDATA_DECLARE_FIELD(polarity_, polarity, Polarity);
  /// Applies a color gradient based on antenna pattern gain
  SIMDATA_DECLARE_FIELD(colorScale_, colorscale, bool);
  /// AREPS file associated with the beam
  SIMDATA_DECLARE_FIELD_CONST_REF(arepsFile_, arepsfile, std::string);
  /// Channel is used for monopulse beams
  SIMDATA_DECLARE_FIELD(channel_, channel, bool);
  /// Apply a weighting function to gain value based on beam width (table and relative patterns only)
  SIMDATA_DECLARE_FIELD(weighting_, weighting, bool);
  /// Interpolate between data points
  SIMDATA_DECLARE_FIELD(interpolateBeamPos_, interpolatebeampos, bool);
  /**
   * Toggle use of platform offsets(as defined by the host platform) in positioning beam
   * Beams should use host platform offset when setting beam origin
   */
  SIMDATA_DECLARE_FIELD(useOffsetPlatform_, useoffsetplatform, bool);
  /**
   * Toggle use of icon offset (as defined by the host platform) in positioning beam
   * Icon offset is controlled by data, default condition is to originate from center of host
   */
  SIMDATA_DECLARE_FIELD(useOffsetIcon_, useoffseticon, bool);
  /**
   * Toggle use of beam position and orientation offsets defined below
   * Beam offset is controlled by data, default condition is to originate from center of host
   */
  SIMDATA_DECLARE_FIELD(useOffsetBeam_, useoffsetbeam, bool);
  /// Azimuth offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(azimuthOffset_, azimuthoffset, double);
  /// Elevation offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(elevationOffset_, elevationoffset, double);
  /// Roll offset to apply to updates, in radians
  SIMDATA_DECLARE_FIELD(rollOffset_, rolloffset, double);
  /// Current target platform; if a BeamType is target, the beam always points to this target
  SIMDATA_DECLARE_FIELD(targetId_, targetid, uint64_t);
  /// Full height of the beam relative, in radians; [0, PI/2]
  SIMDATA_DECLARE_FIELD(verticalWidth_, verticalwidth, double);
  /// Full width of the beam relative, in radians; [0, 2*PI]
  SIMDATA_DECLARE_FIELD(horizontalWidth_, horizontalwidth, double);
  /// Toggle Beam Pulse Animation
  SIMDATA_DECLARE_FIELD(animate_, animate, bool);
  /// Distance between pulse start and finish, in meters
  SIMDATA_DECLARE_FIELD(pulseLength_, pulselength, double);
  /// The rate for a beam to complete the animation pattern, in Hz
  SIMDATA_DECLARE_FIELD(pulseRate_, pulserate, double);
  /// Stipple bit mask for the pattern, with 1 indicating on and 0 indicating off; only uses lower 16 bits
  SIMDATA_DECLARE_FIELD(pulseStipple_, pulsestipple, uint32_t);
  /// Range mode of the beam
  SIMDATA_DECLARE_FIELD(rangeMode_, rangemode, BeamPrefs::RangeMode);
};

/** Time Tick preferences */
class SDKDATA_EXPORT TimeTickPrefs : public FieldList
{
public:
  enum class DrawStyle {
    NONE = 0,  ///< Do not draw time ticks
    POINT = 1, ///< draw ticks as points
    LINE = 2   ///< draw ticks as lines
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(TimeTickPrefs);
  /// Indicates how to render time ticks
  SIMDATA_DECLARE_FIELD(drawStyle_, drawstyle, DrawStyle);
  /// Default color is translucent white
  SIMDATA_DECLARE_FIELD(color_, color, uint32_t);
  /// Time tick interval in seconds
  SIMDATA_DECLARE_FIELD(interval_, interval, double);
  /// Defines interval to draw the tick larger, draws at multiple of the tick interval; 0 means no large ticks
  SIMDATA_DECLARE_FIELD(largeIntervalFactor_, largeintervalfactor, uint32_t);
  /// Defines interval to draw a label on the tick, draws at a multiple of the tick interval; 0 means no labels
  SIMDATA_DECLARE_FIELD(labelIntervalFactor_, labelintervalfactor, uint32_t);
  /// Font name to use for the labels
  SIMDATA_DECLARE_FIELD_CONST_REF(labelFontName_, labelfontname, std::string);
  /// Size of the text in the given font for time tick labels
  SIMDATA_DECLARE_FIELD(labelFontPointSize_, labelfontpointsize, uint32_t);
  /// Label color; if 0, then use the "color" field
  SIMDATA_DECLARE_FIELD(labelColor_, labelcolor, uint32_t);
  /// Tick line length in meters
  SIMDATA_DECLARE_FIELD(lineLength_, linelength, double);
  /// Defines how big to draw the large line tick or the point tick, draws large tick as a multiple of lineLength and large point tick as a multiple of lineWidth
  SIMDATA_DECLARE_FIELD(largeSizeFactor_, largesizefactor, uint32_t);
  /// Format for the time shown in the label text
  SIMDATA_DECLARE_FIELD(labelTimeFormat_, labeltimeformat, ElapsedTimeFormat);
  /// Line and point width for drawing the time ticks
  SIMDATA_DECLARE_FIELD(lineWidth_, linewidth, double);
};

/** Track preferences */
class SDKDATA_EXPORT TrackPrefs : public FieldList
{
public:
  /**  Mode for drawing track position */
  enum class Mode {
    OFF = 0,     ///< nothing is drawn
    POINT = 1,   ///< draw a point at each previous position
    LINE = 2,    ///< draw a line connecting previous positions
    RIBBON = 3,  ///< a line with wings showing roll
    BRIDGE = 4   ///< "ribbon" plus a drop line down to the ground
  };

  SIMDATA_DECLARE_METHODS(TrackPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(timeTicks_, timeticks, TimeTickPrefs);
  /// Off-white, the default color to use for track history. Track color history at time is defined in a data table, simData::INTERNAL_TRACK_HISTORY_TABLE, in column simData::INTERNAL_TRACK_HISTORY_COLOR_COLUMN
  SIMDATA_DECLARE_FIELD(trackColor_, trackcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(multiTrackColor_, multitrackcolor, bool);
  /// Flattens the track history to the earth surface
  SIMDATA_DECLARE_FIELD(flatMode_, flatmode, bool);
  /// Indicates whether to drop a line to the earth for altitude mode
  SIMDATA_DECLARE_FIELD(altMode_, altmode, bool);
  SIMDATA_DECLARE_FIELD(expireMode_, expiremode, bool);
  SIMDATA_DECLARE_FIELD(usePlatformColor_, useplatformcolor, bool);
  SIMDATA_DECLARE_FIELD(useTrackOverrideColor_, usetrackoverridecolor, bool);
  SIMDATA_DECLARE_FIELD(trackOverrideColor_, trackoverridecolor, uint32_t);
  /// Length in seconds with < 0 meaning unlimited
  SIMDATA_DECLARE_FIELD(trackLength_, tracklength, int32_t);
  SIMDATA_DECLARE_FIELD(lineWidth_, linewidth, double);
  SIMDATA_DECLARE_FIELD(trackDrawMode_, trackdrawmode, TrackPrefs::Mode);
};

/// Fragment shader effects applied to 3-D graphics in 2-D screen space.
enum class FragmentEffect {
  /// No fragment effect on icon (default).
  FE_NONE = 0,
  /// Forward striping, like forward slash. Fragment color replaces existing value.
  FE_FORWARD_STRIPE = 1,
  /// Backward striping, like backslash. Fragment color replaces existing value.
  FE_BACKWARD_STRIPE = 2,
  /// Horizontal striping along the X axis. Fragment color replaces existing value.
  FE_HORIZONTAL_STRIPE = 3,
  /// Vertical striping along the Y axis. Fragment color replaces existing value.
  FE_VERTICAL_STRIPE = 4,
  /// Combination of horizontal and vertical striping. Fragment color replaces existing value.
  FE_CHECKERBOARD = 5,
  /// Combination of forward and backward striping. Fragment color replaces existing value.
  FE_DIAMOND = 6,
  /// Icon color changes gradually in a glow. Fragment color is unused.
  FE_GLOW = 7,
  /// Icon blinks on and off. Fragment color is unused.
  FE_FLASH = 8
};

/// Override color combination mode
enum class OverrideColorCombineMode {
  /// Multiply the override color against incoming color; good for shaded items and 2D images
  MULTIPLY_COLOR = 0,
  /// Replace the incoming color with the override color; good for flat items
  REPLACE_COLOR = 1,
  /// Apply color by copying the previous color intensity and replacing with this one, retaining shading better than REPLACE_COLOR
  INTENSITY_GRADIENT = 2
};

/// Platform lifespan definition options for file mode
enum class LifespanMode {
  /**
   * Platform lifespan defined by the first and last point. Static entities exist at all times and are static.
   * Single point platforms exist only at the instant in time of their single data point. This is the legacy
   * SIMDIS behavior in file mode that has been used as the default until approximately 2024.
   */
  LIFE_FIRST_LAST_POINT = 0,

  /**
   * Platform lifespan defined by the first and last point. Static entities exist at all times and are static.
   * Single point platforms start existing at the time of their data point, and exist until the end of the
   * scenario, until removed, or until some preference rule turns them off. This is useful for data sources
   * recorded in live mode, with a propensity of single point entities, where the data source does not
   * correctly terminate the platform's lifespan with an ending point. This is intended to be a low-risk
   * improvement to LIFE_FIRST_LAST_POINT for what typically otherwise might signal an error condition
   * (having a platform with only one point existing only for one instant in time).
   */
  LIFE_EXTEND_SINGLE_POINT = 1
};

/// Circle highlight shape selection
enum class CircleHilightShape {
  /// Filled-in semitransparent animated with a rotation and slight color pulse
  CH_PULSING_CIRCLE = 0,
  /// Circle outline; no animation
  CH_CIRCLE = 1,
  /// Diamond outline; no animation
  CH_DIAMOND = 2,
  /// Square outline; no animation
  CH_SQUARE = 3,
  /// Square outline with line sides cut off, looks like reticle; no animation
  CH_SQUARE_RETICLE = 4,
  /// Coffin shaped, appropriate for e.g. kill/rebirth functionality
  CH_COFFIN = 5
};

/// Polygon face corresponds to the front, back, or the front-and-back faces
enum class PolygonFace {
  FRONT_AND_BACK = 0,  ///< Corresponds to GL_FRONT_AND_BACK
  FRONT = 1,           ///< GL_FRONT
  BACK = 2             ///< GL_BACK
};

/// Polygon rasterization mode; these values match GL defines, and osg::PolygonMode enum
enum class PolygonMode {
  POINT = 0x1B00,   ///< GL_POINT
  LINE = 0x1B01,    ///< GL_LINE
  FILL = 0x1B02     ///< GL_FILL
};

/// Algorithm selection for Dynamic Scale
enum class DynamicScaleAlgorithm {

  /**
   * Traditional SIMDIS Dynamic Scaling algorithm.  When objects are far away, they are
   * scaled to a relatively consistent size based on the maximum X Y or Z dimension of
   * the model.  Therefore a large ship might appear the same size as a small biplane.
   * This algorithm reverts to normal scaling when zoomed in closely to the entry.  The
   * scaling is impacted by the static scale, dynamic scale scalar, and the dynamic
   * scale offset.
   */
  DSA_CONSISTENT_SIZING = 0,

  /**
   * Directly map up the model's dimensions to pixels on the screen.  This means larger
   * models will be larger on the screen, and smaller models will be smaller on the screen.
   * Combined with image icons, this makes an image icon appear as the same dimension as
   * the source image, which DSA_CONSISTENT_SIZING cannot not guarantee.  As a result,
   * zooming into an entity will not change its size, even as you get close to the object.
   * Scaling is impacted by static scale and the dynamic scale scalar only.  Dynamic scale
   * offset does not affect the displayed icon size with this algorithm.
   */
  DSA_METERS_TO_PIXELS = 1
};

/// Preferences for the display of a platform
class SDKDATA_EXPORT PlatformPrefs : public FieldList
{
public:
  /**
   * Controls how to interpret commonPrefs.draw when set to false. When commonPrefs.draw is
   * set to false the default behavior is to not draw the platform, but to continue drawing
   * the platform's children and continue displaying the platform in the entity list. if
   * drawOffBehavior is set to OMIT_CHILDREN_AND_VIS_UPDATE both the platform and the platform's
   * children are not displayed, and omit the platform and its children from the entity list.
   */
  enum class DrawOffBehavior {
    DEFAULT_BEHAVIOR = 0,             ///< Hide the platform, but continue displaying the platform's children
    OMIT_CHILDREN_AND_VIS_UPDATE = 1  ///< Hide the platform and the platform's children
  };

  SIMDATA_DECLARE_METHODS(PlatformPrefs);
  /// Contains name, draw/active flags, label prefs, data limiting, and other settings. See CommonPreferences.h for more details.
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  /// Display options for the track history
  SIMDATA_DECLARE_SUBFIELD_LIST(trackPrefs_, trackprefs, TrackPrefs);
  /// Position offset in X/Y/Z space relative to an ENU frame at platform center
  SIMDATA_DECLARE_SUBFIELD_LIST(platPositionOffset_, platpositionoffset, Position);
  /// Offset to apply to orientation values, in body angles
  SIMDATA_DECLARE_SUBFIELD_LIST(orientationOffset_, orientationoffset, BodyOrientation);
  /**
   * Vector scale to apply to the model, where x is the major axis/length of the model, y is the width, and z the depth;
   * overrides scaleLevel when applied at same time
   *
   * Note:  A value of 0.0 or less for any component means no scaling
   */
  SIMDATA_DECLARE_SUBFIELD_LIST(scaleXYZ_, scalexyz, Position);
  /// 2D or 3D model file location
  SIMDATA_DECLARE_FIELD_CONST_REF(icon_, icon, std::string);
  /// Model draw mode for wireframe, points, or solid models
  SIMDATA_DECLARE_FIELD(drawMode_, drawmode, ModelDrawMode);
  /// Shader effect to apply to graphics
  SIMDATA_DECLARE_FIELD(fragmentEffect_, fragmenteffect, FragmentEffect);
  /// Color associated with the fragment effect
  SIMDATA_DECLARE_FIELD(fragmentEffectColor_, fragmenteffectcolor, uint32_t);
  /// Flag indicating how to draw 2D icons in 3D space; only applies to image icons
  SIMDATA_DECLARE_FIELD(rotateIcons_, rotateicons, IconRotation);
  /// Turns off the depth buffer for drawing 2D icons; can address flickering with surface icons
  SIMDATA_DECLARE_FIELD(noDepthIcons_, nodepthicons, bool);
  /// Sets the horizontal and vertical alignment for 2D image icons
  SIMDATA_DECLARE_FIELD(iconAlignment_, iconalignment, TextAlignment);
  /// Method for applying the override color combine mode (only applies for platforms)
  SIMDATA_DECLARE_FIELD(overrideColorCombineMode_, overridecolorcombinemode, OverrideColorCombineMode);
  /// Clamps the altitude between certain geodetic values during display
  SIMDATA_DECLARE_FIELD(useClampAlt_, useclampalt, bool);
  /// Minimum altitude when useClampAlt is enabled, in meters
  SIMDATA_DECLARE_FIELD(clampValAltMin_, clampvalaltmin, double);
  /// Maximum altitude when useClampAlt is enabled, in meters
  SIMDATA_DECLARE_FIELD(clampValAltMax_, clampvalaltmax, double);
  /// Enables clamping for the Yaw values; useful to keep stationary entities pointing in the same direction
  SIMDATA_DECLARE_FIELD(useClampYaw_, useclampyaw, bool);
  /// Yaw orientation for clamping, in radians
  SIMDATA_DECLARE_FIELD(clampValYaw_, clampvalyaw, double);
  /// Enables clamping for the Pitch values; useful to keep stationary entities pointing in the same direction
  SIMDATA_DECLARE_FIELD(useClampPitch_, useclamppitch, bool);
  /// Pitch orientation for clamping, in radians
  SIMDATA_DECLARE_FIELD(clampValPitch_, clampvalpitch, double);
  /// Enables clamping for the Roll values; useful to keep stationary entities pointing in the same direction
  SIMDATA_DECLARE_FIELD(useClampRoll_, useclamproll, bool);
  /// Roll orientation for clamping, in radians
  SIMDATA_DECLARE_FIELD(clampValRoll_, clampvalroll, double);
  /// Automatically use the platform's orientation clamp values at extremely low velocity.
  SIMDATA_DECLARE_FIELD(clampOrientationAtLowVelocity_, clamporientationatlowvelocity, bool);
  /// Keep platform clamp to the surface
  SIMDATA_DECLARE_FIELD(surfaceClamping_, surfaceclamping, bool);
  /// Keep platform above the surface
  SIMDATA_DECLARE_FIELD(aboveSurfaceClamping_, abovesurfaceclamping, bool);
  /// Toggles lighting
  SIMDATA_DECLARE_FIELD(lighted_, lighted, bool);
  /// Toggles box mode, drawing a 3D box around the entity position
  SIMDATA_DECLARE_FIELD(drawBox_, drawbox, bool);
  /// Toggles flag for drawing the body rotational axis
  SIMDATA_DECLARE_FIELD(drawBodyAxis_, drawbodyaxis, bool);
  /// Toggle scenario reference axis
  SIMDATA_DECLARE_FIELD(drawInertialAxis_, drawinertialaxis, bool);
  /// Toggles a vector drawn towards the current sun position
  SIMDATA_DECLARE_FIELD(drawSunVec_, drawsunvec, bool);
  /// Toggles a vector drawn towards the current moon position
  SIMDATA_DECLARE_FIELD(drawMoonVec_, drawmoonvec, bool);
  /// Scale for the body, inertial, sun, and moon axis
  SIMDATA_DECLARE_FIELD(axisScale_, axisscale, double);
  /// Toggle wireframe display of EM/EO beam
  SIMDATA_DECLARE_FIELD(wireFrame_, wireframe, bool);
  /// Toggle display of optical line of sight range
  SIMDATA_DECLARE_FIELD(drawOpticLos_, drawopticlos, bool);
  /// Toggle display of radio horizon range
  SIMDATA_DECLARE_FIELD(drawRfLos_, drawrflos, bool);
  /// Radar cross section file specific to the platform
  SIMDATA_DECLARE_FIELD_CONST_REF(rcsFile_, rcsfile, std::string);
  /// Toggle display of the 2-D RCS pattern; white ring at 0 dB, rings every 10 dB
  SIMDATA_DECLARE_FIELD(drawRcs_, drawrcs, bool);
  /// Toggle display of the 3-D RCS pattern
  SIMDATA_DECLARE_FIELD(draw3dRcs_, draw3drcs, bool);
  /// RCS data color
  SIMDATA_DECLARE_FIELD(rcsColor_, rcscolor, uint32_t);
  /// Applies a color gradient based on RCS return
  SIMDATA_DECLARE_FIELD(rcsColorScale_, rcscolorscale, bool);
  /// RCS pattern polarization
  SIMDATA_DECLARE_FIELD(rcsPolarity_, rcspolarity, Polarity);
  /// Elevation angle incident on RCS pattern, in degrees
  SIMDATA_DECLARE_FIELD(rcsElevation_, rcselevation, double);
  /// RF frequency in MHz
  SIMDATA_DECLARE_FIELD(rcsFrequency_, rcsfrequency, double);
  /// Angular detail of RCS pattern in degrees
  SIMDATA_DECLARE_FIELD(rcsDetail_, rcsdetail, double);
  /// Draws a rotating "circle" highlight about the model position; useful for drawing attention to platforms
  SIMDATA_DECLARE_FIELD(drawCircleHilight_, drawcirclehilight, bool);
  /// Color value for the "circular" highlight; see drawCircleHilight
  SIMDATA_DECLARE_FIELD(circleHilightColor_, circlehilightcolor, uint32_t);
  /// Shape for "circle" highlight
  SIMDATA_DECLARE_FIELD(circleHilightShape_, circlehilightshape, CircleHilightShape);
  /// Shape size in meters; 0 means use icon size
  SIMDATA_DECLARE_FIELD(circleHilightSize_, circlehilightsize, double);
  /// Highlight shape will follow platform's yaw
  SIMDATA_DECLARE_FIELD(hilightFollowYaw_, hilightfollowyaw, bool);
  /// Position values are interpolated between data points
  SIMDATA_DECLARE_FIELD(interpolatePos_, interpolatepos, bool);
  /// Position values are extrapolated past end of time for a platform
  SIMDATA_DECLARE_FIELD(extrapolatePos_, extrapolatepos, bool);
  /// Scalar to apply to the model, applies same scale to all dimensions of model
  SIMDATA_DECLARE_FIELD(scale_, scale, double);
  /// Adjusts platform brightness (0-100)
  SIMDATA_DECLARE_FIELD(brightness_, brightness, int32_t);
  /// Toggle dynamic scaling based on eye position
  SIMDATA_DECLARE_FIELD(dynamicScale_, dynamicscale, bool);
  SIMDATA_DECLARE_FIELD(dynamicScaleOffset_, dynamicscaleoffset, double);
  SIMDATA_DECLARE_FIELD(dynamicScaleScalar_, dynamicscalescalar, double);
  SIMDATA_DECLARE_FIELD(dynamicScaleAlgorithm_, dynamicscalealgorithm, DynamicScaleAlgorithm);
  /// Toggle velocity vector
  SIMDATA_DECLARE_FIELD(drawVelocityVec_, drawvelocityvec, bool);
  /// Color for the velocity vector line
  SIMDATA_DECLARE_FIELD(velVecColor_, velveccolor, uint32_t);
  /// Toggle between static length and length based on time
  SIMDATA_DECLARE_FIELD(velVecUseStaticLength_, velvecusestaticlength, bool);
  /// Sets the length of the velocity vector, in velVecStaticLenUnits
  SIMDATA_DECLARE_FIELD(velVecStaticLen_, velvecstaticlen, double);
  /// Velocity vector length's distance units
  SIMDATA_DECLARE_FIELD(velVecStaticLenUnits_, velvecstaticlenunits, DistanceUnits);
  /// If velVecUseStaticLength is false, this is time value for velocity length calculation in velVecTimeUnits
  SIMDATA_DECLARE_FIELD(velVecTime_, velvectime, double);
  /// Velocity vector length's time units
  SIMDATA_DECLARE_FIELD(velVecTimeUnits_, velvectimeunits, ElapsedTimeFormat);
  /// Attached GOG files
  SIMDATA_DECLARE_VECTOR_FIELD(gogFile_, gogfile, std::string);
  /// Toggles drawing the platform icon with a second pass that draws back-facing polygons, to show model even when inside
  SIMDATA_DECLARE_FIELD(alphaVolume_, alphavolume, bool);
  /// Toggles the use of polygon culling for the platform icon
  SIMDATA_DECLARE_FIELD(useCullFace_, usecullface, bool);
  /// The platform icon polygon faces to be culled
  SIMDATA_DECLARE_FIELD(cullFace_, cullface, PolygonFace);
  /// Polygon face to be rendered using the specified polygonMode
  SIMDATA_DECLARE_FIELD(polygonModeFace_, polygonmodeface, PolygonFace);
  /// Polygon rasterization mode to be applied to the face specified by polygonModeFace; mode may be GL_POINT, GL_LINE, or GL_FILL
  SIMDATA_DECLARE_FIELD(polygonMode_, polygonmode, PolygonMode);
  /// Toggles drawing the platform icon using the polygonStipple option
  SIMDATA_DECLARE_FIELD(usePolygonStipple_, usepolygonstipple, bool);
  /// The polygonStipple rendering option, a value between 1 and 9 corresponding to 9 SIMDIS stipple patterns
  SIMDATA_DECLARE_FIELD(polygonStipple_, polygonstipple, uint32_t);
  /// The color to use for visible horizon line of sight areas
  SIMDATA_DECLARE_FIELD(visibleLosColor_, visibleloscolor, uint32_t);
  /// The color to use for obstructed horizon line of sight areas
  SIMDATA_DECLARE_FIELD(obstructedLosColor_, obstructedloscolor, uint32_t);
  /// The range resolution to use for line of sight calculations, in meters
  SIMDATA_DECLARE_FIELD(losRangeResolution_, losrangeresolution, double);
  /// The azimuthal resolution to use for line of sight calculations, in degrees
  SIMDATA_DECLARE_FIELD(losAzimuthalResolution_, losazimuthalresolution, double);
  /// The altitude offset applied to the Platform's altitude when calculating LOS, in meters
  SIMDATA_DECLARE_FIELD(losAltitudeOffset_, losaltitudeoffset, double);
  /// Automatic animation of Degree-of-Freedom nodes is enabled or disabled
  SIMDATA_DECLARE_FIELD(animateDofNodes_, animatedofnodes, bool);
  /// Toggles whether the platform data should be interpreted as Earth-Fixed or Inertial
  SIMDATA_DECLARE_FIELD(eciDataMode_, ecidatamode, bool);
  /// Behavior when commonPrefs.draw is false
  SIMDATA_DECLARE_FIELD(drawOffBehavior_, drawoffbehavior, PlatformPrefs::DrawOffBehavior);
  /// Explains how platform TSPI data relates to calculated platform lifespan in file mode.
  SIMDATA_DECLARE_FIELD(lifespanMode_, lifespanmode, LifespanMode);
};

}

#endif
