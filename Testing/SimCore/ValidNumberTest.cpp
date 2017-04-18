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
#include <string>
#include <sstream>
#include <iostream>
#include <limits>
#include <typeinfo>
#include "simCore/Common/Common.h"
#include "simCore/Common/SDKAssert.h"
#include "simCore/String/ValidNumber.h"

namespace
{

/** Returns true when testString conversion to type T worked as expectedValid dictates. */
template <typename T>
bool validateValueNoCompare(const std::string& testString, bool expectedValid, T* convertedValue=NULL, bool allowPlusSign=true)
{
  bool rv = true; // Return value defaults to good

  T val;
  bool wasValid = simCore::isValidNumber(testString, val, allowPlusSign);

  // The docs state that isValidNumber() sets the parameter to 0 on failure
  if (!wasValid && val != 0)
  {
    std::cerr << "isValidNumber<" << typeid(T).name() << "> failure did not set parameter to 0 with input: " << testString << std::endl;
    rv = false; // failure based on not setting return value properly
  }

  // Make sure that we matched on expectation vs reality
  if (wasValid != expectedValid)
  {
    std::cerr << "isValidNumber<" << typeid(T).name() << "> failed with input: " << testString << std::endl;
    rv = false;
  }
  if (convertedValue != NULL)
    *convertedValue = val;
  return rv;
}

/** Returns true when testString conversion to type T worked as expectedValid dictates, with conversion check. */
template <typename T>
bool validateValue(const std::string& testString, bool expectedValid, const T& conversion = 0, bool allowPlusSign = true)
{
  // Make sure we could convert it properly
  T val;
  bool rv = validateValueNoCompare<T>(testString, expectedValid, &val, allowPlusSign);
  if (rv)
  {
    if (conversion != val)
    {
      std::cerr << "isValidNumber<" << typeid(T).name() << ">(" << testString << ") did not convert to expected value (" << val << " != " << conversion << ")" << std::endl;
      rv = false;
    }
  }
  return rv;
}

// Test for valid numbers
int testValidNumber()
{
  int rv = 0;

  //-------------------------------------------------------------
  // 32 bit integer testing
  rv += SDK_ASSERT(validateValue<int32_t>("15", true, 15));
  rv += SDK_ASSERT(validateValue<int32_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<int32_t>("2147483647", true, std::numeric_limits<int>::max()));  // MAX_INT
  rv += SDK_ASSERT(validateValue<int32_t>("2147483648", false));
  rv += SDK_ASSERT(validateValue<int32_t>("4294967295", false));
  rv += SDK_ASSERT(validateValue<int32_t>("4294967296", false));
  rv += SDK_ASSERT(validateValue<int32_t>("4294967297", false));
  rv += SDK_ASSERT(validateValue<int32_t>("-2147483648", true, std::numeric_limits<int>::min()));
  rv += SDK_ASSERT(validateValue<int32_t>("-2147483649", false));
  rv += SDK_ASSERT(validateValue<int32_t>("-5", true, -5));
  rv += SDK_ASSERT(validateValue<int32_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<int32_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<int32_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<int32_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<int32_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<int32_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<int32_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<int32_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<int32_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<int32_t>("", false));
  rv += SDK_ASSERT(validateValue<int32_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<int32_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<int32_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<int32_t>("   ", false));

  //-------------------------------------------------------------
  // 32 bit unsigned integer testing
  rv += SDK_ASSERT(validateValue<uint32_t>("4294967295", true, std::numeric_limits<uint32_t>::max())); // MAX_UINT
  rv += SDK_ASSERT(validateValue<uint32_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<uint32_t>("4294967296", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("-2147483648", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("-5", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<uint32_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<uint32_t>("   ", false));

  //-------------------------------------------------------------
  // 64 bit integer testing
  rv += SDK_ASSERT(validateValue<int64_t>("9223372036854775807", true, std::numeric_limits<int64_t>::max()));
  rv += SDK_ASSERT(validateValue<int64_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<int64_t>("9223372036854775808", false));
  rv += SDK_ASSERT(validateValue<int64_t>("-9223372036854775808", true, std::numeric_limits<int64_t>::min()));
  rv += SDK_ASSERT(validateValue<int64_t>("-9223372036854775809", false));
  rv += SDK_ASSERT(validateValue<int64_t>("-5", true, -5));
  rv += SDK_ASSERT(validateValue<int64_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<int64_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<int64_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<int64_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<int64_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<int64_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<int64_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<int64_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<int64_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<int64_t>("", false));
  rv += SDK_ASSERT(validateValue<int64_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<int64_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<int64_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<int64_t>("   ", false));

  //-------------------------------------------------------------
  // 64 bit unsigned integer testing
  rv += SDK_ASSERT(validateValue<uint64_t>("18446744073709551615", true, std::numeric_limits<uint64_t>::max()));
  rv += SDK_ASSERT(validateValue<uint64_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<uint64_t>("18446744073709551616", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("-2147483648", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("-5", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<uint64_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<uint64_t>("   ", false));

  //-------------------------------------------------------------
  // 16 bit integer testing
  rv += SDK_ASSERT(validateValue<int16_t>("15", true, 15));
  rv += SDK_ASSERT(validateValue<int16_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<int16_t>("32767", true, std::numeric_limits<int16_t>::max()));  // MAX_SHORT
  rv += SDK_ASSERT(validateValue<int16_t>("32768", false));
  rv += SDK_ASSERT(validateValue<int16_t>("-32768", true, std::numeric_limits<int16_t>::min()));
  rv += SDK_ASSERT(validateValue<int16_t>("-32769", false));
  rv += SDK_ASSERT(validateValue<int16_t>("-5", true, -5));
  rv += SDK_ASSERT(validateValue<int16_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<int16_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<int16_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<int16_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<int16_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<int16_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<int16_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<int16_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<int16_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<int16_t>("", false));
  rv += SDK_ASSERT(validateValue<int16_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<int16_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<int16_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<int16_t>("   ", false));

  //-------------------------------------------------------------
  // 16 bit unsigned integer testing
  rv += SDK_ASSERT(validateValue<uint16_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<uint16_t>("65535", true, std::numeric_limits<uint16_t>::max()));
  rv += SDK_ASSERT(validateValue<uint16_t>("65536", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("0", true, 0));
  rv += SDK_ASSERT(validateValue<uint16_t>("-1", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<uint16_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<uint16_t>("   ", false));

  //-------------------------------------------------------------
  // byte-wide integer testing: signed 8 bit integer
  rv += SDK_ASSERT(validateValue<int8_t>("15", true, 15));
  rv += SDK_ASSERT(validateValue<int8_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<int8_t>("127", true, 127));
  rv += SDK_ASSERT(validateValue<int8_t>("128", false));
  rv += SDK_ASSERT(validateValue<int8_t>("-128", true, -128));
  rv += SDK_ASSERT(validateValue<int8_t>("-129", false));
  rv += SDK_ASSERT(validateValue<int8_t>("2147483647", false));
  rv += SDK_ASSERT(validateValue<int8_t>("2147483648", false));
  rv += SDK_ASSERT(validateValue<int8_t>("-2147483648", false));
  rv += SDK_ASSERT(validateValue<int8_t>("-2147483649", false));
  rv += SDK_ASSERT(validateValue<int8_t>("-5", true, -5));
  rv += SDK_ASSERT(validateValue<int8_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<int8_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<int8_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<int8_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<int8_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<int8_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<int8_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<int8_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<int8_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<int8_t>("", false));
  rv += SDK_ASSERT(validateValue<int8_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<int8_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<int8_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<int8_t>("   ", false));

  //-------------------------------------------------------------
  // byte-wide integer testing: unsigned 8 bit integer
  rv += SDK_ASSERT(validateValue<uint8_t>("15", true, 15));
  rv += SDK_ASSERT(validateValue<uint8_t>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<uint8_t>("255", true, 255));
  rv += SDK_ASSERT(validateValue<uint8_t>("256", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("0", true, 0));
  rv += SDK_ASSERT(validateValue<uint8_t>("-1", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("2147483647", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("2147483648", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("-2147483648", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("-2147483649", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("-5", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("3e1", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("3e-1", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("3.222e+10", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("ho ho", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("0xffww", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("0xFF", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("1 1", false));
  rv += SDK_ASSERT(validateValue<uint8_t>(" 11", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("11 ", false));
  rv += SDK_ASSERT(validateValue<uint8_t>("   ", false));

  // Make some strings for testing in float/double below
  std::stringstream dblMax;
  dblMax << std::numeric_limits<double>::max();
  std::stringstream dblMin;
  dblMin << std::numeric_limits<double>::min();
  std::stringstream fltMax;
  fltMax << std::numeric_limits<float>::max();
  std::stringstream fltMin;
  fltMin << std::numeric_limits<float>::min();

  //-------------------------------------------------------------
  // single precision float testing
  rv += SDK_ASSERT(validateValue<float>("8", true, 8.f));
  rv += SDK_ASSERT(validateValue<float>("+8", true, 8.f));
  rv += SDK_ASSERT(validateValue<float>("-8", true, -8.f));
  rv += SDK_ASSERT(validateValue<float>(".6", true, 0.6f));
  rv += SDK_ASSERT(validateValue<float>("+.6", true, 0.6f));
  rv += SDK_ASSERT(validateValue<float>("-.6", true, -0.6f));
  rv += SDK_ASSERT(validateValue<float>("3.402823466e+38", true, std::numeric_limits<float>::max())); // FLT_MAX
  rv += SDK_ASSERT(validateValue<float>(dblMax.str(), false));
  rv += SDK_ASSERT(validateValue<float>(dblMin.str(), true, 0.f));
  rv += SDK_ASSERT(validateValue<float>("1.7976931348623158e+400", false));
  rv += SDK_ASSERT(validateValue<float>("2.2250738585072014e-400", true, 0.f));
  rv += SDK_ASSERT(validateValueNoCompare<float>(fltMax.str(), true));
  rv += SDK_ASSERT(validateValueNoCompare<float>(fltMin.str(), true));
  rv += SDK_ASSERT(validateValue<float>("ho ho", false));
  rv += SDK_ASSERT(validateValue<float>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<float>("0xffww", false));
  rv += SDK_ASSERT(validateValue<float>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<float>("0xFF", false));
  rv += SDK_ASSERT(validateValue<float>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<float>("", false));
  rv += SDK_ASSERT(validateValue<float>("1 1", false));
  rv += SDK_ASSERT(validateValue<float>(" 11", false));
  rv += SDK_ASSERT(validateValue<float>("11 ", false));
  rv += SDK_ASSERT(validateValue<float>("   ", false));
  // Reports of visual studio not converting 0.5 correctly so add some tests
  rv += SDK_ASSERT(validateValue<float>("0.5", true, 0.5));
  rv += SDK_ASSERT(validateValue<float>("-0.5", true, -0.5));
  rv += SDK_ASSERT(validateValue<float>("0.50", true, 0.5));
  rv += SDK_ASSERT(validateValue<float>("-0.50", true, -0.5));

  //-------------------------------------------------------------
  // double precision float testing
  // DBL MAX
  rv += SDK_ASSERT(validateValue<double>("8", true, 8));
  rv += SDK_ASSERT(validateValue<double>("+8", true, 8));
  rv += SDK_ASSERT(validateValue<double>("-8", true, -8));
  rv += SDK_ASSERT(validateValue<double>(".6", true, 0.6));
  rv += SDK_ASSERT(validateValue<double>("+.6", true, 0.6));
  rv += SDK_ASSERT(validateValue<double>("-.6", true, -0.6));
  rv += SDK_ASSERT(validateValue<double>("1.7976931348623158e+308", true, std::numeric_limits<double>::max())); // DBL_MAX
  rv += SDK_ASSERT(validateValue<double>("2.2250738585072014e-308", true, std::numeric_limits<double>::min())); // DBL_MIN
  rv += SDK_ASSERT(validateValueNoCompare<double>(dblMax.str(), true));
  rv += SDK_ASSERT(validateValueNoCompare<double>(dblMin.str(), true));
  rv += SDK_ASSERT(validateValue<double>("1.7976931348623158e+400", false));
  rv += SDK_ASSERT(validateValueNoCompare<double>("2.2250738585072014e-400", true));
  rv += SDK_ASSERT(validateValueNoCompare<double>(fltMax.str(), true));
  rv += SDK_ASSERT(validateValueNoCompare<double>(fltMin.str(), true));
  rv += SDK_ASSERT(validateValue<double>("ho ho", false));
  rv += SDK_ASSERT(validateValue<double>("1234567890a", false));
  rv += SDK_ASSERT(validateValue<double>("0xffww", false));
  rv += SDK_ASSERT(validateValue<double>("#%^&*", false));
  rv += SDK_ASSERT(validateValue<double>("0xFF", false));
  rv += SDK_ASSERT(validateValue<double>("0x01ffee07", false));
  rv += SDK_ASSERT(validateValue<double>("", false));
  rv += SDK_ASSERT(validateValue<double>("1 1", false));
  rv += SDK_ASSERT(validateValue<double>(" 11", false));
  rv += SDK_ASSERT(validateValue<double>("11 ", false));
  rv += SDK_ASSERT(validateValue<double>("   ", false));
  // Reports of visual studio not converting 0.5 correctly so add some tests
  rv += SDK_ASSERT(validateValue<double>("0.5", true, 0.5));
  rv += SDK_ASSERT(validateValue<double>("-0.5", true, -0.5));
  rv += SDK_ASSERT(validateValue<double>("0.50", true, 0.5));
  rv += SDK_ASSERT(validateValue<double>("-0.50", true, -0.5));
  return rv;
}

int testPermitPlus()
{
  int rv = 0;
  {
    uint8_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<uint8_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, false));
  }
  {
    uint16_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<uint16_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, false));
  }
  {
    uint32_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<uint32_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidNumber("0", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidNumber("1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber(" 1 ", val));
    rv += SDK_ASSERT(simCore::isValidNumber("4294967295", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidNumber(" ", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("0xFF", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.9.9", val));
  }
  {
    uint64_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<uint64_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("-8", val, false));
    rv += SDK_ASSERT(!simCore::isValidNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidNumber("0", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidNumber("1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber(" 1 ", val));
    rv += SDK_ASSERT(simCore::isValidNumber("4294967295", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.abcd", val));
    rv += SDK_ASSERT(simCore::isValidNumber("4294967296", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidNumber(" ", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("0xFF", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1,1", val));
    rv += SDK_ASSERT(simCore::isValidNumber("18446744073709551615", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("18446744073709551616", val));
    rv += SDK_ASSERT(!simCore::isValidNumber("1.9.9", val));
  }
  {
    int8_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<int8_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  {
    int16_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<int16_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  {
    int32_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<int32_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  {
    int64_t val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<int64_t>("+8", false, 0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  {
    float val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<float>("+8", false, 0.0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  {
    double val;
    rv += SDK_ASSERT(simCore::isValidNumber("+8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, true));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, true));
    // validateValue() is more stringent and verifies value set to 0 on failure
    rv += SDK_ASSERT(validateValue<double>("+8", false, 0.0, false));
    rv += SDK_ASSERT(simCore::isValidNumber("8", val, false));
    rv += SDK_ASSERT(simCore::isValidNumber("-8", val, false));
  }
  return rv;
}

int testValidHexNumber()
{
  int rv = 0;
  {
    // Test a variety of values similar to what uint32_t accepts
    uint32_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("4294967294", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("99999999", val) && val == 0x99999999); // Well inside range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("100000000", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xFF", val) && val == 0xff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
  }

  // Test variety of values that include hex numbers
  {
    uint32_t val;
    rv += SDK_ASSERT(simCore::isValidHexNumber("1abcd", val) && val == 0x1abcd);
    rv += SDK_ASSERT(simCore::isValidHexNumber("1aBCd", val) && val == 0x1abcd);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1aAbCd", val) && val == 0x1aabcd);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0X1aAbCd", val) && val == 0x1aabcd);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("x1aAbCd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("00x1aAbCd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xx1aAbCd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0y1aAbCd", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x0", val) && val == 0);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x00", val) && val == 0);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x0000000002", val) && val == 2);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xabcdef", val) && val == 0xabcdef);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xABCDEF", val) && val == 0xabcdef);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xaBcDeF", val) && val == 0xabcdef);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xAbCdEf", val) && val == 0xabcdef);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xabcdef00", val) && val == 0xabcdef00);
    rv += SDK_ASSERT(simCore::isValidHexNumber("abcdef00", val) && val == 0xabcdef00);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xabcdef000", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("abcdef000", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("ffffffff", val) && val == 0xffffffff);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xffffffff", val) && val == 0xffffffff);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x020", val) && val == 0x20);
    rv += SDK_ASSERT(simCore::isValidHexNumber("020", val) && val == 0x20);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  // uint16_t testing
  {
    uint16_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("65535", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("9999", val) && val == 0x9999); // Well inside range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("10000", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xFF", val) && val == 0xff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
    rv += SDK_ASSERT(simCore::isValidHexNumber("ffff", val) && val == 0xffff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  // uint8_t testing
  {
    uint8_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("255", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("99", val) && val == 0x99); // Well inside range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("100", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xF", val) && val == 0xf);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
    rv += SDK_ASSERT(simCore::isValidHexNumber("ff", val) && val == 0xff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  // int32_t testing
  {
    int32_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("2147483647", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("79999999", val) && val == 0x79999999); // Inside range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("100000000", val)); // Out of range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("80000000", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xFF", val) && val == 0xff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
    rv += SDK_ASSERT(simCore::isValidHexNumber("7fffffff", val) && val == 0x7fffffff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  // int16_t testing
  {
    int16_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("32767", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("7999", val) && val == 0x7999); // Inside range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("10000", val)); // Out of range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("8000", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xFF", val) && val == 0xff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
    rv += SDK_ASSERT(simCore::isValidHexNumber("7fff", val) && val == 0x7fff);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  // int8_t testing
  {
    int8_t val;
    rv += SDK_ASSERT(!simCore::isValidHexNumber("+8", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("8", val) && val == 0x8);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-8", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0", val) && val == 0x0);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.0", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("1", val) && val == 0x1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("10", val) && val == 0x10); // Note -- 16 decimal
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" 1 ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("127", val)); // Out of range when interpreted as hex
    rv += SDK_ASSERT(simCore::isValidHexNumber("77", val) && val == 0x77); // Inside range range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("100", val)); // Out of range
    rv += SDK_ASSERT(!simCore::isValidHexNumber("80", val)); // Out of range by 1
    rv += SDK_ASSERT(!simCore::isValidHexNumber("", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1.0", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("-1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.1.1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.abcd", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("Junk", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("\"20", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("20\"", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber(" ", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0xF", val) && val == 0xf);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1,1", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1.9.9", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("050", val) && val == 0x50);
    rv += SDK_ASSERT(simCore::isValidHexNumber("7f", val) && val == 0x7f);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0x ", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0xg", val));
    rv += SDK_ASSERT(simCore::isValidHexNumber("0x1", val, true) && val == 1);
    rv += SDK_ASSERT(simCore::isValidHexNumber("0X1", val, true) && val == 1);
    rv += SDK_ASSERT(!simCore::isValidHexNumber("0XX1", val, true));
    rv += SDK_ASSERT(!simCore::isValidHexNumber("1", val, true));
  }

  return rv;
}

int testTrueToken()
{
  int rv = 0;
  rv += SDK_ASSERT(simCore::stringIsTrueToken("1"));
  rv += SDK_ASSERT(simCore::stringIsTrueToken("true"));
  rv += SDK_ASSERT(simCore::stringIsTrueToken("True"));
  rv += SDK_ASSERT(simCore::stringIsTrueToken("tRUe"));
  rv += SDK_ASSERT(simCore::stringIsTrueToken("on"));
  rv += SDK_ASSERT(simCore::stringIsTrueToken("yes"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken(" 1"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken(" 1 "));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("1 "));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("true "));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken(" true"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("\"true\""));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("'true'"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("true1"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("0"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("false"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("off"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("No"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("NO"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken("Junk"));
  rv += SDK_ASSERT(!simCore::stringIsTrueToken(""));
  return rv;
}

}

int ValidNumberTest(int argc, char* argv[])
{
  int rv = 0;
  rv += SDK_ASSERT(testValidNumber() == 0);
  rv += SDK_ASSERT(testPermitPlus() == 0);
  rv += SDK_ASSERT(testValidHexNumber() == 0);
  rv += SDK_ASSERT(testTrueToken() == 0);
  return rv;
}
