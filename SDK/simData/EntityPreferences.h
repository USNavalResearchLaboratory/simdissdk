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

/** Custom rendering preferences */
class SDKDATA_EXPORT CustomRenderingPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CustomRenderingPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_FIELD(persistence_, persistence, double);
  SIMDATA_DECLARE_FIELD(secondsHistory_, secondshistory, double);
  SIMDATA_DECLARE_FIELD(pointsHistory_, pointshistory, uint32_t);
  SIMDATA_DECLARE_FIELD(outline_, outline, bool);
  SIMDATA_DECLARE_FIELD(useHistoryOverrideColor_, usehistoryoverridecolor, bool);
  SIMDATA_DECLARE_FIELD(historyOverrideColor_, historyoverridecolor, uint32_t);
  SIMDATA_DECLARE_FIELD(centerAxis_, centeraxis, bool);
  SIMDATA_DECLARE_FIELD(showLighted_, showlighted, bool);
  SIMDATA_DECLARE_FIELD(depthTest_, depthtest, bool);
};

/** Projector preferences */
class SDKDATA_EXPORT ProjectorPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ProjectorPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_FIELD_CONST_REF(rasterFile_, rasterfile, std::string);
  SIMDATA_DECLARE_FIELD(showFrustum_, showfrustum, bool);
  SIMDATA_DECLARE_FIELD(projectorAlpha_, projectoralpha, float);
  SIMDATA_DECLARE_FIELD(interpolateProjectorFov_, interpolateprojectorfov, bool);
  SIMDATA_DECLARE_FIELD(overrideFov_, overridefov, bool);
  SIMDATA_DECLARE_FIELD(overrideFovAngle_, overridefovangle, double);
  SIMDATA_DECLARE_FIELD(overrideHFov_, overridehfov, bool);
  SIMDATA_DECLARE_FIELD(overrideHFovAngle_, overridehfovangle, double);
  SIMDATA_DECLARE_FIELD(shadowMapping_, shadowmapping, bool);
  SIMDATA_DECLARE_FIELD(maxDrawRange_, maxdrawrange, float);
  SIMDATA_DECLARE_FIELD(doubleSided_, doublesided, bool);
};

/** Laser preferences */
class SDKDATA_EXPORT LaserPrefs : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(LaserPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(laserXyzOffset_, laserxyzoffset, Position);
  SIMDATA_DECLARE_FIELD(maxRange_, maxrange, double);
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
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(xyzOffset_, xyzoffset, Position);
  SIMDATA_DECLARE_FIELD(lobwidth_, lobwidth, int32_t);
  SIMDATA_DECLARE_FIELD(color1_, color1, uint32_t);
  SIMDATA_DECLARE_FIELD(color2_, color2, uint32_t);
  SIMDATA_DECLARE_FIELD(stipple1_, stipple1, uint32_t);
  SIMDATA_DECLARE_FIELD(stipple2_, stipple2, uint32_t);
  SIMDATA_DECLARE_FIELD(maxDataSeconds_, maxdataseconds, double);
  SIMDATA_DECLARE_FIELD(maxDataPoints_, maxdatapoints, uint32_t);
  SIMDATA_DECLARE_FIELD(lobUseClampAlt_, lobuseclampalt, bool);
  SIMDATA_DECLARE_FIELD(useRangeOverride_, userangeoverride, bool);
  SIMDATA_DECLARE_FIELD(rangeOverrideValue_, rangeoverridevalue, double);
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
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_FIELD(gateLighting_, gatelighting, bool);
  SIMDATA_DECLARE_FIELD(gateBlending_, gateblending, bool);
  SIMDATA_DECLARE_FIELD(gateDrawMode_, gatedrawmode, DrawMode);
  SIMDATA_DECLARE_FIELD(fillPattern_, fillpattern, FillPattern);
  SIMDATA_DECLARE_FIELD(drawCentroid_, drawcentroid, bool);
  SIMDATA_DECLARE_FIELD(interpolateGatePos_, interpolategatepos, bool);
  SIMDATA_DECLARE_FIELD(gateAzimuthOffset_, gateazimuthoffset, double);
  SIMDATA_DECLARE_FIELD(gateElevationOffset_, gateelevationoffset, double);
  SIMDATA_DECLARE_FIELD(gateRollOffset_, gaterolloffset, double);
  SIMDATA_DECLARE_FIELD(drawOutline_, drawoutline, bool);
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

  SIMDATA_DECLARE_DEFAULT_METHODS(AntennaPatterns);
  SIMDATA_DECLARE_FIELD(type_, type, AntennaPatterns::Type);
  SIMDATA_DECLARE_FIELD(fileFormat_, fileformat, AntennaPatterns::FileFormat);
  SIMDATA_DECLARE_FIELD_CONST_REF(fileName_, filename, std::string);
  SIMDATA_DECLARE_FIELD(algorithm_, algorithm, AntennaPatterns::Algorithm);
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

  SIMDATA_DECLARE_METHODS(BeamPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(antennaPattern_, antennapattern, AntennaPatterns);
  SIMDATA_DECLARE_SUBFIELD_LIST(beamPositionOffset_, beampositionoffset, Position);
  SIMDATA_DECLARE_FIELD(shaded_, shaded, bool);
  SIMDATA_DECLARE_FIELD(blended_, blended, bool);
  SIMDATA_DECLARE_FIELD(beamDrawMode_, beamdrawmode, BeamPrefs::DrawMode);
  SIMDATA_DECLARE_FIELD(beamScale_, beamscale, double);
  SIMDATA_DECLARE_FIELD(drawType_, drawtype, BeamPrefs::DrawType);
  SIMDATA_DECLARE_FIELD(capResolution_, capresolution, uint32_t);
  SIMDATA_DECLARE_FIELD(coneResolution_, coneresolution, uint32_t);
  SIMDATA_DECLARE_FIELD(renderCone_, rendercone, bool);
  SIMDATA_DECLARE_FIELD(sensitivity_, sensitivity, double);
  SIMDATA_DECLARE_FIELD(gain_, gain, double);
  SIMDATA_DECLARE_FIELD(fieldOfView_, fieldofview, double);
  SIMDATA_DECLARE_FIELD(detail_, detail, double);
  SIMDATA_DECLARE_FIELD(power_, power, double);
  SIMDATA_DECLARE_FIELD(frequency_, frequency, double);
  SIMDATA_DECLARE_FIELD(polarity_, polarity, Polarity);
  SIMDATA_DECLARE_FIELD(colorScale_, colorscale, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(arepsFile_, arepsfile, std::string);
  SIMDATA_DECLARE_FIELD(channel_, channel, bool);
  SIMDATA_DECLARE_FIELD(weighting_, weighting, bool);
  SIMDATA_DECLARE_FIELD(interpolateBeamPos_, interpolatebeampos, bool);
  SIMDATA_DECLARE_FIELD(useOffsetPlatform_, useoffsetplatform, bool);
  SIMDATA_DECLARE_FIELD(useOffsetIcon_, useoffseticon, bool);
  SIMDATA_DECLARE_FIELD(useOffsetBeam_, useoffsetbeam, bool);
  SIMDATA_DECLARE_FIELD(azimuthOffset_, azimuthoffset, double);
  SIMDATA_DECLARE_FIELD(elevationOffset_, elevationoffset, double);
  SIMDATA_DECLARE_FIELD(rollOffset_, rolloffset, double);
  SIMDATA_DECLARE_FIELD(targetId_, targetid, uint64_t);
  SIMDATA_DECLARE_FIELD(verticalWidth_, verticalwidth, double);
  SIMDATA_DECLARE_FIELD(horizontalWidth_, horizontalwidth, double);
  SIMDATA_DECLARE_FIELD(animate_, animate, bool);
  SIMDATA_DECLARE_FIELD(pulseLength_, pulselength, double);
  SIMDATA_DECLARE_FIELD(pulseRate_, pulserate, double);
  SIMDATA_DECLARE_FIELD(pulseStipple_, pulsestipple, uint32_t);
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
  SIMDATA_DECLARE_FIELD(drawStyle_, drawstyle, DrawStyle);
  SIMDATA_DECLARE_FIELD(color_, color, uint32_t);
  SIMDATA_DECLARE_FIELD(interval_, interval, double);
  SIMDATA_DECLARE_FIELD(largeIntervalFactor_, largeintervalfactor, uint32_t);
  SIMDATA_DECLARE_FIELD(labelIntervalFactor_, labelintervalfactor, uint32_t);
  SIMDATA_DECLARE_FIELD_CONST_REF(labelFontName_, labelfontname, std::string);
  SIMDATA_DECLARE_FIELD(labelFontPointSize_, labelfontpointsize, uint32_t);
  SIMDATA_DECLARE_FIELD(labelColor_, labelcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(lineLength_, linelength, double);
  SIMDATA_DECLARE_FIELD(largeSizeFactor_, largesizefactor, uint32_t);
  SIMDATA_DECLARE_FIELD(labelTimeFormat_, labeltimeformat, ElapsedTimeFormat);
  SIMDATA_DECLARE_FIELD(lineWidth_, linewidth, double);
};

/** Track preferences */
class SDKDATA_EXPORT TrackPrefs : public FieldList
{
public:
  /**  mode for drawing track position */
  enum class Mode {
    OFF = 0,     ///< nothing is drawn
    POINT = 1,   ///< draw a point at each previous position
    LINE = 2,    ///< draw a line connecting previous positions
    RIBBON = 3,  ///< a line with wings showing roll
    BRIDGE = 4   ///< "ribbon" plus a drop line down to the ground
  };

  SIMDATA_DECLARE_METHODS(TrackPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(timeTicks_, timeticks, TimeTickPrefs);
  SIMDATA_DECLARE_FIELD(trackColor_, trackcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(multiTrackColor_, multitrackcolor, bool);
  SIMDATA_DECLARE_FIELD(flatMode_, flatmode, bool);
  SIMDATA_DECLARE_FIELD(altMode_, altmode, bool);
  SIMDATA_DECLARE_FIELD(expireMode_, expiremode, bool);
  SIMDATA_DECLARE_FIELD(usePlatformColor_, useplatformcolor, bool);
  SIMDATA_DECLARE_FIELD(useTrackOverrideColor_, usetrackoverridecolor, bool);
  SIMDATA_DECLARE_FIELD(trackOverrideColor_, trackoverridecolor, uint32_t);
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
  SIMDATA_DECLARE_SUBFIELD_LIST(commonPrefs_, commonprefs, CommonPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(trackPrefs_, trackprefs, TrackPrefs);
  SIMDATA_DECLARE_SUBFIELD_LIST(platPositionOffset_, platpositionoffset, Position);
  SIMDATA_DECLARE_SUBFIELD_LIST(orientationOffset_, orientationoffset, BodyOrientation);
  SIMDATA_DECLARE_SUBFIELD_LIST(scaleXYZ_, scalexyz, Position);
  SIMDATA_DECLARE_FIELD_CONST_REF(icon_, icon, std::string);
  SIMDATA_DECLARE_FIELD(drawMode_, drawmode, ModelDrawMode);
  SIMDATA_DECLARE_FIELD(fragmentEffect_, fragmenteffect, FragmentEffect);
  SIMDATA_DECLARE_FIELD(fragmentEffectColor_, fragmenteffectcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(rotateIcons_, rotateicons, IconRotation);
  SIMDATA_DECLARE_FIELD(noDepthIcons_, nodepthicons, bool);
  SIMDATA_DECLARE_FIELD(iconAlignment_, iconalignment, TextAlignment);
  SIMDATA_DECLARE_FIELD(overrideColorCombineMode_, overridecolorcombinemode, OverrideColorCombineMode);
  SIMDATA_DECLARE_FIELD(useClampAlt_, useclampalt, bool);
  SIMDATA_DECLARE_FIELD(clampValAltMin_, clampvalaltmin, double);
  SIMDATA_DECLARE_FIELD(clampValAltMax_, clampvalaltmax, double);
  SIMDATA_DECLARE_FIELD(useClampYaw_, useclampyaw, bool);
  SIMDATA_DECLARE_FIELD(clampValYaw_, clampvalyaw, double);
  SIMDATA_DECLARE_FIELD(useClampPitch_, useclamppitch, bool);
  SIMDATA_DECLARE_FIELD(clampValPitch_, clampvalpitch, double);
  SIMDATA_DECLARE_FIELD(useClampRoll_, useclamproll, bool);
  SIMDATA_DECLARE_FIELD(clampValRoll_, clampvalroll, double);
  SIMDATA_DECLARE_FIELD(clampOrientationAtLowVelocity_, clamporientationatlowvelocity, bool);
  SIMDATA_DECLARE_FIELD(surfaceClamping_, surfaceclamping, bool);
  SIMDATA_DECLARE_FIELD(aboveSurfaceClamping_, abovesurfaceclamping, bool);
  SIMDATA_DECLARE_FIELD(lighted_, lighted, bool);
  SIMDATA_DECLARE_FIELD(drawBox_, drawbox, bool);
  SIMDATA_DECLARE_FIELD(drawBodyAxis_, drawbodyaxis, bool);
  SIMDATA_DECLARE_FIELD(drawInertialAxis_, drawinertialaxis, bool);
  SIMDATA_DECLARE_FIELD(drawSunVec_, drawsunvec, bool);
  SIMDATA_DECLARE_FIELD(drawMoonVec_, drawmoonvec, bool);
  SIMDATA_DECLARE_FIELD(axisScale_, axisscale, double);
  SIMDATA_DECLARE_FIELD(wireFrame_, wireframe, bool);
  SIMDATA_DECLARE_FIELD(drawOpticLos_, drawopticlos, bool);
  SIMDATA_DECLARE_FIELD(drawRfLos_, drawrflos, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(rcsFile_, rcsfile, std::string);
  SIMDATA_DECLARE_FIELD(drawRcs_, drawrcs, bool);
  SIMDATA_DECLARE_FIELD(draw3dRcs_, draw3drcs, bool);
  SIMDATA_DECLARE_FIELD(rcsColor_, rcscolor, uint32_t);
  SIMDATA_DECLARE_FIELD(rcsColorScale_, rcscolorscale, bool);
  SIMDATA_DECLARE_FIELD(rcsPolarity_, rcspolarity, Polarity);
  SIMDATA_DECLARE_FIELD(rcsElevation_, rcselevation, double);
  SIMDATA_DECLARE_FIELD(rcsFrequency_, rcsfrequency, double);
  SIMDATA_DECLARE_FIELD(rcsDetail_, rcsdetail, double);
  SIMDATA_DECLARE_FIELD(drawCircleHilight_, drawcirclehilight, bool);
  SIMDATA_DECLARE_FIELD(circleHilightColor_, circlehilightcolor, uint32_t);
  SIMDATA_DECLARE_FIELD(circleHilightShape_, circlehilightshape, CircleHilightShape);
  SIMDATA_DECLARE_FIELD(circleHilightSize_, circlehilightsize, double);
  SIMDATA_DECLARE_FIELD(hilightFollowYaw_, hilightfollowyaw, bool);
  SIMDATA_DECLARE_FIELD(interpolatePos_, interpolatepos, bool);
  SIMDATA_DECLARE_FIELD(extrapolatePos_, extrapolatepos, bool);
  SIMDATA_DECLARE_FIELD(scale_, scale, double);
  SIMDATA_DECLARE_FIELD(brightness_, brightness, int32_t);
  SIMDATA_DECLARE_FIELD(dynamicScale_, dynamicscale, bool);
  SIMDATA_DECLARE_FIELD(dynamicScaleOffset_, dynamicscaleoffset, double);
  SIMDATA_DECLARE_FIELD(dynamicScaleScalar_, dynamicscalescalar, double);
  SIMDATA_DECLARE_FIELD(dynamicScaleAlgorithm_, dynamicscalealgorithm, DynamicScaleAlgorithm);
  SIMDATA_DECLARE_FIELD(drawVelocityVec_, drawvelocityvec, bool);
  SIMDATA_DECLARE_FIELD(velVecColor_, velveccolor, uint32_t);
  SIMDATA_DECLARE_FIELD(velVecUseStaticLength_, velvecusestaticlength, bool);
  SIMDATA_DECLARE_FIELD(velVecStaticLen_, velvecstaticlen, double);
  SIMDATA_DECLARE_FIELD(velVecStaticLenUnits_, velvecstaticlenunits, DistanceUnits);
  SIMDATA_DECLARE_FIELD(velVecTime_, velvectime, double);
  SIMDATA_DECLARE_FIELD(velVecTimeUnits_, velvectimeunits, ElapsedTimeFormat);
  SIMDATA_DECLARE_VECTOR_FIELD(gogFile_, gogfile, std::string);
  SIMDATA_DECLARE_FIELD(alphaVolume_, alphavolume, bool);
  SIMDATA_DECLARE_FIELD(useCullFace_, usecullface, bool);
  SIMDATA_DECLARE_FIELD(cullFace_, cullface, PolygonFace);
  SIMDATA_DECLARE_FIELD(polygonModeFace_, polygonmodeface, PolygonFace);
  SIMDATA_DECLARE_FIELD(polygonMode_, polygonmode, PolygonMode);
  SIMDATA_DECLARE_FIELD(usePolygonStipple_, usepolygonstipple, bool);
  SIMDATA_DECLARE_FIELD(polygonStipple_, polygonstipple, uint32_t);
  SIMDATA_DECLARE_FIELD(visibleLosColor_, visibleloscolor, uint32_t);
  SIMDATA_DECLARE_FIELD(obstructedLosColor_, obstructedloscolor, uint32_t);
  SIMDATA_DECLARE_FIELD(losRangeResolution_, losrangeresolution, double);
  SIMDATA_DECLARE_FIELD(losAzimuthalResolution_, losazimuthalresolution, double);
  SIMDATA_DECLARE_FIELD(losAltitudeOffset_, losaltitudeoffset, double);
  SIMDATA_DECLARE_FIELD(animateDofNodes_, animatedofnodes, bool);
  SIMDATA_DECLARE_FIELD(eciDataMode_, ecidatamode, bool);
  SIMDATA_DECLARE_FIELD(drawOffBehavior_, drawoffbehavior, PlatformPrefs::DrawOffBehavior);
  SIMDATA_DECLARE_FIELD(lifespanMode_, lifespanmode, LifespanMode);
};

}

#endif
