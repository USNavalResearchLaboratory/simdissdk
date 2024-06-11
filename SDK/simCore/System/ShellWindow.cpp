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
#include "simCore/System/ShellWindow.h"
#ifdef WIN32
#include <windows.h>
#endif

namespace simCore
{

/// Returns true when the executable was started from a command shell window
bool ShellWindow::wasRunFromShell()
{
#ifdef WIN32
  DWORD processIds;
  DWORD numProcesses = GetConsoleProcessList(&processIds, 1);
  return numProcesses > 1;
#else
  // Always run from a shell on Linux
  return true;
#endif
}

/// Sets visibility state on process's black console window
void ShellWindow::setVisible(bool visible)
{
#ifdef WIN32
  HWND consoleWindow = GetConsoleWindow();
  if (consoleWindow != nullptr)
    ShowWindow(consoleWindow, visible ? SW_RESTORE : SW_HIDE);
#endif
}

}
