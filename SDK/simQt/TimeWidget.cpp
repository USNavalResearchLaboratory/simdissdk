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
#include <ctype.h>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QDateTimeEdit>
#include <QSpinBox>

#include "simCore/Time/Constants.h"
#include "simCore/Time/Utils.h"
#include "simQt/TimeFormatContainer.h"
#include "simQt/QtConversion.h"
#include "simQt/TimeWidget.h"

namespace simQt {


TimeWidget::TimeWidget(QWidget* parent)
  : QWidget(parent),
    scenarioReferenceYear_(1970)
{
  // Setup the format widgets to switch between
  addContainer_(new SecondsContainer(this), SLOT(setSeconds_()));
  addContainer_(new MinutesContainer(this), SLOT(setMinutes_()));
  addContainer_(new HoursContainer(this), SLOT(setHours_()));
  addContainer_(new MonthContainer(this), SLOT(setMonth_()));
  addContainer_(new OrdinalContainer(this), SLOT(setOrdinal_()));

  // Make this one the default format
  currentContainer_ = containers_.back();
  currentContainer_->widget()->setHidden(false);

  // Setup the label and right mouse click menu to change formats
  title_ = new QLabel(this);
  title_->setText("Time:");
  title_->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
  title_->setContextMenuPolicy(Qt::CustomContextMenu);
  Q_FOREACH(TimeFormatContainer* w, containers_)
    connect(w, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showRightMouseClickMenu_(const QPoint &)));

  // Need a layout to make the widget fill the parent widget
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->addWidget(title_);
  layout->addWidget(currentContainer_->widget());
  this->setLayout(layout);  // do not use parent since parent could be NULL

  colorCodeAction_ = new QAction(tr("&Color Code Text"), this);
  connect(colorCodeAction_, SIGNAL(triggered()), this, SLOT(setColorCode_()));
  colorCodeAction_->setCheckable(true);

  // The right mouse click menu to change color coding
  rightMouseClickMenu_ = new QMenu(this);
  rightMouseClickMenu_->addAction(colorCodeAction_);
}

TimeWidget::~TimeWidget()
{
  Q_FOREACH(TimeFormatContainer* w, containers_)
    delete w;
  containers_.clear();

  delete colorCodeAction_;
  colorCodeAction_ = NULL;
}

void TimeWidget::addContainer_(TimeFormatContainer* widget, const QString& slotText)
{
  QAction* action = new QAction(widget->name(), this);
  connect(action, SIGNAL(triggered()), this, slotText.toStdString().c_str());
  action->setCheckable(true);
  this->addAction(action);
  widget->setAction(action);
  widget->widget()->setHidden(true);
  connect(dynamic_cast<QObject*>(widget), SIGNAL(timeChanged(const simCore::TimeStamp&)), this, SLOT(timeWasChanged_(const simCore::TimeStamp&)));
  containers_.push_back(widget);
}

QString TimeWidget::label() const
{
  return title_->text();
}

void TimeWidget::setLabel(QString value)
{
  title_->setText(value);
  title_->setHidden(value.isEmpty());
}

QString TimeWidget::labelToolTip() const
{
  return title_->toolTip();
}

void TimeWidget::setLabelToolTip(QString value)
{
  title_->setToolTip(value);
}

bool TimeWidget::colorCodeText() const
{
  return currentContainer_->colorCode();
}

void TimeWidget::setColorCodeText(bool value)
{
  Q_FOREACH(TimeFormatContainer* w, containers_)
    w->setColorCode(value);
}

simCore::TimeStamp TimeWidget::timeStamp() const
{
  return currentContainer_->timeStamp();
}

void TimeWidget::setTimeStamp(const simCore::TimeStamp& value)
{
  if (currentContainer_->hasFocus())
    return;

  bool emitSignal = (value != currentContainer_->timeStamp());

  // Keep all time format widgets in sync
  Q_FOREACH(TimeFormatContainer* w, containers_)
    w->setTimeStamp(value);

  if (emitSignal)
    emit timeModified(currentContainer_->timeStamp());
}

void TimeWidget::setTimeRange(int scenarioReferenceYear, const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  // Only change the time range when it changes
  if (scenarioReferenceYear != scenarioReferenceYear_ ||
    start != timeRangeStart_ || end != timeRangeEnd_)
  {
    scenarioReferenceYear_ = scenarioReferenceYear;
    timeRangeStart_ = start;
    timeRangeEnd_ = end;

    // Keep all time format widgets in sync
    Q_FOREACH(TimeFormatContainer* w, containers_)
      w->setTimeRange(scenarioReferenceYear, start, end);

    emit timeRangeChanged();
  }
}

void TimeWidget::getEnforceLimits(bool& limitBeforeStart, bool& limitAfterEnd) const
{
  currentContainer_->getEnforceLimits(limitBeforeStart, limitAfterEnd);
}

void TimeWidget::setEnforceLimits(bool limitBeforeStart, bool limitAfterEnd)
{
  // Keep all time format widgets in sync
  Q_FOREACH(TimeFormatContainer* w, containers_)
    w->setEnforceLimits(limitBeforeStart, limitAfterEnd);
}


void TimeWidget::showRightMouseClickMenu_(const QPoint &pos)
{
  // Put a check mark next to the current format
  Q_FOREACH(TimeFormatContainer* w, containers_)
    w->action()->setChecked(currentContainer_->timeFormat() == w->timeFormat());

  colorCodeAction_->setChecked(currentContainer_->colorCode());

  rightMouseClickMenu_->exec(title_->mapToGlobal(pos));
}

simCore::TimeFormat TimeWidget::timeFormat() const
{
  return currentContainer_->timeFormat();
}

unsigned int TimeWidget::precision() const
{
  return currentContainer_->precision();
}

/// Switch the display format to newFormat
void TimeWidget::setTimeFormat(simCore::TimeFormat newFormat)
{
  // Currently do not support DTG, so switch over to Month Day format
  if (newFormat == simCore::TIMEFORMAT_DTG)
    newFormat = simCore::TIMEFORMAT_MONTHDAY;

  Q_FOREACH(TimeFormatContainer* w, containers_)
  {
    if (w->timeFormat() == newFormat)
    {
      this->layout()->removeWidget(currentContainer_->widget());
      currentContainer_->widget()->setHidden(true);
      // user might have changed the time before switching, so move time over to new widget
      w->setTimeStamp(currentContainer_->timeStamp());
      currentContainer_ = w;
      currentContainer_->widget()->setHidden(false);
      this->layout()->addWidget(currentContainer_->widget());
      break;
    }
  }
}

void TimeWidget::setPrecision(unsigned int digits)
{
  // save off the current time to force a redraw after setting the precision
  simCore::TimeStamp currentTime = currentContainer_->timeStamp();
  Q_FOREACH(TimeFormatContainer* w, containers_)
  {
    w->setPrecision(digits);
  }
  currentContainer_->setTimeStamp(currentTime);
}

void TimeWidget::timeWasChanged_(const simCore::TimeStamp& time)
{
  // emitted when the time changes via the user
  emit timeChanged(time);
  // emitted when the time is changed by the user or by setTimeStamp
  emit timeModified(time);
}

void TimeWidget::setSeconds_()
{
  setTimeFormat(simCore::TIMEFORMAT_SECONDS);
}

void TimeWidget::setMinutes_()
{
  setTimeFormat(simCore::TIMEFORMAT_MINUTES);
}

void TimeWidget::setHours_()
{
  setTimeFormat(simCore::TIMEFORMAT_HOURS);
}

void TimeWidget::setOrdinal_()
{
  setTimeFormat(simCore::TIMEFORMAT_ORDINAL);
}

void TimeWidget::setMonth_()
{
  setTimeFormat(simCore::TIMEFORMAT_MONTHDAY);
}

void TimeWidget::setColorCode_()
{
  setColorCodeText(!currentContainer_->colorCode());
}

int TimeWidget::scenarioReferenceYear() const
{
  return scenarioReferenceYear_;
}

simCore::TimeStamp TimeWidget::timeRangeStart() const
{
  return timeRangeStart_;
}

simCore::TimeStamp TimeWidget::timeRangeEnd() const
{
  return timeRangeEnd_;
}

}
