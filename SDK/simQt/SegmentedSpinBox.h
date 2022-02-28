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
#ifndef SIMQT_SEGMENTED_SPIN_BOX_H
#define SIMQT_SEGMENTED_SPIN_BOX_H

#include <QSpinBox>
#include "simCore/Common/Export.h"
#include "simCore/Time/TimeClass.h"

class QTimer;

namespace simQt {

class SegmentedTexts;

/**
 * The SegmentedSpinBox class provides a flexible segmented spin box similar to QDateTimeEdit
 * This class handles the user interactions while the SegmentedTexts class handles actual text manipulations.
 */
class SDKQT_EXPORT SegmentedSpinBox : public QSpinBox
{
  Q_OBJECT;

public:
  /// constructor
  SegmentedSpinBox(QWidget* parent=nullptr);
  virtual ~SegmentedSpinBox();

  /// Get Time value
  virtual simCore::TimeStamp timeStamp() const;
  /// Set Time value
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// Set Begin/End Time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return status of the "change font color on error" setting
  virtual bool colorCode() const;
  /// set the "change font color on error" setting
  virtual void setColorCode(bool value);

  ///@return time to wait between user editing time and applying new timestamp
  int applyInterval();
  /// set time to wait between user editing time and applying new timestamp
  void setApplyInterval(int milliseconds);

  /// Set Segmented Line
  void setLine(SegmentedTexts* line);
  /// Get Segmented Line
  SegmentedTexts* line() const { return completeLine_; }

  /// The routine handles when the user presses tab or back tab
  virtual bool event(QEvent *e);
  /// The routine handles when the user clicks the up or down arrow
  virtual void stepBy(int steps);
  /// Necessary over-ride to always return a bogus value
  virtual int valueFromText(const QString &text) const;
  /// Necessary over-ride that returns the time string and ignores the passed in value
  virtual QString textFromValue(int value) const;
  /// Necessary over-ride that always returns QValidator::Acceptable since other code verifies the text
  virtual QValidator::State validate(QString& text, int& pos) const;
  /// Need to calculate the size of the spinner to overcome the Qt limitation of 18 characters for a spinner
  virtual QSize	sizeHint() const;

protected:
  /// Monitor input focus to keep track of time changes
  virtual void focusInEvent(QFocusEvent* e);
  /// Perform updates now that user has stopped input
  virtual void focusOutEvent(QFocusEvent* e);

private slots:
  /// Applies timestamp entered by user
  void applyTimestamp_();

private:
  /// Queue an application of a new timestamp in applyInterval_ seconds.  Used to prevent updates before user is finished inputting edits
  void queueApplyTimestamp_() const;
  /// The text to display in the spin box
  SegmentedTexts* completeLine_;
  /// The time at the last edit
  simCore::TimeStamp lastEditedTime_;
  /// Flag to indicate if text color should change based on validation
  bool colorCode_;
  /// event filter object
  QObject* segmentedEventFilter_;
  /// The cache time from setTimeStamp()
  simCore::TimeStamp timeStamp_;
  /// The timeStamp_ in a string format to check for changes
  QString timeString_;
  /// Timer to manage applying new timestamp on user edit after appropriate interval
  QTimer* timer_;
  /// Amount of time in milliseconds to wait between user editing time and applying new timestamp.  Default is one second.  Interval < 0 disables automatic updates from edits
  int applyInterval_;
};

}

#endif

