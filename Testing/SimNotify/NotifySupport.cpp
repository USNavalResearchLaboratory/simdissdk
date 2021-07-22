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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "NotifySupport.h"

using namespace NotifyTestSupport;

// Counter to ensure that the pointer is destroyed properly based on scope
int StringStreamNotify::g_StringStreamDestructions = 0;

StringStreamNotify::StringStreamNotify()
{
}

StringStreamNotify::~StringStreamNotify()
{
  g_StringStreamDestructions++;
}

// Prefix looks like [Date] [Time] [WARN]
void StringStreamNotify::notifyPrefix()
{
  std::stringstream ss;
  ss << timeStampString_() << " [" << severityString_() << "] ";
  lastLine_ = ss.str();
  stream_ << lastLine_;
}

void StringStreamNotify::notify(const std::string &message)
{
  lastLine_ += message;
  stream_ << message;
}

void StringStreamNotify::clear()
{
  lastLine_ = "";
  stream_.str("");
}

// Retrieve the last message sent to notification
std::string StringStreamNotify::lastLine() const
{
  return lastLine_;
}
// Retrieve all messages sent to notification
std::string StringStreamNotify::allLines() const
{
  return stream_.str();
}

std::string StringStreamNotify::timeStampString_() const
{
  // Don't need anything fancy for testing purposes
  return "[Date] [Time]";
}

std::string StringStreamNotify::severityString_() const
{
  switch (severity())
  {
  case simNotify::NOTIFY_ALWAYS:
    return "ALWAYS";
  case simNotify::NOTIFY_FATAL:
    return "FATAL";
  case simNotify::NOTIFY_ERROR:
    return "ERROR";
  case simNotify::NOTIFY_WARN:
    return "WARN";
  case simNotify::NOTIFY_NOTICE:
    return "NOTICE";
  case simNotify::NOTIFY_INFO:
    return "INFO";
  case simNotify::NOTIFY_DEBUG_INFO:
    return "DEBUG_INFO";
  case simNotify::NOTIFY_DEBUG_FP:
    return "DEBUG_FP";
  }
  return "UNKNOWN";
}

