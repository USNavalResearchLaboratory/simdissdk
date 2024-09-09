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
#ifndef SIMQT_SEGMENTED_LINE_H
#define SIMQT_SEGMENTED_LINE_H

#include <optional>
#include <QSpinBox>
#include "simCore/Time/Constants.h"
#include "simCore/Time/TimeClass.h"

namespace simQt {

class SegmentedTexts;

/// Base definition for a part of a line
class SDKQT_EXPORT SegmentedText
{
public:
  /// If tabStop is true, than the segment support a tab stop
  SegmentedText(bool tabStop)
    : tabStop_(tabStop)
  {
  }
  virtual ~SegmentedText()
  {
  }

  /// Returns true if the segment should be a tab stop
  bool tabStop() const { return tabStop_; }
  /// Number of characters in the segment
  virtual size_t numberOfCharacters() const = 0;
  /// Number of characters left in the segment
  virtual size_t spaceLeft() const = 0;
  /// Changes the segment value by amount
  virtual void stepBy(int amount) = 0;
  /// Returns the segment as a text
  virtual QString text() const = 0;
  /** Extract the segment from the line
   * @pre Should call validate before calling setText to make sure the text is valid
   * @param[in] line User supplied text
   * @param[in] startLocation Where to start the extraction
   * @param[in] state The state of the extraction
   * @return Where the next extraction should start
   */
  virtual size_t setText(const QString& line, size_t startLocation, QValidator::State& state) = 0;
  /** Validates the segment from the line
   * @param[in] line User supplied text
   * @param[in] startLocation Where to start the validation
   * @param[in] state The state of the validation
   * @return Where the next validation should start
   */
  virtual size_t validateText(const QString& line, size_t startLocation, QValidator::State& state) const = 0;

protected:
  bool tabStop_; ///< True means this line segment is a tab stop
};

class SDKQT_EXPORT SeparatorText : public SegmentedText
/// Support for non data characters like :,., and white space
{
public:
  /// constructor
  SeparatorText(QString separator, bool optional);
  virtual ~SeparatorText();

  virtual size_t numberOfCharacters() const;
  virtual size_t spaceLeft() const;
  virtual void stepBy(int amount);
  virtual QString text() const;
  virtual size_t setText(const QString& text, size_t startLocation, QValidator::State& state);
  virtual size_t validateText(const QString& text, size_t startLocation, QValidator::State& state) const;

protected:
  QString separator_; ///< separator string
  bool optional_; ///< is the separator optional
};

/// Support for an integer number
class SDKQT_EXPORT NumberText : public SegmentedText
{
public:
  /** Constructor
   * @param[in] line Parent that contains all the segments
   * @param[in] minValue Minimum value
   * @param[in] maxValue Maximum value
   * @param[in] maxDigits Maximum number of digits for the number
   * @param[in] leadingZeros True means to display leading zeros
   * @param[in] timeScaleFactor The scale factor for the correct unit of time, for example 60.0 means the number represents minutes
   * @param[in] optional The segment is optional
   */
  NumberText(SegmentedTexts* line, int minValue, int maxValue, size_t maxDigits, bool leadingZeros, double timeScaleFactor, bool optional);

  virtual ~NumberText();

  /// Return the number as an integer
  int value() const;
  /// Set the number
  void setValue(int value);
  /// The number of characters in the number
  virtual size_t numberOfCharacters() const;
  /// the number of digits the user can add to the number
  virtual size_t spaceLeft() const;
  /// Changes the segment value by amount
  virtual void stepBy(int amount);
  /// Returns the segment as a text
  virtual QString text() const;
  virtual size_t setText(const QString& text, size_t startLocation, QValidator::State& state);
  virtual size_t validateText(const QString& text, size_t startLocation, QValidator::State& state) const;

protected:
  ///@return the string representation of the given value
  QString toString_(int value) const;

  SegmentedTexts* line_;  ///< Parent that contains all the segments
  int minValue_;  ///< Minimum value
  int maxValue_;  ///< Maximum value
  size_t maxDigits_;  ///< Maximum number of digits for the number
  bool leadingZeros_;  ///< True means to display leading zeros
  QString text_;  ///< The text to display
  double timeScaleFactor_;  ///< The scale factor for the correct unit of time, for example 60.0 means the number represents minutes
  bool optional_;  ///< The segment is optional
};

/// An abbreviated month name ("Jan", "Feb", etc) or integer month representation
class SDKQT_EXPORT MonthText : public SegmentedText
{
public:
  /** Constructor. Set intMode to true to use in integer mode */
  MonthText(SegmentedTexts* parentLine, bool intMode = false);

  /// current month setting (Jan is 0 to 11 Dec)
  int intValue() const;
  /// set the current month (0 to 11)
  void setIntValue(int monthNum);

  // from SegmentedText
  virtual size_t numberOfCharacters() const;
  virtual size_t spaceLeft() const;
  virtual void stepBy(int amount);
  virtual QString text() const;
  virtual size_t setText(const QString& line, size_t startLocation, QValidator::State& state);
  virtual size_t validateText(const QString& line, size_t startLocation, QValidator::State& state) const;

private:
  // increase by one month
  void stepUp_();

  // decrease by one month
  void stepDn_();

  static const int ABBR_LENGTH = 3; ///< string length of the month's abbreviated name

  SegmentedTexts* line_;  ///< Parent that contains all the segments
  int currentMonth_; ///< 0 is Jan
  bool intMode_ = false;
};

/// Implements the base logic for a segmented line, Time is limited to year 2046
class SDKQT_EXPORT SegmentedTexts : public QObject
{
  Q_OBJECT

public:
  /// current state of the text value
  enum ValueState {ValueValid, ValueTooHigh, ValueTooLow};

  SegmentedTexts();
  virtual ~SegmentedTexts();

  /// Returns the displayed time in UTC
  virtual simCore::TimeStamp timeStamp() const = 0;
  /// Sets the displayed time in UTC
  virtual void setTimeStamp(const simCore::TimeStamp& value) = 0;
  /// Sets the time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Return the time range
  virtual void timeRange(int& scenarioReferenceYear, simCore::TimeStamp& start, simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);
  /// Make sure the value is limited by the time range, if necessary
  virtual simCore::TimeStamp clampTime(const simCore::TimeStamp& value);
  /// A segment calls valueEdited when the value has changed via the user.
  virtual void valueEdited() = 0;
  /// A segment calls valueChanged when the value has changed via the user or by setTimeStamp
  virtual void valueChanged() = 0;
  /// Set the number if digits after the decimal point
  void setPrecision(unsigned int digits);
  /// Returns the number of digits after the decimal point
  unsigned int precision() const;
  /// Set the time zone to use when displaying time
  virtual void setTimeZone(simCore::TimeZone Zone) = 0;
  /// Returns the time zone to use when displaying time
  virtual simCore::TimeZone timeZone() const = 0;

  /// Adds a line segment; takes ownership of part
  void addPart(SegmentedText* part);
  /// Clear and delete all line segments
  void clearParts();

  /** Given a character position, the routine returns the Line Segment containing the position.
  * @param[in] pos The current cursor position
  * @return The Line Segment containing the position
  */
  SegmentedText* locatePart(size_t pos) const;
  /** Calculates the first character position for the given Line Segment
  * @param[in] inputPart Given line segment
  * @return The character position in the line for the first character of the line segment
  */
  size_t getFirstCharacterLocation(const SegmentedText* inputPart) const;
  /** Calculates the next tab stop after the given Line Segment
  * @param[in] inputPart Given line segment
  * @return The next tab stop after inputPart, can return nullptr if walked off the end
  */
  SegmentedText* nextTabStop(const SegmentedText* inputPart) const;
  /** Calculates the previous tab stop after the given Line Segment
  * @param[in] inputPart Given line segment
  * @return The previous tab stop after inputPart, can return nullptr if walked off the front
  */
  SegmentedText* previousTabStop(const SegmentedText* inputPart) const;

  /// Returns the text of the complete line
  QString text() const;
  /// Set the widget to the line, the line should have already been validated by validateLine
  QValidator::State setText(const QString& text);
  /// Determines if the argument line has a valid form; the value could be out of range
  virtual QValidator::State validateText(const QString& text) const;

Q_SIGNALS:
  /// emitted when the time changes via the user
  void timeEdited(const simCore::TimeStamp& time);
  /// emitted when the time is changed by the user or by setTimeStamp
  void timeChanged(const simCore::TimeStamp& time);

protected:
  /// Make the segments for the display type
  virtual void makeSegments_() = 0;
  /// Returns true if current is within the time range of start to end as dictated by the flags
  bool inRange_(const simCore::TimeStamp& current, bool limitBeforeStart, bool limitAfterEnd) const;
  /// Creates the fraction part accounting for the precision.  Cannot be const.
  NumberText* createFactionOfSeconds_(int precision);

  /// convert the fractional part of Seconds (# of ns) to a field representation
  int fractionToField_(const simCore::Seconds& secondsRounded) const;
  /// convert the field representation of the fraction to a # of ns
  int fractionFromField_(int fractionFieldValue, int precision) const;

  simCore::TimeStamp start_;  ///< Start Time
  simCore::TimeStamp end_;  ///< End time
  simCore::TimeStamp adjustedStart_;  ///< Start Time adjusted down by the precision
  simCore::TimeStamp adjustedEnd_;  ///< End time adjusted up by the precision
  std::optional<int> scenarioReferenceYear_;  ///< Scenario Reference Year
  unsigned int precision_; ///< The number of digits after the decimal point, must be between 1 and 6
  bool limitBeforeStart_; ///< If true times before the start time are rejected
  bool limitAfterEnd_;  ///< If true times after the end time are rejected

private:
  /// Adjust the time range to account for the precision
  void adjustTimeRange_();

  QList<SegmentedText*> segments_;  ///< A list of segments
};

//----------------------------------------------------------------------------
/// Implements the seconds format, SS.sss
class SDKQT_EXPORT SecondsTexts : public SegmentedTexts
{
public:
  SecondsTexts();

  virtual ~SecondsTexts();
  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  // Seconds texts does not support timezone offset
  virtual void setTimeZone(simCore::TimeZone) { }
  virtual simCore::TimeZone timeZone() const { return simCore::TIMEZONE_UTC; }

protected:
  virtual void makeSegments_();

private:
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
};

//----------------------------------------------------------------------------
/// Implements the minutes format, MM:SS.sss
class SDKQT_EXPORT MinutesTexts : public SegmentedTexts
{
public:
  MinutesTexts();

  virtual ~MinutesTexts();
  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  // Minutes texts does not support timezone offset
  virtual void setTimeZone(simCore::TimeZone) { }
  virtual simCore::TimeZone timeZone() const { return simCore::TIMEZONE_UTC; }

protected:
  virtual void makeSegments_();

private:
  NumberText* minutes_; // Displays the minutes
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
};

//----------------------------------------------------------------------------
/// Implements the hours format, HH:MM::SS.sss
class SDKQT_EXPORT HoursTexts : public SegmentedTexts
{
public:
  HoursTexts();
  virtual ~HoursTexts();

  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  // Hours texts does not support timezone offset
  virtual void setTimeZone(simCore::TimeZone) { }
  virtual simCore::TimeZone timeZone() const { return simCore::TIMEZONE_UTC; }

protected:
  virtual void makeSegments_();

private:
  NumberText* hours_;  // Displays the hours
  NumberText* minutes_; // Displays the minutes
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
};

//----------------------------------------------------------------------------
/// Implements the Ordinal format, DDD YYYY HH:MM::SS.sss
class SDKQT_EXPORT OrdinalTexts : public SegmentedTexts
{
public:
  OrdinalTexts();
  virtual ~OrdinalTexts();

  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  virtual void setTimeZone(simCore::TimeZone zone);
  virtual simCore::TimeZone timeZone() const;

protected:
  virtual void makeSegments_();

private:
  NumberText* days_;  // Displays the day of year
  NumberText* years_;  // Displays the year
  NumberText* hours_;  // Displays the hours
  NumberText* minutes_; // Displays the minutes
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
  simCore::TimeZone zone_;
};

/// Implements the MonthDayYear format, NNN D YYYY HH:MM::SS.sss
class SDKQT_EXPORT MonthDayYearTexts : public SegmentedTexts
{
public:
  MonthDayYearTexts();
  virtual ~MonthDayYearTexts();

  // from SegmentedTexts
  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  virtual void setTimeZone(simCore::TimeZone zone);
  virtual simCore::TimeZone timeZone() const;

protected:
  virtual void makeSegments_();

private:
  MonthText* month_; // Displays the month
  NumberText* days_;  // Displays the day of year
  NumberText* years_;  // Displays the year
  NumberText* hours_;  // Displays the hours
  NumberText* minutes_; // Displays the minutes
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
  simCore::TimeZone zone_;
};

//----------------------------------------------------------------------------
/// Implements the ISO-8601 format, YYYY-MM-DDTHH:MM:SS.sssZ, with optional [.sss]
class SDKQT_EXPORT Iso8601Texts : public SegmentedTexts
{
public:
  Iso8601Texts();
  virtual ~Iso8601Texts();

  virtual simCore::TimeStamp timeStamp() const;
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  virtual void valueEdited();
  virtual void valueChanged();
  virtual QValidator::State validateText(const QString& text) const;
  virtual void setTimeZone(simCore::TimeZone zone);
  virtual simCore::TimeZone timeZone() const;

protected:
  virtual void makeSegments_();

private:
  NumberText* years_;  // Displays the year
  MonthText* months_;  // Displays the month
  NumberText* days_;  // Displays the day of month
  NumberText* hours_;  // Displays the hours
  NumberText* minutes_; // Displays the minutes
  NumberText* seconds_;  // Displays the seconds
  NumberText* fraction_;  // Displays the fraction
  simCore::TimeZone zone_;
};

}

#endif

