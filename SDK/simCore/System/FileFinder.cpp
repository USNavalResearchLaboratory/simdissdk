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

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/System/File.h"
#include "simCore/String/CsvReader.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/UtfUtils.h"
#include "simCore/String/Utils.h"
#include "simCore/System/FileFinder.h"

/// Tag for the System Cache file
static const std::string FF_SYSTEM_CACHE_TAG = "SystemCache";
static const std::string FF_SEARCH_PATH_TAG = "SearchPath";
static const int FF_SYSTEM_CACHE_VERSION = 3;

/// Characters that might cause problems in filenames
static const std::string BAD_CHARACTERS = ",\"";

/////////////////////////////////////////////

namespace
{
  // Utility function for determining if a file is readable.
  static bool isReadable(const std::string& filename)
  {
    const simCore::FileInfo fileInfo(filename);
    return fileInfo.isRegularFile();
  }
}

namespace simCore
{
  // file patterns to exclude while searching
  static const std::string EXCLUDED_FILE_PATTERNS = "*.exe,*.dll,*.so";

  bool FindFileByName(const std::set<std::string>& files, const std::string& fileName, bool isCaseInsensitive)
  {
    if (isCaseInsensitive)
    {
      std::set<std::string>::const_iterator iter = files.begin();
      while (iter != files.end())
      {
        if (simCore::caseCompare(fileName, *iter) == 0)
          return true;

        ++iter;
      }

      return false;
    }

    return files.find(fileName) != files.end();
  }

  bool CompareSubstrings(const std::string& parent, const std::string& child, bool isCaseInsensitive)
  {
    if (isCaseInsensitive)
    {
      // check as-is first; optimization that should catch most comparisons
      if (parent.find(child) != std::string::npos) return true;
      // if no match, then do case insensitive search
      // conversion to lower for both is moderately expensive
      std::string temp = simCore::lowerCase(parent);
      return (temp.find(simCore::lowerCase(child)) != std::string::npos);
    }

    return parent.find(child) != std::string::npos;
  }

  bool ComparePatterns(const std::string& pattern, const std::string& ext, bool isCaseInsensitive)
  {
    if (ext.empty())
      return false;

    // reduce the chances of a false positive by expanding the needle
    std::string needle;
    if (ext[0] == '*')
      needle = ext;
    else if (ext[0] == '.')
      needle = "*" + ext;
    else
      needle = "*." + ext;

    // Conversion to lower for pattern has already occurred, if isCaseInsensitive.  This
    // test is generally only true on Windows, and an ifdef ensures the pattern_ (which
    // is passed in as pattern) is lowercase only.
    if (isCaseInsensitive)
    {
      needle = simCore::lowerCase(needle);
      // Validates the presumption about case sensitivity.  If this fails, the isCaseSensitive
      // will break if ever set to true.
      assert(pattern.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos);
    }

    size_t it = 0;
    while (1)
    {
      it = pattern.find(needle, it);
      if (it == std::string::npos)
        return false;

      // it should end on a "," or null
      it += needle.size();
      if (it == pattern.size() || pattern[it] == ',')
        return true;
      // Start at the next character
      ++it;
    }
    // Loop does not end; it returns in a few conditions
    assert(0);
    return false;
  }

  bool CompareStrings(const std::string& str1, const std::string& str2, bool isCaseInsensitive)
  {
    if (isCaseInsensitive)
    {
      return (simCore::caseCompare(str1, str2) == 0);
    }

    return str1 == str2;
  }

// Represents a directory on the file system
class Node
{
public:
  Node(const std::string& directory, const std::string& patterns, bool checkHidden, unsigned int level, unsigned int maxLevelDown);
  virtual ~Node() {};
  /**
   * Finds a file using a recursive search
   * @param fileName      The file to locate
   * @param skippedPaths  The directories to skip
   * @param callback      Lets an external observer know the current directory
   * @param foundFile     The full path to the found file
   * @return 0 on completed search; 1 on aborted search
   */
  int findFile(const std::string& fileName, const std::vector<std::string>& skippedPaths, FileFinder::Callback* callback, std::string& foundFile);

  /**
   * Clears out the cache and resets the maximum level down
   * @param maxLevelDown    The maximum level down to search for a file
   */
  void reset(unsigned int maxLevelDown);
  /** Clears out the cache */
  void reset();
  /** Return the directory of this node */
  std::string directory() const;

private:
  /**
   * Loads files and sub-directory for a directory
   * @param skippedPaths  Directories to skip
   * @return true if no errors
   */
  bool loadNode_(const std::vector<std::string>& skippedPaths);

  std::string directory_;             ///< Directory to cache
  std::string patterns_;              ///< Which file extensions to look for
  bool searched_;                     ///< If true, the directory has been searched
  bool checkHidden_;                  ///< If true, check hidden directories
  std::set<std::string> filesFound_;  ///< Files in the directory that matches patterns_
  std::vector<Node> directories_;     ///< Sub-directories in the directory
  bool isCaseInsensitive_;            ///< Flag if the OS has case sensitive file names
  unsigned int level_;                ///< The level of this node
  unsigned int maxLevelDown_;         ///< The max level to drill down, if level_ is at maxLevelDown_ don't drill down more
};

/////////////////////////////////////////////

Node::Node(const std::string& directory, const std::string& patterns, bool checkHidden, unsigned int level, unsigned int maxLevelDown)
  : directory_(directory),
  patterns_(patterns),
  searched_(false),
  checkHidden_(checkHidden),
#ifdef WIN32
  isCaseInsensitive_(true),
#else
  isCaseInsensitive_(false),
#endif
  level_(level),
  maxLevelDown_(maxLevelDown)

{
  // File name case sensitive is actually file system dependent, not OS dependent
  // This implementation will solve most, but not all possible problems.
  // Once we have a customer with with the need for both types on the same system
  // logic can be added here to figure out the sensitive of the current directory
  // and everything else down stream will work.

  // Should not drill beyond max level.  Something wrong in loadNode_
  assert(level_ <= maxLevelDown_);
}

void Node::reset(unsigned int maxLevelDown)
{
  reset();
  maxLevelDown_ = maxLevelDown;
}

void Node::reset()
{
  directories_.clear();
  searched_ = false;
}

std::string Node::directory() const
{
  return directory_;
}

bool Node::loadNode_(const std::vector<std::string>& skippedPaths)
{
  filesFound_.clear();
  directories_.clear();
  searched_ = true;

  // Add current directory to search history, checking for duplicate occurrences
  std::error_code ec;
  std::string realPath = simCore::toNativeSeparators(std::filesystem::canonical(directory_, ec).string());
  if (ec)
    return false;

  // Check if directory is to be skipped
  for (size_t i = 0; i < skippedPaths.size(); i++)
  {
    if (CompareStrings(realPath, skippedPaths[i], isCaseInsensitive_))
    {
      return false;
    }
  }

  const std::string pathDir = simCore::toNativeSeparators(realPath);

  // Generate file info from path
  const simCore::FileInfo pathInfo(pathDir);

  // Checks that the given path exists and is not a file
  if (!pathInfo.exists() || !pathInfo.isDirectory())
    return false;

  // Include files, dirs, and drives in the list (excluding '.' and '..')
  // Note: std::filesystem::directory_iterator automatically removes '.' and '..'
  ec.clear();
  auto it = std::filesystem::directory_iterator(pathDir, ec);

  // Attempt to open the directory and return false on access failure
  if (ec)
  {
    // Directory failed to open
    return false;
  }
  else
  {
    // Process directory entries
    for (const std::filesystem::directory_entry& entry : it)
    {
      const std::string ext = entry.path().extension().string();

      // Skip Executables
      if (ComparePatterns(EXCLUDED_FILE_PATTERNS, ext, isCaseInsensitive_))
        continue;

      // Acquire the file path and use it to generate a simCore::FileInfo. FileInfo is safer than raw filesystem operations
      const std::string path = entry.path().string();
      const simCore::FileInfo fileInfo(path);

      // Cache directory path to handle later
      if (fileInfo.isDirectory())
      {
        // If configured to, ignore hidden directories
        const bool hidden = fileInfo.fileName().starts_with('.');
        if (!checkHidden_ && hidden)
          continue;

        // Reached maximum depth, do not go down further
        if (level_ >= maxLevelDown_)
          continue;

        directories_.emplace_back(path, patterns_, checkHidden_, level_ + 1, maxLevelDown_);
      }
      else if (isReadable(fileInfo.absoluteFilePath()))
      {
        // File is readable, check for matching patterns based on file extensions
        // Handle empty patterns or files w/o extensions such as DED
        if (patterns_.empty() || ext.empty() || ComparePatterns(patterns_, ext, isCaseInsensitive_))
        {
          filesFound_.insert(fileInfo.fileName());
        }
      }
    }
  }

  return true;
}

int Node::findFile(const std::string& fileName, const std::vector<std::string>& skippedPaths, simCore::FileFinder::Callback* callback, std::string& foundFile)
{
  if (fileName.empty())
    return 0;

#if SIMDIS_SDK_MIN_VERSION_REQUIRED(1,13,0)
  // Get case sensitive extension, if available
  const std::string& ext = simCore::getExtension(fileName, false);
#else
  const std::string& ext = simCore::getExtension(fileName);
#endif

  // if file extension is not in our pattern, then add
  if (!ext.empty() && !ComparePatterns(patterns_, ext, isCaseInsensitive_))
  {
    // Add the extension to the list of patterns.  ext is e.g. ".ext", so append ",*.ext" in that case.
    patterns_.append(",*");
    if (isCaseInsensitive_)
      patterns_.append(simCore::lowerCase(ext));
    else
      patterns_.append(ext);
    searched_ = false;
  }

  if (!searched_)
  {
    if (loadNode_(skippedPaths))
    {
      if (callback != nullptr)
      {
        // non-zero means abort search
        std::error_code ec;
        if (callback->directoryScanned(simCore::toNativeSeparators(std::filesystem::canonical(directory_, ec).string())) != 0 || ec)
          return 1;
      }
    }
  }

  if (FindFileByName(filesFound_, fileName, isCaseInsensitive_))
  {
    foundFile = directory_ + "/" + fileName;
    return 0;
  }

  for (size_t ii = 0; ii < directories_.size(); ii++)
  {
    if (directories_[ii].findFile(fileName, skippedPaths, callback, foundFile) != 0)
      return 1;
    if (!foundFile.empty())
      return 0;
  }

  return 0;
}

/////////////////////////////////////////////

class FileFinder::FileSearchCache
{
public:
  FileSearchCache() {};
  virtual ~FileSearchCache() {};
  /**
   * A directory to search
   * @param directory    The directory to load
   * @param patterns     File extensions to load
   * @param checkHidden  If true, search hidden directories
   * @param maxLevelDown The number of directory levels to search
   */
  void addNode(const std::string& directory, const std::string& patterns, bool checkHidden, unsigned int maxLevelDown);
  void clearNodes() { files_.clear(); }
  /**
   * Finds a file which a recursive search
   * @param fileName      The file to locate
   * @param skippedPaths  The directories to skip
   * @param callback      Lets an external observer know the current directory
   * @return The full path file name or "" if not found
   */
  std::string findFile(const std::string& fileName, const std::vector<std::string>& skippedPaths, FileFinder::Callback* callback);
  /**
   * Clears out the cache and resets the maximum level down
   * @param maxLevelDown    The maximum level down to search for a file
   */
  void reset(unsigned int maxLevelDown);
  void reset(const std::string& path);
  size_t numNodes() const { return files_.size(); }
  std::string directory(size_t nodeIdx) const
  {
    if (nodeIdx >= files_.size())
      return "";
    return files_[nodeIdx].directory();
  }

private:
  std::vector<Node> files_;       // Top directories to search
};

void FileFinder::FileSearchCache::addNode(const std::string& directory, const std::string& patterns, bool checkHidden, unsigned int maxLevelDown)
{
  files_.emplace_back(directory, patterns, checkHidden, 0, maxLevelDown);
}

void FileFinder::FileSearchCache::reset(unsigned int maxLevelDown)
{
  for (std::vector<Node>::iterator it = files_.begin(); it != files_.end(); ++it)
    it->reset(maxLevelDown);
}

void FileFinder::FileSearchCache::reset(const std::string& path)
{
  for (std::vector<Node>::iterator it = files_.begin(); it != files_.end(); ++it)
  {
    if (it->directory() == path)
    {
      it->reset();
      return;
    }
  }
}

std::string FileFinder::FileSearchCache::findFile(const std::string& fileName, const std::vector<std::string>& skippedPaths, FileFinder::Callback* callback)
{
  std::string foundFile;
  for (size_t ii = 0; ii < files_.size(); ii++)
  {
    // Return (expected) empty string if the user cancels the search
    if (files_[ii].findFile(fileName, skippedPaths, callback, foundFile) != 0)
      return foundFile;

    // Return found string if it was found
    if (!foundFile.empty())
      return foundFile;
  }

  // Nothing found
  return foundFile;
}

///////////////////////////////////////

FileFinder::FileFinder(const std::string& searchPath, const std::string& patterns, unsigned int maxLevelDown)
  :
#ifdef WIN32
  patterns_(simCore::lowerCase(patterns)),
  isCaseInsensitive_(true),
#else
  patterns_(patterns),
  isCaseInsensitive_(false),
#endif
  checkHidden_(false),
  filesFound_(new FileFinder::FileSearchCache()),
  tempsFound_(new FileFinder::FileSearchCache()),
  maxLevelDown_(maxLevelDown),
  fileCacheDirty_(false),
  callback_(nullptr)
{
  addSearchPath(searchPath);
}

FileFinder::FileFinder(const std::vector<std::string>* searchPath, const std::string& patterns, unsigned int maxLevelDown)
  :
#ifdef WIN32
  patterns_(simCore::lowerCase(patterns)),
  isCaseInsensitive_(true),
#else
  patterns_(patterns),
  isCaseInsensitive_(false),
#endif
  checkHidden_(false),
  filesFound_(new FileFinder::FileSearchCache()),
  tempsFound_(new FileFinder::FileSearchCache()),
  maxLevelDown_(maxLevelDown),
  fileCacheDirty_(false),
  callback_(nullptr)
{
  addSearchPath(searchPath);
}

FileFinder::~FileFinder()
{
  delete filesFound_;
  filesFound_ = nullptr;
  delete tempsFound_;
  tempsFound_ = nullptr;
}

void FileFinder::setCheckHiddenDirectories(bool checkHidden)
{
  checkHidden_ = checkHidden;
}

bool FileFinder::getCheckHiddenDirectories() const
{
  return checkHidden_;
}

bool FileFinder::addTemporarySearchPath(const std::string& searchPath, bool clearCache)
{
  // always clear previous results to help with the transition from File Mode to Live Mode. See SIM-4305
  if (true == clearCache)
    tempsFound_->clearNodes();

  // verify incoming path is not empty and is a directory
  if (searchPath.empty())
    return false;

  const std::string fixedPath = simCore::toNativeSeparators(simCore::expandEnv(searchPath));
  const simCore::FileInfo fileInfo(fixedPath);

  if (!fileInfo.isDirectory())
    return false;

  // Because of a watch added, do not kick out early if the temporary path is part of the standard searches

  tempsFound_->addNode(fixedPath, patterns_, checkHidden_, maxLevelDown_);
  return true;
}

void FileFinder::addSearchPath(const std::string& searchPath)
{
  // verify incoming path is not empty and is a directory
  if (searchPath.empty())
    return;

  const std::string fixedPath = simCore::toNativeSeparators(simCore::expandEnv(searchPath));
  const simCore::FileInfo fileInfo(fixedPath);

  if (!fileInfo.isDirectory())
    return;

  // verify we have not already processed path
  for (size_t i = 0; i < searchPaths_.size(); i++)
  {
    if (CompareSubstrings(fixedPath, searchPaths_[i], isCaseInsensitive_))
    {
      return;
    }
  }

  searchPaths_.push_back(fixedPath);
#ifdef DEBUG
  SIM_DEBUG << "Added search path: " << simCore::toNativeSeparators(fixedPath) << std::endl;
#endif
  filesFound_->addNode(fixedPath, patterns_, checkHidden_, maxLevelDown_);
}

void FileFinder::resetSearchPath(const std::string& searchPath)
{
  // verify incoming path is not empty and is a directory
  if (searchPath.empty())
    return;

  const std::string fixedPath = simCore::toNativeSeparators(simCore::expandEnv(searchPath));
  const simCore::FileInfo fileInfo(fixedPath);

  if (!fileInfo.isDirectory())
    return;

  filesFound_->reset(fixedPath);
}

void FileFinder::addSearchPath(const std::vector<std::string>* searchPath)
{
  if (searchPath)
  {
    for (size_t i = 0; i < searchPath->size(); i++)
    {
      addSearchPath((*searchPath)[i]);
    }
  }
}

void FileFinder::skipSearchPath(const std::string& searchPath)
{
  // verify incoming path is not empty and is a directory
  if (searchPath.empty())
    return;

  const std::string fixedPath = simCore::toNativeSeparators(simCore::expandEnv(searchPath));

  // verify we have not already processed path
  for (size_t i = 0; i < skippedPaths_.size(); i++)
  {
    if (CompareSubstrings(fixedPath, skippedPaths_[i], isCaseInsensitive_))
    {
      return;
    }
  }

  skippedPaths_.push_back(fixedPath);
}

void FileFinder::skipSearchPath(const std::vector<std::string>* searchPath)
{
  if (searchPath)
  {
    for (size_t i = 0; i < searchPath->size(); i++)
    {
      skipSearchPath((*searchPath)[i]);
    }
  }
}

std::string FileFinder::formatForSearch_(const std::string& filename, std::string& nameOnly) const
{
  // Fix backslash to forward slash
  const std::string correctFormat = simCore::toNativeSeparators(simCore::expandEnv(filename));
  const simCore::FileInfo fileInfo(correctFormat);
  nameOnly = fileInfo.fileName();

  return fileInfo.filePath();
}

std::string FileFinder::wrapFileString_(const std::string& filename) const
{
  if (filename.find_first_of(BAD_CHARACTERS) == std::string::npos)
    return filename;

  // Escape any \ characters and double quotes in the name before wrapping
  return ("\"" + simCore::StringUtils::addEscapeSlashes(filename) + "\"");
}

std::string FileFinder::findAsIs_(const std::string& filename) const
{
  // Filename may contain a full path to file, check that first; Check file as-is for existence
  const simCore::FileInfo fileInfo(filename);

  if (isReadable(fileInfo.absoluteFilePath()))
    return fileInfo.absoluteFilePath();

  // Check the temporary directories using the as-is filename.  This finds relative path files
  // that are relative to the scenario directory, fixing SIM-8282.  This check is very specific
  // looking for ".." in order to reduce risk on change so that this change only fixes a very
  // small subset of discovered errors, without introducing new errors.  It is intended only to
  // match files that start with ".." because any other filetype should be covered by other
  // types of searches in the correct priority list.
  if (filename.length() > 2 && filename.starts_with(".."))
  {
    const size_t numNodes = tempsFound_->numNodes();
    for (size_t k = 0; k < numNodes; ++k)
    {
      const std::string tempDir(tempsFound_->directory(k));
      const simCore::FileInfo tempFileInfo(simCore::pathJoin({tempDir, filename}));

      if (isReadable(tempFileInfo.absoluteFilePath()))
        return tempFileInfo.absoluteFilePath();
    }
  }

  return "";
}

std::string FileFinder::findRelative_(const std::string& nameOnly, const std::string& searchPathStr) const
{
  // Next, check for relative path
  const std::string relativeFile = simCore::StringUtils::trim(searchPathStr) + "/" + nameOnly;
  const std::string searchPath = simCore::toNativeSeparators(relativeFile);

  const simCore::FileInfo fileInfo(searchPath);
  const std::string absPath = fileInfo.absoluteFilePath();
  if (isReadable(absPath))
    return absPath;
  return "";
}

std::string FileFinder::findInSystemCache_(const std::string& nameOnly)
{
  // Look in system cache
  std::map<std::string, std::string>::iterator it = filenameToLocation_.find(nameOnly);
  if (it != filenameToLocation_.end())
  {
    // make sure the file still exists
    if (isReadable(it->second.c_str()))
      return it->second;

    // No longer exists so remove from cache
    filenameToLocation_.erase(it);
    fileCacheDirty_ = true;
  }
  return "";
}

std::string FileFinder::findOnDisk_(const std::string& nameOnly)
{
  // Next look for file in specified locations (SIMDIS_DIR and similar)
  const std::string rv = filesFound_->findFile(nameOnly, skippedPaths_, callback_);
  if (!rv.empty())
  {
    filenameToLocation_[nameOnly] = rv;
    fileCacheDirty_ = true;
  }
  return rv;
}

std::string FileFinder::findInProvidedSearchPath_(const std::string& nameOnly, const std::string& searchPathStr)
{
  // Last straw, add specified path and search as a temporary
  std::string rv;
  if (!searchPathStr.empty())
  {
    if (addTemporarySearchPath(searchPathStr, false))
      rv = tempsFound_->findFile(nameOnly, skippedPaths_, callback_);
  }
  return rv;
}

std::string FileFinder::find(const std::string& fileNameStr, const std::string& searchPathStr, bool cerrErrorMessage)
{
  const std::string& expandedSearchPath = simCore::expandEnv(searchPathStr);
  std::string nameOnly;
  const std::string fileName = formatForSearch_(fileNameStr, nameOnly);
  if (fileName.empty() || nameOnly.empty())
  {
    if (cerrErrorMessage)
      std::cerr << "Cannot find an empty filename.\n";
    return "";
  }

  // Check given path, might be absolute
  std::string rv = findAsIs_(fileName);
  if (!rv.empty())
    return rv;
  // Check the relative path
  rv = findRelative_(nameOnly, expandedSearchPath);
  if (!rv.empty())
    return rv;
  // Check temporaries, such as cwd of data file or starting location
  rv = tempsFound_->findFile(nameOnly, skippedPaths_, callback_);
  if (!rv.empty())
    return rv;
  // Check the quick system cache next
  rv = findInSystemCache_(nameOnly);
  if (!rv.empty())
    return rv;
  // Perform a slow search on disk, searching for filename provided
  rv = findOnDisk_(nameOnly);
  if (!rv.empty())
    return rv;
  // Last straw, add specified path and search as a temporary
  rv = findInProvidedSearchPath_(nameOnly, expandedSearchPath);
  if (!rv.empty())
    return rv;

  if (cerrErrorMessage)
    std::cerr << "Could not find file: " << fileName << std::endl;

  return "";
}

std::string FileFinder::find(const std::vector<std::string>& filenames)
{
  // Get a vector of the names-only (omitting path)
  std::vector<std::string> namesOnly;
  for (auto i = filenames.begin(); i != filenames.end(); ++i)
  {
    std::string nameOnly;
    const std::string filename = formatForSearch_(*i, nameOnly);
    if (!filename.empty() && !nameOnly.empty())
    {
      // Check given path, might be absolute
      const std::string rv = findAsIs_(filename);
      if (!rv.empty())
        return rv;
      // Save the value in the names-only vector
      namesOnly.push_back(nameOnly);
    }
  }

  // If the vector is empty, then we have nothing to search for
  if (namesOnly.empty())
    return "";

  // Check the relative path
  for (auto i = namesOnly.begin(); i != namesOnly.end(); ++i)
  {
    const std::string rv = findRelative_(*i, "");
    if (!rv.empty())
      return rv;
  }
  // Check temporaries, such as cwd of data file or starting location
  for (auto i = namesOnly.begin(); i != namesOnly.end(); ++i)
  {
    const std::string rv = tempsFound_->findFile(*i, skippedPaths_, callback_);
    if (!rv.empty())
      return rv;
  }
  // Check the quick system cache next
  for (auto i = namesOnly.begin(); i != namesOnly.end(); ++i)
  {
    const std::string rv = findInSystemCache_(*i);
    if (!rv.empty())
      return rv;
  }
  // Perform a slow search on disk, searching for filename provided
  for (auto i = namesOnly.begin(); i != namesOnly.end(); ++i)
  {
    const std::string rv = findOnDisk_(*i);
    if (!rv.empty())
      return rv;
  }

  // Since search path is not provided, no need to call findInProvidedSearchPath_().
  return "";
}

void FileFinder::setMaximumDepth(unsigned int depth)
{
  if (depth == maxLevelDown_)
    return;

  maxLevelDown_ = depth;
  tempsFound_->reset(maxLevelDown_);
  filesFound_->reset(maxLevelDown_);
  clearFileCache();
}

unsigned int FileFinder::getMaximumDepth() const
{
  return maxLevelDown_;
}

int FileFinder::loadFileCache(const std::string& filename)
{
  filenameToLocation_.clear();
  // cache dirty until successful read
  fileCacheDirty_ = true;

  std::ifstream ifs(simCore::streamFixUtf8(simCore::expandEnv(filename)));
  if (!ifs.good())
    return 1;

  simCore::CsvReader reader(ifs);
  std::vector<std::string> tokens;

  if (reader.readLineTrimmed(tokens) != 0)
    return 1;

  // Check for the correct starting values
  if (tokens.size() != 2 || tokens[0] != FF_SYSTEM_CACHE_TAG
    || (atoi(tokens[1].c_str()) != FF_SYSTEM_CACHE_VERSION))
    return 1;

  std::vector<std::string> cachedDirectories;
  // first read the search paths
  while (reader.readLineTrimmed(tokens) == 0)
  {
    if (tokens.size() != 2)
      continue;

    // Remove the quotes used to wrap the tokens (if present)
    simCore::removeQuotes(tokens);

    if (tokens[0] != FF_SEARCH_PATH_TAG)
    {
      // capture the first file
      if (!tokens[0].empty() && !tokens[1].empty())
        filenameToLocation_[tokens[0]] = tokens[1];
      break;
    }

    if (tokens[1].empty())
      continue;

    cachedDirectories.push_back(tokens[1]);
  }

  if (!isMatchingDirectories_(cachedDirectories))
    return 1;

  // read the rest of the files
  while (reader.readLineTrimmed(tokens) == 0)
  {
    if (tokens.size() != 2)
      continue;

    // Remove the quotes used to wrap the tokens (if present)
    const std::string& file = simCore::StringUtils::trim(tokens[0], "\"");
    const std::string& location = simCore::StringUtils::trim(tokens[1], "\"");
    if (!file.empty() && !location.empty())
      filenameToLocation_[file] = location;
  }

  // successful so cache not dirty
  fileCacheDirty_ = false;
  return 0;
}

bool FileFinder::isMatchingDirectories_(const std::vector<std::string>& cachedDirectories) const
{
  if (cachedDirectories.size() != searchPaths_.size())
    return false;

  for (const auto& cached : cachedDirectories)
  {
    const simCore::FileInfo info(simCore::expandEnv(cached));

    if (std::none_of(searchPaths_.begin(), searchPaths_.end(), [info](const std::string& search) { return info.isEquivalent(search);  }))
      return false;
  }

  return true;
}

int FileFinder::saveFileCache(const std::string& filename, bool force) const
{
  if (!fileCacheDirty_ && !force)
    return 0;

  std::ofstream outStream(simCore::streamFixUtf8(simCore::expandEnv(filename)));
  if (!outStream)
    return 1;

  outStream << FF_SYSTEM_CACHE_TAG << "," << FF_SYSTEM_CACHE_VERSION << "\n";

  for (const auto& filename : searchPaths_)
    outStream << FF_SEARCH_PATH_TAG << "," << wrapFileString_(filename) << "\n";
  for (auto it = filenameToLocation_.begin(); it != filenameToLocation_.end(); ++it)
    outStream << wrapFileString_(it->first) << "," << wrapFileString_(it->second) << "\n";

  return 0;
}

void FileFinder::clearFileCache()
{
  fileCacheDirty_ = !filenameToLocation_.empty();
  filenameToLocation_.clear();
}

const std::map<std::string, std::string>& FileFinder::fileCache() const
{
  return filenameToLocation_;
}

void FileFinder::installCallback(Callback* callback)
{
  callback_ = callback;
}

} // namespace
