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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_STRING_UTFUTILS_H
#define SIMCORE_STRING_UTFUTILS_H

#include <string>

// MSVC 2015+ required for wide string versions
#if (defined(_MSC_VER) && _MSC_VER >= 1900)

#include <codecvt>

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
  // Based on solution from https://stackoverflow.com/questions/4358870
  std::wstring_convert<std::codecvt_utf8<wchar_t> > conv;
  return conv.from_bytes(utf8);
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

#endif /* SIMCORE_STRING_UTFUTILS_H */
