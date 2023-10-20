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
#ifndef SIMCORE_SYSTEM_FILE_H
#define SIMCORE_SYSTEM_FILE_H

#include <string>
#include <vector>
#include "simCore/Common/Export.h"

namespace simCore {

#ifdef WIN32
inline const std::string PATH_SEPARATOR = "\\";
#else
inline const std::string PATH_SEPARATOR = "/";
#endif

/**
 * Given a path to a file or directory, returns relevant accessors to query information
 * about that path. Many of these routines wrap std::filesystem calls, but do not throw
 * any exceptions for standard behavior.
 */
class SDKCORE_EXPORT FileInfo
{
public:
  explicit FileInfo(const std::string& path);

  /** True if the path exists as a file, directory, etc. */
  bool exists() const;
  /** True if the path exists and refers to a filename. */
  bool isRegularFile() const;
  /** True if the path exists and refers to a directory. */
  bool isDirectory() const;

  /** True if the given path is equivalent (same file or directory and file status is the same) */
  bool isEquivalent(const std::string& toPath) const;

private:
  std::string path_;
};

/**
 * Multi-value path concatenation. Ignores empty parts. Adds PATH_SEPARATOR as needed
 * between segments. Like Python's os.path.join(), this routine will truncate the
 * results when a segment starts with a slash for absolute path. For example,
 * `simCore::pathJoin({"a", "/b"})` returns `/b`. Note that this routine may insert
 * OS-specific path separators and respects all OS-appropriate path separators, but
 * will not replace existing path separators with native separators; for that, refer
 * to simCore::toNativeSeparators().
 */
SDKCORE_EXPORT std::string pathJoin(const std::vector<std::string>& pathSegments);

/**
 * Creates the directory provided in path. This may be relative or absolute as per
 * rules of std::filesystem::create_directory/create_directories. This method will
 * create only a single directory unless makeParents is true, which is equivalent
 * to using `mkdir -p path` on a UNIX command line.
 * @param path Directory to create, relative or absolute.
 * @param makeParents If true, create all parents of path that do not currently
 *   exist similar to `mkdir -p`. If false, the default, then parents of the given
 *   path are not created and this method will fail.
 * @return 0 on success, non-zero on error. Already-existing directory is an error.
 */
SDKCORE_EXPORT int mkdir(const std::string& path, bool makeParents = false);

/**
 * Removes the file or directory specified by path, which is permanently deleted.
 * If path is a directory that is non-empty, this method will fail unless recursive
 * is true. For files, recursive parameter makes no difference.
 * @param path File or directory to remove
 * @param recursive If true and path is a directory, also remove the entire contents
 *   of the path.
 * @return 0 on success, non-zero on error. Non-existing path is an error.
 */
SDKCORE_EXPORT int remove(const std::string& path, bool recursive = false);

/**
 * Removes the file or directory, placing it in the recycling bin on Windows. On
 * Linux, the file is permanently removed using simCore::remove(), recursively.
 * @param path File or directory to recycle
 * @return 0 on success, non-zero on error. Non-existing path is an error.
 */
SDKCORE_EXPORT int recycle(const std::string& path);

/**
 * Returns true if the directory exists AND it is writable. The only sure-fire way to
 * determine if a directory is writable is to attempt to write to it, which this
 * method will do (and subsequently clean up). This method returns false if the given
 * directory does not exist.
 * @param dir Directory to test, must exist.
 * @return True if the directory can be written to, false otherwise. If dir does not
 *   exist then false is returned.
 */
SDKCORE_EXPORT bool isDirectoryWritable(const std::string& dir);

/**
 * Retrieves the user's application data directory. On Windows this is typically %APPDATA%.
 * On Linux, this is typically $HOME/.config. If roaming is set to false, Windows will
 * return the application data directory for local use, typically %LOCALAPPDATA%. Linux
 * does not respect the roaming flag.
 *
 * This is a generic folder that contains data for all application. Typically to save
 * application data, you will want to use at least one level of hierarchy.
 * @param roaming If true, return roaming location for Windows; if false, local.
 * @return User's application data directory.
 */
SDKCORE_EXPORT std::string userApplicationDataDirectory(bool roaming);

}

#endif /* SIMCORE_SYSTEM_FILE_H */
