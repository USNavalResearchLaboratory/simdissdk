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
#include <QAction>
#include <QDateTimeEdit>
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/SegmentedSpinBox.h"
#include "simQt/SegmentedTexts.h"
#include "simQt/QtFormatting.h"
#include "simQt/TimeFormatContainer.h"

namespace simQt {

static const QString USAGE_STR = QObject::tr("<p>To edit, mouse click into a field and then type new value or use the up and down arrows. Move between fields via Tab or Shift+Tab."
    "<p>Use the right mouse click to toggle color coding.  The text is blue if the time is outside the range of the existing scenario.  The text is red if the format is invalid.");

TimeFormatContainer::TimeFormatContainer(simCore::TimeFormat timeFormat, const QString& name)
  : timeFormat_(timeFormat),
  name_(name),
  action_(nullptr)
{
}

TimeFormatContainer::~TimeFormatContainer()
{
  delete action_;
}

simCore::TimeFormat TimeFormatContainer::timeFormat() const
{
  return timeFormat_;
}

QString TimeFormatContainer::name() const
{
  return name_;
}


QAction* TimeFormatContainer::action() const
{
  return action_;
}

void TimeFormatContainer::setAction(QAction* action)
{
  action_ = action;
}


//----------------------------------------------------------------------------------------------
SecondsContainer::SecondsContainer(QWidget* parent)
  : TimeFormatContainer(simCore::TIMEFORMAT_SECONDS, "Seconds")
{
  widget_ = new SegmentedSpinBox(parent);
  widget_->setToolTip(toolTipText());

  widget_->setLine(new SecondsTexts());
  connect(widget_->line(), SIGNAL(timeEdited(simCore::TimeStamp)), this, SIGNAL(timeEdited(simCore::TimeStamp)));
  connect(widget_->line(), SIGNAL(timeChanged(simCore::TimeStamp)), this, SIGNAL(timeChanged(simCore::TimeStamp)));
  connect(widget_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SIGNAL(customContextMenuRequested(const QPoint &)));
}

SecondsContainer::~SecondsContainer()
{
}

QWidget* SecondsContainer::widget()
{
  return widget_;
}

bool SecondsContainer::hasFocus() const
{
  return widget_->hasFocus();
}

simCore::TimeStamp SecondsContainer::timeStamp() const
{
  return widget_->timeStamp();
}

void SecondsContainer::setTimeStamp(const simCore::TimeStamp& value)
{
  widget_->setTimeStamp(value);
}

void SecondsContainer::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  widget_->setTimeRange(scenarioReferenceYear, start, end);
}

void SecondsContainer::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  widget_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void SecondsContainer::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  widget_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}

bool SecondsContainer::colorCode() const
{
  return widget_->colorCode();
}

void SecondsContainer::setColorCode(bool value)
{
  widget_->setColorCode(value);
}

void SecondsContainer::setPrecision(unsigned int digits)
{
  widget_->line()->setPrecision(digits);
}

unsigned int SecondsContainer::precision()
{
  return widget_->line()->precision();
}

void SecondsContainer::setTimeZone(simCore::TimeZone zone)
{
  widget_->line()->setTimeZone(zone);
}

simCore::TimeZone SecondsContainer::timeZone() const
{
  return widget_->line()->timeZone();
}

void SecondsContainer::disableToolTip()
{
  widget_->setToolTip("");
}

QString SecondsContainer::toolTipText() const
{
  return simQt::formatTooltip(tr("Time"), tr("Set the time in seconds since beginning of reference year.") + USAGE_STR);
}

//----------------------------------------------------------------------------------------------
MonthContainer::MonthContainer(QWidget* parent)
  : TimeFormatContainer(simCore::TIMEFORMAT_MONTHDAY, "Month Day Year"),
    colorCode_(true)
{
  widget_ = new SegmentedSpinBox(parent);
  widget_->setToolTip(toolTipText());

  widget_->setLine(new MonthDayYearTexts());
  connect(widget_->line(), SIGNAL(timeEdited(simCore::TimeStamp)), this, SIGNAL(timeEdited(simCore::TimeStamp)));
  connect(widget_->line(), SIGNAL(timeChanged(simCore::TimeStamp)), this, SIGNAL(timeChanged(simCore::TimeStamp)));
  connect(widget_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SIGNAL(customContextMenuRequested(const QPoint &)));
}

MonthContainer::~MonthContainer()
{
}

QWidget* MonthContainer::widget()
{
  return widget_;
}

bool MonthContainer::hasFocus() const
{
  return widget_->hasFocus();
}

simCore::TimeStamp MonthContainer::timeStamp() const
{
  return widget_->timeStamp();
}

void MonthContainer::setTimeStamp(const simCore::TimeStamp& value)
{
  widget_->setTimeStamp(value);
}

void MonthContainer::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  widget_->setTimeRange(scenarioReferenceYear, start, end);
}

void MonthContainer::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  widget_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void MonthContainer::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  widget_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}

bool MonthContainer::colorCode() const
{
  return colorCode_;
}

void MonthContainer::setColorCode(bool value)
{
  colorCode_ = value;
}

void MonthContainer::setPrecision(unsigned int digits)
{
  widget_->line()->setPrecision(digits);
}

unsigned int MonthContainer::precision()
{
  return widget_->line()->precision();
}

void MonthContainer::setTimeZone(simCore::TimeZone zone)
{
  widget_->line()->setTimeZone(zone);
}

simCore::TimeZone MonthContainer::timeZone() const
{
  return widget_->line()->timeZone();
}

void MonthContainer::disableToolTip()
{
  widget_->setToolTip("");
}

QString MonthContainer::toolTipText() const
{
  return simQt::formatTooltip(tr("Time"), tr("Set the time in Month Day Year format.") + USAGE_STR);
}

//----------------------------------------------------------------------------------------------

OrdinalContainer::OrdinalContainer(QWidget* parent)
  : TimeFormatContainer(simCore::TIMEFORMAT_ORDINAL, "Ordinal")
{
  widget_ = new SegmentedSpinBox(parent);
  widget_->setToolTip(toolTipText());

  widget_->setLine(new OrdinalTexts());
  connect(widget_->line(), SIGNAL(timeEdited(simCore::TimeStamp)), this, SIGNAL(timeEdited(simCore::TimeStamp)));
  connect(widget_->line(), SIGNAL(timeChanged(simCore::TimeStamp)), this, SIGNAL(timeChanged(simCore::TimeStamp)));
  connect(widget_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SIGNAL(customContextMenuRequested(const QPoint &)));
}

OrdinalContainer::~OrdinalContainer()
{
}

QWidget* OrdinalContainer::widget()
{
  return widget_;
}

bool OrdinalContainer::hasFocus() const
{
  return widget_->hasFocus();
}

simCore::TimeStamp OrdinalContainer::timeStamp() const
{
  return widget_->timeStamp();
}

void OrdinalContainer::setTimeStamp(const simCore::TimeStamp& value)
{
  widget_->setTimeStamp(value);
}

void OrdinalContainer::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  widget_->setTimeRange(scenarioReferenceYear, start, end);
}


void OrdinalContainer::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  widget_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void OrdinalContainer::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  widget_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}

bool OrdinalContainer::colorCode() const
{
  return widget_->colorCode();
}

void OrdinalContainer::setColorCode(bool value)
{
  widget_->setColorCode(value);
}

void OrdinalContainer::setPrecision(unsigned int digits)
{
  widget_->line()->setPrecision(digits);
}

unsigned int OrdinalContainer::precision()
{
  return widget_->line()->precision();
}

void OrdinalContainer::setTimeZone(simCore::TimeZone zone)
{
  widget_->line()->setTimeZone(zone);
}

simCore::TimeZone OrdinalContainer::timeZone() const
{
  return widget_->line()->timeZone();
}

void OrdinalContainer::disableToolTip()
{
  widget_->setToolTip("");
}

QString OrdinalContainer::toolTipText() const
{
  return simQt::formatTooltip(tr("Time"), tr("Set the time in Ordinal format.") + USAGE_STR);
}

//----------------------------------------------------------------------------------------------

MinutesContainer::MinutesContainer(QWidget* parent)
  : TimeFormatContainer(simCore::TIMEFORMAT_MINUTES, "Minutes")
{
  widget_ = new SegmentedSpinBox(parent);
  widget_->setToolTip(toolTipText());

  widget_->setLine(new MinutesTexts());
  connect(widget_->line(), SIGNAL(timeEdited(simCore::TimeStamp)), this, SIGNAL(timeEdited(simCore::TimeStamp)));
  connect(widget_->line(), SIGNAL(timeChanged(simCore::TimeStamp)), this, SIGNAL(timeChanged(simCore::TimeStamp)));
  connect(widget_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SIGNAL(customContextMenuRequested(const QPoint &)));
}

MinutesContainer::~MinutesContainer()
{
}

QWidget* MinutesContainer::widget()
{
  return widget_;
}

bool MinutesContainer::hasFocus() const
{
  return widget_->hasFocus();
}

simCore::TimeStamp MinutesContainer::timeStamp() const
{
  return widget_->timeStamp();
}

void MinutesContainer::setTimeStamp(const simCore::TimeStamp& value)
{
  widget_->setTimeStamp(value);
}

void MinutesContainer::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  widget_->setTimeRange(scenarioReferenceYear, start, end);
}

void MinutesContainer::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  widget_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void MinutesContainer::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  widget_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}

bool MinutesContainer::colorCode() const
{
  return widget_->colorCode();
}

void MinutesContainer::setColorCode(bool value)
{
  widget_->setColorCode(value);
}

void MinutesContainer::setPrecision(unsigned int digits)
{
  widget_->line()->setPrecision(digits);
}

unsigned int MinutesContainer::precision()
{
  return widget_->line()->precision();
}

void MinutesContainer::setTimeZone(simCore::TimeZone zone)
{
  widget_->line()->setTimeZone(zone);
}

simCore::TimeZone MinutesContainer::timeZone() const
{
  return widget_->line()->timeZone();
}

void MinutesContainer::disableToolTip()
{
  widget_->setToolTip("");
}

QString MinutesContainer::toolTipText() const
{
  return simQt::formatTooltip(tr("Time"), tr("Set the time in minutes since beginning of reference year.") + USAGE_STR);
}

//----------------------------------------------------------------------------------------------

HoursContainer::HoursContainer(QWidget* parent)
  : TimeFormatContainer(simCore::TIMEFORMAT_HOURS, "Hours")
{
  widget_ = new SegmentedSpinBox(parent);
  widget_->setToolTip(toolTipText());

  widget_->setLine(new HoursTexts());
  connect(widget_->line(), SIGNAL(timeEdited(simCore::TimeStamp)), this, SIGNAL(timeEdited(simCore::TimeStamp)));
  connect(widget_->line(), SIGNAL(timeChanged(simCore::TimeStamp)), this, SIGNAL(timeChanged(simCore::TimeStamp)));
  connect(widget_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SIGNAL(customContextMenuRequested(const QPoint &)));
}

HoursContainer::~HoursContainer()
{
}

QWidget* HoursContainer::widget()
{
  return widget_;
}

bool HoursContainer::hasFocus() const
{
  return widget_->hasFocus();
}

simCore::TimeStamp HoursContainer::timeStamp() const
{
  return widget_->timeStamp();
}

void HoursContainer::setTimeStamp(const simCore::TimeStamp& value)
{
  widget_->setTimeStamp(value);
}

void HoursContainer::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  widget_->setTimeRange(scenarioReferenceYear, start, end);
}

void HoursContainer::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  widget_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void HoursContainer::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  widget_->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}

bool HoursContainer::colorCode() const
{
  return widget_->colorCode();
}

void HoursContainer::setColorCode(bool value)
{
  widget_->setColorCode(value);
}

void HoursContainer::setPrecision(unsigned int digits)
{
  widget_->line()->setPrecision(digits);
}

unsigned int HoursContainer::precision()
{
  return widget_->line()->precision();
}

void HoursContainer::setTimeZone(simCore::TimeZone zone)
{
  widget_->line()->setTimeZone(zone);
}

simCore::TimeZone HoursContainer::timeZone() const
{
  return widget_->line()->timeZone();
}

void HoursContainer::disableToolTip()
{
  widget_->setToolTip("");
}

QString HoursContainer::toolTipText() const
{
  return simQt::formatTooltip(tr("Time"), tr("Set the time in hours since beginning of reference year.") + USAGE_STR);
}

}
