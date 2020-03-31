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
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/String/Format.h"

namespace
{

int testGetExtension()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::getExtension("test.txt") == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT") == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.txt", true) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT", true) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.txt", false) == ".txt");
  rv += SDK_ASSERT(simCore::getExtension("test.TXT", false) == ".TXT");
  rv += SDK_ASSERT(simCore::getExtension("test.") == ".");
  rv += SDK_ASSERT(simCore::getExtension("test") == "");
  rv += SDK_ASSERT(simCore::getExtension("") == "");
  rv += SDK_ASSERT(simCore::getExtension("test.foo.bar") == ".bar");
  rv += SDK_ASSERT(simCore::getExtension("test.a") == ".a");
  rv += SDK_ASSERT(simCore::getExtension("test.ab,cd!ef") == ".ab,cd!ef");
  rv += SDK_ASSERT(simCore::getExtension("test.AbCdEfGhI") == ".abcdefghi");
  rv += SDK_ASSERT(simCore::getExtension("test.AbCdEfGhI", false) == ".AbCdEfGhI");
  return rv;
}

}

int StringFormatTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  int rv = 0;

  rv += SDK_ASSERT(testGetExtension() == 0);

  std::cout << "simCore StringFormatTest " << ((rv == 0) ? "passed" : "failed") << std::endl;

  return rv;
}
