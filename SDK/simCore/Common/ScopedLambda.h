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

#ifndef SIMCORE_SCOPED_LAMBDA_H
#define SIMCORE_SCOPED_LAMBDA_H

#include <functional>

namespace simCore
{
  /** Scoped class that executes a lambda function on destruction. */
  struct ScopedLambda
  {
    explicit ScopedLambda(const std::function<void()>& onDestroy)
      : onDestroy_(onDestroy)
    {
    }
    ~ScopedLambda()
    {
      onDestroy_();
    }

    const std::function<void()> onDestroy_;
  };
}

#endif /* SIMCORE_SCOPED_LAMBDA_H */
