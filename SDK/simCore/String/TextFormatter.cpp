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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/String/TextFormatter.h"

namespace simCore {

std::string TextFormatter::formatRGBA(unsigned int rgba) const
{
  return formatRGBA((rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff, rgba & 0xff);
}

std::string TextFormatter::formatABGR(unsigned int abgr) const
{
  return formatRGBA(abgr & 0xff, (abgr >> 8) & 0xff, (abgr >> 16) & 0xff, (abgr >> 24) & 0xff);
}

}
