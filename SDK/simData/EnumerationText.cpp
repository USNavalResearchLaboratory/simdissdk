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

#include <cassert>
#include "EnumerationText.h"

namespace simData
{

EnumerationText::EnumerationText()
{
}

EnumerationText::~EnumerationText()
{
}

std::string EnumerationText::text(size_t index) const
{
  auto it = text_.find(index);

  if (it == text_.end())
  {
    // passed in an invalid index
    assert(false);
    return "Invalid Index";
  }

  return it->second;
}

void EnumerationText::insert_(size_t index, const std::string& text)
{
  // No duplicates
  assert(text_.find(index) == text_.end());

  text_[index] = text;
}

void EnumerationText::append_(const std::string& text)
{
  if (text_.empty())
  {
    // Must set the initial index value by calling insert_() before calling append_()
    assert(false);
    return;
  }

  insert_(text_.rbegin()->first + 1, text);
}

std::unique_ptr<EnumerationText> EnumerationText::makeBeamTypeName()
{
  auto rv = std::make_unique<EnumerationText>();
  rv->insert_(1, "ABSOLUTE_POSITION"); ///< Beam pointing is defined based on data
  rv->append_("BODY_RELATIVE");        ///< Beam pointing is relative to host's body orientation
  rv->append_("TARGET");               ///< Beam pointing is towards specified target platform

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

}
