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
#ifdef WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>
#include <cassert>
#include <iostream>
#include <QCoreApplication>
#include <QThread>
#include "simCore/Common/Time.h"
#include "simCore/Common/Exception.h"
#include "simNotify/Notify.h"
#include "simQt/FileDescriptorReplacement.h"

namespace simQt
{

/** Size of the read buffer; relatively arbitrary */
static const int FD_BUFFER_SIZE = 1024;

FileDescriptorReplacement::ReadInLoop::ReadInLoop(int fd)
  : fd_(fd),
    teeToFd_(-1),
    done_(true)
{
  buffer_ = new char[FD_BUFFER_SIZE + 1];
}

FileDescriptorReplacement::ReadInLoop::~ReadInLoop()
{
  SAFETRYBEGIN;
  // Close our tee'd file descriptor
  if (teeToFd_ >= 0)
    close(teeToFd_);
  teeToFd_ = -1;

  delete[] buffer_;
  buffer_ = nullptr;
  SAFETRYEND("destroying file descriptor reading loop");
}

void FileDescriptorReplacement::ReadInLoop::pollingLoop_()
{
  // Assertion failure indicates bad call to pollingLoop_ (precondition)
  assert(!done_);
  while (!done_)
  {
    int rv = readBuffer_();
    if (rv < 0) // error
      stop();
    else if (rv == 0) // no data
      Sleep(10);
  }
}

void FileDescriptorReplacement::ReadInLoop::selectLoop_()
{
#ifdef WIN32
  // select() is not available for file descriptors
  pollingLoop_();
#else
  fd_set fdSet;
  timeval timeVal;
  while (!done_)
  {
    // select() will change fdSet to indicate which descriptors are active
    FD_ZERO(&fdSet);
    FD_SET(fd_, &fdSet);
    // Note: TimeVal may be modified by select()
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 100000; // 100 milliseconds

    int rv = select(fd_ + 1, &fdSet, nullptr, nullptr, &timeVal);
    if (rv < 0)
    {
      stop();
    }
    else if (rv > 0)
    {
      if (readBuffer_() < 0)
        stop();
    }
  }
#endif
}

void FileDescriptorReplacement::ReadInLoop::readLoop()
{
  // Avoid reentry
  if (!done_)
    return;

  SAFETRYBEGIN;
  done_ = false;
  selectLoop_();
  Q_EMIT finished();
  SAFETRYEND("reading File Descriptor Replacement in loop")
}

void FileDescriptorReplacement::ReadInLoop::stop()
{
  done_ = true;
}

void FileDescriptorReplacement::ReadInLoop::setTeeFileDescriptor(int fd)
{
  SAFETRYBEGIN;
  // Close out our local tee handle, since we completely own its resources
  if (teeToFd_ >= 0)
    close(teeToFd_);

  // Duplicate the handle, or set to invalid, as needed
  if (fd >= 0)
    teeToFd_ = dup(fd);
  else
    teeToFd_ = -1;
  return;
  SAFETRYEND("setting tee file descriptor");
  teeToFd_ = -1;
}

int FileDescriptorReplacement::ReadInLoop::readBuffer_()
{
  SAFETRYBEGIN;
#ifdef WIN32
  int rv = _read(fd_, buffer_, FD_BUFFER_SIZE);
#else
  int rv = read(fd_, buffer_, FD_BUFFER_SIZE);
#endif

  // Don't write single nullptr characters, and don't write errors
  if (rv > 0 && (rv > 1 || buffer_[0] != '\0'))
  {
    // Send the output to the original destination if required
    if (teeToFd_ != -1)
    {
      // Write should always succeed, unless there was no valid file descriptor
      // to start with.  This can happen in WIN32 apps without a console.
      if (write(teeToFd_, buffer_, rv) != rv)
      {
        // This should only happen when teeToFd_ is invalid; no need to close() there
        teeToFd_ = -1;
      }
    }

    // Emit the data over a Qt signal
    buffer_[rv] = '\0';
    Q_EMIT(textReceived(buffer_));
  }
  return rv;
  SAFETRYEND("reading file descriptor buffer");
  return -1;
}

////////////////////////////////////////////////////////

// Define STDOUT_FILENO and STDERR_FILENO for Windows systems
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

FileDescriptorReplacement::FileDescriptorReplacement(int whichFd, bool teeToOriginalDest, QObject* parent)
  : QObject(parent),
    reader_(nullptr),
    replacedFd_(whichFd),
    savedDupFd_(-1),
    pipeReadFd_(-1),
    pipeWriteFd_(-1),
    teeToOriginalDest_(teeToOriginalDest)
{
  SAFETRYBEGIN;
  if (install_(replacedFd_, savedDupFd_, pipeReadFd_, pipeWriteFd_) == 0)
  {
    startThread_();
  }
  SAFETRYEND("installing file descriptor replacement")
}

FileDescriptorReplacement* FileDescriptorReplacement::replaceStdout(bool teeToStdOut, QObject* parent)
{
  return new FileDescriptorReplacement(STDOUT_FILENO, teeToStdOut, parent);
}

FileDescriptorReplacement* FileDescriptorReplacement::replaceStderr(bool teeToStdErr, QObject* parent)
{
  return new FileDescriptorReplacement(STDERR_FILENO, teeToStdErr, parent);
}

FileDescriptorReplacement::~FileDescriptorReplacement()
{
  SAFETRYBEGIN;
  stopThread_();
  uninstall_(replacedFd_, savedDupFd_);

  // Close the read and write ends of the pipe, which are still open at this point
  if (pipeReadFd_ != -1)
  {
    if (close(pipeReadFd_) != 0)
    {
      const int errNum = errno;
      SIM_ERROR << "Error closing read pipe for file descriptor replacement " << replacedFd_ << ".  Error code: " << errNum << " (" << strerror(errNum) << ")\n";
    }
  }
  pipeReadFd_ = -1;
  if (pipeWriteFd_ != -1)
  {
    if (close(pipeWriteFd_) != 0)
    {
      const int errNum = errno;
      SIM_ERROR << "Error closing write pipe for file descriptor replacement " << replacedFd_ << ".  Error code: " << errNum << " (" << strerror(errNum) << ")\n";
    }
  }
  pipeWriteFd_ = -1;
  SAFETRYEND("destroying file descriptor replacement");
}

int FileDescriptorReplacement::uninstall_(int fromFd, int& copyOfOldFd) const
{
  SAFETRYBEGIN;
  if (fromFd >= 0 && copyOfOldFd >= 0)
  {
    dup2(copyOfOldFd, fromFd);
    // We no longer need copyOfOldFd
    if (close(copyOfOldFd) != 0)
    {
      const int errNum = errno;
      SIM_ERROR << "Error closing duplicated original descriptor for " << replacedFd_ << ".  Error code: " << errNum << " (" << strerror(errNum) << ")\n";
    }
    copyOfOldFd = -1;

    // Also reset the standard handle for Win32 systems
    if (setStdHandle_(fromFd, fromFd) != 0)
    {
      SIM_ERROR << "Error restoring the global standard handle for file descriptor replacement " << replacedFd_ << ".\n";
    }
    return 0;
  }

  // Even if fromFd is invalid, we still need to close out our copy of original
  if (copyOfOldFd >= 0)
  {
    if (close(copyOfOldFd) != 0)
    {
      const int errNum = errno;
      SIM_ERROR << "Error closing duplicated original descriptor for " << replacedFd_ << ".  Error code: " << errNum << " (" << strerror(errNum) << ")\n";
    }
    copyOfOldFd = -1;
  }
  SAFETRYEND("while uninstalling file descriptor replacement");
  return 1;
}

int FileDescriptorReplacement::pipe_(int outPipe[2]) const
{
#ifdef WIN32
  return _pipe(outPipe, FD_BUFFER_SIZE, _O_BINARY);
#else
  return ::pipe(outPipe);
#endif
}

int FileDescriptorReplacement::install_(int toFd, int& copyOfOldFd, int& pipeReadEnd, int& pipeWriteEnd) const
{
  // Save the old file descriptor so we can restore it later
  copyOfOldFd = dup(toFd);
  // Presume error on pipe ends unless we know better
  pipeReadEnd = -1;
  pipeWriteEnd = -1;

  // Note that the copyOfOldFd may "validly" be -1 on Windows systems in some (most?) cases.
  // This doesn't prevent us from being able to replace the FD later with dup2().

  // Make a pipe
  int outPipe[2] = {0};
  if (pipe_(outPipe) != 0)
  {
    SIM_ERROR << "Unable to create system pipe to replace file descriptor " << toFd << ".\n";
    return 1;
  }

  // Redirect to the pipe, and close our portion of the pipe
  if (dup2(outPipe[1], toFd) < 0) // Windows and Linux return value differs, but both have negative error values
  {
    const int errNum = errno;
    SIM_ERROR << "Unable to replace file descriptor " << toFd << " with system pipe.  Error code: " << errNum << " (" << strerror(errNum) << ")\n";
    return 1;
  }
  pipeReadEnd = outPipe[0];
  pipeWriteEnd = outPipe[1];

  // Need to set non-buffered output on the stream.  Without this, the streams could be
  // buffered on write, causing reads to block until the stream decides to flush.  This
  // is for buffered output (e.g. printf, fprintf, cout, etc.) and not FD-based write().
  if (toFd == STDOUT_FILENO)
    setvbuf(stdout, nullptr, _IONBF, 0);
  else if (toFd == STDERR_FILENO)
    setvbuf(stderr, nullptr, _IONBF, 0);

  // Also on Windows set the default handle buffer as needed
  if (setStdHandle_(toFd, pipeWriteEnd) != 0)
  {
    SIM_WARN << "Unable to set global standard handle for file descriptor " << toFd << ".\n";
    // Don't return an error here, since the rest of the replacement worked
  }
  return 0;
}

int FileDescriptorReplacement::setStdHandle_(int toFd, int writeFd) const
{
#ifdef WIN32
  DWORD handleNum = 0;
  if (toFd == STDOUT_FILENO)
    handleNum = STD_OUTPUT_HANDLE;
  else if (toFd == STDERR_FILENO)
    handleNum = STD_ERROR_HANDLE;
  else // Not a standard handle; but also not an error to pass in a non-std handle
    return 0;

  // If the write handle is invalid, we return an error.  This can happen on a
  // dup'd handle in Windows that gets closed eventually through no fault of our own.
  HANDLE writeHandle = reinterpret_cast<HANDLE>(_get_osfhandle(writeFd));
  if (writeHandle == INVALID_HANDLE_VALUE)
    return 1;
  // If SetStdHandle succeeds, the return value is non-zero
  return SetStdHandle(handleNum, writeHandle) == 0 ? 1 : 0;
#endif
  return 0;
}

void FileDescriptorReplacement::startThread_()
{
  if (reader_ != nullptr)
    return;
  QThread *thread = new QThread;
  thread->setObjectName(tr("FDReplacement %1 Thread").arg(replacedFd_));
  reader_ = new ReadInLoop(pipeReadFd_);
  if (teeToOriginalDest_)
    reader_->setTeeFileDescriptor(savedDupFd_);
  // Start the thread with the reader
  reader_->moveToThread(thread);
  // http://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
  connect(reader_, SIGNAL(textReceived(QString)), this, SIGNAL(textReceived(QString)));
  connect(thread, SIGNAL(started()), reader_, SLOT(readLoop()));
  connect(reader_, SIGNAL(finished()), thread, SLOT(quit()));
  connect(reader_, SIGNAL(finished()), this, SLOT(setReaderToNull_()));
  connect(reader_, SIGNAL(finished()), reader_, SLOT(deleteLater()));
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
  thread->start();
}

void FileDescriptorReplacement::setReaderToNull_()
{
  // The purpose of this slot is to set reader_ to nullptr.  It's possible
  // that the reader_ will decide on its own when to stop running, due to
  // an invalid handle for example (this can happen with the use of the
  // windows API FreeConsole()).  Avoid dereferencing stale pointers when
  // this happens by setting reader_ to nullptr.
  reader_ = nullptr;
}

void FileDescriptorReplacement::stopThread_()
{
  SAFETRYBEGIN;
  if (reader_ != nullptr)
  {
    reader_->stop();

    // Add data to the Write end of the pipe to wake up the process if it's sleeping.
    // Note that on Windows, there's no way to make the read non-blocking.  So we have
    // to push data down the write end in order to be sure that the thread wakes up.
    // On Linux, we use select() (no Windows equivalent on anonymous pipes), but this
    // will still wake up the select statement, which is good.
    if (pipeWriteFd_ >= 0)
      write(pipeWriteFd_, "\0", 1);

    // Process events to ensure that the last chunk of stdout/stderr is processed.
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
  SAFETRYEND("while stopping FileDescriptorReplacement thread");

  // reader_ gets deleted automatically with deleteLater() signal
  reader_ = nullptr;
}

}
