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
#include <cassert>
#include <filesystem>
#include <QDir>
#include <QCoreApplication>
#include "simQt/FileUtilities.h"

namespace simQt {

bool FileUtilities::isPathWritable(const QString& absoluteFilePath)
{
  // This is equivalent to "mkdir -p absoluteFilePath". Using this covers
  // an edge case where the input path looks like "c:/path/to/a/dir". With
  // this call, this works. Without this create_directories() call,
  // isPathWritable() will fail if "c:/path" exists but "to/a/dir" does not.
  // For isPathWritable() to succeed without this, "c:/path/to/a" must
  // exist. This allows for the path to create multiple directories.
  std::error_code fsError;
  std::filesystem::create_directories(absoluteFilePath.toStdString(), fsError);
  // ignore fsError, falling back to non-filesystem behavior, which likely
  // will cause us to return false.

  // This will create the absolute file path, but only if "{absoluteFilePath}/.." exists
  const QDir testDir(absoluteFilePath);
  if (testDir.mkpath("testWritable"))
  {
    testDir.rmdir("testWritable");
    return true;
  }
  return false;
}


int FileUtilities::createHomePath(const QString& relativeFilePath, bool roaming, QString& absolutePath)
{
  if (!QDir::isRelativePath(relativeFilePath))
  {
    // Almost certainly a developer error.  relativeFilePath should be a relative format, not absolute
    assert(0);
    return 3;
  }

  // Default the application data to the home path
  absolutePath = QDir::homePath();
#ifdef WIN32
  // Assertion failure means that Windows API has changed and no longer supplies this
  assert(getenv("APPDATA") != nullptr);
  // Pull out the APPDATA variable
  const char* appDataCstr = (roaming ? getenv("APPDATA") : getenv("LOCALAPPDATA"));
  if (appDataCstr != nullptr)
    absolutePath = appDataCstr;
  else if (absolutePath.isEmpty()) // would only happen if QDir::homePath() is empty
    return 2;
#else
  if (absolutePath.isEmpty()) // would only happen if QDir::homePath() is empty
    return 2;
  // Append .config, the Qt directory
  absolutePath += "/.config";
#endif

  // Create the subdirectories needed based on the QCoreApplication Organization name
  QString orgName = QCoreApplication::organizationName();
  if (orgName.isEmpty())
    orgName = "SIMDIS SDK";
  absolutePath += "/" + orgName + "/" + relativeFilePath;

  // If we're roaming and can't write, then fall back to non-roaming position
  if (!FileUtilities::isPathWritable(absolutePath))
  {
#ifdef WIN32
    if (roaming)
      return FileUtilities::createHomePath(relativeFilePath, !roaming, absolutePath);
#endif
    absolutePath.clear();
    return 1;
  }
  absolutePath = QDir::cleanPath(absolutePath);
  absolutePath = QDir::toNativeSeparators(absolutePath);
  return 0;
}

}
