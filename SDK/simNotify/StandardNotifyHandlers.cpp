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
#include <algorithm>
#include <cassert>
#include "simNotify/Notify.h"
#include "simNotify/StandardNotifyHandlers.h"

namespace simNotify {

void StandardNotifyHandler::notify(const std::string &message)
{
  if (severity() <= simNotify::NOTIFY_WARN)
  {
    fputs(message.c_str(), stderr);
  }
  else
  {
    fputs(message.c_str(), stdout);
  }
}

void PrefixedStandardNotifyHandler::notifyPrefix()
{
  std::string str = severityToString(severity());
  str += ":  ";
  notify(str);
}

void StdoutNotifyHandler::notify(const std::string &message)
{
  fputs(message.c_str(), stdout);
}

void StderrNotifyHandler::notify(const std::string &message)
{
  fputs(message.c_str(), stderr);
}

FileNotifyHandler::FileNotifyHandler(const std::string &filename)
  : file_(filename.c_str(), std::ios::out)
{
}

FileNotifyHandler::~FileNotifyHandler()
{
  file_.close();
}

void FileNotifyHandler::notify(const std::string &message)
{
  assert(isValid());
  file_ << message;
}

StreamNotifyHandler::StreamNotifyHandler(std::ostream& os)
  : os_(os)
{
}

void StreamNotifyHandler::notify(const std::string &message)
{
  if (os_.good())
    os_ << message;
}

void CaptureHandler::notifyPrefix()
{
#if __cplusplus >= 201402L
  // C++14 provides an easier way to add the severity
  lines_.push_back(NewLine{ severity(), {} });
#else
  NewLine newLine;
  newLine.severity = severity();
  lines_.push_back(newLine);
#endif
}

void CaptureHandler::notify(const std::string& message)
{
  if (lines_.empty())
  {
    // notifyPrefix() should be called before notify(), filling the vector
    assert(0);
    notifyPrefix();
  }

  auto& line = lines_.back();
  line.messages.push_back(message);
}

void CaptureHandler::clear()
{
  lines_.clear();
}

bool CaptureHandler::empty() const
{
  return lines_.empty();
}

void CaptureHandler::writeTo(simNotify::NotifyHandler& handler, bool respectNotifyLevel)
{
  // Avoid writing to self
  if (&handler == this)
    return;

  for (const auto& line : lines_)
  {
    // Skip the line if it's not enabled
    if (respectNotifyLevel && !simNotify::isNotifyEnabled(line.severity))
      continue;

    handler.setSeverity(line.severity);
    // Typically notifyPrefix() is handled by simNotify::notify(). We do it manually
    handler.notifyPrefix();
    for (const auto& message : line.messages)
      handler.notify(message);
  }
}

/**
 * Writes to the global notify handler. Call simNotify::setNotifyHandlers() or equivalent
 * before calling this. There are no options to ignore notify levels when using this function.
 */
void CaptureHandler::writeToGlobal()
{
  for (const auto& line : lines_)
  {
    // Avoid writing to self, which would invalidate lines_
    if (this == simNotify::notifyHandler(line.severity).get())
      continue;

    // Send the message over
    auto& handler = simNotify::notify(line.severity);
    for (const auto& message : line.messages)
      handler.notify(message);
  }
}

void CompositeHandler::notifyPrefix()
{
  for (const auto& handlerPtr : handlers_)
  {
    // Must set severity for each handler, since we cannot propagate it
    // due to virtual function limitations.
    handlerPtr->setSeverity(severity());
    handlerPtr->notifyPrefix();
  }
}

void CompositeHandler::notify(const std::string& message)
{
  for (const auto& handlerPtr : handlers_)
    handlerPtr->notify(message);
}

int CompositeHandler::addHandler(simNotify::NotifyHandlerPtr handler)
{
  // Do not add null handlers. Note that a NullNotifyHandler is OK
  if (!handler)
    return 1;
  // Do not add the same handler more than once
  if (std::find(handlers_.begin(), handlers_.end(), handler) != handlers_.end())
    return 1;
  handlers_.push_back(handler);
  return 0;
}

int CompositeHandler::removeHandler(simNotify::NotifyHandlerPtr handler)
{
  auto iter = std::find(handlers_.begin(), handlers_.end(), handler);
  if (iter == handlers_.end())
    return 1;
  handlers_.erase(iter);
  return 0;
}

}
