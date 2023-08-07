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
#ifndef SIMCORE_COMMON_EXCEPTION_H
#define SIMCORE_COMMON_EXCEPTION_H

#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <string>
#include <cstdlib>

#include "simNotify/Notify.h"
#include "simCore/Common/Common.h"
#include "simCore/Time/String.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"

namespace simCore
{
#ifdef ENABLE_DEPRECATED_SIMDISSDK_API
  /// General exception class with error reporting.
  class Exception : public std::exception
  {
  public:
    /// Create a new exception.
    /**
    * Create a new exception.
    * @param[in ] file Prefix to the reason, usually supplied as __FILE__; describes the C++ module triggering the exception
    * @param[in ] reason A string providing additional information about the error.
    * @param[in ] line Line number on which the exception was thrown.
    */
    Exception(const std::string& file = "", const std::string& reason = "None", int line = 0)
      : std::exception(),
      err_(reason),
      rawReason_(reason),
      line_(line)
    {
      time_ = simCore::getSystemTime();
      std::ostringstream str;
      str << file << reason << " at line " << line;
      reason_ = str.str();
    }

    /**
    * Exception Destructor.
    * Exception displays its reason for failure to stderr on destruction.
    */
    virtual ~Exception() throw()
    {
      SIM_ERROR << err_ << std::endl;
    }

    /**
    * Get the string representing the type of error.
    * @return type of error on which the exception was thrown.
    */
    virtual const char* what() const throw()
    {
      return err_.c_str();
    }

    /** Returns the abbreviated what(), without line information */
    std::string rawWhat() const
    {
      return rawReason_;
    }

    /**
    * Get the line number on which the exception was thrown.
    * @return line number on which the exception was thrown.
    */
    int line() const
    {
      return line_;
    }

    /**
    * Get the time when the exception was thrown in seconds
    * since Jan 1 1970 the exception was thrown.
    * @return time in seconds
    */
    double time() const
    {
      return time_;
    }

  protected:
    std::string reason_;    ///< A string providing additional information about the error.
    std::string err_;       ///< String representing the type of error
    std::string rawReason_; ///< Abbreviated what(), without line information
    int line_;              ///< Line number on which the exception was thrown
    double time_;           ///< Time in seconds since Jan 1, 1970 the error occurred

  protected:
    /// Generate detailed information about the error.
    void addName_()
    {
      err_ = typeid(*this).name() + std::string(" : ") + reason_;
    }

  };
#endif

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

} // namespace simCore

#endif /* SIMCORE_COMMON_EXCEPTION_H */
