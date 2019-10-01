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
#include <sstream>
#include <iomanip>
#include <cassert>
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/DeprecatedStrings.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/String.h"

namespace simCore
{

std::string NullTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  ss.precision(precision);
  ss << timeStamp.secondsSinceRefYear();
  return ss.str();
}

bool NullTimeFormatter::canConvert(const std::string& timeString) const
{
  return false;
}

int NullTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

///////////////////////////////////////////////////////////////////////

std::string SecondsTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  SecondsTimeFormatter::toStream(ss, timeStamp.secondsSinceRefYear(referenceYear), precision);
  return ss.str();
}

bool SecondsTimeFormatter::canConvert(const std::string& timeString) const
{
  double tmpVal;
  return simCore::isValidNumber(simCore::StringUtils::trim(simCore::removeQuotes(timeString)), tmpVal);
}

int SecondsTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  double tmpVal;
  if (simCore::isValidNumber(simCore::StringUtils::trim(simCore::removeQuotes(timeString)), tmpVal))
  {
    timeStamp = simCore::TimeStamp(referenceYear, tmpVal);
    return 0;
  }
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

void SecondsTimeFormatter::toStream(std::ostream& os, const simCore::Seconds& seconds, unsigned short precision)
{
  os.precision(precision);
  os.fill('0');
  os.setf(std::ios::fixed);
  os << seconds;
}

bool SecondsTimeFormatter::isStrictSecondsString(const std::string& timeString)
{
  double seconds = 0;
  return (isValidNumber(timeString, seconds, false) && seconds >= 0 && seconds < SECPERMIN && timeString[0] != '.');
}

///////////////////////////////////////////////////////////////////////

std::string MinutesTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  MinutesTimeFormatter::toStream(ss, timeStamp.secondsSinceRefYear(referenceYear), precision);
  return ss.str();
}

bool MinutesTimeFormatter::canConvert(const std::string& timeString) const
{
  std::vector<std::string> mmss;
  simCore::stringTokenizer(mmss, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  if (mmss.size() != 2)
    return false;
  int min;
  return SecondsTimeFormatter::isStrictSecondsString(mmss[1]) && isValidNumber(mmss[0], min);
}

int MinutesTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  if (vec.size() == 2)
  {
    int min;
    double sec;
    // Read and combine
    if (simCore::isValidNumber(vec[0], min) && simCore::isValidNumber(vec[1], sec))
    {
      timeStamp = simCore::TimeStamp(referenceYear, min * SECPERMIN + sec);
      return 0;
    }
  }
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

void MinutesTimeFormatter::toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision)
{
  const bool isNegative = (seconds < 0);
  seconds = fabs(seconds.rounded(precision));
  // Rely on static_cast<> to floor the value
  const int minutes = static_cast<int>(seconds.Double() / SECPERMIN);
  seconds -= minutes * SECPERMIN;
  // Account for the decimal spot when setting the width
  int numSpaces = precision + (precision == 0 ? 0 : 1);
  if (isNegative)
    os << "-";
  os << minutes << ':' << std::setw(2 + numSpaces) << std::setfill('0');
  // Add the seconds value
  SecondsTimeFormatter::toStream(os, seconds, precision);
}

bool MinutesTimeFormatter::isStrictMinutesString(const std::string& timeString)
{
  std::vector<std::string> mmss;
  simCore::stringTokenizer(mmss, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  int minutes = 0;
  return mmss.size() == 2 &&
    isValidNumber(mmss[0], minutes, false) &&
    minutes >= 0 && minutes < MINPERHOUR &&
    SecondsTimeFormatter::isStrictSecondsString(mmss[1]);
}

///////////////////////////////////////////////////////////////////////

std::string MinutesWrappedTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  MinutesWrappedTimeFormatter::toStream(ss, timeStamp.secondsSinceRefYear(referenceYear), precision);
  return ss.str();
}

void MinutesWrappedTimeFormatter::toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision)
{
  const Seconds wrapped(seconds.getSeconds() % SECPERHOUR, seconds.getFraction());
  MinutesTimeFormatter::toStream(os, wrapped, precision);
}

///////////////////////////////////////////////////////////////////////

std::string HoursTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  HoursTimeFormatter::toStream(ss, timeStamp.secondsSinceRefYear(referenceYear), precision);
  return ss.str();
}

bool HoursTimeFormatter::canConvert(const std::string& timeString) const
{
  std::vector<std::string> hhmmss;
  simCore::stringTokenizer(hhmmss, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  int hours = 0;
  int minutes = 0;
  return hhmmss.size() == 3 &&
    isValidNumber(hhmmss[0], hours) &&
    isValidNumber(hhmmss[1], minutes, false) &&
    minutes >= 0 && minutes < MINPERHOUR &&
    SecondsTimeFormatter::isStrictSecondsString(hhmmss[2]);
}

int HoursTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  simCore::Seconds seconds;
  if (HoursTimeFormatter::fromString(timeString, seconds) == 0)
  {
    timeStamp = simCore::TimeStamp(referenceYear, seconds);
    return 0;
  }
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

int HoursTimeFormatter::fromString(const std::string& timeString, simCore::Seconds& seconds)
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  if (vec.size() == 3)
  {
    int hours;
    int min;
    double sec;
    // Read and combine
    if (simCore::isValidNumber(vec[0], hours) &&
      simCore::isValidNumber(vec[1], min) &&
      simCore::isValidNumber(vec[2], sec))
    {
      seconds = hours * SECPERHOUR + min * MINPERHOUR + sec;
      return 0;
    }
  }
  seconds = 0;
  return 1;
}

void HoursTimeFormatter::toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision)
{
  const bool isNegative = (seconds < 0);
  seconds = fabs(seconds.rounded(precision));
  // Rely on static_cast<> to floor the value
  int hours = static_cast<int>(seconds.Double() / SECPERHOUR);
  seconds -= hours * SECPERHOUR;
  if (isNegative)
    os << "-";
  os << hours << ':' << std::setfill('0') << std::setw(2);
  // Add the minutes value and seconds value
  MinutesTimeFormatter::toStream(os, seconds, precision);
}

bool HoursTimeFormatter::isStrictHoursString(const std::string& timeString)
{
  std::vector<std::string> hhmmss;
  simCore::stringTokenizer(hhmmss, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), ":", false, false);
  int hours = 0;
  int minutes = 0;
  return hhmmss.size() == 3 &&
    isValidNumber(hhmmss[0], hours, false) &&
    hours >= 0 && hours < HOURPERDAY &&
    isValidNumber(hhmmss[1], minutes, false) &&
    minutes >= 0 && minutes < MINPERHOUR &&
    SecondsTimeFormatter::isStrictSecondsString(hhmmss[2]);
}

///////////////////////////////////////////////////////////////////////

std::string HoursWrappedTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  HoursWrappedTimeFormatter::toStream(ss, timeStamp.secondsSinceRefYear(referenceYear), precision);
  return ss.str();
}

void HoursWrappedTimeFormatter::toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision)
{
  const Seconds wrapped(seconds.getSeconds() % SECPERDAY, seconds.getFraction());
  HoursTimeFormatter::toStream(os, wrapped, precision);
}

///////////////////////////////////////////////////////////////////////

std::string OrdinalTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  OrdinalTimeFormatter::toStream(ss, timeStamp, precision);
  return ss.str();
}

bool OrdinalTimeFormatter::canConvert(const std::string& timeString) const
{
  const std::string& cleanString = simCore::StringUtils::trim(simCore::removeQuotes(timeString));
  if (cleanString.empty())
    return false;
  std::vector<std::string> dayYearHours;
  // Tokenize the string into 3 components (days, year, and hh:mm:ss)
  simCore::stringTokenizer(dayYearHours, cleanString, " ", false, true);
  // Validate the token numbers and sizes: no more than 3 tokens for day, 4 for year
  if (dayYearHours.size() != 3 || dayYearHours[0].size() > 3 || dayYearHours[1].size() != 4)
    return false;

  // Validate and convert the year first
  int year;
  if (!simCore::isValidNumber(dayYearHours[1], year, false) || year < 1900 || year > 9999)
    return false;

  // Validate, read, and bounds check the day string
  int day;
  if (!OrdinalTimeFormatter::isValidOrdinal(dayYearHours[0], year, day))
    return false;

  // Must have a valid strict hours string too
  return HoursTimeFormatter::isStrictHoursString(dayYearHours[2]);
}

int OrdinalTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  std::vector<std::string> dayYearHours;
  // Condense spaces between tokens
  simCore::stringTokenizer(dayYearHours, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ", false, true);
  if (dayYearHours.size() == 3)
  {
    // Pull out the year and the day
    int day;
    int year;
    // Note that isValidOrdinal will test the year as well
    if (simCore::isValidNumber(dayYearHours[1], year) && OrdinalTimeFormatter::isValidOrdinal(dayYearHours[0], year, day))
    {
      // Convert the hours portion
      simCore::Seconds seconds;
      if (HoursTimeFormatter::fromString(dayYearHours[2], seconds) == 0)
      {
        timeStamp = simCore::TimeStamp(year, seconds + simCore::Seconds((day - 1) * SECPERDAY));
        return 0;
      }
    }
  }

  // Falling down to failure
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

void OrdinalTimeFormatter::toStream(std::ostream& os, const simCore::TimeStamp& timeStamp, unsigned short precision)
{
  const int refYear = timeStamp.referenceYear();
  const simCore::TimeStamp roundedStamp(refYear, timeStamp.secondsSinceRefYear(refYear).rounded(precision));
  int days = static_cast<int>(roundedStamp.secondsSinceRefYear().Double() / simCore::SECPERDAY);
  simCore::Seconds seconds = roundedStamp.secondsSinceRefYear() - simCore::Seconds(static_cast<double>(days) * simCore::SECPERDAY);
  os << std::setfill('0') << std::setw(3) << (days + 1) << " " << refYear << " " << std::setw(2);
  HoursTimeFormatter::toStream(os, seconds, precision);
}

bool OrdinalTimeFormatter::isValidOrdinal(const std::string& input, int year, int& ordinal)
{
  try
  {
    if (year > 1900 && simCore::isValidNumber(input, ordinal, false) &&
      ordinal >= 1 && ordinal <= simCore::daysPerYear(year - 1900))
    {
      return true;
    }
  }
  catch (const simCore::TimeException&)
  {
  }
  ordinal = 0;
  return false;
}

///////////////////////////////////////////////////////////////////////

std::string MonthDayTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  const int refYear = timeStamp.referenceYear();
  const simCore::TimeStamp roundedStamp(refYear, timeStamp.secondsSinceRefYear(refYear).rounded(precision));
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  simCore::Seconds seconds;

  // Get components: In case of extreme error, fall back to Ordinal, which can't have exception issues
  if (MonthDayTimeFormatter::getMonthComponents(roundedStamp, month, monthDay, seconds) != 0)
  {
    OrdinalTimeFormatter::toStream(ss, roundedStamp, precision);
    return ss.str();
  }

  // EG Jan 13 2014 00:01:02.03
  ss << MonthDayTimeFormatter::monthIntToString(month) << " " << monthDay << " " << refYear << " " << std::setw(2) << std::setfill('0');
  HoursTimeFormatter::toStream(ss, seconds, precision);
  return ss.str();
}

int MonthDayTimeFormatter::monthStringToInt(const std::string& monthString)
{
  for (int month = 0; month < 12; ++month)
  {
    if (simCore::caseCompare(simCore::ABBREV_MONTH_NAME[month], monthString) == 0)
      return month;
  }
  return -1;
}

std::string MonthDayTimeFormatter::monthIntToString(int monthValue)
{
  if (monthValue >= 0 && monthValue < MONPERYEAR)
    return ABBREV_MONTH_NAME[monthValue];
  return "Unk";
}

int MonthDayTimeFormatter::getMonthComponents(const simCore::TimeStamp& timeStamp, int& month, int& monthDay, simCore::Seconds& secondsPastMidnight)
{
  const int realYear = timeStamp.referenceYear();
  const int days = static_cast<int>(timeStamp.secondsSinceRefYear().Double() / simCore::SECPERDAY);
  try
  {
    simCore::getMonthAndDayOfMonth(month, monthDay, realYear, days);
  }
  catch (const simCore::TimeException& te)
  {
    SIM_ERROR << "Time exception: " << te.what() << std::endl;
    // Should not occur with the massaged simCore::TimeStamp input.
    assert(false);
    return 1;
  }

  // Validate the output of getMonthAndDayOfMonth
  assert(month >= 0 && month < 12);
  assert(monthDay >= 1 && monthDay <= 31);
  secondsPastMidnight = timeStamp.secondsSinceRefYear() - simCore::Seconds(static_cast<double>(days) * simCore::SECPERDAY);
  return 0;
}

bool MonthDayTimeFormatter::canConvert(const std::string& timeString) const
{
  const std::string& cleanString = simCore::StringUtils::trim(simCore::removeQuotes(timeString));
  if (cleanString.empty())
    return false;
  std::vector<std::string> mdyh; // month, day, year, hours
  // Tokenize the string into 3 components (days, year, and hhmmss)
  simCore::stringTokenizer(mdyh, cleanString, " ", false, true);
  // Validate the token numbers and sizes: no more than 3 tokens for day, 4 for year
  if (mdyh.size() != 4 || mdyh[0].size() != 3 || mdyh[1].size() > 2 || mdyh[2].size() != 4)
    return false;

  // Validate the month
  int month = MonthDayTimeFormatter::monthStringToInt(mdyh[0]);
  if (month == -1) return false; // Invalid month string

  // Validate and convert the year
  int year;
  // Validate, read, AND bounds check at once
  if (!simCore::isValidNumber(mdyh[2], year, false) || year < 1900 || year > 9999)
    return false;

  // Validate and convert the day-of-month (year and month are both required to bounds check for leap year)
  try
  {
    int day;
    if (!simCore::isValidNumber(mdyh[1], day) || day <= 0 || day > simCore::daysPerMonth(year - 1900, month))
      return false;
  }
  catch (const simCore::TimeException&) // from daysPerMonth
  {
    return false;
  }

  // Day and year is good; validate the HMS string
  return HoursTimeFormatter::isStrictHoursString(mdyh[3]);
}

int MonthDayTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  std::vector<std::string> mdyh;
  // Condense spaces between tokens
  simCore::stringTokenizer(mdyh, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ", false, true);
  if (mdyh.size() == 4)
  {
    try
    {
      int month = MonthDayTimeFormatter::monthStringToInt(mdyh[0]);
      int monthDay;
      int year;
      // Make sure we can read the days and year, and they're within reasonable bounds
      if (month != -1 && simCore::isValidNumber(mdyh[1], monthDay) &&
        simCore::isValidNumber(mdyh[2], year) &&
        monthDay > 0 && year >= 1900 && monthDay <= simCore::daysPerMonth(year - 1900, month))
      {
        // Convert the hours portion
        simCore::Seconds seconds;
        if (HoursTimeFormatter::fromString(mdyh[3], seconds) == 0)
        {
          int yearDay = getYearDay(month, monthDay, year - 1900);
          timeStamp = simCore::TimeStamp(year, seconds + simCore::Seconds(yearDay * SECPERDAY));
          return 0;
        }
      }
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time exception: " << te.what() << std::endl;
    }
  }

  // Falling down to failure
  timeStamp = simCore::MIN_TIME_STAMP;
  return 1;
}

///////////////////////////////////////////////////////////////////////

std::string DtgTimeFormatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  const int realYear = timeStamp.referenceYear();
  const simCore::TimeStamp roundedStamp(realYear, timeStamp.secondsSinceRefYear(realYear).rounded(precision));
  int days = static_cast<int>(roundedStamp.secondsSinceRefYear().Double() / simCore::SECPERDAY);
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  try
  {
    simCore::getMonthAndDayOfMonth(month, monthDay, realYear, days);
  }
  catch (const simCore::TimeException& te)
  {
    SIM_ERROR << "Time exception: " << te.what() << std::endl;
    // Should not occur with the massaged simCore::TimeStamp input.
    assert(false);
    // In case of extreme error, fall back to Ordinal, which can't have issues like this
    OrdinalTimeFormatter::toStream(ss, roundedStamp, precision);
    return ss.str();
  }

  // Validate the output of getMonthAndDayOfMonth
  assert(month >= 0 && month < MONPERYEAR);
  assert(monthDay >= 1 && monthDay <= 31);

  // Avoid any possible out-of-bounds issues
  std::string monthName = "Unk"; // Unknown
  if (month >= 0 && month < MONPERYEAR)
    monthName = ABBREV_MONTH_NAME[month];

  simCore::Seconds seconds = roundedStamp.secondsSinceRefYear() - simCore::Seconds(static_cast<double>(days) * simCore::SECPERDAY);
  // Rely on static_cast<> to floor the value
  int hours = static_cast<int>(seconds.Double() / SECPERHOUR);
  seconds -= hours * SECPERHOUR; // seconds now holds minutes+seconds past hour

  // EG 061435:03.010 Z Apr07
  ss << std::setw(2) << std::setfill('0') << monthDay
    << std::setw(2) << hours << std::setw(2);
  MinutesTimeFormatter::toStream(ss, seconds, precision);
  ss << " Z " << monthName << std::setw(2) << (realYear % 100);
  return ss.str();
}

bool DtgTimeFormatter::canConvert(const std::string& timeString) const
{
  const std::string& cleanString = simCore::StringUtils::trim(simCore::removeQuotes(timeString));
  if (cleanString.empty())
    return false;
  std::vector<std::string> timesZoneMonth;
  // Tokenize the string into 3 components (times, time zone, month)
  simCore::stringTokenizer(timesZoneMonth, cleanString, " ", false, true);
  // Validate the token numbers and sizes: MMMYY for month, and we only support Zulu time ("Z")
  if (timesZoneMonth.size() != 3 || timesZoneMonth[1] != "Z" || timesZoneMonth[2].size() != 5 ||
    timesZoneMonth[0].size() < 9 || timesZoneMonth[0][6] != ':' || timesZoneMonth[0][7] == '.' ||
    timesZoneMonth[0][8] == '.')
  {
    return false;
  }

  // Validate the month and year
  const std::string& monthYearString = timesZoneMonth[2];
  int month = MonthDayTimeFormatter::monthStringToInt(monthYearString.substr(0, 3));
  if (month == -1)
    return false;
  int year; // Need to decode the year in order to validate month-day
  if (!isValidNumber(monthYearString.substr(3), year, false))
    return false;
  year += (year >= 70) ? 1900 : 2000; // Valid from 1970 to 2069

  // Validate the digits on the time string; should be MMHHSS.sss with optional [.sss]
  try
  {
    const std::string& times = timesZoneMonth[0];
    int dayOfMonth = 0;
    int hours = 0;
    int minutes = 0;
    return SecondsTimeFormatter::isStrictSecondsString(times.substr(7)) &&
      isValidNumber(times.substr(0, 2), dayOfMonth, false) &&
      dayOfMonth >= 1 && dayOfMonth <= simCore::daysPerMonth(year - 1900, month) &&
      isValidNumber(times.substr(2, 2), hours, false) &&
      hours >= 0 && hours < HOURPERDAY &&
      isValidNumber(times.substr(4, 2), minutes, false) &&
      minutes >= 0 && minutes < MINPERHOUR;
  }
  catch (const simCore::TimeException&)
  {
  }
  return false;
}

int DtgTimeFormatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  // presume failure
  timeStamp = simCore::TimeStamp();

  std::vector<std::string> timesZoneMonth;
  // Tokenize the string into 3 components (times, time zone, month)
  simCore::stringTokenizer(timesZoneMonth, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ", false, true);
  // Validate the token numbers and sizes: MMMYY for month, and we only support Zulu time ("Z")
  if (timesZoneMonth.size() != 3 || timesZoneMonth[1] != "Z" || timesZoneMonth[2].size() != 5 || timesZoneMonth[0].size() < 9)
    return 1;

  // Pull out the various components
  int month = MonthDayTimeFormatter::monthStringToInt(timesZoneMonth[2].substr(0, 3));
  int year;
  int monthDay;
  int hours;
  int minutes;
  double seconds;
  const std::string& times = timesZoneMonth[0];
  if (isValidNumber(timesZoneMonth[2].substr(3), year) &&
    isValidNumber(times.substr(0, 2), monthDay) &&
    isValidNumber(times.substr(2, 2), hours) &&
    isValidNumber(times.substr(4, 2), minutes) &&
    isValidNumber(times.substr(7), seconds)) // Skip the colon
  {
    year += (year >= 70) ? 1900 : 2000; // Valid from 1970 to 2069
    // Convert into a TimeString
    try
    {
      int yearDay = getYearDay(month, monthDay, year - 1900);
      timeStamp = simCore::TimeStamp(year, yearDay * SECPERDAY + hours * SECPERHOUR + minutes * SECPERMIN + seconds);
      return 0;
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time exception: " << te.what() << std::endl;
    }
  }

  // Falling down to failure
  return 1;
}

///////////////////////////////////////////////////////////////////////

TimeFormatterRegistry::TimeFormatterRegistry(bool wrappedFormatters)
  : nullFormatter_(new NullTimeFormatter),
    lastUsedFormatter_(nullFormatter_)
{
  knownFormatters_[TIMEFORMAT_SECONDS] = TimeFormatterPtr(new SecondsTimeFormatter);
  if (wrappedFormatters)
  {
    knownFormatters_[TIMEFORMAT_MINUTES] = TimeFormatterPtr(new MinutesWrappedTimeFormatter);
    knownFormatters_[TIMEFORMAT_HOURS] = TimeFormatterPtr(new HoursWrappedTimeFormatter);
  }
  else
  {
    knownFormatters_[TIMEFORMAT_MINUTES] = TimeFormatterPtr(new MinutesTimeFormatter);
    knownFormatters_[TIMEFORMAT_HOURS] = TimeFormatterPtr(new HoursTimeFormatter);
  }
  knownFormatters_[TIMEFORMAT_ORDINAL] = TimeFormatterPtr(new OrdinalTimeFormatter);
  knownFormatters_[TIMEFORMAT_MONTHDAY] = TimeFormatterPtr(new MonthDayTimeFormatter);
  knownFormatters_[TIMEFORMAT_DTG] = TimeFormatterPtr(new DtgTimeFormatter);

  registerCustomFormatter(TimeFormatterPtr(new Deprecated::DDD_HHMMSS_Formatter));
  registerCustomFormatter(TimeFormatterPtr(new Deprecated::DDD_HHMMSS_YYYY_Formatter));
  registerCustomFormatter(TimeFormatterPtr(new Deprecated::MD_MON_YYYY_HHMMSS_Formatter));
  registerCustomFormatter(TimeFormatterPtr(new Deprecated::MON_MD_HHMMSS_YYYY_Formatter));
  registerCustomFormatter(TimeFormatterPtr(new Deprecated::WKD_MON_MD_HHMMSS_Formatter));
  registerCustomFormatter(TimeFormatterPtr(new Deprecated::WKD_MON_MD_HHMMSS_YYYY_Formatter));
}

TimeFormatterRegistry::~TimeFormatterRegistry()
{
}

void TimeFormatterRegistry::registerCustomFormatter(TimeFormatterPtr formatter)
{
  if (formatter != NULL)
    foreignFormatters_.push_back(formatter);
}

const TimeFormatter& TimeFormatterRegistry::formatter(simCore::TimeFormat format) const
{
  std::map<int, TimeFormatterPtr>::const_iterator i = knownFormatters_.find(format);
  if (i != knownFormatters_.end() && i->second != NULL)
    return *i->second;
  return *nullFormatter_;
}

const TimeFormatter& TimeFormatterRegistry::formatter(const std::string& timeString) const
{
  // Examine the cached formatter first to improve performance
  TimeFormatterPtr lastFormatter = lastUsedFormatter_;
  if (lastFormatter->canConvert(timeString))
    return *lastFormatter;

  // Look through foreign formatters first
  for (std::vector<TimeFormatterPtr>::const_iterator i = foreignFormatters_.begin(); i != foreignFormatters_.end(); ++i)
  {
    // Don't double-check the last-used formatter
    if (*i != lastUsedFormatter_ && (*i)->canConvert(timeString))
    {
      lastUsedFormatter_ = *i;
      return **i;
    }
  }

  // Check our well-known formatters
  for (std::map<int, TimeFormatterPtr>::const_iterator i = knownFormatters_.begin(); i != knownFormatters_.end(); ++i)
  {
    // Don't double-check the last-used formatter
    if (i->second != lastUsedFormatter_ && i->second->canConvert(timeString))
    {
      lastUsedFormatter_ = i->second;
      return *i->second;
    }
  }
  lastUsedFormatter_ = nullFormatter_;
  return *nullFormatter_;
}

std::string TimeFormatterRegistry::toString(simCore::TimeFormat format, const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  const TimeFormatter& printer = formatter(format);
  return printer.toString(timeStamp, referenceYear, precision);
}

int TimeFormatterRegistry::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  const TimeFormatter& parser = formatter(timeString);
  return parser.fromString(timeString, timeStamp, referenceYear);
}

}
