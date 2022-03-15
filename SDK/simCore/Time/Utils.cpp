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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>

#include "simCore/Common/Time.h"
#include "simCore/Time/Constants.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"

//------------------------------------------------------------------------

int simCore::currentYear()
{
  struct timeval tp;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  // put system time into a tm struct
  const time_t t(tp.tv_sec);
  const struct tm* pTime = gmtime(&t);

  if (pTime == nullptr)
    return std::numeric_limits<int>::max();

  // years are stored as values since 1900
  return static_cast<int>(pTime->tm_year + 1900);
}

//------------------------------------------------------------------------

double simCore::getSystemTime()
{
  struct timeval tp;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  return tp.tv_sec + tp.tv_usec * 1e-06;
}

//------------------------------------------------------------------------

double simCore::systemTimeToSecsBgnYr()
{
  struct timeval tp;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  // put system time into a tm struct
  const time_t t(tp.tv_sec);
  const struct tm* pTime = gmtime(&t);

  if (pTime == nullptr)
    return std::numeric_limits<double>::max();

  // assemble a UTC "system time"
  const unsigned int pSecs = static_cast<unsigned int>(pTime->tm_sec)  +
    ((static_cast<unsigned int>(pTime->tm_min)) * simCore::SECPERMIN) +
    ((static_cast<unsigned int>(pTime->tm_hour)) * simCore::SECPERHOUR) +
    ((static_cast<unsigned int>(pTime->tm_yday)) * simCore::SECPERDAY);

  return pSecs + (tp.tv_usec * 1e-06);
}

//------------------------------------------------------------------------

void simCore::systemTimeToSecsBgnYr(unsigned int &pSecs, unsigned short &pMillisec)
{
  struct timeval tp;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  // put system time into a tm struct
  const time_t t(tp.tv_sec);
  const struct tm* pTime = gmtime(&t);

  if (pTime == nullptr)
  {
    pSecs = std::numeric_limits<unsigned int>::max();
    pMillisec = std::numeric_limits<unsigned short>::max();
  }
  else
  {
    // assemble a UTC "system time"
    pSecs = static_cast<unsigned int>(pTime->tm_sec) +
      ((static_cast<unsigned int>(pTime->tm_min)) * simCore::SECPERMIN) +
      ((static_cast<unsigned int>(pTime->tm_hour)) * simCore::SECPERHOUR) +
      ((static_cast<unsigned int>(pTime->tm_yday)) * simCore::SECPERDAY);

    pMillisec = static_cast<unsigned short>(tp.tv_usec * 1e-03);
  }
}

//------------------------------------------------------------------------

double simCore::systemTimeToSecsBgnDay()
{
  struct timeval tp;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  // put system time into a tm struct
  time_t t(tp.tv_sec);
  struct tm* pTime = gmtime(&t);

  if (pTime == nullptr)
    return std::numeric_limits<double>::max();

  // assemble a UTC "system time"
  const unsigned int pSecs = (static_cast<unsigned int>(pTime->tm_sec) +
    ((static_cast<unsigned int>(pTime->tm_min)) * simCore::SECPERMIN) +
    ((static_cast<unsigned int>(pTime->tm_hour)) * simCore::SECPERHOUR));

  return pSecs + (tp.tv_usec * 1e-06);
}

//------------------------------------------------------------------------

void simCore::timeSinceJan1970ToSecsBgnYr(double timeSinceJan1970, unsigned int &pSecs, unsigned short &pMillisec, unsigned int &pRefyear)
{
  // 64bit int required, since secs from 1970 - 2200 can overflow int
  const int64_t seconds = static_cast<int64_t>(floor(timeSinceJan1970));

  // put system time into a tm struct
  time_t t(seconds);
  const struct tm* pTime = gmtime(&t);

  if (pTime == nullptr)
  {
    // timeSinceJan1970 is invalid
    pSecs = 0;
    pMillisec = 0;
    pRefyear = 1970;
    return;
  }

  // assemble a UTC "system time"
  pSecs = static_cast<unsigned int>(pTime->tm_sec) +
    ((static_cast<unsigned int>(pTime->tm_min)) * simCore::SECPERMIN) +
    ((static_cast<unsigned int>(pTime->tm_hour)) * simCore::SECPERHOUR) +
    ((static_cast<unsigned int>(pTime->tm_yday)) * simCore::SECPERDAY);

  pMillisec = static_cast<unsigned short>((timeSinceJan1970 - seconds) * 1e+03);

  // years are stored as values since 1900
  pRefyear = static_cast<unsigned int>(pTime->tm_year + 1900);
}

//------------------------------------------------------------------------

void simCore::checkValidDMY(unsigned int day, unsigned int month, int year)
{
  // Verify a given day, month and year is correct.
  std::stringstream errBuf;

  if (day < 1 || day > 31)
  {
    errBuf << "simCore::checkValidDMY, Invalid Day: " << day << " is < 1 or > 31";
    throw simCore::TimeException(simCore::MONTHDAY_NOT_VALID, errBuf.str());
  }
  else if (month < 1 || month > static_cast<unsigned int>(simCore::MONPERYEAR))
  {
    errBuf << "simCore::checkValidDMY, Invalid Month: " << month << " is < 1 or > 12";
    throw simCore::TimeException(simCore::MONTH_NOT_VALID, errBuf.str());
  }
  else if (year < 1900)
  {
    errBuf << "simCore::checkValidDMY, Invalid Year: " << year << " is < 1900";
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, errBuf.str());
  }
  else if (!isValidDMY(day, month, year))
  {
    errBuf << "simCore::checkValidDMY, Invalid Date";
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, errBuf.str());
  }
}

//------------------------------------------------------------------------

bool simCore::isValidDMY(unsigned int day, unsigned int month, int year)
{
  if (day < 1 || day > 31)
    return false;
  if ((month < 1) || (month > static_cast<unsigned int>(simCore::MONPERYEAR)))
    return false;
  if (year < 1900)
    return false;

  return day <= simCore::DAYS_IN_MONTHS[isLeapYear(year) ? 1 : 0][month];
}

//------------------------------------------------------------------------

tm simCore::getTimeStruct(double secSinceBgnEpochTime, unsigned int yearsSince1900)
{
  if (secSinceBgnEpochTime < 0)
    throw simCore::TimeException(simCore::SECONDS_SINCE_EPOCHTIME_NOT_VALID, "simCore::getTimeStruct, The seconds since epoch time is < 0.");

  if (secSinceBgnEpochTime > (1000.0 * 365.0 * simCore::SECPERDAY))
    throw simCore::TimeException(simCore::SECONDS_SINCE_EPOCHTIME_NOT_VALID, "simCore::getTimeStruct, The seconds since epoch time is > 1000 years.");

  const int refYear = 1900 + yearsSince1900;
  if (refYear >= MIN_TIME_YEAR && refYear <= MAX_TIME_YEAR)
  {
    const simCore::TimeStamp timeStamp(refYear, secSinceBgnEpochTime);
    // if normalized timeStamp is MIN_TIME_STAMP or MAX_TIME_STAMP,
    //  possible that we are outside TimeStamp capability. fall back to old impl.
    if (timeStamp != MIN_TIME_STAMP && timeStamp != MAX_TIME_STAMP)
    {
      return getTimeStruct(timeStamp);
    }
  }
  tm returnTime = { 0 };
  returnTime.tm_year = yearsSince1900;
  double tmSec = floor(secSinceBgnEpochTime);

  // calculate the year and update both tm_sec and returnTime.tm_year accordingly
  int daysPerCurrentYear = simCore::daysPerYear(yearsSince1900);
  double secondsPerYear = daysPerCurrentYear * simCore::SECPERDAY;
  while (tmSec >= secondsPerYear)
  {
    tmSec -= secondsPerYear;
    daysPerCurrentYear = simCore::daysPerYear(++(returnTime.tm_year));
    secondsPerYear = daysPerCurrentYear * simCore::SECPERDAY;
  }

  // calculate the day of the year (returnTime.tm_yday) and update returnTime.tm_sec accordingly
  returnTime.tm_yday = static_cast<int>(floor(tmSec / simCore::SECPERDAY));
  tmSec -= static_cast<double>(returnTime.tm_yday)*simCore::SECPERDAY;

  // calculate the hour of the day (returnTime.tm_hour) and update returnTime.tm_sec accordingly
  returnTime.tm_hour = static_cast<int>(floor(tmSec / simCore::SECPERHOUR));
  tmSec -= static_cast<double>(returnTime.tm_hour)*simCore::SECPERHOUR;

  // calculate the minute of the hour (returnTime.tm_min) and update returnTime.tm_sec accordingly
  returnTime.tm_min = static_cast<int>(floor(tmSec / simCore::SECPERMIN));
  tmSec -= static_cast<double>(returnTime.tm_min)*simCore::SECPERMIN;
  returnTime.tm_sec = static_cast<int>(tmSec);

  // calculate the month (returnTime.tm_mon) of the year and the day of the month (returnTime.tm_mday)
  simCore::getMonthAndDayOfMonth(returnTime.tm_mon, returnTime.tm_mday, returnTime.tm_year, returnTime.tm_yday);

  // calculate the weekday (returnTime.tm_wday)
  returnTime.tm_wday = simCore::getWeekDay(returnTime.tm_year, returnTime.tm_yday);

  return returnTime;
}

//------------------------------------------------------------------------

tm simCore::getTimeStruct(const TimeStamp& timeStamp)
{
  tm returnTime = { 0 };

  returnTime.tm_year = timeStamp.referenceYear() - 1900;
  int64_t seconds = timeStamp.secondsSinceRefYear().getSeconds();

  // calculate the day of the year (returnTime.tm_yday)
  returnTime.tm_yday = static_cast<int>(floor(seconds / simCore::SECPERDAY));
  seconds -= (returnTime.tm_yday * simCore::SECPERDAY);

  // calculate the hour of the day (returnTime.tm_hour)
  returnTime.tm_hour = static_cast<int>(floor(seconds / simCore::SECPERHOUR));
  seconds -= (returnTime.tm_hour * simCore::SECPERHOUR);

  // calculate the minute of the hour (returnTime.tm_min)
  returnTime.tm_min = static_cast<int>(floor(seconds / simCore::SECPERMIN));
  seconds -= (returnTime.tm_min * simCore::SECPERMIN);
  returnTime.tm_sec = static_cast<int>(seconds);

  // calculate the month (returnTime.tm_mon) of the year and the day of the month (returnTime.tm_mday)
  simCore::getMonthAndDayOfMonth(returnTime.tm_mon, returnTime.tm_mday, returnTime.tm_year, returnTime.tm_yday);

  // calculate the weekday (returnTime.tm_wday)
  returnTime.tm_wday = simCore::getWeekDay(returnTime.tm_year, returnTime.tm_yday);

  return returnTime;
}

//------------------------------------------------------------------------

double simCore::getTimeStructDifferenceInSeconds(const tm& epochTime, const tm& compareTime)
{
  double returnSeconds = 0;
  int daysFromBeginEpochYearToBeginNewTimeYear = 0;

  returnSeconds += compareTime.tm_sec - epochTime.tm_sec;
  returnSeconds += (static_cast<double>(compareTime.tm_min)*simCore::SECPERMIN) - (static_cast<double>(epochTime.tm_min)*simCore::SECPERMIN);
  returnSeconds += (static_cast<double>(compareTime.tm_hour)*simCore::SECPERHOUR) - (static_cast<double>(epochTime.tm_hour)*simCore::SECPERHOUR);
  returnSeconds += (static_cast<double>(compareTime.tm_yday)*simCore::SECPERDAY) - (static_cast<double>(epochTime.tm_yday)*simCore::SECPERDAY);

  if (epochTime.tm_year < compareTime.tm_year)
  {
    for (int currentYear = epochTime.tm_year; currentYear < compareTime.tm_year; ++currentYear)
      daysFromBeginEpochYearToBeginNewTimeYear += simCore::daysPerYear(currentYear);
  }
  else if (compareTime.tm_year < epochTime.tm_year)
  {
    for (int currentYear = compareTime.tm_year; currentYear < epochTime.tm_year; ++currentYear)
      daysFromBeginEpochYearToBeginNewTimeYear -= simCore::daysPerYear(currentYear);
  }

  returnSeconds += static_cast<double>(daysFromBeginEpochYearToBeginNewTimeYear) * simCore::SECPERDAY;

  return returnSeconds;
}

//------------------------------------------------------------------------

int simCore::getYearDay(int month, int monthDay, int year)
{
  if (year < 0)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::getYearDay, The given year is not valid.");
  if ((month < 0) || (month >= simCore::MONPERYEAR))
    throw simCore::TimeException(simCore::MONTH_NOT_VALID, "simCore::getYearDay, The given month is not valid.");
  if ((monthDay <= 0) || (monthDay > simCore::daysPerMonth(year, month)))
    throw simCore::TimeException(simCore::MONTHDAY_NOT_VALID, "simCore::getYearDay, The given month day is not valid.");

  int yearDay = 0;
  for (int currentMonth = 0; currentMonth < simCore::MONPERYEAR; ++currentMonth)
  {
    if (month == currentMonth)
      return yearDay + (monthDay - 1);
    yearDay += simCore::daysPerMonth(year, currentMonth);
  }

  return yearDay;
}

//------------------------------------------------------------------------

void simCore::getMonthAndDayOfMonth(int &month, int &monthDay, int year, int yearDay)
{
  if (year < 0)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::getMonthAndDayOfMonth, The given year is not valid.");
  if ((yearDay < 0) || (yearDay >= simCore::daysPerYear(year)))
    throw simCore::TimeException(simCore::YEARDAY_NOT_VALID, "simCore::getMonthAndDayOfMonth, The given year day is not valid.");

  int lastDayofCurrentMonthYearDay = -1;
  for (int currentMonth = 0; currentMonth < simCore::MONPERYEAR; ++currentMonth)
  {
    lastDayofCurrentMonthYearDay += simCore::daysPerMonth(year, currentMonth);
    if (yearDay <= lastDayofCurrentMonthYearDay)
    {
      month = currentMonth;
      monthDay = 1 + (yearDay - (lastDayofCurrentMonthYearDay - (simCore::daysPerMonth(year, currentMonth) - 1)));
      return;
    }
  }
}

//------------------------------------------------------------------------

int simCore::getWeekDay(int yearsSince1900, int yearDay)
{
  if (yearsSince1900 < 0)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::getWeekDay, The given year is not valid.");
  if ((yearDay < 0) || (yearDay >= simCore::daysPerYear(yearsSince1900)))
    throw simCore::TimeException(simCore::YEARDAY_NOT_VALID, "simCore::getWeekDay, The given year day is not valid.");

  int leaps = 0;
  int currDay = 0;

  // set weekday based on closest leap year
  if (yearsSince1900 >= 4)
    currDay = simCore::getLeapDay(yearsSince1900);
  else
  {
    // leap days started 1904. for [1900,1904), this ensures correct result
    currDay = 0;
  }

  // determine difference between current year and previous
  // leap year, the difference will match the exact weekday
  // for the current year of interest
  if (yearsSince1900 >= 200)
  {
    // 2100 is not a leap year, so the pattern is different
    if (yearsSince1900 >= 201 && !simCore::isLeapYear(yearsSince1900))
      leaps = (yearsSince1900 % 4);
  }
  else if (!simCore::isLeapYear(yearsSince1900))
  {
    // increment extra day due to previous leap year
    leaps = 1 + (yearsSince1900 % 4);
  }

  // using beginning weekday of year, add number of years
  // since previous leap year plus the current yearday
  currDay += leaps + yearDay;

  return currDay % 7;
}

//------------------------------------------------------------------------

int simCore::getLeapDay(int yearsSince1900)
{
  if (yearsSince1900 < 4)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::getLeapDay, The given year is not valid.");

  // The first day of the first leap year was a Friday (01.01.1904)
  // each successive leap year's first day is 5 days beyond that of
  // previous leap year, until 2104 (b/c 2100 is not a leap year).
  const int leap = yearsSince1900 / 4;

  // every 7 leap years, the cycle repeats itself, until 2104, when it restarts with a -1 offset
  return (yearsSince1900 < 204) ? (leap * 5) % 7 : ((leap * 5) % 7) - 1;
}

//------------------------------------------------------------------------

unsigned int simCore::leapDays(int yearsSince1900)
{
  if (yearsSince1900 < 0)
  {
    assert(0);
    return 0;
  }
  // -1 means that a leap year (e.g., 1904) does not generate a count, whereas leap year+1 (.e.g. 1905) does.
  const unsigned int normalLeapDays = (yearsSince1900 - 1) / 4;
  // -1 means that 100-year (e.g., 2000) does not generate a count, whereas 100-year+1 (.e.g. 2001) does.
  const unsigned int nonLeapDays100 = (yearsSince1900 - 1) / 100;
  // +299 means that 400-year (e.g., 2000) does not generate a count, whereas 400-year+1 (.e.g. 2001) does.
  const unsigned int leapDays400 = (yearsSince1900 + 299) / 400;
  return normalLeapDays + leapDays400 - nonLeapDays100;
}

//------------------------------------------------------------------------

int simCore::daysPerYear(int year)
{
  return simCore::isLeapYear(year) ? 366 : 365;
}

//------------------------------------------------------------------------

int simCore::daysPerMonth(int year, int month)
{
  if (year < 0)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::daysPerMonth, The given year is not valid.");
  if ((month < 0) || (month >= simCore::MONPERYEAR))
    throw simCore::TimeException(simCore::MONTH_NOT_VALID, "simCore::daysPerMonth, The given month is not valid.");

  // Month + 1 is used because DAYS_IN_MONTHS is a 13-element array
  return simCore::DAYS_IN_MONTHS[isLeapYear(year) ? 1 : 0][month+1];
}

//------------------------------------------------------------------------

bool simCore::isLeapYear(int year)
{
  if (year < 0)
    throw simCore::TimeException(simCore::YEAR_NOT_VALID, "simCore::isLeapYear, The given year is not valid.");

  const int gregorianYear((year > 1900) ? year : year+1900);
  // A year is a leap year if it is divisible by 4, but not by 100
  // except that years divisible by 400 are leap years.
  return ((gregorianYear % 4 == 0 && gregorianYear % 100 != 0) || gregorianYear % 400 == 0) ? true : false;
}

//------------------------------------------------------------------------

///Breaks a time value referenced to a calendar year into individual components
std::string simCore::getTimeComponents(double time, unsigned int *day, unsigned int *hour, unsigned int *min, unsigned int *sec, unsigned int *tenthSec, bool ordinal)
{
  unsigned int d = static_cast<unsigned int>(time / simCore::SECPERDAY);
  const unsigned int h = static_cast<unsigned int>((time - d*simCore::SECPERDAY) / simCore::SECPERHOUR);
  const unsigned int m = static_cast<unsigned int>((time - d*simCore::SECPERDAY - h*simCore::SECPERHOUR) / simCore::SECPERMIN);
  const unsigned int s = static_cast<unsigned int>((time - d*simCore::SECPERDAY - h*simCore::SECPERHOUR - m*simCore::SECPERMIN));
  const unsigned int t = static_cast<unsigned int>((time - d*simCore::SECPERDAY - h*simCore::SECPERHOUR - m*simCore::SECPERMIN - s) * 10);
  if (ordinal)
    ++d;

  if (day && hour && min && sec && tenthSec)
  {
    *day = d;
    *hour = h;
    *min = m;
    *sec = s;
    *tenthSec = t;
  }

  ///format return value
  std::stringstream str;
  str << std::setfill('0') << std::setw(3) << d << " ";
  str << std::setfill('0') << std::setw(2) << h << ":";
  str << std::setfill('0') << std::setw(2) << m << ":";
  str << std::setfill('0') << std::setw(2) << s;
  return str.str();
}

//------------------------------------------------------------------------

double simCore::getNextTimeStep(bool faster, double lastStep)
{
  if (lastStep < 0.0)
    lastStep = -lastStep;
  else if (lastStep == 0.0)
    return 0;

  int factor = 0;
  if (faster)
  {
    if (lastStep >= 0.1 && lastStep < 1.0)
    {
      if (lastStep >= 0.5)
        lastStep = 1.0;
      else if (lastStep >= .25)
        lastStep = 0.5;
      else
        lastStep = .25;
    }
    else if (lastStep < 0.1)
    {
      while (lastStep < 1.0)
      {
        lastStep *= 10.0;
        factor++;
      }

      lastStep = (lastStep >= 5.0) ? 10.0 : 5.0;
      lastStep /= pow(10.0, factor);
    }
    else if (lastStep >= 1.0)
      lastStep = floor(lastStep + 1.0);
    else
    {
      // Dev error, all number ranges should be covered by the logic above
      assert(0);
    }
  }
  else  // Handle backward time
  {
    if (lastStep > 0.1 && lastStep < 1.0)
    {
      if (lastStep <= .25)
        lastStep = 0.1;
      else if (lastStep <= 0.5)
        lastStep = .25;
      else
        lastStep = 0.5;
    }
    else if (lastStep <= 0.1)
    {
      while (lastStep < 1.0)
      {
        lastStep *= 10.0;
        factor++;
      }

      // increment once more for backward time condition
      factor++;
      lastStep = (lastStep >= 5.0) ? 10.0 : 5.0;
      lastStep /= pow(10.0, factor);
    }
    else if (lastStep >= 2.0)
      lastStep = floor(lastStep - 1.0);
    else if (lastStep == 1.0)
      lastStep = 0.5;
    else if (lastStep > 1.0 && lastStep < 2.0)
      lastStep = 1.0;
    else
    {
      // Dev error, all number ranges should be covered by the logic above
      assert(0);
    }
  }
  return lastStep;
}
