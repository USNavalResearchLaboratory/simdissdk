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
#include <QDir>
#include "simNotify/Notify.h"
#include "simQt/PersistentFileLogger.h"
#include "simQt/ConsoleDataModel.h"
#include "simQt/ConsoleLogger.h"

namespace simQt
{

/** Defines the template for the output strings */
static const QString OUTPUT_FORMAT = "[%1]\t[%2]\t[%3]\t%4\n";


RemovableFiles::RemovableFiles(bool enableMaxSeconds, int maxSeconds, bool enableMaxSpace, qint64 maxSpace, bool enableMaxNumber, int maxNumber)
  : DetermineRemovable(),
    enableMaxSeconds_(enableMaxSeconds),
    maxSeconds_(maxSeconds),
    enableMaxSpace_(enableMaxSpace),
    maxSpace_(maxSpace),
    enableMaxNumber_(enableMaxNumber),
    maxNumber_(maxNumber)
{
}

RemovableFiles::~RemovableFiles()
{
}

namespace {
  /// Used to time sort the files
  bool compareByTime(const QFileInfo& a, const QFileInfo& b)
  {
    return (a.lastModified() < b.lastModified());
  }
}

void RemovableFiles::calculate(const QDir& files, QStringList& removableFiles) const
{
  QDateTime startTime(QDateTime::currentDateTime());
  QList<QFileInfo> pending;
  qint64 totalFileSize = 0;
  int totalFiles = 0;

  auto infoList = files.entryInfoList(QDir::Files);
  for (auto it = infoList.begin(); it != infoList.end(); ++it)
  {
    QFileInfo info = *it;
    ++totalFiles;
    int timeDelta = info.lastModified().secsTo(startTime);
    if (enableMaxSeconds_ && (timeDelta >= maxSeconds_))
      removableFiles.push_back(info.fileName());  // Files too old can be immediately removed
    else if (enableMaxSpace_ || enableMaxNumber_)
    {
      pending.push_back(info);   // Can only remove after sorting by time
      totalFileSize += info.size();
    }
  }

  // Sort by time
  std::stable_sort(pending.begin(), pending.end(), compareByTime);

  // Now delete from oldest
  for (auto it = pending.begin(); it != pending.end(); ++it)
  {
    QFileInfo info = *it;
    QString name = info.fileName();
    if (enableMaxSpace_ && (totalFileSize > maxSpace_))
    {
      removableFiles.push_back(name);
      totalFileSize -= info.size();
    }
    else if (enableMaxNumber_ && ((totalFiles - removableFiles.size()) > maxNumber_))
    {
      removableFiles.push_back(name);
    }
  }
}


//------------------------------------------------------------------------------------------------------------------------------

ConsoleLogger::ConsoleLogger(const QString& filePrefix, QObject* parent)
  : QObject(parent),
    fileLogger_(new simQt::PersistentFileLogger(filePrefix))
{
  if (fileLogger_->open() == 0)
  {
    SIM_INFO << "Opened console log file: " << QDir::toNativeSeparators(fileLogger_->filename()).toStdString() << "\n";
  }
  else
  {
    // Print an error message to the end user
    QString filename = fileLogger_->filename();
    if (filename.isEmpty())
      filename = tr("in [%1]").arg(fileLogger_->filePath());
    else
      filename = QDir::toNativeSeparators(filename);
    SIM_WARN << "Unable to open console log file " << filename.toStdString() << "\n";
  }
}

ConsoleLogger::~ConsoleLogger()
{
  delete fileLogger_;
  fileLogger_ = NULL;
}

int ConsoleLogger::clean(const DetermineRemovable& removable) const
{
  return fileLogger_->clean(removable);
}

void ConsoleLogger::bindTo(ConsoleDataModel* dataModel)
{
  connect(dataModel, SIGNAL(textAdded(double, simNotify::NotifySeverity, const QString&, const QString&)),
    this, SLOT(addEntry(double, simNotify::NotifySeverity, const QString&, const QString&)));
}

void ConsoleLogger::addEntry(double timeStamp, simNotify::NotifySeverity severity, const QString& channel, const QString& text)
{
  fileLogger_->addText(OUTPUT_FORMAT.arg(ConsoleDataModel::dateTimeString(timeStamp),
    QString::fromStdString(simNotify::severityToString(severity)), channel, text));
}

}
