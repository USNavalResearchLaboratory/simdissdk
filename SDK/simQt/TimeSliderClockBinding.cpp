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
#include <cassert>
#include <QSlider>
#include "simCore/Time/Clock.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/TimeSliderClockBinding.h"

namespace simQt
{

/// forward callbacks to slider when clock time changes
class TimeSliderClockBinding::TimeObserver : public simCore::Clock::TimeObserver
{
public:
  /// constructor
  explicit TimeObserver(TimeSliderClockBinding* binding)
    : binding_(binding)
  {
  }
  virtual void onSetTime(const simCore::TimeStamp &t, bool isJump)
  {
    binding_->updateSliderTime_(t);
  }
  virtual void onTimeLoop()
  { // no-op
  }
  virtual void adjustTime(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
  { // no-op
  }
private:
  TimeSliderClockBinding* binding_;
};

/// forward callbacks to slider when clock mode changes
class TimeSliderClockBinding::ModeObserver : public simCore::Clock::ModeChangeObserver
{
public:
  /// constructor
  explicit ModeObserver(TimeSliderClockBinding* binding)
    : binding_(binding)
  {
  }
  /// clock mode has changed
  virtual void onModeChange(simCore::Clock::Mode newMode) {}
  /// direction has changed
  virtual void onDirectionChange(simCore::TimeDirection newDirection) {}
  /// time scale has changed
  virtual void onScaleChange(double newValue) {}
  /// start/end times have changed
  virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end)
  {
    binding_->updateSliderTime_();
  }
  /// canLoop has changed
  virtual void onCanLoopChange(bool newVal) {}
  /// Editable state has changed
  virtual void onUserEditableChanged(bool userCanEdit)
  {
    binding_->fixEnabledState_(userCanEdit);
  }
private:
  TimeSliderClockBinding* binding_;
};

TimeSliderClockBinding::TimeSliderClockBinding(QSlider* parent, simCore::Clock* clock, DisabledMode disabledMode)
 : QObject(parent),
   slider_(parent),
   clock_(clock),
   disabledMode_(disabledMode),
   allowVisible_(true)
{
  assert(parent != nullptr); // must pass in slider in constructor
  timeObserver_.reset(new TimeObserver(this));
  modeObserver_.reset(new ModeObserver(this));
  connect(slider_, SIGNAL(valueChanged(int)), this, SLOT(valueChanged_(int)));
  bindClock(clock_);
}

TimeSliderClockBinding::~TimeSliderClockBinding()
{
  unbindClock();
}

void TimeSliderClockBinding::bindClock(simCore::Clock* clock)
{
  unbindClock();
  clock_ = clock;
  if (clock_)
  {
    clock_->registerTimeCallback(timeObserver_);
    clock_->registerModeChangeCallback(modeObserver_);
  }
  // Set the initial state
  fixEnabledState_(clock_ != nullptr && clock_->isUserEditable());
  updateSliderTime_();
}

void TimeSliderClockBinding::unbindClock()
{
  if (clock_)
  {
    clock_->removeTimeCallback(timeObserver_);
    clock_->removeModeChangeCallback(modeObserver_);
  }
  clock_ = nullptr;
}


void TimeSliderClockBinding::valueChanged_(int sliderPos)
{
  assert(sender() == slider_); // Only valid sender of this signal is our slider
  if (clock_ == nullptr)
    return;
  const simCore::Seconds& deltaTime = clock_->endTime() - clock_->startTime();
  if (deltaTime == 0)
    return;

  // Calculate the new time based on slider min/max
  const double sliderSize = slider_->maximum() - slider_->minimum();
  const double newTime = ((sliderPos - slider_->minimum())/ sliderSize) * deltaTime + clock_->startTime().secondsSinceRefYear();
  // Block signals from the slider to prevent loopback
  simQt::ScopedSignalBlocker blockSignals(*slider_);
  clock_->setTime(simCore::TimeStamp(clock_->startTime().referenceYear(), newTime));
}

void TimeSliderClockBinding::fixEnabledState_(bool enableIt)
{
  // Take care not to show/hide if our disabled mode doesn't support visibility changes
  if (disabledMode() == HIDE || disabledMode() == HIDE_AND_DISABLE)
    slider_->setVisible(allowVisible_ && enableIt);

  // Take care not to enable/disable if our disabled mode doesn't support setEnabled changes
  if (disabledMode() == DISABLE || disabledMode() == HIDE_AND_DISABLE)
    slider_->setEnabled(enableIt);
}

TimeSliderClockBinding::DisabledMode TimeSliderClockBinding::disabledMode() const
{
  return disabledMode_;
}

void TimeSliderClockBinding::updateSliderTime_()
{
  if (clock_)
    updateSliderTime_(clock_->currentTime());
}

void TimeSliderClockBinding::updateSliderTime_(const simCore::TimeStamp &t)
{
  assert(clock_ != nullptr); // Should not get this notification unless clock_ is non-nullptr
  if (clock_ == nullptr || clock_->endTime() == simCore::INFINITE_TIME_STAMP)
    return;
  const simCore::Seconds& deltaTime = clock_->endTime() - clock_->startTime();
  if (deltaTime <= 0)
    return;

  // Calculate the percentage from 0.0 to 1.0 of time
  const double percentTime = (clock_->currentTime() - clock_->startTime()).Double() / deltaTime;
  // Block signals from the slider to prevent loopback
  simQt::ScopedSignalBlocker blockSignals(*slider_);
  slider_->setValue(static_cast<int>(percentTime * (slider_->maximum() - slider_->minimum()) + slider_->minimum()));
}

void TimeSliderClockBinding::setAllowVisible(bool allowVisible)
{
  if (allowVisible_ == allowVisible)
    return;
  allowVisible_ = allowVisible;

  // If visibility is not allowed, set it to hidden
  if (!allowVisible_)
    slider_->setVisible(false);
  else
  {
    // Visibility is allowed -- it was previously unallowed.  We can only mark the item visible
    // if the clock is user editable, and if that matters for visibility.  Let fixEnabledState_()
    // take care of it.
    fixEnabledState_(clock_ != nullptr && clock_->isUserEditable());
  }
}

bool TimeSliderClockBinding::allowVisible() const
{
  return allowVisible_;
}

}
