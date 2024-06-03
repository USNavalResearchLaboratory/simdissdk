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
#include <fcntl.h>
#ifdef WIN32
#include <io.h>
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include "DescriptorStringCapture.h"

namespace simCore { 

// Use define STDOUT_FILENO and STDERR_FILENO for non-Windows systems
#ifdef STDOUT_FILENO
const int DescriptorStringCapture::FD_STDOUT = STDOUT_FILENO;
#else
const int DescriptorStringCapture::FD_STDOUT = 1;
#endif

#ifdef STDERR_FILENO
const int DescriptorStringCapture::FD_STDERR = STDERR_FILENO;
#else
const int DescriptorStringCapture::FD_STDERR = 2;
#endif

DescriptorStringCapture::DescriptorStringCapture(int whichFd, unsigned int fdBufferSize)
  : replacedFd_(whichFd),
  fdBufferSize_(fdBufferSize),
  buffer_(fdBufferSize_ + 1, '\0')
{
  install_(replacedFd_);
}

DescriptorStringCapture::~DescriptorStringCapture()
{
  uninstall_(replacedFd_, savedDupFd_);

  // Close the read and write ends of the pipe, which are still open at this point
  if (pipeReadFd_ >= 0)
    close(pipeReadFd_);
  pipeReadFd_ = -1;
  if (pipeWriteFd_ >= 0)
    close(pipeWriteFd_);
  pipeWriteFd_ = -1;
}

bool DescriptorStringCapture::readReady() const
{
#ifdef WIN32
  // Windows cannot test this properly.  Instead we write a null character on each
  // read to avoid having windows block permanently.
  return true;
#else
  // select() will change fdSet to indicate which descriptors are active
  fd_set fdSet;
  FD_ZERO(&fdSet);
  FD_SET(pipeReadFd_, &fdSet);
  // Note: TimeVal may be modified by select()
  timeval timeVal = { 0 };
  const int selectRv = select(pipeReadFd_ + 1, &fdSet, nullptr, nullptr, &timeVal);
  return selectRv > 0;
#endif
}

std::string DescriptorStringCapture::read()
{
#ifdef WIN32
  // On Windows, work around _read() not being non-blocking.
  if (pipeWriteFd_ >= 0)
    write(pipeWriteFd_, "\0", 1);
#endif
  if (!readReady())
    return "";

#ifdef WIN32
  int rv = _read(pipeReadFd_, &buffer_[0], fdBufferSize_);
#else
  int rv = ::read(pipeReadFd_, &buffer_[0], fdBufferSize_);
#endif

  // Don't write single nullptr characters, and don't write errors
  if (rv > 0 && (rv > 1 || buffer_[0] != '\0'))
  {
    // Return the read data
    buffer_[rv] = '\0';
    return &buffer_[0];
  }
  return "";
}

int DescriptorStringCapture::install_(int toFd)
{
  // Save the old file descriptor so we can restore it later
  savedDupFd_ = dup(toFd);
  // Presume error on pipe ends unless we know better
  pipeReadFd_ = -1;
  pipeWriteFd_ = -1;

  // Note that the copyOfOldFd may "validly" be -1 on Windows systems in some (most?) cases.
  // This doesn't prevent us from being able to replace the FD later with dup2().

  // Make a pipe
  int outPipe[2] = { 0 };
  if (pipe_(outPipe) != 0)
    return 1;

  // Redirect to the pipe, and close our portion of the pipe
  if (dup2(outPipe[1], toFd) < 0) // Windows and Linux return value differs, but both have negative error values
    return 1;

  pipeReadFd_ = outPipe[0];
  pipeWriteFd_ = outPipe[1];

  // Need to set non-buffered output on the stream.  Without this, the streams could be
  // buffered on write, causing reads to block until the stream decides to flush.  This
  // is for buffered output (e.g. printf, fprintf, cout, etc.) and not FD-based write().
  if (toFd == FD_STDOUT)
    setvbuf(stdout, nullptr, _IONBF, 0);
  else if (toFd == FD_STDERR)
    setvbuf(stderr, nullptr, _IONBF, 0);

  // Also on Windows set the default handle buffer as needed
  setStdHandle_(toFd, pipeWriteFd_);
  return 0;
}

int DescriptorStringCapture::uninstall_(int fromFd, int& copyOfOldFd) const
{
  if (fromFd >= 0 && copyOfOldFd >= 0)
  {
    dup2(copyOfOldFd, fromFd);
    // We no longer need copyOfOldFd
    close(copyOfOldFd);
    copyOfOldFd = -1;

    // Also reset the standard handle for Win32 systems
    setStdHandle_(fromFd, fromFd);
    return 0;
  }

  // Even if fromFd is invalid, we still need to close out our copy of original
  if (copyOfOldFd >= 0)
  {
    close(copyOfOldFd);
    copyOfOldFd = -1;
  }
  return 1;
}

int DescriptorStringCapture::pipe_(int outPipe[2]) const
{
#ifdef WIN32
  return _pipe(outPipe, fdBufferSize_, _O_BINARY);
#else
  return ::pipe(outPipe);
#endif
}

int DescriptorStringCapture::setStdHandle_(int toFd, int writeFd) const
{
#ifdef WIN32
  DWORD handleNum = 0;
  if (toFd == FD_STDOUT)
    handleNum = STD_OUTPUT_HANDLE;
  else if (toFd == FD_STDERR)
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

}
