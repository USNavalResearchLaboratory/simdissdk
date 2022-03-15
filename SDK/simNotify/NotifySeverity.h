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
#ifndef SIMNOTIFY_NOTIFYSEVERITY
#define SIMNOTIFY_NOTIFYSEVERITY

#include <string>
#include "simCore/Common/Common.h"

namespace simNotify
{

  /**
  * @ingroup Notify
  * @brief Notification system's severity levels
  *
  * Notification system's severity levels defined as a hierarchy where
  * levels with smaller numbers are of higher importance.  The notification
  * system keeps track of a configurable minimum notification level that is used to
  * suppress notification's associated with severity levels of lesser importance.
  * The ALWAYS severity level will never be suppressed.
  *
  * @see setNotifyLevel()
  */
  enum NotifySeverity
  {
    NOTIFY_ALWAYS=0,    ///< Notification is always logged; cannot be suppressed.
    NOTIFY_FATAL,       ///< Notification level for fatal errors.
    NOTIFY_ERROR,       ///< Notification level for non-fatal errors.
    NOTIFY_WARN,        ///< Notification level for non-fatal warnings.
    NOTIFY_NOTICE,      ///< Notification level for application specific information which provides instruction to the user and may require user response.
    NOTIFY_INFO,        ///< Notification level for general purpose information.
    NOTIFY_DEBUG_INFO,  ///< Notification level for debugging information.
    NOTIFY_DEBUG_FP     ///< Notification level for reporting file path information related to the search paths for resource files and plug-ins.
  };

  /** Returns a string representation of the notification severity */
  SDKNOTIFY_EXPORT std::string severityToString(NotifySeverity severity);

  /** Returns notification severity based on incoming string, NOTIFY_NOTICE is returned if no match is made */
  SDKNOTIFY_EXPORT NotifySeverity stringToSeverity(const std::string &in);

}

#endif /* SIMNOTIFY_NOTIFYSEVERITY */
