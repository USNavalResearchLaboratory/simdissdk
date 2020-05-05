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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
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
  Display* d = XOpenDisplay(NULL);
  if (d != NULL)
  {
    XCloseDisplay(d);
    return false;
  }
  return true;
}

#endif
