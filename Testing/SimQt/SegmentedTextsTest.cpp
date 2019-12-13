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
#include <memory>
#include "simCore/Calc/Math.h"
#include "simCore/Common/SDKAssert.h"
#include "simQt/SegmentedTexts.h"

namespace {

// test results of precision adjustments on time in the SegmentedTexts
int testPrecision(simQt::SegmentedTexts& segs)
{
  int rv = 0;

  int refYear = 2012;
  double seconds = 60.;

  segs.setPrecision(5);

  // check setting time range
  simCore::TimeStamp startTime(refYear, seconds + 0.200009);
  simCore::TimeStamp endTime(refYear, seconds +  10.300001);
  segs.setTimeRange(refYear, startTime, endTime);
  int refYearOut;
  simCore::TimeStamp startTimeOut;
  simCore::TimeStamp endTimeOut;
  segs.timeRange(refYearOut, startTimeOut, endTimeOut);
  // time range values should be the same
  rv += SDK_ASSERT(refYear == refYearOut);
  rv += SDK_ASSERT(startTime == startTimeOut);
  rv += SDK_ASSERT(endTime == endTimeOut);

  // set time to start, which should be changed to > the original value, since it's been rounded to 5 decimal places
  segs.setTimeStamp(startTime);
  simCore::TimeStamp adjustedTime = segs.timeStamp();
  rv += SDK_ASSERT(adjustedTime > startTime);
  // check that rounding worked as expected
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getSeconds(), seconds));
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getFraction(), 0.20001));

  // set time to end, which should be changed to < original value, since it's been rounded to 5 decimal places
  segs.setTimeStamp(endTime);
  adjustedTime = segs.timeStamp();
  rv += SDK_ASSERT(adjustedTime < endTime);
  // check that rounding worked as expected
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getSeconds(), seconds + 10.));
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getFraction(), 0.3));

  // check normal time stamp in range, which requires no rounding
  simCore::TimeStamp inRange(refYear, seconds + 2.);
  segs.setTimeStamp(inRange);
  rv += SDK_ASSERT(segs.timeStamp() == inRange);

  // check the uncommon edge case, that a time slightly > than end time will still work, since it's accepted
  segs.setTimeStamp(simCore::TimeStamp(refYear, seconds + 10.300002));
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getSeconds(), seconds + 10.));
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear().getFraction(), 0.3));

  // now test with precision of 0
  segs.setPrecision(0);

  segs.setTimeStamp(startTime);
  // end time should have been rounded down, since no trailing digits with 0 precision
  adjustedTime = segs.timeStamp();
  rv += SDK_ASSERT(adjustedTime < startTime);
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear(), seconds));

  segs.setTimeStamp(endTime);
  adjustedTime = segs.timeStamp();
  // end time should have been rounded down, since no trailing digits with 0 precision
  rv += SDK_ASSERT(adjustedTime < endTime);
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear(), seconds + 10.));

  simCore::TimeStamp roundUp(refYear, seconds + 1.9);
  simCore::TimeStamp roundDown(refYear, seconds + 2.1);
  segs.setTimeStamp(roundUp);
  adjustedTime = segs.timeStamp();
  // time was rounded up, should be > original value
  rv += SDK_ASSERT(adjustedTime > roundUp);
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear(), seconds + 2.));

  segs.setTimeStamp(roundDown);
  adjustedTime = segs.timeStamp();
  // time was rounded down, should be < original value
  rv += SDK_ASSERT(adjustedTime < roundDown);
  rv += SDK_ASSERT(simCore::areEqual(adjustedTime.secondsSinceRefYear(), seconds + 2.));

  return rv;
}

}

int SegmentedTextsTest(int argc, char* argv[])
{
  int rv = 0;

  // test all the different SegmentedTexts implementations
  simQt::SecondsTexts secs;
  rv += testPrecision(secs);
  simQt::MinutesTexts mins;
  rv += testPrecision(mins);
  simQt::HoursTexts hrs;
  rv += testPrecision(hrs);
  simQt::OrdinalTexts ord;
  rv += testPrecision(ord);
  simQt::MonthDayYearTexts mon;
  rv += testPrecision(mon);

  return rv;
}
