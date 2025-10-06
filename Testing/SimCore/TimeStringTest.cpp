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
#include "simCore/Calc/Math.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/String.h"
#include "simCore/Time/DeprecatedStrings.h"

namespace
{

int testTimeStringValidate()
{
  int rv = 0;
  int intVal;
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("1", 2004, intVal) && intVal == 1);
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("01", 2004, intVal) && intVal == 1);
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("001", 2004, intVal) && intVal == 1);
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("365", 2004, intVal) && intVal == 365);
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("365", 2005, intVal) && intVal == 365);
  rv += SDK_ASSERT(simCore::OrdinalTimeFormatter::isValidOrdinal("366", 2004, intVal) && intVal == 366);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("367", 2004, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("366", 2005, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("-1", 2005, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("0", 2005, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal(" 1", 2005, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("1 ", 2005, intVal) && intVal == 0);
  rv += SDK_ASSERT(!simCore::OrdinalTimeFormatter::isValidOrdinal("+1", 2005, intVal) && intVal == 0);
  return rv;
}

int testPrintSeconds()
{
  int rv = 0;
  simCore::SecondsTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "0.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "0.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "0.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "31536000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "-31536000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.234567), 1970, 2) == "1.23");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.234567), 1970, 3) == "1.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3.9), 1971, 0) == "4");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "-5");
  return rv;
}

int testPrintMinutes()
{
  int rv = 0;
  simCore::MinutesTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "0:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "0:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "0:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "0:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "525600:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "-525600:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 61.234567), 1970, 2) == "1:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 61.234567), 1970, 3) == "1:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "61:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "61:01.235");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3.9), 1971, 0) == "0:04");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), 1971, 0) == "8:00");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "-0:05");
  return rv;
}

int testPrintMinutesWrapped()
{
  int rv = 0;
  simCore::MinutesWrappedTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "0:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "0:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "0:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "0:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "0:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "0:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 61.234567), 1970, 2) == "1:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 61.234567), 1970, 3) == "1:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "1:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "1:01.235");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3.9), 1971, 0) == "0:04");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8 * 60 - 0.1), 1971, 0) == "8:00");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "-0:05");
  return rv;
}

int testPrintHours()
{
  int rv = 0;
  simCore::HoursTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "0:00:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "0:00:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "0:00:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "0:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "8760:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "-8760:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "1:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "1:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 90061.234567), 1970, 2) == "25:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 90061.234567), 1970, 3) == "25:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86461.234567), 1970, 2) == "24:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86461.234567), 1970, 3) == "24:01:01.235");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), 1971, 0) == "0:08:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), 1971, 0) == "1:00:00");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "-0:00:05");
  return rv;
}

int testPrintHoursWrapped()
{
  int rv = 0;
  simCore::HoursWrappedTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "0:00:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "0:00:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "0:00:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "0:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "0:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "0:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "1:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "1:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 90061.234567), 1970, 2) == "1:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 90061.234567), 1970, 3) == "1:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86461.234567), 1970, 2) == "0:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86461.234567), 1970, 3) == "0:01:01.235");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8 * 60 - 0.1), 1971, 0) == "0:08:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), 1971, 0) == "1:00:00");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "-0:00:05");
  return rv;
}

int testPrintOrdinal()
{
  int rv = 0;
  simCore::OrdinalTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "001 1970 00:00:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "001 1970 00:00:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "001 1970 00:00:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "001 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "001 1971 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "001 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "001 1970 01:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "001 1970 01:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400 + 3661.234567), 1970, 3) == "002 1970 01:01:01.235");
  // Test year rollover
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "001 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400-1), 1970, 0) == "001 1970 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365-1), 1970, 0) == "365 1970 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365), 1970, 0) == "001 1971 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366-1), 1970, 0) == "001 1971 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366), 1970, 0) == "002 1971 00:00:00");
  // Look at leap years
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 0), 1970, 0) == "001 1972 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400-1), 1970, 0) == "001 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*365-1), 1970, 0) == "365 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366-1), 1970, 0) == "366 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366), 1970, 0) == "001 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*367-1), 1970, 0) == "001 1973 23:59:59");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), 1971, 0) == "001 1971 00:08:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), 1971, 0) == "001 1971 01:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 24*3600 - 0.1), 1971, 0) == "002 1971 00:00:00");
  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "365 1970 23:59:55");
  return rv;
}

int testPrintMonthDay()
{
  int rv = 0;
  simCore::MonthDayTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "Jan 1 1970 00:00:00.000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "Jan 1 1970 00:00:00.00000");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "Jan 1 1970 00:00:00.0");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "Jan 1 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "Jan 1 1971 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "Jan 1 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "Jan 1 1970 01:01:01.23");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "Jan 1 1970 01:01:01.235");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400 + 3661.234567), 1970, 3) == "Jan 2 1970 01:01:01.235");
  // Test year rollover
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "Jan 1 1970 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400-1), 1970, 0) == "Jan 1 1970 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365-1), 1970, 0) == "Dec 31 1970 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365), 1970, 0) == "Jan 1 1971 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366-1), 1970, 0) == "Jan 1 1971 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366), 1970, 0) == "Jan 2 1971 00:00:00");
  // Look at leap years
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 0), 1970, 0) == "Jan 1 1972 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400-1), 1970, 0) == "Jan 1 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*365-1), 1970, 0) == "Dec 30 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366-1), 1970, 0) == "Dec 31 1972 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366), 1970, 0) == "Jan 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*367-1), 1970, 0) == "Jan 1 1973 23:59:59");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*58), 1970, 0) == "Feb 28 1972 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*59), 1970, 0) == "Feb 29 1972 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*60), 1970, 0) == "Mar 1 1972 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*58), 1970, 0) == "Feb 28 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*59), 1970, 0) == "Mar 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*60), 1970, 0) == "Mar 2 1973 00:00:00");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), 1971, 0) == "Jan 1 1971 00:08:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), 1971, 0) == "Jan 1 1971 01:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 24*3600 - 0.1), 1971, 0) == "Jan 2 1971 00:00:00");

  // Test each month string
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*0),   1970, 0) == "Jan 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*31),  1970, 0) == "Feb 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*59),  1970, 0) == "Mar 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*90),  1970, 0) == "Apr 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*120), 1970, 0) == "May 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*151), 1970, 0) == "Jun 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*181), 1970, 0) == "Jul 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*212), 1970, 0) == "Aug 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*243), 1970, 0) == "Sep 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*273), 1970, 0) == "Oct 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*304), 1970, 0) == "Nov 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*334), 1970, 0) == "Dec 1 1973 00:00:00");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*365), 1970, 0) == "Jan 1 1974 00:00:00");

  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "Dec 31 1970 23:59:55");
  return rv;
}

int testPrintDtg()
{
  int rv = 0;
  simCore::DtgTimeFormatter format;
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 3) == "010000:00.000 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 5) == "010000:00.00000 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 1) == "010000:00.0 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "010000:00 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 0), 1970, 0) == "010000:00 Z Jan71");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1971, 0) == "010000:00 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 2) == "010101:01.23 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), 1970, 3) == "010101:01.235 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400 + 3661.234567), 1970, 3) == "020101:01.235 Z Jan70");
  // Test year rollover
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), 1970, 0) == "010000:00 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400-1), 1970, 0) == "012359:59 Z Jan70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365-1), 1970, 0) == "312359:59 Z Dec70");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*365), 1970, 0) == "010000:00 Z Jan71");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366-1), 1970, 0) == "012359:59 Z Jan71");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 86400*366), 1970, 0) == "020000:00 Z Jan71");
  // Look at leap years
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 0), 1970, 0) == "010000:00 Z Jan72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400-1), 1970, 0) == "012359:59 Z Jan72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*365-1), 1970, 0) == "302359:59 Z Dec72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366-1), 1970, 0) == "312359:59 Z Dec72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*366), 1970, 0) == "010000:00 Z Jan73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*367-1), 1970, 0) == "012359:59 Z Jan73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*58), 1970, 0) == "280000:00 Z Feb72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*59), 1970, 0) == "290000:00 Z Feb72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1972, 86400*60), 1970, 0) == "010000:00 Z Mar72");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*58), 1970, 0) == "280000:00 Z Feb73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*59), 1970, 0) == "010000:00 Z Mar73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*60), 1970, 0) == "020000:00 Z Mar73");
  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), 1971, 0) == "010008:00 Z Jan71");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), 1971, 0) == "010100:00 Z Jan71");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 24*3600 - 0.1), 1971, 0) == "020000:00 Z Jan71");

  // Test each month string
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*0),   1970, 0) == "010000:00 Z Jan73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*31),  1970, 0) == "010000:00 Z Feb73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*59),  1970, 0) == "010000:00 Z Mar73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*90),  1970, 0) == "010000:00 Z Apr73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*120), 1970, 0) == "010000:00 Z May73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*151), 1970, 0) == "010000:00 Z Jun73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*181), 1970, 0) == "010000:00 Z Jul73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*212), 1970, 0) == "010000:00 Z Aug73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*243), 1970, 0) == "010000:00 Z Sep73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*273), 1970, 0) == "010000:00 Z Oct73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*304), 1970, 0) == "010000:00 Z Nov73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*334), 1970, 0) == "010000:00 Z Dec73");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1973, 86400*365), 1970, 0) == "010000:00 Z Jan74");

  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), 1971, 0) == "312359:55 Z Dec70");
  return rv;
}

int testPrintIso8601()
{
  int rv = 0;
  simCore::Iso8601TimeFormatter format;
  int ignoredRefYearArg = 0;

  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), ignoredRefYearArg) == "1970-01-01");

  // Iso8601TimeFormatter completely suppresses the hh:mm:ss if all zero
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), ignoredRefYearArg, 3) == "1970-01-01");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 0), ignoredRefYearArg, 5) == "1970-01-01");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 24*60*60), ignoredRefYearArg) == "1970-01-02");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 24*60*60), ignoredRefYearArg, 3) == "1970-01-02");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 24*60*60), ignoredRefYearArg, 5) == "1970-01-02");

  // when any of h, m or s is non-zero, Iso8601TimeFormatter does not suppress hh:mm:ss and honors precision
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1), ignoredRefYearArg) == "1970-01-01T00:00:01Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1), ignoredRefYearArg, 3) == "1970-01-01T00:00:01.000Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1), ignoredRefYearArg, 5) == "1970-01-01T00:00:01.00000Z");

  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.001), ignoredRefYearArg) == "1970-01-01T00:00:01Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.001), ignoredRefYearArg, 3) == "1970-01-01T00:00:01.001Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.001), ignoredRefYearArg, 4) == "1970-01-01T00:00:01.0010Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.001), ignoredRefYearArg, 5) == "1970-01-01T00:00:01.00100Z");

  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.0001), ignoredRefYearArg, 3) == "1970-01-01T00:00:01.000Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.0001), ignoredRefYearArg, 4) == "1970-01-01T00:00:01.0001Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.0001), ignoredRefYearArg, 5) == "1970-01-01T00:00:01.00010Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.00001), ignoredRefYearArg, 4) == "1970-01-01T00:00:01.0000Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 1.00001), ignoredRefYearArg, 5) == "1970-01-01T00:00:01.00001Z");

  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1970, 3661.234567), ignoredRefYearArg, 3) == "1970-01-01T01:01:01.235Z");

  // Note rounding
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 8*60 - 0.1), ignoredRefYearArg, 0) == "1971-01-01T00:08:00Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 3600 - 0.1), ignoredRefYearArg, 0) == "1971-01-01T01:00:00Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, 24*3600 - 0.1), ignoredRefYearArg, 0) == "1971-01-02");

  // Negative value
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), ignoredRefYearArg, 0) == "1970-12-31T23:59:55Z");
  rv += SDK_ASSERT(format.toString(simCore::TimeStamp(1971, -5.), ignoredRefYearArg, 5) == "1970-12-31T23:59:55.00000Z");

  return rv;
}

int canConvert(const std::string& timeString, bool expectSeconds, bool expectMinutes, bool expectHours,
               bool expectMonthDay, bool expectOrdinal, bool expectDtg, bool expectIso8601, const simCore::TimeStamp expectRef1971=simCore::TimeStamp(1970, 0))
{
  int rv = 0;
  simCore::NullTimeFormatter null;
  rv += SDK_ASSERT(null.canConvert(timeString) == false);
  simCore::SecondsTimeFormatter seconds;
  rv += SDK_ASSERT(seconds.canConvert(timeString) == expectSeconds);
  simCore::MinutesTimeFormatter minutes;
  rv += SDK_ASSERT(minutes.canConvert(timeString) == expectMinutes);
  simCore::HoursTimeFormatter hours;
  rv += SDK_ASSERT(hours.canConvert(timeString) == expectHours);
  simCore::MonthDayTimeFormatter monthDay;
  rv += SDK_ASSERT(monthDay.canConvert(timeString) == expectMonthDay);
  simCore::OrdinalTimeFormatter ordinal;
  rv += SDK_ASSERT(ordinal.canConvert(timeString) == expectOrdinal);
  simCore::DtgTimeFormatter dtg;
  rv += SDK_ASSERT(dtg.canConvert(timeString) == expectDtg);
  simCore::Iso8601TimeFormatter iso8601;
  rv += SDK_ASSERT(iso8601.canConvert(timeString) == expectIso8601);

  bool convertable = expectSeconds || expectMinutes || expectHours || expectMonthDay || expectOrdinal || expectDtg || expectIso8601;

  // Is the expected time set?  If so, test it
  if (expectRef1971.referenceYear() != 1970 || expectRef1971.secondsSinceRefYear() != 0)
  {
    simCore::TimeStamp gotTime;
    if (expectSeconds)
      rv += SDK_ASSERT(seconds.fromString(timeString, gotTime, 1971) == 0);
    if (expectMinutes)
      rv += SDK_ASSERT(minutes.fromString(timeString, gotTime, 1971) == 0);
    if (expectHours)
      rv += SDK_ASSERT(hours.fromString(timeString, gotTime, 1971) == 0);
    if (expectMonthDay)
      rv += SDK_ASSERT(monthDay.fromString(timeString, gotTime, 1971) == 0);
    if (expectOrdinal)
      rv += SDK_ASSERT(ordinal.fromString(timeString, gotTime, 1971) == 0);
    if (expectDtg)
      rv += SDK_ASSERT(dtg.fromString(timeString, gotTime, 1971) == 0);
    if (expectIso8601)
      rv += SDK_ASSERT(iso8601.fromString(timeString, gotTime, 1971) == 0);

    if (convertable)
      rv += SDK_ASSERT(gotTime == expectRef1971);
  }

  // Test the registry
  simCore::TimeFormatterRegistry registry;
  const simCore::TimeFormatter& formatter = registry.formatter(timeString);
  // Only test the value if one was passed in
  if (expectRef1971.referenceYear() != 1970 || expectRef1971.secondsSinceRefYear() != 0)
  {
    rv += SDK_ASSERT(formatter.canConvert(timeString)); // We should be expecting something back
    simCore::TimeStamp gotTime;
    rv += SDK_ASSERT(formatter.fromString(timeString, gotTime, 1971) == 0);
    rv += SDK_ASSERT(expectRef1971 == gotTime);
    gotTime = simCore::TimeStamp();
    // Test without using the formatter directly
    rv += SDK_ASSERT(registry.fromString(timeString, gotTime, 1971) == 0);
    rv += SDK_ASSERT(expectRef1971 == gotTime);
  }

  // Test the negative and that the values get set to invalid on error
  if (!formatter.canConvert(timeString))
  {
    simCore::TimeStamp gotTime(1972, 0);
    rv += SDK_ASSERT(formatter.fromString(timeString, gotTime, 1971) != 0);
    rv += SDK_ASSERT(gotTime.secondsSinceRefYear(1970) == 0);
    gotTime = simCore::TimeStamp(1972, 0);
    rv += SDK_ASSERT(registry.fromString(timeString, gotTime, 1971) != 0);
    rv += SDK_ASSERT(gotTime.secondsSinceRefYear(1970) == 0);
  }

  return rv;
}

int testPrintDeprecated()
{
  int rv = 0;
  // Lightly tested because of deprecation

  simCore::Deprecated::DDD_HHMMSS_YYYY_Formatter ddd_HHMMSS_YYYY;
  rv += SDK_ASSERT(ddd_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "002 10:17:36.70 1972");
  rv += SDK_ASSERT(ddd_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "002 10:17:37 1972");

  simCore::Deprecated::DDD_HHMMSS_Formatter ddd_HHMMSS;
  rv += SDK_ASSERT(ddd_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1972, 2) == "002 10:17:36.70");
  rv += SDK_ASSERT(ddd_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1972, 0) == "002 10:17:37");
  // Different reference year simply cannot be represented; falls back to ordinal format
  rv += SDK_ASSERT(ddd_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "002 1972 10:17:36.70");
  rv += SDK_ASSERT(ddd_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "002 1972 10:17:37");

  simCore::Deprecated::MON_MD_HHMMSS_YYYY_Formatter mon_MD_HHMMSS_YYYY;
  rv += SDK_ASSERT(mon_MD_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "Jan 2 10:17:36.70 1972");
  rv += SDK_ASSERT(mon_MD_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "Jan 2 10:17:37 1972");

  simCore::Deprecated::MD_MON_YYYY_HHMMSS_Formatter md_MON_YYYY_HHMMSS;
  rv += SDK_ASSERT(md_MON_YYYY_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "2 Jan 1972 10:17:36.70");
  rv += SDK_ASSERT(md_MON_YYYY_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "2 Jan 1972 10:17:37");

  // Note for following tests that January 2 1972 was a Monday

  simCore::Deprecated::WKD_MON_MD_HHMMSS_YYYY_Formatter wkd_MON_MD_HHMMSS_YYYY;
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "Mon Jan 2 10:17:36.70 1972");
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS_YYYY.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "Mon Jan 2 10:17:37 1972");

  simCore::Deprecated::WKD_MON_MD_HHMMSS_Formatter wkd_MON_MD_HHMMSS;
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1972, 2) == "Mon Jan 2 10:17:36.70");
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1972, 0) == "Mon Jan 2 10:17:37");
  // Different reference year simply cannot be represented; falls back to weekday format with year
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 2) == "Mon Jan 2 10:17:36.70 1972");
  rv += SDK_ASSERT(wkd_MON_MD_HHMMSS.toString(simCore::TimeStamp(1972, 123456.7), 1971, 0) == "Mon Jan 2 10:17:37 1972");

  return rv;
}

int canConvertTest()
{
  int rv = 0;
  // Good seconds
  rv += SDK_ASSERT(0 == canConvert("55.45", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 55.45)));
  rv += SDK_ASSERT(0 == canConvert("55.", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 55)));
  rv += SDK_ASSERT(0 == canConvert("55", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 55)));
  rv += SDK_ASSERT(0 == canConvert("-1.0", true, false, false, false, false, false, false, simCore::TimeStamp(1971, -1)));
  rv += SDK_ASSERT(0 == canConvert("-1", true, false, false, false, false, false, false, simCore::TimeStamp(1971, -1)));
  rv += SDK_ASSERT(0 == canConvert("\" 20.0\"", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 20)));
  rv += SDK_ASSERT(0 == canConvert("\" 20.0  \"", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 20)));
  rv += SDK_ASSERT(0 == canConvert("20", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 20)));
  rv += SDK_ASSERT(0 == canConvert("\" -1.0\"", true, false, false, false, false, false, false, simCore::TimeStamp(1971, -1)));
  rv += SDK_ASSERT(0 == canConvert("\" -1.0 \"", true, false, false, false, false, false, false, simCore::TimeStamp(1971, -1)));
  rv += SDK_ASSERT(0 == canConvert(".1", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 0.1)));
  rv += SDK_ASSERT(0 == canConvert("-.1", true, false, false, false, false, false, false, simCore::TimeStamp(1971, -0.1)));
  rv += SDK_ASSERT(0 == canConvert("+1.0", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 1)));
  rv += SDK_ASSERT(0 == canConvert("+125.2", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 125.2)));
  rv += SDK_ASSERT(0 == canConvert("+42", true, false, false, false, false, false, false, simCore::TimeStamp(1971, 42)));
  rv += SDK_ASSERT(0 == canConvert("+.5", true, false, false, false, false, false, false, simCore::TimeStamp(1971, .5)));
  // Bad seconds
  rv += SDK_ASSERT(0 == canConvert(".55.", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5.55.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5..55", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5,55", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5.+55", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5.-55", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5.55$", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("5.55:", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert(":55", false, false, false, false, false, false, false));

  // Good minutes
  rv += SDK_ASSERT(0 == canConvert("1:55.45", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 115.45)));
  rv += SDK_ASSERT(0 == canConvert("1:55.", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 115)));
  rv += SDK_ASSERT(0 == canConvert("1:55", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 115)));
  rv += SDK_ASSERT(0 == canConvert("\" 24:23.15 \"", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 1463.15)));
  rv += SDK_ASSERT(0 == canConvert("\"   24:23.15\"", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 1463.15)));
  rv += SDK_ASSERT(0 == canConvert("\"24:23.15  \"", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 1463.15)));
  rv += SDK_ASSERT(0 == canConvert("+2:00", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 120)));
  rv += SDK_ASSERT(0 == canConvert("+2:0.2", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 120.2)));
  rv += SDK_ASSERT(0 == canConvert("+2:24.5", false, true, false, false, false, false, false, simCore::TimeStamp(1971, 144.5)));
  // Bad minutes
  rv += SDK_ASSERT(0 == canConvert("++2:24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("+:24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert(":24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("24.5:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("24:", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("24:+0", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("24:.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("24:65.5", false, false, false, false, false, false, false));

  // Good hours
  rv += SDK_ASSERT(0 == canConvert("10:01:55.45", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36115.45)));
  rv += SDK_ASSERT(0 == canConvert("10:1:55.", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36115)));
  rv += SDK_ASSERT(0 == canConvert("10:1:55", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36115)));
  rv += SDK_ASSERT(0 == canConvert("\"  10:10:12.2 \"", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36612.2)));
  rv += SDK_ASSERT(0 == canConvert("\"10:10:12.2  \"", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36612.2)));
  rv += SDK_ASSERT(0 == canConvert("\"    10:10:12.2\"", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 36612.2)));
  rv += SDK_ASSERT(0 == canConvert("+31:00:00", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 111600)));
  rv += SDK_ASSERT(0 == canConvert("+31:0:0.32", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 111600.32)));
  rv += SDK_ASSERT(0 == canConvert("+31:43:0.13", false, false, true, false, false, false, false, simCore::TimeStamp(1971, 114180.13)));
  // Bad hours
  rv += SDK_ASSERT(0 == canConvert("++1:2:24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("+1::24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1::24.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:24.5:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:24:", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:24:+0", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:24:.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:24:65.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1::35.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("::35.5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("::", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1::", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:1:", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1::1", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:60:1", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:59:61", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1:5.9:5", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1.1:5:5", false, false, false, false, false, false, false));

  // Good ordinal
  rv += SDK_ASSERT(0 == canConvert("10 2004 10:1:55.45", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 9*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 2004 10:1:55.", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 9*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 9*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("010 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 9*86400 + 10*3600 + 1*60 + 55)));;
  rv += SDK_ASSERT(0 == canConvert("1 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 0*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("001 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 0*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("100 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 99*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("365 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 364*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("366 2004 10:1:55", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 365*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("1 2004 23:59:59", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 86399)));
  rv += SDK_ASSERT(0 == canConvert("\"   001   2004    10:14:05.5   \"", false, false, false, false, true, false, false,
    simCore::TimeStamp(2004, 0*86400 + 10*3600 + 14*60 + 5.5)));
  // Bad ordinal
  rv += SDK_ASSERT(0 == canConvert("0001 2004 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 1899 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("000 2004 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 11981 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 -1970 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 +1970 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("+10 1970 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("-10 1970 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 1970 -22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 1970 +22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 1970 24:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1.0 1970 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 1970.0 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 19.8 22:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("10 1970 220:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("366 2005 20:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("000 1971 10:00:00", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 1971 10:00:00 0", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("001 1971 00:00", false, false, false, false, false, false, false));

  // Good month/day
  rv += SDK_ASSERT(0 == canConvert("Jan 1 2004 00:01:00.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, 60.45)));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 2004 10:1:55", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("FEB 10 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("mar 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(2, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("APr 10 2004 10:1:55", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(3, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("MaY 10 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("jun 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(5, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("jul 10 2004 10:1:55", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(6, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("aug 10 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(7, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("sep 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(8, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("oct 10 2004 10:1:55", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(9, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("NOV 10 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(10, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Dec 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(11, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 2004 10:1:55.", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("\"  Jul  10    2004    10:01:01.5 \"", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(6, 10, 104)*86400 + 10*3600 + 1*60 + 1.5)));
  rv += SDK_ASSERT(0 == canConvert("Feb 29 2004 10:1:55.45", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 29, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Jan 02 2004 01:01:01", false, false, false, true, false, false, false,
    simCore::TimeStamp(2004, 90061)));
  // Bad month/day
  rv += SDK_ASSERT(0 == canConvert("January 10 2004 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Ja 10 2004 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Ja. 10 2004 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Jan 1.0 2004 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 2.04 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Jan 32 2.04 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Feb 29 2005 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Feb 028 2005 10:1:55.45", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Feb 28 2005 10:1:55.45 0", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Feb 28 2005 01:55", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("Feb 28 05 01:55:00", false, false, false, false, false, false, false));

  // Good DTG
  rv += SDK_ASSERT(0 == canConvert("010000:10 Z Jan13", false, false, false, false, false, true, false,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 113)*86400 + 0*3600 + 0*60 + 10)));
  rv += SDK_ASSERT(0 == canConvert("211505:30.5 Z Jan13", false, false, false, false, false, true, false,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 21, 113)*86400 + 15*3600 + 5*60 + 30.5)));
  rv += SDK_ASSERT(0 == canConvert("211505:30.5 Z FEB06", false, false, false, false, false, true, false,
    simCore::TimeStamp(2006, simCore::getYearDay(1, 21, 106)*86400 + 15*3600 + 5*60 + 30.5)));
  rv += SDK_ASSERT(0 == canConvert("020801:00. Z APR92", false, false, false, false, false, true, false,
    simCore::TimeStamp(1992, simCore::getYearDay(3, 2, 92)*86400 + 8*3600 + 1*60 + 0)));
  rv += SDK_ASSERT(0 == canConvert("301600:45 Z JUN12", false, false, false, false, false, true, false,
    simCore::TimeStamp(2012, simCore::getYearDay(5, 30, 112)*86400 + 16*3600 + 0*60 + 45)));
  rv += SDK_ASSERT(0 == canConvert("191934:14.123 Z FEB70", false, false, false, false, false, true, false,
    simCore::TimeStamp(1970, simCore::getYearDay(1, 19, 70)*86400 + 19*3600 + 34*60 + 14.123)));
  rv += SDK_ASSERT(0 == canConvert("191934:14.123 Z FEB69", false, false, false, false, false, true, false,
    simCore::TimeStamp(2069, simCore::getYearDay(1, 19, 69)*86400 + 19*3600 + 34*60 + 14.123)));
  rv += SDK_ASSERT(0 == canConvert("170249:51.5832 Z MAY04", false, false, false, false, false, true, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 17, 104)*86400 + 2*3600 + 49*60 + 51.5832)));
  rv += SDK_ASSERT(0 == canConvert("170249:59.5832 Z MAY04", false, false, false, false, false, true, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 17, 104)*86400 + 2*3600 + 49*60 + 59.5832)));
  rv += SDK_ASSERT(0 == canConvert("170259:51.5832 Z MAY04", false, false, false, false, false, true, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 17, 104)*86400 + 2*3600 + 59*60 + 51.5832)));
  rv += SDK_ASSERT(0 == canConvert("170249:51.5832   Z   MAY04", false, false, false, false, false, true, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 17, 104)*86400 + 2*3600 + 49*60 + 51.5832)));
  rv += SDK_ASSERT(0 == canConvert("291001:55.45 Z FEB04", false, false, false, false, false, true, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 29, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  // Bad DTG
  rv += SDK_ASSERT(0 == canConvert("170249:51 Z MAY+4", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("320249:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("290249:51 Z FEB05", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("-170249:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("000249:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("+170249:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("17+0249:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("1702+49:51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:+51 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:55.1. Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:5 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:5. Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:5.1 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170249:60 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170260:50 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("170250:60 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("172449:50 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("172.149:50 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("17214:50 Z MAY04", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("012014:50 Z MAY4", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("012014:50 z MAY14", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("012014:50 W MAY14", false, false, false, false, false, false, false));

  // Good ISO 8601
  // can also be converted by Seconds formatter, which could be a problem
  //rv += SDK_ASSERT(0 == canConvert("2013", true, false, false, false, false, false, true,
  //  simCore::TimeStamp(2013, simCore::ZERO_SECONDS)));
  rv += SDK_ASSERT(0 == canConvert("2013-01", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::ZERO_SECONDS)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::ZERO_SECONDS)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.1Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10.1)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.01Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10.01)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.001Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10.001)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.0001Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10.0001)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.000Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013)*86400 + 0*3600 + 0*60 + 10)));
  rv += SDK_ASSERT(0 == canConvert("2013-07-04T13:14:15.030Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(6, 4, 2013)*86400 + 13*3600 + 14*60 + 15.03)));

  // ISO 8601 with time zone.  See https://militarybenefits.info/military-time/ for zone info
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00A", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -1)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00B", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -2)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00C", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -3)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00D", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -4)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00E", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00F", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -6)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00G", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -7)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00H", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -8)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00I", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -9)));
  // "J" is local time and not tested since it depends on computer's local settings; just check validity
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00J", false, false, false, false, false, false, true));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00K", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -10)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00L", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -11)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00M", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -12)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00N", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 1)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00O", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 2)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00P", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 3)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00Q", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 4)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00R", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00S", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 6)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00T", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 7)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00U", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 8)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00V", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 9)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00W", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 10)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00X", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 11)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00Y", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 12)));
  // Do offset testing with year wraparound, for either side of the year boundary
  rv += SDK_ASSERT(0 == canConvert("2013-12-31T23:00:00W", false, false, false, false, false, false, true,
    simCore::TimeStamp(2014, 3600 * 9)));  // 2300 whiskey is 0900 zulu
  rv += SDK_ASSERT(0 == canConvert("2014-01-01T01:00:00D", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(11, 31, 2013) * 86400 + 3600 * 21)));   // 0100 delta is 2100 zulu

  // Test +/- time offsets
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00-05", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00-0500", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00-05:00", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00+05", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00+0500", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00+05:00", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -5)));
  rv += SDK_ASSERT(0 == canConvert("2013-01-02T00:00:00+05:30", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * -5.5)));

  // Test various "basic" formatted ISO 8601 formats
  auto canConvertIso8601 = [](const std::string& str, bool expectValid, const simCore::TimeStamp& equals = simCore::MIN_TIME_STAMP) -> int {
    int rv = 0;
    simCore::Iso8601TimeFormatter iso8601;
    rv += SDK_ASSERT(iso8601.canConvert(str) == expectValid);
    simCore::TimeStamp ts;
    rv += SDK_ASSERT((iso8601.fromString(str, ts, 1970) == 0) == expectValid);
    if (expectValid)
      rv += SDK_ASSERT(equals == ts);
    return rv;
  };
  rv += SDK_ASSERT(0 == canConvertIso8601("2", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("201", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("2013", true, simCore::TimeStamp(2013, 0.)));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("2013Z", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("201302", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("2013020", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203", true, simCore::TimeStamp(2013, 86400. * simCore::getYearDay(1, 3, 2013))));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203Z", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T0", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T00", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T01", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T012", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T0123", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T01234", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T012345", false));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T012345Z", true, simCore::TimeStamp(2013, 86400. * simCore::getYearDay(1, 3, 2013) + 3600. + 23 * 60. + 45.)));
  rv += SDK_ASSERT(0 == canConvertIso8601("20130203T012345z", false));
  // Repeat selected earlier tests with basic format
  rv += SDK_ASSERT(0 == canConvert("20130101T00:00:10.0001Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013) * 86400 + 0 * 3600 + 0 * 60 + 10.0001)));
  rv += SDK_ASSERT(0 == canConvert("20130101T000010.0001Z", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013) * 86400 + 0 * 3600 + 0 * 60 + 10.0001)));
  rv += SDK_ASSERT(0 == canConvert("20130102T000000V", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 9)));
  rv += SDK_ASSERT(0 == canConvert("20131231T230000W", false, false, false, false, false, false, true,
    simCore::TimeStamp(2014, 3600 * 9)));  // 2300 whiskey is 0900 zulu
  rv += SDK_ASSERT(0 == canConvert("20140101T010000D", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(11, 31, 2013) * 86400 + 3600 * 21)));   // 0100 delta is 2100 zulu
  // Repeat selected earlier tests with basic format, but quoted
  rv += SDK_ASSERT(0 == canConvert("\"20130101T00:00:10.0001Z\"", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013) * 86400 + 0 * 3600 + 0 * 60 + 10.0001)));
  rv += SDK_ASSERT(0 == canConvert("\"20130101T000010.0001Z\"", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 1, 2013) * 86400 + 0 * 3600 + 0 * 60 + 10.0001)));
  rv += SDK_ASSERT(0 == canConvert("\"20130102T000000V\"", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(0, 2, 2013) * 86400 + 3600 * 9)));
  rv += SDK_ASSERT(0 == canConvert("\"20131231T230000W\"", false, false, false, false, false, false, true,
    simCore::TimeStamp(2014, 3600 * 9)));  // 2300 whiskey is 0900 zulu
  rv += SDK_ASSERT(0 == canConvert("\"20140101T010000D\"", false, false, false, false, false, false, true,
    simCore::TimeStamp(2013, simCore::getYearDay(11, 31, 2013) * 86400 + 3600 * 21)));   // 0100 delta is 2100 zulu


  // Bad ISO 8601
  // no support for years before 1970 or after 2200
  rv += SDK_ASSERT(0 == canConvert("1969-01-01", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2201-01-01", false, false, false, false, false, false, false));

  rv += SDK_ASSERT(0 == canConvert("2013-", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T", false, false, false, false, false, false, false));

  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01t00:00:10Z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013:01:01T00:00:10Z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013.01.01T00:00:10Z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00-00-10Z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:100Z", false, false, false, false, false, false, false));
  rv += SDK_ASSERT(0 == canConvert("2013-01-01T00:00:10.Z", false, false, false, false, false, false, false));

  // Legacy ordinal with no year
  rv += SDK_ASSERT(0 == canConvert("001 00:00:00.00000", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 0)));
  rv += SDK_ASSERT(0 == canConvert("100 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("100 10:01:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("001 14:03:53.233", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 0*86400 + 14*3600 + 3*60 + 53.233)));
  rv += SDK_ASSERT(0 == canConvert("100 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("100 10:01:55.000", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("100 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("100 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, 99*86400 + 10*3600 + 1*60 + 55)));

  // Legacy ordinal with year at the end
  rv += SDK_ASSERT(0 == canConvert("103 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, 102*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("103 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, 102*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("103 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, 102*86400 + 10*3600 + 1*60 + 55)));

  // Legacy Month/Day format with year at end
  rv += SDK_ASSERT(0 == canConvert("Jan 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Jan 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("FEB 10 10:01:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("mar 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(2, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("APr 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(3, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("MaY 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("jun 10 10:01:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(5, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("jul 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(6, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("aug 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(7, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("sep 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(8, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("oct 10 10:01:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(9, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("NOV 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(10, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Dec 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(11, 10, 104)*86400 + 10*3600 + 1*60 + 55)));

  // Legacy Monthday/Month/Year format
  rv += SDK_ASSERT(0 == canConvert("10 Jan 2004 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 Jan 2004 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 Jan 2004 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 FEB 2004 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 mar 2004 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(2, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 APr 2004 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(3, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 MaY 2004 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 jun 2004 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(5, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 jul 2004 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(6, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 aug 2004 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(7, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 sep 2004 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(8, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 oct 2004 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(9, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("10 NOV 2004 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(10, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("10 Dec 2004 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(11, 10, 104)*86400 + 10*3600 + 1*60 + 55)));

  // Legacy Weekday format with year
  rv += SDK_ASSERT(0 == canConvert("MON Jan 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("WeD Jan 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("thu FEB 10 10:01:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(1, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("fRi mar 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(2, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("SAt APr 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(3, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("suN MaY 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(4, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Mon jun 10 10:01:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(5, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Tue jul 10 10:1:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(6, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Wed aug 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(7, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Thu sep 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(8, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Fri oct 10 10:01:55 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(9, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Sat NOV 10 10:1:55.45 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(10, 10, 104)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Sun Dec 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(11, 10, 104)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Tue Jan 10 10:1:55. 2004", false, false, false, false, false, false, false,
    simCore::TimeStamp(2004, simCore::getYearDay(0, 10, 104)*86400 + 10*3600 + 1*60 + 55)));

  // Legacy Weekday format without year
  rv += SDK_ASSERT(0 == canConvert("Tue Jan 10 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(0, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("MON Jan 10 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(0, 10, 71)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("WeD Jan 10 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(0, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("thu FEB 10 10:01:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(1, 10, 71)*86400 + 10*3600 + 1*60 + 55.45)));
  // Note: presuming no leap year due to 1971 reference year
  rv += SDK_ASSERT(0 == canConvert("fRi mar 10 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(2, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("SAt APr 10 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(3, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("suN MaY 10 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(4, 10, 71)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Mon jun 10 10:01:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(5, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Tue jul 10 10:1:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Wed aug 10 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(7, 10, 71)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Thu sep 10 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(8, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Fri oct 10 10:01:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(9, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Sat NOV 10 10:1:55.45", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(10, 10, 71)*86400 + 10*3600 + 1*60 + 55.45)));
  rv += SDK_ASSERT(0 == canConvert("Sun Dec 10 10:1:55.", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(11, 10, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Thr jul 8 10:01:55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 8, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("\"Thr Jul 8 10:01:55\"", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 8, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("\"\"\"Thu Jul 8 10:01:55\"\"\"", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 8, 71)*86400 + 10*3600 + 1*60 + 55)));
  rv += SDK_ASSERT(0 == canConvert("Thu  Jul   15     10:01:51.55", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 15, 71)*86400 + 10*3600 + 1*60 + 51.55)));
  rv += SDK_ASSERT(0 == canConvert("\"   Thu   Jul 10   10:01:51.55   \"", false, false, false, false, false, false, false,
    simCore::TimeStamp(1971, simCore::getYearDay(6, 10, 71)*86400 + 10*3600 + 1*60 + 51.55)));
  return rv;
}

int testFreeformTimeStr()
{
  struct TestCase
  {
    std::string input;
    bool expectedSuccess = false;
    simCore::FreeFormResult expected;
    std::string description;
  };

  auto intStr = [](const std::optional<int>& opt) { if (opt) return std::to_string(*opt); return std::string("{}"); };
  auto doubleStr = [](const std::optional<double>& opt) { if (opt) return std::to_string(*opt); return std::string("{}"); };

  auto runTest = [&](const TestCase& test)
    {
      const simCore::FreeFormResult result = simCore::parseFreeFormTimeStr(test.input);

      if (result.isValid() != test.expectedSuccess)
      {
        std::cout << "  FAIL: \"" << test.description << "\" expected " << (test.expectedSuccess ? "success" : "failure")
          << " but got " << (result.isValid() ? "success" : "failure") << "\n";
        return 1;
      }

      if (result.isValid() && test.expectedSuccess)
      {
        if (result != test.expected)
        {
          std::cout << "  FAIL: \"" << test.description << "\" expected (" << intStr(test.expected.hours) << ":"
            << intStr(test.expected.minutes) << ":" << doubleStr(test.expected.seconds) << ") but got ("
            << intStr(result.hours) << ":" << intStr(result.minutes) << ":" << doubleStr(result.seconds) << ")\n";
          return 1;
        }
      }

      return 0;
    };

  int rv = 0;

  // HH:MM:SS.sss format
  rv += SDK_ASSERT(runTest({ "12:34:56.789", true, { 12, 34, 56.789 }, "Standard HH:MM:SS.sss format" }) == 0);
  rv += SDK_ASSERT(runTest({ "01:02:03.1", true, { 1, 2, 3.1 }, "HH:MM:SS with single fractional digit" }) == 0);
  rv += SDK_ASSERT(runTest({ "23:59:59.999", true, { 23, 59, 59.999 }, "Maximum valid time with fractions" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:56.123456", true, { 12, 34, 56.123456 }, "HH:MM:SS with microsecond precision" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:56.", true, { 12, 34, 56.0 }, "Trailing dot with no fractional part" }) == 0);

  // HH:MM:SS format (no fractions)
  rv += SDK_ASSERT(runTest({ "12:34:56", true, { 12, 34, 56.0 }, "Standard HH:MM:SS format" }) == 0);
  rv += SDK_ASSERT(runTest({ "00:00:00", true, { 0, 0, 0.0 }, "Midnight" }) == 0);

  // HHMMSS format
  rv += SDK_ASSERT(runTest({ "123456", true, { 12, 34, 56.0 }, "Compact HHMMSS format" }) == 0);
  rv += SDK_ASSERT(runTest({ "000000", true, { 0, 0, 0.0 }, "Compact midnight" }) == 0);
  rv += SDK_ASSERT(runTest({ "235959", true, { 23, 59, 59.0 }, "Compact maximum time" }) == 0);

  // HMMSS format
  rv += SDK_ASSERT(runTest({ "12345", true, { 1, 23, 45.0 }, "Compact HMMSS format" }) == 0);
  rv += SDK_ASSERT(runTest({ "00000", true, { 0, 0, 0.0 }, "Compact HMMSS midnight" }) == 0);

  // MMSS format
  rv += SDK_ASSERT(runTest({ "1234", true, { {}, 12, 34.0 }, "Compact MMSS format" }) == 0);
  rv += SDK_ASSERT(runTest({ "1234.56", true, { {}, 12, 34.56 }, "Compact MMSS format fractional" }) == 0);
  rv += SDK_ASSERT(runTest({ "0000", true, { {}, 0, 0.0 }, "Compact MMSS midnight" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34", true, { {}, 12, 34.0 }, "Separated MM:SS format" }) == 0);

  // MSS format
  rv += SDK_ASSERT(runTest({ "123", true, { {}, 1, 23.0 }, "Compact MSS format" }) == 0);
  rv += SDK_ASSERT(runTest({ "000", true, { {}, 0, 0.0 }, "Compact MSS midnight" }) == 0);
  rv += SDK_ASSERT(runTest({ "123.45", true, { {}, 1, 23.45 }, "Compact MSS.ss format" }) == 0);

  // SS format
  rv += SDK_ASSERT(runTest({ "12", true, { {}, {}, 12.0 }, "SS seconds format" }) == 0);
  rv += SDK_ASSERT(runTest({ "12.34", true, { {}, {}, 12.34 }, "SS.ss seconds format with fractional" }) == 0);
  rv += SDK_ASSERT(runTest({ "2", true, { {}, {}, 2.0 }, "S seconds format" }) == 0);
  rv += SDK_ASSERT(runTest({ "2.34", true, { {}, {}, 2.34 }, "S.ss seconds format with fractional" }) == 0);
  rv += SDK_ASSERT(runTest({ ".123", true, { {}, {}, 0.123 }, "Leading dot only" }) == 0);

  // HHMMSS.s format
  rv += SDK_ASSERT(runTest({ "123456.5", true, { 12, 34, 56.5 }, "Compact HHMMSS.s format" }) == 0);
  rv += SDK_ASSERT(runTest({ "000000.123", true, { 0, 0, 0.123 }, "Compact with fractional seconds" }) == 0);
  rv += SDK_ASSERT(runTest({ "12345.75", true, { 1, 23, 45.75 }, "Compact HMMSS.s format" }) == 0);
  rv += SDK_ASSERT(runTest({ "123456.", true, { 12, 34, 56.0 }, "Compact trailing dot with no fractional part" }) == 0);

  // Space separated formats
  rv += SDK_ASSERT(runTest({ "12 34 56.5", true, { 12, 34, 56.5 }, "Space separated HH MM SS.s" }) == 0);
  rv += SDK_ASSERT(runTest({ "1 23 45", true, { 1, 23, 45.0 }, "Space separated H MM SS" }) == 0);
  rv += SDK_ASSERT(runTest({ "12 34", true, { {}, 12, 34.0 }, "Space separated MM SS" }) == 0);

  // Comma separated formats
  rv += SDK_ASSERT(runTest({ "12,34,56", true, { 12, 34, 56.0 }, "Comma separated" }) == 0);
  rv += SDK_ASSERT(runTest({ "12,34,56.789", true, { 12, 34, 56.789 }, "Comma separated with fractions" }) == 0);
  rv += SDK_ASSERT(runTest({ "12,34", true, { {}, 12, 34.0 }, "Comma separated MM,SS" }) == 0);

  // Mixed separators
  rv += SDK_ASSERT(runTest({ "12:34,56", true, { 12, 34, 56.0 }, "Mixed separators : and ," }) == 0);
  rv += SDK_ASSERT(runTest({ "12 34:56.5", true, { 12, 34, 56.5 }, "Mixed separators with fractions" }) == 0);
  rv += SDK_ASSERT(runTest({ "12,34:56.25", true, { 12, 34, 56.25 }, "Mixed separators , and : with decimal" }) == 0);

  // High precision fractional seconds
  rv += SDK_ASSERT(runTest({ "12:34:56.001", true, { 12, 34, 56.001 }, "1 millisecond" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:56.000001", true, { 12, 34, 56.000001 }, "1 microsecond" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:56.000000001", true, { 12, 34, 56.000000001 }, "1 nanosecond" }) == 0);

  // Invalid cases - fractional minutes/hours NOT supported
  rv += SDK_ASSERT(runTest({ "12.5 34", false, {}, "Space separated HH.s MM (fractional hours - NOT supported)" }) == 0);
  rv += SDK_ASSERT(runTest({ "12.5:34", false, {}, "Separated HH.s:MM (fractional hours - NOT supported)" }) == 0);
  rv += SDK_ASSERT(runTest({ "12 34.5 56", false, {}, "Separated HH:MM.s:SS (fractional minutes - NOT supported)" }) == 0);
  rv += SDK_ASSERT(runTest({ "12 34.5 56", false, {}, "Separated HH MM.s SS (fractional minutes - NOT supported)" }) == 0);

  // Other invalid cases
  rv += SDK_ASSERT(runTest({ "", false, {}, "Empty string" }) == 0);
  rv += SDK_ASSERT(runTest({ "25:00:00", false, {}, "Invalid hours" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:60:00", false, {}, "Invalid minutes" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:60", false, {}, "Invalid seconds" }) == 0);
  rv += SDK_ASSERT(runTest({ "70", false, {}, "Invalid seconds compact" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:56:78", false, {}, "Too many components" }) == 0);
  rv += SDK_ASSERT(runTest({ "abc:def:ghi", false, {}, "Non-numeric input" }) == 0);
  rv += SDK_ASSERT(runTest({ "1234567", false, {}, "Too many digits for compact format" }) == 0);
  rv += SDK_ASSERT(runTest({ "12.34.56", false, {}, "Multiple dots (dots are not valid separators)" }) == 0);
  rv += SDK_ASSERT(runTest({ "12.34.56.789", false, {}, "Multiple dots fractional (dots are not valid separators)" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34.56.789", false, {}, "Multiple dots in seconds field" }) == 0);

  // Additional edge cases
  rv += SDK_ASSERT(runTest({ ":12:34", false, {}, "Empty hours" }) == 0);
  rv += SDK_ASSERT(runTest({ "12::34", false, {}, "Empty minutes" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:", false, {}, "Empty seconds" }) == 0);
  rv += SDK_ASSERT(runTest({ "12:34:.5", false, {}, "Empty seconds with fractional" }) == 0);

  return rv;
}

}

int TimeStringTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testTimeStringValidate() == 0);
  rv += SDK_ASSERT(testPrintSeconds() == 0);
  rv += SDK_ASSERT(testPrintMinutes() == 0);
  rv += SDK_ASSERT(testPrintMinutesWrapped() == 0);
  rv += SDK_ASSERT(testPrintHours() == 0);
  rv += SDK_ASSERT(testPrintHoursWrapped() == 0);
  rv += SDK_ASSERT(testPrintOrdinal() == 0);
  rv += SDK_ASSERT(testPrintMonthDay() == 0);
  rv += SDK_ASSERT(testPrintDtg() == 0);
  rv += SDK_ASSERT(testPrintIso8601() == 0);
  rv += SDK_ASSERT(testPrintDeprecated() == 0);
  rv += SDK_ASSERT(canConvertTest() == 0);
  rv += SDK_ASSERT(testFreeformTimeStr() == 0);
  return rv;
}
