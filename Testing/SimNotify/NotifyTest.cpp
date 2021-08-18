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
#include <iostream>
#include <string>
#include <sstream>
#include "simNotify/Notify.h"
#include "simCore/Common/Version.h"
#include "simCore/Common/SDKAssert.h"
#include "NotifySupport.h"

using namespace simCore;
using namespace std;
using NotifyTestSupport::StringStreamNotify;

namespace
{
// Verifies that one string matches the other
int testOutput(const std::string& str, const std::string& expected)
{
  if (str != expected)
  {
    cerr << "Expected <" << expected << ">, but got <" << str << ">" << endl;
    return 1;
  }
  return 0;
}
}

#define SIM_NOTIFY_FILE(level) SIM_NOTIFY(level) << "[" << __FILE__ << ":" << __LINE__ << "]  "

int NotifyTest(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  // Test out the default notification, and default levels
  SIM_WARN << "1 This should go to the console." << endl;
  SIM_DEBUG_FP << "ERROR - This should be ignored." << endl;
  int rv = 0;

  // Create a scope so our pointer can fall out of scope
  {
    // Create a new handler; note automatic memory management
    StringStreamNotify* ssNotifyPtr = new StringStreamNotify;
    simNotify::NotifyHandlerPtr ssNotify(ssNotifyPtr);
    simNotify::setNotifyHandlers(ssNotify);

    // First test that the notifier even works
    SIM_WARN << "Warn 1" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [WARN] Warn 1\n");
    // Make sure it ignores stuff below the threshold
    SIM_WARN << "Warn 2" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [WARN] Warn 2\n");
    // Make sure we can output another console message
    simNotify::setNotifyHandler(simNotify::NOTIFY_WARN, simNotify::defaultNotifyHandler());
    SIM_WARN << "2 This should go to the console." << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [WARN] Warn 2\n"); // Same as last check
    // Test another level for good measure
    SIM_FATAL << "Fatal 1" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [FATAL] Fatal 1\n");
    // Test that we can modify the display level
    SIM_INFO << "Info 1" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [FATAL] Fatal 1\n"); // Verifies that INFO is not displayed
    simNotify::setNotifyLevel(simNotify::NOTIFY_INFO);
    SIM_INFO << "Info 2" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [INFO] Info 2\n");
    // Make sure we're ignoring below the notify line
    SIM_DEBUG << "Debug 1" << endl;
    rv += testOutput(ssNotifyPtr->lastLine(), "[Date] [Time] [INFO] Info 2\n");

    // Reset the notify handlers to the console
    simNotify::setNotifyHandlers(simNotify::defaultNotifyHandler());
    SIM_WARN << "3 This should go to the console." << endl;

    // Demo printing out the file + line number
    SIM_NOTIFY_FILE(simNotify::NOTIFY_ALWAYS) << "This could be useful for traces\n" << endl;

    // Print out the stringstream for good measure
    SIM_ALWAYS << "Contents of the stringstream log:" << endl;
    SIM_ALWAYS << ssNotifyPtr->allLines() << endl;
  }

  // Verify that the string stream pointer got deallocated
  rv += (SDK_ASSERT(StringStreamNotify::g_StringStreamDestructions == 1));

  SIM_ALWAYS << "Test complete! " << ((rv == 0) ? "PASSED" : "FAILED") << endl;
  return rv;
}

