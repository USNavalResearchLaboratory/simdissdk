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
#ifndef SIMCORE_STRING_UTFUTILS_H
#define SIMCORE_STRING_UTFUTILS_H

#include <istream>
#include <string>
#include "simCore/Common/Export.h"

// MSVC 2015+ required for wide string versions
#if (defined(_MSC_VER) && _MSC_VER >= 1900)

#include <windows.h>

namespace simCore {

/**
 * Most (nearly all?) SIMDIS strings are UTF-8 encoded.  However, under Windows MSVC the C standard
 * library will not open filenames or directories using UTF-8 strings, and require wide strings.
 * The opposite is true on Linux gcc; wide strings cannot be used to open files but UTF-8 strings
 * work.  This function returns an fstream-appropriate string regardless of OS.  This same string
 * can be used with fstream, ifstream, ofstream, fopen, and many other POSIX functions.  MSVC provides
 * wide character implementations for each.
 */
inline std::wstring streamFixUtf8(const std::string& utf8)
{
  // Based on solution from https://stackoverflow.com/questions/14184709
  std::wstring wide;
  if (utf8.empty())
    return wide;
  const int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
  // Error out with empty string
  if (len <= 0)
    return wide;

  wide.resize(static_cast<size_t>(len));
  // No need to error check, because of earlier call retrieving length
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), &wide[0], len);
  return wide;
}

}

#else

namespace simCore {

/** Since UNIX is OK with UTF-8 std::string, return the input string.  Note difference in return value type. */
inline std::string streamFixUtf8(const std::string& utf8)
{
  return utf8;
}

}

#endif

namespace simCore {

/**
 * Skips past a UTF-8 BOM, presumed to be called at start of file. Use this method on an input
 * stream that you suspect might be encoded in UTF-8 with a UTF-8 byte order mark.
 * @return 0 on successful skip. 1 if the value is not present. Non-zero does not mean error.
 */
SDKCORE_EXPORT int skipUtf8ByteOrderMark(std::istream& is);

}

#endif /* SIMCORE_STRING_UTFUTILS_H */
