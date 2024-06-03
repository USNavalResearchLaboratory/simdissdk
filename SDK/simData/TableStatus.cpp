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
#include "simData/TableStatus.h"

namespace simData
{

/// Private constructor
TableStatus::TableStatus()
{
}

/// Private constructor, with an error
TableStatus::TableStatus(const std::string& what)
  : what_(what)
{
}

/// Public copy constructor
TableStatus::TableStatus(const TableStatus& copyConstructor)
  : what_(copyConstructor.what_)
{
}

TableStatus TableStatus::Success()
{
  return TableStatus();
}

TableStatus TableStatus::Error(const std::string& what)
{
  return TableStatus(what);
}

std::string TableStatus::what() const
{
  return what_;
}

bool TableStatus::isError() const
{
  return !what_.empty();
}

bool TableStatus::isSuccess() const
{
  return what_.empty();
}

bool TableStatus::operator==(int unixIntTest) const
{
  return (unixIntTest == 0 && isSuccess()) ||
    (unixIntTest != 0 && isError());
}

bool TableStatus::operator==(const std::string& what) const
{
  return what == what_;
}

bool TableStatus::operator==(const TableStatus& st) const
{
  return operator==(st.what_);
}

bool TableStatus::operator!=(int unixIntTest) const
{
  return !operator==(unixIntTest);
}

bool TableStatus::operator!=(const std::string& what) const
{
  return !operator==(what);
}

bool TableStatus::operator!=(const TableStatus& st) const
{
  return !operator==(st);
}

TableStatus& TableStatus::operator=(const TableStatus& copy)
{
  if (this == &copy)
    return *this;
  what_ = copy.what_;
  return *this;
}

}
