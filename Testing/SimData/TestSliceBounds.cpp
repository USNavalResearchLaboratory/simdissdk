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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simCore/Common/SDKAssert.h"
#include "simData/LinearInterpolator.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

int testUpperBound(const simData::PlatformUpdateSlice& slice)
{
  int rv = 0;
  // 0.0: upper bound should be 1
  simData::PlatformUpdateSlice::Iterator i(slice.upper_bound(0.0));
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  const simData::PlatformUpdate* update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 1.0));

  // 1.0: upper bound should be 10
  i = slice.upper_bound(1.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 2.0: upper bound should be 10
  i = slice.upper_bound(2.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 10.0: upper bound should be 20
  i = slice.upper_bound(10.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 20.0));

  // 19.0: upper bound should be 20
  i = slice.upper_bound(19.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 20.0));

  // 20.0: upper bound should be [end]
  i = slice.upper_bound(20.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  // 21.0: upper bound should be [end]
  i = slice.upper_bound(21.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  return rv;
}

int testLowerBound(const simData::PlatformUpdateSlice& slice)
{
  int rv = 0;
  // 0.0: lower bound should be 1
  simData::PlatformUpdateSlice::Iterator i(slice.lower_bound(0.0));
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  const simData::PlatformUpdate* update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 1.0));

  // 1.0: lower bound should be 1
  i = slice.lower_bound(1.0);
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 1.0));

  // 2.0: lower bound should be 10
  i = slice.lower_bound(2.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 10.0: lower bound should be 10
  i = slice.lower_bound(10.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 19.0: lower bound should be 20
  i = slice.lower_bound(19.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 20.0));

  // 20.0: lower bound should be 20
  i = slice.lower_bound(20.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 20.0));

  // 21.0: lower bound should be [end]
  i = slice.lower_bound(21.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  return rv;
}

int testSingleItem(const simData::PlatformUpdateSlice& slice)
{
  int rv = 0;

  // 9.0: lower bound should be 10.0
  simData::PlatformUpdateSlice::Iterator i(slice.lower_bound(9.0));
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  const simData::PlatformUpdate* update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 9.0: upper bound should be 10.0
  i = slice.upper_bound(9.0);
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 10.0: lower bound should be 10.0
  i = slice.lower_bound(10.0);
  rv += SDK_ASSERT(!i.hasPrevious());
  rv += SDK_ASSERT(i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update != nullptr);
  rv += SDK_ASSERT(simCore::areEqual(update->time(), 10.0));

  // 10.0: upper bound should be [end]
  i = slice.upper_bound(10.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  // 11.0: lower bound should be [end]
  i = slice.lower_bound(11.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  // 11.0: upper bound should be [end]
  i = slice.upper_bound(11.0);
  rv += SDK_ASSERT(i.hasPrevious());
  rv += SDK_ASSERT(!i.hasNext());
  update = i.next();
  rv += SDK_ASSERT(update == nullptr);

  return rv;
}

static const double NO_VALUE = std::numeric_limits<double>::max();
double prevInclusiveTime(const simData::PlatformUpdateSlice& slice, double timeVal)
{
  simData::PlatformUpdateSlice::Iterator i(slice.upper_bound(timeVal));
  if (i.hasPrevious())
    return i.previous()->time();
  return NO_VALUE;
}

int testSinglePreviousInclusive(const simData::PlatformUpdateSlice& slice)
{
  int rv = 0;

  rv += SDK_ASSERT(prevInclusiveTime(slice, 9.0) == NO_VALUE);
  rv += SDK_ASSERT(simCore::areEqual(prevInclusiveTime(slice, 10.0), 10.0));
  rv += SDK_ASSERT(simCore::areEqual(prevInclusiveTime(slice, 11.0), 10.0));

  return rv;
}

int testInterp(simUtil::DataStoreTestHelper& helper)
{
  uint64_t id = helper.addPlatform();
  helper.addPlatformUpdate(10.0, id);
  helper.addPlatformUpdate(20.0, id);
  int rv = 0;
  rv += SDK_ASSERT(!helper.dataStore()->isInterpolationEnabled());

  helper.dataStore()->update(15.0);
  const simData::PlatformUpdateSlice* slice = helper.dataStore()->platformUpdateSlice(id);
  rv += SDK_ASSERT(simCore::areEqual(slice->current()->time(), 10.0));

  simData::LinearInterpolator interpolator;
  helper.dataStore()->setInterpolator(&interpolator);
  helper.dataStore()->enableInterpolation(true);
  rv += SDK_ASSERT(helper.dataStore()->isInterpolationEnabled());
  helper.dataStore()->update(15.0);
  slice = helper.dataStore()->platformUpdateSlice(id);
  rv += SDK_ASSERT(simCore::areEqual(slice->current()->time(), 15.0));
  return rv;
}

}

int TestSliceBounds(int argc, char* argv[])
{
  int rv = 0;

  simUtil::DataStoreTestHelper helper;
  uint64_t id = helper.addPlatform();
  helper.addPlatformUpdate(1.0, id);
  helper.addPlatformUpdate(10.0, id);
  helper.addPlatformUpdate(20.0, id);
  const simData::PlatformUpdateSlice* slice = helper.dataStore()->platformUpdateSlice(id);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr)
    return rv;
  // Test properties of the slice to make sure it's valid
  rv += SDK_ASSERT(slice->firstTime() == 1.0);
  rv += SDK_ASSERT(slice->lastTime() == 20.0);
  rv += SDK_ASSERT(slice->numItems() == 3);

  rv += testUpperBound(*slice);
  rv += testLowerBound(*slice);

  // Test single-point cases
  id = helper.addPlatform();
  helper.addPlatformUpdate(10.0, id);
  slice = helper.dataStore()->platformUpdateSlice(id);
  rv += SDK_ASSERT(slice != nullptr);
  if (slice == nullptr)
    return rv;
  // Test properties of the slice to make sure it's valid
  rv += SDK_ASSERT(slice->firstTime() == 10.0);
  rv += SDK_ASSERT(slice->lastTime() == 10.0);
  rv += SDK_ASSERT(slice->numItems() == 1);

  rv += testSingleItem(*slice);
  rv += testSinglePreviousInclusive(*slice);
  rv += testInterp(helper);

  return rv;
}
