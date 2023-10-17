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
#include "simCore/String/Utils.h"
#include "simCore/System/Utils.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace simCore
{

/** Location of the soft link for the current process (Linux only) */
static const std::string PROCESS_EXE_PATH = "/proc/self/exe";
/** Maximum size of a path, limited for C-style API calls */
static constexpr size_t PATH_MAX_LEN = 4096;

#ifdef WIN32
static const std::string PATH_SEPARATOR = "\\";
#else
static const std::string PATH_SEPARATOR = "/";
#endif

std::string getExecutableFilename()
{
  std::string rv;
  char realPath[PATH_MAX_LEN];

#ifndef WIN32
  // On Linux, use /proc/self/exe (stackoverflow.com/questions/7051844)
  size_t count = readlink(PROCESS_EXE_PATH.c_str(), realPath, sizeof(realPath) - 1);
#else
  DWORD count = GetModuleFileName(nullptr, realPath, sizeof(realPath) - 1);
#endif

  // Terminate and assign to return value
  if (count > 0 && count < sizeof(realPath))
  {
    realPath[count] = '\0';
    rv = realPath;
  }

  // Make it have proper slashes
  return simCore::toNativeSeparators(rv);
}

std::string getExecutablePath()
{
  return simCore::StringUtils::beforeLast(simCore::getExecutableFilename(), PATH_SEPARATOR);
}

}
