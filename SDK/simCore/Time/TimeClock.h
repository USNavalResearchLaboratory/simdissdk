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
#ifndef SIMCORE_TIME_TIMECLOCK_H
#define SIMCORE_TIME_TIMECLOCK_H

#include <string>
#include "simCore/Common/Common.h"
#include "simCore/Time/TimeClass.h"

namespace simCore
{

  /// A top level class for implementing a scalable wall clock.
  /**
  * The simCore::TimeClock class facilitates the creation and setup of a scalable
  * clock for basic timing needs.
  *
  * For a more complete clock implementation that includes SIMDIS-like VCR
  * controls, consider looking at the Clock/ClockImpl classes, which use
  * this under the hood.
  */
  class SDKCORE_EXPORT TimeClock
  {
  public:
    /// simCore::TimeClock constructor.
    TimeClock();
    /// simCore::TimeClock destructor.
    ~TimeClock();

    ///Returns the last error string
    std::string getLastError() const;
    ///Resets the last error
    void resetLastError();

    ///Returns a flag indicating that getTime(), and getDeltaTime() are valid
    bool isStarted() const;

    /**
    * Returns the elapsed time of the total amount of time
    * clock has been in play mode (state_ = STATE_RUNNING).  Scaling is taken into effect.
    * A real-time delta (getSystemTime() - reference_) value is added to the offset
    * to maintain the current time.  If simCore::TimeClock was never started, then 0.0 is returned.
    * @return a Seconds instance representing the scaled time since simCore::TimeClock was started
    */
    Seconds getTime() const;

    /**
    * Returns elapsed seconds since simCore::TimeClock was started for the first time.
    * If simCore::TimeClock was never started, then 0.0 is returned.
    * @return a double representing the elapsed seconds since simCore::TimeClock was started
    */
    double getDeltaTime() const;

    /**
    * Sets the offset time.  This value is the starting time basis for the simCore::TimeClock
    * @param newtime a Seconds instance specifying the simCore::TimeClock's start time.
    */
    void setTime(const Seconds& newtime);

    /**
    * Once this method is executed, simCore::TimeClock stores the initial time of
    * day.  This value is stored in order to maintain a
    * reference time, from which a delta time is derived.  This
    * delta time ( gettimeofday - reference_ ) is added to the start
    * value in order to maintain a real-time counter.
    */
    void start();

    /**
    * Once this method is executed, simCore::TimeClock stores the initial time of
    * day.  This value is stored in order to maintain a
    * reference time, from which a delta time is derived.  This
    * delta time ( gettimeofday - reference_ ) is added to the start
    * value in order to maintain a real-time counter.
    * @param newtime a Seconds instance specifying the simCore::TimeClock's start time.
    */
    void start(const Seconds& newtime);

    /** Stops the clock.  The last known time is stored in start_ */
    void stop();

    /** Resets simCore::TimeClock to default values. */
    void reset();

    /**
    * Sets the simCore::TimeClock's current time scale
    * @param scl a double representing the current time scale.  Real-time is 1.
    * @param force If true, then scale is applied if clock is still going; clock automatically resets reference based on current time; useful for "reversing" time or changing scale on the fly; If false, then only sets scale when clock is stopped
    */
    void setScale(double scl, bool force=false);

    /**
    * Returns the simCore::TimeClock's current time scale
    * @return a double representing the current time scale.  Real-time is 1.
    */
    double getScale() const;

  private:

    enum State
    {
      STATE_RESET = 0,    ///< Clock state has been reset
      STATE_RUNNING = 1,  ///< Clock state is running
      STATE_PAUSED = 2    ///< Clock state is paused
    };

    double scale_;                  /**< clock time scale (1. == real time) */
    double reference_;              /**< time clock was last started (sec) */
    Seconds start_;                  /**< length of time in play mode (state_ == STATE_RUNNING) */
    State state_;                   /**< current clock state */
    mutable std::string lastError_; /**< Last error string; might be changed in const functions */
  };

}

#endif /* SIMCORE_TIME_TIMECLOCK_H */
