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
#ifndef SIMVIS_HEADLESS_H
#define SIMVIS_HEADLESS_H

/** \file simVis/Headless.h
 * The purpose of this file is to provide the routine simVis::isHeadless().  This
 * routine will return true when running in a non-windowed environment, where
 * instantiating a window or windowed application would cause errors.  This is
 * most useful during unit testing, to prevent unit tests from failing in cases
 * where they need to instantiate a QApplication or other windowing structure.
 */

#ifdef WIN32

namespace simVis
{
  /** Returns true when executing in a headless display environment */
  inline bool isHeadless()
  { // Windows is never headless
    return false;
  }
}

#else

namespace simVis
{
  /** Returns true when executing in a headless display environment */
  bool isHeadless();
}

#endif

#endif /* SIMVIS_HEADLESS_H */
