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
#include <QString>
#include <QStringList>
#include "simNotify/NotifySeverity.h"
#include "simQt/FileDescriptorReplacement.h"
#include "simQt/ConsoleChannel.h"
#include "simQt/ConsoleDataModel.h"
#include "simQt/StdStreamConsoleChannel.h"

namespace simQt {

/** Performs buffering of the text data so it comes out one line at a time. */
class StdStreamConsoleChannel::TextBuffer
{
public:
  /** Initializes the text buffer to send to a channel with a particular severity */
  explicit TextBuffer(simNotify::NotifySeverity severity)
    : severity_(severity)
  {
  }

  /** Sends the last of the buffer off, if any buffer exists */
  virtual ~TextBuffer()
  {
    if (!buffer_.isEmpty() && channel_)
      channel_->addText(severity_, buffer_);
  }

  /** Changes the channel pointer */
  void setChannel(ConsoleChannelPtr channel)
  {
    channel_ = channel;
    segmentBuffer_();
  }

  /** Adds text to the channel */
  void addText(const QString& str)
  {
    buffer_.append(str);
    segmentBuffer_();
  }

private:
  /**
   * Responsible for breaking out a long text string into individual lines and
   * sending that data one-at-a-time to the channel
   */
  void segmentBuffer_()
  {
    if (channel_ == NULL)
      return;

    // Segment out by newline, then add to the console
    QStringList lines = buffer_.split('\n', QString::KeepEmptyParts);
    for (int k = 0; k < lines.count() - 1; ++k)
    {
      channel_->addText(severity_, lines[k]);
    }
    // Set the buffer to the last set of text that had no newline
    buffer_ = lines.isEmpty() ? "" : lines.last();

    // Assertion failure means algorithm left a newline in there
    assert(buffer_.count('\n') == 0);
  }

  /** Most recent text string, without newlines */
  QString buffer_;
  /** Severity with which to write to the channel */
  simNotify::NotifySeverity severity_;
  /** Channel to write text to */
  ConsoleChannelPtr channel_;
};


StdStreamConsoleChannel::StdStreamConsoleChannel(QObject* parent)
  : QObject(parent),
    stdoutBuffer_(new TextBuffer(simNotify::NOTIFY_NOTICE)),
    stderrBuffer_(new TextBuffer(simNotify::NOTIFY_ERROR)),
    stdoutFd_(FileDescriptorReplacement::replaceStdout(true)),
    stderrFd_(FileDescriptorReplacement::replaceStderr(true))
{
  // Connect output from stdout and stderr to our slots
  connect(stdoutFd_, SIGNAL(textReceived(const QString&)), this, SLOT(addStdoutText_(const QString&)));
  connect(stderrFd_, SIGNAL(textReceived(const QString&)), this, SLOT(addStderrText_(const QString&)));
}

StdStreamConsoleChannel::~StdStreamConsoleChannel()
{
  delete stdoutFd_;
  delete stderrFd_;
  delete stdoutBuffer_;
  delete stderrBuffer_;
}

void StdStreamConsoleChannel::bindTo(ConsoleDataModel& model)
{
  stdoutBuffer_->setChannel(model.registerChannel("Standard Output"));
  stderrBuffer_->setChannel(model.registerChannel("Standard Error"));
}

void StdStreamConsoleChannel::addStdoutText_(const QString& str)
{
  stdoutBuffer_->addText(str);
}

void StdStreamConsoleChannel::addStderrText_(const QString& str)
{
  stderrBuffer_->addText(str);
}

}
