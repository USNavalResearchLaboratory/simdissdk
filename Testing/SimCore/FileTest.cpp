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
#include <fstream>
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/System/File.h"

namespace {

int testFileInfo()
{
  int rv = 0;

  const std::string thisCppFile = __FILE__;
  // Make sure the file exists. If not, the rest of the test is bogus.
  std::ifstream ifs(thisCppFile);
  if (!ifs)
  {
    std::cerr << "Unable to run testFileInfo(), cpp file does not exist.\n"
      << "This test application is non-portable.\n";
    return 0;
  }
  ifs.close();

  const simCore::FileInfo thisCppFileInfo(thisCppFile);
  rv += SDK_ASSERT(thisCppFileInfo.exists());
  rv += SDK_ASSERT(thisCppFileInfo.isRegularFile());
  rv += SDK_ASSERT(!thisCppFileInfo.isDirectory());

  const simCore::FileInfo cwdInfo(".");
  rv += SDK_ASSERT(cwdInfo.exists());
  rv += SDK_ASSERT(!cwdInfo.isRegularFile());
  rv += SDK_ASSERT(cwdInfo.isDirectory());

  const simCore::FileInfo nonExist("doesNotExist");
  rv += SDK_ASSERT(!nonExist.exists());
  rv += SDK_ASSERT(!nonExist.isRegularFile());
  rv += SDK_ASSERT(!nonExist.isDirectory());

  return rv;
}

}

int FileTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testFileInfo() == 0);

  std::cout << "simCore FileTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";

  return rv;
}
