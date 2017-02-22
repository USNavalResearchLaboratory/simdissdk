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
#include <string>
#include <iostream>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <cerrno>

#ifndef _MSC_VER
// needed for isdigit on non-MSVC systems
#include <ctype.h>
#endif

#include "simCore/String/Utils.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/Calc/Math.h"


namespace simCore
{

/// Returns whether a string contains a real number with strict checking
bool stringIsRealNumber(const std::string& str, bool ignoreWhitespace, bool permitPlusToken)
{
  if (str.empty()) return false;
  bool foundDigit = false;
  const char* p = str.c_str();
  while (ignoreWhitespace && isspace(static_cast<unsigned char>(*p))) p++;
  if (*p=='-' || (permitPlusToken && *p=='+')) p++;
  while (isdigit(static_cast<unsigned char>(*p))) { p++; foundDigit = true; }
  if (*p=='.') p++;
  while (isdigit(static_cast<unsigned char>(*p))) { p++; foundDigit = true; }
  if (*p=='E' || *p=='e')
  {
    p++;
    if (*p=='-' || *p=='+') p++;
    while (isdigit(static_cast<unsigned char>(*p))) { p++; foundDigit = true; }
  }
  while (ignoreWhitespace && isspace(static_cast<unsigned char>(*p))) p++;
  return (*p == '\0') && foundDigit;
}

/// Returns whether a string contains an integer number with strict checking
bool stringIsIntegerNumber(const std::string& str, bool isUnsigned, bool ignoreWhitespace, bool permitPlusToken)
{
  if (str.empty()) return false;
  bool foundDigit = false;
  const char* p = str.c_str();
  while (ignoreWhitespace && isspace(static_cast<unsigned char>(*p))) p++;
  if ((!isUnsigned && *p=='-') || (permitPlusToken && *p=='+')) p++;
  while (isdigit(static_cast<unsigned char>(*p))) {p++; foundDigit = true;}
  while (ignoreWhitespace && isspace(static_cast<unsigned char>(*p))) p++;
  return (*p == '\0') && foundDigit;
}

bool isValidNumber(const std::string& token, uint64_t& val, bool permitSign)
{
  if (!stringIsIntegerNumber(token, true, false, permitSign))
  {
    val = 0;
    return false;
  }

  errno = 0;
#ifdef WIN32
  val = _strtoui64(token.c_str(), NULL, 10);
#else
  val = strtoull(token.c_str(), NULL, 10);
#endif
  if (errno != 0)
  {
    val = 0;
    return false;
  }
  return true;
}

bool isValidNumber(const std::string& token, uint32_t& val, bool permitPlusToken)
{
  const char* start = token.c_str();
  char* end;
  errno = 0;
  unsigned long longVal = strtoul(token.c_str(), &end, 10);

  // Error conditions are: Failed to convert, failed to end on a NULL, or having leading white space or leading minus sign
  if ((errno != 0) || (end == start) || (*end != '\0') || isspace(*start) || (*start == '-'))
  {
    val = 0;
    return false;
  }

  // If plus token is not permitted need to return error if there is one
  if (!permitPlusToken && (*start == '+'))
  {
    val = 0;
    return false;
  }

  if ((longVal >= std::numeric_limits<uint32_t>::min()) &&
    (longVal <= std::numeric_limits<uint32_t>::max()))
  {
    val = static_cast<uint32_t>(longVal);
    return true;
  }

  val = 0;
  return false;
}

bool isValidNumber(const std::string& token, uint16_t& val, bool permitPlusToken)
{
  val = 0;
  // For 8 bit values, use a 32 bit then bounds check
  uint32_t val32;
  if (!isValidNumber(token, val32, permitPlusToken))
    return false;
  // Bounds check
  if (val32 < std::numeric_limits<uint16_t>::min() || val32 > std::numeric_limits<uint16_t>::max())
    return false;
  // Convert
  val = static_cast<uint16_t>(val32);
  return true;
}

bool isValidNumber(const std::string& token, uint8_t& val, bool permitPlusToken)
{
  val = 0; // Default return value to return 0 for error case
  // For 8 bit values, use a 32 bit then bounds check
  uint32_t val32;
  if (!isValidNumber(token, val32, permitPlusToken))
    return false;
  // Bounds check
  if (val32 < std::numeric_limits<uint8_t>::min() || val32 > std::numeric_limits<uint8_t>::max())
    return false;
  // Convert
  val = static_cast<uint8_t>(val32);
  return true;
}

bool isValidNumber(const std::string& token, int64_t& val, bool permitPlusToken)
{
  if (!stringIsIntegerNumber(token, false, false, permitPlusToken))
  {
    val = 0;
    return false;
  }
  errno = 0;
#ifdef WIN32
  val = _atoi64(token.c_str());
#else
  val = strtoll(token.c_str(), NULL, 10);
#endif
  if (errno != 0)
  {
    val = 0;
    return false;
  }
  return true;
}

bool isValidNumber(const std::string& token, int32_t& val, bool permitPlusToken)
{
  const char* start = token.c_str();
  char* end;
  errno = 0;
  long longVal = strtol(token.c_str(), &end, 10);

  // Error conditions are: Failed to convert, failed to end on a NULL, or having leading white space
  if ((errno != 0) || (end == start) || (*end != '\0') || isspace(*start))
  {
    val = 0;
    return false;
  }

  // If plus token is not permitted need to return error if there is one
  if (!permitPlusToken && (*start == '+'))
  {
    val = 0;
    return false;
  }

  if ((longVal >= std::numeric_limits<int32_t>::min()) &&
      (longVal <= std::numeric_limits<int32_t>::max()))
  {
    val = static_cast<int32_t>(longVal);
    return true;
  }

  val = 0;
  return false;
}

bool isValidNumber(const std::string& token, int16_t& val, bool permitPlusToken)
{
  val = 0;
  int32_t val32;
  if (!isValidNumber(token, val32, permitPlusToken))
    return false;
  // Bounds check
  if (val32 < std::numeric_limits<int16_t>::min() || val32 > std::numeric_limits<int16_t>::max())
    return false;
  // Convert
  val = static_cast<int16_t>(val32);
  return true;
}

bool isValidNumber(const std::string& token, int8_t& val, bool permitPlusToken)
{
  val = 0; // Default return value to return 0 for error case
  // For 8 bit values, use a 16 bit then bounds check
  int32_t val32;
  if (!isValidNumber(token, val32, permitPlusToken))
    return false;
  // Bounds check
  if (val32 < std::numeric_limits<int8_t>::min() || val32 > std::numeric_limits<int8_t>::max())
    return false;
  // Convert
  val = static_cast<int8_t>(val32);
  return true;
}

bool isValidNumber(const std::string& token, double& val, bool permitPlusToken)
{
  // This routine does not allow leading or trailing whitespace

  const char* start = token.c_str();
  char* end;
  val = std::strtod(start, &end);

  // Error conditions are: Failed to convert, failed to end on a NULL, or having leading white space
  if ((end == start) || (*end != '\0') || (isspace(*start)))
  {
    val = 0.0;
    return false;
  }

  // Must be finite
  if (!finite(val))
  {
    val = 0.0;
    return false;
  }

  // Some versions of std::strtod process hex numbers which we don't want, so kick out if there is an x or X
  if (token.find_first_of("xX") != std::string::npos)
  {
    val = 0.0;
    return false;
  }

  // If plus token is not permitted need to return error if there is one
  if (!permitPlusToken && (*start == '+'))
  {
    val = 0.0;
    return false;
  }

  return true;
}

bool isValidNumber(const std::string& token, float& val, bool permitPlusToken)
{
  val = 0.f;
  double dVal;
  if (!isValidNumber(token, dVal, permitPlusToken))
    return false;
  // Bounds check
  if (dVal < -std::numeric_limits<float>::max() || dVal > std::numeric_limits<float>::max())
    return false;
  // Convert
  val = static_cast<float>(dVal);
  return true;
}

}
