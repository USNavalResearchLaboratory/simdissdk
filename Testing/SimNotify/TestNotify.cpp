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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstring>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include "simNotify/Notify.h"
#include "simNotify/NotifyHandler.h"
#include "simNotify/StandardNotifyHandlers.h"
#include "simCore/Common/Common.h"
#include "simCore/Common/Version.h"
#include "simCore/String/UtfUtils.h"

using namespace std;

namespace
{

class AssertionException : public exception
{
public:
  explicit AssertionException(const string &message) : message_(message) { }
  ~AssertionException() throw() {}

  virtual const char *what() const throw() { return message_.c_str(); }

private:
  string message_;

};

// Test ability to set the notify level, query for notify level and
// check that a specific severity is enabled based on the current level
// Specific test parameters:
//   Test default notify level: expected NOTICE
//   Set notify level to FATAL and test notify level: expected FATAL
//   Test is notify enabled for ALWAYS: expected true
//   Test is notify enabled for FATAL: expected true
//   Test is notify enabled for WARN, NOTICE, INFO, DEBUG_INFO, DEBUG_FP: expected false
void testNotifyLevel()
{
  // Make sure default value is NOTICE
  simNotify::setNotifyLevel(simNotify::defaultNotifyLevel());
  if (simNotify::notifyLevel() != simNotify::NOTIFY_NOTICE)
  {
    throw AssertionException("Default level for simNotify::notify() is not simNotify::NOTIFY_INFO");
  }

  // Set the notification level to FATAL
  simNotify::setNotifyLevel(simNotify::NOTIFY_FATAL);

  // Check notify level; should be FATAL
  if (simNotify::notifyLevel() != simNotify::NOTIFY_FATAL)
  {
    throw AssertionException("Current level for simNotify::notify() is not simNotify::NOTIFY_FATAL; simNotify::setNotifyLevel failed");
  }

  // Check enabled/disabled state for severity values based on current notify level (FATAL)
  // ALWAYS and FATAL should be enabled; WARN, NOTICE, INFO, DEBUG_INFO, and DEBUG_FP should be disabled
  if (!isNotifyEnabled(simNotify::NOTIFY_ALWAYS))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_ALWAYS is not enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (!isNotifyEnabled(simNotify::NOTIFY_FATAL))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_FATAL is not enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_ERROR))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_ERROR is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_WARN))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_WARN is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_NOTICE))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_NOTICE is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_INFO))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_INFO is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_DEBUG_INFO))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_DEBUG_INFO is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }

  if (isNotifyEnabled(simNotify::NOTIFY_DEBUG_FP))
  {
    throw AssertionException("simNotify::isNotifyEnabled() reports simNotify::NOTIFY_DEBUG_FP is enabled with notify level set to simNotify::NOTIFY_FATAL");
  }
}

// Test implementation of NotifyHandler interface to capture output as a string.
template <typename BaseHandler>
class NotifyHandlerTest : public BaseHandler
{
public:
  std::string getBuffer() const { return buffer_; }

  void clearBuffer() { buffer_.clear(); }

  virtual void notify(const std::string &message)
  {
    buffer_ += message;
  }

private:
  std::string buffer_;
};

// Test functionality for assigning notify handler objects to notify severity levels.
void testSetNotifyHandler()
{
  simNotify::NotifyHandlerPtr handler1(new NotifyHandlerTest<simNotify::NotifyHandler>());
  simNotify::NotifyHandlerPtr handler2(new NotifyHandlerTest<simNotify::NotifyHandler>());

  // Change the NOTICE handler
  simNotify::setNotifyHandler(simNotify::NOTIFY_NOTICE, handler1);
  if (simNotify::notifyHandler(simNotify::NOTIFY_NOTICE) != handler1)
  {
    throw AssertionException("simNotify::notifyHandler() reports that the NotifyHandler object associated with simNotify::NOTIFY was not changed by simNotify::setNotifyHandler()");
  }

  // Make sure it did not change the other handlers
  if (simNotify::notifyHandler(simNotify::NOTIFY_ALWAYS) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_FATAL) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_ERROR) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_WARN) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_INFO) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_DEBUG_INFO) == handler1 ||
      simNotify::notifyHandler(simNotify::NOTIFY_DEBUG_FP) == handler1)
  {
    throw AssertionException("simNotify::notifyHandler() reports that NotifyHandler objects associated with levels other than simNotify::NOTIFY_NOTICE were changed when using simNotify::setNotifyHandler() to change the NotifyHandler object associated with simNotify::NOTIFY_NOTICE");
  }

  // Change the ALWAYS handler
  simNotify::setNotifyHandler(simNotify::NOTIFY_ALWAYS, handler2);
  if (simNotify::notifyHandler(simNotify::NOTIFY_ALWAYS) != handler2)
  {
    throw AssertionException("simNotify::notifyHandler() reports that the NotifyHandler object associated with simNotify::NOTIFY_ALWAYS was not changed by simNotify::setNotifyHandler()");
  }

  // Change all of the notify handlers back to the default
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());

  // Make sure all handlers are default
  if (simNotify::notifyHandler(simNotify::NOTIFY_ALWAYS) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_FATAL) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_ERROR) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_WARN) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_INFO) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_DEBUG_INFO) != simNotify::defaultNotifyHandler() ||
      simNotify::notifyHandler(simNotify::NOTIFY_DEBUG_FP) != simNotify::defaultNotifyHandler())
  {
    throw AssertionException("simNotify::notifyHandler() reports that not all NotifyHandler objects were set to the default NotifyHandler object");
  }
}

// Test ability to suppress messages based on the notification level.  Also
// tests the NullNotifyHandler class.
void testNotifyHandlerSuppression()
{
  const std::string expected = "DEBUG_FP:  Who put the bomp in the bomp-sha-bomp-sha-bomp";
  NotifyHandlerTest<simNotify::NotifyHandler> *rawHandler = new NotifyHandlerTest<simNotify::NotifyHandler>();
  simNotify::NotifyHandlerPtr handler(rawHandler);

  // Set our test notify handler and notify limit
  simNotify::setNotifyHandlers(handler);
  simNotify::setNotifyLevel(simNotify::NOTIFY_DEBUG_FP);

  // Writing a message to DEBUG_FP should not be suppressed
  simNotify::notify(simNotify::NOTIFY_DEBUG_FP) << "Who put the bomp in the bomp-sha-bomp-sha-bomp";

  if (rawHandler->getBuffer() != expected)
  {
    std::stringstream stream;
    stream << "simNotify::NotifyHandler::operator<< did not produce the expected result during the suppression test" << std::endl <<
              "\tExpected: " << expected <<
              "\tActual: " << rawHandler->getBuffer() << std::endl;

    throw AssertionException(stream.str());
  }

  // Clear the buffer before next test
  rawHandler->clearBuffer();

  // Change the level to suppress DEBUG_FP
  simNotify::setNotifyLevel(simNotify::NOTIFY_DEBUG_INFO);

  // Writing a message to DEBUG_FP should now be suppressed
  simNotify::notify(simNotify::NOTIFY_DEBUG_FP) << "Who put the bomp in the bomp-sha-bomp-sha-bomp";

  // The string should not be
  if (!rawHandler->getBuffer().empty())
  {
    std::stringstream stream;
    stream << "simNotify::NotifyHandler::operator<< did not produce the expected result during the suppression test" << std::endl <<
              "\tExpected: <empty string>" <<
              "\tActual: " << rawHandler->getBuffer() << std::endl;

    throw AssertionException(stream.str());
  }

  // Reset notify handlers to default
  simNotify::setNotifyLevel(simNotify::defaultNotifyLevel());
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
}

// Test string formatting capability provided by NotifyHandler::operator<<().
void testNotifyHandlerFormatting()
{
  const std::string expected = "NOTICE:  Testing 1, 2.0, c\n";
  NotifyHandlerTest<simNotify::NotifyHandler> *rawHandler = new NotifyHandlerTest<simNotify::NotifyHandler>();
  simNotify::NotifyHandlerPtr handler(rawHandler);

  // Set our test notify handler and notify limit
  simNotify::setNotifyHandlers(handler);
  simNotify::setNotifyLevel(simNotify::NOTIFY_NOTICE);

  // First test the notify function
  simNotify::notify(simNotify::NOTIFY_NOTICE).notify("Testing 1, 2.0, c\n");

  if (rawHandler->getBuffer() != expected)
  {
    std::stringstream stream;
    stream << "simNotify::NotifyHandler::notify(message) did not produce the expected result" << std::endl <<
              "\tExpected: " << expected <<
              "\tActual: " << rawHandler->getBuffer() << std::endl;

    throw AssertionException(stream.str());
  }

  // Clear the buffer before the next test
  rawHandler->clearBuffer();

  // Now test the stream operation
  simNotify::notify(simNotify::NOTIFY_NOTICE) << std::string("Testing ") << 1 << ", " << std::fixed << std::setprecision(1) << 2.0 << ", " << std::hex << 12 << std::endl;

  if (rawHandler->getBuffer() != expected)
  {
    std::stringstream stream;
    stream << "simNotify::NotifyHandler::operator<< did not produce the expected result" << std::endl <<
              "\tExpected: " << expected <<
              "\tActual: " << rawHandler->getBuffer() << std::endl;

    throw AssertionException(stream.str());
  }

  // Reset notify handlers to default
  simNotify::setNotifyLevel(simNotify::defaultNotifyLevel());
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
}

// Test StandardNotifyHandler.  ALWAYS, FATAL, and WARN should write to stderr.
// NOTICE, INFO, DEBUG_INFO, and DEBUG_FP should write to stdout.
void testStandardNotifyHandler()
{
  simNotify::NotifyHandlerPtr handler(new simNotify::StandardNotifyHandler());

  simNotify::setNotifyHandlers(handler);
  simNotify::setNotifyLevel(simNotify::NOTIFY_DEBUG_FP);

  const char testString[] = "Test\n";

  // Direct stdout and stderr to a buffer
  char stdoutBuf[BUFSIZ];
  char stderrBuf[BUFSIZ];

  setbuf(stdout, stdoutBuf);
  setbuf(stderr, stderrBuf);

  // Workaround for setbuf related problem on Linux:
  // First time writing to stdout writes correct message to console
  // ("NOTIFY:  Test\n"), but incorrect message in buffer ("Test\nY:  ")
  // This only happens on the first write, so we write something before
  // running the test to trigger and clear the issue
  simNotify::notify(simNotify::NOTIFY_NOTICE) << testString;
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;

  fflush(stdout);
  fflush(stderr);

  memset(stdoutBuf, '\0', sizeof(stdoutBuf));
  memset(stderrBuf, '\0', sizeof(stderrBuf));

  // Write message with severity ALWAYS
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;
  if (std::string(stderrBuf) != (std::string("ALWAYS:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stderr for severity level ALWAYS.");
  }
  // Clear/reset the buffer for next test
  fflush(stderr);
  memset(stderrBuf, '\0', sizeof(stderrBuf));

  // Write message with severity FATAL
  simNotify::notify(simNotify::NOTIFY_FATAL) << testString;
  if (std::string(stderrBuf) != (std::string("FATAL:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stderr for severity level FATAL.");
  }
  // Clear/reset the buffer for next test
  fflush(stderr);
  memset(stderrBuf, '\0', sizeof(stderrBuf));

   // Write message with severity ERROR
  simNotify::notify(simNotify::NOTIFY_ERROR) << testString;
  if (std::string(stderrBuf) != (std::string("ERROR:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stderr for severity level ERROR.");
  }
  // Clear/reset the buffer for next test
  fflush(stderr);
  memset(stderrBuf, '\0', sizeof(stderrBuf));

 // Write message with severity WARN
  simNotify::notify(simNotify::NOTIFY_WARN) << testString;
  if (std::string(stderrBuf) != (std::string("WARN:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stderr for severity level WARN.");
  }
  // Clear/reset the buffer for next test
  fflush(stderr);
  memset(stderrBuf, '\0', sizeof(stderrBuf));

  // Write message with severity NOTICE
  simNotify::notify(simNotify::NOTIFY_NOTICE) << testString;
  if (std::string(stdoutBuf) != (std::string("NOTICE:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stdout for severity level NOTICE.");
  }
  // Clear/reset the buffer for next test
  fflush(stdout);
  memset(stdoutBuf, '\0', sizeof(stdoutBuf));

  // Write message with severity INFO
  simNotify::notify(simNotify::NOTIFY_INFO) << testString;
  if (std::string(stdoutBuf) != (std::string("INFO:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stdout for severity level INFO.");
  }
  // Clear/reset the buffer for next test
  fflush(stdout);
  memset(stdoutBuf, '\0', sizeof(stdoutBuf));

  // Write message with severity DEBUG_INFO
  simNotify::notify(simNotify::NOTIFY_DEBUG_INFO) << testString;
  if (std::string(stdoutBuf) != (std::string("DEBUG_INFO:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stdout for severity level DEBUG_INFO.");
  }
  // Clear/reset the buffer for next test
  fflush(stdout);
  memset(stdoutBuf, '\0', sizeof(stdoutBuf));

  // Write message with severity DEBUG_FP
  simNotify::notify(simNotify::NOTIFY_DEBUG_FP) << testString;
  if (std::string(stdoutBuf) != (std::string("DEBUG_FP:  ") + std::string(testString)))
  {
    throw AssertionException("StandardNotifyHandler did not write the correct message to stdout for severity level DEBUG_FP.");
  }
  // Clear/reset the buffer for next test
  fflush(stdout);
  memset(stdoutBuf, '\0', sizeof(stdoutBuf));

  // Reset stdout and stderr
  setbuf(stdout, nullptr);
  setbuf(stderr, nullptr);

  // Reset notify handlers to default
  simNotify::setNotifyLevel(simNotify::defaultNotifyLevel());
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
}

// Test StdoutNotifyHandler.  Unlike StandardNotifyHandler which selects between
// stdout and stderr based on notify severity, StdoutNotifyHandler always writes
// to stdout.  Because of this it should be sufficient to test only simNotify::NOTIFY_ALWAYS.
void testStdoutNotifyHandler()
{
  simNotify::NotifyHandlerPtr handler(new simNotify::StdoutNotifyHandler());

  simNotify::setNotifyHandlers(handler);

  const char testString[] = "Test\n";

  // Direct stdout to a buffer
  char stdoutBuf[BUFSIZ];
  setbuf(stdout, stdoutBuf);

  // Workaround for setbuf related problem on Linux:
  // First time writing to stdout writes correct message to console
  // ("ALWAYS:  Test\n"), but incorrect message in buffer ("Test\nS:  ")
  // This only happens on the first write, so we write something before
  // running the test to trigger and clear the issue
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;

  fflush(stdout);
  memset(stdoutBuf, '\0', sizeof(stdoutBuf));

  // Write message with severity ALWAYS
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;
  if (std::string(stdoutBuf) != (std::string("ALWAYS:  ") + std::string(testString)))
  {
    throw AssertionException("StdoutNotifyHandler did not write the correct message to stdout.");
  }

  // Reset stdout and stderr
  setbuf(stdout, nullptr);

  // Reset notify handlers to default
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
}

// Test StderrNotifyHandler.  Unlike StandardNotifyHandler which selects between
// stdout and stderr based on notify severity, StderrNotifyHandler always writes
// to stderr.  Because of this it should be sufficient to test only simNotify::NOTIFY_ALWAYS.
void testStderrNotifyHandler()
{
  simNotify::NotifyHandlerPtr handler(new simNotify::StderrNotifyHandler());

  simNotify::setNotifyHandlers(handler);

  const char testString[] = "Test\n";

  // Direct stderr to a buffer
  char stderrBuf[BUFSIZ];
  setbuf(stderr, stderrBuf);
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;
  fflush(stderr);
  memset(stderrBuf, '\0', sizeof(stderrBuf));

  // Write message with severity ALWAYS
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;
  if (std::string(stderrBuf) != (std::string("ALWAYS:  ") + std::string(testString)))
  {
    throw AssertionException("StderrNotifyHandler did not write the correct message to stderr.");
  }

  // Reset stdout and stderr
  setbuf(stderr, nullptr);

  // Reset notify handlers to default
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
}

// Test FileNotifyHandler.  Unlike StandardNotifyHandler which selects between
// stdout and stderr based on notify severity, FileNotifyHandler always writes
// to the same file.  Because of this it should be sufficient to test only
// simNotify::NOTIFY_ALWAYS.
void testFileNotifyHandler()
{
  const char filename[] = "testFileNotifyHandler.out";
  simNotify::NotifyHandlerPtr handler(new simNotify::FileNotifyHandler("testFileNotifyHandler.out"));

  simNotify::setNotifyHandlers(handler);

  // For this test, our test string does not terminate with '\n'.
  // It is simpler to not have the '\n', as istream::getline will discard it.
  // Instead we will just let istream::getline read until EOF.
  std::string testString = "Test";

  // Write message with severity ALWAYS
  simNotify::notify(simNotify::NOTIFY_ALWAYS) << testString;

  // Reset notify handlers to default
  simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());

  // Delete the handler to close file and ensure file buffer is flushed
  handler.reset();

  // Read the string from the file
  ifstream fd(simCore::streamFixUtf8(filename));

  if (!fd.is_open())
  {
    throw AssertionException("testFileNotifyHandler failed to open test file for reading");
  }

  char fileBuf[BUFSIZ];
  fd.getline(fileBuf, BUFSIZ);

  if (std::string(fileBuf) != (std::string("ALWAYS:  ") + std::string(testString)))
  {
    throw AssertionException("FileNotifyHandler did not successfully write a message to a file.");
  }

  fd.close();

  // Try to remove the test file
#ifdef WIN32
  _unlink(filename);
#else
  unlink(filename);
#endif
}

void testStreamNotifyHandler()
{
  std::stringstream ss;
  simNotify::NotifyHandlerPtr handler(new simNotify::StreamNotifyHandler(ss));
  simNotify::setNotifyHandlers(handler);
  SIM_ALWAYS << "Hello, world!\n";
  if (ss.str() != "ALWAYS:  Hello, world!\n")
    throw AssertionException("StreamNotifyHandler did not capture the string completely.");
  ss.str("");
}

}

int TestNotify(int argc, char** const argv)
{
  simCore::checkVersionThrow();
  try
  {
    // Test functions for setting and querying notify severity level
    testNotifyLevel();

    // Test functions to change the notify handler associated with a specific severity level
    testSetNotifyHandler();
    testNotifyHandlerSuppression();
    testNotifyHandlerFormatting();
    testStandardNotifyHandler();
    testStdoutNotifyHandler();
    testStderrNotifyHandler();
    testFileNotifyHandler();
    testStreamNotifyHandler();
  }
  catch (AssertionException& e)
  {
    cout << e.what() << endl;
    return 1;
  }

  return 0;
}
