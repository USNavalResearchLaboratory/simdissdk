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
#ifndef SIMCORE_OPTIONAL_H
#define SIMCORE_OPTIONAL_H

namespace simCore {

/**
 * Template class that represents an optional item that may or may not be set.
 * Interface methods loosely based on C++17's std::optional.
 */
template <typename T>
class Optional
{
public:
  /** Default initialization */
  Optional()
    : value_(T()),
      isSet_(false)
  {
  }

  /** Returns true if the value is set */
  bool has_value() const
  {
    return isSet_;
  }

  /** Assignment operator */
  Optional<T>& operator=(const T& rhs)
  {
    value_ = rhs;
    isSet_ = true;
    return *this;
  }

  /** Destroy any contained value */
  void reset()
  {
    value_ = T();
    isSet_ = false;
  }

  /** Access via pointer notation */
  const T* operator->() const { return &value_; }
  T* operator->() { return &value_; }
  /** Access via dereference notation */
  const T& operator*() const { return value_; }
  T& operator*() { return value_; }

  /** Returns value_ if set, else returns provided value. */
  const T& value_or(const T& orValue) const { return isSet_ ? value_ : orValue; }

private:
  T value_;
  bool isSet_;
};

}

#endif /* SIMCORE_OPTIONAL_H */
