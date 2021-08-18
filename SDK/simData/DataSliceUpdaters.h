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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMDATA_DATASLICEUPDATERS_H
#define SIMDATA_DATASLICEUPDATERS_H

#include <algorithm>
#include "simCore/Calc/Math.h"
#include "simData/UpdateComp.h"
#include "simData/Interpolator.h"

namespace simData
{
/// How long to do a sequential search before giving up and do a complete search
const size_t FastSearchWidth = 3;

/// Like std::lower_bound, but uses currentIt to find quickly a neighboring location.
// (provides significant performance improvement when sequentially moving through time)
template <typename ForwardIterator, typename T>
ForwardIterator computeLowerBound(ForwardIterator begin, ForwardIterator currentIt, ForwardIterator end, double time)
{
  if (currentIt != end)
  {
    if ((*currentIt)->time() <= time)
    {
      for (size_t ii = 0; ii < FastSearchWidth && currentIt != end; ++ii, ++currentIt)
      {
        if ((*currentIt)->time() >= time)
          return currentIt;
      }
      if (currentIt == end)
        return end;
    }
    else
    {
      for (size_t ii = 0; ii < FastSearchWidth && currentIt != begin; ii++, currentIt--)
      {
        if ((*currentIt)->time() < time)
          return ++currentIt;
      }
    }
  }

 return std::lower_bound(begin, end, time, UpdateComp<T>());
}

/// Like std::upper_bound, but uses currentIt to find quickly a neighboring location.
// (provides significant performance improvement when sequentially moving through time)
template <typename ForwardIterator, typename T>
ForwardIterator computeUpperBound(ForwardIterator begin, ForwardIterator currentIt, ForwardIterator end, double time)
{
  if (currentIt != end)
  {
    if ((*currentIt)->time() <= time)
    {
      for (size_t ii = 0; ii < FastSearchWidth && currentIt != end; ++ii, ++currentIt)
      {
        if ((*currentIt)->time() > time)
          return currentIt;
      }
      if (currentIt == end)
        return end;
    }
    else
    {
      for (size_t ii = 0; ii < FastSearchWidth && currentIt != begin; ++ii, --currentIt)
      {
        if ((*currentIt)->time() <= time)
          return ++currentIt;
      }

      // Performance optimization: avoid upper_bound when before/at first time
      if (currentIt == begin)
      {
        if ((*currentIt)->time() <= time)
          return ++currentIt;  // First point is before the requested time, so return second point
        else
          return currentIt;  // First point is after the requested time, so return first point
      }
    }
  }

  return std::upper_bound(begin, end, time, UpdateComp<T>());
}

/** Update slices to the specified time */
template <typename ForwardIterator, typename T>
ForwardIterator computeTimeUpdate(ForwardIterator begin, ForwardIterator currentIt, ForwardIterator end, double time)
{
  if (begin == end)
  {
    return end;
  }

  currentIt = computeLowerBound<ForwardIterator, T>(begin, currentIt, end, time);

  // Current update is selected as the point <= to current time;
  if (currentIt == end)
  {
    // Closest update is the last point
    --currentIt;
    return currentIt;
  }

  if (time < (*currentIt)->time())
  {
    // Get the previous point
    if (currentIt != begin)
    {
      return --currentIt;
    }
    else
    {
      // Time is before the first point
      return end;
    }
  }
  else
  {
    // Assuming that not > from lower_bound and not < from previous if
    // means ==; probably need better == comparison for double
    // that is to be applied directly
    return currentIt;
  }

  return end;
}


class Interpolator;

/**
 * Update slices to the specified time, using interpolation as needed.
 * All pointers must point to valid values, nullptr values are not valid.
 */

template <typename ForwardIterator, typename T, typename B>
T *computeTimeUpdate(ForwardIterator begin, ForwardIterator& currentIt, ForwardIterator end, double time, Interpolator *interpolator, bool *isInterpolated, T *interpolatedPoint, B *bounds)
{
  assert(interpolator && isInterpolated && interpolatedPoint && bounds);

  if (begin == end)
  {
    *isInterpolated = false;
    *bounds = B(static_cast<T*>(nullptr), static_cast<T*>(nullptr));
    return nullptr;
  }

  currentIt = computeUpperBound<ForwardIterator, T>(begin, currentIt, end, time);

  // Current update is selected as the point <= to current time;
  // will be interpolated/extrapolated in the future
  // Find bounding points for update (when result is not end()
  if (currentIt == end)
  {
    // Closest update is the last point
    T *current = *(--currentIt);
    *isInterpolated = false;
    *bounds = B(static_cast<T*>(nullptr), static_cast<T*>(nullptr));
    return current;
  }

  // time is before the first point
  if (currentIt == begin)
  {
    *isInterpolated = false;
    *bounds = B(static_cast<T*>(nullptr), static_cast<T*>(nullptr));
    return nullptr;
  }

  // time is between points
  bounds->second = *currentIt;
  --currentIt;
  bounds->first = *currentIt;

  if (simCore::areEqual(time, (*currentIt)->time()))
  {
    *isInterpolated = false;
    *bounds = B(static_cast<T*>(nullptr), static_cast<T*>(nullptr));
    return (*currentIt);
  }

  interpolator->interpolate(time, *bounds->first, *bounds->second, interpolatedPoint);

  *isInterpolated = true;

  return interpolatedPoint;
}


} // End of namespace simData

#endif // SIMDATA_DATASLICEUPDATERS_H
