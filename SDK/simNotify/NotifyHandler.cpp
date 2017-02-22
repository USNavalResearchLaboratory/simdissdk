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
#include "simNotify/NotifyHandler.h"

using namespace simNotify;

NotifyHandler::NotifyHandler()
  : severity_(simNotify::NOTIFY_INFO)
{
}

NotifyHandler::~NotifyHandler()
{
}

void NotifyHandler::setSeverity(NotifySeverity severity)
{
  severity_ = severity;
}

NotifySeverity NotifyHandler::severity() const
{
  return severity_;
}

// To be replaced with a formatting object
void NotifyHandler::notifyPrefix()
{
  // Could add processing here for a prefix based on severity_
  switch (severity())
  {
  case simNotify::NOTIFY_ALWAYS:
    notify("ALWAYS:  ");
    break;
  case simNotify::NOTIFY_FATAL:
    notify("FATAL:  ");
    break;
  case simNotify::NOTIFY_ERROR:
    notify("ERROR:  ");
    break;
  case simNotify::NOTIFY_WARN:
    notify("WARN:  ");
    break;
  case simNotify::NOTIFY_NOTICE:
    notify("NOTICE:  ");
    break;
  case simNotify::NOTIFY_INFO:
    notify("INFO:  ");
    break;
  case simNotify::NOTIFY_DEBUG_INFO:
    notify("DEBUG_INFO:  ");
    break;
  case simNotify::NOTIFY_DEBUG_FP:
    notify("DEBUG_FP:  ");
    break;
  }
}

// std::string overload - no need to convert to string with ostringstream
NotifyHandler &NotifyHandler::operator<<(const std::string &message)
{
  notify(message);
  return *this;
}

// char * overload - no need to convert to string with ostringstream
NotifyHandler &NotifyHandler::operator<<(const char *message)
{
  notify(message);
  return *this;
}

// std::endl
NotifyHandler &NotifyHandler::operator<<(NotifyHandlerEndlFunction &)
{
  notify("\n");
  return *this;
}

// std::hex/dec/oct, etc
NotifyHandler &NotifyHandler::operator<<(NotifyHandlerManipFunction &manip)
{
  lockMutex_();
  manip(stream_);
  unlockMutex_();
  return *this;
}
