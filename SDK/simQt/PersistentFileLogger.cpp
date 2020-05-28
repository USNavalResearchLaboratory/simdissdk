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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <cassert>
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QRegExp>
#include "simNotify/Notify.h"
#include "simQt/FileUtilities.h"
#include "simQt/PersistentFileLogger.h"

namespace simQt {

/** Path under (Application Data/Organization) folder for logs */
static const QString LOGS_SUBDIRECTORY = "logs";

/** Filename pattern, including Prefix, DateTime string, and PID string */
static const QString FILENAME_PATTERN("%1_%2_%3.log");

/** Describes the format for the date time.  If updating, update the wildcard pattern */
static const QString DATETIME_STRING_FORMAT("yyyy-MM-dd_hh-mm-ss");
/** Prefix, y-m-d, h-m-s, pid, .log, optional .# */
static const QString WILDCARD_PATTERN = "%1_*-*-*_*-*-*_*.log*";

/** Illegal filename characters */
static const QString ILLEGAL_FILENAME_CHARS = "\\/:*?\"<>|";
/** Regex matcher for illegal filename characters */
static const QRegExp ILLEGAL_REGEXP(QString("[%1]").arg(QRegExp::escape(ILLEGAL_FILENAME_CHARS)));

PersistentFileLogger::PersistentFileLogger(const QString& prefix, QObject* parent)
  : QObject(parent),
    prefix_(sanitizeFilename_(prefix)),
    startTime_(QDateTime::currentDateTime().toUTC()),
    file_(NULL),
    stream_(NULL),
    openAttempted_(false)
{
}

PersistentFileLogger::~PersistentFileLogger()
{
  delete stream_;
  delete file_;
}

int PersistentFileLogger::addText(const QString& text)
{
  if (!stream_ || stream_->status() != QTextStream::Ok)
    return 1;

  (*stream_) << text;
  stream_->flush();
  return (stream_->status() == QTextStream::Ok) ? 0 : 1;
}

bool PersistentFileLogger::isOpen() const
{
  return stream_ != NULL && file_ != NULL  && stream_->status() == QTextStream::Ok;
}

QString PersistentFileLogger::filePath() const
{
  return filePath_;
}

int PersistentFileLogger::open()
{
  // make sure stream is not open
  if (stream_ != NULL || file_ != NULL || openAttempted_)
  {
    // Check for an error on the stream before returning
    if (stream_ == NULL || stream_->status() != QTextStream::Ok)
      return 1;
    // File is open, all is good for writing
    return 0; // success
  }

  // Make sure we don't re-enter this function.  This could happen if we output an error
  // message (for example) but the error message gets routed back to the log file
  openAttempted_ = true;

  // Open the file
  filePath_ = createFilePath_();
  const QDir logsDir = filePath_;
  if (!logsDir.exists())
    return 1; // Log directory doesn't exist, can't open file

  // Open the file and the stream
  filename_ = logsDir.filePath(makeFileName_(logsDir));
  file_ = new QFile(filename_);
  if (!file_->open(QFile::WriteOnly | QFile::Text))
  {
    SIM_DEBUG << "Unable to open: " << filename_.toStdString() << "\n";
    delete file_;
    file_ = NULL;
    return 1;
  }
  // Create the stream to write to the file
  stream_ = new QTextStream(file_);
  return 0;
}

int PersistentFileLogger::clean(const DetermineRemovable& removable)
{
  QDir logsDir = createFilePath_();
  // If the path doesn't exist, no log files to remove
  if (!logsDir.exists())
    return 1;

  // Set up a name filter to catch only files that match our prefix
  logsDir.setNameFilters(QStringList() << WILDCARD_PATTERN.arg(prefix_));

  // Determine which files to delete
  QStringList filesToDelete;
  removable.calculate(logsDir, filesToDelete);

  int rv = 0;
  // Try to remove the files we just gathered
  for (auto it = filesToDelete.begin(); it != filesToDelete.end(); ++it)
  {
    // Don't delete current file
    if (filename_ == (logsDir.path() + "/" +  *it))
      continue;

    if (!logsDir.remove(*it))
    {
      SIM_WARN << "Unable to remove " << (*it).toStdString() << "\n";
      rv = 1;
    }
  }

  return rv;
}

QString PersistentFileLogger::createFilePath_() const
{
  // Return the already-found file path if it's been set already
  if (!filePath_.isEmpty())
    return filePath_;

  // Use the File Utilities to create the home path
  QString returnPath;
  if (FileUtilities::createHomePath(LOGS_SUBDIRECTORY, false, returnPath) == 0)
  {
    return returnPath;
  }
  // Fall back on homePath(), which also might not be writable
  return QDir::homePath();
}

QString PersistentFileLogger::makeFileName_(const QDir& filePath) const
{
  // Try to get a unique name.  This should only matter if we make the same file multiple times
  // in a short time span (e.g. during unit testing)
  const QString rv = expectedFileName_();
  if (filePath.exists(rv))
  {
    // Attempt a few times; if we don't get a name in 10 tries, just fall back to default
    for (int k = 1; k < 10; ++k)
    {
      // Append a .#
      const QString possibleName = QString("%1.%2").arg(rv).arg(k);
      if (!filePath.exists(possibleName))
        return possibleName;
    }
  }

  // Return the calculated name
  return rv;
}

QString PersistentFileLogger::expectedFileName_() const
{
#ifdef WIN32
  unsigned int pid = static_cast<unsigned int>(GetCurrentProcessId());
#else
  unsigned int pid = static_cast<unsigned int>(getpid());
#endif
  const QString dateTimeString = startTime_.toString(DATETIME_STRING_FORMAT);
  const QString pidString = QString::number(pid);

  // Try to get a unique name.  This should only matter if we make the same file multiple times
  // in a short time span (e.g. during unit testing)
  return FILENAME_PATTERN.arg(prefix_, dateTimeString, pidString);
}

QString PersistentFileLogger::sanitizeFilename_(const QString& prefix) const
{
  // Method of sanitization borrowed from http://comments.gmane.org/gmane.comp.lib.qt.general/20276
  // Note that simCore::sanitizeFilename() was insufficient due to interpretation of illegal slash
  // characters.  For example, sanitizeFilename("path/to/file") would (and should) return "path/to/file",
  // but this function only lets you set the name, and returns "pathtofile".
  return QString(prefix).replace(ILLEGAL_REGEXP, "");
}

QString PersistentFileLogger::filename() const
{
  return filename_;
}

}
