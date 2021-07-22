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
#ifndef UTILS_LOG_NOTIFY_SUPPORT_H
#define UTILS_LOG_NOTIFY_SUPPORT_H

#include "simNotify/Notify.h"

namespace NotifyTestSupport
{
// Create our own NotifyHandler to save data to a string stream for testing
class StringStreamNotify : public simNotify::NotifyHandler
{
public:
  StringStreamNotify();
  virtual ~StringStreamNotify();

  // Prefix looks like [Date] [Time] [WARN]
  virtual void notifyPrefix();

  virtual void notify(const std::string &message);

  // Retrieve the last message sent to notification
  std::string lastLine() const;

  // Retrieve all messages sent to notification
  std::string allLines() const;

  // clear the stream data
  void clear();

  // Counter to ensure that the pointer is destroyed properly based on scope
  static int g_StringStreamDestructions;

private:
  // Holds all the data in the log
  std::stringstream stream_;
  // Holds the most recent line
  std::string lastLine_;

  std::string timeStampString_() const;
  std::string severityString_() const;
};

}
#endif

