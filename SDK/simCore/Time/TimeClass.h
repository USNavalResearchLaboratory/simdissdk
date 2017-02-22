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
#ifndef SIMCORE_TIME_TIMECLASS_H
#define SIMCORE_TIME_TIMECLASS_H

#include <iostream>

#include "simCore/Common/Common.h"

namespace simCore
{
  static const double INPUT_CONV_FACTOR_PREC_LIMIT = 1e+09; /**< Conversion factor for incoming fraction of seconds and maximum precision limit */
  static const double OUTPUT_CONV_FACTOR = 1e-09;           /**< Conversion factor for outgoing fraction of seconds */
  static const double INPUT_ROUND_UP_VALUE = 5e-10;         /**< Round up value for incoming fraction of seconds */

  //------------------------------------------------------------------------

  ///Time comparison values
  enum TimeCompVal
  {
    TCV_LESS,
    TCV_EQUAL,
    TCV_GREATER
  };

  //------------------------------------------------------------------------

  /**
   * @brief Utility class for storing and managing decimal-based fixed-point seconds for elapsed (relative or delta) time
   *
   * Seconds is a representation of relative, elapsed, or delta time.  It stores a value of seconds
   * using two integer values (one for whole number, one for the fractional part), which gives it
   * much better accuracy and faster comparisons, compared to a traditional double or floating point
   * representation.
   *
   * To convert Seconds from a relative time to an absolute or fixed time, a reference point ("epoch")
   * needs to be applied.  In SIMDIS, this is typically a reference year, and often is the scenario's
   * reference year.
   */
  class SDKCORE_EXPORT Seconds
  {
    friend class TimeStamp;

  public:

    /// Default constructor
    Seconds() : seconds_(0), fraction_(0) {}

    /// Seconds constructor with a floating point time value as an argument
    /**
    * This creates an instance of the Seconds object. When
    * calling the constructor you may provide an initial time
    * value as a double.
    * @param[in ] time Initial seconds value
    */
    Seconds(double time) {convert_(time);}

    /// Seconds constructor with seconds and fraction as arguments
    /**
    * This creates an instance of the Seconds object. When
    * calling the constructor you may provide initial seconds an
    * nanoseconds values.
    * @param[in ] sec Initial seconds value
    * @param[in ] frac Initial fraction value of seconds in nanoseconds
    */
    Seconds(int sec, int frac) : seconds_(sec), fraction_(frac) {fix_();}

    /// Seconds constructor with seconds and fraction as arguments
    /**
    * This creates an instance of the Seconds object. When
    * calling the constructor you may provide initial seconds an
    * nanoseconds values.
    * @param[in ] sec Initial seconds value
    * @param[in ] frac Initial fraction value of seconds
    */
    Seconds(int sec, double frac) {convert_(frac); seconds_ += sec;}

    /// Copy constructor
    Seconds(const Seconds& time) : seconds_(time.seconds_), fraction_(time.fraction_) {}

    /**
     * Returns a new Seconds value, rounded to the precision requested.  For example, 4.5 rounded
     * to 0 precision will return 5.0.  4.58 rounded to a precision of 1 return 4.6.
     * @param toPrecision Precision which to round the Seconds value.
     */
    Seconds rounded(unsigned short toPrecision) const;

    /// Returns the saved seconds
    /** @return The saved seconds of this time  */
    int getSeconds() const { return seconds_; }

    /// Returns the floating point value of the saved fraction (nanoseconds)
    /** @return the floating point value of the  saved fraction (nanoseconds) of this time  */
    double getFraction() const {return (fraction_ * OUTPUT_CONV_FACTOR);}

    /// Returns the saved fraction in nanoseconds
    /** @return the saved fraction in nanoseconds of this time  */
    int getFractionLong() const {return fraction_;}

    /// Returns the saved time value as a double
    /** @return the time value of seconds as a double  */
    operator double() const {return convert_();}

    // method must be capitalized, otherwise its an illegal type cast
    /// Returns the saved time value as a double
    /** @return the time value of seconds as a double  */
    double Double() const {return convert_();}

    /// Assignment
    /** @param[in ] time Seconds class to be assigned  */
    /** @return the assigned Seconds  */
    Seconds& operator =(const Seconds& time);

    /// Assignment
    /** @param[in ] time Floating point time value to be assigned  */
    /** @return the assigned Seconds  */
    Seconds& operator =(double time) {convert_(time); return *this;}

    /// Increment
    /** @return the updated result of the current Seconds plus one second */
    Seconds& operator ++() {seconds_ += 1; return *this;}

    /// Decrement
    /** @return the updated result of the current Seconds minus one second */
    Seconds& operator --() {seconds_ -= 1; return *this;}

    /// Addition
    /** @param[in ] t Seconds class to be added to existing value  */
    /** @return The updated result of the addition of the Seconds */
    Seconds& operator +=(const Seconds& t) {*this = *this + t; return *this;}

    /// Subtraction
    /** @param[in ] t Seconds class to be subtracted to existing value  */
    /** @return The updated result of the subtraction of the Seconds */
    Seconds& operator -=(const Seconds& t) {*this = *this - t; return *this;}

    /// Multiplication
    /** @param[in ] t Seconds class to be multiplied to existing value  */
    /** @return The updated result of the multiplication of the Seconds */
    Seconds& operator *=(const Seconds& t) {*this = *this * t; return *this;}

    /// Division
    /** @param[in ] t Seconds class to be divided to existing value  */
    /** @return The updated result of the division of the Seconds */
    Seconds& operator /=(const Seconds& t) {*this = *this / t; return *this;}

    /// Scale
    /** @param[in ] scl Seconds class to be scaled (multiplied) to existing value  */
    /** @return The scaled result of the Seconds */
    Seconds& scale(double scl) {*this *= (Seconds)scl; return *this;}

    /// Add
    /** @param[in ] t Seconds class to be added to existing value  */
    /** @return The updated result of the addition */
    Seconds operator +(const Seconds& t) const;

    /// Subtract
    /** @param[in ] t Seconds class to be subtracted from existing value  */
    /** @return The updated result of the subtraction */
    Seconds operator -(const Seconds& t) const;

    /// Multiply
    /** @param[in ] t Seconds class to be multiplied to existing value  */
    /** @return The updated result of the multiplication */
    Seconds operator *(const Seconds& t) const;

    /// Divide
    /** @param[in ] t Seconds class to be divided from existing value  */
    /** @return The updated result of the division */
    Seconds operator /(const Seconds& t) const;

    /// Output a Seconds class to the ostream.
    /**
    * @param[in ] out ostream
    * @return Seconds value displayed to the ostream
    */
    std::ostream& operator <<(std::ostream& out) const;

  protected:
    int seconds_;   /**< Whole second representation  */
    int fraction_;  /**< Fraction of second, nanosecond precision */

    /// Converts incoming double time value to seconds and nanoseconds
    /**
    * @param[in ] time Time value to be converted
    */
    void convert_(double time);

    /// Converts seconds and nanoseconds to a floating point value
    double convert_() const;

    /// Verifies the precision and sign of stored time values
    void fix_();

    /// Boolean comparison to another Seconds class.
    /**
    * @param[in ] time Seconds
    * @return The boolean comparison to the two Seconds classes
    */
    TimeCompVal compare_(const Seconds& time) const
    {
      if (seconds_ > time.seconds_) return TCV_GREATER;
      if (seconds_ < time.seconds_) return TCV_LESS;
      if (fraction_ > time.fraction_) return TCV_GREATER;
      if (fraction_ < time.fraction_) return TCV_LESS;
      return TCV_EQUAL;
    }
  };

  //------------------------------------------------------------------------

  /**
   * @brief Utility class for storing and managing absolute time values
   *
   * simCore::TimeStamp is an absolute (or fixed) time.  It is not relative to anything except the
   * real-world calendar.
   *
   * There are two types of time that SIMDIS deals with: absolute time, and relative time.  They could also
   * be referred to as absolute time and elapsed time, or absolute time and delta time.  The simCore::Seconds
   * class is a representation of the relative time, and simCore::TimeStamp is a representation of fixed or
   * absolute time.
   *
   * Absolute time is a specific point in time -- for example, 11/27/2013 at 8:06AM EDT.  Some language
   * frameworks call absolute time a Date or a Date/Time, or some similar variant.
   *
   * Relative time requires two pieces of data to turn into an absolute time.  It requires an absolute time
   * for the base of the offset, and itself.  A relative time is usually stored as a double (or using
   * simCore::Seconds).  For example, consider "2.0".  If the reference year is 2013, then "2.0" resolves
   * "1/1/13 at 12:00:02 AM".  But if the reference year is 1970, then it resolves to "1/1/1970 12:00:02 AM".
   *
   * simCore::TimeStamp is the first type.  It refers to a real, actual, absolute, resolved point in time.
   * To do that, it does internally store a relative time and a reference year.  However, those values are
   * ephemeral, in the sense that simCore::TimeStamp internally fixes its relative time to be closer to the
   * reference year when possible (e.g. when the delta is larger than a full year).  So the reference year can
   * change in the fix_() method, and that's okay, because that gives better precision.  For example:
   *
   * There are 31536000 seconds in a non-leap year, and 63072000 seconds in two non-leap years.  So:
   *
   * simCore::TimeStamp(1971, 31536000) == simCore::TimeStamp(1972, 0) == simCore::TimeStamp(1970, 63072000)
   *
   * In any of the above cases, simCore::TimeStamp COULD hold the time as any of the 3 internal formats and it
   * would be the exact same.  The reference year only matters when trying to resolve the Seconds value (seconds
   * since reference year).
   */
  class SDKCORE_EXPORT TimeStamp
  {
  public:

    /// Default constructor
    TimeStamp();

    /// TimeStamp constructor with reference year and time as arguments
    /**
    * This creates an instance of the TimeStamp object. When
    * calling the constructor you may provide initial seconds and
    * nanoseconds values.
    * @param[in ] refYear Initial reference year,  must be >= 1900
    * @param[in ] secs Initial seconds since beginning of reference year
    */
    TimeStamp(int refYear, Seconds secs);

    /// Returns the reference year
    /** @return The saved reference year of this time stamp  */
    int referenceYear() const {return referenceYear_;}

    /// Returns the saved seconds
    /** @return The saved seconds of this time stamp  */
    Seconds secondsSinceRefYear() const {return secondsSinceRefYear_;}

    /// Return the saved seconds, relative to given reference year
    /**
    * Return the saved seconds, relative to given reference year
    * @param[in ] refYear Reference year to compare; must be >= 1900
    * @return Seconds since the given reference year
    */
    Seconds secondsSinceRefYear(int refYear) const;

    /// Updates TimeStamp with reference year and time as arguments
    /**
    * This updates the TimeStamp object.
    * @param[in ] refYear Initial reference year,  must be >= 1900
    * @param[in ] secs Initial seconds since beginning of reference year
    */
    void setTime(int refYear, Seconds secs);

    ///Determine object's data size.
    /**
     * Returns the total size, in bytes, required by the data members of
     * the current object.  Can be used to determine the proper size of
     * buffers for the pack and unpack methods.
     * @return an integer representing the size of the TimeStamp's data.
     */
    size_t sizeOf() const;

    /// Assignment
    /** @param[in ] time TimeStamp class to be assigned  */
    /** @return the assigned TimeStamp  */
    TimeStamp& operator =(const TimeStamp& time);

    /// Increment
    /** @return the updated result of the current TimeStamp plus one second */
    TimeStamp& operator ++() {secondsSinceRefYear_ += 1; fix_(); return *this;}

    /// Decrement
    /** @return the updated result of the current TimeStamp minus one second */
    TimeStamp& operator --() {secondsSinceRefYear_ -= 1; fix_(); return *this;}

    /// Addition
    //** @param[in ] t Seconds class to be added to existing TimeStamp value  */
    //** @return The updated result of the addition to the TimeStamp */
    TimeStamp& operator +=(const Seconds& t) {*this = (*this + t); return *this;}

    /// Subtraction
    //** @param[in ] t Seconds class to be subtracted from the existing TimeStamp value  */
    //** @return The updated result of the subtraction to the TimeStamp */
    TimeStamp& operator -=(const Seconds& t) {*this = (*this - t); return *this;}

   /// Subtract a TimeStamp returning the difference in a Seconds class.
    /**
    * @param[in ] t TimeStamp
    * @return difference in a new Seconds class
    */
    Seconds operator -(const TimeStamp& t) const;

    /// Subtract a TimeStamp from Seconds returning the difference in a TimeStamp class.
    /**
    * @param[in ] s Seconds
    * @return difference in a new TimeStamp class
    */
    TimeStamp operator -(const Seconds& s) const;

    /// Add a Seconds returning the sum in a TimeStamp class.
    /**
    * @param[in ] s Seconds
    * @return sum in a new TimeStamp class
    */
    TimeStamp operator +(const Seconds& s) const;

    /// Equals comparison between a TimeStamp class.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not the two classes are equal
    */
    bool operator ==(const TimeStamp& t) const {return (compare_(t) == TCV_EQUAL);}

    /// Not equals comparison between a TimeStamp class.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not the two classes are not equal
    */
    bool operator !=(const TimeStamp& t) const {return (compare_(t) != TCV_EQUAL);}

    /// Less than comparison between two TimeStamp classes.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not this class is less than the input
    */
    bool operator <(const TimeStamp& t) const {return (compare_(t) == TCV_LESS);}

    /// Greater than comparison between two TimeStamp classes.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not this class is greater than the input
    */
    bool operator >(const TimeStamp& t) const {return (compare_(t) == TCV_GREATER);}

    /// Less than equal to comparison between two TimeStamp classes.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not this class is less than equal to the input
    */
    bool operator <=(const TimeStamp& t) const {return (compare_(t) != TCV_GREATER);}

    /// Greater than equal to comparison between two TimeStamp classes.
    /**
    * @param[in ] t TimeStamp
    * @return Whether or not this class is greater than equal the input
    */
    bool operator >=(const TimeStamp& t) const {return (compare_(t) != TCV_LESS);}

  protected:

    int referenceYear_;            /**< Reference Gregorian calendar year, such as 1970, 2000, etc.  Must be >= 1900 */
    Seconds secondsSinceRefYear_;  /**< Number of seconds relative to reference year */

    /// Verifies the precision and sign of stored time values
    void fix_();

    /// Boolean comparison to another TimeStamp class.
    /**
    * @param[in ] time TimeStamp
    * @return The boolean comparison to the two TimeStamp classes
    */
    TimeCompVal compare_(const TimeStamp& time) const;
  };

  //------------------------------------------------------------------------

  /** Sentinel value for simCore::TimeStamp that represents an infinite time value. */
  static const TimeStamp INFINITE_TIME_STAMP(16384, 0.0);
  /** Sentinel value for simCore::TimeStamp that represents the minimum valid time value. */
  static const TimeStamp MIN_TIME_STAMP(1970, 0.0);
  /** Static value representing zero seconds, shared for performance reasons. */
  static const Seconds ZERO_SECONDS;

  /**
  * Computes a scale factor [0,1] between a set of bounded TimeStamps at the specified value.
  * Note that this overrides getFactor() from simCore/Calc for TimeStamp classes.
  * @param[in ] lowVal Low TimeStamp value
  * @param[in ] exactVal Exact TimeStamp value to determine scale
  * @param[in ] highVal High TimeStamp value
  * @return scale factor between [0,1] where 0 is the low and 1 is the high TimeStamp value
  */
  SDKCORE_EXPORT double getFactor(const TimeStamp &lowVal, const TimeStamp &exactVal, const TimeStamp &highVal);

}

#endif /* SIMCORE_TIME_TIMECLASS_H */
