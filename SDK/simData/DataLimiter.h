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
#ifndef SIMDATA_DATA_LIMITER_H
#define SIMDATA_DATA_LIMITER_H

// required for gcc to find size_t
#include <cstdlib>

namespace simData
{

/**
 * Implementation of a Data Limit algorithm that works for maps and multimaps.
 * It will limit by seconds or points.
 * This incarnation only erase()'s elements from the vector and is appropriate
 * for maps and multimaps that store values or references to values in the
 * data or iter->second place.  For dynamically allocated structures, see
 * DataLimiterDynamic, which deletes the memory.
 * The map key should be time, using either double or simCore::TimeStamp.
 * Example usage:
 * <code>
 *   std::map<double, DrawStyle> drawStyles;
 *   simData::DataLimiter<std::map<double, DrawStyle> > limiter;
 *   limiter.limitDataPoints(15);
 * </code>
 *  also:
 * <code>
 *   std::map<simCore::TimeStamp, DrawStyle> drawStyles;
 *   simData::DataLimiter<std::map<simCore::TimeStamp, DrawStyle> > limiter;
 *   limiter.limitDataSeconds(60.0);
 * </code>
 *
 * @tparam StlContainer Container type that is being limited.  This should be
 *   either a map or multimap; an associative container that supports iterators,
 *   such that iterator->first is the time associated with the data item.
 */
template<typename StlContainer>
class DataLimiter
{
public:
  virtual ~DataLimiter()
  {
  }

  /** Limit a container to the given number of points */
  void limitDataPoints(StlContainer &container, size_t limitPoints) const
  {
    limitPoints_(container, limitPoints);
  }

  /** Limit a container within the given number of seconds */
  void limitDataSeconds(StlContainer &container, double limitSeconds) const
  {
    limitSeconds_(container, limitSeconds);
  }

protected:
  /** You can override this in classes to be notified of an erasure from the map
   * or multimap STL container.  DataLimiterDynamic uses this to delete memory.
   * @param iter Iterator inside the STL container that is about to get erase()'d
   */
  virtual void deleteItem_(const typename StlContainer::iterator& iter) const
  {
    // You can override this method to delete the pointer value, if there is a pointer
  }

private:
  /** Helper to deleteItem_() on a range of data */
  void deleteRange_(const typename StlContainer::iterator &begin, const typename StlContainer::iterator &end) const
  {
    for (typename StlContainer::iterator i = begin; i != end; ++i)
      deleteItem_(i);
  }

  /** Limits the container to hold no more than maxNumPoints items */
  void limitPoints_(StlContainer &container, size_t maxNumPoints) const
  {
    // NOTE: This code will typically remove the DEFAULT time value of -1.  We could modify this
    // algorithm to not remove -1 times, or to not count -1 times, but there are some potentially
    // odd after-effects of this (i.e. either not removing the right number of points, or higher
    // probably of miscounting on removal, or more expensive algorithm).

    // Don't let the user limit us to 0 points
    if (container.size() <= maxNumPoints || maxNumPoints <= 0)
      return;

    // Figure out number to remove, and count down that value
    typename StlContainer::iterator iter = container.begin();
    for (size_t numToRemove = container.size() - maxNumPoints; numToRemove > 0; --numToRemove)
    {
      // Assertion failure indicates logic error in calculation of numToRemove
      assert(iter != container.end());
      ++iter;
    }

    // Remove the items from map; note that memory is statically allocated
    deleteRange_(container.begin(), iter);
    container.erase(container.begin(), iter);

    // Validate the algorithm with an assert
    assert(container.size() == maxNumPoints);
  }

  /** Limits the container to hold no more than maxSeconds data, as per looking at iter->first */
  void limitSeconds_(StlContainer &container, double maxSeconds) const
  {
    // Don't do data limiting with bad values
    if (maxSeconds < 0.0 || container.empty())
      return;

    // NOTE: This code will typically remove the DEFAULT time value of -1.  We could modify this
    // algorithm to not remove -1 times, or to not count -1 times, but there are some potentially
    // odd after-effects of this (i.e. either not removing the right number of points, or higher
    // probably of miscounting on removal, or more expensive algorithm).

    // If the time span is greater than the limit value, we limit
    if ((container.rbegin()->first - container.begin()->first) > maxSeconds)
    {
      typename StlContainer::iterator iter = container.lower_bound(container.rbegin()->first - maxSeconds);
      // This assertion could trigger only if maxSeconds is negative.  It's intended to help
      // keep this from erasing all elements.  If maxSeconds is 0, the lower_bound will return
      // the rbegin item, and erase() will keep it.
      assert(iter != container.end());
      deleteRange_(container.begin(), iter);
      container.erase(container.begin(), iter);
    }

    // Validate the algorithm with an assert; note the .empty() is to avoid a crash "just in case"
    assert(container.empty() || (container.rbegin()->first - container.begin()->first <= maxSeconds));
  }
};

/**
 * Implementation of a Data Limit algorithm similar to DataLimiterStatic<>,
 * except this one actually deletes the memory.  This must be implemented as
 * a separate class, else template errors will cause compile errors when the
 * delete function is called on a static structure.  The map key should be time.
 * For statically allocated structures, see DataLimiterStatic.
 * Example usage:
 *
 * <code>
 *   std::map<double, DataPoint*> drawStyles;
 *   UTILS::DataLimiterDynamic<std::map<double, DataPoint*> > limiter;
 *   limiter.limitData(UTILS::ePointsDataLimit, 15);
 * </code>
 *
 * @tparam StlContainer Container type that is being limited.  This should be
 *   either a map or multimap; an associative container that supports iterators,
 *   such that iterator->first is the time associated with the data item.
 */
template <typename StlContainer>
class DataLimiterDynamic : public DataLimiter<StlContainer>
{
protected:
  /** Delete the iter->second value */
  virtual void deleteItem_(const typename StlContainer::iterator &iter) const
  {
    delete iter->second;
  }
};

}

#endif

