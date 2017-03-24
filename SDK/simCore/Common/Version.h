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
#ifndef SIMCORE_COMMON_VERSION_H
#define SIMCORE_COMMON_VERSION_H

#include <string>
#include "simCore/Common/Export.h"
#include "simCore/Common/Exception.h"

namespace simCore
{
  /** SIMDIS SDK major version number */
  static const int SDKVERSION_MAJOR = 1;
  /** SIMDIS SDK minor version number */
  static const int SDKVERSION_MINOR = 6;
  /** SIMDIS SDK revision version number */
  static const int SDKVERSION_REVISION = 0;
  /** Numeric version of SIMDIS SDK in single variable; [MAJOR][MINOR][REVISION], 2 digits per */
  static const int SDKVERSION_BUILDNUMBER = 10600;

  /** Retrieves the SDKVERSION_MAJOR in the compiled library */
  SDKCORE_EXPORT int majorVersion();
  /** Retrieves the SDKVERSION_MINOR in the compiled library */
  SDKCORE_EXPORT int minorVersion();
  /** Retrieves the SDKVERSION_REVISION in the compiled library */
  SDKCORE_EXPORT int revisionVersion();
  /** Retrieves the SDKVERSION_BUILDNUMBER in the compiled library */
  SDKCORE_EXPORT int buildNumber();
  /** Retrieves the compiled-in version string (Major.Minor.Revision) */
  SDKCORE_EXPORT std::string versionString();

  /** Checks the major and minor version for API differences between what was compiled
   * against, vs what is being linked against.  This will help track down errors during
   * active development, especially on Windows systems.
   * The check works by compiling the version information into the DLL (or .so) and
   * comparing the compiled-in version information against the compiled-against version
   * information in this header, specifically in this inline function.
   * @return 0 on no errors with version; 1 when there's some sort of version mismatch
   */
  inline int checkVersion()
  {
    if (SDKVERSION_MAJOR == majorVersion() &&
        SDKVERSION_MINOR == minorVersion())
    { // revision is not checked, should be binary compatible across revisions
      if (SDKVERSION_BUILDNUMBER == buildNumber())
        return 0;
    }
    return 1;
  }

  /** Declare library version error exception to throw */
  SIMCORE_EXCEPTION(LibraryVersionError);

  /** Version of checkVersion that throws an exception when incompatible with loaded version */
  inline void checkVersionThrow()
  {
    if (simCore::checkVersion() != 0)
      throw(SIMCORE_MAKE_EXCEPTION(LibraryVersionError, " Version number mismatch against SDK library"));
  }
}

#endif /* SIMCORE_COMMON_VERSION_H */
