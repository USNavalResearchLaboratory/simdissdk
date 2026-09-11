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
#include <filesystem>
#include "simCore/String/Format.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/Utils.h"
#include "simCore/System/FileFinder.h"

namespace {

/**
 * Compare file names accounting for the differences in OS.  The file names must have absolute paths.
 * @param name1 The first file name.
 * @param name2 The second file name.
 * @return true if it succeeds, false if it fails.
 */
bool compareFileNames(const std::string& name1, const std::string& name2)
{
  std::string file1 = simCore::StringUtils::trim(simCore::backslashToFrontslash(name1));
  std::string file2 = simCore::StringUtils::trim(simCore::backslashToFrontslash(name2));

#ifdef WIN32
  return (simCore::caseCompare(file1, file2) == 0);
#else
  return (file1 == file2);
#endif
}

int testFileFinder(const std::string& path)
{
  int rv = 0;

  // Need to resolve any soft links on linux systems
  std::error_code ec;
  std::string base = std::filesystem::canonical(path, ec).string();
  if (ec)
    return 1;

  simCore::FileFinder ff(base, ".txt");

  std::string fileName = ff.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));

  fileName = ff.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right/RightRight.txt"));

  fileName = ff.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));

  fileName = ff.find("DoesNotExist.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  simCore::FileFinder left(base+"/Left", ".txt");

  fileName = left.find("root.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = left.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = left.find("RightRight.txt", base+"/Right", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right/RightRight.txt"));

  fileName = left.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));

  fileName = left.find("DoesNotExist.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  simCore::FileFinder right(base+"/Right", "");

  fileName = right.find("root.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = right.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  right.addTemporarySearchPath(base+"/Left", true);
  fileName = right.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));

  right.addTemporarySearchPath(base+"/Left/Right", true);
  fileName = right.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  right.addSearchPath(base+"/Left");
  fileName = right.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));

  // Look in the wrong place, but it will still find it
  fileName = right.find("Left/RightRight.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right/RightRight.txt"));

  fileName = right.find("RightRight.fin", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = right.find("RightLeft.fin", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Left/RightLeft.fin"));

  fileName = right.find("DoesNotExist.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  simCore::FileFinder skip(base, "");
  skip.skipSearchPath(base+"/Left");
  fileName = skip.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

#ifdef WIN32
  simCore::FileFinder caseInsensitive(base, "");
  fileName = caseInsensitive.find("leftleft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));

  fileName = caseInsensitive.find("Right.Txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));

  simCore::FileFinder caseInsensitive2(simCore::lowerCase(base), "");
  fileName = caseInsensitive2.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Left/Left/LeftLeft.txt"));
#else
  simCore::FileFinder caseSensitive(base, "");
  fileName = caseSensitive.find("leftleft.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = caseSensitive.find("Right.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  fileName = caseSensitive.find("Right.TXT", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));

  simCore::FileFinder caseSensitive2(simCore::lowerCase(base), "");
  fileName = caseSensitive2.find("LeftLeft.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());
#endif

  return rv;
}


int testRecursive(const std::string& path)
{
  int rv = 0;

  // Need to resolve any soft links on Linux systems
  std::error_code ec;
  std::string base = std::filesystem::canonical(path, ec).string();
  if (ec)
    return 1;

  // No recursive search; wont' find the files at one and two levels down
  simCore::FileFinder zero(base, ".txt", 0);
  std::string fileName = zero.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = zero.find("Right.TXT", "", true);
  rv += SDK_ASSERT(fileName.empty());
  fileName = zero.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  // One level recursive search; will find the file one level down; won't find the file two levels down
  simCore::FileFinder one(base, ".txt", 1);
  fileName = one.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = one.find("Right.TXT", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));
  fileName = one.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  // Two level recursive search; will find all files
  simCore::FileFinder two(base, ".txt", 2);
  fileName = two.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = two.find("Right.TXT", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));
  fileName = two.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right/RightRight.txt"));

  return rv;
}

int testMaxDepth(const std::string& path)
{
  int rv = 0;

  // Need to resolve any soft links on Linux systems
  std::error_code ec;
  std::string base = std::filesystem::canonical(path, ec).string();
  if (ec)
    return 1;

  // Recursive search deep enough to find everything
  simCore::FileFinder ff(base, ".txt", 2);
  std::string fileName = ff.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = ff.find("Right.TXT", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));
  fileName = ff.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right/RightRight.txt"));

  // Now drop the bottom level so won't find RightRight.txt
  ff.setMaximumDepth(1);
  fileName = ff.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = ff.find("Right.TXT", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));
  fileName = ff.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  // Now drop the next level so won't find RightRight.txt and Right.TXT
  ff.setMaximumDepth(0);
  fileName = ff.find("root.txt", "", true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/root.txt"));
  fileName = ff.find("Right.TXT", "", true);
  rv += SDK_ASSERT(fileName.empty());
  fileName = ff.find("RightRight.txt", "", true);
  rv += SDK_ASSERT(fileName.empty());

  return rv;
}

int testWrongAbsolute(const std::string& path)
{
  int rv = 0;

  // Need to resolve any soft links on Linux systems
  std::error_code ec;
  std::string base = std::filesystem::canonical(path, ec).string();
  if (ec)
    return 1;

  // Recursive search deep enough to find everything
  simCore::FileFinder ff(base, ".txt", 2);

  // Even though the absolute path is wrong; simCore::FileFinder should still find the file. SIM-4408
  std::string fileName = ff.find(base + "/Left/Right.TXT", base, true);
  rv += SDK_ASSERT(compareFileNames(fileName, base + "/Right/Right.TXT"));

  return rv;
}

} // namespace

int FileFinderTest(int argc, char* argv[])
{
  if(argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " [filename]" << std::endl;
    return 1;
  }

  int rv = 0;
  rv += testFileFinder(argv[1]);
  rv += testRecursive(argv[1]);
  rv += testMaxDepth(argv[1]);
  rv += testWrongAbsolute(argv[1]);

  return rv;
}
