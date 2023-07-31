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
#ifndef SIMCORE_TIME_EXCEPTION_H
#define SIMCORE_TIME_EXCEPTION_H

#include <string>

namespace simCore
{
  /** @brief Handles time input/output errors */
  class TimeException : public std::exception
  {
  public:
    int id;                   /**< TimeException identifier. */
    std::string description;  /**< TimeException description. */

    /// TimeException constructor
    /** Creates time exception handle.
    * @param[in ] ident An integer containing the exception identifier.
    * @param[in ] desc A string containing the description of the exception.
    */
    TimeException(int ident, const std::string& desc)
      : id(ident),
      description(desc)
    {
    }
    virtual ~TimeException() throw() {}

    virtual const char* what() const noexcept
    {
      return description.c_str();
    }
  };
} // simCore namespace

#endif  /* SIMCORE_TIME_EXCEPTION_H */
