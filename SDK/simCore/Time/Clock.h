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
#ifndef SIMCORE_TIME_CLOCK_H
#define SIMCORE_TIME_CLOCK_H

#include <memory>
#include "simCore/Time/Constants.h"
#include "simCore/Time/TimeClass.h"

namespace simCore
{

/**
 * Abstract interface that lets users manipulate a clock using SIMDIS-like controls for increasing
 * the scale, decreasing scale, stepping forward/backward, looping, playing in forward and reverse, etc.
 */
class SDKCORE_EXPORT Clock
{
public:
  virtual ~Clock() {}

  /// Control how time advances
  enum Mode
  {
    /** File mode: Each frame steps the clock forward a given amount of time */
    MODE_STEP = 0,
    /** File mode: The clock advances at a multiplication of real-time */
    MODE_REALTIME = 1,
    /**
     * Live mode: Time advances automatically in multiple of real time, with a default
     * scale of 1.0x real time.  Setting a new clock time only applies if new time is
     * outside threshold of current time.
     */
    MODE_FREEWHEEL = 2,
    /**
     * Live mode: Time advances automatically in multiple of real time, with a default
     * scale of 0.0x real time (stopped).  Setting a new clock time always applies the
     * new value.  Start and end times have no meaning in MODE_SIMULATION mode, and
     * are forced internally to min and max timestamp values.
     */
    MODE_SIMULATION = 3
  };

  /**@name Accessors
   *@{
   */
  /** Retrieves current clock mode */
  virtual Clock::Mode mode() const = 0;
  /** Retrieves if the clock is in Live mode */
  virtual bool isLiveMode() const = 0;
  /** Retrieves current clock time */
  virtual simCore::TimeStamp currentTime() const = 0;
  /** Returns whether time is stopped, playing forward, or playing backward. */
  virtual simCore::TimeDirection timeDirection() const = 0;
  /** Returns the time step for step mode, and real-time scalar for real-time mode */
  virtual double timeScale() const = 0;
  /** Returns true in real-time mode, false in step time mode. THIS IS UNRELATED TO LIVE MODE VERSUS FILE MODE */
  virtual bool realTime() const = 0;
  /** Start of the clock bounds */
  virtual simCore::TimeStamp startTime() const = 0;
  /** End of the clock bounds */
  virtual simCore::TimeStamp endTime() const = 0;
  /** Returns true when playing past the end will loop time to the beginning */
  virtual bool canLoop() const = 0;
  /** Returns true when the clock is playing. */
  virtual bool isPlaying() const = 0;
  /** Returns true if the disable-controls flag is set (typically from Plug-in API) */
  virtual bool controlsDisabled() const = 0;
  /**
   * Returns true only if the end user should be permitted to change fields.  For example, in
   * live mode (freewheel) the user should be unable to change the current time, so this function
   * would return false.  When in step mode but controlsDisabled() is true, this function should
   * return false.  Sometimes this logic can be complicated, so this is a single stop for
   * determining whether the end user should be able to edit values.  Unlike controlsDisabled(),
   * this is a "meta" value combining other clock states.
   */
  virtual bool isUserEditable() const = 0;

  /**
  * Note that for MODE_SIMULATION start time is set to the minimum time stamp, while end time is set to the
  * maximum time stamp. For MODE_FREEWHEEL, start time and end time are both set to liveStartTime specified.
  * Sets the current time to liveStartTime as well, if mode is freewheel or simulation.
  */
  virtual void setMode(Clock::Mode mode, const simCore::TimeStamp& liveStartTime) = 0;
  virtual void setMode(Clock::Mode mode) = 0;
  virtual void setTime(const simCore::TimeStamp& timeVal) = 0;
  virtual void setTimeScale(double scale) = 0;
  /** Calling this method has the effect of changing clock mode from live to file mode */
  virtual void setRealTime(bool fl) = 0;
  /** This setter has no effect if mode is MODE_SIMULATION */
  virtual void setStartTime(const simCore::TimeStamp& timeVal) = 0;
  /** This setter has no effect if mode is MODE_SIMULATION */
  virtual void setEndTime(const simCore::TimeStamp& timeVal) = 0;
  virtual void setCanLoop(bool fl) = 0;
  virtual void setControlsDisabled(bool fl) = 0;
  ///@}

  /**@name controls
   *@{
   */
  virtual void decreaseScale() = 0;
  virtual void stepBackward() = 0;
  virtual void playReverse() = 0;
  virtual void stop() = 0;
  virtual void playForward() = 0;
  virtual void stepForward() = 0;
  virtual void increaseScale() = 0;
  ///@}

  /// Observer Class for time value changes
  class TimeObserver
  {
  public:
    virtual ~TimeObserver() {}

    /** Notification fired when time has been changed
     * @param t TimeStamp with new time
     * @param isJump True if the time is from a jump (e.g. time slider, setTime, or other out-of-band time leap).
     *    Will be false when time is reached from naturally playing forward or backward in time using VCR controls,
     *    or when using Step time to advance.
     */
    virtual void onSetTime(const TimeStamp &t, bool isJump) = 0;

    /// Time has looped
    virtual void onTimeLoop() = 0;

    /**
     * Allows the observer to provide a minor adjustment to the next new time.  This routine is only called
     * during forward play. A callback to onSetTime() will occur after this callback.
     * @param oldTime The current time
     * @param newTime The proposed new time.  The observer can only reduce the newTime, but the newTime must be greater
     *                than oldTime
     */
    virtual void adjustTime(const TimeStamp &oldTime, TimeStamp& newTime) = 0;
  };

  /// Observer class for time mode changes
  class ModeChangeObserver
  {
  public:
    virtual ~ModeChangeObserver() {}

    /**@name callbacks for each event type
     *@{
     */
    virtual void onModeChange(simCore::Clock::Mode newMode) = 0;
    virtual void onDirectionChange(simCore::TimeDirection newDirection) = 0;
    virtual void onScaleChange(double newValue) = 0;
    virtual void onBoundsChange(const simCore::TimeStamp& start, const simCore::TimeStamp& end) = 0;
    virtual void onCanLoopChange(bool newVal) = 0;
    virtual void onUserEditableChanged(bool userCanEdit) = 0;
    ///@}
  };

  /// Observer reference object
  typedef std::shared_ptr<TimeObserver> TimeObserverPtr;
  /// ModeChangeObserver reference object
  typedef std::shared_ptr<ModeChangeObserver> ModeChangeObserverPtr;

  /**@name callback management
   *@{
   */
  virtual void registerTimeCallback(TimeObserverPtr np) = 0;
  virtual void removeTimeCallback(TimeObserverPtr np) = 0;

  virtual void registerModeChangeCallback(ModeChangeObserverPtr p) = 0;
  virtual void removeModeChangeCallback(ModeChangeObserverPtr p) = 0;
  ///@}
};

}

#endif /* SIMCORE_TIME_CLOCK_H */
