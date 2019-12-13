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
#ifndef SIMCORE_TIME_UTILS_H
#define SIMCORE_TIME_UTILS_H

#include <string>

#include "simCore/Common/Common.h"
#include "simCore/Common/Time.h"

namespace simCore
{
  class TimeStamp;

  //------------------------------------------------------------------------
  //
  // <time.h> struct tm definition
  //
  //    int  tm_sec;   /* seconds after the minute - [0, 61] for leap seconds */
  //    int  tm_min;   /* minutes after the hour - [0, 59] */
  //    int  tm_hour;  /* hour since midnight - [0, 23] */
  //    int  tm_mday;  /* day of the month - [1, 31] */
  //    int  tm_mon;   /* months since January - [0, 11] */
  //    int  tm_year;  /* years since 1900 */
  //    int  tm_wday;  /* days since Sunday - [0, 6] */
  //    int  tm_yday;  /* days since January 1 - [0, 365] */
  //    int  tm_isdst; /* flag for alternate daylight savings time */
  //
  /**
  * The system's notion of the current time is obtained with the gettimeofday call.  The time is expressed in seconds and micro-
  * seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, 1970.
  * @return system time a double in seconds since midnight UTC Jan 1, 1970.
  */
  SDKCORE_EXPORT double getSystemTime();

  /**
  * The system's notion of the current time is obtained with the gettimeofday call.  The time is expressed in seconds and micro-
  * seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, 1970.
  * @return system time a double in seconds referenced to the beginning of the current Gregorian year (or "calendrical year").
  */
  SDKCORE_EXPORT double systemTimeToSecsBgnYr();

  /**
  * The system's notion of the current time is obtained with the gettimeofday call.  The time is expressed in seconds and micro-
  * seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, 1970.
  * @param[out] pSecs A pointer to an unsigned int.  Stores elapsed seconds referenced to the beginning of the current Gregorian year (or "calendrical year").
  * @param[out] pMillisec A pointer to an unsigned short.  Stores elapsed milliseconds referenced to the beginning of the current Gregorian year (or "calendrical year").
  * @pre valid seconds and milliseconds params
  */
  SDKCORE_EXPORT void systemTimeToSecsBgnYr(unsigned int &pSecs, unsigned short &pMillisec);

  /**
  * The system's notion of the current time is obtained with the gettimeofday call.  The time is expressed in seconds and micro-
  * seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, 1970.
  * @return system time a double in seconds referenced to the beginning of the current day.
  */
  SDKCORE_EXPORT double systemTimeToSecsBgnDay();

  /**
  * Converts a UTC system time broken into seconds and millisecs referenced to the beginning of the current Gregorian year (or "calendrical year").
  * @param[in ] timeSinceJan1970 A double containing time referenced to seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, 1970.
  * @param[out] pSecs A pointer to an unsigned int.  Stores seconds referenced to the beginning of the current Gregorian year (or "calendrical year").
  * @param[out] pMillisec A pointer to an unsigned short.  Stores millisec referenced to the beginning of the current Gregorian year (or "calendrical year").
  * @param[out] pRefyear A pointer to an unsigned int.  Stores year of data referenced to the Gregorian year (i.e. 2003 or "calendrical year").
  * @pre valid seconds, milliseconds and reference year params
  */
  SDKCORE_EXPORT void timeSinceJan1970ToSecsBgnYr(double timeSinceJan1970, unsigned int &pSecs, unsigned short &pMillisec, unsigned int &pRefyear);

  /**
  * Returns the current Gregorian year.  A Gregorian year (or "calendrical year") is the number of days in a given year of the Gregorian calendar (namely, 365 days in non-leap years and 366 days in a leap year)
  * @return current year an integer referenced to the Gregorian calendar.
  */
  SDKCORE_EXPORT int currentYear();

  /**
  *  Verify a given day, month and year is valid.
  * @param[in ] monthDay Day of month (1-31)
  * @param[in ] month Month of year (1-12)
  * @param[in ] year Gregorian calendar year (1900-)
  * @throw TimeException
  */
  SDKCORE_EXPORT void checkValidDMY(unsigned int monthDay, unsigned int month, int year);

  /**
  * Returns a boolean whether or not the indicated day, month, year is valid.
  * @param[in ] monthDay An integer containing the day in the range of [1-31]
  * @param[in ] month An integer containing the month in the range of [1-12]
  * @param[in ] year Gregorian calendar year [1900-)
  * @return boolean value indicating whether or not the given day, month, year was valid.
  */
  SDKCORE_EXPORT bool isValidDMY(unsigned int monthDay, unsigned int month, int year);

  //------------------------------------------------------------------------

  /** @name TimeException types
  * TimeException types.
  */
  //@{
  ///TimeException types.
  /**
  * A list of handled time exception types.  Used to identify the exception
  */
  static const int TIME_STRING_NOT_VALID              = 1; /**< Input time string is not valid. */
  static const int JULIANDAY_NOT_VALID                = 2; /**< Input Julian day is not valid. */
  static const int DAY_STRING_NOT_VALID               = 3; /**< Input day string is not valid. */
  static const int MONTH_NOT_VALID                    = 4; /**< Input month is not valid. */
  static const int MONTHDAY_NOT_VALID                 = 5; /**< Input month day is not valid. */
  static const int HOURS_NOT_VALID                    = 6; /**< Input hours is not valid. */
  static const int YEAR_NOT_VALID                     = 7; /**< Input year is not valid. */
  static const int WEEKDAY_NOT_VALID                  = 8; /**< Input weekday is not valid. */
  static const int TOO_MANY_VALUES                    = 9; /**< Too many values input. */
  static const int REFERENCE_YEAR_NOT_VALID           = 10; /**< Input reference year is not valid. */
  static const int SECONDS_SINCE_EPOCHTIME_NOT_VALID  = 11; /**< Input seconds since epoch time is not valid. */
  static const int STRING_FORMAT_NOT_VALID            = 12; /**< Input string format is not valid. */
  static const int YEARDAY_NOT_VALID                  = 13; /**< Input year day is not valid. */
  static const int MINUTES_NOT_VALID                  = 14; /**< Input minutes is not valid. */
  static const int SECONDS_NOT_VALID                  = 15; /**< Input seconds is not valid. */
  static const int DELTAT_NOT_VALID                   = 16; /**< Input Julian date is not valid. */
  static const int GPS_WEEK_NOT_VALID                 = 17; /**< Input GPS week not valid. */
  static const int GPS_EPOCH_NOT_VALID                = 18; /**< Input GPS epoch not valid. */
  static const int UTC_NOT_VALID_FOR_GPS              = 19; /**< Input UTC not valid for GPS */
  //@}

  /**
  * Returns a tm time struct that corresponds to the input time referenced to the input epoch.
  * Note: the struct tm uses an int for storing the seconds value, so the tm struct that is returned
  * by this function is less accurate a time than the input seconds since epoch time.
  * @param[in ] secSinceBgnOfEpochTime A double containing seconds (>=0) referenced to the input input epoch.
  * @param[in ] yearsSince1900 year to use as the reference time.
  * @return A tm time struct referenced to the input time referenced to the input epoch.
  * @throw TimeException
  */
  SDKCORE_EXPORT tm getTimeStruct(double secSinceBgnOfEpochTime, unsigned int yearsSince1900);

  /**
  * Returns a tm time struct that corresponds to the input timeStamp.
  * Note: the struct tm uses an int for storing the seconds value, so the tm struct that is returned
  * by this function is less accurate a time than the input seconds since epoch time.
  * @param[in ] timeStamp A timestamp containing reference year and seconds since that reference year.
  * @return A tm time struct referenced to the input timeStamp.
  */
  SDKCORE_EXPORT tm getTimeStruct(const simCore::TimeStamp& timeStamp);

  /**
  * Returns the difference in seconds between two tm time structs
  * @param[in ] epochTime A tm time struct containing the reference time.
  * @param[in ] compareTime A tm time struct containing the comparison time.
  * @return the difference in seconds between two tm time structs as a double.
  * @throw TimeException
  */
  SDKCORE_EXPORT double getTimeStructDifferenceInSeconds(const tm& epochTime, const tm& compareTime);

  /**
  * Returns the # of days since the beginning of the Gregorian year ("calendrical year") that corresponds to the given month and monthDay values
  *   month values [0, 11]
  *   monthDay values [1, DaysPerMonth(year, month)]
  * @param[in ] month An integer containing the month in the range of [0-11]
  * @param[in ] monthDay An integer containing the day of the month
  * @param[in ] year Year value; values less than 1900 will be treated as 1900+year
  * @return integer containing the number of days since the beginning of the year for the specified input values.
  * @throw TimeException
  */
  SDKCORE_EXPORT int getYearDay(int month, int monthDay, int year);

  /**
  * Assigns the values of "month" and "monthDay" that correspond to the given year and the associated Gregorian year day value.
  *   month values [0, 11]
  *   monthDay values [1, DaysPerMonth(year, month)]
  *   yearDay values [0,DaysPerYear(year)]
  * @param[out] month An integer that is assigned the month in the range of [0-11]
  * @param[out] monthDay An integer that is assigned the day of the month
  * @param[in ] year Year value; values less than 1900 will be treated as 1900+year
  * @param[in ] yearDay An integer containing the number of days since the beginning of the specified Gregorian year [0-365]
  * @throw TimeException
  * @pre month and monthDay valid params
  */
  SDKCORE_EXPORT void getMonthAndDayOfMonth(int &month, int &monthDay, int year, int yearDay);

  /**
  * Returns the week day value [0-6] mapping to [Sunday,...,Saturday] that corresponds to the given years since 1900 and the associated Gregorian year day value.
  *   yearsSince1900 >= 0, i.e. 1999 would be represented as 99
  *   yearDay values [0,DaysPerYear(yearsSince1900)]
  * @param[in ] yearsSince1900 An integer containing the number of elapsed years since 1900, must be <= 200.
  * @param[in ] yearDay An integer containing the number of days since the beginning of the specified Gregorian year
  * @return integer containing the week day number [0-6] associated to the given input values.
  * @throw TimeException
  */
  SDKCORE_EXPORT int getWeekDay(int yearsSince1900, int yearDay);

  /**
  * Returns the week day value [0-6] ([Sunday,...,Saturday]) for Jan 01 of the most recent leap year that corresponds to the given yearsSince1900.
  *   yearsSince1900 >= 0, i.e. 1999 would be represented as 99
  * @param[in ] yearsSince1900 An integer containing the number of elapsed years since 1900, must be <= 200.
  * @return integer containing the week day [0-6] of the first day of the most recent leap year that corresponds to the given yearsSince1900.
  * @throw TimeException
  */
  SDKCORE_EXPORT int getLeapDay(int yearsSince1900);

  /**
  * Returns a boolean whether or not the indicated year was a leap year.
  * Year values less than 1900 will be treated as 1900+year
  * @param[in ] year An integer representing a Gregorian year (1970, 2000, etc.)
  * @return boolean value indicating whether or not the year was a leap year (true) or not.
  * @throw TimeException
  */
  SDKCORE_EXPORT bool isLeapYear(int year);

  /**
  * Returns the number of leap days from 1900 up to (but not including) the year specified by yearsSince1900.
  * @param[in ] yearsSince1900 An integer specifying the year, as a number of years since 1900.
  * @return value indicating number of leap days since 1900 up to the year specified by yearsSince1900.
  */
  SDKCORE_EXPORT unsigned int leapDays(int yearsSince1900);

  /**
  * Returns the number of days in the Gregorian year that corresponds to the given year.
  * @param[in ] year Year value; values less than 1900 will be treated as 1900+year
  * @return an integer containing the number of days in the Gregorian year that corresponds to the given years since 1900.
  * @throw TimeException
  */
  SDKCORE_EXPORT int daysPerYear(int year);

  /**
  * Returns the number of days in the specified month for the associated Gregorian year.
  *   month values [0, 11]
  *   monthDay values [1, DaysPerMonth(year, month)]
  * @param[in ] year Year value; values less than 1900 will be treated as 1900+year
  * @param[in ] month An integer containing the month in the range of [0-11]
  * @return integer containing the number of days in the specified month for the associated Gregorian year.
  * @throw TimeException
  */
  SDKCORE_EXPORT int daysPerMonth(int year, int month);

  /**
  * Breaks a time value referenced to a calendar year into individual components
  * @param[in ] time A double containing time referenced to seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, of a reference year.
  * @param[out] day Computed number of days starting on Jan 1, if ordinal flag is set, days range from [1, n] otherwise [0, n]
  * @param[out] hour Computed number of hours since midnight [0, 23]
  * @param[out] min Computed number of minutes after the hour [0, 59]
  * @param[out] sec Computed number of seconds after the minute [0, 59]
  * @param[out] tenthSec Computed number of tenth of seconds after the second [0, 9]
  * @param[in ] ordinal boolean that indicates reference range for days, true: [1, n], false: [0, n]
  * @return string of time components in the format of "%03i %02i:%02i:%02i"
  * @pre day, hour, min, sec and tenthSec valid params
  */
  SDKCORE_EXPORT std::string getTimeComponents(double time, unsigned int *day, unsigned int *hour, unsigned int *min, unsigned int *sec, unsigned int *tenthSec, bool ordinal);

  /**
   * Normalize year and seconds values so that the seconds value is less than one year.
   * @param[in,out] refYear An integer containing the year (>1900) referenced to the Gregorian calendar.
   * @param[in,out] secondsSinceRefYear A double in seconds referenced to the beginning of the given Gregorian year (or "calendrical year").
   * @throw TimeException
   * @pre refYear and secondsSinceRefYear valid params
   * @deprecated
   */
#ifdef USE_DEPRECATED_SIMDISSDK_API
  SDK_DEPRECATE(SDKCORE_EXPORT void normalizeTime(int &refYear, double &secondsSinceRefYear), "Method will be removed in a future SDK release");
#endif
  /**
   * Algorithm to get a new time step based on a step up or step down from a given step value
   * Calculates the proper step to use when stepping up or down from a time step
   * value in an application that uses SIMDIS time formats
   * @param faster If true, steps faster, else steps slower
   * @param lastStep Positive step value being used before the next time step is calculated
   * @return Positive step value to use after stepping in the direction indicated by faster, from lastStep
   */
  SDKCORE_EXPORT double getNextTimeStep(bool faster, double lastStep);

} // simCore namespace

#endif  /* SIMCORE_TIME_UTILS_H */
