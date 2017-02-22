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
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <limits>

#include "simCore/Calc/Math.h"
#include "simCore/Time/Utils.h"
#include "simCore/Time/TimeClass.h"
#include "simCore/Time/Constants.h"

using namespace simCore;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// Seconds methods

void Seconds::fix_()
{
  if (fraction_ >= 0)
  {
    while (fraction_ >= INPUT_CONV_FACTOR_PREC_LIMIT)
    {
      fraction_ -= static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT);
      seconds_ += 1;
    }
  }
  else
  {
    while (fraction_ < 0)
    {
      fraction_ += static_cast<int>(INPUT_CONV_FACTOR_PREC_LIMIT);
      seconds_ -= 1;
    }
  }
}

//------------------------------------------------------------------------

void Seconds::convert_(double dtime)
{
  // maximum storage value of class is INT_MAX
  if (dtime >= std::numeric_limits<int>::max())
  {
    seconds_ = std::numeric_limits<int>::max();
    fraction_ = 0;
    return;
  }
  // minimum storage value of class is INT_MIN
  if (dtime <= std::numeric_limits<int>::min())
  {
    seconds_ = std::numeric_limits<int>::min();
    fraction_ =  0;
    return;
  }

  seconds_ = static_cast<int>(dtime);
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

std::ostream& Seconds::operator << (std::ostream& out) const
{
  out << Double();
  return out;
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

TimeStamp::TimeStamp(int refYear, Seconds secondsSinceRefYear)
{
  // While this class could conceivably handle times before 1900, this is asserted on
  // here to avoid common errors where the order of operands is reversed.
  assert(refYear >= 1900);
  setTime(refYear, secondsSinceRefYear);

  // In an attempt to catch parameter reversal problems, we are also checking the
  // actual year of the date.  It should generally be between 1900 and 16384.  This is
  // of course a soft limit, but this code will likely not exist past 16384.
  assert(referenceYear_ >= 1900 && (referenceYear_ <= 16384));
}

//------------------------------------------------------------------------

void TimeStamp::fix_()
{
  double dSecPerDay = SECPERDAY;
  double secondsPerYear = dSecPerDay * daysPerYear(referenceYear_);
  secondsSinceRefYear_.fix_();

  while (secondsSinceRefYear_ >= (Seconds)secondsPerYear)
  {
    secondsSinceRefYear_ -= secondsPerYear;
    secondsPerYear = dSecPerDay * daysPerYear(++referenceYear_);
  }

  while (secondsSinceRefYear_ < ZERO_SECONDS)
  {
    secondsPerYear = dSecPerDay * daysPerYear(--referenceYear_);
    secondsSinceRefYear_ += secondsPerYear;
  }
}

//------------------------------------------------------------------------

Seconds TimeStamp::secondsSinceRefYear(int refYear) const
{
  TimeStamp ref(refYear, 0);
  return *this - ref;
}

//------------------------------------------------------------------------

void TimeStamp::setTime(int refYear, Seconds secondsSinceRefYear)
{
  referenceYear_ = refYear;
  secondsSinceRefYear_ = secondsSinceRefYear;
  fix_();
}

//------------------------------------------------------------------------

size_t TimeStamp::sizeOf() const
{
  return sizeof(referenceYear_) + sizeof(secondsSinceRefYear_.seconds_) + sizeof(secondsSinceRefYear_.fraction_);
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
  int yearDifference = (referenceYear_ - t.referenceYear_);
  int year = 0;
  Seconds secondsValue;
  double dSecPerDay = SECPERDAY;

  if (yearDifference > 0)
  {
    secondsValue = ((Seconds)(dSecPerDay * daysPerYear(t.referenceYear_))) - t.secondsSinceRefYear_;
    for (year = t.referenceYear_ + 1; year < referenceYear_; ++year)
      secondsValue += dSecPerDay * daysPerYear(year);
    secondsValue += secondsSinceRefYear_;
  }
  else if (yearDifference < 0)
  {
    secondsValue = (Seconds)(-1.0) * (t.secondsSinceRefYear_);
    for (year = t.referenceYear_ - 1; year > referenceYear_; --year)
      secondsValue -= dSecPerDay * daysPerYear(year);
    secondsValue -= (Seconds)(dSecPerDay * daysPerYear(referenceYear_)) - secondsSinceRefYear_;
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
  TimeStamp tempTime(referenceYear_, secondsSinceRefYear_ - s);
  tempTime.fix_();
  return tempTime;
}

//------------------------------------------------------------------------

TimeStamp TimeStamp::operator + (const Seconds& s) const
{
  TimeStamp tempTime(referenceYear_, secondsSinceRefYear_ + s);
  tempTime.fix_();
  return tempTime;
}

//------------------------------------------------------------------------

TimeCompVal TimeStamp::compare_(const TimeStamp& time) const
{
  if (referenceYear_ > time.referenceYear_) return TCV_GREATER;
  if (referenceYear_ < time.referenceYear_) return TCV_LESS;
  if (simCore::areEqual(secondsSinceRefYear_, time.secondsSinceRefYear_, 1e-7))
    return TCV_EQUAL;
  return (secondsSinceRefYear_ > time.secondsSinceRefYear_) ? TCV_GREATER : TCV_LESS;
}

//------------------------------------------------------------------------

double simCore::getFactor(const TimeStamp &lowVal, const TimeStamp &exactVal, const TimeStamp &highVal)
{
  // Perform bounds check and prevent divide by zero
  if (exactVal <= lowVal) return 0.;
  if (exactVal >= highVal || (highVal - lowVal) == 0) return 1.;
  return (exactVal - lowVal) / (highVal - lowVal);
}
