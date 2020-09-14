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
#include <cassert>
#include <vector>
#include <algorithm>

#include "simNotify/Notify.h"
#include "simNotify/NotifyHandler.h"
#include "simNotify/NullNotifyHandler.h"
#include "simNotify/StandardNotifyHandlers.h"

namespace simNotify
{

  namespace
  {
    /// Default severity limit is to show notice and more important
    const NotifySeverity defaultSeverityLimit_ = simNotify::NOTIFY_NOTICE;

    /// The default notification handler
    NotifyHandlerPtr defaultNotifyHandler_(new StandardNotifyHandler());

    /// NULL object for notify handlers
    NotifyHandlerPtr nullNotifyHandler_(new NullNotifyHandler());

    /// This is the last item in the 0 based list of enumerations; possible problem keeping this in sync with the enum
    const unsigned int numLevels_ = simNotify::NOTIFY_DEBUG_FP + 1;
  }

  /**
   * Defines a context for notifications that can be passed around and shared between
   * modules in a single application.  This encapsulates the values required for notifying
   * to a consistent target with a consistent severity.  This is only required if the SDK
   * is built as a static library and is linked to DLLs in addition to the main application.
   */
  class NotifyContext
  {
  public:
    /** Default constructor */
    NotifyContext()
      : severityLimit_(defaultSeverityLimit_),
        handlers_(numLevels_, defaultNotifyHandler_)
    {
      initNotifyLevel_();
    }

    /** Retrieves the current severity threshold */
    NotifySeverity severityLimit() const
    {
      return severityLimit_;
    }

    /** Changes the severity threshold */
    void setSeverityLimit(NotifySeverity severity)
    {
      severityLimit_ = severity;
    }

    /** Retrieves the handler for the given severity, never nullptr */
    NotifyHandlerPtr handler(NotifySeverity severity) const
    {
      assert(severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP);
      if (!(severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP))
        return nullNotifyHandler_;
      return handlers_[severity];
    }

    /** Changes the handler for a single severity */
    void setHandler(NotifySeverity severity, NotifyHandlerPtr handler)
    {
      assert(severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP);
      if (severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP)
      {
        // Avoid nullptr assignments
        if (handler == nullptr)
          handler = nullNotifyHandler_;
        handlers_[severity] = handler;
      }
    }

    /** Changes the handler for all severities */
    void setNotifyHandlers(NotifyHandlerPtr handler)
    {
      assert(handler != nullptr);
      if (handler == nullptr)
        return;
      for (std::vector<NotifyHandlerPtr>::iterator i = handlers_.begin(); i != handlers_.end(); ++i)
        *i = handler;
    }

  private:
    /// Threshold for severity printing
    NotifySeverity severityLimit_;
    /// Handler for each severity level
    std::vector<NotifyHandlerPtr> handlers_;

    /** Initializes the notify level */
    void initNotifyLevel_()
    {
      severityLimit_ = defaultSeverityLimit_;

      const char* SDKNOTIFYLEVEL = getenv("SDK_NOTIFY_LEVEL");

      if (!SDKNOTIFYLEVEL)
        SDKNOTIFYLEVEL = getenv("SDKNOTIFYLEVEL");

      if (SDKNOTIFYLEVEL)
      {
        severityLimit_ = stringToSeverity(SDKNOTIFYLEVEL);
      }

    }
  };

  namespace
  {
    /// Contains the default notification context; automatic destruction
    NotifyContext defaultNotifyContext_;

    /// Points to the default notification context, but could change; requires no destruction
    NotifyContext* notifyContext_ = &defaultNotifyContext_;
  }


  NotifySeverity defaultNotifyLevel()
  {
    return defaultSeverityLimit_;
  }

  NotifySeverity notifyLevel()
  {
    return notifyContext_->severityLimit();
  }


  void setNotifyLevel(NotifySeverity severity)
  {
    assert(severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP);
    notifyContext_->setSeverityLimit(severity);
  }

  bool isNotifyEnabled(NotifySeverity severity)
  {
    return severity <= notifyContext_->severityLimit();
  }

  NotifyHandlerPtr defaultNotifyHandler()
  {
    return defaultNotifyHandler_;
  }

  NotifyHandlerPtr nullNotifyHandler()
  {
    return nullNotifyHandler_;
  }

  NotifyHandlerPtr notifyHandler(NotifySeverity severity)
  {
    NotifyHandlerPtr rv = notifyContext_->handler(severity);
    if (rv)
      rv->setSeverity(severity);
    return rv;
  }

  void setNotifyHandler(NotifySeverity severity, NotifyHandlerPtr handler)
  {
    return notifyContext_->setHandler(severity, handler);
  }

  void setNotifyHandlers(NotifyHandlerPtr handler)
  {
    notifyContext_->setNotifyHandlers(handler);
  }

  NotifyHandler &notify(NotifySeverity severity)
  {
    assert(severity >= simNotify::NOTIFY_ALWAYS && severity <= simNotify::NOTIFY_DEBUG_FP);
    if (isNotifyEnabled(severity))
    {
      NotifyHandlerPtr handler = notifyContext_->handler(severity);
      if (handler != nullptr)
      {
        handler->setSeverity(severity);
        handler->notifyPrefix();
        return *handler;
      }
    }

    // Return the null handler
    return *nullNotifyHandler_;
  }

  std::string severityToString(NotifySeverity severity)
  {
    switch (severity)
    {
    case NOTIFY_ALWAYS:
      return "ALWAYS";
    case NOTIFY_FATAL:
      return "FATAL";
    case NOTIFY_ERROR:
      return "ERROR";
    case NOTIFY_WARN:
      return "WARN";
    case NOTIFY_NOTICE:
      return "NOTICE";
    case NOTIFY_INFO:
      return "INFO";
    case NOTIFY_DEBUG_INFO:
      return "DEBUG_INFO";
    case NOTIFY_DEBUG_FP:
      return "DEBUG_FP";
    }
    return "UNKNOWN";
  }

  NotifySeverity stringToSeverity(const std::string &in)
  {
    // convert to upper case prior to comparison
    std::string val;
    val.resize(in.size());
    std::transform(in.begin(), in.end(), val.begin(), ::toupper);

    if (val == "ALWAYS")
      return NOTIFY_ALWAYS;
    if (val == "FATAL")
      return NOTIFY_FATAL;
    if (val == "ERROR")
      return NOTIFY_ERROR;
    if (val == "WARN")
      return NOTIFY_WARN;
    if (val == "INFO")
      return NOTIFY_INFO;
    if (val == "DEBUG_INFO")
      return NOTIFY_DEBUG_INFO;
    if (val == "DEBUG_FP")
      return NOTIFY_DEBUG_FP;

    // fall through to notice
    return NOTIFY_NOTICE;
  }

  NotifyContext* const notifyContext()
  {
    return notifyContext_;
  }

  void installNotifyContext(NotifyContext* const context)
  {
    if (context != nullptr)
      notifyContext_ = context;
  }

}
