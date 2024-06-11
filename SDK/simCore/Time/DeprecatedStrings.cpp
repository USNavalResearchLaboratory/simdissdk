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
#include <sstream>
#include <iomanip>
#include "simNotify/Notify.h"
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/DeprecatedStrings.h"
#include "simCore/Time/Exception.h"

namespace simCore { namespace Deprecated {

///////////////////////////////////////////////////////////////////////

std::string DDD_HHMMSS_YYYY_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  const int days = static_cast<int>(timeStamp.secondsSinceRefYear().getSeconds() / simCore::SECPERDAY);
  const simCore::Seconds seconds = timeStamp.secondsSinceRefYear() - simCore::Seconds(days * simCore::SECPERDAY, 0);
  std::stringstream ss;
  ss << std::setw(3) << std::setfill('0') << (days + 1) << " ";
  HoursTimeFormatter::toStream(ss, seconds, precision);
  ss << " " << timeStamp.referenceYear();
  return ss.str();
}

bool DDD_HHMMSS_YYYY_Formatter::canConvert(const std::string& timeString) const
{
  int days;
  simCore::Seconds seconds;
  int year;
  return (getComponents_(timeString, days, seconds, year) == 0);
}

int DDD_HHMMSS_YYYY_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int days;
  simCore::Seconds seconds;
  int year;
  if (getComponents_(timeString, days, seconds, year) != 0)
  {
    timeStamp = simCore::MIN_TIME_STAMP;
    return 1;
  }
  timeStamp = simCore::TimeStamp(year, (days - 1) * SECPERDAY + seconds);
  return 0;
}

int DDD_HHMMSS_YYYY_Formatter::getComponents_(const std::string& timeString, int& days, simCore::Seconds& seconds, int& year) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 3 ||
    !isValidNumber(vec[2], year, false) || year < 1900 || year > 9999 ||
    !HoursTimeFormatter::isStrictHoursString(vec[1]) ||
    !OrdinalTimeFormatter::isValidOrdinal(vec[0], year, days))
  {
    return 1;
  }
  return HoursTimeFormatter::fromString(vec[1], seconds);
}

///////////////////////////////////////////////////////////////////////

std::string DDD_HHMMSS_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  // Make sure we can represent the actual year; if not, fall back to the real ordinal format
  if (timeStamp.referenceYear() != referenceYear)
  {
    OrdinalTimeFormatter::toStream(ss, timeStamp, precision);
  }
  else
  {
    const int days = static_cast<int>(timeStamp.secondsSinceRefYear().getSeconds() / simCore::SECPERDAY);
    const simCore::Seconds seconds = timeStamp.secondsSinceRefYear() - simCore::Seconds(days * simCore::SECPERDAY, 0);
    ss << std::setw(3) << std::setfill('0') << (days + 1) << " ";
    HoursTimeFormatter::toStream(ss, seconds, precision, true);
  }
  return ss.str();
}

bool DDD_HHMMSS_Formatter::canConvert(const std::string& timeString) const
{
  int days;
  simCore::Seconds seconds;
  // Be strict, don't let in 366 because we don't know if it's a leap year
  return (getComponents_(timeString, days, seconds, 1970) == 0);
}

int DDD_HHMMSS_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int days;
  simCore::Seconds seconds;
  if (getComponents_(timeString, days, seconds, referenceYear) != 0)
  {
    timeStamp = simCore::MIN_TIME_STAMP;
    return 1;
  }
  timeStamp = simCore::TimeStamp(referenceYear, (days - 1) * SECPERDAY + seconds);
  return 0;
}

int DDD_HHMMSS_Formatter::getComponents_(const std::string& timeString, int& days, simCore::Seconds& seconds, int referenceYear) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 2 ||
    !HoursTimeFormatter::isStrictHoursString(vec[1]) ||
    !OrdinalTimeFormatter::isValidOrdinal(vec[0], referenceYear, days))
  {
    return 1;
  }
  return HoursTimeFormatter::fromString(vec[1], seconds);
}

///////////////////////////////////////////////////////////////////////

std::string MON_MD_HHMMSS_YYYY_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  simCore::Seconds seconds;

  // Get components: In case of extreme error, fall back to Ordinal, which can't have exception issues
  if (MonthDayTimeFormatter::getMonthComponents(timeStamp, month, monthDay, seconds) != 0)
  {
    OrdinalTimeFormatter::toStream(ss, timeStamp, precision);
    return ss.str();
  }

  // EG Jan 13 00:01:02.03 2014
  ss << MonthDayTimeFormatter::monthIntToString(month) << " " << monthDay << " " << std::setw(2) << std::setfill('0');
  HoursTimeFormatter::toStream(ss, seconds, precision);
  ss << " " << timeStamp.referenceYear();
  return ss.str();
}

bool MON_MD_HHMMSS_YYYY_Formatter::canConvert(const std::string& timeString) const
{
  int month;
  int monthDay;
  simCore::Seconds seconds;
  int year;
  return (getComponents_(timeString, month, monthDay, seconds, year) == 0);
}

int MON_MD_HHMMSS_YYYY_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int month;
  int monthDay;
  simCore::Seconds seconds;
  int year;
  if (getComponents_(timeString, month, monthDay, seconds, year) == 0)
  {
    try
    {
      timeStamp.setTime(year, SECPERDAY * simCore::getYearDay(month, monthDay, year) + seconds);
      return 0;
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time Exception: " << te.what() << std::endl;
    }
  }
  timeStamp = simCore::TimeStamp();
  return 1;
}

int MON_MD_HHMMSS_YYYY_Formatter::getComponents_(const std::string& timeString, int& month, int& monthDay, simCore::Seconds& seconds, int& year) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 4 ||
    !isValidNumber(vec[1], monthDay, false) ||
    !HoursTimeFormatter::isStrictHoursString(vec[2]) ||
    !isValidNumber(vec[3], year, false) || year < 1900 || year > 9999)
  {
    return 1;
  }
  // Validate the month/monthday
  month = MonthDayTimeFormatter::monthStringToInt(vec[0]);
  try
  {
    if (month != -1 && monthDay >= 1 && monthDay <= simCore::daysPerMonth(year, month))
    {
      return HoursTimeFormatter::fromString(vec[2], seconds);
    }
  }
  catch (const simCore::TimeException&)
  {
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////

std::string MD_MON_YYYY_HHMMSS_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  simCore::Seconds seconds;

  // Get components: In case of extreme error, fall back to Ordinal, which can't have exception issues
  if (MonthDayTimeFormatter::getMonthComponents(timeStamp, month, monthDay, seconds) != 0)
  {
    OrdinalTimeFormatter::toStream(ss, timeStamp, precision);
    return ss.str();
  }

  // EG 13 Jan 2014 00:01:02.03
  ss << monthDay << " " << MonthDayTimeFormatter::monthIntToString(month) << " " <<
    timeStamp.referenceYear() << " " << std::setw(2) << std::setfill('0');
  HoursTimeFormatter::toStream(ss, seconds, precision);
  return ss.str();
}

bool MD_MON_YYYY_HHMMSS_Formatter::canConvert(const std::string& timeString) const
{
  int monthDay;
  int month;
  int year;
  simCore::Seconds seconds;
  return (getComponents_(timeString, monthDay, month, year, seconds) == 0);
}

int MD_MON_YYYY_HHMMSS_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int monthDay;
  int month;
  int year;
  simCore::Seconds seconds;
  if (getComponents_(timeString, monthDay, month, year, seconds) == 0)
  {
    try
    {
      timeStamp.setTime(year, SECPERDAY * simCore::getYearDay(month, monthDay, year) + seconds);
      return 0;
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time Exception: " << te.what() << std::endl;
    }
  }
  timeStamp = simCore::TimeStamp();
  return 1;
}

int MD_MON_YYYY_HHMMSS_Formatter::getComponents_(const std::string& timeString, int& monthDay, int& month, int& year, simCore::Seconds& seconds) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 4 ||
    !isValidNumber(vec[0], monthDay, false) ||
    !isValidNumber(vec[2], year, false) || year < 1900 || year > 9999 ||
    !HoursTimeFormatter::isStrictHoursString(vec[3]))
  {
    return 1;
  }
  // Validate the month/monthday
  month = MonthDayTimeFormatter::monthStringToInt(vec[1]);
  try
  {
    if (month != -1 && monthDay >= 1 && monthDay <= simCore::daysPerMonth(year, month))
    {
      return HoursTimeFormatter::fromString(vec[3], seconds);
    }
  }
  catch (const simCore::TimeException&)
  {
  }
  return 1;
}

///////////////////////////////////////////////////////////////////////

std::string WKD_MON_MD_HHMMSS_YYYY_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  WKD_MON_MD_HHMMSS_YYYY_Formatter::toStream(ss, timeStamp, precision);
  return ss.str();
}

bool WKD_MON_MD_HHMMSS_YYYY_Formatter::canConvert(const std::string& timeString) const
{
  int weekDay;
  int month;
  int monthDay;
  simCore::Seconds seconds;
  int year;
  return 0 == getComponents_(timeString, weekDay, month, monthDay, seconds, year);
}

int WKD_MON_MD_HHMMSS_YYYY_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int weekDay;
  int month;
  int monthDay;
  simCore::Seconds seconds;
  int year;
  if (0 == getComponents_(timeString, weekDay, month, monthDay, seconds, year))
  {
    try
    {
      timeStamp.setTime(year, SECPERDAY * simCore::getYearDay(month, monthDay, year) + seconds);
      return 0;
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time Exception: " << te.what() << std::endl;
    }
  }
  timeStamp = simCore::TimeStamp();
  return 1;
}

void WKD_MON_MD_HHMMSS_YYYY_Formatter::toStream(std::ostream& os, const simCore::TimeStamp& timeStamp, unsigned short precision)
{
  // Values for getMonthComponents() below
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  simCore::Seconds seconds;

  // Get components: In case of extreme error, fall back to Ordinal, which can't have exception issues
  if (MonthDayTimeFormatter::getMonthComponents(timeStamp, month, monthDay, seconds) != 0)
  {
    OrdinalTimeFormatter::toStream(os, timeStamp, precision);
    return;
  }
  std::string weekDay = WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayString(timeStamp);

  // EG Mon Jan 1 00:01:02.03 1981
  os << weekDay << " " << MonthDayTimeFormatter::monthIntToString(month) << " " << monthDay
    << " " << std::setw(2) << std::setfill('0');
  HoursTimeFormatter::toStream(os, seconds, precision);
  os << " " << timeStamp.referenceYear();
}

std::string WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayString(const simCore::TimeStamp& timeStamp)
{
  int days = static_cast<int>(timeStamp.secondsSinceRefYear().Double() / simCore::SECPERDAY);
  try
  {
    int weekDay = simCore::getWeekDay(timeStamp.referenceYear() - 1900, days);
    // Bounds check on the return
    if (weekDay >= 0 && weekDay < 7)
      return ABBREV_WEEKDAY_NAME[weekDay];
  }
  catch (const simCore::TimeException&)
  {
  }
  return "Unk"; // Unknown weekday
}

int WKD_MON_MD_HHMMSS_YYYY_Formatter::getComponents_(const std::string& timeString, int& weekDay, int& month, int& monthDay, simCore::Seconds& seconds, int& year) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 5 ||
    !isValidNumber(vec[2], monthDay, false) ||
    !HoursTimeFormatter::isStrictHoursString(vec[3]) ||
    !isValidNumber(vec[4], year, false) || year < 1900 || year > 9999)
  {
    return 1;
  }
  // Validate the month/monthday and weekday
  month = MonthDayTimeFormatter::monthStringToInt(vec[1]);
  weekDay = WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayStringToInt(vec[0]);
  try
  {
    // Ignore the weekday except to make sure it's a valid one
    if (month != -1 && weekDay != -1 && monthDay >= 1 &&
      monthDay <= simCore::daysPerMonth(year, month))
    {
      return HoursTimeFormatter::fromString(vec[3], seconds);
    }
  }
  catch (const simCore::TimeException&)
  {
  }
  return 1;
}

int WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayStringToInt(const std::string& weekDay)
{
  for (int k = 0; k < 7; ++k)
  {
    if (simCore::caseCompare(weekDay, simCore::ABBREV_WEEKDAY_NAME[k]) == 0)
      return k;
  }
  // Special case for Thr (alternate legacy spelling
  if (simCore::caseCompare(weekDay, "Thr") == 0)
    return 3; // Index of "Thu"
  return -1;
}

///////////////////////////////////////////////////////////////////////

std::string WKD_MON_MD_HHMMSS_Formatter::toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision) const
{
  std::stringstream ss;
  // Make sure we can represent the actual year; if not, fall back to the format with a year
  if (timeStamp.referenceYear() != referenceYear)
  {
    WKD_MON_MD_HHMMSS_YYYY_Formatter::toStream(ss, timeStamp, precision);
    return ss.str();
  }

  // Values for getMonthComponents() below
  int month = 0; // Between 0-11
  int monthDay = 0; // Between 1-31
  simCore::Seconds seconds;

  // Get components: In case of extreme error, fall back to Ordinal, which can't have exception issues
  // Note that falling back to the weekday formatter won't work here.
  if (MonthDayTimeFormatter::getMonthComponents(timeStamp, month, monthDay, seconds) != 0)
  {
    OrdinalTimeFormatter::toStream(ss, timeStamp, precision);
    return ss.str();
  }
  std::string weekDay = WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayString(timeStamp);

  // EG Mon Jan 1 00:01:02.03
  ss << weekDay << " " << MonthDayTimeFormatter::monthIntToString(month) << " " << monthDay
    << " " << std::setw(2) << std::setfill('0');
  HoursTimeFormatter::toStream(ss, seconds, precision);
  return ss.str();
}

bool WKD_MON_MD_HHMMSS_Formatter::canConvert(const std::string& timeString) const
{
  int weekDay;
  int month;
  int monthDay;
  simCore::Seconds seconds;
  // Be strict, don't let in Feb 29 because we don't know if it's a leap year
  return 0 == getComponents_(timeString, weekDay, month, monthDay, seconds, 1970);
}

int WKD_MON_MD_HHMMSS_Formatter::fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const
{
  int weekDay;
  int month;
  int monthDay;
  simCore::Seconds seconds;
  if (0 == getComponents_(timeString, weekDay, month, monthDay, seconds, referenceYear))
  {
    try
    {
      timeStamp.setTime(referenceYear, SECPERDAY * simCore::getYearDay(month, monthDay, referenceYear) + seconds);
      return 0;
    }
    catch (const simCore::TimeException& te)
    {
      SIM_ERROR << "Time Exception: " << te.what() << std::endl;
    }
  }
  timeStamp = simCore::TimeStamp();
  return 1;
}

int WKD_MON_MD_HHMMSS_Formatter::getComponents_(const std::string& timeString, int& weekDay, int& month, int& monthDay, simCore::Seconds& seconds, int referenceYear) const
{
  std::vector<std::string> vec;
  simCore::stringTokenizer(vec, simCore::StringUtils::trim(simCore::removeQuotes(timeString)), " ");
  if (vec.size() != 4 ||
    !isValidNumber(vec[2], monthDay, false) ||
    !HoursTimeFormatter::isStrictHoursString(vec[3]))
  {
    return 1;
  }
  // Validate the month/monthday and weekday
  month = MonthDayTimeFormatter::monthStringToInt(vec[1]);
  weekDay = WKD_MON_MD_HHMMSS_YYYY_Formatter::weekDayStringToInt(vec[0]);
  try
  {
    // Ignore the weekday except to make sure it's a valid one
    if (month != -1 && weekDay != -1 && monthDay >= 1 &&
      monthDay <= simCore::daysPerMonth(referenceYear, month))
    {
      return HoursTimeFormatter::fromString(vec[3], seconds);
    }
  }
  catch (const simCore::TimeException&)
  {
  }
  return 1;
}

} }
