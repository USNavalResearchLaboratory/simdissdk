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
#include <QCoreApplication>
#include <QDir>
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Time.h"
#include "simQt/ConsoleLogger.h"
#include "simQt/PersistentFileLogger.h"

namespace
{

/** Filename prefix for written logs */
static QString LOG_PREFIX;
/** Wildcard pattern for our logs based on expectation of persistent logger */
static QString WILDCARD_PATTERN;

/** Organization name, expected to be used in the filename generation */
static const QString ORG_DOMAIN = "Naval Research Laboratory";
/** Expected subdirectory under ORG_DOMAIN for log files */
static const QString LOGS_SUBDIRECTORY = "logs";
/** Number of files for testing remove by size and remove by number */
static const int NUM_FILES = 3;

/** Creates a new log file, saves a few lines of text, and closes it */
int writeFileAndClose(int maxAgeSeconds=0, const QString& prefix=LOG_PREFIX)
{
  simQt::PersistentFileLogger logger(prefix);
  // Do a clean now to match the original behavior of setting the max age
  logger.clean(simQt::RemovableFiles(true, maxAgeSeconds, false, 0, false, 0)); // seconds
  int rv = 0;
  rv += SDK_ASSERT(logger.open() == 0);
  // Do a clean now to match the original behavior open and clean
  logger.clean(simQt::RemovableFiles(true, maxAgeSeconds, false, 0, false, 0)); // seconds
  rv += SDK_ASSERT(logger.addText("Line 1\n") == 0);
  rv += SDK_ASSERT(logger.addText("Line 2\n") == 0);
  rv += SDK_ASSERT(logger.addText("Line 3\n") == 0);
  return rv;
}

/** Creates a new log file, saves a few lines of text, and closes it */
int writeFileAndClose(const simQt::RemovableFiles& removable, const QString& prefix = LOG_PREFIX)
{
  simQt::PersistentFileLogger logger(prefix);
  int rv = 0;
  rv += SDK_ASSERT(logger.open() == 0);
  rv += SDK_ASSERT(logger.clean(removable) == 0);
  rv += SDK_ASSERT(logger.addText("Line 1\n") == 0);
  rv += SDK_ASSERT(logger.addText("Line 2\n") == 0);
  rv += SDK_ASSERT(logger.addText("Line 3\n") == 0);
  return rv;
}

/** Returns the full path to the logs/ subdirectory */
QString logsPath()
{
#ifdef WIN32
  // This could crash if APPDATA not defined (which is an error)
  QString path = getenv("LOCALAPPDATA");
#else
  QString path = QDir::homePath() + "/.config";
#endif
  return path + QString("/%1/%2").arg(ORG_DOMAIN, LOGS_SUBDIRECTORY);
}

/** Returns true if any files exist in the logs/ subdirectory */
bool hasFiles(const QString& filePattern=WILDCARD_PATTERN)
{
  // Return 0 if the path doesn't exist, else list contents
  QDir path(logsPath());
  if (!path.exists())
    return false;

  // Catch all log files
  path.setNameFilters(QStringList() << filePattern);
  return !path.entryList(QDir::Files).isEmpty();
}

/** Returns total number of files in the logs/ subdirectory; -1 if directory doesn't exist */
int countFiles(const QString& filePattern = WILDCARD_PATTERN)
{
  // Return 0 if the path doesn't exist, else list contents
  QDir path(logsPath());
  if (!path.exists())
    return -1;

  // Catch all log files
  path.setNameFilters(QStringList() << filePattern);
  return path.entryList(QDir::Files).count();
}

/** Removes all log files matching our file pattern, returning 0 on success */
int removeAllLogs(const QString& filePattern=WILDCARD_PATTERN)
{
  QDir path(logsPath());
  if (!path.exists())
    return 0; // Logs don't exist, no errors

  // Set up the search filter and remove the files
  path.setNameFilters(QStringList() << filePattern);
  int rv = 0;
  auto entryList = path.entryList(QDir::Files);
  for (auto it = entryList.begin(); it != entryList.end(); ++it)
    rv += SDK_ASSERT(path.remove(*it));
  return rv;
}

int testFileRemoveByDate()
{
  int rv = 0;

  // Remove all the log files
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());

  // Write a single file and make sure it's still present
  rv += SDK_ASSERT(writeFileAndClose() == 0);
  rv += SDK_ASSERT(hasFiles());

  // Sleep again to invalidate these logs
  Sleep(50);
  rv += SDK_ASSERT(writeFileAndClose() == 0);
  rv += SDK_ASSERT(hasFiles());

  // Sleep again to invalidate those logs, and write 3 more
  Sleep(50);
  for (int k = 0; k < 3; ++k)
    rv += SDK_ASSERT(writeFileAndClose() == 0);
  rv += SDK_ASSERT(hasFiles());

  // Sleep and make sure it removed those files
  Sleep(50);
  rv += SDK_ASSERT(writeFileAndClose() == 0);
  rv += SDK_ASSERT(hasFiles());

  // Clean up
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());

  return rv;
}

int settingAgeCleansFiles(const QString& logPrefix=LOG_PREFIX, const QString& wildcardForLogs=WILDCARD_PATTERN)
{
  int rv = 0;

  // Remove all the log files
  rv += SDK_ASSERT(removeAllLogs(wildcardForLogs) == 0);
  rv += SDK_ASSERT(!hasFiles(wildcardForLogs));

  // Write 2 files in quick succession
  rv += SDK_ASSERT(writeFileAndClose(100, logPrefix) == 0);
  rv += SDK_ASSERT(writeFileAndClose(100, logPrefix) == 0);
  rv += SDK_ASSERT(hasFiles(wildcardForLogs));

  {
    // Create a logger object so we can test setMaximumAgeSeconds()
    simQt::PersistentFileLogger fileLogger(logPrefix);
    rv += SDK_ASSERT(fileLogger.open() == 0);
    rv += SDK_ASSERT(fileLogger.addText("Sample text\n") == 0);
    rv += SDK_ASSERT(hasFiles(wildcardForLogs));
    fileLogger.clean(simQt::RemovableFiles(true, 10, false, 0, false, 0));; // Shouldn't delete anything
    rv += SDK_ASSERT(hasFiles(wildcardForLogs));

    // Have a very short sleep, then change the max age to 0 (should clear out other files)
    Sleep(1);
    fileLogger.clean(simQt::RemovableFiles(true, 0, false, 0, false, 0));
    rv += SDK_ASSERT(hasFiles(wildcardForLogs));

    // Write out the to file logger to ensure it's still valid
    rv += SDK_ASSERT(fileLogger.addText("Another sample.\n") == 0);
  }

  // Now delete the files and return
  rv += SDK_ASSERT(hasFiles(wildcardForLogs));
  rv += SDK_ASSERT(removeAllLogs(wildcardForLogs) == 0);
  rv += SDK_ASSERT(!hasFiles(wildcardForLogs));

  return rv;
}

int testBadFilename()
{
  const QString weirdPrefix = "w\\e/i<r\"d>_f*i:l?en|ame";
  const QString weirdLogWildcard = QString("weird_filename_*.log*");
  return settingAgeCleansFiles(weirdPrefix, weirdLogWildcard);
}

int writeWithoutOpenIsError()
{
  int rv = 0;
  // Remove all the log files
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());

  {
    // Create a logger object so we can test setMaximumAgeMsec()
    simQt::PersistentFileLogger fileLogger(LOG_PREFIX);
    rv += SDK_ASSERT(fileLogger.addText("Sample text\n") != 0);
    rv += SDK_ASSERT(!hasFiles());
    rv += SDK_ASSERT(fileLogger.open() == 0);
    rv += SDK_ASSERT(hasFiles());
  }

  // Didn't clean up yet...
  rv += SDK_ASSERT(hasFiles());
  // Clean up now
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());
  return rv;
}

int testFileRemoveByNumber()
{
  int rv = 0;
  // Remove all the log files
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(countFiles() == 0);

  // Create the files
  for (int ii = 0; ii < NUM_FILES; ++ii)
  {
    writeFileAndClose(simQt::RemovableFiles(false, 0, false, 0, true, 10));
    // Sleep between file creates to they get unique names
    if (ii != (NUM_FILES-1))
      Sleep(1100);
    int count = countFiles();
    rv += SDK_ASSERT(count == (1 + ii));
    if (count != (1 + ii))
      std::cout << "In testFileRemoveByNumber adding files Expecting " << 1 + ii << " Got " << count << std::endl;
  }

  // Delete the files one by one
  for (int ii = 0; ii < NUM_FILES; ++ii)
  {
    simQt::PersistentFileLogger fileLogger(LOG_PREFIX);
    fileLogger.clean(simQt::RemovableFiles(false, 0, false, 0, true, NUM_FILES - 1 - ii));
    int count = countFiles();
    rv += SDK_ASSERT(count == (NUM_FILES - 1  - ii));
    if (count != (NUM_FILES - 1 - ii))
      std::cout << "In testFileRemoveByNumber deleting files Expecting " << NUM_FILES - 1 - ii << " Got " << count << std::endl;
  }

  // Should be clean
  rv += SDK_ASSERT(!hasFiles());
  // Clean up just in case
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());

  return rv;
}

int testFileRemoveBySize()
{
  int rv = 0;
  // Remove all the log files
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(countFiles() == 0);

  // Create the files
  for (int ii = 0; ii < NUM_FILES; ++ii)
  {
    writeFileAndClose(simQt::RemovableFiles(false, 0, true, 4000, false, 0));
    // Sleep between file creates to they get unique names
    if (ii != (NUM_FILES - 1))
      Sleep(1100);
    int count = countFiles();
    rv += SDK_ASSERT(count == (1 + ii));
    if (count != (1 + ii))
      std::cout << "In testFileRemoveBySize adding files Expecting " << 1 + ii << " Got " << count << std::endl;
  }

  // Delete the files one by one
#ifdef _WIN32
  int fileSize = 24;  // File sizes are different because of CR/LF
#else
  int fileSize = 21;
#endif

  for (int ii = 0; ii < NUM_FILES; ++ii)
  {
    simQt::PersistentFileLogger fileLogger(LOG_PREFIX);
    fileLogger.clean(simQt::RemovableFiles(false, 0, true, fileSize*(NUM_FILES - ii) - 1, false, 0));
    int count = countFiles();
    rv += SDK_ASSERT(count == (NUM_FILES - 1 - ii));
    if (count != (NUM_FILES - 1 - ii))
      std::cout << "In testFileRemoveBySize deleting files Expecting " << NUM_FILES - 1 - ii << " Got " << count << std::endl;
  }

  // Should be clean
  rv += SDK_ASSERT(!hasFiles());
  // Clean up just in case
  rv += SDK_ASSERT(removeAllLogs() == 0);
  rv += SDK_ASSERT(!hasFiles());

  return rv;
}

}


int PersistentLoggerTest(int argc, char* argv[])
{
  int rv = 0;

#ifdef WIN32
  unsigned int pid = static_cast<unsigned int>(GetCurrentProcessId());
#else
  unsigned int pid = static_cast<unsigned int>(getpid());
#endif

  LOG_PREFIX = "LogTest_" + QString::number(pid) + "_";;
  WILDCARD_PATTERN = QString("%1_*.log*").arg(LOG_PREFIX);

  QCoreApplication::setApplicationName("Persistent Logger Test");
  QCoreApplication::setOrganizationName(ORG_DOMAIN);
  QCoreApplication::setOrganizationDomain("https://www.trmc.osd.mil/simdis.html");

  rv += SDK_ASSERT(testFileRemoveByDate() == 0);
  rv += SDK_ASSERT(settingAgeCleansFiles() == 0);
  rv += SDK_ASSERT(testBadFilename() == 0);
  rv += SDK_ASSERT(writeWithoutOpenIsError() == 0);

  rv += SDK_ASSERT(testFileRemoveByNumber() == 0);
  rv += SDK_ASSERT(testFileRemoveBySize() == 0);

  return rv;
}
