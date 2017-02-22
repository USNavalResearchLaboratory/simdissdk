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
#ifndef _STD_GL_H_
#define _STD_GL_H_


#ifdef WIN32
#include <windows.h>
#include "stdgl_windows.h"
#include <GL/glu.h>
#else
#if defined Linux
#include "stdgl_linux.h"
#elif defined IRIX
#include "stdgl_irix.h"
#elif defined Solaris
#include "stdgl_solaris.h"
#endif
#endif
#include "stdgl_constants.h"

#endif  /* _STD_GL_H_ */

