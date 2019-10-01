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
#include <cmath>
#include <algorithm>
#include "simCore/Time/Utils.h"
#include "simCore/Time/ClockImpl.h"

namespace simCore
{

/** Helper class that will monitor the "isUserEditable" flag and fire a notification when it changes on destruction. */
class ClockImpl::ScopedUserEditableWatch
{
public:
  /**
   * Constructor; intended to be used as scoped object on stack.
   * @param clock Instance saves state of clock and emits a notification on this clock if, at
   *   time of destruction, the state for user-editable has changed.
   */
  explicit ScopedUserEditableWatch(ClockImpl& clock)
    : clock_(clock),
      wasEditable_(clock.isUserEditable())
  {
  }
  virtual ~ScopedUserEditableWatch()
  {
    if (wasEditable_ != clock_.isUserEditable())
      clock_.notifyUserEditable_(!wasEditable_);
  }
private:
  simCore::ClockImpl& clock_;
  bool wasEditable_;
};


ClockImpl::ClockImpl()
  : currentTime_(simCore::MIN_TIME_STAMP),
    beginTime_(simCore::MIN_TIME_STAMP),
    endTime_(simCore::INFINITE_TIME_STAMP),
    canLoop_(true),
    mode_(Clock::MODE_STEP),
    direction_(simCore::FORWARD),
    isPlaying_(false),
    disabled_(false),
    realScale_(1.0),
    stepScale_(0.1)
{
}

ClockImpl::~ClockImpl()
{
}

bool ClockImpl::controlsDisabled() const
{
  return disabled_;
}

bool ClockImpl::isUserEditable() const
{
  return !(controlsDisabled() || endTime() == simCore::INFINITE_TIME_STAMP || isLiveMode_(mode()));
}

void ClockImpl::setControlsDisabled(bool fl)
{
  ScopedUserEditableWatch userEditableWatch(*this);
  disabled_ = fl;
}

Clock::Mode ClockImpl::mode() const
{
  return mode_;
}

bool ClockImpl::isLiveMode() const
{
  return isLiveMode_(mode_);
}

bool ClockImpl::realTime() const
{
  return mode_ == Clock::MODE_REALTIME || mode_ == Clock::MODE_FREEWHEEL || mode_ == Clock::MODE_SIMULATION;
}

void ClockImpl::setRealTime(bool fl)
{
  setMode(fl ? Clock::MODE_REALTIME : Clock::MODE_STEP);
}

simCore::TimeStamp ClockImpl::currentTime() const
{
  return currentTime_;
}

double ClockImpl::timeScale() const
{
  return realTime() ? realScale_ : stepScale_;
}

void ClockImpl::setTimeScale(double scale)
{
  if (scale < 0.0) // < 0.0 is invalid; 0.0 indicates time stands still
    return;
  double oldScale = timeScale(); // returns the real or step scale as appropriate
  if (oldScale == scale)
    return;
  if (realTime())
    realScale_ = scale;
  else
    stepScale_ = scale;
  restartRTClock_(currentTime());
  notifyScaleChange_(scale);
}

simCore::TimeStamp ClockImpl::startTime() const
{
  return beginTime_;
}

simCore::TimeStamp ClockImpl::endTime() const
{
  return endTime_;
}

void ClockImpl::setStartTime(const simCore::TimeStamp& timeVal)
{
  if (timeVal == startTime())
    return;

  // Simulation mode always has the minimum begin time
  if (mode() == Clock::MODE_SIMULATION)
    beginTime_ = MIN_TIME_STAMP;
  else
    beginTime_ = timeVal;
  notifyBoundsChange_(startTime(), endTime());
  setTime(clamp_(currentTime()));
}

void ClockImpl::setEndTime(const simCore::TimeStamp& timeVal)
{
  if (timeVal == endTime())
    return;

  ScopedUserEditableWatch userEditableWatch(*this);

  // Simulation mode always has the maximum end time
  if (mode() == Clock::MODE_SIMULATION)
    endTime_ = INFINITE_TIME_STAMP;
  else
    endTime_ = timeVal;
  notifyBoundsChange_(startTime(), endTime());
  setTime(clamp_(currentTime()));
}

void ClockImpl::setCanLoop(bool fl)
{
  if (fl != canLoop_)
  {
    canLoop_ = fl;
    notifyCanLoopChange_(fl);
  }
}

bool ClockImpl::canLoop() const
{
  // Don't let freewheel mode loop
  if (mode() == Clock::MODE_FREEWHEEL)
    return false;
  return canLoop_;
}

bool ClockImpl::isPlaying() const
{
  return isPlaying_;
}

void ClockImpl::setMode(Clock::Mode newMode)
{
  setMode(newMode, currentTime());
}

void ClockImpl::setMode(Clock::Mode newMode, const simCore::TimeStamp& liveStartTime)
{
  // Avoid no-op, unless we're in freewheel mode.  Freewheel allows double setMode() due
  // to the difference of the fwStartTime
  Clock::Mode oldMode = mode();
  if (newMode == oldMode && newMode != simCore::Clock::MODE_FREEWHEEL)
    return;
  double oldTimeScale = timeScale();
  // Note that mode changes can affect whether user can edit time (e.g. freewheel to step mode)
  ScopedUserEditableWatch userEditableWatch(*this);
  mode_ = newMode;

  // Test if we went from live mode to non-live mode; special processing
  if (isLiveMode_(oldMode) && !isLiveMode_(newMode))
  {
    setCanLoop(true);
    direction_ = simCore::FORWARD;
    stop();
  }

  // Freewheel needs to start time moving
  if (newMode == Clock::MODE_FREEWHEEL)
  {
    setStartTime(liveStartTime);
    setEndTime(liveStartTime);
    setTime(liveStartTime);
    playForward();
  }
  else if (newMode == Clock::MODE_SIMULATION)
  {
    setStartTime(MIN_TIME_STAMP);
    setEndTime(INFINITE_TIME_STAMP);
    setTime(liveStartTime);
    realScale_ = 0.0;
    playForward();
  }

  // If leaving simulation mode, set the real scale back to 1.0
  if (oldMode == Clock::MODE_SIMULATION)
    realScale_ = 1.0;

  notifyModeChange_(newMode);
  // Detect a change in time scale and send a message if needed
  if (timeScale() != oldTimeScale)
    notifyScaleChange_(timeScale());

  if (newMode == Clock::MODE_REALTIME)
    restartRTClock_(currentTime());
}

simCore::TimeDirection ClockImpl::timeDirection() const
{
  if (!isPlaying())
    return simCore::STOP;
  return direction_;
}

void ClockImpl::setTime(const simCore::TimeStamp& timeVal)
{
  setTime_(timeVal, true);
}

void ClockImpl::setTime_(const simCore::TimeStamp& timeVal, bool isJump)
{
  // If we're in freewheel mode, don't set the time unless we pass a threshold
  static const double FREEWHEEL_THRESHOLD = 0.1;
  if (mode() == Clock::MODE_FREEWHEEL && fabs(timeVal - currentTime()) < FREEWHEEL_THRESHOLD)
    return;
  restartRTClock_(timeVal);
  setTimeNoThresholdCheck_(timeVal, isJump);
}

void ClockImpl::setTimeNoThresholdCheck_(const simCore::TimeStamp& timeVal, bool isJump)
{
  const simCore::TimeStamp& newTime = clamp_(timeVal);
  if (newTime != currentTime_)
  {
    currentTime_ = newTime;
    notifySetTime_(newTime, isJump);
  }
}

void ClockImpl::decreaseScale()
{
  setTimeScale(simCore::getNextTimeStep(false, timeScale()));
}

void ClockImpl::stepBackward()
{
  // Cannot step backward in live mode
  if (isLiveMode())
    return;
  stop();
  subtractFromTime_(timeScale());
}

void ClockImpl::playReverse()
{
  // Cannot play backwards in freewheel mode
  if (mode_ == MODE_FREEWHEEL)
    return;
  if (!isPlaying_ || direction_ != simCore::REVERSE)
  {
    isPlaying_ = true;
    direction_ = simCore::REVERSE;
    restartRTClock_(currentTime());
    notifyDirectionChange_(simCore::REVERSE);
  }
}

void ClockImpl::stop()
{
  // Only way to stop freewheel mode is to change modes
  if (mode() == Clock::MODE_FREEWHEEL || (!isPlaying_ && direction_ == simCore::STOP))
    return;
  isPlaying_ = false;
  direction_ = simCore::STOP;
  notifyDirectionChange_(simCore::STOP);
}

void ClockImpl::playForward()
{
  if (!isPlaying_ || direction_ != simCore::FORWARD)
  {
    isPlaying_ = true;
    direction_ = simCore::FORWARD;
    restartRTClock_(currentTime());
    notifyDirectionChange_(simCore::FORWARD);
  }
}

void ClockImpl::stepForward()
{
  // Cannot step forward in live mode
  if (isLiveMode())
    return;
  stop();
  addToTime_(timeScale());
}

void ClockImpl::increaseScale()
{
  setTimeScale(simCore::getNextTimeStep(true, timeScale()));
}

// per-frame callback, advance time as needed
void ClockImpl::idle()
{
  if (!isPlaying_)
    return;

  // Stop should not be allowed
  assert(direction_ != simCore::STOP);
  if (direction_ == simCore::FORWARD)
  {
    if (mode() == Clock::MODE_STEP)
      addToTime_(stepScale_);
    else
    {
      bool timeJumped = false;
      simCore::TimeStamp newTime = simCore::TimeStamp(beginTime_.referenceYear(), clock_.getTime());
      // Allow observers to adjust the time
      notifyAdjustTime_(currentTime_, newTime);
      currentTime_ = newTime;

      if (mode() == Clock::MODE_FREEWHEEL)
      {
        // Freewheel never loops, but does increase the end time...
        if (currentTime_ > endTime())
          setEndTime(currentTime_);
      }
      else
      {
        // The !isPlayback at the start of the routine and the if (mode()==... should cause the else to be only for real-time
        assert(mode() == Clock::MODE_REALTIME || mode() == Clock::MODE_SIMULATION);
        if (currentTime_ > endTime_)
        {
          if (canLoop())
          {
            currentTime_ = beginTime_;
            restartRTClock_(currentTime());
            timeJumped = true;
          }
          else
          {
            currentTime_ = endTime_;
            stop();
          }
        }
      }
      notifySetTime_(currentTime_, timeJumped);
    }
  }
  else
  {
    if (mode() == Clock::MODE_STEP)
      subtractFromTime_(stepScale_);
    else
      subtractFromTime_(clock_.getDeltaTime() * realScale_);
  }
}

simCore::TimeStamp ClockImpl::clamp_(const simCore::TimeStamp& val) const
{
  if (val < beginTime_)
    return beginTime_;
  // Let freewheel mode go past the end time for clamping purposes
  if (val > endTime_ && mode() != Clock::MODE_FREEWHEEL)
    return endTime_;
  return val;
}

bool ClockImpl::isLiveMode_(simCore::Clock::Mode mode) const
{
  // Freewheel and Simulation are "live" modes, where time cannot be advanced
  return (mode == Clock::MODE_FREEWHEEL || mode == Clock::MODE_SIMULATION);
}

void ClockImpl::restartRTClock_(const simCore::TimeStamp& syncTime)
{
  clock_.stop();
  clock_.reset();
  clock_.setScale(timeScale());
  clock_.start(syncTime.secondsSinceRefYear(beginTime_.referenceYear()));
}

void ClockImpl::addToTime_(double howMuch)
{
  // To reduce accumulated error, MODE_FREEWHEEL does it own calculation and does not call addToTime_
  assert(mode() != Clock::MODE_FREEWHEEL);
  if (mode() == Clock::MODE_FREEWHEEL)
    return;

  // In some external modes (like TCS) it's feasible that a perfectly timed set-time
  // and then idle could result in a 0 howMuch -- which means we do nothing.  Assertion
  // failure here means addToTime_ was called with negative time -- instead,
  // subtractFromTime_ should have been used, so something is wrong in the call stack.
  assert(howMuch >= 0.0);
  if (howMuch <= 0) // 0.0 values are not erroneous, but do nothing
    return;

  simCore::TimeStamp newTime;
  bool jump = false;
  if (currentTime_ >= endTime_)
  {
    if (canLoop())
    {
      newTime = beginTime_;
      jump = true;
    }
    else
      stop(); // Note: setTime..() takes care of clamping
  }
  else
  {
    newTime = currentTime_ + howMuch;
    if (newTime > endTime_)
      newTime = endTime_;

    // Allow observers to adjust the time
    notifyAdjustTime_(currentTime_, newTime);
  }

  setTimeNoThresholdCheck_(newTime, jump);
}

void ClockImpl::notifyAdjustTime_(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime)
{
  // first pass only support playing forward; when/if needed a future pass could add playing backwards
  assert(direction_ != REVERSE);

  // Algorithm is "smallest change wins"
  for (std::vector<Clock::TimeObserverPtr>::const_iterator i = timeObservers_.begin(); i != timeObservers_.end(); ++i)
  {
    simCore::TimeStamp verifyTime = newTime;
    (*i)->adjustTime(oldTime, verifyTime);
    // only accept a small adjustment to the time
    if ((verifyTime > oldTime) && (verifyTime < newTime))
      newTime = verifyTime;
  }
}

void ClockImpl::subtractFromTime_(double howMuch)
{
  assert(howMuch >= 0.0);

  simCore::TimeStamp newTime;
  bool jump = false;
  if (currentTime_ <= beginTime_)
  {
    if (canLoop())
    {
      newTime = endTime_;
      jump = true;
    }
    else
      stop(); // Note: setTime..() takes care of clamping
  }
  else
  {
    newTime = currentTime_ - howMuch;
    if (newTime < beginTime_)
      newTime = beginTime_;
  }

  // Note: Threshold check shouldn't matter, because this shouldn't
  // be called when threshold does matter (freewheel mode)
  setTime_(newTime, jump);
}

void ClockImpl::registerTimeCallback(Clock::TimeObserverPtr p)
{
  std::vector<Clock::TimeObserverPtr>::const_iterator i = std::find(timeObservers_.begin(), timeObservers_.end(), p);
  if (i == timeObservers_.end())
    timeObservers_.push_back(p);
}

void ClockImpl::removeTimeCallback(Clock::TimeObserverPtr p)
{
  std::vector<Clock::TimeObserverPtr>::iterator i = std::find(timeObservers_.begin(), timeObservers_.end(), p);
  if (i != timeObservers_.end())
    timeObservers_.erase(i);
}

void ClockImpl::registerModeChangeCallback(Clock::ModeChangeObserverPtr p)
{
  std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = std::find(modeChangeObservers_.begin(), modeChangeObservers_.end(), p);
  if (i == modeChangeObservers_.end())
    modeChangeObservers_.push_back(p);
}

void ClockImpl::removeModeChangeCallback(Clock::ModeChangeObserverPtr p)
{
  std::vector<Clock::ModeChangeObserverPtr>::iterator i = std::find(modeChangeObservers_.begin(), modeChangeObservers_.end(), p);
  if (i != modeChangeObservers_.end())
    modeChangeObservers_.erase(i);
}

void ClockImpl::notifySetTime_(const simCore::TimeStamp& newTime, bool isJump)
{
  for (std::vector<Clock::TimeObserverPtr>::const_iterator i = timeObservers_.begin(); i != timeObservers_.end(); ++i)
    (*i)->onSetTime(newTime, isJump);
}

void ClockImpl::notifyTimeLoop_()
{
  for (std::vector<Clock::TimeObserverPtr>::const_iterator i = timeObservers_.begin(); i != timeObservers_.end(); ++i)
    (*i)->onTimeLoop();
}

void ClockImpl::notifyModeChange_(Clock::Mode newMode)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onModeChange(newMode);
}

void ClockImpl::notifyDirectionChange_(simCore::TimeDirection newDir)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onDirectionChange(newDir);
}

void ClockImpl::notifyScaleChange_(double newScale)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onScaleChange(newScale);
}

void ClockImpl::notifyBoundsChange_(const simCore::TimeStamp& start, const simCore::TimeStamp& end)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onBoundsChange(start, end);
}

void ClockImpl::notifyCanLoopChange_(bool canLoop)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onCanLoopChange(canLoop);
}

void ClockImpl::notifyUserEditable_(bool newUserEditable)
{
  for (std::vector<Clock::ModeChangeObserverPtr>::const_iterator i = modeChangeObservers_.begin(); i != modeChangeObservers_.end(); ++i)
    (*i)->onUserEditableChanged(newUserEditable);
}

}
