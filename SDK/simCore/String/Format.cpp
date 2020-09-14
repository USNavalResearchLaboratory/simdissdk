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
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "simCore/Calc/Math.h"
#include "simCore/String/Format.h"

namespace simCore
{

/// Case insensitive string comparison for std::string
int caseCompare(const std::string &str1, const std::string &str2)
{
  if (str1.empty())
    return str2.empty() ? 0 : -1;
  if (str2.empty())
    return 1;
#ifdef WIN32
  return _stricmp(str1.c_str(), str2.c_str());
#else
  return ::strcasecmp(str1.c_str(), str2.c_str());
#endif
}

/// Convert input string to lower-case
std::string lowerCase(const std::string &in)
{
  std::string outString;
  outString.resize(in.size());
  std::transform(in.begin(), in.end(), outString.begin(), ::tolower);
  return outString;
}

/// Convert input string to upper-case
std::string upperCase(const std::string &in)
{
  std::string outString;
  outString.resize(in.size());
  std::transform(in.begin(), in.end(), outString.begin(), ::toupper);
  return outString;
}

/// Case insensitive string find for std::string
size_t stringCaseFind(const std::string &str1, const std::string &str2)
{
  const std::string& upStr1 = upperCase(str1);
  return upStr1.find(upperCase(str2));
}

/// Removes trailing white space from a line read from a stream
bool getStrippedLine(std::istream& is, std::string& str)
{
  // strips newline and everything after
  if (!std::getline(is, str))
  {
    return false;
  }
  // strips trailing white space
  str.erase(str.find_last_not_of(" \r\t") + 1);
  return true;
}

/// Returns the extension of incoming string (lower-case by default), including the '.'
std::string getExtension(const std::string &inName, bool toLower)
{
  if (inName.empty())
    return "";

  // convert to lower-case for insensitive comparison
  std::string outString = toLower ? lowerCase(inName) : inName;
  size_t found = outString.find_last_of(".");
  return (found != std::string::npos) ? outString.substr(found) : "";
}

/// Verifies the incoming string has the specified extension
bool hasExtension(const std::string& inName, std::string newExt)
{
  std::transform(newExt.begin(), newExt.end(), newExt.begin(), ::tolower);
  return getExtension(inName) == newExt;
}

/// Builds a formatted string with prefix and suffix
std::string buildString(const std::string &prefix, double value, size_t width, size_t precision,
  const std::string &suffix, bool padZero, double sciNoteGT, double sciNoteLT)
{
  std::stringstream strVal;
  strVal << prefix;

  if (std::isnan(value))
    strVal << "NaN";
  else if (std::isinf(value))
  {
    if (value < 0)
      strVal << "-inf";
    else
      strVal << "inf";
  }
  else
  {
    // Limit the precision
    const size_t realPrecision = simCore::sdkMin(precision, static_cast<size_t>(16));
    // scientific notation limit
    const double sciNoteGT2 = simCore::sdkMin(1e+80, sciNoteGT);
    const bool useScientific = (value > sciNoteGT2) || (value < -sciNoteGT2) ||
      (value != 0.0 && value < sciNoteLT&& value > -sciNoteLT);
    if (useScientific)
      strVal.setf(std::ios::scientific, std::ios::floatfield);
    else
      strVal.setf(std::ios::fixed, std::ios::floatfield);

#if defined(NDEBUG) && defined(_MSC_VER) && _MSC_VER > 1922
    // Avoid processing in https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/printf-printf-l-wprintf-wprintf-l?view=vs-2019
    // due to bug reported in https://developercommunity.visualstudio.com/content/problem/1085732/different-printf-double-rounding-behaviour-between.html
    if (!useScientific)
    {
      const double multFactor = std::pow(10.0, realPrecision);
      value = simCore::rint(value * multFactor) / multFactor;
    }
#endif

    if (padZero)
      strVal << std::setfill('0') << std::setw(width) << std::setprecision(realPrecision) << value;
    else
      strVal << std::setw(width) << std::setprecision(realPrecision) << value;
  }

  strVal << suffix;
  return strVal.str();
}

} // namespace simCore
