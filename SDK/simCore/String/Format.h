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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMCORE_STRING_FORMAT_H
#define SIMCORE_STRING_FORMAT_H

#include <sstream>
#include <string>
#include <vector>

#include "simCore/Common/Common.h"
#include "simCore/Common/Export.h"

namespace simCore
{
  /**
  * Joins the specified parameters into a delimited string
  * @param[in ] params vector of template parameters that will be joined into a string
  * @param[in ] delimiter delimiter value that separates each parameter
  * @return string of joined parameters
  */
  template <class T>
  inline std::string join(const std::vector<T>& params, const std::string& delimiter)
  {
    std::stringstream s;
    typename std::vector<T>::const_iterator i;
    for (i = params.begin(); i != params.end(); ++i)
    {
      if (i != params.begin())
        s << delimiter;
      s << *i;
    }
    return s.str();
  }

  /**
  * Case insensitive string comparison for std::string
  * @param[in ] str1 First string
  * @param[in ] str2 Second string
  * @return an integer greater than, equal to, or less than 0, if the string pointed to by str1 is, ignoring case, greater than, equal to, or less than the string pointed to by str2
  */
  SDKCORE_EXPORT int caseCompare(const std::string &str1, const std::string &str2);

  /**
  * Convert input string to lower-case
  * @param[in ] in Input string
  * @return lower-case equivalent of input string
  */
  SDKCORE_EXPORT std::string lowerCase(const std::string &in);

  /**
  * Convert input string to upper-case
  * @param[in ] in Input string
  * @return upper-case equivalent of input string
  */
  SDKCORE_EXPORT std::string upperCase(const std::string &in);

  /**
  * Case insensitive string find for std::string
  * @param[in ] str1 First string
  * @param[in ] str2 Content of this string is searched in str1; if empty, it will always be found, and the return value will always be 0
  * @return starting location of str2 in str1
  */
  SDKCORE_EXPORT size_t stringCaseFind(const std::string &str1, const std::string &str2);

  /**
  * Removes trailing white space from a line read from a stream
  * @param[in ] is Input stream
  * @param[out] str Content read from stream and placed into string
  * @return boolean, true if stream read was successful, false otherwise
  * @pre is valid input stream
  */
  SDKCORE_EXPORT bool getStrippedLine(std::istream& is, std::string& str);

  /**
  * Returns the extension of incoming string (lower-case by default), including the '.'
  * @param[in ] inName Input file name
  * @param[in ] toLower If true (default), return extension in all lowercase.  If false, then no case mangling is performed.
  * @return Extension based value after last '.', including the '.' if found, otherwise an empty string.  For example, ".txt"
  */
  SDKCORE_EXPORT std::string getExtension(const std::string &inName, bool toLower=true);

  /**
  * Verifies (case-insensitive) the incoming string has the specified extension, including the '.'
  * @param[in ] inName Input file name
  * @param[in ] newExt Extension to verify, including the '.'
  * @return boolean, true if found, false otherwise
  */
  SDKCORE_EXPORT bool hasExtension(const std::string& inName, std::string newExt);

  /**
   * Builds a formatted double output string, with optional prefix and suffix.  Will fall
   * back from decimal notation to scientific notation if the value is greater-than or
   * less-than the specified values.
   * @param prefix Text to print before the value in the return string.
   * @param value Double value to print as a string.
   * @param width Minimum number of characters to use as field width when printing value.
   * @param precision Decimal precision to be used in formatting the value.
   * @param suffix Text to print after the value in the return string.
   * @param padZero If true, values to left of decimal (based on width) fill with '0'.
   * @param sciNotationGT If value > sciNotationGT, then scientific notation used.
   * @param sciNotationLT If value < sciNotationLT, then scientific notation used.
   * @return String formatted with the given options.
   */
  SDKCORE_EXPORT std::string buildString(const std::string& prefix, double value, size_t width=11,
    size_t precision=7, const std::string& suffix="", bool padZero=false, double sciNotationGT=1e+15,
    double sciNotationLT=1e-15);

} // namespace simCore

#endif /* SIMCORE_STRING_FORMAT_H */
