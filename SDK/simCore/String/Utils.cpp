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
    const std::string middlePart = value.substr(start + 2, end - start - 2);
    const std::string endPart = value.substr(end + 1);
    // Replace $(ENV) only if ENV is not empty and contains no whitespace
    bool replace = (middlePart.empty() ? false : true);
    for (size_t i = 0; i < middlePart.size() && replace; i++)
      replace = !isspace(middlePart[i]);

    size_t searchFrom;
    rv = firstPart;
    if (replace)
    {
      const std::string envVar = simCore::getEnvVar(middlePart);
      rv += envVar;
      // Search starting from the end of the replacement text
      searchFrom = start + envVar.size();
    }
    else
    {
      rv += "$(" + middlePart + ")";
      // Search starting from after the current $ to avoid circular loop
      searchFrom = start + 1;
    }
    rv += endPart;
    // check remainder for additional envs to expand
    value = rv;
    start = value.find("$(", searchFrom);
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

  // Duplicate slashes at the start indicate a UNC path so the duplication should NOT be removed
  std::string prefix;
  if (fixedDirection.substr(0, 2 * GOOD_SLASH.size()) == GOOD_SLASH + GOOD_SLASH)
  {
    prefix = GOOD_SLASH + GOOD_SLASH;
    fixedDirection = fixedDirection.substr(2 * GOOD_SLASH.size());
  }

  // Now remove duplicates from the rest of the path
  std::string noDuplicates;
  do
  {
    noDuplicates = fixedDirection;
    // Iteratively replace all instances of double slash with single slash
    fixedDirection = StringUtils::substitute(fixedDirection, GOOD_SLASH + GOOD_SLASH, GOOD_SLASH);
  } while (noDuplicates != fixedDirection);

  // Add prefix, if any
  return prefix + noDuplicates;
}

std::string getEnvVar(const std::string &env)
{
  const char *cenv = getenv(env.c_str());
  if (!cenv)
    return "";
  return StringUtils::trimRight(cenv, "\r");
}

} // namespace simCore
