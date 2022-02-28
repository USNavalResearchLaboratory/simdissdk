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
#ifndef SIMQT_CONSOLECHANNEL_H
#define SIMQT_CONSOLECHANNEL_H

#include <memory>
#include <QString>
#include <QMutex>
#include "simNotify/NotifyHandler.h"
#include "simNotify/NotifySeverity.h"
#include "simCore/Common/Export.h"

namespace simQt {

/**
 * Handle to a console channel, returned by the ConsoleDialog::registerChannel() call.
 */
class ConsoleChannel
{
public:
  virtual ~ConsoleChannel() {}

  /**
   * Call this to notify all observers of your new text string.  When adding a new channel
   * to the console dialog, you should call this method to notify the dialog of text.
   * @param severity Severity level with which to push out text
   * @param text Text string to push out to all observers; should be completely formed (buffered) message
   */
  virtual void addText(simNotify::NotifySeverity severity, const QString& text) = 0;

  /** Returns the name of the channel */
  virtual const QString& name() const = 0;
};
typedef std::shared_ptr<ConsoleChannel> ConsoleChannelPtr;


/** simNotify::NotifyHandler that pushes data to a generic ConsoleChannel */
class SDKQT_EXPORT ChannelNotifyHandler : public simNotify::NotifyHandler
{
public:
  ChannelNotifyHandler();

  /** Changes the channel to push strings to */
  void setChannel(ConsoleChannelPtr channel);

  /** If true, then the notifyPrefix() is respected and prepended to each message */
  void setUsePrefix(bool fl);
  /** If true, then the notifyPrefix() is respected and prepended to each message */
  bool usePrefix() const;

  /** Override NotifyHandler::notify(). Is thread safe, but threaded message may introduce text mangling. */
  virtual void notify(const std::string& message);
  /** Override NotifyHandler::notifyPrefix(). Is thread safe, but threaded message may introduce text mangling. */
  virtual void notifyPrefix();

protected:
  /** @see NotifyHandler::lockMutex_() */
  virtual void lockMutex_();
  /** @see NotifyHandler::unlockMutex_() */
  virtual void unlockMutex_();

private:
  ConsoleChannelPtr channel_;
  QString currentLine_;
  bool usePrefix_;
  QMutex mutex_;
};

}

#endif /* SIMQT_CONSOLECHANNEL_H */
