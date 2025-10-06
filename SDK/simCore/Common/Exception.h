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
#ifndef SIMCORE_COMMON_EXCEPTION_H
#define SIMCORE_COMMON_EXCEPTION_H

#include <stdexcept>

#include "simNotify/Notify.h"
#include "simCore/Time/String.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"

#define SAFETRYBEGIN try \
  {
#define SAFETRYEND(exceptionText) } \
  catch (std::exception const& ex) \
  { \
  simCore::OrdinalTimeFormatter ordinalFormatter; \
  SIM_ERROR << "\n< STD EXC > " << ordinalFormatter.toString(simCore::TimeStamp(1970, simCore::getSystemTime()), 1970, 2) << " The following std exception was raised " << exceptionText << ":\n\t " << ex.what() << std::endl; \
  } \
  catch (...) \
  { \
  simCore::OrdinalTimeFormatter ordinalFormatter; \
  SIM_ERROR << "\n< UNKNOWN EXC > " << ordinalFormatter.toString(simCore::TimeStamp(1970, simCore::getSystemTime()), 1970, 2) << " An unexpected exception was raised " << exceptionText << "." << std::endl; \
  }
#define SAFETRYCATCH(function, exceptionText) SAFETRYBEGIN; \
  function; \
  SAFETRYEND(exceptionText)

#endif /* SIMCORE_COMMON_EXCEPTION_H */
