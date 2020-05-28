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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cmath>
#include <limits>
#include "simCore/Common/Time.h"
#include "simCore/Time/Constants.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/Julian.h"
#include "simCore/Time/Utils.h"

int simCore::julianDay()
{
  struct timeval tp;
  struct tm *pTime;

  // get the current system time, using timezone value of 0
  // returns UTC time
  gettimeofday(&tp, 0);

  // put system time into a tm struct
  time_t t(tp.tv_sec);
  pTime = gmtime(&t);

  if (pTime == NULL)
    return std::numeric_limits<int>::max();

  // tm struct year days range from 0 to 365, Julian days are 1 to 366
  // hence, need to add an extra day for Julian
  return static_cast<int>(pTime->tm_yday + 1);
}

int simCore::julianDay(double secsSinceRefYear, unsigned int refYear)
{
  const struct tm& timeval = simCore::getTimeStruct(secsSinceRefYear, refYear - 1900);
  // tm struct year days range from 0 to 365, Julian days are 1 to 366
  // hence, need to add an extra day for Julian
  return static_cast<int>(timeval.tm_yday + 1);
}

double simCore::julianDayFrac()
{
  return static_cast<double>(simCore::julianDay()) - 1.
    + simCore::systemTimeToSecsBgnDay() / static_cast<double>(simCore::SECPERDAY);
}

double simCore::julianDate(int yr, double juldayfrac)
{
  // Astronomical Formulae for Calculators, Jean Meeus, pages 23-25
  // Calculate Julian Date based on epoch time of -4712 Jan 1 12:00 GMT
  int year = yr - 1;
  int A = static_cast<int>(year/100);
  int B = 2 - A + static_cast<int>(A/4);

  // Julian Date of Year, referenced to year 0
  return (static_cast<int>(365.25 * year) + static_cast<int>(30.6001 * 14) + 1720994.5 + B) + juldayfrac;
}

double simCore::julianDate()
{
  // Add one because julianDayFrac() subtracts one to get the last full day
  return simCore::julianDate(simCore::currentYear(), simCore::julianDayFrac() + 1);
}

int simCore::julianDate(int year, int month, int monthDay)
{
  // Calculate a Julian date from a given Gregorian calendar date.
  // Fliegel and van Flandern (1968) published compact computer algorithms for
  // converting between Julian dates and Gregorian calendar dates. In this code,
  // year is the full representation of the year, such as 1970, 2000, etc.
  // (not a two-digit abbreviation);  month is a number from 1 to 12;
  // monthDay is the day of the month, a number in the range 1-31; and JD is the the
  // Julian date at Greenwich noon on the specified year, month, and monthDay.
  //
  // Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
  // Communications of the ACM, Vol. 11, No. 10 (October, 1968).
  // http://aa.usno.navy.mil/faq/docs/JD_Formula.html

  // Conversion from a Gregorian calendar date to a Julian date.
  // Valid for any Gregorian calendar date producing a Julian date greater than zero:
  int jd = (monthDay-32075+1461*(year+4800+(month-14)/12)/4+367*
           (month-2-(month-14)/12*12)/12-3*
           ((year+4900+(month-14)/12)/100)/4);
  return jd;
}

void simCore::calendarDateFromJulianDate(int jd, int &year, unsigned int &month, unsigned int &monthday)
{
  // Calculate a Gregorian calendar date (day, month, & year) from a given
  // Julian date.
  // Reference: Fliegel, H. F. and van Flandern, T. C. (1968).
  // Communications of the ACM, Vol. 11, No. 10 (October, 1968).
  // http://aa.usno.navy.mil/faq/docs/JD_Formula.html

  int L=(jd+68569);
  int N=(4*L/146097);
  L = L-(146097*N+3)/4;
  int I=(4000*(L+1)/1461001);
  L = L-1461*I/4+31;
  int J=(80*L/2447);
  int K=(L-2447*J/80);
  L = J/11;
  J = J+2-12*L;
  I = 100*(N-49)+I+L;

  // verify settings
  simCore::checkValidDMY(K, J, I);

  year = I;
  month = J;
  monthday = K;
}

void simCore::calendarDateFromJulianDate(double jd, int &year, unsigned int &month, unsigned int &monthday,
                                         int &hour, int &minute, double &second)
{
  // Round 'jd' to the nearest integer (i.e. nearest 12h).  Then, break up
  // 'jd' into integer and fractional parts.
  unsigned int intjd = static_cast<unsigned int>(jd);
  unsigned int jd12h = static_cast<unsigned int>(jd + 0.5);
  double fracjd = (jd - static_cast<double>(intjd));

  // Compute time of day from fractional part; add one-half day since
  // Julian day and calendar day start 12 hours apart.
  double dummy = (fmod(((fracjd * HOURPERDAY) + 12.0), HOURPERDAY));
  hour = static_cast<unsigned int>(dummy);

  dummy = (dummy - static_cast<double>(hour)) * MINPERHOUR;
  minute = static_cast<unsigned int>(dummy);

  second = (dummy - static_cast<double>(minute)) * SECPERMIN;

  // Rectify output values; adjust calendar date taking into account
  // rounding the seconds.
  unsigned int test = static_cast<unsigned int>(floor(second + 0.5));

  if (test >= SECPERMIN)
  {
    second = 0;
    minute += 1;
  }

  if (minute >= MINPERHOUR)
  {
    minute = 0;
    hour  += 1;
  }

  if (hour >= HOURPERDAY)
  {
    hour  = 0;
    jd12h += 1;
  }

  // Compute calendar date
  simCore::calendarDateFromJulianDate(jd12h, year, month, monthday);
}

void simCore::calendarDateFromJulianDate(double jd, double &fyear)
{
  // Purpose:
  // Converts an Julian date to a year, including the fractional part,
  // on the Gregorian calendar.

  // Set output to zero, if the input Julian date is invalid
  if (jd <= 0.0)
  {
    fyear = 0.0;
    return;
  }

  // Increase the input Julian date by 0.5 day to facilitate the
  // computations.
  jd += 0.5;

  // Decompose 'jd' into integer and fractional parts.
  int jdInt = static_cast<int>(jd);
  double jdFrac = jd - static_cast<double>(jdInt);

  // Compute Gregorian calendar date corresponding to integer jd.
  int year;
  unsigned int month;
  unsigned int monthday;
  simCore::calendarDateFromJulianDate(jdInt, year, month, monthday);

  // Determine if the input year is a leap year.
  size_t lpYrIndx = (simCore::isLeapYear(year - 1900)) ? 1 : 0;

  // Compute the year corresponding to the input Julian date.
  double daysinyr[2] = {365.0, 366.0};
  fyear = static_cast<double>(year) +
    (static_cast<double>(simCore::DAYS_IN_YEAR[lpYrIndx][month] + monthday - 1) + jdFrac) /
    daysinyr[lpYrIndx];
}

double simCore::getDeltaT(double tjd)
{
  // Purpose:
  // To compute an approximate value of "Delta T" (TT - UT1) for
  // the interval 1970-2050.  See note below.
  //
  // TDT: Terrestrial Dynamical Time
  // Universal Time (UT) is a time scale based on the mean solar day,
  // defined to be as uniform as possible despite variations in the rotation of the Earth.
  //
  // UT0 is the rotational time of a particular place of observation. It is observed as the
  // diurnal motion of stars or extraterrestrial radio sources.
  // UT1 is computed by correcting UT0 for the effect of polar motion on the longitude of the
  // observing site. It varies from uniformity because of the irregularities in the Earth's rotation.

  // * This function is valid for dates from 1970 through 2050. *

  // tjd_min is 1970.0; tjd_max is 2050.0
  const double tjd_min = 2440586.5;
  const double tjd_max = 2469807.5;
  double delt = -1000.0;

  // Make sure input 'tjd' is in valid range.  If it isn't, set Delta T
  // to a value of -1000 seconds.
  if ((tjd < tjd_min) || (tjd > tjd_max))
  {
    delt = -1000.0;
    throw simCore::TimeException(simCore::DELTAT_NOT_VALID, "simCore::getDeltaT, This function is valid for dates from 1970 through 2050");
  }
  else
  {
    double fy;
    simCore::calendarDateFromJulianDate(tjd, fy);
    double t = (fy - 2000.0) / 100.0;
    // degree of the polynomial
    const int degree = 5;
    int i = 0;

    // delta T coefficients, based on 2012 values
    const double coeff[] = { 62.96117620920749, 40.72414272333056, -86.78906680769823, 245.6252926768666, 295.8460515505873, -851.1075983781398 };

    // evaluate a polynomial of degree 'degree' using Horner's rule
    delt = coeff[degree];
    for (i = (degree - 1); i >= 0; i--)
    {
      delt = delt * t + coeff[i];
    }
  }

  return delt;
}
