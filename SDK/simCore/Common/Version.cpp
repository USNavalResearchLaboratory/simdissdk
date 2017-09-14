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
#include <sstream>
#include "simCore/Common/Version.h"

namespace simCore
{

/** Retrieves the compiled-in version string */
std::string versionString()
{
  std::stringstream ss;
  ss << SDKVERSION_MAJOR << "." << SDKVERSION_MINOR << "." << SDKVERSION_REVISION;
  return ss.str();
}

/** Retrieves the SDKVERSION_MAJOR in the compiled library */
int majorVersion()
{
  return SDKVERSION_MAJOR;
}

/** Retrieves the SDKVERSION_MINOR in the compiled library */
int minorVersion()
{
  return SDKVERSION_MINOR;
}

/** Retrieves the SDKVERSION_REVISION in the compiled library */
int revisionVersion()
{
  return SDKVERSION_REVISION;
}

/** Retrieves the SDKVERSION_SOVERSION in the compiled library */
int soVersion()
{
  return SDKVERSION_SOVERSION;
}

}

