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
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include "simCore/Common/ScopeGuard.h"
#include "simCore/String/Utils.h"
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

bool FileInfo::isEquivalent(const std::string& toPath) const
{
  std::error_code unused;
  return std::filesystem::equivalent(path_, toPath, unused);
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

std::tuple<std::string, std::string> pathSplit(const std::string& path)
{
  static const std::string VALID_PATH_SEP =
#ifdef WIN32
    "/\\";
#else
    "/";
#endif

  const auto pathLastSlash = path.find_last_of(VALID_PATH_SEP);
  // No path separator, return incoming path as the tail
  if (pathLastSlash == std::string::npos)
    return { "", path };

  // At this point, tail is correct, and head may or may not contain trailing slashes
  const std::string& tail = path.substr(pathLastSlash + 1);
  const std::string& head = path.substr(0, pathLastSlash + 1);

  // Strip trailing slashes
  const auto headLastNonSlash = head.find_last_not_of(VALID_PATH_SEP);
  // If it's all slashes, then return head and tail
  if (headLastNonSlash == std::string::npos)
    return { head, tail };

  // Last character should have been a slash. We know there's a non-slash at this
  // point, so there's at least 2 characters left (slash and null).
  assert(headLastNonSlash + 2 <= head.size());
  return { head.substr(0, headLastNonSlash + 1), tail };
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

bool isDirectoryWritable(const std::string& dir)
{
  if (!simCore::FileInfo(dir).isDirectory())
    return false;

  // Loop multiple times to try to get unique directory name
  int numTries = 3;
  while (--numTries >= 0)
  {
    // Skip directories that already exist (however unlikely that is). Though seemingly unlikely,
    // this can happen if two applications test writable directories on initialization, e.g.
    // when setting up SIMDIS_HOME path.
    const std::string& candidateName = simCore::pathJoin(dir, "testWrite" + std::to_string(rand()));
    if (simCore::FileInfo(candidateName).exists())
      continue;

    // Remove the directory when scope falls. Recurse not required because we create nothing in it
    const simCore::ScopeGuard rmDir([candidateName]() { simCore::remove(candidateName); });
    // Create directory to test writable, rather than a file, in an attempt to reduce
    // false remove errors with virus scanners.
    if (simCore::mkdir(candidateName) != 0)
      return false;
    return simCore::FileInfo(candidateName).isDirectory();
  }
  return false;
}

std::string userApplicationDataDirectory(bool roaming)
{
#ifdef WIN32
  return simCore::getEnvVar(roaming ? "APPDATA" : "LOCALAPPDATA");
#else
  return simCore::pathJoin(simCore::getEnvVar("HOME"), ".config");
#endif
}

std::vector<std::string> filesMissingFromPath(const std::string& path, const std::vector<std::string>& expectedRelativeFiles)
{
  std::vector<std::string> rv;
  for (const auto& relative : expectedRelativeFiles)
  {
    const auto& absolute = simCore::pathJoin({ path, relative });
    const simCore::FileInfo fi(absolute);
    if (!fi.exists() || fi.isDirectory())
      rv.push_back(relative);
  }
  return rv;
}

}
