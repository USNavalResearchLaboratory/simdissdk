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
#ifndef SIMQT_PERSISTENTFILELOGGER_H
#define SIMQT_PERSISTENTFILELOGGER_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include "simCore/Common/Export.h"

class QFile;
class QTextStream;
class QDir;

namespace simQt {

/// Interface for determining which files to remove
class DetermineRemovable
{
public:
  virtual ~DetermineRemovable() {}
  ///All possible file are given by files and files to remove are returned by removableFiles
  virtual void calculate(const QDir& files, QStringList& removableFiles) const = 0;
};

/**
 * Provides a mechanism for creating persistent log files.  Log files are kept persistently
 * on disk in the user's home directory (~/.config/ORGANIZATION on Linux, %LOCALAPPDATA%/ORGANIZATION
 * on Windows).  The subdirectory for log files is the QCoreApplication::organizationName().
 *
 * Logs are time stamped, and include the process ID for uniqueness.  In some cases, this
 * is not enough for uniqueness so a .1, .2, .3, etc. might be appended to the filename.
 *
 * Log files are cleaned up automatically based on a maximum file age, relative to the time
 * of the class instantiation.  The default time period for clean-up is 2 weeks.  Clean up
 * will only occur for logs matching the same prefix (set at construction time).  Files
 * are cleaned up on a call to clean().
 *
 * Multiple loggers can exist in the same process, with either the same or different
 * prefixes.  This means the class can be easily reused for different contexts (e.g. an
 * application log, an error log, and an audit log).
 */
class SDKQT_EXPORT PersistentFileLogger : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  PersistentFileLogger(const QString& prefix, QObject* parent=nullptr);
  virtual ~PersistentFileLogger();

  /** Returns true if the file is open */
  bool isOpen() const;
  /** Returns the filename for the log.  if !isOpen(), then this string is empty. */
  QString filename() const;
  /** File path used for the logs.  Only set after open() is called. */
  QString filePath() const;
  /** Remove files based on the determination of removable */
  int clean(const DetermineRemovable& removable);

public slots:
  /**
   * Opens the log file if it was not already open, returning 0 on success and non-zero
   * on error.  If the file was already open, 0 is returned.  Log file will be in the
   * application's data directory based on (~/.config/ORGANIZATION on Linux,
   * %APPDATA%/ORGANIZATION on Windows).  The subdirectory for log files is the
   * QCoreApplication::organizationName().
   */
  int open();
  /**
   * Appends text to the log file.  Raw output, no trimming.  Returns 0 on success.  Will
   * NOT open the file if it not already open.
   */
  int addText(const QString& text);

private:
  /** Returns the QString-based path for the logs file output.  Has side effect of mkpath()'ing the logs directory if needed. */
  QString createFilePath_() const;
  /** Makes the name for the file (not full filename) for current process/time (uses expectedFileName_()) */
  QString makeFileName_(const QDir& filePath) const;
  /** Returns the possibly-not-unique expected filename (before makeFileName_()) */
  QString expectedFileName_() const;
  /** Sanitizes user input for filename */
  QString sanitizeFilename_(const QString& filename) const;

  /** Application-specific prefix, for the purpose of forming filename */
  const QString prefix_;
  /** Time at which this class was instantiated, for purpose of forming filename */
  const QDateTime startTime_;

  /** File for the log */
  QFile* file_;
  /** Streams output to the log file */
  QTextStream* stream_;
  /** Avoid opening the log file more than once, even in error conditions */
  bool openAttempted_;

  /** Actual filename written (full path).  Empty until file is opened. */
  QString filename_;
  /** Path used for the logs.  Set only after first attempted open. */
  QString filePath_;
};

}

#endif /* SIMQT_PERSISTENTFILELOGGER_H */
