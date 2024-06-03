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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */

#ifndef SIMCORE_SCOPEGUARD_H
#define SIMCORE_SCOPEGUARD_H

#include <functional>

namespace simCore
{
  /** Scoped class that executes a lambda function on destruction. */
  struct ScopeGuard
  {
    explicit ScopeGuard(const std::function<void()>& onDestroy)
      : onDestroy_(onDestroy)
    {
    }
    ~ScopeGuard()
    {
      onDestroy_();
    }

    const std::function<void()> onDestroy_;
  };
}

#endif /* SIMCORE_SCOPEGUARD_H */
