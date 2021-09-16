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
#include "simVis/Headless.h"

#ifdef WIN32

bool simVis::isHeadless()
{
  // Windows is never headless
  return false;
}

#else

#include <X11/Xlib.h>

bool simVis::isHeadless()
{
  // UNIX systems that cannot XOpenDisplay() are considered headless
  Display* d = XOpenDisplay(nullptr);
  if (d != nullptr)
  {
    XCloseDisplay(d);
    return false;
  }
  return true;
}

#endif
