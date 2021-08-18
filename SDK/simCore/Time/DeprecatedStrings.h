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
#ifndef SIMCORE_TIME_DEPRECATEDSTRINGS_H
#define SIMCORE_TIME_DEPRECATEDSTRINGS_H

#include "simCore/Common/Export.h"
#include "simCore/Time/String.h"

namespace simCore { namespace Deprecated {

/** Formatter that matches "DDD HH:MM:SS.sss YYYY" */
class SDKCORE_EXPORT DDD_HHMMSS_YYYY_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& days, simCore::Seconds& seconds, int& year) const;
};

/** Formatter that matches "DDD HH:MM:SS.sss" */
class SDKCORE_EXPORT DDD_HHMMSS_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& days, simCore::Seconds& seconds, int referenceYear) const;
};

/** Formatter that matches "MON MD HH:MM:SS.sss YYYY" */
class SDKCORE_EXPORT MON_MD_HHMMSS_YYYY_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& month, int& monthDay, simCore::Seconds& seconds, int& year) const;
};

/** Formatter that matches "MD MON YYYY HH:MM:SS.sss" */
class SDKCORE_EXPORT MD_MON_YYYY_HHMMSS_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& monthDay, int& month, int& year, simCore::Seconds& seconds) const;
};

/** Formatter that matches "WKD MON MD HH:MM:SS.sss YYYY" */
class SDKCORE_EXPORT WKD_MON_MD_HHMMSS_YYYY_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

  /** Converts a TimeStamp value to an output string for an ostream. */
  static void toStream(std::ostream& os, const simCore::TimeStamp& timeStamp, unsigned short precision);
  /** Gets the weekday string for the given values */
  static std::string weekDayString(const simCore::TimeStamp& timeStamp);
  /** Gets the weekday integer index value, or -1 on error */
  static int weekDayStringToInt(const std::string& weekDay);

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& weekDay, int& month, int& monthDay, simCore::Seconds& seconds, int& year) const;
};

/** Formatter that matches "WKD MON MD HH:MM:SS.sss" */
class SDKCORE_EXPORT WKD_MON_MD_HHMMSS_Formatter : public TimeFormatter
{
public:
  virtual std::string toString(const simCore::TimeStamp& timeStamp, int referenceYear, unsigned short precision=5) const;
  virtual bool canConvert(const std::string& timeString) const;
  virtual int fromString(const std::string& timeString, simCore::TimeStamp& timeStamp, int referenceYear) const;

private:
  /** Retrieves the components of the string, returning 0 on success */
  int getComponents_(const std::string& timeString, int& weekDay, int& month, int& monthDay, simCore::Seconds& seconds, int referenceYear) const;
};

} }

#endif /* SIMCORE_TIME_DEPRECATEDSTRINGS_H */
