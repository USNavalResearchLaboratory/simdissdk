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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <vector>
#include "simCore/String/UtfUtils.h"

namespace simCore {

int skipUtf8ByteOrderMark(std::istream& is)
{
  // Byte order mark for UTF-8 BOM files
  static const std::vector<unsigned char> UTF8_BOM = { 0xEF, 0xBB, 0xBF };

  // Eat the UTF-8 BOM, which we can cleanly just remove from the stream
  if (is.peek() == UTF8_BOM[0])
  {
    const auto oldPos = is.tellg();
    is.get(); // eat 0xEF
    const auto bom2 = is.get();
    const auto bom3 = is.get();
    if (bom2 == UTF8_BOM[1] && bom3 == UTF8_BOM[2])
      return 0;
    is.seekg(oldPos);
  }
  return 1;
}

}
