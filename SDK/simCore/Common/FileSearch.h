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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef SIMCORE_FILE_SEARCH_H
#define SIMCORE_FILE_SEARCH_H

#include <string>
#include <fstream>
#include "simCore/Common/Memory.h"

namespace simCore
{
  /// An abstract interface for find a file based on the file type
  class FileSearch
  {
  public:
    /// Different file types may prompt different results depending on the implementation
    enum SearchFileType
    {
      GOG,
      MEDIA,
      MODEL,
      TERRAIN,
      PREFERENCE_RULE,
      VIEW,
      HOTKEY,
      DISCN,
      ANTENNAPATTERN,
      RFPROP,
      SCREENTEXT,
      OTHER
    };

    virtual ~FileSearch() {}

    /**
     * Returns the full path for the given filename.
     * @param filename Can be a full or partial file name for either OS and with environment variables
     * @param type The type of file can affect which directories are searched
     * @return Path to full filename if found, or empty string if not found.  URL filename strings are returned as-is.
     */
    virtual std::string findFile(const std::string& filename, SearchFileType type) = 0;
  };

  /// Shared pointer of a FileSearch
  typedef std::shared_ptr<FileSearch> FileSearchPtr;

  /// A class that does no searching (Null Object implementation)
  class NoSearchFileSearch : public FileSearch
  {
  public:
    NoSearchFileSearch() {}
    virtual ~NoSearchFileSearch() {}

    virtual std::string findFile(const std::string& filename, SearchFileType type)
    {
      // Attempt to load the file as-is
      std::fstream ifs(filename.c_str(), std::ios::in);
      if (ifs.good())
        return filename;
      return "";
    }
  };
}

#endif
