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
#include "simVis/GOG/GOGNode.h"
#include "simVis/Locator.h"

namespace simVis { namespace GOG
{

GogFollowData::GogFollowData()
: locatorFlags(simVis::Locator::COMP_NONE)
{
}

GogMetaData::GogMetaData()
 : metadata(""),
   shape(GOG_UNKNOWN),
   loadFormat(FORMAT_GOG),
   altitudeUnits_(simCore::Units::FEET),
   setFields_(0),
   allowingSetExplicitly_(true)
{
}

bool GogMetaData::isSetExplicitly(GogSerializableField field) const
{
  return ((setFields_ & (0x01 << field)) != 0);
}

void GogMetaData::setExplicitly(GogSerializableField field)
{
  if (allowingSetExplicitly_)
    setFields_ |= (0x01 << field);
  }

void GogMetaData::clearSetFields()
{
  setFields_ = GOG_ALL_DEFAULTS;
}

void GogMetaData::allowSetExplicitly(bool allow)
{
  allowingSetExplicitly_ = allow;
}

}}

