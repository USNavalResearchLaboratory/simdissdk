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

#include <algorithm>
#include <cassert>
#include <limits>
#include "EnumerationText.h"

namespace simData
{

EnumerationText::EnumerationText()
{
}

EnumerationText::~EnumerationText()
{
}

std::string EnumerationText::text(size_t value) const
{
  auto it = text_.find(value);

  if (it == text_.end())
  {
    // passed in an invalid index
    assert(false);
    return "Invalid Index";
  }

  return it->second;
}

size_t EnumerationText::valueToIndex(size_t value) const
{
  auto it = std::ranges::find(values_, value);
  if (it == values_.end())
    return std::numeric_limits<size_t>::max();

  return std::distance(values_.begin(), it);
}

size_t EnumerationText::indexToValue(size_t index) const
{
  if (index >= values_.size())
    return std::numeric_limits<size_t>::max();

  return values_[index];
}

void EnumerationText::visit(VisitorFn fn) const
{
  for (const auto& [index, enumText] : text_)
    fn(index, enumText);
}

void EnumerationText::insert(size_t value, const std::string& text)
{
  // No duplicates
  assert(text_.find(value) == text_.end());

  text_[value] = text;
  values_.push_back(value);
}

void EnumerationText::append(const std::string& text)
{
  if (text_.empty())
  {
    // Must set the initial value by calling insert() before calling append()
    assert(false);
    return;
  }

  insert(text_.rbegin()->first + 1, text);
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "WIRE");
  rv->append("SOLID");
  rv->append("WIRE_ON_SOLID");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamDrawTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "BEAM_3DB");      ///< Beam drawn using 3 dB half power points
  rv->append("ANTENNA_PATTERN");  ///< Beam drawn using antenna pattern
  rv->append("COVERAGE");         ///< Beam drawn as a spherical slice (cap only)
  rv->append("LINE");             ///< Beam drawn as a line

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "ABSOLUTE_POSITION"); ///< Beam pointing is defined based on data
  rv->append("BODY_RELATIVE");        ///< Beam pointing is relative to host's body orientation
  rv->append("TARGET");               ///< Beam pointing is towards specified target platform

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamRangeMode()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "BEAM_UPDATE"); ///< Beam uses range as provided in beam update
  rv->append("ONE_WAY_FREE_SPACE");  ///< Beam uses calculated one-way free-space range
  rv->append("TWO_WAY_FREE_SPACE");  ///< Beam uses calculated two-way free-space range

  return rv;
}
std::unique_ptr<EnumerationText> EnumerationText::makeGateDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "UNKNOWN");
  rv->append("RANGE");
  rv->append("GUARD");
  rv->insert(4, "ANGLE");
  rv->append("RAIN");
  rv->append("CLUTTER");
  rv->append("FOOTPRINT");
  rv->append("SECTOR");
  rv->append("PUSH");
  rv->append("COVERAGE");  ///< Gate rendered as a spherical slice

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGateFillPatternName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "STIPPLE");
  rv->append("SOLID");
  rv->append("ALPHA");
  rv->append("WIRE");
  rv->append("CENTROID");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGateTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "ABSOLUTE_POSITION"); ///< Gate pointing is defined based on data
  rv->append("BODY_RELATIVE");        ///< Gate pointing is relative to host's body orientation
  rv->append("TARGET");               ///< Gate pointing is towards specified target platform

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeCoordinateSystemName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "NED"); ///< North/East/Down
  rv->append("NWU");    ///< North/West/Up
  rv->append("ENU");    ///< East/North/Up
  rv->append("LLA");    ///< Lat/Lon/Alt
  rv->append("ECEF");   ///< Earth-centered, Earth-fixed (stationary frame)
  rv->append("ECI");    ///< Earth-centered, inertial (rotates in time)
  rv->append("XEAST");  ///< Tangent plane, X-axis pointing East
  rv->append("GTP");    ///< Generic tangent plane that can be rotated and/or translated)

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeMagneticVarianceName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "MV_WMM"); ///< Variance based on World Magnetic Model (WMM)
  rv->append("MV_TRUE");   ///< No variance, also known as True North
  rv->append("MV_USER");   ///< User defined variance

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeVerticalDatumName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "VD_WGS84"); ///< Referenced to WGS-84 ellipsoid
  rv->append("VD_MSL");      ///< Referenced to Earth Gravity Model (EGM)
  rv->append("VD_USER");     ///< User defined datum

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTextOutlineName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "TO_NONE");
  rv->append("TO_THIN");
  rv->append("TO_THICK");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTimeTickDrawStyleName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "NONE");
  rv->append("POINT");
  rv->append("LINE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTrackModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "OFF");
  rv->append("POINT");
  rv->append("LINE");
  rv->append("RIBBON");
  rv->append("BRIDGE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBackdropTypeName()
{
  auto rv = std::make_unique<EnumerationText>();

  rv->insert(0, "BDT_SHADOW_BOTTOM_RIGHT");
  rv->append("BDT_SHADOW_CENTER_RIGHT");
  rv->append("BDT_SHADOW_TOP_RIGHT");
  rv->append("BDT_SHADOW_BOTTOM_CENTER");
  rv->append("BDT_SHADOW_TOP_CENTER");
  rv->append("BDT_SHADOW_BOTTOM_LEFT");
  rv->append("BDT_SHADOW_CENTER_LEFT");
  rv->append("BDT_SHADOW_TOP_LEFT");
  rv->append("BDT_OUTLINE");
  rv->append("BDT_NONE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBackdropImplementationName()
{
  auto rv = std::make_unique<EnumerationText>();

  rv->insert(0, "BDI_POLYGON_OFFSET");
  rv->append("BDI_NO_DEPTH_BUFFER");
  rv->append("BDI_DEPTH_RANGE");
  rv->append("BDI_STENCIL_BUFFER");
  rv->append("BDI_DELAYED_DEPTH_WRITES");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTextAlignmentName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "ALIGN_LEFT_TOP");
  rv->append("ALIGN_LEFT_CENTER");
  rv->append("ALIGN_LEFT_BOTTOM");
  rv->append("ALIGN_CENTER_TOP");
  rv->append("ALIGN_CENTER_CENTER");
  rv->append("ALIGN_CENTER_BOTTOM");
  rv->append("ALIGN_RIGHT_TOP");
  rv->append("ALIGN_RIGHT_CENTER");
  rv->append("ALIGN_RIGHT_BOTTOM");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeElapsedTimeFormatName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "ELAPSED_SECONDS");
  rv->append("ELAPSED_MINUTES");
  rv->append("ELAPSED_HOURS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAngleUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(10, "UNITS_RADIANS");
  rv->append("UNITS_DEGREES");
  rv->append("UNITS_DEGREES_MINUTES");
  rv->append("UNITS_DEGREES_MINUTES_SECONDS");
  rv->append("UNITS_UTM");
  rv->append("UNITS_BAM");
  rv->append("UNITS_MIL");
  rv->append("UNITS_MILLIRADIANS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAnimatedLineBendName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "ALB_AUTO");
  rv->append("ALB_STRAIGHT");
  rv->append("ALB_BEND");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternAlgorithmName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "PEDESTAL");
  rv->append("GAUSS");
  rv->append("CSCSQ");
  rv->append("SINXX");
  rv->append("OMNI");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternFileFormatName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(6, "TABLE");
  rv->append("MONOPULSE");
  rv->insert(9, "RELATIVE_TABLE");
  rv->append("BILINEAR");
  rv->append("NSMA");
  rv->append("EZNEC");
  rv->append("XFDTD");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "NONE");
  rv->append("FILE");
  rv->append("ALGORITHM");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGeodeticUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(11, "GEODETIC_DEGREES");
  rv->append("GEODETIC_DEGREES_MINUTES");
  rv->append("GEODETIC_DEGREES_MINUTES_SECONDS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeDistanceUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(20, "UNITS_METERS");
  rv->append("UNITS_KILOMETERS");
  rv->append("UNITS_YARDS");
  rv->append("UNITS_MILES");
  rv->append("UNITS_FEET");
  rv->append("UNITS_INCHES");
  rv->append("UNITS_NAUTICAL_MILES");
  rv->append("UNITS_CENTIMETERS");
  rv->append("UNITS_MILLIMETERS");
  rv->append("UNITS_KILOYARDS");
  rv->append("UNITS_DATAMILES");
  rv->append("UNITS_FATHOMS");
  rv->append("UNITS_KILOFEET");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeSpeedUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(40, "UNITS_METERS_PER_SECOND");
  rv->append("UNITS_KILOMETERS_PER_HOUR");
  rv->append("UNITS_KNOTS");
  rv->append("UNITS_MILES_PER_HOUR");
  rv->append("UNITS_FEET_PER_SECOND");
  // Note: Index 45 is reserved and not for public use.
  rv->insert(46, "UNITS_KILOMETERS_PER_SECOND");
  rv->append("UNITS_DATAMILES_PER_HOUR");
  rv->append("UNITS_YARDS_PER_SECOND");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolarityName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "POL_UNKNOWN");
  rv->append("POL_HORIZONTAL");
  rv->append("POL_VERTICAL");
  rv->append("POL_CIRCULAR");
  rv->append("POL_HORZVERT");
  rv->append("POL_VERTHORZ");
  rv->append("POL_LEFTCIRC");
  rv->append("POL_RIGHTCIRC");
  rv->append("POL_LINEAR");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeVolumeTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "GAIN_AS_RANGE_SCALAR");
  rv->append("FREE_SPACE_RANGE_LINEAR");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeModelDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "MDM_SOLID");
  rv->append("MDM_WIRE");
  rv->append("MDM_POINTS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeIconRotationName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "IR_2D_UP");
  rv->append("IR_2D_YAW");
  rv->append("IR_3D_YPR");
  rv->append("IR_3D_NORTH");
  rv->append("IR_3D_YAW");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeUseValueName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "ACTUAL_VALUE");
  rv->append("DISPLAY_VALUE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeLocalGridTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(1, "CARTESIAN");
  rv->append("POLAR");
  rv->append("RANGE_RINGS");
  rv->append("SPEED_RINGS");
  rv->append("SPEED_LINE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeFragmentEffectName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "FE_NONE");
  rv->append("FE_FORWARD_STRIPE");
  rv->append("FE_BACKWARD_STRIPE");
  rv->append("FE_HORIZONTAL_STRIPE");
  rv->append("FE_VERTICAL_STRIPE");
  rv->append("FE_CHECKERBOARD");
  rv->append("FE_DIAMOND");
  rv->append("FE_GLOW");
  rv->append("FE_FLASH");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeOverrideColorCombineModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "MULTIPLY_COLOR");
  rv->append("REPLACE_COLOR");
  rv->append("INTENSITY_GRADIENT");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeLegendBinName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "LEGEND_1");
  rv->append("LEGEND_2");
  rv->append("LEGEND_3");
  rv->append("LEGEND_4");
  rv->append("LEGEND_5");
  rv->append("LEGEND_6");
  rv->append("LEGEND_7");
  rv->append("LEGEND_8");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeLifespanModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "LIFE_FIRST_LAST_POINT");
  rv->append("LIFE_EXTEND_SINGLE_POINT");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeCircleHighlightShapeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "CH_PULSING_CIRCLE");
  rv->append("CH_CIRCLE");
  rv->append("CH_DIAMOND");
  rv->append("CH_SQUARE");
  rv->append("CH_SQUARE_RETICLE");
  rv->append("CH_COFFIN");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolygonFaceName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "FRONT_AND_BACK");
  rv->append("FRONT");
  rv->append("BACK");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolygonModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0x1B00, "POINT");
  rv->insert(0x1B01, "LINE");
  rv->insert(0x1B02, "FILL");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeDynamicScaleAlgorithmName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "DSA_CONSISTENT_SIZING");
  rv->append("DSA_METERS_TO_PIXELS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePlatformDrawOffBehaviorName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert(0, "DEFAULT_BEHAVIOR");
  rv->append("OMIT_CHILDREN_AND_VIS_UPDATE");

  return rv;
}

}
