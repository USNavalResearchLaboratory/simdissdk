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
#include <cassert>
#include <QDateTime>
#include <QColor>
#include <QTimer>
#include <QThread>
#include "simNotify/Notify.h"
#include "simCore/Time/Utils.h"
#include "simCore/Calc/Math.h"
#include "simQt/QtFormatting.h"
#include "simQt/ConsoleChannel.h"
#include "simQt/ConsoleDataModel.h"

namespace simQt {

/////////////////////////////////////////////////////////////////

/** Implementation of Channel interface that adds the text to the data model */
class ConsoleDataModel::ChannelImpl : public ConsoleChannel
{
public:
  /** Constructor */
  ChannelImpl(ConsoleDataModel* dataModel, const QString& name)
    : dataModel_(dataModel),
      name_(name)
  {
  }

  virtual ~ChannelImpl()
  {
  }

  /**
   * Call this to notify all observers of your new text string.  When adding a new channel
   * to the console dialog, you should call this method to notify the dialog of text.
   * Note that this method is thread-safe.  Threaded calls to addText() will trigger
   * a queued connection to the data model's addEntry() method.
   * @param severity Severity level with which to push out text
   * @param text Text string to push out to all observers; should be completely formed (buffered) message
   */
  virtual void addText(simNotify::NotifySeverity severity, const QString& text)
  {
    if (dataModel_ == NULL)
      return;
    // Directly call addEntry to avoid a look-up, for performance reasons in the common case.
    if (QThread::currentThread() == dataModel_->thread())
    {
      dataModel_->addEntry(severity, name(), text);
      return;
    }
    // Go ahead and send a queued message
    QMetaObject::invokeMethod(dataModel_, "addEntry",
      Qt::QueuedConnection,
      Q_ARG(simNotify::NotifySeverity, severity),
      Q_ARG(QString, name()),
      Q_ARG(QString, text)
      );
  }

  /** Returns the name of the channel */
  virtual const QString& name() const
  {
    return name_;
  }

  /** Changes the console data model */
  void setConsoleDataModel(ConsoleDataModel* dataModel)
  {
    dataModel_ = dataModel;
  }

private:
  ConsoleDataModel* dataModel_;
  QString name_;
};

/////////////////////////////////////////////////////////////////

const QString ConsoleDataModel::DEFAULT_TIME_FORMAT = "M/d/yy h:mm:ss.zzz";
static const int DEFAULT_MAX_LINES_SIZE = 1000;
static const int PROCESS_PENDING_TIMEOUT = 250; // milliseconds between processing of pending data

ConsoleDataModel::ConsoleDataModel(QObject* parent)
  : QAbstractItemModel(parent),
    newestOnTop_(false),
    colorizeText_(true),
    numLines_(DEFAULT_MAX_LINES_SIZE),
    spamFilterTimeout_(5.0),
    minSeverity_(simNotify::NOTIFY_INFO),
    timeFormatString_(DEFAULT_TIME_FORMAT),
    pendingTimer_(new QTimer)
{

  pendingTimer_->setInterval(PROCESS_PENDING_TIMEOUT);
  pendingTimer_->setSingleShot(true);
  connect(pendingTimer_, SIGNAL(timeout()), this, SLOT(processPendingAdds_()));
}

ConsoleDataModel::~ConsoleDataModel()
{
  delete pendingTimer_;
  qDeleteAll(lines_);
  qDeleteAll(pendingLines_);
  Q_FOREACH(ConsoleChannelPtr ptr, channels_.values())
  {
    ChannelImpl* impl = dynamic_cast<ChannelImpl*>(ptr.get());
    // Clear out the pointer to "this", in case it survives beyond us
    impl->setConsoleDataModel(NULL);
  }
}

QString ConsoleDataModel::dateTimeString(double timeSince1970)
{
  const QString UTC_POSTFIX = " UTC";

  // Get the timestamp and convert it to UTC
  QDateTime date = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(timeSince1970 * 1000)).toUTC();
  return date.toString(DEFAULT_TIME_FORMAT) + UTC_POSTFIX;
}

QVariant ConsoleDataModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid() || idx.parent().isValid())
    return QVariant();
  LineEntry* line = static_cast<LineEntry*>(idx.internalPointer());
  assert(line);
  if (line == NULL)
    return QVariant();

  switch (role)
  {
  case Qt::DisplayRole:
    switch (idx.column())
    {
    case COLUMN_TIME:
    {
      QDateTime date = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(line->timeStamp() * 1000)).toUTC();
      return date.toString(timeFormatString_);
    }
    case COLUMN_SEVERITY:
      return QString::fromStdString(simNotify::severityToString(line->severity()));
    case COLUMN_CATEGORY:
      return line->channel();
    case COLUMN_TEXT:
      return line->text();
    }
    break;

  case ConsoleDataModel::SEVERITY_ROLE:
    return line->severity();

  case Qt::ForegroundRole:
    // Colorization is optional
    if (!colorizeText_)
      break;
    return colorForSeverity_(line->severity());
  }
  return QVariant();
}

Qt::ItemFlags ConsoleDataModel::flags(const QModelIndex& index) const
{
  if (index.isValid())
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  return Qt::NoItemFlags;
}

QVariant ConsoleDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
    case COLUMN_TIME:
      return "Time";
    case COLUMN_SEVERITY:
      return "Severity";
    case COLUMN_CATEGORY:
      return "Category";
    case COLUMN_TEXT:
      return "Text";
    }
  }

  // Set the tooltip of the header
  if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
  {
    switch (section)
    {
    case COLUMN_TIME:
      return simQt::formatTooltip(tr("Time"),
        tr("Time column is in Coordinated Universal Time (UTC)."));
    case COLUMN_SEVERITY:
      return simQt::formatTooltip(tr("Severity"),
        tr("Displays the severity of the console log entries."));
    case COLUMN_CATEGORY:
      return simQt::formatTooltip(tr("Category"),
        tr("Displays the category of the console log entries."));
    case COLUMN_TEXT:
      return simQt::formatTooltip(tr("Text"),
        tr("Displays the details of the console log entries."));
    }
  }

  return QAbstractItemModel::headerData(section, orientation, role);
}

int ConsoleDataModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return COLUMN_MAX;
}

int ConsoleDataModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return lines_.count();
}

QModelIndex ConsoleDataModel::parent(const QModelIndex &child) const
{
  return QModelIndex();
}

QModelIndex ConsoleDataModel::index(int row, int column, const QModelIndex &parent) const
{
  // Error check validity
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  // Pull out list entry
  int indexInLines = row;
  if (newestOnTop()) // Reverse it if newest is on top
    indexInLines = lines_.size() - row - 1;
  LineEntry* entry = lines_[indexInLines];
  return createIndex(row, column, entry);
}

ConsoleChannelPtr ConsoleDataModel::registerChannel(const QString& name)
{
  QMap<QString, ConsoleChannelPtr>::iterator i = channels_.find(name);
  if (i == channels_.end())
  {
    ConsoleChannelPtr ptr(new ChannelImpl(this, name));
    channels_[name] = ptr;
    return ptr;
  }
  return i.value();
}

void ConsoleDataModel::clear()
{
  if (lines_.count() <= 0)
    return;

  beginRemoveRows(QModelIndex(), 0, lines_.count() - 1);
  Q_FOREACH(LineEntry* line, lines_)
    delete line;
  lines_.clear();
  endRemoveRows();
}

void ConsoleDataModel::addEntry(simNotify::NotifySeverity severity, const QString& channel, const QString& text)
{
  // One message per line, omit empty lines
  Q_FOREACH(QString str, text.split("\n"))
  {
    // Remove instances of carriage return before adding text
    str.remove('\r');
    if (!str.isEmpty())
      addPlainEntry_(severity, channel, str);
  }
}

bool ConsoleDataModel::isDuplicateEntry_(const QString& channel, const QString& text, double sinceTime) const
{
  return isDuplicateEntry_(pendingLines_, channel, text, sinceTime) ||
    isDuplicateEntry_(lines_, channel, text, sinceTime);
}

bool ConsoleDataModel::isDuplicateEntry_(const QList<LineEntry*>& whichList, const QString& channel,
  const QString& text, double sinceTime) const
{
  QListIterator<LineEntry*> iter(whichList);
  iter.toBack();
  // Iterate from the back to get proper time sorting
  while (iter.hasPrevious())
  {
    const LineEntry* line = iter.previous();
    // Break out if we hit the since-time
    if (line->timeStamp() < sinceTime)
      return false;
    // Break out if we have a match
    if (line->text() == text && line->channel() == channel)
      return true;
  }
  return false;
}

void ConsoleDataModel::addPlainEntry_(simNotify::NotifySeverity severity, const QString& channel, const QString& text)
{
  // Don't add duplicates
  const double currentTime = LineEntry::currentTime();
  if (spamFilterTimeout() > 0 && isDuplicateEntry_(channel, text, currentTime - spamFilterTimeout()))
    return;

  // Process the entry through filters (if filters are defined)
  LineEntry* newEntry = NULL;
  if (!entryFilters_.empty())
  {
    // Put into a struct for processing
    ConsoleEntry consoleEntry;
    consoleEntry.time = currentTime;
    consoleEntry.severity = severity;
    consoleEntry.channel = channel;
    consoleEntry.text = text;

    // Pass through the filters
    Q_FOREACH(EntryFilterPtr entryFilter, entryFilters_)
    {
      // If any filter rejects text, return early
      if (!entryFilter->acceptEntry(consoleEntry))
        return;
    }

    // Allocate the line entry based on modified values
    newEntry = new LineEntry(consoleEntry.severity, consoleEntry.channel, consoleEntry.text);
  }
  else
  {
    // Allocate the line entry based on non-modified values
    newEntry = new LineEntry(severity, channel, text);
  }

  // Save in the pending list, only add items that meet the minimum severity level
  if (severity <= minSeverity_)
    pendingLines_.push_back(newEntry);

  // Notify users of new data -- this should be instant, even if we are just pending
  // NOTE that this signal is emitted no matter what the severity level is, unlike items in the pendingLines_
  emit(textAdded(newEntry->severity()));
  emit(textAdded(newEntry->timeStamp(), newEntry->severity(), newEntry->channel(), newEntry->text()));
  if (severity <= minSeverity_ && !pendingTimer_->isActive())
    pendingTimer_->start();

  // If not pushed onto pending, it needs to be deleted
  if (severity > minSeverity_)
    delete newEntry;
}

void ConsoleDataModel::processPendingAdds_()
{
  if (pendingLines_.empty())
    return;

  // Add the new lines: Pay attention to newest on top flag, which impacts whether
  // people watching us see these at the beginning (true), or end (false)
  // Note that indices are inclusive, so a size of 1 means an offset of 0 (hence the -1)
  if (newestOnTop())
    beginInsertRows(QModelIndex(), 0, pendingLines_.size() - 1);
  else
    beginInsertRows(QModelIndex(), lines_.count(), lines_.count() + pendingLines_.size() - 1);
  QListIterator<LineEntry*> iter(pendingLines_);
  // Iterate from the front to get proper time sorting
  while (iter.hasNext())
    lines_.push_back(iter.next());
  pendingLines_.clear();
  endInsertRows();

  // Limit the internal number of lines saved
  limitData_();
}

int ConsoleDataModel::numLines() const
{
  return numLines_;
}

void ConsoleDataModel::setMinimumSeverity(int severity)
{
  simNotify::NotifySeverity newSeverity = static_cast<simNotify::NotifySeverity>(severity);
  if (minSeverity_ == newSeverity)
    return;

  // if we are changing to a lower severity level, clear out all lines that exceed our minimum severity
  if (newSeverity < minSeverity_)
  {
    int removeEndIndex = 0;
    int lineIndex = 0;
    // iterate through the lines_ list to check and remove lines
    while (lineIndex < lines_.size())
    {
      if (lineIndex + removeEndIndex < lines_.size() &&
        lines_.at(lineIndex + removeEndIndex)->severity() > newSeverity)
      {
        // found an invalid severity level, update the index of our removal end block
        removeEndIndex++;
      }
      // if we have any lines to remove, do it now, since we've reached the end of a block of invalid lines
      else
      {
        if (removeEndIndex > 0)
        {
          beginRemoveRows(QModelIndex(), lineIndex, lineIndex + removeEndIndex - 1);
          for (; removeEndIndex > 0; removeEndIndex--)
          {
            delete lines_.at(lineIndex);
            lines_.removeAt(lineIndex);
          }
          endRemoveRows();
        }
        lineIndex++;
      }
    }

    // remove messages with invalid severity from the pendinglines_ list
    lineIndex = 0;
    while (lineIndex < pendingLines_.size())
    {
      if (pendingLines_.at(lineIndex)->severity() > newSeverity)
      {
        delete pendingLines_.at(lineIndex);
        pendingLines_.removeAt(lineIndex);
      }
      else
        lineIndex++;
    }
  }

  minSeverity_ = newSeverity;
}

void ConsoleDataModel::setNumLines(int numLines)
{
  if (numLines != numLines_ && numLines > 0)
  {
    numLines_ = numLines;
    lines_.reserve(numLines_);
    limitData_();
  }
}

double ConsoleDataModel::spamFilterTimeout() const
{
  return spamFilterTimeout_;
}

void ConsoleDataModel::setSpamFilterTimeout(double seconds)
{
  spamFilterTimeout_ = simCore::sdkMax(0.0, seconds);
}

void ConsoleDataModel::limitData_()
{
  int linesLimit = simCore::sdkMax(1, numLines());
  if (lines_.size() > linesLimit)
  {
    // and the pop_front below isn't going to be sufficient to limit data.
    int numToRemove = lines_.size() - linesLimit;

    // Remove as many rows as needed to get under the numLines(); line removal location
    // is based on what observers see, so if newest is on top (true), remove from bottom
    if (newestOnTop())
      beginRemoveRows(QModelIndex(), lines_.size() - numToRemove - 1, lines_.size() - 1);
    else
      beginRemoveRows(QModelIndex(), 0, numToRemove - 1);
    QList<LineEntry*>::iterator endIter = lines_.begin() + numToRemove;
    for (QList<LineEntry*>::iterator iter = lines_.begin(); iter != endIter; ++iter)
    {
      delete *iter;
    }
    lines_.erase(lines_.begin(), endIter);
    endRemoveRows();

    // Make sure the math is right for arguments to lines_.erase()
    assert(lines_.size() == linesLimit);
  }
}

QVariant ConsoleDataModel::colorForSeverity_(simNotify::NotifySeverity severity) const
{
  switch (severity)
  {
  case simNotify::NOTIFY_FATAL: // Bright red fatal
    return QColor::fromRgb(255, 0, 0);
  case simNotify::NOTIFY_ERROR:  // Red errors
    return QColor::fromRgb(128, 0, 0);
  case simNotify::NOTIFY_WARN:  // Yellow warnings
    return QColor::fromRgb(64, 64, 0);
  case simNotify::NOTIFY_DEBUG_FP: // Gray-ish so it can be easily ignored
    return QColor::fromRgb(128, 128, 128);
  default: // Everything else is default (likely black)
    break;
  }
  return QVariant();
}

bool ConsoleDataModel::colorizeText() const
{
  return colorizeText_;
}

void ConsoleDataModel::setColorizeText(bool fl)
{
  if (fl == colorizeText())
    return;
  beginResetModel();
  colorizeText_ = fl;
  endResetModel();
  emit(colorizeTextChanged(fl));
}

void ConsoleDataModel::addEntryFilter(EntryFilterPtr entryFilter)
{
  entryFilters_.append(entryFilter);
}

void ConsoleDataModel::removeEntryFilter(EntryFilterPtr entryFilter)
{
  entryFilters_.removeOne(entryFilter);
}

bool ConsoleDataModel::newestOnTop() const
{
  return newestOnTop_;
}

void ConsoleDataModel::setNewestOnTop(bool fl)
{
  if (newestOnTop() != fl)
  {
    beginResetModel();
    newestOnTop_ = fl;
    endResetModel();
    emit newestOnTopChanged(fl);
  }
}

void ConsoleDataModel::setTimeFormatString(const QString& formatString)
{
  const QString realString = (formatString.isEmpty() ? DEFAULT_TIME_FORMAT : formatString);
  // Check for no-op
  if (realString == timeFormatString_)
    return;
  // Emit that the data has changed for the time column
  timeFormatString_ = formatString;
  // Return early if we have no data
  if (lines_.empty())
    return;
  emit dataChanged(index(0, COLUMN_TIME, QModelIndex()), index(lines_.size() - 1, COLUMN_TIME, QModelIndex()));
}

////////////////////////////////////////

ConsoleDataModel::LineEntry::LineEntry(simNotify::NotifySeverity severity, const QString& channel, const QString& text)
  : time_(ConsoleDataModel::LineEntry::currentTime()),
    severity_(severity),
    channel_(channel),
    text_(text)
{
}

double ConsoleDataModel::LineEntry::currentTime()
{
  return simCore::getSystemTime();
}

double ConsoleDataModel::LineEntry::timeStamp() const
{
  return time_;
}

simNotify::NotifySeverity ConsoleDataModel::LineEntry::severity() const
{
  return severity_;
}

QString ConsoleDataModel::LineEntry::channel() const
{
  return channel_;
}

QString ConsoleDataModel::LineEntry::text() const
{
  return text_;
}

/////////////////////////////////////////////////////////////////

SimpleConsoleTextFilter::SimpleConsoleTextFilter()
#ifndef NDEBUG
  : showInDebugMode_(true)
#endif
{
}

void SimpleConsoleTextFilter::addFilter(const QString& filter)
{
  filters_.push_back(filter);
}

void SimpleConsoleTextFilter::setShowInDebugMode(bool showInDebug)
{
#ifndef NDEBUG
  showInDebugMode_ = showInDebug;
#endif
}

void SimpleConsoleTextFilter::addCommonQtPngFilters()
{
  // Matches error messages from SIM-4260, like:
  // QOpenGLContext::swapBuffers() called with non-exposed window, behavior is undefined
  filters_.push_back("swapBuffers() called with non-exposed window, behavior is undefined");

  // Matches error messages from SIM-4433, like:
  // QWindowsWindow::setGeometryDp: Attempt to set a size (283x177) violating the constraints(283x295 - 524287x524287) on window QWidgetWindow/'Super FormWindow'
  filters_.push_back("QWindowsWindow::setGeometryDp: Attempt to set a size (");

  // Matches error messages from Intel 4600 on start-up from SIM-4703, like:
  // Warning: detected OpenGL error 'invalid enumerant' at After Renderer::compile
  // Warning: detected OpenGL error 'invalid enumerant' at after RenderBin::draw(..)
  filters_.push_back("Warning: detected OpenGL error 'invalid enumerant' at ");

  // Matches error messages from MSVC 2015 with Qt 5.5 which uses PNG 1.6, like:
  // "libpng warning: iCCP: known incorrect sRGB profile"
  filters_.push_back("libpng warning: iCCP: known incorrect sRGB profile");

  // Matches PNG 1.6 from GDAL:
  // "PNG lib warning : Interlace handling should be turned on when using png_read_image"
  filters_.push_back("Interlace handling should be turned on when using png_read_image");

  // Matches error messages from Qt about untested version of Windows:
  // "libpng warning: iCCP: known incorrect sRGB profile"
#ifdef WIN32
  filters_.push_back("Qt: Untested Windows version ");
#endif

  // Errors displayed in Red Hat at start up
#ifndef WIN32
  filters_.push_back("QXcbConnection: XCB error: 8 (BadMatch),");
#endif
}

void SimpleConsoleTextFilter::addCommonOsgEarthFilters()
{
  // osgEarth warnings from MGRS grid that we can't do anything about, like:
  // "[osgEarth]* [MGRSGraticule] Empty SQID geom at 10W DE"
  filters_.push_back("[osgEarth]* [MGRSGraticule] Empty SQID geom at ");
  // "[osgEarth]  SQID100kmCell SW=6.30464349477,0 NE=7.20284692297,0.904282609865, SRS=WGS 84"
  filters_.push_back("[osgEarth]  SQID100kmCell SW=");
}

bool SimpleConsoleTextFilter::acceptEntry(ConsoleDataModel::ConsoleEntry& entry) const
{
  // Hide several messages in release mode.  If debug mode, let them through with different priority
  for (std::vector<QString>::const_iterator i = filters_.begin(); i != filters_.end(); ++i)
  {
    // String matching, case-sensitive
    if (entry.text.contains((*i)))
    {
#ifndef NDEBUG
      // Drop message, if showInDebugMode_ is off
      if (!showInDebugMode_)
        return false;
      // In debug mode, lower severity and change the channel
      entry.severity = simNotify::NOTIFY_DEBUG_INFO;
      entry.channel = "Ignored Errors";
#else
      // In release mode, drop them entirely
      return false;
#endif
    }
  }
  return true;
}

}
