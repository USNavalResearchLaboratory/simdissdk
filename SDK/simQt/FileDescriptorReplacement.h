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
#ifndef SIMQT_FILEDESCRIPTOR_REPLACEMENT_H
#define SIMQT_FILEDESCRIPTOR_REPLACEMENT_H

#include <QObject>
#include "simCore/Common/Export.h"

class QString;

namespace simQt
{

/**
 * This class intercepts output from a file descriptor (typically stdout or stderr)
 * and emits a signal with the intercepted text.  This can either be a complete
 * redirection where the original data never reaches its original destination, or
 * a tee-like fork of the data where it does reach its original destination.
 *
 * This class does not play well with calls to the Windows API FreeConsole() under
 * Windows 8, 8.1, 10, and presumably newer versions of Windows.  This is because
 * FreeConsole() has been updated in some cases to leave the stdout and stderr handles
 * in bad states after the call, forcing a close on the global standard handle.
 * While this behavior is not present in Windows 7 and earlier, it is recommended
 * that if you use this method, to replace it with a call instead to:
 *
 * <code>
 * ShowWindow(GetConsoleWindow(), SW_HIDE);
 * </code>
 */
class SDKQT_EXPORT FileDescriptorReplacement : public QObject
{
  Q_OBJECT;
public:

  /**
   * Replaces an arbitrary file descriptor.
   * @param whichFd File descriptor to replace
   * @param teeToOriginalDest If true, data is written to the original destination as
   *   well as emitted as a signal.  If this is attached to stdout, the console will
   *   receive no text from stdout unless teeToOriginalDest is true.
   * @param parent QObject parent for automatic deletion
   */
  FileDescriptorReplacement(int whichFd, bool teeToOriginalDest, QObject* parent=nullptr);
  /** Replaces the descriptor so regular output will continue */
  virtual ~FileDescriptorReplacement();

  /**
   * Factory method to replace stdout.  Useful because STDOUT_FILENO is not universally defined.
   * @param teeToStdOut If true, data is written to the original stdout as well as emitted as
   *   a signal.  The console will receive no text from stdout unless teeToStdOut is true.
   * @param parent QObject parent for automatic deletion
   */
  static FileDescriptorReplacement* replaceStdout(bool teeToStdOut, QObject* parent = nullptr);

  /**
   * Factory method to replace stderr.  Useful because STDERR_FILENO is not universally defined.
   * @param teeToStdErr If true, data is written to the original stderr as well as emitted as
   *   a signal.  The console will receive no text from stderr unless teeToStdErr is true.
   * @param parent QObject parent for automatic deletion
   */
  static FileDescriptorReplacement* replaceStderr(bool teeToStdErr, QObject* parent = nullptr);

Q_SIGNALS:
  /** Text has been read on the specified file descriptor */
  void textReceived(const QString& newText);

private Q_SLOTS:
  /** Clear out the pointer to reader_ due to lazy deletion on thread */
  void setReaderToNull_();

private:
  /** Creates and installs the pipe to the file descriptor */
  int install_(int toFd, int& copyOfOldFd, int& pipeReadEnd, int& pipeWriteEnd) const;
  /** Removes the pipe and puts the old file descriptor back.  Invalidates copyOfOldFd. */
  int uninstall_(int fromFd, int& copyOfOldFd) const;
  /** Wrapper around Windows API SetStdHandle(); returns 0 on success */
  int setStdHandle_(int toFd, int writeFd) const;
  /** Starts the thread that is required to monitor the pipe */
  void startThread_();
  /** Stops the thread that is required to monitor the pipe */
  void stopThread_();
  /** Wrapper around OS-specific pipe() */
  int pipe_(int outPipe[2]) const;

  class ReadInLoop;
  /** Thread that monitors the pipe for output */
  ReadInLoop* reader_;
  /** File descriptor that was passed in by the user on construction */
  int replacedFd_;
  /** Duplicate version of the original file descriptor, used during uninstall to revert */
  int savedDupFd_;
  /** Our pipe's read end file descriptor, to be monitored by the reader_ */
  int pipeReadFd_;
  /** Our pipe's write end file descriptor, saved so we can wake up a blocking read on pipeReadFd_ */
  int pipeWriteFd_;
  /** Flag indicating whether we should tee data to the original destination */
  bool teeToOriginalDest_;
};

/**
 * Standalone algorithm to read from a file descriptor and emit QStrings when
 * data has been read.  This algorithm can be used in a thread.  The algorithm
 * is subjected to polling on Windows, but uses select() on UNIX
 */
class SDKQT_EXPORT FileDescriptorReplacement::ReadInLoop : public QObject
{
  Q_OBJECT;
public:
  /** Constructor */
  ReadInLoop(int fd);
  virtual ~ReadInLoop();

Q_SIGNALS:
  /** Signaled when readLoop finally finishes. */
  void finished();
  /** Signaled when readLoop encounters a new text string. */
  void textReceived(const QString& newText);

public Q_SLOTS:
  /** Starts the loop with the file descriptor set up on construction */
  void readLoop();
  /** Tell the loop to stop execution */
  void stop();
  /** Sets the file descriptor to which we tee data (-1 for none).  Will be locally dup()'d. */
  void setTeeFileDescriptor(int fd);

private:
  /** Reads from the file descriptor, emitting textReceived as needed */
  int readBuffer_();
  /** Polling based loop option */
  void pollingLoop_();
  /** Select-based loop, only available on UNIX */
  void selectLoop_();

  /** Character array that holds the text data before it is emitted */
  char* buffer_;
  /** File descriptor to monitor */
  int fd_;
  /** File descriptor to tee data to; -1 for none.  Is dup'd when setTeeFileDescriptor() is called, so thread is responsible for closing. */
  int teeToFd_;
  /** Flag to stop the loop */
  bool done_;
};

}

#endif /* SIMQT_FILEDESCRIPTOR_REPLACEMENT_H */
