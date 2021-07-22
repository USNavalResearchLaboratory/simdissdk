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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_CONSOLELOGGER_H
#define SIMQT_CONSOLELOGGER_H

#include <QObject>
#include "simCore/Common/Export.h"
#include "simNotify/NotifySeverity.h"
#include "simQt/ConsoleDataModel.h"
#include "simQt/PersistentFileLogger.h"

class QString;
class QTemporaryFile;
class QTextStream;

namespace simQt
{

struct ConsoleEntry;

/** Class to identify files to removed potentially based on age, size and number */
class SDKQT_EXPORT RemovableFiles : public DetermineRemovable
{
public:
  /**
   * Constructor
   * @param enableMaxSeconds If true files are limited by age
   * @param maxSeconds Max age in seconds
   * @param enableMaxSpace If true files are limited by disk space usage
   * @param maxSpace Max disk space usage in bytes
   * @param enableMaxNumber If true files are limited by number
   * @param maxNumber Max number of files
   */
  RemovableFiles(bool enableMaxSeconds = true, int maxSeconds = 1209600, bool enableMaxSpace = false, qint64 maxSpace = 1048576, bool enableMaxNumber = false, int maxNumber = 10);
  virtual ~RemovableFiles();

  /**
  * Determines what files in "files" should be deleted and places them in "removableFiles"
  * @param files Possible files to delete
  * @param removableFiles Files to remove; maybe empty
  */
  virtual void calculate(const QDir& files, QStringList& removableFiles) const;

private:
  bool enableMaxSeconds_;
  int maxSeconds_;
  bool enableMaxSpace_;
  qint64 maxSpace_;
  bool enableMaxNumber_;
  int maxNumber_;
};

/** Responsible for logging console output to a file.  On graceful destruction, the temporary file will be deleted. */
class SDKQT_EXPORT ConsoleLogger : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  ConsoleLogger(const QString& filePrefix="SIMDIS_SDK", QObject* parent=nullptr);
  virtual ~ConsoleLogger();

  /**
   * Binds a console data model and logger together.  A single logger can be bound to multiple
   * data models, if your application requires multiple data models.
   */
  void bindTo(ConsoleDataModel* dataModel);

  /** Remove files based on the determination of removable */
  int clean(const DetermineRemovable& removable) const;

public slots:
  /** Handle single new console entry */
  void addEntry(double timeStamp, simNotify::NotifySeverity severity, const QString& channel, const QString& text);

private:
  simQt::PersistentFileLogger* fileLogger_;
};

}

#endif /* SIMQT_CONSOLELOGGER_H */
