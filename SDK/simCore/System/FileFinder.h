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
#pragma once

#include <map>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"

namespace simCore
{

class SDKCORE_EXPORT FileFinder
{
public:
  /// Constructor, sets a single search path and patterns to search.  Hidden directory checking is initially disabled.
  FileFinder(const std::string& searchPath, const std::string& patterns, unsigned int maxLevelDown=6);

  /// Constructor, sets vector of default search paths and patterns to match.  Hidden directory checking is initially disabled.
  FileFinder(const std::vector<std::string>* searchPath, const std::string& patterns, unsigned int maxLevelDown=6);

  /// Destructor
  virtual ~FileFinder();

  /// True if file searches will check hidden directories, false if not
  void setCheckHiddenDirectories(bool checkHidden);
  bool getCheckHiddenDirectories() const;

  /**
   * Returns the complete path and filename of file found
   * @param filename file name to be searched
   * @param searchPath Path to be added to the search cache
   * @param cerrErrorMessage If true, prints an error message to the console that the file could not be found
   * @return complete path and filename of file found, empty if not found.
   */
  std::string find(const std::string& fileNameStr, const std::string& searchPathStr="", bool cerrErrorMessage=true);

  /**
   * Searches for the first file found in the search paths from a list of provided files.  This search will
   * prioritize the search order over the filenames.  In other words, a filename with lower priority will be
   * returned if it occurs earlier in the search order than other filenames.
   * @param filenames Filenames to search, in priority order.
   * @return Complete path and filename of file found, empty if not found.
   */
  std::string find(const std::vector<std::string>& filenames);

  /**
   * Adds a temporary path to the search cache
   * @param searchPath Path to be added to the search cache
   * @param clearCache If true, clears temporary cache.  Usually done when loading a new data file
   * @return true if valid search path detected, false otherwise.
   */
  bool addTemporarySearchPath(const std::string& searchPath, bool clearCache);

  /// Adds a path to the search cache
  void addSearchPath(const std::string& searchPath);

  /// Resets the search cache and then adds the given path
  void resetSearchPath(const std::string& searchPath);

  /// Adds a vector of paths to the search cache
  void addSearchPath(const std::vector<std::string>* searchPath);

  /// Adds a path to be skipped during the search cache
  void skipSearchPath(const std::string& searchPath);

  /// Adds a vector of paths to be skipped during the search cache
  void skipSearchPath(const std::vector<std::string>* searchPath);

  /// Sets the depth of the file search; will clear out the cache
  void setMaximumDepth(unsigned int depth);
  unsigned int getMaximumDepth() const;

  /**
   * Clears the current cache and loads from the given filename. If the cache contains the
   * filename requested in find(), the cached location will be used rather than searching
   * on-disk.
   * @param filename  Complete filename or path indicating the cache file to load
   * @return 0 if the indicated cache file was successfully loaded. Non-zero otherwise
   */
  int loadFileCache(const std::string& filename);

  /**
   * Saves the cache to the given filename, skipping saving if there are no changes.
   * @param filename  Complete filename or path indicating where to save the cache file
   * @param force  If true, will save the cache even when no changes have been made
   * @return 0 if the cache was successfully saved (or skipped because no changes). Non-zero otherwise
   */
  int saveFileCache(const std::string& filename, bool force = false) const;

  /// Clears the cache
  void clearFileCache();

  /// Return the cache to speed up icon searching when the extension is not specified
  const std::map<std::string, std::string>& fileCache() const;

  /// Allows an external observer to see which directory being scanned and to reply with continue or abort the scan
  class Callback
  {
  public:
    virtual ~Callback() {}

    /**
     * Allows an external observer to see which directory is being scanned and to reply with continue or abort the scan
     * @param directory The currently scanned directory
     * @return 0 continue scan; non-zero abort scan
     */
    virtual int directoryScanned(const std::string& directory) = 0;
  };

  /// Install a callback; caller retains ownership
  void installCallback(Callback* callback);

private:
  /// Trims and converts slashes for input filename
  std::string formatForSearch_(const std::string& filename, std::string& nameOnly) const;
  /// Wraps the given string, if necessary, to protect from troublesome characters
  std::string wrapFileString_(const std::string& filename) const;
  /// Returns true if cachedDirectories matches searchPaths_
  bool isMatchingDirectories_(const std::vector<std::string>& cachedDirectories) const;

  /// Various search methods employed by the find() call
  std::string findAsIs_(const std::string& filename) const;
  std::string findRelative_(const std::string& nameOnly, const std::string& searchPathStr) const;
  std::string findInSystemCache_(const std::string& nameOnly);
  std::string findOnDisk_(const std::string& nameOnly);
  std::string findInProvidedSearchPath_(const std::string& nameOnly, const std::string& searchPathStr);

  /** Manages a cache for searching for files */
  class FileSearchCache;

  std::vector<std::string> skippedPaths_; ///< Vector of search paths to skip
  std::vector<std::string> searchPaths_;  ///< Vector of cached search paths
  std::string patterns_;                  ///< Cached file patterns to match
  bool isCaseInsensitive_;                ///< Flag if the OS has case sensitive file names
  bool checkHidden_;                      ///< Search hidden directories
  FileSearchCache* filesFound_;           ///< Map of found file and associated path
  FileSearchCache* tempsFound_;           ///< Map of found temporary file and associated path
  unsigned int maxLevelDown_;             ///< Number of directory levels to go down before quitting
  std::map<std::string, std::string> filenameToLocation_; ///< Map the given filename to an actual location, the system cache
  bool fileCacheDirty_; ///< If true the system cache needs to be written to a file
  Callback* callback_;  ///< Do a callback when a directory is scanned (not owned)
};
}
