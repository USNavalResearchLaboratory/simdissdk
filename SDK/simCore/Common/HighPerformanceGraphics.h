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
#ifndef SIMCORE_COMMON_HIGHPERFORMANCEGRAPHICS_H
#define SIMCORE_COMMON_HIGHPERFORMANCEGRAPHICS_H

/**
 * \file HighPerformanceGraphics.h
 * This utility file can be included in a static library or module linked to your
 * main application in order to enable high performance graphics over integrated
 * chipsets when supported by the end user's graphics hardware manufacturer.
 *
 * Both major chipset vendors NVidia and AMD have chipsets that collaborate with
 * integrated graphics (typically Intel HD in our experience), with NVidia or AMD
 * supplying the high performance chip.  End users can rely on graphics driver tools
 * to manually swap between integrated mode and high performance mode, and both
 * vendors support an 'Auto' selection mode which is default.
 *
 * When in 'Auto' selection, the low performance chip is used to conserve power,
 * unless an application is running that indicates it should use the high performance
 * chip instead.  NVidia and AMD determine this in a similar way on Windows systems,
 * by determining if the running application has a variable name in its exported
 * symbols table.  This header defines these variables for both chipsets to encourage
 * the selection of high performance graphics rendering.
 *
 * These symbols must be defined in your main application.  We have not had success
 * in defining them in shared objects (DLLs), so exposing these automatically in the
 * SIMDIS SDK may not work for your application (unless you statically link in the
 * SDK).  This header is provided for your use in your main applications, in order to
 * enable the High Performance GPU on systems that support switchable graphics.
 *
 * To enable:
 *
 * <code>
 * \#include "simCore/Common/HighPerformanceGraphics.h"
 * </code>
 */

#ifdef WIN32
extern "C" {

  /// Declare this variable in public to enable the NVidia side of Optimus - http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
  __declspec(dllexport) int NvOptimusEnablement = 1;

  /// Declare this variable in public to enable the AMD side of AMD Switchable Graphics (13.35 driver or newer needed) - http://devgurus.amd.com/thread/169965
  __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

}
#endif /* WIN32 */

#endif /* SIMCORE_COMMON_HIGHPERFORMANCEGRAPHICS_H */
