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
#include "simCore/System/MemoryInfo.h"

#ifdef _WIN32
#include <windows.h>

namespace simCore {

/** Implementation for the Windows side to get memory information. */
MemoryInfo::Result getWindowsMemoryInfo()
{
  using Result = MemoryInfo::Result;
  using Error = MemoryInfo::Error;

  // Use Windows API to request values
  MEMORYSTATUSEX memStatus;
  memStatus.dwLength = sizeof(memStatus);
  if (!GlobalMemoryStatusEx(&memStatus))
  {
    DWORD errorCode = GetLastError();
    return Result {
      Error::WindowsApiError, std::nullopt,
      "GlobalMemoryStatusEx failed with error code: " + std::to_string(errorCode)
    };
  }

  // Record in the return value
  MemoryInfo::MemoryStats stats;
  stats.totalBytes = memStatus.ullTotalPhys;
  stats.freeBytes = memStatus.ullAvailPhys;
  stats.usedBytes = stats.totalBytes - stats.freeBytes;

  return Result { Error::Success, stats, "" };
}

}

#elif __linux__
#include <fstream>
#include <sstream>
#include <map>
#include "simCore/String/ValidNumber.h"

namespace simCore {

static const std::string PROC_MEMINFO_FILE = "/proc/meminfo";
static const std::string MEMINFO_TOTAL = "MemTotal";
static const std::string MEMINFO_AVAILABLE = "MemAvailable";
static const std::string MEMINFO_FREE = "MemFree";
static const std::string ESTR_PROC_NOT_FOUND = "Could not open " + PROC_MEMINFO_FILE;
static const std::string ESTR_FAILED_PARSE_VALUE = "Failed to parse memory value for key: ";
static const std::string ESTR_READ_FAILED = "Error reading from " + PROC_MEMINFO_FILE;
static const std::string ESTR_MEMTOTAL_MISSING = MEMINFO_TOTAL + " field not found in " + PROC_MEMINFO_FILE;
static const std::string ESTR_AVAIL_FREE_MISSING = "Neither " + MEMINFO_AVAILABLE + " nor " + MEMINFO_FREE + " field found in " + PROC_MEMINFO_FILE;

/** Given a value string, attempts to convert into an unsigned long long. */
std::optional<uint64_t> parseMemoryValue(const std::string& valueStr)
{
  uint64_t rv = 0;
  if (simCore::isValidNumber(valueStr, rv))
    return rv;
  return {};
}

/** Linux implementation to get RAM from /proc/meminfo */
MemoryInfo::Result getLinuxMemoryInfo()
{
  using Result = MemoryInfo::Result;
  using Error = MemoryInfo::Error;

  std::ifstream meminfoFile(PROC_MEMINFO_FILE);
  if (!meminfoFile)
    return Result { Error::LinuxProcFileNotFound, std::nullopt, ESTR_PROC_NOT_FOUND };

  std::map<std::string, uint64_t> memoryValues;
  std::string line;

  // Loop through the file putting all key/values into a map
  while (std::getline(meminfoFile, line))
  {
    std::istringstream iss(line);
    std::string key;
    std::string valueStr;
    std::string unit;

    if (iss >> key >> valueStr >> unit)
    {
      // Remove the colon from the key
      if (!key.empty() && key.back() == ':')
        key.pop_back();

      const auto valueOpt = parseMemoryValue(valueStr);
      if (!valueOpt)
        return Result { Error::LinuxProcFileParseError, std::nullopt, ESTR_FAILED_PARSE_VALUE + key };

      // Convert from kB to bytes (most values in /proc/meminfo are in kB)
      uint64_t valueBytes = *valueOpt;
      if (unit == "kB")
        valueBytes *= 1024;

      memoryValues[key] = valueBytes;
    }
  }

  // Check for file read error
  if (meminfoFile.bad())
  {
    return Result {
      Error::LinuxProcFileReadError,
      std::nullopt,
      ESTR_READ_FAILED
    };
  }

  // Check if we have the required fields
  const auto totalIt = memoryValues.find(MEMINFO_TOTAL);
  const auto availableIt = memoryValues.find(MEMINFO_AVAILABLE);
  const auto freeIt = memoryValues.find(MEMINFO_FREE);

  if (totalIt == memoryValues.end())
    return Result { Error::LinuxMissingMemoryFields, std::nullopt, ESTR_MEMTOTAL_MISSING };

  MemoryInfo::MemoryStats stats;
  stats.totalBytes = totalIt->second;

  // Prefer MemAvailable over MemFree if available (more accurate)
  if (availableIt != memoryValues.end())
    stats.freeBytes = availableIt->second;
  else if (freeIt != memoryValues.end())
    stats.freeBytes = freeIt->second;
  else
    return Result { Error::LinuxMissingMemoryFields, std::nullopt, ESTR_AVAIL_FREE_MISSING };

  stats.usedBytes = stats.totalBytes - stats.freeBytes;
  return Result { Error::Success, stats, "" };
}

}

#endif

namespace simCore {

MemoryInfo::Result MemoryInfo::getMemoryInfo()
{
#ifdef _WIN32
  return getWindowsMemoryInfo();
#elif __linux__
  return getLinuxMemoryInfo();
#else
  return Result {
    Error::PlatformNotSupported, std::nullopt,
    "Platform not supported. Only Windows and Linux are currently supported."
  };
#endif
}

}
