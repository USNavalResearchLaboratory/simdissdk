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
#ifndef SIMDATA_TABLESTATUS_H
#define SIMDATA_TABLESTATUS_H

#include <string>
#include "simCore/Common/Common.h"

namespace simData
{

/**
 * Represents the status return of a function or operation in explicit terms.
 * Success is defined in only one way, and multiple Errors can be defined, with
 * string descriptions to match the error context.  TableStatus is not meant
 * to be instantiated directly; instead, use the factory methods Success() and
 * Error().  For example, you can use this class like:
 *
 * <code>
 * return TableStatus::Success()
 * </code>
 *
 * Or:
 *
 * <code>
 * return TableStatus::Error("Invalid column ID.");
 * </code>
 */
class SDKDATA_EXPORT TableStatus
{
public:
  /** Factory method to create a successful status */
  static TableStatus Success();
  /** Factory method to create a descriptive error status */
  static TableStatus Error(const std::string& what);
  /** Copy constructor for status values */
  TableStatus(const TableStatus& copyConstructor);
  /** Assignment operator */
  TableStatus& operator=(const TableStatus& copy);

  /**
   * Returns true if UNIX-based error code (0 for success, non-zero for error condition)
   * matches the state of this status (isError, isSuccess).
   */
  bool operator==(int unixIntTest) const;
  /**
   * Returns true if the status is an error condition and the passed-in string matches
   * the error condition in this status.
   */
  bool operator==(const std::string& what) const;
  /**
   * Returns true if the other table status matches this table status.
   */
  bool operator==(const TableStatus& st) const;
  /**
   * Returns false if UNIX-based error code (0 for success, non-zero for error condition)
   * matches the state of this status (isError, isSuccess).
   */
  bool operator!=(int unixIntTest) const;
  /**
   * Returns false if the status is an error condition and the passed-in string matches
   * the error condition in this status.
   */
  bool operator!=(const std::string& what) const;
  /**
   * Returns false if the other table status matches this table status.
   */
  bool operator!=(const TableStatus& st) const;

  /** Retrieves the contents of the error message if isError(). */
  std::string what() const;
  /** Returns true if this status represents an error condition. */
  bool isError() const;
  /** Returns true if this status does not represent an error condition. */
  bool isSuccess() const;

private:
  /** Default constructor; private, use factory methods Success() and Error(). */
  TableStatus();
  /** Error-based constructor; private, use factory methods Success() and Error(). */
  explicit TableStatus(const std::string& what);

  std::string what_;
};

}

#endif /* SIMDATA_TABLESTATUS_H */
