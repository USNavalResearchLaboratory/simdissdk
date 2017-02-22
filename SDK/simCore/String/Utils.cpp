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
#include <vector>
#include <cstdlib>

#include "simCore/Calc/Angle.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/ValidNumber.h"
#include "simCore/String/Utils.h"

namespace simCore
{

/// Sanitizes a file name by converting back slashes to forward slashes and removing illegal characters
std::string sanitizeFilename(const std::string& fname)
{
  if (fname.empty())
    return "";
  // normalize path separators first
  std::string fixedSlashes = StringUtils::substitute(fname, "\\", "/", true);
  std::string basename;
  std::string fixedName;
  // retrieve base and path names
  if (fixedSlashes.find("/") == std::string::npos)
  {
    // no slashes found
    basename = fixedSlashes;
  }
  else
  {
    // set base and path based on position of last slash found
    basename = StringUtils::afterLast(fixedSlashes, '/');
    // set path and add path separator
    fixedName = StringUtils::beforeLast(fixedSlashes, '/');
    fixedName += "/";
  }
  // detect and remove illegal characters from base file name
  size_t len = basename.length();
  for (size_t i = 0; i < len; i++)
  {
    if (basename[i] != ':' &&
      basename[i] != '?' &&
      basename[i] != '*' &&
      basename[i] != '<' &&
      basename[i] != '>' &&
      basename[i] != '|')
      fixedName += basename[i];
  }
  return fixedName;
}

/// Detects if the given string has the $() environment variable name pattern
bool hasEnv(const std::string& val)
{
  // look for the env syntax
  const size_t start = val.find("$(");
  if (start == std::string::npos)
    return false; // not found

  // look for the closing paren
  size_t end = val.find(")", start);
  return end != std::string::npos;
}

/// Expands all known environment variable names in the given string
std::string expandEnv(const std::string& val)
{
  std::string rv = val;
  std::string value = val;
  size_t start = value.find("$(");
  while (start != std::string::npos)
  {
    size_t end = value.find(")", start);
    if (end == std::string::npos)
      return rv; // not found

    std::string firstPart = "";
    if (start > 0)
      firstPart = value.substr(0, start);
    std::string middlePart = value.substr(start + 2, end - start - 2);
    std::string endPart = value.substr(end + 1);
    rv = firstPart;
    if (getenv(middlePart.c_str()) != NULL)
      rv += getenv(middlePart.c_str());
    else // Retain the environment variable part, as per review 546
      rv += "$(" + middlePart + ")";
    rv += endPart;
    // check remainder for additional envs to expand
    value = rv;
    // set starting value 1 past last starting position to avoid circular loop
    start = value.find("$(", start + 1);
  }

  return rv;
}

std::string toNativeSeparators(const std::string& path)
{
  if (path.find("://") != std::string::npos)
    return path;
#ifdef WIN32
  const std::string BAD_SLASH = "/";
  const std::string GOOD_SLASH = "\\";
#else
  const std::string BAD_SLASH = "\\";
  const std::string GOOD_SLASH = "/";
#endif
  std::string fixedDirection = StringUtils::substitute(path, BAD_SLASH, GOOD_SLASH);
  std::string noDuplicates;
  do
  {
    noDuplicates = fixedDirection;
    // Iteratively replace all instances of double slash with single slash
    fixedDirection = StringUtils::substitute(fixedDirection, GOOD_SLASH + GOOD_SLASH, GOOD_SLASH);
  } while (noDuplicates != fixedDirection);
  return noDuplicates;
}

} // namespace simCore
