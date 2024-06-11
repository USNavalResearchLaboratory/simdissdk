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
#include <iostream>
#include <stdexcept>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"

int VersionTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(0 == simCore::checkVersion());
  try
  {
    simCore::checkVersionThrow();
  }
  catch (const std::runtime_error&)
  {
    rv += 1;
    std::cerr << "VersionTest failed.  Compiled against version "
      << simCore::SDKVERSION_MAJOR << "." << simCore::SDKVERSION_MINOR << "." << simCore::SDKVERSION_REVISION << "." << simCore::SDKVERSION_SOVERSION
      << " but linked against version " << simCore::versionString() << "." << simCore::soVersion() << std::endl << std::endl;
  }
  return rv;
}

