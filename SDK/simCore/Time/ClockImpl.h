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
#ifndef SIMCORE_TIME_CLOCKIMPL_H
#define SIMCORE_TIME_CLOCKIMPL_H

#include <vector>
#include <map>

#include "simCore/Time/TimeClock.h"
#include "simCore/Time/Clock.h"

namespace simCore
{

  /** Implementation of clock controls (play rate, start/stop, etc) */
  class SDKCORE_EXPORT ClockImpl : public Clock
  {
  public:
    ClockImpl();
    virtual ~ClockImpl();

    /**@name Accessors
    *@{
    */
    virtual Clock::Mode mode() const;
    virtual bool isLiveMode() const;
    virtual simCore::TimeStamp currentTime() const;
    virtual double timeScale() const;
    virtual bool realTime() const;
    virtual simCore::TimeStamp startTime() const;
    virtual simCore::TimeStamp endTime() const;
    virtual bool canLoop() const;
    virtual bool isPlaying() const;
    virtual simCore::TimeDirection timeDirection() const;
    virtual bool controlsDisabled() const;
    virtual bool isUserEditable() const;

    virtual void setMode(Clock::Mode mode);
    virtual void setMode(Clock::Mode mode, const simCore::TimeStamp& liveStartTime);
    virtual void setTime(const simCore::TimeStamp& timeVal);
    virtual void setTimeScale(double scale);
    virtual void setRealTime(bool fl);
    virtual void setStartTime(const simCore::TimeStamp& timeVal);
    virtual void setEndTime(const simCore::TimeStamp& timeVal);
    virtual void setCanLoop(bool fl);
    virtual void setControlsDisabled(bool fl);
    ///@}

    /**@name controls
    *@{
    */
    virtual void decreaseScale();
    virtual void stepBackward();
    virtual void playReverse();
    virtual void stop();
    virtual void playForward();
    virtual void stepForward();
    virtual void increaseScale();
    ///@}

    /**@name callback management
    *@{
    */
    virtual void registerTimeCallback(Clock::TimeObserverPtr np);
    virtual void removeTimeCallback(Clock::TimeObserverPtr np);

    virtual void registerModeChangeCallback(Clock::ModeChangeObserverPtr p);
    virtual void removeModeChangeCallback(Clock::ModeChangeObserverPtr p);
    ///@}

    /// Called to update the current data time
    void idle();

  private:
    /// Performs a freewheel threshold check, then goes to the time requested
    void setTime_(const simCore::TimeStamp& timeVal, bool isJump);
    /// Just like setTime_(), except no freewheel-threshold check
    void setTimeNoThresholdCheck_(const simCore::TimeStamp& timeVal, bool isJump);
    /// Returns true in live (freewheel/simulation) mode
    bool isLiveMode_(Clock::Mode mode) const;
    /// Returns a value >= begin and <= end
    simCore::TimeStamp clamp_(const simCore::TimeStamp& val) const;
    /// Restarts the wall clock wrt the syncTime; this routine should be called BEFORE doing callbacks to reduce time skew
    void restartRTClock_(const simCore::TimeStamp& syncTime);
    /// Increments the current time by a given number of seconds
    void addToTime_(double howMuch);
    /// Decrements the current time by a given number of seconds
    void subtractFromTime_(double howMuch);

    // Observer notification functions
    void notifySetTime_(const simCore::TimeStamp& newTime, bool isJump);
    void notifyTimeLoop_();
    void notifyModeChange_(Clock::Mode newMode);
    void notifyDirectionChange_(simCore::TimeDirection newDir);
    void notifyScaleChange_(double newScale);
    void notifyBoundsChange_(const simCore::TimeStamp& start, const simCore::TimeStamp& end);
    void notifyCanLoopChange_(bool canLoop);
    void notifyUserEditable_(bool newUserEditable);
    void notifyAdjustTime_(const simCore::TimeStamp& oldTime, simCore::TimeStamp& newTime);

    /// current time in the simulation
    TimeStamp currentTime_;
    /// earliest permitted time
    TimeStamp beginTime_;
    /// latest permitted time
    TimeStamp endTime_;

    /// Can time loop around?
    bool canLoop_;
    /// Current clock mode
    Clock::Mode mode_;
    /// Can only be forward, reverse, or stop
    simCore::TimeDirection direction_;
    /// Turned on when the clock is playing, off when stopped
    bool isPlaying_;
    /// Disabled flag, fed by plug-ins
    bool disabled_;
    /// Real-time scalar value
    double realScale_;
    /// Step mode step value
    double stepScale_;
    /// Wall clock for keeping time in real-time modes
    simCore::TimeClock clock_;

    /// List of all observers interested in time value changes
    std::vector<Clock::TimeObserverPtr> timeObservers_;
    /// List of all observers interested in changes not related to time value directly (mode, loop, etc.)
    std::vector<Clock::ModeChangeObserverPtr> modeChangeObservers_;

    /// Utility class to send out notifications when user's permission to edit changes
    class ScopedUserEditableWatch;
  };

}

#endif /* SIMCORE_TIME_CLOCKIMPL_H */
