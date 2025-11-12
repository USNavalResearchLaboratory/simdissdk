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

void EnumerationText::insert_(size_t value, const std::string& text)
{
  // No duplicates
  assert(text_.find(value) == text_.end());

  text_[value] = text;
  values_.push_back(value);
}

void EnumerationText::append_(const std::string& text)
{
  if (text_.empty())
  {
    // Must set the initial value by calling insert_() before calling append_()
    assert(false);
    return;
  }

  insert_(text_.rbegin()->first + 1, text);
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "WIRE");
  rv->append_("SOLID");
  rv->append_("WIRE_ON_SOLID");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamDrawTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "BEAM_3DB");      ///< Beam drawn using 3 dB half power points
  rv->append_("ANTENNA_PATTERN");  ///< Beam drawn using antenna pattern
  rv->append_("COVERAGE");         ///< Beam drawn as a spherical slice (cap only)
  rv->append_("LINE");             ///< Beam drawn as a line

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "ABSOLUTE_POSITION"); ///< Beam pointing is defined based on data
  rv->append_("BODY_RELATIVE");        ///< Beam pointing is relative to host's body orientation
  rv->append_("TARGET");               ///< Beam pointing is towards specified target platform

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamRangeMode()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "BEAM_UPDATE"); ///< Beam uses range as provided in beam update
  rv->append_("ONE-WAY_FREE_SPACE");  ///< Beam uses calculated one-way free-space range
  rv->append_("TWO-WAY_FREE_SPACE");  ///< Beam uses calculated two-way free-space range

  return rv;
}
std::unique_ptr<EnumerationText> EnumerationText::makeGateDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "UNKNOWN");
  rv->append_("RANGE");
  rv->append_("GUARD");
  rv->insert_(4, "ANGLE");
  rv->append_("RAIN");
  rv->append_("CLUTTER");
  rv->append_("FOOTPRINT");
  rv->append_("SECTOR");
  rv->append_("PUSH");
  rv->append_("COVERAGE");  ///< Gate rendered as a spherical slice

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGateFillPatternName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "STIPPLE");
  rv->append_("SOLID");
  rv->append_("ALPHA");
  rv->append_("WIRE");
  rv->append_("CENTROID");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGateTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "ABSOLUTE_POSITION"); ///< Gate pointing is defined based on data
  rv->append_("BODY_RELATIVE");        ///< Gate pointing is relative to host's body orientation
  rv->append_("TARGET");               ///< Gate pointing is towards specified target platform

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeCoordinateSystemName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "NED"); ///< North/East/Down
  rv->append_("NWU");    ///< North/West/Up
  rv->append_("ENU");    ///< East/North/Up
  rv->append_("LLA");    ///< Lat/Lon/Alt
  rv->append_("ECEF");   ///< Earth-centered, Earth-fixed (stationary frame)
  rv->append_("ECI");    ///< Earth-centered, inertial (rotates in time)
  rv->append_("XEAST");  ///< Tangent plane, X-axis pointing East
  rv->append_("GTP");    ///< Generic tangent plane that can be rotated and/or translated)

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeMagneticVarianceName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "MV_WMM"); ///< Variance based on World Magnetic Model (WMM)
  rv->append_("MV_TRUE");   ///< No variance, also known as True North
  rv->append_("MV_USER");   ///< User defined variance

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeVerticalDatumName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "VD_WGS84"); ///< Referenced to WGS-84 ellipsoid
  rv->append_("VD_MSL");      ///< Referenced to Earth Gravity Model (EGM)
  rv->append_("VD_USER");     ///< User defined datum

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTextOutlineName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "TO_NONE");
  rv->append_("TO_THIN");
  rv->append_("TO_THICK");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTimeTickDrawStyleName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "NONE");
  rv->append_("POINT");
  rv->append_("LINE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTrackModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "OFF");
  rv->append_("POINT");
  rv->append_("LINE");
  rv->append_("RIBBON");
  rv->append_("BRIDGE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBackdropTypeName()
{
  auto rv = std::make_unique<EnumerationText>();

  rv->insert_(0, "BDT_SHADOW_BOTTOM_RIGHT");
  rv->append_("BDT_SHADOW_CENTER_RIGHT");
  rv->append_("BDT_SHADOW_TOP_RIGHT");
  rv->append_("BDT_SHADOW_BOTTOM_CENTER");
  rv->append_("BDT_SHADOW_TOP_CENTER");
  rv->append_("BDT_SHADOW_BOTTOM_LEFT");
  rv->append_("BDT_SHADOW_CENTER_LEFT");
  rv->append_("BDT_SHADOW_TOP_LEFT");
  rv->append_("BDT_OUTLINE");
  rv->append_("BDT_NONE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeBackdropImplementationName()
{
  auto rv = std::make_unique<EnumerationText>();

  rv->insert_(0, "BDI_POLYGON_OFFSET");
  rv->append_("BDI_NO_DEPTH_BUFFER");
  rv->append_("BDI_DEPTH_RANGE");
  rv->append_("BDI_STENCIL_BUFFER");
  rv->append_("BDI_DELAYED_DEPTH_WRITES");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeTextAlignmentName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "ALIGN_LEFT_TOP");
  rv->append_("ALIGN_LEFT_CENTER");
  rv->append_("ALIGN_LEFT_BOTTOM");
  rv->append_("ALIGN_CENTER_TOP");
  rv->append_("ALIGN_CENTER_CENTER");
  rv->append_("ALIGN_CENTER_BOTTOM");
  rv->append_("ALIGN_RIGHT_TOP");
  rv->append_("ALIGN_RIGHT_CENTER");
  rv->append_("ALIGN_RIGHT_BOTTOM");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeElapsedTimeFormatName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "ELAPSED_SECONDS");
  rv->append_("ELAPSED_MINUTES");
  rv->append_("ELAPSED_HOURS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAngleUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(10, "UNITS_RADIANS");
  rv->append_("UNITS_DEGREES");
  rv->append_("UNITS_DEGREES_MINUTES");
  rv->append_("UNITS_DEGREES_MINUTES_SECONDS");
  rv->append_("UNITS_UTM");
  rv->append_("UNITS_BAM");
  rv->append_("UNITS_MIL");
  rv->append_("UNITS_MILLIRADIANS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAnimatedLineBendName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "ALB_AUTO");
  rv->append_("ALB_STRAIGHT");
  rv->append_("ALB_BEND");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternAlgorithmName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "PEDESTAL");
  rv->append_("GAUSS");
  rv->append_("CSCSQ");
  rv->append_("SINXX");
  rv->append_("OMNI");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternFileFormatName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(6, "TABLE");
  rv->append_("MONOPULSE");
  rv->insert_(9, "RELATIVE_TABLE");
  rv->append_("BILINEAR");
  rv->append_("NSMA");
  rv->append_("EZNEC");
  rv->append_("XFDTD");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeAntennaPatternTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "NONE");
  rv->append_("FILE");
  rv->append_("ALGORITHM");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeGeodeticUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(11, "GEODETIC_DEGREES");
  rv->append_("GEODETIC_DEGREES_MINUTES");
  rv->append_("GEODETIC_DEGREES_MINUTES_SECONDS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeDistanceUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(20, "UNITS_METERS");
  rv->append_("UNITS_KILOMETERS");
  rv->append_("UNITS_YARDS");
  rv->append_("UNITS_MILES");
  rv->append_("UNITS_FEET");
  rv->append_("UNITS_INCHES");
  rv->append_("UNITS_NAUTICAL_MILES");
  rv->append_("UNITS_CENTIMETERS");
  rv->append_("UNITS_MILLIMETERS");
  rv->append_("UNITS_KILOYARDS");
  rv->append_("UNITS_DATAMILES");
  rv->append_("UNITS_FATHOMS");
  rv->append_("UNITS_KILOFEET");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeSpeedUnitsName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(40, "UNITS_METERS_PER_SECOND");
  rv->append_("UNITS_KILOMETERS_PER_HOUR");
  rv->append_("UNITS_KNOTS");
  rv->append_("UNITS_MILES_PER_HOUR");
  rv->append_("UNITS_FEET_PER_SECOND");
  // Note: Index 45 is reserved and not for public use.
  rv->insert_(46, "UNITS_KILOMETERS_PER_SECOND");
  rv->append_("UNITS_DATAMILES_PER_HOUR");
  rv->append_("UNITS_YARDS_PER_SECOND");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolarityName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "POL_UNKNOWN");
  rv->append_("POL_HORIZONTAL");
  rv->append_("POL_VERTICAL");
  rv->append_("POL_CIRCULAR");
  rv->append_("POL_HORZVERT");
  rv->append_("POL_VERTHORZ");
  rv->append_("POL_LEFTCIRC");
  rv->append_("POL_RIGHTCIRC");
  rv->append_("POL_LINEAR");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeVolumeTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "GAIN_AS_RANGE_SCALAR");
  rv->append_("FREE_SPACE_RANGE_LINEAR");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeModelDrawModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "MDM_SOLID");
  rv->append_("MDM_WIRE");
  rv->append_("MDM_POINTS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeIconRotationName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "IR_2D_UP");
  rv->append_("IR_2D_YAW");
  rv->append_("IR_3D_YPR");
  rv->append_("IR_3D_NORTH");
  rv->append_("IR_3D_YAW");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeUseValueName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "ACTUAL_VALUE");
  rv->append_("DISPLAY_VALUE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeLocalGridTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "CARTESIAN");
  rv->append_("POLAR");
  rv->append_("RANGE_RINGS");
  rv->append_("SPEED_RINGS");
  rv->append_("SPEED_LINE");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeFragmentEffectName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "FE_NONE");
  rv->append_("FE_FORWARD_STRIPE");
  rv->append_("FE_BACKWARD_STRIPE");
  rv->append_("FE_HORIZONTAL_STRIPE");
  rv->append_("FE_VERTICAL_STRIPE");
  rv->append_("FE_CHECKERBOARD");
  rv->append_("FE_DIAMOND");
  rv->append_("FE_GLOW");
  rv->append_("FE_FLASH");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeOverrideColorCombineModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "MULTIPLY_COLOR");
  rv->append_("REPLACE_COLOR");
  rv->append_("INTENSITY_GRADIENT");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeLifespanModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "LIFE_FIRST_LAST_POINT");
  rv->append_("LIFE_EXTEND_SINGLE_POINT");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeCircleHilightShapeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "CH_PULSING_CIRCLE");
  rv->append_("CH_CIRCLE");
  rv->append_("CH_DIAMOND");
  rv->append_("CH_SQUARE");
  rv->append_("CH_SQUARE_RETICLE");
  rv->append_("CH_COFFIN");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolygonFaceName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "FRONT_AND_BACK");
  rv->append_("FRONT");
  rv->append_("BACK");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePolygonModeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0x1B00, "POINT");
  rv->insert_(0x1B01, "LINE");
  rv->insert_(0x1B02, "FILL");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makeDynamicScaleAlgorithmName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "DSA_CONSISTENT_SIZING");
  rv->append_("DSA_METERS_TO_PIXELS");

  return rv;
}

std::unique_ptr<EnumerationText> EnumerationText::makePlatformDrawOffBehaviorName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(0, "DEFAULT_BEHAVIOR");
  rv->append_("OMIT_CHILDREN_AND_VIS_UPDATE");

  return rv;
}

}
