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
{
#ifdef WIN32
  // On Windows, convert all backslashes to forward slashes for consistency
  auto reslashed = simCore::backslashToFrontslash(path);
  // Need to start search after 0th char to avoid UNC path issues
  auto pos = reslashed.find("//", 1);
#else
  // But on Linux, keep backslashes, they are legal characters
  auto reslashed = path;
  auto pos = reslashed.find("//");
#endif

  // Remove unnecessary duplicate slashes
  while (pos != std::string::npos)
  {
    reslashed.erase(pos, 1);
    pos = reslashed.find("//", pos);
  }

  path_ = reslashed;
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

bool FileInfo::makeAbsolute()
{
  std::filesystem::path path(path_);
  if (path.is_absolute())
    return false;

  path_ = std::filesystem::absolute(path).string();
#ifdef WIN32
  path_ = simCore::backslashToFrontslash(path_);
#endif
  return true;
}

std::string FileInfo::fileName() const
{
  return std::get<1>(simCore::pathSplit(path_));
}

std::string FileInfo::fileNameStem() const
{
  if (path_.empty())
    return "";
  const std::filesystem::path path(path_);
  return path.stem().string();
}

std::string FileInfo::path() const
{
  const auto& [path, name] = simCore::pathSplit(path_);

  // Catch cases like "foo" which should return "."
  if (path.empty() && !name.empty())
    return ".";

#ifdef WIN32
  // Catch cases like "c:/foo" where the slash is relevant to root path
  if (path.size() == 2 && path[1] == ':')
    return path + "/"; // always forward slashes

  // Catch edge case on Windows where path is "//" with no host
  if (path == "//")
    return "/";
#endif

  return path;
}

std::string FileInfo::absolutePath() const
{
  std::error_code unused;
  // Use path() to catch the edge cases (which also cause problems for absolute())
  const auto& pathStr = std::filesystem::absolute(path(), unused).string();
#ifdef WIN32
  // On Windows, convert all backslashes to forward slashes for consistency
  return simCore::backslashToFrontslash(pathStr);
#else
  // But on Linux, keep backslashes, they are legal characters
  return pathStr;
#endif
}

std::string simCore::FileInfo::filePath() const
{
  return path_;
}

std::string simCore::FileInfo::absoluteFilePath() const
{
  std::filesystem::path path(path_);
  if (path.is_absolute())
    return path_;

  auto absPath = std::filesystem::absolute(path).string();
#ifdef WIN32
  absPath = simCore::backslashToFrontslash(absPath);
#endif
  return absPath;
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

#ifdef WIN32
  // Check for UNC use case where the text after the // is the system name and not a file name
  if ((pathLastSlash == 1) && (path.front() == '/'))
    return { path, ""};
#endif

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
  {
    const int rv = std::filesystem::create_directories(path, unused) ? 0 : 1;
    if (rv == 0) // success
      return 0;
    // Still a success if the directory now exists
    return simCore::FileInfo(path).isDirectory() ? 0 : 1;
  }
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
  SHFileOp.hwnd = nullptr;
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

std::string normalizeFilepath(const std::string& filePath)
{
  // Expand the EnvVars, then normalize it
  return std::filesystem::path(simCore::expandEnv(filePath)).lexically_normal().string();
}

}
