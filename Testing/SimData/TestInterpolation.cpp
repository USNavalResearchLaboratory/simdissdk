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
#include <iostream>

#include "simCore/Calc/CoordinateSystem.h"
#include "simCore/Calc/Units.h"
#include "simCore/Common/Version.h"
#include "simData/MemoryDataStore.h"
#include "simData/LinearInterpolator.h"
#include "simData/NearestNeighborInterpolator.h"
#include "simUtil/DataStoreTestHelper.h"

using namespace std;

namespace {
class InterpAssertionException : public std::exception
{
};

void assertTrue(bool value)
{
  if (!value)
  {
    throw InterpAssertionException();
  }
}

template <class T> void assertEquals(const T& expected, const T& actual)
{
  if (!(expected == actual))
  {
    throw InterpAssertionException();
  }
}

void assertEquals(const std::string& expected, const std::string& actual)
{
  if (!(expected == actual))
  {
    throw InterpAssertionException();
  }
}

template <class T> void assertNotEquals(const T& expected, const T& actual)
{
  if (expected == actual)
  {
    throw InterpAssertionException();
  }
}

void testInterpolation_enable()
{
  simData::MemoryDataStore ds;

  // no interpolation yet
  assertTrue(ds.canInterpolate());
  assertTrue(!ds.isInterpolationEnabled());

  // can't enable without an interpolator
  assertTrue(!ds.enableInterpolation(true));

  // set interpolator
  simData::LinearInterpolator interpolator;
  ds.setInterpolator(&interpolator);

  // setting interpolator alone doesn't enable interpolation
  assertTrue(!ds.isInterpolationEnabled());

  // enable interpolation
  assertTrue(ds.enableInterpolation(true));
  assertTrue(ds.isInterpolationEnabled());

  // unset interpolator
  ds.setInterpolator(nullptr);
  assertTrue(!ds.isInterpolationEnabled());

  // disabling should succeed
  assertTrue(!ds.enableInterpolation(false));
  assertTrue(!ds.isInterpolationEnabled());
}

void testInterpolation_nearest()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::DataStore::Transaction t;

  // setup interpolation
  simData::NearestNeighborInterpolator interpolator;
  ds->setInterpolator(&interpolator);
  ds->enableInterpolation(true);

  // insert platform
  uint64_t platId = testHelper.addPlatform();

  double satelliteHeight = simCore::Units::MILES.convertTo(simCore::Units::METERS, 22600.0);
  // insert data point at around satellite distance
  {
    simData::PlatformUpdate *u = ds->addPlatformUpdate(platId, &t);
    u->set_time(1.0);
    u->set_x(simCore::WGS_A + satelliteHeight + 10.0);
    u->set_y(11.0);
    u->set_z(12.0);
    t.commit();
  }

  // insert data point at around satellite distance
  {
    simData::PlatformUpdate *u = ds->addPlatformUpdate(platId, &t);
    u->set_time(2.0);
    u->set_x(simCore::WGS_A + satelliteHeight + 20.0);
    u->set_y(21.0);
    u->set_z(22.0);
    t.commit();
  }

  const simData::PlatformUpdateSlice *pslice = ds->platformUpdateSlice(platId);

  // nothing should exist before first datapoint
  ds->update(0.9);
  assertEquals(pslice->current(), (const simData::PlatformUpdate*)nullptr);
  assertTrue(!pslice->hasChanged());
  assertEquals(pslice->isInterpolated(), false);
  // test that re-updating datastore at same time does not signal a changed dataslice update (non-interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(0.9);
    assertTrue(!pslice->hasChanged());
  }

  // after end datapoint, we get nothing : "file mode" behavior
  ds->update(2.1);
  assertTrue(!pslice->hasChanged());  // if invalid before, and invalid after, no change.
  assertEquals(pslice->current(), (const simData::PlatformUpdate*)nullptr);
  assertEquals(pslice->isInterpolated(), false);
  // currently, an expired platform signals hasChanged at every update

  // at borders, should match the datapoints exactly
  ds->update(1.0);
  assertTrue(pslice->current() != nullptr);
  assertEquals(pslice->isInterpolated(), false);
  assertEquals(pslice->current()->time(), 1.0);

  // at borders, should match the datapoints exactly
  ds->update(2.0);
  assertTrue(pslice->current() != nullptr);
  assertTrue(pslice->hasChanged());
  assertEquals(pslice->isInterpolated(), false);
  assertEquals(pslice->current()->time(), 2.0);
  // test that re-updating datastore at same time does not signal a changed dataslice update (non-interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(2.0);
    assertTrue(!pslice->hasChanged());
  }

  // should match the nearest time when "interpolating"
  ds->update(1.4);
  assertTrue(pslice->current() != nullptr);
  assertTrue(pslice->hasChanged());
  assertEquals(pslice->isInterpolated(), true);
  assertEquals(pslice->current()->time(), 1.4);
  assertEquals(pslice->current()->x(), simCore::WGS_A + satelliteHeight + 10.0);
  // test that re-updating datastore at same time does not signal a changed dataslice update (interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(1.4);
    assertTrue(!pslice->hasChanged());
  }

  ds->update(1.6);
  assertTrue(pslice->current() != nullptr);
  assertEquals(pslice->isInterpolated(), true);
  assertEquals(pslice->current()->time(), 1.6);
  assertEquals(pslice->current()->x(), simCore::WGS_A + satelliteHeight + 20.0);
}

void testInterpolation_linear(simData::DataStore::InterpolatorState state)
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::PlatformPrefs objprefs;
  simData::DataStore::Transaction t;

  // setup interpolation
  simData::LinearInterpolator interpolator;
  ds->setInterpolator(&interpolator);
  ds->enableInterpolation(state);

  // insert platform
  uint64_t platId = testHelper.addPlatform();

  // insert data point near the earth's surface
  {
    simData::PlatformUpdate *u = ds->addPlatformUpdate(platId, &t);
    u->set_time(1.0);
    u->set_x(simCore::WGS_A + 10.0);
    u->set_y(11.0);
    u->set_z(12.0);
    t.commit();
  }

  // insert data point near the earth's surface
  {
    simData::PlatformUpdate *u = ds->addPlatformUpdate(platId, &t);
    u->set_time(2.0);
    u->set_x(simCore::WGS_A + 20.0);
    u->set_y(21.0);
    u->set_z(22.0);
    t.commit();
  }

  const simData::PlatformUpdateSlice *pslice = ds->platformUpdateSlice(platId);

  // nothing should exist before first datapoint or after end
  ds->update(0.9);
  assertEquals(pslice->current(), (const simData::PlatformUpdate*)nullptr);
  assertEquals(pslice->isInterpolated(), false);

  // after end datapoint, we get nothing : "file mode" behavior
  ds->update(2.1);
  assertEquals(pslice->current(), (const simData::PlatformUpdate*)nullptr);
  assertEquals(pslice->isInterpolated(), false);

  // at borders, should match the datapoints exactly
  ds->update(1.0);
  assertTrue(pslice->current() != nullptr);
  assertEquals(pslice->isInterpolated(), false);
  assertEquals(pslice->current()->time(), 1.0);

  // at borders, should match the datapoints exactly
  ds->update(2.0);
  assertTrue(pslice->current() != nullptr);
  assertEquals(pslice->isInterpolated(), false);
  assertEquals(pslice->current()->time(), 2.0);

  // should interpolate
  ds->update(1.5);
  assertTrue(pslice->hasChanged());
  assertTrue(pslice->current() != nullptr);
  assertEquals(pslice->isInterpolated(), true);
  assertEquals(pslice->current()->time(), 1.5);
  // test that re-updating datastore at same time does not signal a changed dataslice update (interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(1.5);
    assertTrue(!pslice->hasChanged());
  }
}

void testInterpolation_linearAngle()
{
  simUtil::DataStoreTestHelper testHelper;
  simData::DataStore* ds = testHelper.dataStore();
  simData::PlatformPrefs objprefs;

  // setup interpolation
  simData::LinearInterpolator interpolator;
  ds->setInterpolator(&interpolator);
  ds->enableInterpolation(true);
  assertTrue(ds->isInterpolationEnabled() == true);

  // insert platform
  uint64_t platId = testHelper.addPlatform();
  uint64_t laserId = testHelper.addLaser(platId);

  // insert data point
  {
    simData::DataStore::Transaction t;
    simData::PlatformUpdate *u = ds->addPlatformUpdate(platId, &t);
    u->set_time(1.0);
    u->set_x(simCore::WGS_A + 10.0);
    u->set_y(11.0);
    u->set_z(12.0);
    t.commit();
  }

  {
    // add valid datapoints
    simData::DataStore::Transaction t;
    simData::LaserUpdate *u = ds->addLaserUpdate(laserId, &t);
    assertTrue(u != nullptr);
    u->set_time(1.0);
    u->mutable_orientation()->set_yaw(0.0);
    u->mutable_orientation()->set_pitch(0.0);
    u->mutable_orientation()->set_roll(0.0);
    t.commit();
  }

  {
    // add valid datapoints
    simData::DataStore::Transaction t;
    simData::LaserUpdate *u = ds->addLaserUpdate(laserId, &t);
    assertTrue(u != nullptr);
    u->set_time(2.0);
    u->mutable_orientation()->set_yaw(0.5);
    u->mutable_orientation()->set_pitch(0.5);
    u->mutable_orientation()->set_roll(0.5);
    t.commit();
  }

  {
    // add valid datapoints
    simData::DataStore::Transaction t;
    simData::LaserUpdate *u = ds->addLaserUpdate(laserId, &t);
    assertTrue(u != nullptr);
    u->set_time(3.0);
    u->mutable_orientation()->set_yaw(M_TWOPI - 0.5);
    u->mutable_orientation()->set_pitch(1.0);
    u->mutable_orientation()->set_roll(1.0);
    t.commit();
  }

  const simData::LaserUpdateSlice *lslice = ds->laserUpdateSlice(laserId);
  assertTrue(lslice != nullptr);

  // nothing should exist before first datapoint
  ds->update(0.9);
  assertTrue(!lslice->hasChanged());
  assertEquals(lslice->current(), (const simData::LaserUpdate*)nullptr);
  assertEquals(lslice->isInterpolated(), false);
  // test that re-updating datastore at same time does not signal a changed dataslice update (interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(0.9);
    assertTrue(!lslice->hasChanged());
  }

  ds->update(3.1);
  // after end datapoint, we get the end datapoint
  assertEquals(lslice->current()->time(), 3.0);
  assertEquals(lslice->isInterpolated(), false);

  // at borders, should match the datapoints exactly
  ds->update(1.0);
  assertTrue(lslice->hasChanged());
  assertTrue(lslice->current() != nullptr);
  assertEquals(lslice->isInterpolated(), false);
  assertEquals(lslice->current()->time(), 1.0);
  // test that re-updating datastore at same time does not signal a changed dataslice update (interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(1.0);
    assertTrue(!lslice->hasChanged());
  }

  // at borders, should match the datapoints exactly
  ds->update(3.0);
  assertTrue(lslice->current() != nullptr);
  assertEquals(lslice->isInterpolated(), false);
  assertEquals(lslice->current()->time(), 3.0);

  // should interpolate
  ds->update(1.5);
  assertTrue(lslice->hasChanged());
  assertTrue(lslice->current() != nullptr);
  assertEquals(lslice->isInterpolated(), true);
  assertEquals(lslice->current()->time(), 1.5);
  assertTrue(simCore::areEqual(lslice->current()->orientation().yaw(), 0.25));
  assertTrue(simCore::areEqual(lslice->current()->orientation().pitch(), 0.25));
  assertTrue(simCore::areEqual(lslice->current()->orientation().roll(), 0.25));
  // test that re-updating datastore at same time does not signal a changed dataslice update (interpolated case)
  {
    // insert platform to dirty the datastore (but not affect the original platform data slice)
    testHelper.addPlatform();
    ds->update(1.5);
    assertTrue(!lslice->hasChanged());
  }

  // should interpolate
  ds->update(2.5);
  assertTrue(lslice->current() != nullptr);
  assertEquals(lslice->isInterpolated(), true);
  assertEquals(lslice->current()->time(), 2.5);
  assertTrue(simCore::areEqual(lslice->current()->orientation().yaw(), 0.0));
  assertTrue(simCore::areEqual(lslice->current()->orientation().pitch(), 0.75));
  assertTrue(simCore::areEqual(lslice->current()->orientation().roll(), 0.75));


  // disable interpolation
  assertTrue(ds->enableInterpolation(false) == false);
  assertTrue(ds->isInterpolationEnabled() == false);

  ds->update(2.6);
  // returns most recent (non-interpolated) datapoint
  assertTrue(lslice->current() != nullptr);
  assertEquals(lslice->current()->time(), 2.0);
  assertEquals(lslice->isInterpolated(), false);
}

}

int TestInterpolation(int argc, char* argv[])
{
  simCore::checkVersionThrow();
  try
  {
    testInterpolation_enable();
    testInterpolation_nearest();
    testInterpolation_linear(simData::DataStore::InterpolatorState::EXTERNAL);
    testInterpolation_linear(simData::DataStore::InterpolatorState::INTERNAL);
    testInterpolation_linearAngle();

    return 0;
  }
  catch (InterpAssertionException& e)
  {
    cout << e.what() << endl;
    return 1;
  }
}

