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
#ifndef SIMCORE_SYSTEM_SHELLWINDOW_H
#define SIMCORE_SYSTEM_SHELLWINDOW_H

namespace simCore
{

/** Convenience class to give easy access to Windows API functions */
class ShellWindow
{
public:
  /** Returns true when the executable was started from a command shell window */
  static bool wasRunFromShell();

  /**
   * Shows or hides the black console window associated with the application.  Note that
   * if you call this method when the executable was run from a command shell window, it
   * WILL hide that command shell window.  It is recommended that you call this conditionally
   * based on the result of ShellWindow::wasRunFromShell().
   */
  static void setVisible(bool visible);
};

}

#endif /* SIMCORE_SYSTEM_SHELLWINDOW_H */
