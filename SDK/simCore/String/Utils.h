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
#ifndef SIMCORE_STRING_UTILS_H
#define SIMCORE_STRING_UTILS_H

#include <string>
#include "simCore/Common/Export.h"
#include "simCore/String/Constants.h"

namespace simCore
{

/**
 * @brief Container class for string utility functions
 *
 * NOTE: find/rfind vs. find_first_of/find_last_of
 *
 * find: searches the input string for the sequence of characters specified
 *
 * find_first_of: searches the input string for a character that matches any of the characters specified
 *
 * The before and after functions in this class perform matches based on the sequence specified.
 */
class StringUtils
{
public:

  /**
   * Returns the portion of the input string before the first occurrence of the second string
   * @param[in ] in Input string to check
   * @param[in ] str String to search in the input string
   * @return Portion of the input string before the first occurrence of the second string
   */
  static std::string before(const std::string& in, const std::string& str)
  {
    size_t pos = in.find(str);
    return in.substr(0, pos);
  };

  /**
   * Returns the portion of the input string before the first occurrence of the specified character
   * @param[in ] in Input string to check
   * @param[in ] chr Character to search in the input string
   * @return Portion of the input string before the first occurrence of the specified character
   */
  static std::string before(const std::string& in, char chr)
  {
    size_t pos = in.find(chr);
    return in.substr(0, pos);
  };

  /**
   * Returns the portion of the input string after the first occurrence of the second string
   * @param[in ] in Input string to check
   * @param[in ] str String to search in the input string
   * @return Portion of the input string after the first occurrence of the second string
   */
  static std::string after(const std::string& in, const std::string& str)
  {
    size_t pos = in.find(str);
    if (pos == std::string::npos)
      return "";
    return in.substr(pos+str.length(), std::string::npos);
  };

  /**
   * Returns the portion of the input string after the first occurrence of the specified character
   * @param[in ] in Input string to check
   * @param[in ] chr Character to search in the input string
   * @return Portion of the input string after the first occurrence of the specified character
   */
  static std::string after(const std::string& in, char chr)
  {
    size_t pos = in.find(chr);
    if (pos == std::string::npos)
      return "";
    return in.substr(pos+1, std::string::npos);
  };

  /**
   * Returns the portion of the input string before the last occurrence of the second string
   * @param[in ] in Input string to check
   * @param[in ] str String to search in the input string
   * @return Portion of the input string before the last occurrence of the second string
   */
  static std::string beforeLast(const std::string& in, const std::string& str)
  {
    size_t pos = in.rfind(str);
    return in.substr(0, pos);
  };

  /**
   * Returns the portion of the input string before the last occurrence of the specified character
   * @param[in ] in Input string to check
   * @param[in ] chr Character to search in the input string
   * @return Portion of the input string before the last occurrence of the specified character
   */
  static std::string beforeLast(const std::string& in, char chr)
  {
    size_t pos = in.rfind(chr);
    return in.substr(0, pos);
  };

  /**
   * Returns the portion of the input string after the last occurrence of the second string
   * @param[in ] in Input string to check
   * @param[in ] str String to search in the input string
   * @return Portion of the input string after the last occurrence of the second string
   */
  static std::string afterLast(const std::string& in, const std::string& str)
  {
    size_t pos = in.rfind(str);
    if (pos == std::string::npos)
      return "";
    return in.substr(pos+str.length(), std::string::npos);
  };

  /**
   * Returns the portion of the input string after the last occurrence of the specified character
   * @param[in ] in Input string to check
   * @param[in ] chr Character to search in the input string
   * @return Portion of the input string after the last occurrence of the specified character
   */
  static std::string afterLast(const std::string& in, char chr)
  {
    size_t pos = in.rfind(chr);
    if (pos == std::string::npos)
      return "";
    return in.substr(pos+1, std::string::npos);
  };

  /**
   * Performs a string substitution
   * @param[in ] haystack Input string to perform substitution
   * @param[in ] needle String to find in the input string
   * @param[in ] newValue String to replace the found value in the input string
   * @param[in ] replaceAll Boolean, true: replace all string patterns found
   * @return Substituted string
   */
  static std::string substitute(std::string haystack, const std::string& needle, const std::string& newValue, bool replaceAll=true)
  {
    size_t pos = haystack.find(needle);
    while (pos != std::string::npos)
    {
      haystack.replace(pos, needle.length(), newValue);
      pos = haystack.find(needle, pos+newValue.length());
      if (!replaceAll)
      {
        break;
      }
    }
    return haystack;
  }

  /**
   * Adds an extra backslash to any existing backslash. Also adds a backslash to any " character.
   * Escaped strings from this function are meant to be parsed by escapeTokenize or removeEscapeSlashes.
   * @param[in ] input String to modify
   * @param[in ] escapeNewLine flag to indicate if newline should be escaped
   * @return escaped string
   */
  static std::string addEscapeSlashes(const std::string& input, bool escapeNewLine=true)
  {
    std::string rv = input;
    rv = simCore::StringUtils::substitute(rv, "\\", "\\\\"); // single slash becomes double slash (e.g. \ => \\)
    rv = simCore::StringUtils::substitute(rv, "\"", "\\\""); // single quote becomes slash quote  (e.g. " => \")
    if (escapeNewLine)
      rv = simCore::StringUtils::substitute(rv, "\n", "\\0xA"); // newline becomes \0xA  (e.g. \n => \\0xA")
    return rv;
  }

  /**
   * Removes extra backslashes added by addEscapeSlashes
   * @param[in ] input String to modify
   * @return un-escaped string
   */
  static std::string removeEscapeSlashes(const std::string& input)
  {
    std::string rv = input;
    rv = simCore::StringUtils::substitute(rv, "\\\\", "\\");
    rv = simCore::StringUtils::substitute(rv, "\\\"", "\"");
    rv = simCore::StringUtils::substitute(rv, "\\0xA", "\n");
    return rv;
  }

  /**
   * Trims away specified characters from the beginning of a string.
   * @param[in ] str String to trim
   * @param[in ] trimChars Characters to trim from input string, defaults to common white space characters
   * @return trimmed string
   */
  static std::string trimLeft(const std::string& str, const std::string& trimChars=STR_WHITE_SPACE_CHARS)
  {
    // NOTE: trims individual characters found, not the sequence of characters specified
    size_t pos = str.find_first_not_of(trimChars);
    return (pos == std::string::npos) ? "" : str.substr(pos);
  }

  /**
   * Trims away specified characters from the end of a string.
   * @param[in ] str String to trim
   * @param[in ] trimChars Characters to trim from input string, defaults to common white space characters
   * @return trimmed string
   */
  static std::string trimRight(const std::string& str, const std::string& trimChars=STR_WHITE_SPACE_CHARS)
  {
    // NOTE: trims individual characters found, not the sequence of characters specified
    size_t pos = str.find_last_not_of(trimChars);
    return (pos == std::string::npos) ? "" : str.substr(0, 1 + pos);
  }

  /**
   * Trims away specified characters from both ends of a string.
   * @param[in ] str String to trim
   * @param[in ] trimChars Characters to trim from input string, defaults to common white space characters
   * @return trimmed string
   */
  static std::string trim(const std::string& str, const std::string& trimChars=STR_WHITE_SPACE_CHARS)
  {
    // NOTE: trims individual characters found, not the sequence of characters specified
    size_t firstPos = str.find_first_not_of(trimChars);
    if (firstPos == std::string::npos)
      return "";
    size_t lastPos = str.find_last_not_of(trimChars);
    return str.substr(firstPos, 1 + lastPos - firstPos);
  }
};

/* ************************************************************************ */

/**
 * Convert backslashes to forward slashes, useful for OS path normalization
 * @param[in ] path Input path that may or may not have backslashes
 * @return Path, with backslashes converted to forward slashes
 */
inline std::string backslashToFrontslash(const std::string& path)
{
  return StringUtils::substitute(path, "\\", "/", true);
}

/**
 * Converts the filename to native separators.  On Windows systems, forward slashes are
 * swapped to backslashes.  On Linux systems, backslashes are swapped to forward slashes.
 * On both systems, filenames with protocols ("://") are ignored and returned as given.
 * Note that this method was inspired by and serves the same function as both
 * osgDB::convertFileNameToNativeStyle() and QDir::toNativeSeparators(), but without
 * the dependency upon osgDB or Qt.  Duplicate slashes (e.g. "path//file") are reduced
 * to single slashes ("path/file").  Backslashes in the path are always considered to
 * be directory separators, and never escape sequences.  Tokenizing escape sequences in
 * path should be handled by the caller prior to calling this method.
 * @param path Path to file or directory to swap to native separators.
 * @return Path value with native separators: / on Linux, \ on Windows.  Server addresses
 *    with "://" are not modified.
 */
SDKCORE_EXPORT std::string toNativeSeparators(const std::string& path);

/**
 * Sanitizes a file name by converting back slashes to forward slashes and removing illegal characters
 * such as \ / : ? * < > |
 * @param[in ] fname Input file name to sanitize
 * @return sanitized file name, with back slashes converted to forward slashes and illegal characters removed
 */
SDKCORE_EXPORT std::string sanitizeFilename(const std::string& fname);

/**
 * Detects whether a given input string contains environment variables in
 * the format $(ENVVAR).
 * @param[in ] val Input string to test for environment variables of the format $(ABC)
 * @return True if an environment variable string exists inside the string
 */
SDKCORE_EXPORT bool hasEnv(const std::string& val);

/**
 * Expands all environment variables found inside the original string.  Respects
 * environment variables in the format $(ENV)
 * @param[in ] val Input value to expand environment variables
 * @return Updated string with environment variables expanded
 */
SDKCORE_EXPORT std::string expandEnv(const std::string& val);

/**
* Gets an environment variable
* @param env Environment variable to get
* @return value of env, with trailing carriage return (\r) chars trimmed, otherwise an empty string if it does not exist
*/
SDKCORE_EXPORT std::string getEnvVar(const std::string &env);

/**
 * Sets an environment variable.
 * @param key Environment variable to set
 * @param value Value to set the variable to
 * @param overrideExisting If false, then current setting is kept if one exists
 * @return 0 on success, non-zero on error
 */
SDKCORE_EXPORT int setEnvVar(const std::string& key, const std::string& value, bool overrideExisting);

/**
 * Remove trailing zeros
 * @param str String from which to remove trailing zeros
 * @param leaveDecimal If true, the decimal will not be removed with trailing zeros
 * @return The resulting string with trailing zeros removed
 */
SDKCORE_EXPORT std::string removeTrailingZeros(const std::string& str, bool leaveDecimal = false);

} // namespace simCore

#endif /* SIMCORE_STRING_UTILS_H */
