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
#include <cassert>
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

}