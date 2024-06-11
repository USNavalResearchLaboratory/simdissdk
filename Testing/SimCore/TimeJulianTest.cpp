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
#include "simCore/Common/SDKAssert.h"
#include "simCore/Calc/Math.h"
#include "simCore/Time/Julian.h"

namespace {

int testDeltaT()
{
  int rv = 0;
  double tol = 1e-6;
  // Test values computed from USNO
  // 2008, 4, 24, 10:  65.89854284475079
  // 2018, 2, 20, 10:  69.11132526818395
  // 2019, 1, 1, 0:    69.4252287628706

  double fracSecsPerDay = 10. * 3600. / 86400.;

  // Compute Julian date based on a Universal Time (UT) scale.
  // add Julian date + fraction of day, then subtract one-half day since
  // the Julian and the calendar day start 12 hours apart.
  int jd12h = simCore::julianDate(2008, 4, 24);
  double timeUt = jd12h + fracSecsPerDay - 0.5;
  double deltaT = simCore::getDeltaT(timeUt);
  rv += SDK_ASSERT(simCore::areEqual(deltaT, 65.89854284475079, tol));

  jd12h = simCore::julianDate(2018, 2, 20);
  timeUt = jd12h + fracSecsPerDay - 0.5;
  deltaT = simCore::getDeltaT(timeUt);
  rv += SDK_ASSERT(simCore::areEqual(deltaT, 69.11132526818395, tol));

  jd12h = simCore::julianDate(2019, 1, 1);
  // no fractions of seconds for this date
  timeUt = jd12h - 0.5;
  deltaT = simCore::getDeltaT(timeUt);
  rv += SDK_ASSERT(simCore::areEqual(deltaT, 69.4252287628706, tol));

  return rv;
}

int testJulianDate()
{
  int rv = 0;

  // convert calendar date to Julian
  int jd12h = simCore::julianDate(2008, 4, 24);
  rv += SDK_ASSERT(jd12h == 2454581);
  // convert Julian date back to calendar
  int year;
  unsigned int month;
  unsigned int monthday;
  simCore::calendarDateFromJulianDate(jd12h, year, month, monthday);
  rv += SDK_ASSERT(year == 2008);
  rv += SDK_ASSERT(month == 4);
  rv += SDK_ASSERT(monthday == 24);

  jd12h = simCore::julianDate(2018, 2, 20);
  rv += SDK_ASSERT(jd12h == 2458170);
  simCore::calendarDateFromJulianDate(jd12h, year, month, monthday);
  rv += SDK_ASSERT(year == 2018);
  rv += SDK_ASSERT(month == 2);
  rv += SDK_ASSERT(monthday == 20);

  jd12h = simCore::julianDate(1970, 1, 1);
  rv += SDK_ASSERT(jd12h == 2440588);
  simCore::calendarDateFromJulianDate(jd12h, year, month, monthday);
  rv += SDK_ASSERT(year == 1970);
  rv += SDK_ASSERT(month == 1);
  rv += SDK_ASSERT(monthday == 1);

  return rv;
}

int testJulianCalendarDate()
{
  int rv = 0;

  // http://aa.usno.navy.mil/jdconverter
  //A.D. 2018 Feb 20	18:48:22.4	2458170.283593
  int year;
  unsigned int month;
  unsigned int monthday;
  int hour;
  int minute;
  double second;
  simCore::calendarDateFromJulianDate(2458170.283593, year, month, monthday, hour, minute, second);
  rv += SDK_ASSERT(year == 2018);
  rv += SDK_ASSERT(month == 2);
  rv += SDK_ASSERT(monthday == 20);
  rv += SDK_ASSERT(hour == 18);
  rv += SDK_ASSERT(minute == 48);
  rv += SDK_ASSERT(simCore::areEqual(second, 22.4, 1e-1));

  // from USNO NOVAS
  // computed Julian date and fraction of year
  // 2009, 6, 29, 9, 18, 44.58
  // 2455011.888015972 : 2009.491474016363
  double fyear;
  simCore::calendarDateFromJulianDate(2455011.888015972, fyear);
  rv += SDK_ASSERT(simCore::areEqual(fyear, 2009.491474016363));
  return rv;
}

int testJulianDayFrac()
{
  int rv = 0;

  // from USNO NOVAS
  // computed Julian date and fraction of year
  // 2009, 6, 29, 9, 18, 44.58
  // 2455011.888015972 : 2009  180.388015972495 days
  double jd = simCore::julianDate(2009, 180.388015972495);
  rv += SDK_ASSERT(simCore::areEqual(jd, 2455011.888015972, 1e-5));
  return rv;
}

}

int TimeJulianTest(int argc, char *argv[])
{
  int rv = 0;

  rv += SDK_ASSERT(testDeltaT() == 0);
  rv += SDK_ASSERT(testJulianDate() == 0);
  rv += SDK_ASSERT(testJulianCalendarDate() == 0);
  rv += SDK_ASSERT(testJulianDayFrac() == 0);

  return rv;
}
