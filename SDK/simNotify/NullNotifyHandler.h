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
#ifndef SIMNOTIFY_NULLNOTIFYHANDLER_H
#define SIMNOTIFY_NULLNOTIFYHANDLER_H

#include <string>

#include "simCore/Common/Common.h"
#include "simNotify/NotifyHandler.h"

namespace simNotify
{

  /**
  * @ingroup Notify
  * @brief NotifyHandler implementation for suppression of messages.
  *
  * NotifyHandler implementation for suppressing the writing of messages.
  */
  class NullNotifyHandler : public NotifyHandler
  {
  public:
    /**
    * @brief NOOP implementation of NotifyHandler::notifyPrefix().
    *
    * NOOP implementation of NotifyHandler::notifyPrefix().
    */
    virtual void notifyPrefix() { }

    /**
    * @brief NOOP implementation of NotifyHandler::notify().
    *
    * NOOP implementation of NotifyHandler::notify().
    */
    virtual void notify(const std::string &message) { }
  };

}

#endif /* SIMNOTIFY_NULLNOTIFYHANDLER_H */
