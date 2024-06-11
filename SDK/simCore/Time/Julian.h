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
#ifndef SIMCORE_TIME_JULIAN_H
#define SIMCORE_TIME_JULIAN_H

#include "simCore/Common/Common.h"

namespace simCore
{
/**
* Returns the current Julian day for the current year, referenced to the Julian Calendar [1-366]
* @return Julian day an integer referenced to the current Gregorian year (or "calendrical year").
*/
SDKCORE_EXPORT int julianDay();

/**
* Returns the current Julian day for the specified time and reference year, referenced to the Julian Calendar [1-366]
* @param[in ] secsSinceRefYear A double containing time referenced to seconds since midnight (00:00) Coordinated Universal Time (UTC), January 1, of the specified reference year.
* @param[in ] refYear An unsigned int containing the year referenced to the Gregorian year (i.e. 2003 or "calendrical year").
* @return Julian day an integer referenced to the specified time and Gregorian year (or "calendrical year").  NOTE: This is a YEAR date (plus one) or JULIAN DAY and NOT a Julian DATE; to get a Year Date (0-based), subtract 1.
* @throw TimeException
*/
SDKCORE_EXPORT int julianDay(double secsSinceRefYear, unsigned int refYear);

/**
* Returns the last full Julian day + elapsed percentage of the current day as referenced to UTC and the Julian Calendar [1-366]
* @return Julian day a double referenced to the current Gregorian year (or "calendrical year").
* @throw TimeException
*/
SDKCORE_EXPORT double julianDayFrac();

/**
* Returns the current Julian date count,
* referenced to the remote epoch of -4712 January 1, 12 hours Greenwich Mean Time (GMT).  At this
* instant, the Julian date is 0.  Primarily useful for astronomy purposes.
* @return Julian date, for the current system time.
*/
SDKCORE_EXPORT double julianDate();

/**
* Returns the Julian date count for the specified Gregorian year (or "calendrical year") and Julian Day,
* referenced to the remote epoch of -4712 January 1, 12 hours Greenwich Mean Time (GMT).  At this
* instant, the Julian date is 0.  Primarily useful for astronomy purposes.
* @param[in ] refyr An integer containing the year (>1900) referenced to the Gregorian calendar.
* @param[in ] juldayfrac A double containing the last full Julian day + elapsed percentage of day as referenced to UTC and the Julian Calendar [1-366]
* @return Julian date a double referenced to the remote epoch of -4712 January 1, 12 hours Greenwich Mean Time (GMT)
*/
SDKCORE_EXPORT double julianDate(int refyr, double juldayfrac);

/**
* Returns the Julian date count for the specified Gregorian year (or "calendrical year") and Julian Day,
* referenced to the remote epoch of -4712 January 1, 12 hours Greenwich Mean Time (GMT).  At this
* instant, the Julian date is 0.  Primarily useful for astronomy purposes.
* @param[in ] year An integer containing the Gregorian year (e.g. 1989, 2003, etc.)
* @param[in ] month An integer containing the month in the range of [1-12]
* @param[in ] monthDay An integer containing the day of the month
* @return Julian date referenced to the remote epoch of -4712 January 1, 12 hours Greenwich Mean Time (GMT)
*/
SDKCORE_EXPORT int julianDate(int year, int month, int monthDay);

/**
* Calculate a Gregorian calendar date (day, month, & year) from a given Julian date.
* Valid for any Gregorian calendar date producing a Julian date greater than zero:
* Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
* Communications of the ACM, Vol. 11, No. 10 (October, 1968).
* http://aa.usno.navy.mil/faq/docs/JD_Formula.html
* @param[in ] jd Julian Date
* @param[out] year Calculated Gregorian calendar year
* @param[out] month Calculated calendar month of year (1-12)
* @param[out] monthday Calculated calendar day of month (1-31)
* @throw TimeException
* @pre valid year, month and monthday params
*/
SDKCORE_EXPORT void calendarDateFromJulianDate(int jd, int &year, unsigned int &month, unsigned int &monthday);

/**
* Calculate a Gregorian calendar date from a given Julian date.
* Valid for any Gregorian calendar date producing a Julian date greater than zero:
* Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
* Communications of the ACM, Vol. 11, No. 10 (October, 1968).
* http://aa.usno.navy.mil/faq/docs/JD_Formula.html
* @param[in ] jd Julian Date
* @param[out] year Calculated Gregorian calendar year
* @param[out] month Calculated calendar month of year (1-12)
* @param[out] monthday Calculated calendar day of month (1-31)
* @param[out] hour Calculated hour of day (0-11)
* @param[out] minute Calculated minute of hour (0-59)
* @param[out] second Calculated second of minute (0-59)
* @throw TimeException
* @pre valid year, month, monthday, hour, minute and second params
*/
SDKCORE_EXPORT void calendarDateFromJulianDate(double jd, int &year, unsigned int &month, unsigned int &monthday, int &hour, int &minute, double &second);

/**
* Calculate a Gregorian calendar date from a given Julian date.
* Valid for any Gregorian calendar date producing a Julian date greater than zero:
* Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
* Communications of the ACM, Vol. 11, No. 10 (October, 1968).
* http://aa.usno.navy.mil/faq/docs/JD_Formula.html
* @param[in ] jd Julian Date
* @param[out] fyear Calculated Gregorian calendar year, including fractional part
* @throw TimeException
* @pre valid fyear param
*/
SDKCORE_EXPORT void calendarDateFromJulianDate(double jd, double &fyear);

/**
* Computes 'Delta T', defined as the difference TDT-UT1, by evaluating polynomials that fit a set of determinations and predictions
* provided by the National Earth Orientation Service (NEOS)
* This function is valid for dates from 1970 through 2008
* @param[in ] tjd Julian date
* @return Difference TDT-UT1 in seconds
* @throw TimeException
*/
SDKCORE_EXPORT double getDeltaT(double tjd);
}

#endif

