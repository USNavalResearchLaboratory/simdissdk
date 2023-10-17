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
#ifndef SIMCORE_SYSTEM_UTILS_H
#define SIMCORE_SYSTEM_UTILS_H

#include <string>
#include "simCore/Common/Export.h"

namespace simCore
{

/**
 * Returns the full absolute filename of the current process's executable.
 * @return Full executable absolute filename of the current process executable
 */
SDKCORE_EXPORT std::string getExecutableFilename();

/**
 * Returns the full absolute containing path of the current process's executable.
 * @return Full executable absolute containing path of the current process executable
 */
SDKCORE_EXPORT std::string getExecutablePath();

} // namespace simCore

#endif /* SIMCORE_SYSTEM_UTILS_H */
