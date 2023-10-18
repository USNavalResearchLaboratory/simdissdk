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
#include <filesystem>
#include "simCore/System/File.h"

namespace simCore {

FileInfo::FileInfo(const std::string& path)
  : path_(path)
{
}

bool FileInfo::exists() const
{
  std::error_code unused;
  return std::filesystem::exists(path_, unused);
}

bool FileInfo::isRegularFile() const
{
  std::error_code unused;
  return std::filesystem::is_regular_file(path_, unused);
}

bool FileInfo::isDirectory() const
{
  std::error_code unused;
  return std::filesystem::is_directory(path_, unused);
}

std::string pathJoin(const std::string& path1, const std::string& path2)
{
  return pathJoin({ path1, path2 });
}

std::string pathJoin(const std::vector<std::string>& pathSegments)
{
  // Windows permits forward and backward slash separators. Linux only supports forward slash.
  constexpr auto isSeparator = [](char value) constexpr {
    return value == '/'
#ifdef WIN32
      || value == '\\'  // Not a path separator on Linux
#endif
      ; };

  std::string rv;
  for (const auto& segment : pathSegments)
  {
    // Ignore empty segments, but follow Python's path join behavior in appending separator if non-empty
    if (segment.empty())
    {
      if (!rv.empty() && !isSeparator(rv.back()))
        rv += simCore::PATH_SEPARATOR;
      continue;
    }

    if (!rv.empty())
    {
      // Follow Python's path join behavior by clearing input if new segment is absolute
      if (isSeparator(segment.front()))
        rv.clear();
      else if (!isSeparator(rv.back()))
        rv += simCore::PATH_SEPARATOR;
    }
    rv += segment;
  }
  return rv;
}


}
