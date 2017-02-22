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

#include "simCore/Common/SDKAssert.h"
#include "simUtil/DataStoreTestHelper.h"

namespace
{

int testDeltaTime()
{
  int rv = 0;

  simUtil::DataStoreTestHelper helper;
  uint64_t id = helper.addPlatform();

  const simData::PlatformUpdateSlice* slice = helper.dataStore()->platformUpdateSlice(id);

  // Empty slice should return -1;
  rv += SDK_ASSERT(slice->deltaTime(0.0) == -1.0);

  helper.addPlatformUpdate(1.0, id);

  // Before the first point should return -1
  rv += SDK_ASSERT(slice->deltaTime(0.0) == -1.0);

  // At the point should return 0.0
  rv += SDK_ASSERT(slice->deltaTime(1.0) == 0.0);

  // After the point should return delta
  rv += SDK_ASSERT(slice->deltaTime(2.0) == 1.0);

  // Add two more points and test again
  helper.addPlatformUpdate(2.0, id);
  helper.addPlatformUpdate(3.0, id);

  // Before the first point should return -1
  rv += SDK_ASSERT(slice->deltaTime(0.0) == -1.0);

  // Test between points
  rv += SDK_ASSERT(slice->deltaTime(1.5) == 0.5);
  rv += SDK_ASSERT(slice->deltaTime(2.5) == 0.5);

  // Test at points
  rv += SDK_ASSERT(slice->deltaTime(1.0) == 0.0);
  rv += SDK_ASSERT(slice->deltaTime(2.0) == 0.0);
  rv += SDK_ASSERT(slice->deltaTime(3.0) == 0.0);

  // Test after all point
  rv += SDK_ASSERT(slice->deltaTime(4.0) == 1.0);

  // Test with bad time
  rv += SDK_ASSERT(slice->deltaTime(-4.0) == -1.0);

  return rv;
}

void addPlatformUpdate(simData::DataStore* ds, uint64_t id, double time, double x, double y, double z)
{
  simData::DataStore::Transaction t;
  simData::PlatformUpdate *u = ds->addPlatformUpdate(id, &t);
  u->set_time(time);
  u->set_x(x);
  u->set_y(y);
  u->set_z(z);
  t.commit();
}


int duplicatePoints()
{
  int rv = 0;

  simUtil::DataStoreTestHelper helper;
  uint64_t id = helper.addPlatform();
  const simData::PlatformUpdateSlice* slice = helper.dataStore()->platformUpdateSlice(id);

  // Should start off empty
  rv += SDK_ASSERT(slice->numItems() == 0);

  simData::DataStore* ds = helper.dataStore();
  addPlatformUpdate(ds, id, 1.0, 2.0, 3.0, 4.0);
  rv += SDK_ASSERT(slice->numItems() == 1);

  // Verify first point
  ds->update(1.0);
  rv += SDK_ASSERT(slice->hasChanged());
  rv += SDK_ASSERT(slice->current()->time() == 1.0);
  rv += SDK_ASSERT(slice->current()->x() == 2.0);

  // Add duplicate point that should override
  addPlatformUpdate(ds, id, 1.0, 20.0, 3.0, 4.0);
  rv += SDK_ASSERT(slice->numItems() == 1);
  ds->update(1.0);
  rv += SDK_ASSERT(slice->hasChanged());
  rv += SDK_ASSERT(slice->current()->time() == 1.0);
  rv += SDK_ASSERT(slice->current()->x() == 20.0);

  // Add a few points
  addPlatformUpdate(ds, id, 2.0, 3.0, 3.0, 4.0);
  addPlatformUpdate(ds, id, 3.0, 4.0, 3.0, 4.0);
  addPlatformUpdate(ds, id, 4.0, 5.0, 3.0, 4.0);

  // Duplicate last point that should override
  addPlatformUpdate(ds, id, 4.0, 50.0, 3.0, 4.0);
  rv += SDK_ASSERT(slice->numItems() == 4);
  ds->update(4.0);
  rv += SDK_ASSERT(slice->hasChanged());
  rv += SDK_ASSERT(slice->current()->time() == 4.0);
  rv += SDK_ASSERT(slice->current()->x() == 50.0);

  // Duplicate middle point that should override
  addPlatformUpdate(ds, id, 3.0, 40.0, 3.0, 4.0);
  rv += SDK_ASSERT(slice->numItems() == 4);
  ds->update(3.0);
  rv += SDK_ASSERT(slice->hasChanged());
  rv += SDK_ASSERT(slice->current()->time() == 3.0);
  rv += SDK_ASSERT(slice->current()->x() == 40.0);

  return rv;
}

int testStaticPlatformUpdates()
{
  int rv = 0;
  simUtil::DataStoreTestHelper testHelper;

  // insert platform to establish scenario time bounds
  const uint64_t pID = testHelper.addPlatform();
  testHelper.addPlatformUpdate(0.0, pID);
  testHelper.addPlatformUpdate(10.0, pID);

  // insert static platform
  const uint64_t staticID = testHelper.addPlatform();
  testHelper.addPlatformUpdate(-1.0, staticID);

  simData::DataStore* ds = testHelper.dataStore();
  const simData::PlatformUpdateSlice* slice = ds->platformUpdateSlice(staticID);

  // Verify that only the first update results in slice hasChanged
  ds->update(1.0);
  rv += SDK_ASSERT(slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  ds->update(1.1);
  rv += SDK_ASSERT(!slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  ds->update(2.0);
  rv += SDK_ASSERT(!slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  ds->update(10.0);
  rv += SDK_ASSERT(!slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  ds->update(1.0);
  rv += SDK_ASSERT(!slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  ds->flush(staticID);
  ds->update(1.0);
  rv += SDK_ASSERT(!slice->hasChanged());
  rv += SDK_ASSERT(slice->current() != NULL);
  rv += SDK_ASSERT(slice->current()->time() == -1.0);

  return rv;
}

}

int TestMemorySlice(int argc, char* argv[])
{
  int rv = 0;

  rv += testDeltaTime();
  rv += duplicatePoints();
  rv += testStaticPlatformUpdates();

  return rv;
}
