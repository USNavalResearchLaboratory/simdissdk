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
#ifndef SIMCORE_STRING_VALIDNUMBER_H
#define SIMCORE_STRING_VALIDNUMBER_H

#include <string>
#include "simCore/Common/Common.h"

namespace simCore
{
  /**
   * Returns whether a string contains a real number with strict checking
   * @param[in ] str String to check
   * @param[in ] ignoreWhitespace If true, spaces are permitted on either side of the value
   * @param[in ] permitPlusToken Permits + sign in front of the string; if false, '+' is an error
   * @return boolean, true if the string has only real numbers
   */
  SDKCORE_EXPORT bool stringIsRealNumber(const std::string& str, bool ignoreWhitespace=true, bool permitPlusToken=true);

  /**
   * Determines whether the string is a valid integer.  A valid integer is a series of digits
   * with an optional + or - sign prefix.  Valid integers do not have exponent ('e') values,
   * decimals, or errant other characters.  This string can also strictly check whitespace.
   * @param[in ] str Potential integer string
   * @param[in ] isUnsigned True if the integer should be unsigned; false if signed.  False values
   *   mean that negatives are supported, and true means that negatives are not supported.
   * @param[in ] ignoreWhitespace If true, spaces are permitted on either side of the value
   * @param[in ] permitPlusToken Permits + sign in front of the string; if false, '+' is an error
   * @return True if the string token is an integer value given the restrictions desired.
   */
  SDKCORE_EXPORT bool stringIsIntegerNumber(const std::string& str, bool isUnsigned=false, bool ignoreWhitespace=true, bool permitPlusToken=true);

  /**
   * Returns true when the passed-in string token is a token that can be interpreted as True.
   * Valid true values include (case insensitive) "true", "1", "yes", "on".  All other values
   * including the empty string are considered false.
   * @param[in ] str Potential boolean token
   * @return True if the string can be interpreted as a True token; false otherwise.
   */
  SDKCORE_EXPORT bool stringIsTrueToken(const std::string& str);

  ///@{
  /**
   * Determines if the incoming string is a valid number and then performs the conversion.
   * Hexadecimal values will fail, as will values outside the bounds of the data type.
   * @param[in ] token String to validate
   * @param[out] val Converted number, set to 0 if conversion fails
   * @param[in ] permitPlusToken Permits positive '+' signs on the string; if false, having '+' is an error
   * @return true if valid, false if not
   */
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, uint64_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, uint32_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, uint16_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, uint8_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, int64_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, int32_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, int16_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, int8_t& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, double& val, bool permitPlusToken=true);
  SDKCORE_EXPORT bool isValidNumber(const std::string& token, float& val, bool permitPlusToken=true);
  ///@}

  ///@{
  /**
   * Determines if the incoming string is a valid hexadecimal number and then performs the conversion.
   * Hexadecimal values will succeed.  Strings support but do not require a leading 0x (or 0X).  String
   * is case insensitive.  Values outside the valid range of the data type will fail.
   * @param[in ] token String to validate
   * @param[out] val Converted number, set to 0 if conversion fails
   * @param[in ] require0xPrefix If true, the conversion will fail if the token does not start with "0x".
   *   The 0x prefix is case insensitive.
   * @return true if valid, false if not
   */
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, uint32_t& val, bool require0xPrefix=false);
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, uint16_t& val, bool require0xPrefix=false);
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, uint8_t& val, bool require0xPrefix=false);
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, int32_t& val, bool require0xPrefix=false);
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, int16_t& val, bool require0xPrefix=false);
  SDKCORE_EXPORT bool isValidHexNumber(const std::string& token, int8_t& val, bool require0xPrefix=false);
  ///@}
}

#endif /* SIMCORE_STRING_VALIDNUMBER_H */
