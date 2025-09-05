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
#ifndef SIMCORE_SYSTEM_MEMORYINFO_H
#define SIMCORE_SYSTEM_MEMORYINFO_H

#include <optional>
#include <string>
#include "simCore/Common/Common.h"

namespace simCore {

/** Provides RAM/memory information about the current system, for Linux and Windows systems. */
class SDKCORE_EXPORT MemoryInfo
{
public:
  /** Actual memory values from the system. */
  struct MemoryStats
  {
  public:
    uint64_t totalBytes = 0;
    uint64_t freeBytes = 0;
    uint64_t usedBytes = 0;

    // Convenience methods for different units
    double totalMb() const { return totalBytes * BYTES_TO_MB; }
    double freeMb() const { return freeBytes * BYTES_TO_MB; }
    double usedMb() const { return usedBytes * BYTES_TO_MB; }

    double totalGb() const { return totalBytes * BYTES_TO_GB; }
    double freeGb() const { return freeBytes * BYTES_TO_GB; }
    double usedGb() const { return usedBytes * BYTES_TO_GB; }

  private:
    // Convenience conversion factors
    static constexpr double BYTES_TO_MB = 1.0 / (1024.0 * 1024.0);
    static constexpr double BYTES_TO_GB = BYTES_TO_MB / 1024.0;
  };

  /** Defines the errors that might occur in memory processing */
  enum class Error
  {
    Success,
    PlatformNotSupported,
    WindowsApiError,
    LinuxProcFileNotFound,
    LinuxProcFileReadError,
    LinuxProcFileParseError,
    LinuxMissingMemoryFields
  };

  /** Results are returned in a structure */
  struct Result
  {
    Error error = Error::Success;
    std::optional<MemoryStats> stats;
    std::string errorMessage;

    /** Convenience method for detecting success */
    bool isSuccess() const { return error == Error::Success; }
    /** Convenience method for detecting success */
    explicit operator bool() const { return isSuccess(); }
  };

  /** Primary method: Get current system memory statistics */
  static Result getMemoryInfo();

private:
  /** Returns the error message as a string. */
  static std::string errorToString_(Error error);
};

}

#endif /* SIMCORE_SYSTEM_MEMORYINFO_H */
