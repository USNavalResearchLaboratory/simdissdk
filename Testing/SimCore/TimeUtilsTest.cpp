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
#include "simCore/Time/Constants.h"
#include "simCore/Time/Utils.h"

namespace {

int timeStructDifferenceTest()
{
  int rv = 0;

  // test some tm struct differences
  const tm tmZero = {0};
  tm tmOne;
  tm tmTwo;

  // seconds overflow
  tmOne = tmZero;
  tmTwo = tmZero;
  tmOne.tm_sec = 61;
  tmTwo.tm_sec = 1;
  tmTwo.tm_min = 1;
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmOne, tmTwo) == 0);

  // year delta
  tmOne = tmZero;
  tmOne.tm_year = 1;
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmZero, tmOne) == simCore::daysPerYear(0) * simCore::SECPERDAY);

  // year overflow - 23:59:60 Dec 31, 1900 == 00:00:00 Jan 1, 1901
  tmOne = tmZero;
  tmTwo = tmZero;
  tmOne.tm_yday = 364;
  tmOne.tm_sec = 60;
  tmOne.tm_min = 59;
  tmOne.tm_hour = 23;
  tmTwo.tm_year = 1;
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmOne, tmTwo) == 0);

  // minutes overflow
  tmOne = tmZero;
  tmTwo = tmZero;
  tmOne.tm_sec = 59 * 60 + 60; // 00:59:60
  tmTwo.tm_hour = 1;
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmOne, tmTwo) == 0);

  // hours overflow
  tmOne = tmZero;
  tmTwo = tmZero;
  tmOne.tm_hour = 24; // 24:00:00
  tmTwo.tm_yday = 1;
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmOne, tmTwo) == 0);

  // test getTimeStruct() - add a minute to an epoch at 1900
  tmOne = simCore::getTimeStruct(60, 0);
  rv += SDK_ASSERT(simCore::getTimeStructDifferenceInSeconds(tmZero, tmOne) == 60);

  return rv;
}


int testIsValidDMY()
{
  int rv = 0;

  // Good dates
  rv += SDK_ASSERT(simCore::isValidDMY(10, 7, 1993)); // July 10, 1993
  rv += SDK_ASSERT(simCore::isValidDMY(25, 12, 2010)); // Dec 25, 2010
  rv += SDK_ASSERT(simCore::isValidDMY(1, 5, 2000)); // May 1, 2000
  rv += SDK_ASSERT(simCore::isValidDMY(14, 8, 1969)); // Aug 14, 1969

  // Bounds testing
  // General
  rv += SDK_ASSERT(!simCore::isValidDMY(1, 0, 1900)); // Invalid month
  rv += SDK_ASSERT(!simCore::isValidDMY(1, 13, 1900)); // Invalid month
  rv += SDK_ASSERT(!simCore::isValidDMY(1, 1, -1)); // Invalid year
  rv += SDK_ASSERT(!simCore::isValidDMY(1, 1, 0)); // Invalid year
  rv += SDK_ASSERT(!simCore::isValidDMY(1, 1, 1899)); // Invalid year

  // January
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 1, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 1, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 1, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 1, 1904)); // Invalid day, leap year

  // February
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 2, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 2, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(29, 2, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(30, 2, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(simCore::isValidDMY(28, 2, 1900)); // Valid day
  rv += SDK_ASSERT(simCore::isValidDMY(29, 2, 1904)); // Valid day, leap year

  // March
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 3, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 3, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 3, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 3, 1904)); // Invalid day, leap year

  // April
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 4, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 4, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 4, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 4, 1904)); // Invalid day, leap year

  // May
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 5, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 5, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 5, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 5, 1904)); // Invalid day, leap year

  // June
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 6, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 6, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 6, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 6, 1904)); // Invalid day, leap year

  // July
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 7, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 7, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 7, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 7, 1904)); // Invalid day, leap year

  // August
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 8, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 8, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 8, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 8, 1904)); // Invalid day, leap year

  // September
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 9, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 9, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 9, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 9, 1904)); // Invalid day, leap year

  // October
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 10, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 10, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 10, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 10, 1904)); // Invalid day, leap year

  // November
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 11, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 11, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 11, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(31, 11, 1904)); // Invalid day, leap year

  // December
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 12, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 12, 1900)); // Invalid day
  rv += SDK_ASSERT(!simCore::isValidDMY(0, 12, 1904)); // Invalid day, leap year
  rv += SDK_ASSERT(!simCore::isValidDMY(32, 12, 1904)); // Invalid day, leap year
  return rv;
}

}

int TimeUtilsTest(int argc, char *argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(timeStructDifferenceTest() == 0);
  rv += SDK_ASSERT(testIsValidDMY() == 0);

  return rv;
}

