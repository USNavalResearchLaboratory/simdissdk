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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <limits>

#include "simCore/Calc/Math.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Constants.h"

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

  int year = 0;
  Seconds secondsValue;

  if (yearDifference > 0)
  {
    secondsValue = Seconds(SECPERDAY * daysPerYear(t.referenceYear_), 0) - t.secondsSinceRefYear_;
    for (year = t.referenceYear_ + 1; year < referenceYear_; ++year)
      secondsValue += Seconds(SECPERDAY * daysPerYear(year), 0);
    secondsValue += secondsSinceRefYear_;
  }
  else if (yearDifference < 0)
  {
    secondsValue = (Seconds(-1, 0) * t.secondsSinceRefYear_);
    for (year = t.referenceYear_ - 1; year > referenceYear_; --year)
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
}
