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
#ifndef SIMQT_TIME_WIDGET_H
#define SIMQT_TIME_WIDGET_H

#include <QWidget>
#include "simCore/Common/Export.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Constants.h"

class QMenu;
class QLabel;

namespace simQt {

// Helper class to hold all the info for a time format
class TimeFormatContainer;

/** Widget for displaying the Time Edit Widget with user selectable time formats */
class SDKQT_EXPORT TimeWidget : public QWidget
{
  Q_OBJECT;

  /** Sets/gets the label in Qt Designer */
  Q_PROPERTY(QString Label READ label WRITE setLabel)
  /** Sets/gets the label tool tip in Qt Designer */
  Q_PROPERTY(QString LabelToolTip READ labelToolTip WRITE setLabelToolTip)
  /** Sets/gets the flag for color coding text in Qt Designer */
  Q_PROPERTY(bool ColorCodeText READ colorCodeText WRITE setColorCodeText)

public:
  /** constructor */
  TimeWidget(QWidget* parent = 0);
  virtual ~TimeWidget();

  /** Get label to the left of the spin box */
  QString label() const;
  /** Set label to the left of the spin box */
  void setLabel(QString value);

  /** Get the tool tip for the label */
  QString labelToolTip() const;
  /** Set the tool tip for the label */
  void setLabelToolTip(QString value);

  /** get the status of the "change font color on error" setting */
  bool colorCodeText() const;
  /** set the "change font color on error" setting */
  void setColorCodeText(bool value);

  /** Get the current time */
  simCore::TimeStamp timeStamp() const;
  /** Set the current time */
  void setTimeStamp(const simCore::TimeStamp& value);

  /** Sets the time range
   * @param[in] scenarioReferenceYear The reference year of the scenario
   * @param[in] start The start of the time range
   * @param[in] end The end of the time range
   */
  void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);

  /** Retrieves the previously set scenario reference year (from setTimeRange()). */
  int scenarioReferenceYear() const;
  /** Retrieves the previously set start of the time range */
  simCore::TimeStamp timeRangeStart() const;
  /** Retrieves the previously set end of the time range */
  simCore::TimeStamp timeRangeEnd() const;

  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  /** Get the time format */
  simCore::TimeFormat timeFormat() const;

  /** Returns the number of digits after the decimal point */
  unsigned int precision() const;

public slots:
  /** Set the time format */
  void setTimeFormat(simCore::TimeFormat newFormat);
  /** Set the number of digits after the decimal point */
  void setPrecision(unsigned int digits);

signals:
  /** emitted when the time changes via the user */
  void timeChanged(const simCore::TimeStamp& time);
  /** emitted when the time is changed by the user or by setTimeStamp */
  void timeModified(const simCore::TimeStamp& time);
  /** emitted when the time range changes */
  void timeRangeChanged();

protected slots:
  /** User wants to see the right mouse click menu */
  void showRightMouseClickMenu_(const QPoint &pos);

  /** forwards to timeChanged signal */
  void timeWasChanged_(const simCore::TimeStamp& time);

  // Callbacks to switch the time format
  void setSeconds_(); ///< set time format to seconds
  void setMinutes_(); ///< set time format to minutes
  void setHours_(); ///< set time format to hours
  void setOrdinal_(); ///< set time format to ordinal
  void setMonth_(); ///< set time format to month
  void setColorCode_(); ///< toggle the color code setting

private:
  /**
   * Helper function for adding time formats
   * @param[in] container The container to add
   * @param[in] slot The slot description for the right click mouse menu
   */
  void addContainer_(TimeFormatContainer* container, const QString& slot);

  /// the text to the right of the custom spin box
  QLabel* title_;
  /// A list of all the time formats
  QList<TimeFormatContainer*> containers_;
  /// The current time format
  TimeFormatContainer* currentContainer_;
  /// The right mouse menu
  QMenu* rightMouseClickMenu_;
  /// Allows the user to toggle the color coding
  QAction* colorCodeAction_;

  /// Cache of the last scenario reference year
  int scenarioReferenceYear_;
  /// Cache of the start of the time range
  simCore::TimeStamp timeRangeStart_;
  /// Cache of the end of the time range
  simCore::TimeStamp timeRangeEnd_;
};

} // namespace

#endif /* SIMQT_TIME_WIDGET_H */

