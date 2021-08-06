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
#ifndef SIMCORE_TIME_STRING_H
#define SIMCORE_TIME_STRING_H

#include <map>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "simCore/Common/Common.h"
#include "simCore/Time/Constants.h"

namespace simCore
{
class TimeStamp;
class Seconds;

/** Interface for a formatter of time; can convert to and from simCore::TimeStamp and std::string. */
class SDKCORE_EXPORT TimeFormatter
{
public:
  virtual ~TimeFormatter() {}

  /**
   * Converts the time stamp to a string.  In cases where applicable, the reference year provides an
   * epoch.  This is useful for time formats that are relative to different epochs, such as the built-
   * in seconds or minutes time format.  The precision changes the number of places after the decimal
   * place in formats that support decimal values.  Trailing 0's fill empty precision values (e.g. 0.500)
   * @param timeStamp Time to print to string.  Should be after the epoch referenceYear.
   * @param referenceYear For formats that rely on resolving to an epoch such as Seconds, Minutes,
   *   and Hours, this is the epoch reference year.  This value may be unused in formats that do not
   *   require an epoch, such as ordinal.
   * @param precision Precision after the decimal place for components in individual formatters that
   *   use decimal output.  Trailing 0's will be included in all output to match the precision requested.
   * @return Time string of the value passed in, in the formatter's text format.  If for some reason the
   *   formatter cannot convert the value, an empty string might be returned.  Errors include passing
   *   in a time stamp that is after the reference year, for formats that rely on the reference year
   *   for an epoch value.
   */
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const = 0;

  /**
   * Returns true if the passed-in time string matches this formatter's style.  This is a check of
   * validity for whether fromString() will be able to successfully convert the time string to a
   * simCore::TimeStamp value.  This check should be as stringent as possible.
   * @param timeString Time string to check whether this formatter can convert
   * @return True, if this converter will be able to convert the value properly.  False if the
   *   formatter knows that the fromString() will fail for this input string.
   */
  virtual bool canConvert(const std::string& timeString) const = 0;

  /**
   * Converts a time string to a time stamp.  This is the inverse of the toString() function.  Some
   * time formats such as Seconds and Minutes require a reference year to properly decode the time
   * string into a simCore::TimeStamp structure.  If the formatter is unable to convert the string,
   * a non-zero error will be set in the return value.
   * @param timeString Time string to convert to a simCore::TimeStamp
   * @param timeStamp Value to fill with the interpreted time.  If there is an error in conversion,
   *   this value is reset to simCore::TimeStamp(1970, 0).
   * @param referenceYear Reference year epoch for time formats that require a reference year.  Formats
   *   such as Minute and Hours require an epoch to convert their relative values to an absolute date
   *   and time in the simCore::TimeStamp.
   * @return 0 on successful conversion, and timeStamp will be filled with the valid time stamp.  Non-zero
   *   on error, and timeStamp will be set to simCore::TimeStamp(1970, 0).
   */
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const = 0;
};

/** Null object pattern formatter. */
class SDKCORE_EXPORT NullTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;
};

/** Formatter for simCore::TimeStamp's TIMEFORMAT_SECONDS */
class SDKCORE_EXPORT SecondsTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a Seconds value to a seconds string for an ostream. */
  static void toStream(std::ostream& os, const simCore::Seconds& seconds, unsigned short precision);
  /** Returns true when the string strictly matches a seconds value < 60 (decimals allowed) */
  static bool isStrictSecondsString(const std::string& str);
};

/** Formatter for simCore::TimeStamp's TIMEFORMAT_MINUTES */
class SDKCORE_EXPORT MinutesTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a Seconds value to a minutes string for an ostream. */
  static void toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision);
  /** Returns true when the string strictly matches a minutes value < 60 (no decimal) and seconds value < 60 (decimals allowed) */
  static bool isStrictMinutesString(const std::string& str);
};

/** Alternate formatter for simCore::TimeStamp's TIMEFORMAT_MINUTES that wraps minutes from [0,59] */
class SDKCORE_EXPORT MinutesWrappedTimeFormatter : public MinutesTimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision = 5) const;
  /** Converts a Seconds value to a minutes string for an ostream. */
  static void toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision);
};

/** Formatter for simCore::TimeStamp's TIMEFORMAT_HOURS */
class SDKCORE_EXPORT HoursTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a Seconds value to an hours string for an ostream. */
  static void toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision);
  /** Converts a Seconds value to an hours string for an ostream, with optional leading zero. */
  static void toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision, bool showLeadingZero);
  /** Converts an hours time string to a seconds value; returns 0 on success, non-zero on error (sets seconds to 0 on error) */
  static int fromString(const std::string& timeString, simCore::Seconds& seconds);
  /**
   * Returns true when the character pointer strictly matches an hours value < 24 (no decimals),
   * a minutes value < 60 (no decimal) and seconds value < 60 (decimals allowed)
   */
  static bool isStrictHoursString(const std::string& str);
};

/** Alternate formatter for simCore::TimeStamp's TIMEFORMAT_HOURS that wraps hours from [0,23] */
class SDKCORE_EXPORT HoursWrappedTimeFormatter : public HoursTimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision = 5) const;
  /** Converts a Seconds value to an hours string for an ostream. */
  static void toStream(std::ostream& os, simCore::Seconds seconds, unsigned short precision);
};

/** Formatter for simCore::TimeStamp's TIMEFORMAT_ORDINAL */
class SDKCORE_EXPORT OrdinalTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a Seconds value to an hours string for an ostream. */
  static void toStream(std::ostream& os, const simCore::TimeStamp& timeStamp, unsigned short precision);

  /**
   * Reads input and sets ordinal value if valid based on the year.  Returns true when
   * the input string is valid and converted, or false if invalid.  Ordinal values are
   * 1 to 3 digits, in the range of 1 to 366 (based on input year).  Leading zeros are
   * permitted.  Examples of valid strings include "1", "01", "001", "365", "080".
   * @param input Input string for ordinal values
   * @param year Year used to dereference the ordinal value
   * @param ordinal Output value for 'input'.  Set to 0 on error.
   * @return True if input is valid and ordinal was set; false if invalid (and ordinal was set to 0)
   */
  static bool isValidOrdinal(const std::string& input, int year, int& ordinal);
};

/** Formatter for simCore::TimeStamp's TIMEFORMAT_MONTHDAY */
class SDKCORE_EXPORT MonthDayTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a month string to a [0-11] month value; returns -1 on erroneous input. */
  static int monthStringToInt(const std::string& monthString);
  /** Converts a month integer value [0,11] to a month string. */
  static std::string monthIntToString(int monthValue);
  /**
   * Reads out the month, month day, and seconds-past-midnight components for a time stamp (returns 0 on success)
   * @param timeStamp Time stamp to convert into month, month day, and time-of-day
   * @param month Month value returned, from [0,11]
   * @param monthDay Month value returned, from [1,31]
   * @param secondsPastMidnight Seconds since midnight value returned, from 0 to 86400
   * @return 0 on success and all fields set; non-zero on error
   */
  static int getMonthComponents(const simCore::TimeStamp& timeStamp, int& month, int& monthDay, simCore::Seconds& secondsPastMidnight);
};

/**
 * Formatter for simCore::TimeStamp's TIMEFORMAT_DTG (Date Time Group).  Prints times in military time,
 * showing day-of-month, hours, minutes, seconds, time zones, month, then year.  For example, April 6 at
 * 14:35:03.01 Zulu in the year 2007 would be formatted as: "061435:03.010 Z Apr07"
 */
class SDKCORE_EXPORT DtgTimeFormatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;
};

/**
 * Formatter for simCore::Time's TIMEFORMAT_ISO8601.
 * Official ISO 8601 format can take many forms YYYY, YYYY-MM, YYYY-MM-DD, YYYY-MM-DDThh:mm:ssZ, YYYY-MM-DDThh:mm:sszzzzzz
 * YYYY-MM-DD, and YYYY-MM-DDThh:mm:ss.sssZ with optional [.sss...] are fully supported.
 * No support for the YYYY-MM-DDThh:mm:sszzzzzz local-time format.
 * Support for the YYYY-MM format is limited.
 * Support for the YYYY format is limited, due to conflict with SecondsFormatter.
 * April 6 at 14:35:03.01 Zulu in the year 2007 would be formatted as: "2007-04-06T14:35:03.010Z"
 * Since the format always includes year, toString's referenceYear arg is always ignored.
 */
class SDKCORE_EXPORT Iso8601TimeFormatter : public TimeFormatter
{
public:
  /**
   * Converts the time stamp to a string.  The reference year arg is always ignored.
   * The precision changes the number of places after the decimal
   * place in formats that support decimal values.  Trailing 0's fill empty precision values (e.g. 0.500)
   *
   * The Iso8601TimeFormatter does not output to the ISO8601 YYYY or YYYY-MM formats.
   * Where the timeStamp specifies a time with 0.0 seconds in the day, the output string will have
   * the YYYY-MM-DD format (regardless of precision).
   * If there is a non-zero value for the seconds in the day, the YYYY-MM-DDThh:mm:ss[.s...]Z format
   * will be output, with specified precision governing the output.
   *
   * @param timeStamp Time to print to string.  Should be after the epoch referenceYear.
   * @param referenceYear  always ignored.
   * @param precision Precision after the decimal place for components in individual formatters that
   *   use decimal output.  Trailing 0's will be included in all output to match the precision requested.
   * @return Time string of the value passed in, in the formatter's text format.  If for some reason the
   *   formatter cannot convert the value, an empty string might be returned.
   */
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=0) const;

  /**
   * Returns true if the passed-in time string matches this formatter's style.  This is a check of
   * validity for whether fromString() will be able to successfully convert the time string to a
   * simCore::TimeStamp value.  This check should be as stringent as possible.
   * Supports ISO8601 YYYY-MM, YYYY-MM-DD, YYYY-MM-DDThh:mm:ssZ, YYYY-MM-DDThh:mm:ss.sZ formats.
   * Returns false for otherwise valid ISO8601 YYYY format, as that would conflict with
   * the SecondsFormatter.
   * Returns false for ISO8601 YYYY-MM-DDThh:mm:sszzzzzz local-time format.
   * @param timeString Time string to check whether this formatter can convert
   * @return True, if this converter will be able to convert the value properly.  False if the
   *   formatter knows that the fromString() will fail for this input string.
   */
  virtual bool canConvert(const std::string& timeString) const;

  /**
   * Converts a time string to a time stamp.  This is the inverse of the toString() function.
   * If the formatter is unable to convert the string,
   * a non-zero error will be set in the return value.
   *
   * Note that fromString can convert from ISO8601 YYYY format, where canConvert returns false.
   * This allows uses where values are known to be ISO8601 formatted to be processed.
   *
   * @param timeString Time string to convert to a simCore::TimeStamp
   * @param timeStamp Value to fill with the interpreted time.  If there is an error in conversion,
   *   this value is reset to simCore::TimeStamp(1970, 0).
   * @param referenceYear Reference year epoch for time formats that require a reference year.
   * @return 0 on successful conversion, and timeStamp will be filled with the valid time stamp.  Non-zero
   *   on error, and timeStamp will be set to simCore::TimeStamp(1970, 0).
   */
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;
};


/**
 * Composite class of several built-in time formats.  Accepts registration of foreign time formatters.
 * During conversion, foreign time formatters are given priority over built-in formatters when multiple
 * formatters are able to parse the incoming time string.
 */
class SDKCORE_EXPORT TimeFormatterRegistry
{
public:
  /**
   * Constructs a time formatter registry and registers all the built-in formatters.
   * By default, the normal formatters are used.  Setting wrappedFormatters true will
   * use the wrapped minutes and hours formatters, keeping them under 60 minutes and
   * 24 hours respectively.  This can be useful for clock displays.
   * @param wrappedFormatters If true, Hours and Minutes formatters will use the wrapped version
   * @param addDefaults If true, add all built-in time formats during construction
   */
  explicit TimeFormatterRegistry(bool wrappedFormatters=false, bool addDefaults=true);
  /** Destroys the registry */
  virtual ~TimeFormatterRegistry();

  /** Typedef the TimeFormatter abstract interface to a shared pointer */
  typedef std::shared_ptr<TimeFormatter> TimeFormatterPtr;

  /**
   * Registers a custom formatter with this registry.  Custom formatters are given priority over
   * built-in formatters in the formatter() and fromString() functions.
   * @param formatter Custom formatter that abides by the simCore::TimeFormatter interface.
   */
  void registerCustomFormatter(TimeFormatterPtr formatter);

  /** Retrieves a reference to the built-in formatter requested. */
  const TimeFormatter& formatter(simCore::TimeFormat format) const;

  /**
   * Retrieves a reference to the formatter that is 'best' able to read the time string passed in.
   * Foreign formatters are given priority over built-in formatters in this function.  If no formatter
   * is able to convert the time string, the NullTimeFormatter is returned.
   * @param timeString Time string that might match a given time format.
   * @return Time formatter that best matches the time string, or a reference to a NullTimeFormatter.
   */
  const TimeFormatter& formatter(const std::string& timeString) const;

  /**
   * Converts the simCore::TimeStamp to the requested built-in format.
   * @param format Format enumeration that matches up with a built-in formatter.
   * @param timeStamp Time to print to string.  Should be after the epoch referenceYear.
   * @param referenceYear For formats that rely on resolving to an epoch such as Seconds, Minutes,
   *   and Hours, this is the epoch reference year.  This value may be unused in formats that do not
   *   require an epoch, such as ordinal.
   * @param precision Precision after the decimal place for components in individual formatters that
   *   use decimal output.  Trailing 0's will be included in all output to match the precision requested.
   * @return Time string of the value passed in, in the formatter's text format.  If for some reason the
   *   formatter cannot convert the value, an empty string might be returned.  Errors include passing
   *   in a time stamp that is after the reference year, for formats that rely on the reference year
   *   for an epoch value.
   */
  std::string toString(simCore::TimeFormat format, const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;

  /**
   * Determines the best matching formatter and uses it to convert a time string to a time stamp.  The
   * formatter() function determines the proper simCore::TimeFormatter to use for parsing the time
   * string.  This is the inverse of the toString() function.  Some time formats such as Seconds and
   * Minutes require a reference year to properly decode the time string into a simCore::TimeStamp
   * structure.  If the formatter is unable to convert the string, a non-zero error will be set in the
   * return value.
   * @param timeString Time string to convert to a simCore::TimeStamp
   * @param timeStamp Value to fill with the interpreted time.  If there is an error in conversion,
   *   this value is reset to simCore::TimeStamp(1970, 0).
   * @param referenceYear Reference year epoch for time formats that require a reference year.  Formats
   *   such as Minute and Hours require an epoch to convert their relative values to an absolute date
   *   and time in the simCore::TimeStamp.
   * @return 0 on successful conversion, and timeStamp will be filled with the valid time stamp.  Non-zero
   *   on error, and timeStamp will be set to simCore::TimeStamp(1970, 0).
   */
  int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Maps built-in formatters by simCore::TimeFormat enumeration */
  std::map<int, TimeFormatterPtr> knownFormatters_;
  /** Vector of all registered formatters from foreign sources */
  std::vector<TimeFormatterPtr> foreignFormatters_;
  /** Saves a NullFormatter for null object pattern use. */
  TimeFormatterPtr nullFormatter_;

  /** Cache the most recent formatter to leverage locality principle in format conversion */
  mutable TimeFormatterPtr lastUsedFormatter_;
};


}

#endif /* SIMCORE_TIME_STRING_H */
