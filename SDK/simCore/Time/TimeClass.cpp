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
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <limits>
#include <locale>
#include <time.h>

#include "simCore/Common/Exception.h"
#include "simCore/Calc/Math.h"
#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Time/Constants.h"
#include "simCore/Time/Exception.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Utils.h"

namespace simCore {

static const unsigned int SECPERYEAR = SECPERDAY * 365; // seconds in a standard non-leap year: 31536000
static const int MAX_FIX = MAX_TIME_YEAR - MIN_TIME_YEAR + 1;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Seconds methods

void Seconds::fix_()
{
  // ensure that fraction is within precision limits
  if ((fraction_ >= static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT)) ||
    (fraction_ <= -static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT)))
  {
    seconds_ += (fraction_ / static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT));
    fraction_ = fraction_ % static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT);
  }

  // ensure that Seconds has a single representation for all equivalent values.
  // convention adopted here is: fix_() ensures that fraction is always positive.
  // this entails that once fix_()'d:
  //   Seconds::getSeconds() == floor(Seconds::Double())
  //   for negative non-integer values: Seconds::getSeconds() != (int)Seconds::Double()
  //   this might be unexpected behavior, especially for negative values.

  if (fraction_ < 0)
  {
    seconds_--;
    fraction_ += static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT);
  }
}

//------------------------------------------------------------------------

void Seconds::convert_(double dtime)
{
  // maximum storage value of class is INT_MAX
  if (dtime >= std::numeric_limits<int64_t>::max())
  {
    seconds_ = std::numeric_limits<int64_t>::max();
    fraction_ = 0;
    return;
  }
  // minimum storage value of class is INT_MIN
  if (dtime <= std::numeric_limits<int64_t>::min())
  {
    seconds_ = std::numeric_limits<int64_t>::min();
    fraction_ =  0;
    return;
  }

  seconds_ = static_cast<int64_t>(dtime);
  fraction_ =  (dtime < 0) ?
    static_cast<int>((dtime - seconds_ - INPUT_ROUND_UP_VALUE) * INPUT_CONV_FACTOR_PREC_LIMIT) :
    static_cast<int>((dtime - seconds_ + INPUT_ROUND_UP_VALUE) * INPUT_CONV_FACTOR_PREC_LIMIT);
  fix_();
}

//------------------------------------------------------------------------

double Seconds::convert_() const
{
  // implicit cast to double
  return (OUTPUT_CONV_FACTOR * fraction_) + seconds_;
}

//------------------------------------------------------------------------

Seconds& Seconds::operator = (const Seconds& time)
{
  if (this != &time)
  {
    seconds_ = time.seconds_;
    fraction_ = time.fraction_;
  }
  return *this;
}

//------------------------------------------------------------------------

Seconds Seconds::rounded(unsigned short toPrecision) const
{
  // Nanosecond precision is best we can do
  if (toPrecision > 8)
    return *this;
  int powerOfTen = static_cast<int>(pow(10.0, 8 - toPrecision));
  // Note MAX_INT means we can't go over ~2.1e10; we here can add to 1.5e10, which is within limits
  return Seconds(seconds_, 10 * powerOfTen * ((fraction_ + 5 * powerOfTen) / (10 * powerOfTen)));
}

//------------------------------------------------------------------------

Seconds Seconds::operator + (const Seconds& time) const
{
  return Seconds(seconds_ + time.seconds_, fraction_ + time.fraction_);
}

//------------------------------------------------------------------------

Seconds Seconds::operator - (const Seconds& time) const
{
  return Seconds(seconds_ - time.seconds_, fraction_ - time.fraction_);
}

//------------------------------------------------------------------------

Seconds Seconds::operator * (const Seconds& time) const
{
  if (time == ZERO_SECONDS || (seconds_ == 0 && fraction_ == 0))
    return ZERO_SECONDS;

  return Seconds(Double() * time.Double());
}

//------------------------------------------------------------------------

Seconds Seconds::operator / (const Seconds& time) const
{
  // catch divide by zero
  if (time == ZERO_SECONDS)
    return ZERO_SECONDS;

  return Seconds(Double() / time.Double());
}

//------------------------------------------------------------------------

TimeCompVal Seconds::compare(const Seconds& time) const
{
  if (seconds_ > time.seconds_)
    return TCV_GREATER;
  if (seconds_ < time.seconds_)
    return TCV_LESS;
  if (fraction_ > (time.fraction_ + 1))
    return TCV_GREATER;
  if ((fraction_ + 1) < time.fraction_)
    return TCV_LESS;
  return TCV_EQUAL;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// TimeStamp methods

TimeStamp::TimeStamp()
: referenceYear_(currentYear())
{
  fix_();
}

//------------------------------------------------------------------------

TimeStamp::TimeStamp(int refYear, const Seconds& secondsSinceRefYear)
{
  setTime(refYear, secondsSinceRefYear);
}

//------------------------------------------------------------------------

int TimeStamp::fixRequired_() const
{
  const double secondsSinceRefYear = secondsSinceRefYear_.Double();
  if (secondsSinceRefYear < 0.)
    return 1;
  const double secondsInRefYear = SECPERDAY * daysPerYear(referenceYear_);
  return (secondsSinceRefYear < secondsInRefYear) ?  0 : 2;
}

//------------------------------------------------------------------------

void TimeStamp::fix_()
{
  if (referenceYear_ == INFINITE_TIME_YEAR)
  {
    secondsSinceRefYear_ = ZERO_SECONDS;
    return;
  }
  if (referenceYear_ < MIN_TIME_YEAR)
  {
    *this = MIN_TIME_STAMP;
    return;
  }
  if (referenceYear_ > MAX_TIME_YEAR)
  {
    *this = MAX_TIME_STAMP;
    return;
  }
  if (secondsSinceRefYear_ == ZERO_SECONDS)
    return;
  if (0 == fixRequired_())
    return;

  // treat all intervening years as non-leap years
  const double years = secondsSinceRefYear_.getSeconds() >= 0 ? floor(secondsSinceRefYear_.Double() / SECPERYEAR) : ceil(secondsSinceRefYear_.Double() / SECPERYEAR);
  if (fabs(years) > MAX_FIX)
  {
    if (years < 0)
    {
      *this = MIN_TIME_STAMP;
      return;
    }
    *this = MAX_TIME_STAMP;
    return;
  }

  const int64_t seconds = static_cast<int64_t>(years * SECPERYEAR);
  secondsSinceRefYear_ -= Seconds(seconds, 0);
  int newReferenceYear = referenceYear_ + static_cast<int>(years);
  // now account for the leap days in those years
  const int leapDays = simCore::leapDays(newReferenceYear - 1900) - simCore::leapDays(referenceYear_ - 1900);
  secondsSinceRefYear_ -= Seconds(leapDays * SECPERDAY, 0);

  // leap days calculation may result in reference year needing to be corrected +/- 1
  const int validFixResult = fixRequired_();
  if (validFixResult == 1)
  {
    newReferenceYear--;
    const int secondsInRefYear = SECPERDAY * daysPerYear(newReferenceYear);
    secondsSinceRefYear_ += Seconds(secondsInRefYear, 0);
  }
  else if (validFixResult == 2)
  {
    // The code above should prevent this
    assert(false);
    const int secondsInRefYear = SECPERDAY * daysPerYear(newReferenceYear);
    newReferenceYear++;
    secondsSinceRefYear_ -= Seconds(secondsInRefYear, 0);
  }
  if (newReferenceYear < MIN_TIME_YEAR)
  {
    *this = MIN_TIME_STAMP;
    return;
  }
  if (newReferenceYear > MAX_TIME_YEAR)
  {
    *this = MAX_TIME_STAMP;
    return;
  }
  referenceYear_ = newReferenceYear;
  return;
}

//------------------------------------------------------------------------

Seconds TimeStamp::secondsSinceRefYear(int refYear) const
{
  if (referenceYear_ == refYear)
    return secondsSinceRefYear_;
  const TimeStamp ref(refYear, ZERO_SECONDS);
  return *this - ref;
}

//------------------------------------------------------------------------

void TimeStamp::setTime(int refYear, const Seconds& secondsSinceRefYear)
{
  // In an attempt to catch parameter reversal problems, we are also checking the
  // actual year of the date.  It should generally be between 1900 and INFINITE_TIME_YEAR.  This is
  // of course a soft limit, but this code will likely not exist past INFINITE_TIME_YEAR.
  assert(refYear >= 1900 && (refYear <= INFINITE_TIME_YEAR));

  referenceYear_ = refYear;
  secondsSinceRefYear_ = secondsSinceRefYear;
  fix_();
}

//------------------------------------------------------------------------

TimeStamp& TimeStamp::operator = (const TimeStamp& time)
{
  if (this != &time)
  {
    referenceYear_ = time.referenceYear_;
    secondsSinceRefYear_ = time.secondsSinceRefYear_;
  }
  return *this;
}

//------------------------------------------------------------------------

Seconds TimeStamp::operator - (const TimeStamp& t) const
{
  // If either year presents a infinity return with zero seconds
  if ((referenceYear_ == INFINITE_TIME_YEAR) || (t.referenceYear_ == INFINITE_TIME_YEAR))
    return ZERO_SECONDS;

  const int yearDifference = (referenceYear_ - t.referenceYear_);
  if (std::abs(yearDifference) > (MAX_TIME_STAMP.referenceYear() - MIN_TIME_STAMP.referenceYear()))
  {
    // class only tested between years MIN_YEAR and MAX_YEAR
    assert(false);
    return ZERO_SECONDS;
  }

  Seconds secondsValue;

  if (yearDifference > 0)
  {
    secondsValue = Seconds(SECPERDAY * daysPerYear(t.referenceYear_), 0) - t.secondsSinceRefYear_;
    for (auto year = t.referenceYear_ + 1; year < referenceYear_; ++year)
      secondsValue += Seconds(SECPERDAY * daysPerYear(year), 0);
    secondsValue += secondsSinceRefYear_;
  }
  else if (yearDifference < 0)
  {
    secondsValue = (Seconds(-1, 0) * t.secondsSinceRefYear_);
    for (auto year = t.referenceYear_ - 1; year > referenceYear_; --year)
      secondsValue -= Seconds(SECPERDAY * daysPerYear(year), 0);
    secondsValue -= (Seconds(SECPERDAY * daysPerYear(referenceYear_), 0) - secondsSinceRefYear_);
  }
  else
  {
    secondsValue = secondsSinceRefYear_ - t.secondsSinceRefYear_;
  }

  return secondsValue;
}

//------------------------------------------------------------------------

TimeStamp TimeStamp::operator - (const Seconds& s) const
{
  return TimeStamp(referenceYear_, secondsSinceRefYear_ - s);
}

//------------------------------------------------------------------------

TimeStamp TimeStamp::operator + (const Seconds& s) const
{
  return TimeStamp(referenceYear_, secondsSinceRefYear_ + s);
}

//------------------------------------------------------------------------

TimeCompVal TimeStamp::compare_(const TimeStamp& time) const
{
  if (referenceYear_ > time.referenceYear_)
    return TCV_GREATER;
  if (referenceYear_ < time.referenceYear_)
    return TCV_LESS;
  return secondsSinceRefYear_.compare(time.secondsSinceRefYear_);
}

//------------------------------------------------------------------------

double getFactor(const TimeStamp &lowVal, const TimeStamp &exactVal, const TimeStamp &highVal)
{
  // Perform bounds check and prevent divide by zero
  if (exactVal <= lowVal) return 0.;
  if (exactVal >= highVal || (highVal - lowVal) == 0) return 1.;
  return (exactVal - lowVal) / (highVal - lowVal);
}

//------------------------------------------------------------------------

void TimeStamp::getTimeComponents(unsigned int& day, unsigned int& hour, unsigned int& min, unsigned int& sec) const
{
  int64_t time = secondsSinceRefYear().getSeconds();
  // TimeStamp fix() always normalizes Seconds to a positive number.
  assert(time >= 0);
  day = static_cast<unsigned int>(time / simCore::SECPERDAY);

  time -= (day*simCore::SECPERDAY);
  hour = static_cast<unsigned int>(time / simCore::SECPERHOUR);

  time -= (hour*simCore::SECPERHOUR);
  min = static_cast<unsigned int>(time / simCore::SECPERMIN);

  time -= (min*simCore::SECPERMIN);
  sec = static_cast<unsigned int>(time);
}

//------------------------------------------------------------------------

/**
 * MSVC strftime() and put_time() both execute the invalid parameter handler in cases
 * of invalid parameters. In debug mode, this may also assert. The default implementation
 * will fatally terminate the application, which is almost always not what we want. This
 * is not a problem on Linux, so MSVC implementation installs a temporary handler using
 * this class.
 */
class InvalidParameterDetection
{
public:
#ifndef _MSC_VER
  InvalidParameterDetection() {}
  virtual ~InvalidParameterDetection() {}

#else
  InvalidParameterDetection()
  {
    oldHandler_ = _get_invalid_parameter_handler();
    _set_invalid_parameter_handler(&InvalidParameterDetection::invalidParameter_);
  }

  virtual ~InvalidParameterDetection()
  {
    _set_invalid_parameter_handler(oldHandler_);
  }

private:
  static void invalidParameter_(const wchar_t* expression, const wchar_t* function,
    const wchar_t* file, unsigned int line, uintptr_t pReserved)
  {
    throw simCore::TimeException(0, "Invalid parameter detected");
  }

  _invalid_parameter_handler oldHandler_;
#endif
};

//------------------------------------------------------------------------------------------

TimeStampStr::TimeStampStr()
{
#ifdef _MSC_VER
  is_.imbue(std::locale(setlocale(LC_ALL, nullptr)));
#endif
}
TimeStampStr::~TimeStampStr()
{
}

int TimeStampStr::strptime(TimeStamp& timeStamp, const std::string& timeStr, const std::string& format, std::string* remainder)
{
  // Avoid any simCore time utility exceptions
  try {
    std::tm tm = {};
    if (remainder)
      remainder->clear();

#ifdef _MSC_VER
    // Adapted from https://stackoverflow.com/questions/321849/strptime-equivalent-on-windows
    is_.clear();
    is_.str(timeStr);

    // Note that std::get_Time wasn't implemented in GCC until 5.1, and is
    // reported as broken in MSVC 2015.
    is_ >> std::get_time(&tm, format.c_str());
    if (is_.fail())
    {
      if (remainder)
        *remainder = timeStr;
      return 1;
    }

    // Fill out the remainder value
    if (remainder && !is_.eof())
      *remainder = timeStr.substr(is_.tellg());
#else
    // Linux use strptime(), which returns null on error
    const char* rv = ::strptime(timeStr.c_str(), format.c_str(), &tm);
    if (!rv)
    {
      if (remainder)
        *remainder = timeStr;
      return 1;
    }
    if (remainder)
      *remainder = rv;
#endif

    // Make sane values for year, mday, and mon, before calling mktime()
    tm.tm_year = simCore::sdkMax(70, tm.tm_year);
    tm.tm_mday = simCore::sdkMax(tm.tm_mday, 1);
    if (tm.tm_yday > 0 && tm.tm_mday == 1 && tm.tm_mon == 0)
      simCore::getMonthAndDayOfMonth(tm.tm_mon, tm.tm_mday, tm.tm_year, tm.tm_yday);

    // mktime() will return in local time, use timezone to offset back to UTC
#ifdef _MSC_VER
    auto asTime = std::mktime(&tm) - _timezone;
#else
    auto asTime = std::mktime(&tm) - timezone;
#endif

    // Check for failure state
    if (asTime < 0)
    {
      if (remainder)
        *remainder = timeStr;
      return 1;
    }

    timeStamp.setTime(1970, static_cast<double>(asTime));
    return 0;
  }
  catch (const simCore::TimeException&)
  {
    // noop
  }
  return 1;
}

int TimeStampStr::strptime(TimeStamp& timeStamp, const std::string& timeStr, const std::string& format)
{
  std::string remainder;
  if (strptime(timeStamp, timeStr, format, &remainder) != 0)
    return 1;

  // MSVC doesn't respect %f in format string, so process remainder of time string
  remainder = simCore::StringUtils::trim(remainder);
  if (!remainder.empty())
  {
    // Remainder might look like ".17482"
    double decimalSeconds = atof(remainder.c_str());
    if (decimalSeconds != 0.0)
      timeStamp += decimalSeconds;
  }

  return 0;
}

std::string TimeStampStr::strftime(const TimeStamp& timeStamp, const std::string& format)
{
  // Avoid testing INFINITE_TIME_STAMP
  if (simCore::INFINITE_TIME_STAMP == timeStamp)
    return "";

  // Do not remove; cppCheck flags as unused but the constructor and destructor do actual work.
  InvalidParameterDetection detectInvalid;

  const auto& timeStruct = simCore::getTimeStruct(timeStamp);

  // Try/catch since MSVC version will throw an exception on error
  SAFETRYBEGIN;

#if defined(__GNUC__) && __GNU__C < 5
  // g++ 5.x is first to support std::put_time(), although it's a C++11 feature.
  // Provides a fallback of ::strftime() if put_time() is unavailable.
  std::string localFormatStr = format;
  localFormatStr += '\a'; // Avoid infinite loop by having some content
  std::string buffer;
  buffer.resize(format.size());
  int bufferLen = ::strftime(&buffer[0], buffer.size(), localFormatStr.c_str(), &timeStruct);
  while (bufferLen == 0)
  {
    buffer.resize(buffer.size() * 2);
    bufferLen = ::strftime(&buffer[0], buffer.size(), localFormatStr.c_str(), &timeStruct);
  }
  // Remove the character added to local format string
  buffer.resize(bufferLen - 1);
  return buffer;
#else
  // C++11-compatible put_time()
  os_.clear();
  os_.str(std::string());
  os_ << std::put_time(&timeStruct, format.c_str());
  return os_.str();
#endif

  SAFETRYEND("during TimeStamp::strftime");
  return "";
}

}
