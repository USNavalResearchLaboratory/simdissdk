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

int testPathJoin()
{
  int rv = 0;

  // Single directory returns
  const std::string PS = simCore::PATH_SEPARATOR;
  rv += SDK_ASSERT(simCore::pathJoin({}) == "");
  rv += SDK_ASSERT(simCore::pathJoin({ "a" }) == "a");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "" }) == ("a" + PS));
  rv += SDK_ASSERT(simCore::pathJoin({ "", "a" }) == "a");
  rv += SDK_ASSERT(simCore::pathJoin({ "", "" }) == "");
  rv += SDK_ASSERT(simCore::pathJoin({ "", "", "a" }) == "a");

  // Typical non-empty non-slashed input
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "", "a"}) == ("a" + PS + "a"));
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "b" }) == ("a" + PS + "b"));
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "b", "c" }) == ("a" + PS + "b" + PS + "c"));
  rv += SDK_ASSERT(simCore::pathJoin({ "", "b", "c" }) == ("b" + PS + "c"));
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "b", ""}) == ("a" + PS + "b" + PS));

  // First token ends with slashes
  rv += SDK_ASSERT(simCore::pathJoin({ "a/", "" }) == "a/");
  rv += SDK_ASSERT(simCore::pathJoin({ "a/", "b" }) == "a/b");
  rv += SDK_ASSERT(simCore::pathJoin({ "a/", "", "b" }) == "a/b");
  rv += SDK_ASSERT(simCore::pathJoin({ "a//", "b" }) == "a//b");

  // Starts with slash
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "/b" }) == "/b");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "/", "b" }) == "/b");

  // Multiple directories in one call
  rv += SDK_ASSERT(simCore::pathJoin({ "a/b", "c/d/" }) == ("a/b" + PS + "c/d/"));
  rv += SDK_ASSERT(simCore::pathJoin({ "/a/b", "c/d/" }) == ("/a/b" + PS + "c/d/"));
  rv += SDK_ASSERT(simCore::pathJoin({ "/a/b/", "c/d/" }) == "/a/b/c/d/");
  rv += SDK_ASSERT(simCore::pathJoin({ "/a/b", "/c/d/" }) == "/c/d/");
  rv += SDK_ASSERT(simCore::pathJoin({ "a/b", "/c/d/" }) == "/c/d/");
  rv += SDK_ASSERT(simCore::pathJoin({ "a/b", "///c/d/" }) == "///c/d/");

  // This test differs from Python on Windows, which returns R"(\\\b)"); Linux returns
  // what is shown below.
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "//", "/b" }) == "/b");

  // Windows allows backslash as a separator, but Linux does not, so different tests
  // with different outcomes.s
#ifdef WIN32
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "b" }) == R"(a\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "\\b" }) == R"(\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\b" }) == R"(\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a//\\", "b" }) == R"(a//\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\", "b" }) == R"(\b)");
  // This test differs from Python, which returns R"(\\\b)")
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\\\", "\\b" }) == R"(\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "\\\\a", "b" }) == R"(\\a\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "" }) == R"(a\)");
#else
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "b" }) == R"(a\/b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "\\b" }) == R"(a\/\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\b" }) == R"(a/\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a//\\", "b" }) == R"(a//\/b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\", "b" }) == R"(a/\/b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a", "\\\\", "\\b" }) == R"(a/\\/\b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "\\\\a", "b" }) == R"(\\a/b)");
  rv += SDK_ASSERT(simCore::pathJoin({ "a\\", "" }) == R"(a\/)");
#endif

  return rv;
}

}

int FileTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testFileInfo() == 0);
  rv += SDK_ASSERT(testPathJoin() == 0);

  std::cout << "simCore FileTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";

  return rv;
}
