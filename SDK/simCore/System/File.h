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

private:
  std::string path_;
};

}

#endif /* SIMCORE_SYSTEM_FILE_H */
