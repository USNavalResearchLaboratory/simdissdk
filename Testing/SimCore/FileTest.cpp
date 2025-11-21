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
#include <filesystem>
#include <fstream>
#include <iostream>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/ScopeGuard.h"
#include "simCore/String/Utils.h"
#include "simCore/System/File.h"

#ifndef WIN32
#include <unistd.h>
#endif

namespace {

int testFileInfo()
{
  int rv = 0;

#ifdef WIN32
  // On some Windows test machines, __FILE__ uses backslashes, messes up direct comparisons below
  const std::string thisCppFile = simCore::backslashToFrontslash(__FILE__);
#else
  const std::string thisCppFile = __FILE__;
#endif

  // Make sure the file exists. If not, the rest of the test is bogus.
  std::ifstream ifs(thisCppFile);
  if (!ifs)
  {
    std::cerr << "Unable to run testFileInfo(), CPP file does not exist.\n"
      << "This test application is non-portable.\n";
    return 0;
  }
  ifs.close();

  const simCore::FileInfo thisCppFileInfo(thisCppFile);
  rv += SDK_ASSERT(thisCppFileInfo.exists());
  rv += SDK_ASSERT(thisCppFileInfo.isRegularFile());
  rv += SDK_ASSERT(!thisCppFileInfo.isDirectory());
  rv += SDK_ASSERT(thisCppFileInfo.filePath() == thisCppFile);
  rv += SDK_ASSERT(thisCppFileInfo.absoluteFilePath() == thisCppFile);

#ifdef WIN32
  const std::string rootFilePath = "C:/test";
#else
  const std::string rootFilePath = "/usr/test";
#endif

  // Confirm that FileInfo properly handles a path that is just a drive
  const simCore::FileInfo rootLevelFile(rootFilePath);
  auto [path, name] = simCore::pathSplit(rootFilePath);
#ifdef WIN32
  path = path + "/"; // Slash is relevant to root path for Windows drives
#endif
  rv += SDK_ASSERT(rootLevelFile.filePath() == rootFilePath);
  rv += SDK_ASSERT(rootLevelFile.absolutePath() == path);
  rv += SDK_ASSERT(rootLevelFile.path() == path);

  simCore::FileInfo cwdInfo(".");
  rv += SDK_ASSERT(cwdInfo.exists());
  rv += SDK_ASSERT(!cwdInfo.isRegularFile());
  rv += SDK_ASSERT(cwdInfo.isDirectory());
  rv += SDK_ASSERT(!cwdInfo.absolutePath().empty());
  rv += SDK_ASSERT(cwdInfo.makeAbsolute());
  rv += SDK_ASSERT(cwdInfo.path() == cwdInfo.absolutePath());

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
  const std::string& PS = simCore::PATH_SEPARATOR;
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

  // UNC paths should work with forward or back slashes on Windows
  rv += SDK_ASSERT(simCore::pathJoin({ "//unc/path/dir", "file" }) == "//unc/path/dir" + PS + "file");
  rv += SDK_ASSERT(simCore::pathJoin({ "\\\\unc\\path\\dir", "file" }) == "\\\\unc\\path\\dir" + PS + "file");

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

int testPathSplit()
{
  int rv = 0;

  /** Helper function that returns the joined string of a split path */
  const auto splitJoin = [](const std::string& path) -> std::string {
    const auto& [split1, split2] = simCore::pathSplit(path);
    return simCore::pathJoin({ split1, split2 });
    };
  const std::string& PS = simCore::PATH_SEPARATOR;

  rv += SDK_ASSERT(simCore::pathSplit("a/b") == std::make_tuple("a", "b"));
  rv += SDK_ASSERT(splitJoin("a/b") == "a" + PS + "b");
  rv += SDK_ASSERT(simCore::pathSplit("a/b/c") == std::make_tuple("a/b", "c"));
  rv += SDK_ASSERT(splitJoin("a/b/c") == "a/b" + PS + "c");
  rv += SDK_ASSERT(simCore::pathSplit("a") == std::make_tuple("", "a"));
  rv += SDK_ASSERT(splitJoin("a") == "a");
  rv += SDK_ASSERT(simCore::pathSplit("") == std::make_tuple("", ""));
  rv += SDK_ASSERT(splitJoin("") == "");

  // Multiple slashes
  rv += SDK_ASSERT(simCore::pathSplit("a/b///c") == std::make_tuple("a/b", "c"));
  rv += SDK_ASSERT(splitJoin("a/b///c") == "a/b" + PS + "c");
  rv += SDK_ASSERT(simCore::pathSplit("a//b///c") == std::make_tuple("a//b", "c"));
  rv += SDK_ASSERT(splitJoin("a//b///c") == "a//b" + PS + "c");
  rv += SDK_ASSERT(simCore::pathSplit("a//b/c") == std::make_tuple("a//b", "c"));
  rv += SDK_ASSERT(splitJoin("a//b/c") == "a//b" + PS + "c");

  // Starting slash
  rv += SDK_ASSERT(simCore::pathSplit("/") == std::make_tuple("/", ""));
  rv += SDK_ASSERT(splitJoin("/") == "/");
  rv += SDK_ASSERT(simCore::pathSplit("////") == std::make_tuple("////", ""));
  rv += SDK_ASSERT(splitJoin("////") == "////");
  rv += SDK_ASSERT(simCore::pathSplit("/abc") == std::make_tuple("/", "abc"));
  rv += SDK_ASSERT(splitJoin("/abc") == "/abc");
  rv += SDK_ASSERT(simCore::pathSplit("////abc") == std::make_tuple("////", "abc"));
  rv += SDK_ASSERT(splitJoin("////abc") == "////abc");

  // Trailing slash
  rv += SDK_ASSERT(simCore::pathSplit("/ab/c/") == std::make_tuple("/ab/c", ""));
  rv += SDK_ASSERT(splitJoin("/ab/c/") == "/ab/c" + PS);
  rv += SDK_ASSERT(simCore::pathSplit("/ab/c///") == std::make_tuple("/ab/c", ""));
  rv += SDK_ASSERT(splitJoin("/ab/c///") == "/ab/c" + PS);

  // UNC path testing with forward slash
  rv += SDK_ASSERT(simCore::pathSplit("//host/path/file") == std::make_tuple("//host/path", "file"));

  // Windows allows backslash as a separator, but Linux does not, so different tests
  // with different outcomes.
#ifdef WIN32
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\b)") == std::make_tuple("a", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\\b)") == std::make_tuple("a", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a/\b)") == std::make_tuple("a", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\/\/\b)") == std::make_tuple("a", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\a\b)") == std::make_tuple(R"(\\a)", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\a/b)") == std::make_tuple(R"(\\a)", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\)") == std::make_tuple("a", ""));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\\/\\)") == std::make_tuple("a", ""));

  // UNC Path testing
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\host\path\file)") == std::make_tuple(R"(\\host\path)", "file"));
#else
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\b)") == std::make_tuple("", "a\\b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\\b)") == std::make_tuple("", "a\\\\b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a/\b)") == std::make_tuple("a", "\\b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\/\/\b)") == std::make_tuple("a\\/\\", "\\b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\a\b)") == std::make_tuple("", R"(\\a\b)"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\a/b)") == std::make_tuple(R"(\\a)", "b"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\)") == std::make_tuple("", "a\\"));
  rv += SDK_ASSERT(simCore::pathSplit(R"(a\\/\\)") == std::make_tuple(R"(a\\)", R"(\\)"));

  // UNC Path testing
  rv += SDK_ASSERT(simCore::pathSplit(R"(\\host\path\file)") == std::make_tuple("", R"(\\host\path\file)"));
#endif

  return rv;
}

void touchFile(const std::string& filename)
{
  std::ofstream ofs(filename, std::ios::app | std::ios::binary);
  ofs.close();
}

int testMkdirAndRemove()
{
  int rv = 0;

  std::error_code unused;
  const std::string& systemTemp = std::filesystem::temp_directory_path(unused).string();
  // Create an empty testing directory; all our files go in here as a clean test. First,
  // make sure the directory is empty, removing it if it exists
  const std::string& tmpDir = simCore::pathJoin({ systemTemp, "testMkdirTmp" });
  if (simCore::FileInfo(tmpDir).exists())
    rv += SDK_ASSERT(simCore::remove(tmpDir, true) == 0);

  // Create the temporary directory and delete it when we fall out of scope
  rv += SDK_ASSERT(!simCore::FileInfo(tmpDir).exists());
  rv += SDK_ASSERT(simCore::mkdir(tmpDir) == 0);
  const simCore::ScopeGuard rmOurTemp([tmpDir]() { simCore::remove(tmpDir, true); });
  rv += SDK_ASSERT(simCore::FileInfo(tmpDir).exists());
  rv += SDK_ASSERT(simCore::FileInfo(tmpDir).isDirectory());

  // Start real testing. First make sure that recursive flag works
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "a/b/c" })) != 0);
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "c" })) == 0);

  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b/c" })).exists());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" })).exists());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" })).isDirectory());

  // First without make-parents
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "a/b/c" })) != 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b/c" })).exists());
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b/c" })).isDirectory());

  // Now with make-parents
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "a/b/c" }), true) == 0);
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b/c" })).exists());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b/c" })).isDirectory());

  // Test that when we remove b, recursive flag matters
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b" })).isDirectory());
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "a/b" })) != 0);
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b" })).isDirectory());
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "a/b" }), true) == 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "a/b" })).isDirectory());

  // "a" is empty, remove it
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "a" })).isDirectory());
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "a" })) == 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "a" })).isDirectory());

  // Test non-existing remove (a does not exist)
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "a" })) != 0);
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "a" }), true) != 0);

  // Test files
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" })).isDirectory());
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" })).exists());
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f2" })).exists());
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f3" })).exists());

  touchFile(simCore::pathJoin({ tmpDir, "c/f1" }));
  touchFile(simCore::pathJoin({ tmpDir, "c/f2" }));
  touchFile(simCore::pathJoin({ tmpDir, "c/f3" }));
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" })).isRegularFile());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f2" })).isRegularFile());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f3" })).isRegularFile());

  // Test equivalence
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/f1" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/f2" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/../f1" })));
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/../c/f1" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/../c" })));
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/../c" })));
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c" })));
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "c/f1" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "d" })));
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "e" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "d" })));
  // Though the same path, neither one exists, and therefore cannot be equivalent
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "e" }))
    .isEquivalent(simCore::pathJoin({ tmpDir, "e" })));

  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "c/f1" })) == 0);
  // Can't remove more than once
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "c/f1" })) != 0);
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "c/f1" }), true) != 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f1" })).exists());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f2" })).isRegularFile());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f3" })).isRegularFile());

  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "c/f2" }), true) == 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f2" })).exists());
  rv += SDK_ASSERT(simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f3" })).isRegularFile());

  // Recursive remove on parent dir should also get rid of the remaining file
  rv += SDK_ASSERT(simCore::remove(simCore::pathJoin({ tmpDir, "c" }), true) == 0);
  rv += SDK_ASSERT(!simCore::FileInfo(simCore::pathJoin({ tmpDir, "c/f3" })).isRegularFile());

  return rv;
}

int testWritable()
{
  int rv = 0;

  std::error_code unused;
  const std::string& systemTemp = std::filesystem::temp_directory_path(unused).string();
  // Create an empty testing directory; all our files go in here as a clean test. First,
  // make sure the directory is empty, removing it if it exists
  const std::string& tmpDir = simCore::pathJoin({ systemTemp, "testWritable" });
  if (simCore::FileInfo(tmpDir).exists())
    rv += SDK_ASSERT(simCore::remove(tmpDir, true) == 0);

  // Create the directory and make sure it's in a reasonable state
  rv += SDK_ASSERT(simCore::mkdir(tmpDir) == 0);
  const simCore::ScopeGuard rmOurTemp([tmpDir]() { simCore::remove(tmpDir, true); });
  rv += SDK_ASSERT(simCore::FileInfo(tmpDir).exists());
  rv += SDK_ASSERT(simCore::FileInfo(tmpDir).isDirectory());

  // Should be writable
  rv += SDK_ASSERT(simCore::isDirectoryWritable(tmpDir));
  // Directory that does not exist should not be writable.
  rv += SDK_ASSERT(!simCore::isDirectoryWritable(simCore::pathJoin({ tmpDir, "doesNotExist" })));

  // If a file is there, then it shouldn't be writable
  touchFile(simCore::pathJoin({ tmpDir, "file" }));
  rv += SDK_ASSERT(!simCore::isDirectoryWritable(simCore::pathJoin({ tmpDir, "file" })));

  // Create a subdirectory and make sure it's writable
  const std::string& subdir = simCore::pathJoin({ tmpDir, "dir" });
  simCore::mkdir(subdir);
  rv += SDK_ASSERT(simCore::isDirectoryWritable(subdir));

  // Set the directory to not-writable, then retest; this only works on Linux
  // because Windows ACL overrides the chmod here. Linux also fails if root.
#ifndef WIN32
  if (geteuid() != 0)
  {
    std::filesystem::permissions(subdir, std::filesystem::perms::none, unused);
    rv += SDK_ASSERT(!simCore::isDirectoryWritable(subdir));
    std::filesystem::permissions(subdir, std::filesystem::perms::all, unused);
    rv += SDK_ASSERT(simCore::isDirectoryWritable(subdir));
  }
#endif

  return rv;
}

int testFilesMissingFromPath()
{
  int rv = 0;

  // Create top level testing directory
  std::error_code unused;
  const std::string& systemTemp = std::filesystem::temp_directory_path(unused).string();
  const std::string& tmpDir = simCore::pathJoin({ systemTemp, "testFilesMissingFromPath" });
  simCore::remove(tmpDir, true);
  rv += SDK_ASSERT(simCore::mkdir(tmpDir) == 0);
  const simCore::ScopeGuard rmOurTemp([tmpDir]() { simCore::remove(tmpDir, true); });

  // Create some subdirectories and files
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "a/b/c" }), true) == 0);
  rv += SDK_ASSERT(simCore::mkdir(simCore::pathJoin({ tmpDir, "d/e/f" }), true) == 0);
  touchFile(simCore::pathJoin({ tmpDir, "a", "a" }));
  touchFile(simCore::pathJoin({ tmpDir, "a/b/c", "abc" }));
  touchFile(simCore::pathJoin({ tmpDir, "d/e", "de" }));
  touchFile(simCore::pathJoin({ tmpDir, "d/e", "de2" }));

  // Empty list is noop empty return
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, {}).empty());

  // Test single existing files
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a/a" }).empty());
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a/b/c/abc" }).empty());
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "d/e/de" }).empty());
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "d/e/de2" }).empty());

  // Directories should fail
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a" }).size() == 1);
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a", "d" }).size() == 2);
  // Missing files should fail
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a/b" }).size() == 1);
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "a/a", "a/b" }).size() == 1);
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir, { "doesnotexist" }).size() == 1);

  // Use a different path and change relative subdirectories
  rv += SDK_ASSERT(simCore::filesMissingFromPath(tmpDir + "/a", {"a", "b"}).size() == 1);

  return rv;
}

int testFileInfoNamePath()
{
  int rv = 0;

  rv += SDK_ASSERT(simCore::FileInfo("/tmp/foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp/two/foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("c:/tmp/foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt/baz").fileName() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("/foo").fileName() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("//a//foo").fileName() == "foo");

#ifdef WIN32
  // UNC path, cannot be "foo"
  rv += SDK_ASSERT(simCore::FileInfo("//foo").fileName() == "");
#else
  // UNC path not supported in same way, so "foo"
  rv += SDK_ASSERT(simCore::FileInfo("//foo").fileName() == "foo");
#endif

  rv += SDK_ASSERT(simCore::FileInfo("/foo/").fileName() == "");
  rv += SDK_ASSERT(simCore::FileInfo("foo").fileName() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/").fileName() == "");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar").fileName() == "bar");
  rv += SDK_ASSERT(simCore::FileInfo("foo//bar").fileName() == "bar");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar/baz").fileName() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("/").fileName() == "");
  rv += SDK_ASSERT(simCore::FileInfo("").fileName() == "");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp///foo/bar").fileName() == "bar");

#ifdef WIN32
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").fileName() == "foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").fileName() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").fileName() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").fileName() == "file");
#else
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").fileName() == "c:\\foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").fileName() == "c:\\tmp\\foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").fileName() == "bar\\baz");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").fileName() == "foo\\bar\\baz");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").fileName() == "\\\\host\\unc\\path\\file");
#endif

  rv += SDK_ASSERT(simCore::FileInfo("/tmp/foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp/foo.one.two.three").fileNameStem() == "foo.one.two");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp/two/foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("c:/tmp/foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt/baz").fileNameStem() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("/foo").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("//a//foo").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("/foo/").fileNameStem() == "");
  rv += SDK_ASSERT(simCore::FileInfo("foo").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/").fileNameStem() == "");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar").fileNameStem() == "bar");
  rv += SDK_ASSERT(simCore::FileInfo("foo//bar").fileNameStem() == "bar");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar/baz").fileNameStem() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("/").fileNameStem() == "");
  rv += SDK_ASSERT(simCore::FileInfo("").fileNameStem() == "");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp///foo/bar").fileNameStem() == "bar");

#ifdef WIN32
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").fileNameStem() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").fileNameStem() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").fileNameStem() == "baz");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").fileNameStem() == "file");
  rv += SDK_ASSERT(simCore::FileInfo("//foo").fileNameStem() == ""); // UNC path, it can't be a filename
#else
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").fileNameStem() == "c:\\foo");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").fileNameStem() == "c:\\tmp\\foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").fileNameStem() == "bar\\baz");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").fileNameStem() == "foo\\bar\\baz");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").fileNameStem() == "\\\\host\\unc\\path\\file");
  rv += SDK_ASSERT(simCore::FileInfo("//foo").fileNameStem() == "foo");
#endif

  rv += SDK_ASSERT(simCore::FileInfo("/tmp/foo.txt").path() == "/tmp");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp/two/foo.txt").path() == "/tmp/two");
  rv += SDK_ASSERT(simCore::FileInfo("c:/tmp/foo.txt").path() == "c:/tmp");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt").path() == "/");
  rv += SDK_ASSERT(simCore::FileInfo("/foo.txt/baz").path() == "/foo.txt");
  rv += SDK_ASSERT(simCore::FileInfo("/foo").path() == "/");

#ifdef WIN32
  // UNC path
  rv += SDK_ASSERT(simCore::FileInfo("//foo").path() == "//foo");
#else
  // UNC path not supported in same way, so //
  rv += SDK_ASSERT(simCore::FileInfo("//foo").path() == "/");
#endif

  rv += SDK_ASSERT(simCore::FileInfo("/foo/").path() == "/foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo").path() == ".");
  rv += SDK_ASSERT(simCore::FileInfo("foo/").path() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar").path() == "foo");
  // Note, the below behavior differs from QFileInfo::path(), which returns "foo/"
  rv += SDK_ASSERT(simCore::FileInfo("foo//bar").path() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar/baz").path() == "foo/bar");
  rv += SDK_ASSERT(simCore::FileInfo("/").path() == "/");
  rv += SDK_ASSERT(simCore::FileInfo("").path() == "");
  rv += SDK_ASSERT(simCore::FileInfo("/tmp///foo/bar").path() == "/tmp/foo");

#ifdef WIN32
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").path() == "c:/");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").path() == "c:/tmp");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").path() == "foo/bar");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").path() == "foo/bar");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").path() == "//host/unc/path");
#else
  rv += SDK_ASSERT(simCore::FileInfo("c:\\foo.txt").path() == ".");
  rv += SDK_ASSERT(simCore::FileInfo("c:\\tmp\\foo.txt").path() == ".");
  rv += SDK_ASSERT(simCore::FileInfo("foo/bar\\baz").path() == "foo");
  rv += SDK_ASSERT(simCore::FileInfo("foo\\bar\\baz").path() == ".");
  rv += SDK_ASSERT(simCore::FileInfo(R"(\\host\unc\path\file)").path() == ".");
#endif

  return rv;
}

int testNormalizeFile()
{
  int rv = 0;

  // ExpandEnv tested elsewhere, just a cursory check to make sure it's happening
#ifdef WIN32
  rv += SDK_ASSERT(simCore::normalizeFilepath("$(SIMDIS_DIR)\\path") != "$(SIMDIS_DIR)\\path");
#else
  rv += SDK_ASSERT(simCore::normalizeFilepath("$(SIMDIS_DIR)/path") != "$(SIMDIS_DIR)/path");
#endif

  // Test removal of useless dots
#ifdef WIN32
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\a\\.\\") == "\\a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath(".\\a\\") == "a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\.\\a\\") == "\\a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("a\\b\\.\\") == "a\\b\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:\\a\\b\\.\\") == "C:\\a\\b\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:\\a\\.\\b\\") == "C:\\a\\b\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\a\\b\\..\\") == "\\a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\a\\b\\..\\file") == "\\a\\file");
#else
  rv += SDK_ASSERT(simCore::normalizeFilepath("/a/./") == "/a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("./a/") == "a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("/./a/") == "/a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("a/b/./") == "a/b/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:/a/b/./") == "C:/a/b/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:/a/./b/") == "C:/a/b/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("/a/b/../") == "/a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("/a/b/../file") == "/a/file");
#endif


  // Test that valid dots are left
#ifdef WIN32
  rv += SDK_ASSERT(simCore::normalizeFilepath("..\\a\\.\\") == "..\\a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("..\\..\\a\\.\\") == "..\\..\\a\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("a\\..\\..\\") == ".."); // For a relative path, a\\ cancels the first .. (go into a, then back up), leaving only the second (one level above start)
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\a\\..\\") == "\\"); // For an absolute path, only the root directory is above "\\a\\"
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\a\\..\\..\\") == "\\"); // Cannot go beyond root
#else
  rv += SDK_ASSERT(simCore::normalizeFilepath("../a/./") == "../a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("../../a/./") == "../../a/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("a/../../") == ".."); // For a relative path, a/ cancels the first .. (go into a, then back up), leaving only the second (one level above start)
  rv += SDK_ASSERT(simCore::normalizeFilepath("/a/../") == "/"); // For an absolute path, only the root directory is above /a/
  rv += SDK_ASSERT(simCore::normalizeFilepath("/a/../../") == "/"); // Cannot go beyond root
#endif


  // Test normalization of slashes
#ifdef WIN32
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:\\test\\path\\") == "C:\\test\\path\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:/test/path/") == "C:\\test\\path\\");
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\test\\\\path\\") == "\\test\\path\\"); // Removal of extra slashes
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\\\test\\\\path\\") == "\\\\test\\path\\"); // Double slash at start left alone
  rv += SDK_ASSERT(simCore::normalizeFilepath("\\\\\\test\\\\path\\") == "\\test\\path\\"); // Any more than two slashes at start treated as incorrect, reverts to single
#else
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:\\test\\path\\") == "C:\\test\\path\\"); // std::filesystem leaves backslashes as-is on Linux, since they are valid filename characters
  rv += SDK_ASSERT(simCore::normalizeFilepath("C:/test/path/") == "C:/test/path/");
  rv += SDK_ASSERT(simCore::normalizeFilepath("/test///path/") == "/test/path/"); // Removal of extra slashes
  rv += SDK_ASSERT(simCore::normalizeFilepath("//test///path/") == "/test/path/"); // Double slash doesn't mean anything on Linux
  rv += SDK_ASSERT(simCore::normalizeFilepath("///test///path/") == "/test/path/"); // Any more than two slashes at start treated as incorrect, reverts to single
#endif

  return rv;
}

}

int FileTest(int argc, char* argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testFileInfo() == 0);
  rv += SDK_ASSERT(testPathJoin() == 0);
  rv += SDK_ASSERT(testPathSplit() == 0);
  rv += SDK_ASSERT(testMkdirAndRemove() == 0);
  // recycle() is intentionally not tested to avoid cluttering recycling bin
  rv += SDK_ASSERT(testWritable() == 0);
  rv += SDK_ASSERT(testFilesMissingFromPath() == 0);
  rv += SDK_ASSERT(testFileInfoNamePath() == 0);
  rv += SDK_ASSERT(testNormalizeFile() == 0);

  std::cout << "simCore FileTest: " << (rv == 0 ? "PASSED" : "FAILED") << "\n";

  return rv;
}
