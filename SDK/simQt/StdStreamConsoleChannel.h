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
#ifndef SIMQT_STDSTREAMCONSOLECHANNEL_H
#define SIMQT_STDSTREAMCONSOLECHANNEL_H

#include <QObject>
#include "simCore/Common/Export.h"

class QString;

namespace simQt {

class ConsoleDataModel;
class FileDescriptorReplacement;

/** Directs stdout and stderr to a channel in the ConsoleDataModel, to pass console output to the model */
class SDKQT_EXPORT StdStreamConsoleChannel : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  StdStreamConsoleChannel(QObject* parent = nullptr);
  virtual ~StdStreamConsoleChannel();

  /** Binds the streams to the console data model provided */
  void bindTo(ConsoleDataModel& model);

private Q_SLOTS:
  /** Called when new text is available from stdout */
  void addStdoutText_(const QString& str);
  /** Called when new text is available from stderr */
  void addStderrText_(const QString& str);

private:
  class TextBuffer;
  TextBuffer* stdoutBuffer_;
  TextBuffer* stderrBuffer_;
  FileDescriptorReplacement* stdoutFd_;
  FileDescriptorReplacement* stderrFd_;
};

}

#endif /* SIMQT_STDSTREAMCONSOLECHANNEL_H */
