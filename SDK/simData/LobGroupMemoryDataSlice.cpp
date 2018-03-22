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
#include <limits>
#include "simData/MemoryDataSlice.h"

namespace simData
{

LobGroupMemoryDataSlice::LobGroupMemoryDataSlice()
  :MemoryDataSlice<LobGroupUpdate>(),
  maxDataPoints_(std::numeric_limits<int>::max()),
  maxDataSeconds_(std::numeric_limits<double>::max()),
  currentTime_(0.0)
{}


LobGroupMemoryDataSlice::~LobGroupMemoryDataSlice()
{
  // need to delete current_ since it is newed separately
  delete current_;
};


void LobGroupMemoryDataSlice::update(double time)
{
  // Mark as unchanged
  clearChanged();

  if (!dirty_ && currentTime_ == time)
    return;

  currentTime_ = time;
  dirty_ = false;

  // find the item just after the current time
  std::deque<LobGroupUpdate*>::const_iterator curTimeIter = std::upper_bound(updates_.begin(), updates_.end(), time, UpdateComp<LobGroupUpdate>());

  // find the start of the time window.  startTime is set to the desired time, so that
  // lower_bound returns the desired value, the next time >= than startTime
  double startTime = time - simCore::sdkMax(maxDataSeconds_, 0.0);
  std::deque<LobGroupUpdate*>::const_iterator startTimeIter = std::lower_bound(updates_.begin(), updates_.end(), startTime, UpdateComp<LobGroupUpdate>());

  // find the start of the point number window
  std::deque<LobGroupUpdate*>::const_iterator startNumIter;
  if (curTimeIter - updates_.begin() <= static_cast<int>(maxDataPoints_))
    startNumIter = updates_.begin();
  else
    startNumIter = curTimeIter - maxDataPoints_;

  // choose the more restrictive limit
  std::deque<LobGroupUpdate*>::const_iterator useIter;
  if ((startNumIter-updates_.begin()) > (startTimeIter-updates_.begin()))
    useIter = startNumIter;
  else
    useIter = startTimeIter;
  // Assertion failure means that the loop below is going to cause problems.  Assertion trigger means
  // we picked a start___Iter that is AFTER the current time, which shouldn't be feasible.
  assert(useIter <= curTimeIter);

  // create the new update
  LobGroupUpdate* currentUpdate = new LobGroupUpdate();
  currentUpdate->set_time(time);
  for (; useIter != curTimeIter; ++useIter)
  {
    // copy all points from each update record to the new current update
    for (int pointIndex = 0; pointIndex < (*useIter)->datapoints().size(); pointIndex++)
    {
      LobGroupUpdatePoint* newPoint = currentUpdate->add_datapoints();
      newPoint->CopyFrom((*useIter)->datapoints(pointIndex));
    }
  }

  // remove the old current_ object here, since we are replacing it
  delete current_;
  // only pass in the currentUpdate if it has data points
  if (currentUpdate->datapoints_size())
  {
    current_ = NULL;
    setCurrent(currentUpdate);
  }
  else // set current to NULL, need to call setCurrent to trigger update flag
  {
    delete currentUpdate;
    setCurrent(NULL);
  }
}

void LobGroupMemoryDataSlice::flush(bool keepStatic)
{
  if (MemorySliceHelper::flush(updates_, keepStatic) == 0)
  {
    delete current_;
    current_ = NULL;
  }
  dirty_ = true;
}

void LobGroupMemoryDataSlice::insert(LobGroupUpdate *data)
{
  // first, ensure that all data points have the time of the LobGroupUpdate they are associated with
  for (int pointIndex = 0; pointIndex < data->datapoints().size(); pointIndex++)
  {
    data->mutable_datapoints()->Mutable(pointIndex)->set_time(data->time());
  }

  std::deque<LobGroupUpdate*>::iterator iter = std::lower_bound(updates_.begin(), updates_.end(), data, UpdateComp<LobGroupUpdate>());
  if (iter != updates_.end() && (*iter)->time() == data->time())
  {
    // add to update record with same time
    for (int pointIndex = 0; pointIndex < data->datapoints().size(); pointIndex++)
    {
      LobGroupUpdatePoint* newPoint = (*iter)->add_datapoints();
      newPoint->CopyFrom(data->datapoints(pointIndex));
      data->mutable_datapoints()->RemoveLast();
    }
    // done with data, since we added its points to an existing update record
    delete data;
  }
  else
  {
    // no update record with this time, so insert new
    std::deque<LobGroupUpdate*>::iterator iter = std::upper_bound(updates_.begin(), updates_.end(), data, UpdateComp<LobGroupUpdate>());
    updates_.insert(iter, data);
  }
  dirty_ = true;
}


void LobGroupMemoryDataSlice::setMaxDataPoints(size_t maxDataPoints)
{
  if (maxDataPoints_ != maxDataPoints)
  {
    maxDataPoints_ = maxDataPoints;
    dirty_ = true;
  }
}

void LobGroupMemoryDataSlice::setMaxDataSeconds(double maxDataSeconds)
{
  if (maxDataSeconds_ != maxDataSeconds)
  {
    maxDataSeconds_ = maxDataSeconds;
    dirty_ = true;
  }
}

} // End of namespace simData

