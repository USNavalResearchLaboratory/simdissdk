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

#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/SquareMatrix.h"

namespace {

simCore::SquareMatrix setMatrix(double r0c0, double r0c1, double r1c0, double r1c1)
{
  simCore::SquareMatrix m(2);
  m.set(0, 0, r0c0);
  m.set(0, 1, r0c1);
  m.set(1, 0, r1c0);
  m.set(1, 1, r1c1);

  return m;
}

bool areEqual(const simCore::SquareMatrix& m1, double r0c0, double r0c1, double r1c0, double r1c1)
{
  simCore::SquareMatrix m2 = setMatrix(r0c0, r0c1, r1c0, r1c1);
  return simCore::areEqual(m1, m2);
}

}

int SquareMatrixTest(int argc, char* argv[])
{
  int rv = 0;

  simCore::SquareMatrix m(2);
  // Should start as zero matrix
  rv += SDK_ASSERT(areEqual(m, 0.0, 0.0, 0.0, 0.0));

  // Check the dimension
  rv += SDK_ASSERT(m.dimension() == 2);

  // Check identity
  m.makeIdentity();
  rv += SDK_ASSERT(areEqual(m, 1.0, 0.0, 0.0, 1.0));

  // Check zero
  m.makeZero();
  rv += SDK_ASSERT(areEqual(m, 0.0, 0.0, 0.0, 0.0));

  // Make non-trivial
  m = setMatrix(1.0, 2.0, 3.0, 4.0);
  rv += SDK_ASSERT(areEqual(m, 1.0, 2.0, 3.0, 4.0));

  // Test get
  rv += SDK_ASSERT(simCore::areEqual(m.get(0, 0), 1.0));
  rv += SDK_ASSERT(simCore::areEqual(m.get(0, 1), 2.0));
  rv += SDK_ASSERT(simCore::areEqual(m.get(1, 0), 3.0));
  rv += SDK_ASSERT(simCore::areEqual(m.get(1, 1), 4.0));

  // Test row
  auto row = m.row(0);
  rv += SDK_ASSERT(row.size() == 2);
  if (row.size() == 2)
  {
    rv += SDK_ASSERT(simCore::areEqual(row[0], 1.0));
    rv += SDK_ASSERT(simCore::areEqual(row[1], 2.0));
  }
  row = m.row(1);
  rv += SDK_ASSERT(row.size() == 2);
  if (row.size() == 2)
  {
    rv += SDK_ASSERT(simCore::areEqual(row[0], 3.0));
    rv += SDK_ASSERT(simCore::areEqual(row[1], 4.0));
  }

  // Test column
  auto column = m.column(0);
  rv += SDK_ASSERT(column.size() == 2);
  if (column.size() == 2)
  {
    rv += SDK_ASSERT(simCore::areEqual(column[0], 1.0));
    rv += SDK_ASSERT(simCore::areEqual(column[1], 3.0));
  }
  column = m.column(1);
  rv += SDK_ASSERT(column.size() == 2);
  if (column.size() == 2)
  {
    rv += SDK_ASSERT(simCore::areEqual(column[0], 2.0));
    rv += SDK_ASSERT(simCore::areEqual(column[1], 4.0));
  }

  // Test scale
  m.scale(2.0);
  rv += SDK_ASSERT(areEqual(m, 2.0, 4.0, 6.0, 8.0));

  // Test add
  m.add(m);
  rv += SDK_ASSERT(areEqual(m, 4.0, 8.0, 12.0, 16.0));

  // Test transpose
  m.transpose();
  rv += SDK_ASSERT(areEqual(m, 4.0, 12.0, 8.0, 16.0));

  // Test postMultiply, calculations verified by hand
  m = setMatrix(1.0, 2.0, 3.0, 4.0);
  // Test that is possible to declare a SquareMatrix without a size
  simCore::SquareMatrix m2;
  m2 = setMatrix(5.0, 6.0, 7.0, 8.0);
  m.postMultiply(m2);
  rv += SDK_ASSERT(areEqual(m, 19.0, 22.0, 43.0, 50.0));

  // Test preMultiply, calculations verified by hand
  m = setMatrix(1.0, 2.0, 3.0, 4.0);
  m.preMultiply(m2);
  rv += SDK_ASSERT(areEqual(m, 23.0, 34.0, 31.0, 46.0));

  return rv;
}
