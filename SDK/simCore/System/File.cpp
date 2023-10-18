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

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#endif

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

///////////////////////////////////////////////////////////////

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

int mkdir(const std::string& path, bool makeParents)
{
  std::error_code unused;
  if (makeParents)
    return std::filesystem::create_directories(path, unused) ? 0 : 1;
  return std::filesystem::create_directory(path, unused) ? 0 : 1;
}

int remove(const std::string& path, bool recursive)
{
  std::error_code unused;
  if (recursive)
  {
    const auto rv = std::filesystem::remove_all(path, unused);
    if (rv == 0 || rv == static_cast<std::uintmax_t>(-1))
      return 1;
    return 0;
  }
  return std::filesystem::remove(path, unused) ? 0 : 1;
}

int recycle(const std::string& path)
{
#ifdef WIN32
  // Based off code from http://www.codeproject.com/KB/shell/SHFileOperation_Demo.aspx
  SHFILEOPSTRUCT SHFileOp;
  ZeroMemory(&SHFileOp, sizeof(SHFILEOPSTRUCT));
  SHFileOp.hwnd = NULL;
  SHFileOp.wFunc = FO_DELETE;

  SHFileOp.pFrom = path.c_str();
  SHFileOp.fFlags = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
  return SHFileOperation(&SHFileOp);
#else
  return simCore::remove(path);
#endif
}

}
