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
#ifndef SIMQT_TIME_FORMAT_CONTAINER_H
#define SIMQT_TIME_FORMAT_CONTAINER_H

#include <QString>
#include <QDateTime>
#include <QEvent>
#include <QKeyEvent>

#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Constants.h"

class QDateTimeEdit;

namespace simQt {

class SegmentedSpinBox;

/**
 * The TimeFormatContainer wraps the different format implementations into a standard interface for the TimeWidget.
 * The Seconds format uses QDoubleSpinBox, the DateTime format uses QDateTimeEdit and all the other formats use SegmentedSpinBox.
 */
class SDKQT_EXPORT TimeFormatContainer : public QObject
{
  Q_OBJECT

public:
  /// constructor
  TimeFormatContainer(simCore::TimeFormat timeFormat, const QString& name);
  virtual ~TimeFormatContainer();

  ///@return the enumeration of the time format
  simCore::TimeFormat timeFormat() const;
  ///@return the name that the user will see
  QString name() const;
  ///@return the QAction for the format
  QAction* action() const;
  /// Set the QAction for the format
  void setAction(QAction* action);

  ///@return the widget actually displaying the time
  virtual QWidget* widget() = 0;

  ///@return true if the container has input focus
  virtual bool hasFocus() const = 0;

  /// Get current time
  virtual simCore::TimeStamp timeStamp() const = 0;
  /// Get current time as text
  virtual QString timeText() const = 0;
  /// Set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value) = 0;
  /// Set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end) = 0;
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const = 0;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd) = 0;

  ///@return true if the color changes on error
  virtual bool colorCode() const = 0;
  /// set the behavior of color change on error
  virtual void setColorCode(bool value) = 0;

  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits) = 0;
  /// get the number of digits after the decimal point
  virtual unsigned int precision() = 0;

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone) = 0;
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const = 0;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip() = 0;
  /// Returns the text for a tool tip
  virtual QString toolTipText() const = 0;

signals:
  /// Emitted when the time changes via the user
  void timeEdited(const simCore::TimeStamp& value);
  /// Emitted when the time is changed by the user or by setTimeStamp
  void timeChanged(const simCore::TimeStamp& value);
  /// Tell the outside the user is requesting a context menu
  void customContextMenuRequested(const QPoint& point);

protected:
  simCore::TimeFormat timeFormat_;  ///< Enumeration used to identify this widget
  QString name_;  ///< The name to display to the user
  QAction* action_;  ///< The action to invoke the switch
};

/// Implements the Seconds format
class SDKQT_EXPORT SecondsContainer : public TimeFormatContainer
{
public:
  /// constructor
  SecondsContainer(QWidget* parent=nullptr);
  virtual ~SecondsContainer();

  ///@return the underlying widget
  virtual QWidget* widget();
  ///@return true if this has focus
  virtual bool hasFocus() const;
  ///@return current time
  virtual simCore::TimeStamp timeStamp() const;
  ///@return current time as text
  virtual QString timeText() const;
  /// set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return the state of "change color on error"
  virtual bool colorCode() const;
  /// set "change color on error"
  virtual void setColorCode(bool value);
  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits);
  /// get the number of digits after the decimal point
  virtual unsigned int precision();

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone zone);
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip();
  /// Returns the text for a tool tip
  virtual QString toolTipText() const;

protected:
  SegmentedSpinBox* widget_; ///< The widget to display the time in seconds
};

/// Implements the Months format
class SDKQT_EXPORT MonthContainer : public TimeFormatContainer
{
public:
  /// constructor
  MonthContainer(QWidget* parent=nullptr);
  virtual ~MonthContainer();

  ///@return the underlying widget
  virtual QWidget* widget();
  ///@return true if this has focus
  virtual bool hasFocus() const;
  ///@return current time
  virtual simCore::TimeStamp timeStamp() const;
  ///@return current time as text
  virtual QString timeText() const;
  /// set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return the state of "change color on error"
  virtual bool colorCode() const;
  /// set "change color on error"
  virtual void setColorCode(bool value);
  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits);
  /// get the number of digits after the decimal point
  virtual unsigned int precision();

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone zone);
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip();
  /// Returns the text for a tool tip
  virtual QString toolTipText() const;

protected:
  SegmentedSpinBox* widget_; ///< The widget to display the ordinal time
  bool colorCode_; ///< if true, change color when validation fails
};

/// Implements the Ordinal format
class SDKQT_EXPORT OrdinalContainer :  public TimeFormatContainer
{
public:
  /// constructor
  OrdinalContainer(QWidget* parent=nullptr);
  virtual ~OrdinalContainer();

  ///@return the underlying widget
  virtual QWidget* widget();
  ///@return true if this has focus
  virtual bool hasFocus() const;
  ///@return current time
  virtual simCore::TimeStamp timeStamp() const;
  ///@return current time as text
  virtual QString timeText() const;
  /// set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return the state of "change color on error"
  virtual bool colorCode() const;
  /// set "change color on error"
  virtual void setColorCode(bool value);
  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits);
  /// get the number of digits after the decimal point
  virtual unsigned int precision();

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone zone);
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip();
  /// Returns the text for a tool tip
  virtual QString toolTipText() const;

protected:
  SegmentedSpinBox* widget_; ///< The widget to display the ordinal time
};

/// Implements the Minutes format
class SDKQT_EXPORT MinutesContainer : public TimeFormatContainer
{
public:
  /// constructor
  MinutesContainer(QWidget* parent=nullptr);
  virtual ~MinutesContainer();

  ///@return the underlying widget
  virtual QWidget* widget();
  ///@return true if this has focus
  virtual bool hasFocus() const;
  ///@return current time
  virtual simCore::TimeStamp timeStamp() const;
  ///@return current time as text
  virtual QString timeText() const;
  /// set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return the state of "change color on error"
  virtual bool colorCode() const;
  /// set "change color on error"
  virtual void setColorCode(bool value);
  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits);
  /// get the number of digits after the decimal point
  virtual unsigned int precision();

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone zone);
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip();
  /// Returns the text for a tool tip
  virtual QString toolTipText() const;

protected:
  SegmentedSpinBox* widget_; ///< The widget to display the minutes
};

/// Implements the Hours format
class SDKQT_EXPORT HoursContainer : public TimeFormatContainer
{
public:
  /// constructor
  HoursContainer(QWidget* parent=nullptr);
  virtual ~HoursContainer();

  ///@return the underlying widget
  virtual QWidget* widget();
  ///@return true if this has focus
  virtual bool hasFocus() const;
  ///@return current time
  virtual simCore::TimeStamp timeStamp() const;
  ///@return current time as text
  virtual QString timeText() const;
  /// set current time
  virtual void setTimeStamp(const simCore::TimeStamp& value);
  /// set begin/end time range
  virtual void setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end);
  /// Returns which time limits are enforced
  virtual void getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const;
  /// Sets which time limits to enforced
  virtual void setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd);

  ///@return the state of "change color on error"
  virtual bool colorCode() const;
  /// set "change color on error"
  virtual void setColorCode(bool value);
  /// Set the number of digits after the decimal point
  virtual void setPrecision(unsigned int digits);
  /// get the number of digits after the decimal point
  virtual unsigned int precision();

  /// Set the time zone to use when displaying text
  virtual void setTimeZone(simCore::TimeZone zone);
  /// Get the time zone to use when displaying text
  virtual simCore::TimeZone timeZone() const;

  /// Disable tool tip since it can interfere with editing the time
  virtual void disableToolTip();
  /// Returns the text for a tool tip
  virtual QString toolTipText() const;

protected:
  SegmentedSpinBox* widget_; ///< The widget to display the hours
};

} // namespace

#endif

