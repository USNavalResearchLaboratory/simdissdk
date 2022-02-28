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
#ifndef SIMDATA_GENERICITERATOR_H
#define SIMDATA_GENERICITERATOR_H

#include <memory>

namespace simData
{

/** Implementation of iterator */
template <typename ValueType>
class GenericIteratorImpl
{
public:
  virtual ~GenericIteratorImpl() {}
  /** Retrieves next element and increments iterator to position after that element
  * @return Element after original iterator position,
  * or nullptr if no such element
  */
  virtual const ValueType next() = 0;
  /** Retrieves next element and does not change iterator position */
  virtual const ValueType peekNext() const = 0;
  /** Retrieves previous element and decrements iterator to position before that element
  * @return Element before original iterator position,
  * or nullptr if no such element
  */
  virtual const ValueType previous() = 0;
  /** Retrieves previous element and does not change iterator position */
  virtual const ValueType peekPrevious() const = 0;

  /** Resets the iterator to the front of the data structure, before the first element */
  virtual void toFront() = 0;
  /** Sets the iterator to the end of the data structure, after the last element */
  virtual void toBack() = 0;

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  virtual bool hasNext() const = 0;
  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  virtual bool hasPrevious() const = 0;

  /** Create a copy of the actual implementation */
  virtual GenericIteratorImpl<ValueType>* clone() const = 0;
};

/**
 * Implementation of GenericIteratorImpl that can be used for default-constructible objects.
 * This is useful because GenericIterator requires a valid GenericIteratorImpl in all cases,
 * and there may be conditions where a valid GenericIteratorImpl cannot be constructed.  In
 * these cases, pass in an instance of a NullIteratorImpl.
 */
template <typename ValueType>
class NullIteratorImpl : public GenericIteratorImpl<ValueType>
{
public:
  // Return a default-constructible instance of ValueType
  virtual const ValueType next() { return ValueType(); }
  virtual const ValueType peekNext() const { return ValueType(); }
  virtual const ValueType previous() { return ValueType(); }
  virtual const ValueType peekPrevious() const { return ValueType(); }

  // Noop, cannot move to front or back
  virtual void toFront() {}
  virtual void toBack() {}

  // No next, no previous
  virtual bool hasNext() const { return false; }
  virtual bool hasPrevious() const { return false; }

  // Clone another NullIteratorImpl
  virtual GenericIteratorImpl<ValueType>* clone() const { return new NullIteratorImpl<ValueType>(); }
};

/** Iterator for containers, modeled after Qt and Java iteration, returns time values */
template <typename ValueType>
class GenericIterator
{
public:
  /// Initializes with the given iterator implementation; accepts ownership of pointer through shared_ptr
  explicit GenericIterator(GenericIteratorImpl<ValueType>* impl)
    : impl_(impl)
  {
  }
  /// Copy constructor uses clone
  GenericIterator(const GenericIterator<ValueType>& other)
    : impl_(other.impl_->clone())
  {
  }
  /// Virtual destructor
  virtual ~GenericIterator() {}

  /** Retrieves next element and increments iterator to position after that element
  * @return Element after original iterator position,
  * or nullptr if no such element
  */
  const ValueType next() {return impl_->next();}
  /** Retrieves next element and does not change iterator position */
  const ValueType peekNext() const {return impl_->peekNext();}
  /** Retrieves previous element and decrements iterator to position before that element
  * @return Element before original iterator position,
  * or nullptr if no such element
  */
  const ValueType previous() {return impl_->previous();}
  /** Retrieves previous element and does not change iterator position */
  const ValueType peekPrevious() const {return impl_->peekPrevious();}

  /** Resets the iterator to the front of the data structure, before the first element */
  void toFront() {impl_->toFront();}
  /** Sets the iterator to the end of the data structure, after the last element */
  void toBack() {impl_->toBack();};

  /** Returns true if next() / peekNext() will be a valid entry in the data slice */
  bool hasNext() const {return impl_->hasNext();}
  /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
  bool hasPrevious() const {return impl_->hasPrevious();}

  /** Returns a pointer to the impl, which can be used to implement functionality
   * in containers that accept the iterators they generate.
   */
  GenericIteratorImpl<ValueType>* impl() const
  {
     return impl_.get();
  }
protected:
  /** Implementation for the iterator, used to implement the actual functionality */
  typename std::shared_ptr<GenericIteratorImpl<ValueType> > impl_;
};

}

#endif /* SIMDATA_GENERICITERATOR_H */
