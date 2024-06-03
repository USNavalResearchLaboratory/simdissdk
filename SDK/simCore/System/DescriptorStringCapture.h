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
#ifndef SIMCORE_SYSTEM_DESCRIPTORSTRINGCAPTURE_H
#define SIMCORE_SYSTEM_DESCRIPTORSTRINGCAPTURE_H

#include <vector>
#include <string>
#include "simCore/Common/Export.h"

namespace simCore { 

/**
 * Amalgamation of simQt::FileDescriptorReplacement and its nested class ReadInLoop.
 * Reads from the file descriptor provided to the constructor using readReady() and read().
 */
class SDKCORE_EXPORT DescriptorStringCapture
{
public:
  explicit DescriptorStringCapture(int whichFd, unsigned int fdBufferSize = 1024u);
  virtual ~DescriptorStringCapture();

  /** Cross-platform file number for stdout */
  static const int FD_STDOUT;
  /** Cross-platform file number for stderr */
  static const int FD_STDERR;

  /** Returns true if a non-empty read is ready and possible, false otherwise */
  bool readReady() const;
  /** Perform a read on the file descriptor, returns the read result. May return an empty string */
  std::string read();

private:
  /** Creates and installs the pipe to the file descriptor */
  int install_(int toFd);
  /** Removes the pipe and puts the old file descriptor back.  Invalidates copyOfOldFd. */
  int uninstall_(int fromFd, int& copyOfOldFd) const;
  /** Wrapper around OS-specific pipe() */
  int pipe_(int outPipe[2]) const;
  /** Wrapper around Windows API SetStdHandle(); returns 0 on success */
  int setStdHandle_(int toFd, int writeFd) const;

  /** File descriptor that was passed in by the user on construction */
  int replacedFd_ = -1;
  /** Size of buffer to hold redirect content */
  unsigned int fdBufferSize_ = 1024u;
  /** Duplicate version of the original file descriptor, used during uninstall to revert */
  int savedDupFd_ = -1;
  /** Our pipe's read end file descriptor, to be monitored by readReady() and read() */
  int pipeReadFd_ = -1;
  /** Our pipe's write end file descriptor, saved so we can wake up a blocking read on pipeReadFd_ */
  int pipeWriteFd_ = -1;
  /** Character array that holds the text data before it is emitted */
  std::vector<char> buffer_;
};

}

#endif
