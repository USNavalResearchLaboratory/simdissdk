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
#include "simCore/Time/Utils.h"
#include "simCore/Time/CountDown.h"

namespace simCore
{

CountDown::CountDown(unsigned int waitMilliseconds)
 : waitMilliseconds_(waitMilliseconds),
   done_(waitMilliseconds == 0),
   startTime_(waitMilliseconds == 0 ? 0.0 : simCore::getSystemTime())
{
}

CountDown::~CountDown()
{
}

bool CountDown::isDone() const
{
  if (done_)
    return true;

  const double deltaTime = simCore::getSystemTime() - startTime_;

  // if current time is before start (must be due to system time change), or
  // delta time (scale seconds to milliseconds) is more than the desired wait time
  if (deltaTime < 0 || ((deltaTime * 1000.0) > waitMilliseconds_))
  {
    done_ = true;
    return true;
  }

  return false;
}

}
