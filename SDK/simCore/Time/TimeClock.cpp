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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cmath>
#include "simCore/Time/Utils.h"
#include "simCore/Time/TimeClock.h"

namespace simCore
{

  /** Performs default initialization of TimeClock data members */
  TimeClock::TimeClock()
    : scale_(1.),
      reference_(-1.),
      start_(0.0),
      state_(STATE_RESET)
  {
  }

  /** No dynamic memory allocation, therefore no action here... */
  TimeClock::~TimeClock()
  {
  }

  ///Returns the last error string
  std::string TimeClock::getLastError() const
  {
    return lastError_;
  }

  ///Resets the last error
  void TimeClock::resetLastError()
  {
    lastError_.clear();
  }

  bool TimeClock::isStarted() const
  {
    return state_ == STATE_RUNNING;
  }

  /// TimeClock getTime.
  double TimeClock::getTime() const
  {
    if (state_ == STATE_RESET)
    {
      lastError_ = "getTime()  TimeClock has not been started";
      return 0;
    }
    if (state_ == STATE_RUNNING)
      return ((simCore::getSystemTime() - reference_) * scale_) + start_;
    if (state_ == STATE_PAUSED)
      return start_;
    return 0;
  }

  /// TimeClock getDeltaTime.
  double TimeClock::getDeltaTime() const
  {
    if (state_ == STATE_RESET)
    {
      lastError_ = "getDeltaTime()  TimeClock has not been started";
      return 0;
    }
    return simCore::getSystemTime() - reference_;
  }

  /// TimeClock setTime.
  void TimeClock::setTime(double newtime)
  {
    if (state_ == STATE_RUNNING)
    {
      lastError_ = "setTime()  TimeClock must be stopped in order to set time";
      return;
    }
    start_ = newtime;
  }

  /// TimeClock start.
  void TimeClock::start()
  {
    reference_ = simCore::getSystemTime();
    state_ = STATE_RUNNING;
  }

  /// TimeClock start.
  void TimeClock::start(double newtime)
  {
    start_ = newtime;
    reference_ = simCore::getSystemTime();
    state_ = STATE_RUNNING;
  }

  /// TimeClock stop.
  void TimeClock::stop()
  {
    if (state_ == STATE_RUNNING)
    {
      start_ = getTime();
      state_ = STATE_PAUSED;
    }
  }

  /// TimeClock reset.
  void TimeClock::reset()
  {
    state_ = STATE_RESET;
    start_ = 0.0;
    reference_ = -1.;
  }

  void TimeClock::setScale(double scl, bool force)
  {
    if ((state_ == STATE_RUNNING) && (scl != scale_))
    {
      if (force)
      {
        start_ = getTime();
        reference_ = simCore::getSystemTime();
      }
      else
      {
        lastError_ = "setScale()  TimeClock must be stopped in order to set scale";
        return;
      }
    }
    scale_ = scl;
  }

  double TimeClock::getScale() const
  {
    return scale_;
  }

}
