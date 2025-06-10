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
#ifndef SIMCORE_SYSTEM_UTILS_H
#define SIMCORE_SYSTEM_UTILS_H

#include <string>
#include <vector>
#include "simCore/Common/Export.h"

namespace simCore
{

/**
 * Returns the full absolute filename of the current process's executable.
 * @return Full executable absolute filename of the current process executable
 */
SDKCORE_EXPORT std::string getExecutableFilename();

/**
 * Returns the full absolute containing path of the current process's executable.
 * @return Full executable absolute containing path of the current process executable
 */
SDKCORE_EXPORT std::string getExecutablePath();


/** Defines initialization steps to follow when setting environment variables. */
struct InitializeEnvironmentConfig
{
  /** Initialize random number generator */
  bool random = true;

  /** Set SIMDIS_DIR based on executable's path */
  bool simdisDir = true;
  /** Print an error when SIMDIS_DIR cannot be confirmed */
  bool cerrOnBadSimdisDir = true;
  /** Set SIMDIS_HOME environment variable */
  bool simdisHome = true;

  /**
   * Sentinel files for testing good expected SIMDIS_DIR, relative to SIMDIS_DIR.
   * When simdisDir is true, these files are used as sentinels to detect good
   * SIMDIS_DIR and whether or not to print an error with cerrOnBadSimdisDir.
   */
  std::vector<std::string> simdisDirSentinels = {
    "data/fonts/arialbd.ttf",
    "data/fonts/arial.ttf",
    "data/fonts/times.ttf",
  };

  // All remaining values below depend on a valid SIMDIS_DIR

  /** Prepend SIMDIS_DIR/bin/HW-OS to PATH */
  bool path = true;

  /** Set up Python3: PYTHONHOME, PYTHONPATH, PYTHONPYCACHEPREFIX */
  bool python3 = true;
  /** Python version string, used to combine to create the .ZIP and dynamic-load folders */
  std::string python3Version = "3.12";

  /** Set SIMDIS_SDK_FILE_PATH to SIMDIS_DIR/data */
  bool simdisSdkFilePath = true;
  /** Set SIMDIS_FONTPATH to SIMDIS_DIR/data/fonts */
  bool simdisFontPath = true;
  /** Set SIMDIS_TERRAIN to SIMDIS_DIR/data/sdTerrain */
  bool simdisTerrain = true;
  /** Set GDAL_DATA, GDAL_DRIVER_PATH, and PROJ_LIB */
  bool gdal = true;
  /** Set ROCKY_FILE_PATH, ROCKY_DEFAULT_FONT, and PROJ_LIB */
  bool rocky = true;

  /** Set QT_PLUGIN_PATH, unset XDG_SESSION_TYPE */
  bool qt = true;
};

/**
 * Given configuration options, set various SIMDIS-related environment variables like
 * SIMDIS_SDK_FILE_PATH, SIMDIS_DIR, QT_PLUGIN_PATH, etc. The code assumes that if
 * the calling executable is in a good SIMDIS distribution that its path is used for
 * SIMDIS_DIR; else it assumes SIMDIS_DIR is preset for an eventual installation path.
 * @param config Configuration of operations to perform.
 * @return 0 on success, non-zero on error. The only failure is if the SIMDIS_DIR
 *   is not correctly detected based on expected files in SIMDIS_DIR.
 */
SDKCORE_EXPORT int initializeSimdisEnvironmentVariables(const InitializeEnvironmentConfig& config);

/** Initializes the SIMDIS environment variables using the default configuration. Convenience wrapper. */
SDKCORE_EXPORT int initializeSimdisEnvironmentVariables();

} // namespace simCore

#endif /* SIMCORE_SYSTEM_UTILS_H */
