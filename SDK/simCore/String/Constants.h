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
#ifndef SIMCORE_STRING_CONSTANTS_H
#define SIMCORE_STRING_CONSTANTS_H

#include <string>

namespace simCore
{
  /// Common white space characters encountered during string parsing
  static const std::string STR_WHITE_SPACE_CHARS = " \n\r\t";

  // Character constants for degree symbol; Qt requires UTF8. 
  static const std::string STR_DEGREE_SYMBOL_ASCII = "\xB0";      ///< ASCII degree symbol value
  static const std::string STR_DEGREE_SYMBOL_UNICODE = "\u00B0";  ///< Unicode degree symbol value
  static const std::string STR_DEGREE_SYMBOL_UTF8 = "\xC2\xB0";   ///< UTF-8 degree symbol value

} // namespace simCore

#endif /* SIMCORE_STRING_CONSTANTS_H */
