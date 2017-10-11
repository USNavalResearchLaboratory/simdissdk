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
#ifndef SIMDATA_CATEGORY_DATA_H
#define SIMDATA_CATEGORY_DATA_H

#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Common/Memory.h"
#include "simData/DataTypes.h"

namespace simData {

/** interface to category data
 *
 * Category Data is a time-based name/value string pair.  In the interest of
 * performance, the strings also have numerical indexes.  CategoryNameManager is
 * responsible for the mapping between indexes and values (but this interface
 * provides either as needed).
 */
class CategoryDataPair
{
public:

  virtual ~CategoryDataPair()
  {
  }

  ///@return the category name as a string
  virtual std::string name() const = 0;
  ///@return the string value for the current category
  virtual std::string value() const = 0;

  ///@return the integer key for the category name
  virtual int nameInt() const = 0;
  ///@return the integer key for the value for the current category
  virtual int valueInt() const = 0;
};

/** interface to all the category data for an entity at a given time
 *
 * Any entity has a number of category data values at any time.  These values
 * might have been set recently, or might have been set a long time ago.
 *
 * Acts like an iterator, but also provides total dumps
 */
class SDKDATA_EXPORT CategoryDataSlice
{
protected: // types
  /** Interface to implementation of the iterator */
  class IteratorImpl
  {
  public:
    virtual ~IteratorImpl() {}
    /** Retrieves next item and increments iterator to next element */
    virtual std::shared_ptr<CategoryDataPair> next() = 0;
    /** Retrieves next item and does not increment iterator to next element */
    virtual std::shared_ptr<CategoryDataPair> peekNext() const = 0;
    /** Retrieves previous item and increments iterator to next element */
    virtual std::shared_ptr<CategoryDataPair> previous() = 0;
    /** Retrieves previous item and does not increment iterator to next element */
    virtual std::shared_ptr<CategoryDataPair> peekPrevious() const = 0;

    /** Resets the iterator to the front of the data structure */
    virtual void toFront() = 0;
    /** Sets the iterator to the end of the data structure */
    virtual void toBack() = 0;

    /** Returns true if next() / peekNext() will be a valid entry in the data slice */
    virtual bool hasNext() const = 0;
    /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
    virtual bool hasPrevious() const = 0;

    /** Create a copy of the actual implementation */
    virtual IteratorImpl* clone() const = 0;
  };

public: // types
  /// Get all the category data for the current time, one pair per callback
  class Visitor
  {
  public:
    /// called by CategoryDataSlice::visit()
    virtual void operator()(const CategoryData *update) = 0;

    virtual ~Visitor() {}
  };

  /** Iterator for category data, modeled after Qt and Java iteration
   *
   * Note that iteration is within the categories for a given time (not across
   * time).
   */
  class Iterator
  {
  public:
    ///@param[in] slice parent slice to iterator on
    explicit Iterator(const CategoryDataSlice *slice)
    : impl_(slice->iterator_())
    {
    }

    /// construct a reference from an existing iterator
    // Note: no clone here, to prevent memory loss
    explicit Iterator(IteratorImpl *impl)
    : impl_(impl)
    {
    }

    /// Copy constructor
    // (uses clone)
    Iterator(const Iterator &other)
      : impl_(other.impl_->clone())
    {
    }

    virtual ~Iterator() {}

    /** Retrieves next item and increments iterator to next element */
    const std::shared_ptr<CategoryDataPair> next() {return impl_->next();}
    /** Retrieves next item and does not increment iterator to next element */
    const std::shared_ptr<CategoryDataPair> peekNext() const {return impl_->peekNext();}
    /** Retrieves previous item and increments iterator to next element */
    const std::shared_ptr<CategoryDataPair> previous() {return impl_->previous();}
    /** Retrieves previous item and does not increment iterator to next element */
    const std::shared_ptr<CategoryDataPair> peekPrevious() const {return impl_->peekPrevious();}

    /** Resets the iterator to the front of the data structure */
    void toFront() {impl_->toFront();}
    /** Sets the iterator to the end of the data structure */
    void toBack() {impl_->toBack();};

    /** Returns true if next() / peekNext() will be a valid entry in the data slice */
    bool hasNext() const {return impl_->hasNext();}
    /** Returns true if previous() / peekPrevious() will be a valid entry in the data slice */
    bool hasPrevious() const {return impl_->hasPrevious();}

  private:
    // this class owns the iteratorImpl passed to it during construction
    std::unique_ptr<IteratorImpl> impl_;
  };

public:
  virtual ~CategoryDataSlice()
  {
  }

  ///@return last update time
  virtual double lastUpdateTime() const = 0;

  ///@return true if the category data changes
  virtual bool update(double time) = 0;

  /// receive all the category data for the last update time
  virtual void visit(Visitor *visitor) const = 0;

  ///@return an iterator for the current data
  virtual Iterator current() const = 0;

  /// retrieve all
  ///@{
  virtual void allNames(std::vector<std::string> &nameVec) const = 0;
  virtual void allValues(std::vector<std::string> &valueVec) const = 0;
  virtual void allStrings(std::vector<std::pair<std::string, std::string> > &nameValueVec) const = 0;

  virtual void allNameInts(std::vector<int> &nameIntVec) const = 0;
  virtual void allValueInts(std::vector<int> &valueIntVec) const = 0;
  virtual void allInts(std::vector<std::pair<int, int> > &nameValueIntVec) const = 0;
  ///@}

protected:
  /// used by the iterator implementation
  virtual std::unique_ptr<IteratorImpl> iterator_() const = 0;
};
} // namespace

#endif

