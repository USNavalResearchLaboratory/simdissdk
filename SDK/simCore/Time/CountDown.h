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
#ifndef SIMCORE_TIME_COUNTDOWN_H
#define SIMCORE_TIME_COUNTDOWN_H

#include "simCore/Common/Export.h"

namespace simCore
{

  /**
  * A timer which counts down for the specified amount of milliseconds.
  *
  * Usage:
  *   CountDown myCount(50000); // count for 50 seconds
  *
  * Periodically call myCount.isDone(): when true, count is done.
  */
  class SDKCORE_EXPORT CountDown
  {
  public:
    /** Counts down for the given number of milliseconds. */
    CountDown(unsigned int waitMilliseconds);
    virtual ~CountDown();

    ///@return true when enough time has passed
    bool isDone() const;

  private:
    unsigned int waitMilliseconds_;
    mutable bool done_;
    double startTime_;
  };

}

#endif /* SIMCORE_TIME_COUNTDOWN_H */
