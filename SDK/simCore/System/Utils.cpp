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
#include <iostream>
#include "simCore/Calc/Random.h"
#include "simCore/String/Utils.h"
#include "simCore/System/File.h"
#include "simCore/System/Utils.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace simCore
{

namespace {

// Various environment variables that this code can set
inline const std::string GDAL_DATA = "GDAL_DATA";
inline const std::string GDAL_DRIVER_PATH = "GDAL_DRIVER_PATH";
inline const std::string PATH = "PATH";
inline const std::string PROJ_LIB = "PROJ_LIB";
inline const std::string PYTHONHOME = "PYTHONHOME";
inline const std::string PYTHONPATH = "PYTHONPATH";
inline const std::string PYTHONPYCACHEPREFIX = "PYTHONPYCACHEPREFIX";
inline const std::string QT_PLUGIN_PATH = "QT_PLUGIN_PATH";
inline const std::string ROCKY_DEFAULT_FONT = "ROCKY_DEFAULT_FONT";
inline const std::string ROCKY_FILE_PATH = "ROCKY_FILE_PATH";
inline const std::string SIMDIS_DIR = "SIMDIS_DIR";
inline const std::string SIMDIS_FONTPATH = "SIMDIS_FONTPATH";
inline const std::string SIMDIS_HOME = "SIMDIS_HOME";
inline const std::string SIMDIS_SDK_FILE_PATH = "SIMDIS_SDK_FILE_PATH";
inline const std::string SIMDIS_TERRAIN = "SIMDIS_TERRAIN";
inline const std::string SIMDIS_USER_DIR = "SIMDIS_USER_DIR";
inline const std::string XDG_SESSION_TYPE = "XDG_SESSION_TYPE";

#ifdef WIN32
inline const std::string VARSEP_STR = ";";
inline const std::string HWOS = "amd64-nt";
#else
inline const std::string VARSEP_STR = ":";
inline const std::string HWOS = "amd64-linux";
#endif

/** Location of the soft link for the current process (Linux only) */
inline const std::string PROCESS_EXE_PATH = "/proc/self/exe";
/** Maximum size of a path, limited for C-style API calls */
inline constexpr size_t PATH_MAX_LEN = 4096;

/**
 * Internal helper function that is responsible for attempting to guess the SIMDIS_DIR
 * environment based on the current executable's path. This is a text-only comparison
 * that checks path portions like "SIMDIS/bin/amd64-nt" to check for expected SIMDIS
 * path portions.
 */
inline
std::string guessSimdisDir(bool& confident)
{
  confident = true;
  const std::string& executablePath = simCore::getExecutablePath();

  // Expectations out of the simCore functions
  assert(!executablePath.empty());
  if (executablePath.empty())
  {
    confident = false;
    return executablePath;
  }
  // Should not end in a slash
  assert(executablePath.back() != '/');
#ifdef WIN32
  assert(executablePath.back() != '\\');
#endif

  // Extract the last 3 path items, which will be used for testing
  const auto& [pathMinus1, expectedHwOs] = simCore::pathSplit(executablePath);
  const auto& [pathMinus2, expectedBin] = simCore::pathSplit(pathMinus1);
  const auto& [pathMinus3, expectedSimdis] = simCore::pathSplit(pathMinus2);

  // Check for something like SIMDIS/bin/amd64-nt or amd64-linux
  if (expectedHwOs != HWOS ||
    expectedBin != "bin" ||
    expectedSimdis != "SIMDIS")
  {
    confident = false;
  }
  return pathMinus2;
}

/** Internal function that initializes Python for SIMDIS_DIR init code */
inline
void initializePython3(const std::string python3Version)
{
  const std::string& pythonWithVersion = "python" + python3Version;

  const std::string& simdisDir = simCore::getEnvVar(SIMDIS_DIR);
  const std::string& pythonHome = simCore::pathJoin({ simdisDir, "lib", HWOS, pythonWithVersion });
  // PYTHONHOME
  if (simCore::FileInfo(pythonHome).exists())
    simCore::setEnvVar(PYTHONHOME, pythonHome, true);

  // PYTHONPATH
  std::string pythonPath;

  // The PYTHONPATH should always start with our .zip file for finding site import
  const std::string& python3Zip = simCore::pathJoin({ pythonHome, pythonWithVersion + ".zip" });
  if (simCore::FileInfo(python3Zip).exists())
    pythonPath = python3Zip;

  const std::string& binPythonScripts = simCore::pathJoin({ simdisDir, "bin", "pythonScripts" });
  if (simCore::FileInfo(binPythonScripts).exists())
    pythonPath += VARSEP_STR + binPythonScripts;

  const std::string& simdisUserDir = simCore::getEnvVar(SIMDIS_USER_DIR);
  if (!simdisUserDir.empty() && simCore::FileInfo(simdisUserDir).isDirectory())
    pythonPath += VARSEP_STR + simdisUserDir;

  // add path to python pyd/.so files
#ifdef WIN32
  const std::string& libDynLoadPython = simCore::pathJoin({ simdisDir, "lib", HWOS, pythonWithVersion });
#else
  const std::string libDynLoadPython = simCore::pathJoin({ simdisDir, "lib", HWOS, pythonWithVersion, "lib-dynload" });
#endif
  if (simCore::FileInfo(libDynLoadPython).exists())
    pythonPath += VARSEP_STR + libDynLoadPython;

  // Prepend this SIMDIS pythonpath to pre-existing PYTHONPATH if it exists
  const auto& oldPythonPath = simCore::getEnvVar(PYTHONPATH);
  // initializePython3 can be called twice if invoked via StartSIMDIS app, so avoid redundant addition.
  // SIMDIS paths must be at the front, else we override whatever was there. If SIMDIS paths
  // were at the front, then we don't need to set PYTHONPATH at all, it was already good.
  if (oldPythonPath.find(pythonPath) != 0)
  {
    pythonPath += VARSEP_STR + oldPythonPath;
    simCore::setEnvVar(PYTHONPATH, pythonPath, true);
  }

  // SIM-12654: python since 3.8 supports setting a pycache path
  const std::string& simdisHome = simCore::getEnvVar(SIMDIS_HOME);
  if (!simdisHome.empty())
    simCore::setEnvVar(PYTHONPYCACHEPREFIX, simCore::pathJoin({ simdisHome, "pycache" }), false);
}

}

std::string getExecutableFilename()
{
  std::string rv;
  char realPath[PATH_MAX_LEN];

#ifndef WIN32
  // On Linux, use /proc/self/exe (stackoverflow.com/questions/7051844)
  const size_t count = readlink(PROCESS_EXE_PATH.c_str(), realPath, sizeof(realPath) - 1);
#else
  const DWORD count = GetModuleFileName(nullptr, realPath, static_cast<DWORD>(sizeof(realPath) - 1));
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

int initializeSimdisEnvironmentVariables(const InitializeEnvironmentConfig& config)
{
  // Initialize random number generator, which helps here with randomization for is-
  // directory-writable tests.
  if (config.random)
    simCore::initializeRandomSeedWithTime();

  // SIMDIS_DIR
  std::string simdisDir;
  int rv = 0;
  if (config.simdisDir)
  {
    bool confidentInGuess = false;
    simdisDir = simCore::toNativeSeparators(guessSimdisDir(confidentInGuess));
    const std::string& simdisDirFromEnv = simCore::toNativeSeparators(simCore::getEnvVar(SIMDIS_DIR));

    // Sync up the SIMDIS_DIR environment variable and simdisDir local variable.  Trust
    // the guessed SIMDIS_DIR (from path) only if the guess is confident, or if we find
    // expected files in the path, or if there is no alternative (empty ENV)
    if (confidentInGuess ||
      simCore::filesMissingFromPath(simdisDir, config.simdisDirSentinels).empty() ||
      simdisDirFromEnv.empty())
    {
      // Don't need simdisDir set, because that's the value we're using
      simCore::setEnvVar(SIMDIS_DIR, simdisDir, true);
    }
    else
    {
      // Don't need simCore::setEnvVar(), because that's already the value we're using
      simdisDir = simdisDirFromEnv;
    }

    // check SIMDIS_DIR
    const auto& missingFiles = simCore::filesMissingFromPath(simdisDir, config.simdisDirSentinels);
    rv = (missingFiles.empty() ? 0 : 1);
    if (!missingFiles.empty() && config.cerrOnBadSimdisDir)
    {
      std::cerr << "SIMDIS_DIR environment variable cannot be properly set!\n";
      std::cerr << "Required files not found in path:\n";
      for (const auto& relativeFilename : missingFiles)
        std::cerr << "  " << relativeFilename << "\n";
      std::cerr << "\n";
    }
  }
  else
    simdisDir = simCore::toNativeSeparators(simCore::getEnvVar(SIMDIS_DIR));

  // SIMDIS_HOME (must be set after SIMDIS_DIR)
  if (config.simdisHome)
  {
    const auto expectedSimdisHome = [](bool roaming) {
      return simCore::pathJoin({
        simCore::userApplicationDataDirectory(roaming),
        "Naval Research Laboratory",
        "home"
        });
      };

    // Pull out the SIMDIS_HOME that we'll be using, and whether it's writable with
    // respect to copying files over from SIMDIS_DIR
    std::string simdisHome = expectedSimdisHome(true);
    simCore::mkdir(simdisHome, true);
    bool copyFiles = simCore::isDirectoryWritable(simdisHome);
#ifdef WIN32
    if (!copyFiles)
    {
      simdisHome = expectedSimdisHome(false);
      simCore::mkdir(simdisHome, true);
      copyFiles = simCore::isDirectoryWritable(simdisHome);
    }
#endif
    // If neither is writable, try falling back to SIMDIS_DIR/config/SIMDIS, matching legacy behavior
    const std::string& simdisDirConfigSimdis = simCore::pathJoin({ simdisDir, "config", "SIMDIS" });
    if (!copyFiles && !simdisDir.empty() && simCore::FileInfo(simdisDir).isDirectory())
      simdisHome = simdisDirConfigSimdis;

    // Ignore previous settings and override SIMDIS_HOME
    simCore::setEnvVar(SIMDIS_HOME, simdisHome, true);
  }

  // PATH
  if (config.path)
  {
    const std::string path = simCore::toNativeSeparators(simCore::getEnvVar(PATH));
    const std::string simdisBinDir = simCore::pathJoin({ simdisDir, "bin", HWOS });
    if (path.find(simdisBinDir) == std::string::npos)
      simCore::setEnvVar(PATH, (simdisBinDir + VARSEP_STR + path), true);
  }

  //////////////////////////////////////////////////////////////////////
  // SIMDIS_SDK_FILE_PATH: do not overwrite previous setting if it exists
  if (config.simdisSdkFilePath)
    simCore::setEnvVar(SIMDIS_SDK_FILE_PATH, simCore::pathJoin({ simdisDir, "data" }), false);

  // Python setup
  if (config.python3)
    initializePython3(config.python3Version);

  // SIMDIS_FONTPATH
  if (config.simdisFontPath)
    simCore::setEnvVar(SIMDIS_FONTPATH, simCore::pathJoin({ simdisDir, "data", "fonts" }), true);

  // SIMDIS_TERRAIN: do not overwrite previous setting
  if (config.simdisTerrain)
  {
    // Is the current SIMDIS_TERRAIN good? if so do nothing
    const simCore::FileInfo fi(simCore::pathJoin({ simCore::getEnvVar(SIMDIS_TERRAIN), "simdisDefault.earth" }));
    if (!fi.exists())
    {
      // Fall back to SIMDIS_DIR/data/sdTerrain
      const std::string& sdTerrain = simCore::pathJoin({ simdisDir, "data", "sdTerrain" });
      simCore::setEnvVar(SIMDIS_TERRAIN, sdTerrain, true);
    }
  }

  if (config.gdal)
  {
    // GDAL_DATA
    simCore::setEnvVar(GDAL_DATA, simCore::pathJoin({ simdisDir, "data", "GDAL" }), true);
    // GDAL_DRIVER_PATH
#ifdef WIN32
    simCore::setEnvVar(GDAL_DRIVER_PATH, simCore::pathJoin({ simdisDir, "bin", HWOS, "gdalplugins" }), false);
#else
    simCore::setEnvVar(GDAL_DRIVER_PATH, simCore::pathJoin({ simdisDir, "lib", HWOS, "gdalplugins" }), false);
#endif
  }
  if (config.gdal || config.rocky)
  {
    // PROJ_LIB
    simCore::setEnvVar(PROJ_LIB, simCore::pathJoin({ simdisDir, "data", "proj" }), true);
  }

  if (config.rocky)
  {
    simCore::setEnvVar(ROCKY_DEFAULT_FONT, simCore::pathJoin({ simdisDir, "data", "fonts", "arialbd.ttf" }), false);
    simCore::setEnvVar(ROCKY_FILE_PATH, simCore::pathJoin({ simdisDir, "data", "rocky" }), false);
  }

  if (config.qt)
  {
    // QT_PLUGIN_PATH: required for 5.14, does not hurt 5.9
    simCore::setEnvVar(QT_PLUGIN_PATH, simCore::pathJoin({ simdisDir, "bin", HWOS }), true);
#ifndef WIN32
    // Remove XDG_SESSION_TYPE to avoid Qt 5.15 warning on environment
    ::unsetenv(XDG_SESSION_TYPE.c_str());
#endif
  }

  // SIMDIS_HOME
  simCore::setEnvVar(SIMDIS_HOME, simCore::pathJoin({ simdisDir, "config", "SIMDIS" }), false);

  return rv;
}

}
