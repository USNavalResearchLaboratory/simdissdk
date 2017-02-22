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
#ifndef SIMQT_CONSOLEDATAMODEL_H
#define SIMQT_CONSOLEDATAMODEL_H

#include <QSortFilterProxyModel>
#include <QList>
#include <QMap>
#include <QMetaType>
#include "simNotify/NotifySeverity.h"
#include "simCore/Common/Export.h"
#include "simCore/Common/Memory.h"

class QTimer;

namespace simQt {

class ConsoleChannel;

/** Maintains a persistent database of console output. */
class SDKQT_EXPORT ConsoleDataModel : public QAbstractItemModel
{
  Q_OBJECT;
public:

  /** Struct to manage passing the Console entry data easily */
  struct ConsoleEntry
  {
    double time;                        ///< time of console entry
    simNotify::NotifySeverity severity; ///< severity level of console entry
    QString channel;                    ///< channel of console entry
    QString text;                       ///< text of the console entry
  };

  /**
   * Defines a filter that can be applied to drop data before it enters
   * the console data model.  Only applies to new entries.
   */
  class EntryFilter
  {
  public:
    virtual ~EntryFilter() {}
    /**
     * Return false to reject the ConsoleEntry so that it does not enter the data
     * model.  Note that entry values can be edited.  Only called for new entries.
     * @param entry Candidate entry for the console data model, that has not yet been
     *   added to the data model.  You may edit the fields in the entry if desired,
     *   but note that the remainder of the filter chain will use your edited entry
     *   and not the original entry.
     * @return Return true if your filter permits the entry in the data model, or
     *   return false to drop the entry and reject it from adding to the data model.
     */
    virtual bool acceptEntry(ConsoleEntry& entry) const = 0;
  };
  /** Typedef a smart pointer onto EntryFilter class. */
  typedef std::tr1::shared_ptr<EntryFilter> EntryFilterPtr;

  /** Severity of the row, in conjunction with data() (regardless of column) */
  static const int SEVERITY_ROLE = Qt::UserRole + 1;

  /**
  * Define the data in each column of the model.
  * example of usage: ourModel_->data(index(row, ConsoleDataModel::COLUMN_TEXT, parentIndex), Qt::DisplayRole);
  */
  enum ColumnOrder
  {
    COLUMN_TIME = 0,
    COLUMN_SEVERITY,
    COLUMN_CATEGORY,
    COLUMN_TEXT,
    COLUMN_MAX
  };

  /** Constructor */
  ConsoleDataModel(QObject* parent=NULL);
  virtual ~ConsoleDataModel();

  /** From QAbstractItemModel::data */
  virtual QVariant data(const QModelIndex& idx, int role) const;
  /** From QAbstractItemModel::headerData */
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  /** From QAbstractItemModel::flags */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  /** From QAbstractItemModel::columnCount */
  virtual int columnCount(const QModelIndex &parent) const;
  /** From QAbstractItemModel::rowCount */
  virtual int rowCount(const QModelIndex &parent) const;
  /** From QAbstractItemModel::parent */
  virtual QModelIndex parent(const QModelIndex &child) const;
  /** From QAbstractItemModel::index */
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;

  /** Define a shared pointer to a console channel */
  typedef std::tr1::shared_ptr<ConsoleChannel> ConsoleChannelPtr;
  /** Registers a channel with the data model, returning a handle to add text */
  ConsoleChannelPtr registerChannel(const QString& name);

  /** Returns true when text should be colorized */
  bool colorizeText() const;
  /** Returns the maximum length of the in-memory console, including all logged values */
  int numLines() const;
  /** Returns the number of seconds of spam filtering to prevent duplicate messages on the console; 0 for none. */
  double spamFilterTimeout() const;
  /** If true, newest entries are at the top of the model; else they're at bottom. */
  bool newestOnTop() const;

  /** Adds a new entry filter that can reject or edit console entries. */
  void addEntryFilter(EntryFilterPtr entryFilter);
  /** Removes an entry filter. */
  void removeEntryFilter(EntryFilterPtr entryFilter);

  /** Sets the current time format string.  See QDateTime::toString() documentation.  Relies on dateTimeString() (default setting) if not set. */
  void setTimeFormatString(const QString& formatString);

  /** Default date/time format string shows Month, Day, Year, Hour, Minutes, and seconds with millisecond precision. */
  static const QString DEFAULT_TIME_FORMAT;

  /** Helper function to get the time/date in the format that is displayed by default */
  static QString dateTimeString(double timeSince1970);

public slots:
  /** Appends a new entry to the data model and updates the attached views; performs processing on text data. */
  void addEntry(simNotify::NotifySeverity severity, const QString& channel, const QString& text);
  /** Clears out the data model */
  void clear();
  /** Turns on or off text colorization */
  void setColorizeText(bool fl);
  /** Set the minimum severity level for console messages. Pass severity as an int for ease of use with Qt signals */
  void setMinimumSeverity(int severity);
  /** Changes the number of lines saved in memory */
  void setNumLines(int numLines);
  /** Changes the seconds of history to check for spam filtering; use value <= 0 to disable feature.  Only applies to new entries. */
  void setSpamFilterTimeout(double seconds);
  /** If true, newest entries are at the top of the model; else they're at bottom. */
  void setNewestOnTop(bool fl);

signals:
  /** Emitted when the console gets a new line of text. This signal is not affected by the severity filter */
  void textAdded(simNotify::NotifySeverity severity);
  /** Emitted when the console gets a new line of text. This signal is not affected by the severity filter */
  void textAdded(double timeStamp, simNotify::NotifySeverity severity, const QString& channel, const QString& text);
  /** Emitted when colorization changes */
  void colorizeTextChanged(bool colorize);
  /** Emitted when newest-on-top changes */
  void newestOnTopChanged(bool newestOnTop);

private slots:
  /** New entries are kept in a pending list, to be batched up for processing all at once.  This processes the list */
  void processPendingAdds_();

private:
  /** Appends a new entry to the data model; text must be single line with no newlines */
  void addPlainEntry_(simNotify::NotifySeverity severity, const QString& channel, const QString& text);
  /** Returns an appropriate color, given a severity (QVariant() return is possible for default color) */
  QVariant colorForSeverity_(simNotify::NotifySeverity severity) const;
  /** Applies a data limit to the number of entries in memory based on numLines() */
  void limitData_();
  /** Returns true if there is a match to the channel/text, at or after the time supplied */
  bool isDuplicateEntry_(const QString& channel, const QString& text, double sinceTime) const;

  /** Immutable line entry class holds a single line of data */
  class LineEntry
  {
  public:
    LineEntry(simNotify::NotifySeverity severity, const QString& channel, const QString& text);
    static double currentTime();
    double timeStamp() const;
    simNotify::NotifySeverity severity() const;
    QString channel() const;
    QString text() const;

  private:
    double time_;
    simNotify::NotifySeverity severity_;
    QString channel_;
    QString text_;
  };

  /** Returns true if there is a match, but only searching a single QList<> */
  bool isDuplicateEntry_(const QList<LineEntry*>& whichList, const QString& channel, const QString& text, double sinceTime) const;

  class ChannelImpl;
  /// Map of channel name to channel pointer
  QMap<QString, ConsoleChannelPtr> channels_;
  /// Reverses order of the console text
  bool newestOnTop_;
  /// Turn on or off the colorization of text
  bool colorizeText_;
  /// Changes the number of lines to limit
  int numLines_;
  /// Seconds of history to search for spam reduction (0 to disable feature)
  double spamFilterTimeout_;
  /// Minimum severity level for messages to keep in the model
  simNotify::NotifySeverity minSeverity_;
  /// (Automatically) Sorted list of added lines
  QList<LineEntry*> lines_;
  /// (Automatically) Sorted list of lines ready to be added, but not yet put into the data model
  QList<LineEntry*> pendingLines_;

  /// Contains a list of all entry filters to apply before adding data
  QList<EntryFilterPtr> entryFilters_;

  /// Time formatting string
  QString timeFormatString_;

  /// Use a timer to process pending items
  QTimer* pendingTimer_;
};

#ifdef USE_DEPRECATED_SIMDISSDK_API
/** @deprecated Use ConsoleDataModel::setMinimumSeverity instead. Performs filtering based on severity */
class SDKQT_EXPORT SeverityFilterProxy : public QSortFilterProxyModel
{
  Q_OBJECT;
public:
  SeverityFilterProxy(QObject* parent);
public slots:
  void setMinimumSeverity(int severity);
protected:
  virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
private:
  simNotify::NotifySeverity minSeverity_;
};
#endif

}

Q_DECLARE_METATYPE(simNotify::NotifySeverity);

#endif /* SIMQT_CONSOLEDATAMODEL_H */
