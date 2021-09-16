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
#include "simQt/ConsoleChannel.h"

namespace simQt {

ChannelNotifyHandler::ChannelNotifyHandler()
  : usePrefix_(true)
{
}

void ChannelNotifyHandler::setChannel(ConsoleChannelPtr channel)
{
  channel_ = channel;
}

void ChannelNotifyHandler::notify(const std::string& message)
{
  if (channel_ == nullptr)
    return;

  lockMutex_();
  currentLine_ += message.c_str();
  // Only post full messages that end in newline
  if (currentLine_.endsWith('\n'))
  {
    // Strip the newline, because the console requires it; copy to new string so we can unlock mutex
    QString textToAdd = currentLine_.left(currentLine_.length() - 1);
    currentLine_.clear();

    // Unlock the mutex before calling into Qt, which might lock on its own
    unlockMutex_();
    channel_->addText(severity(), textToAdd);
  }
  else
    unlockMutex_();
}

void ChannelNotifyHandler::setUsePrefix(bool usePrefix)
{
  usePrefix_ = usePrefix;
}

bool ChannelNotifyHandler::usePrefix() const
{
  return usePrefix_;
}

void ChannelNotifyHandler::notifyPrefix()
{
  if (usePrefix())
    NotifyHandler::notifyPrefix();
}

void ChannelNotifyHandler::lockMutex_()
{
  mutex_.lock();
}

void ChannelNotifyHandler::unlockMutex_()
{
  mutex_.unlock();
}

}
