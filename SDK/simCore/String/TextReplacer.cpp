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
#include <cassert>
#include <algorithm>
#include <sstream>
#include "simCore/String/Utils.h"
#include "simCore/String/TextReplacer.h"

namespace simCore
{

TextReplacer::TextReplacer()
  : undefinedHandler_(new IgnoreUndefinedVariables)
{
}

TextReplacer::~TextReplacer()
{
  for (auto i = replaceables_.begin(); i != replaceables_.end(); ++i)
    delete i->second;
}

std::string TextReplacer::format(const std::string& formatString) const
{
  // Allow up to 4 recursions before giving up
  return formatImpl_(formatString, 4);
}

std::string TextReplacer::formatImpl_(const std::string& formatString, int depth) const
{
  // Prevent infinite recursion
  if (depth < 0)
    return formatString;

  if (formatString.empty())
    return formatString;
  size_t writtenUpTo = 0;
  std::stringstream ss;
  do
  {
    // Validate preconditions
    assert(writtenUpTo != std::string::npos);
    assert(writtenUpTo < formatString.length());

    // Find start of next variable
    const size_t openVariable = formatString.find('%', writtenUpTo);

    // First, write until the open variable (OK if npos)
    if (writtenUpTo != openVariable)
      ss << formatString.substr(writtenUpTo, openVariable - writtenUpTo);
    // If that open variable was at npos (not found), then we're done
    if (openVariable == std::string::npos)
      break;

    // Need to find a closing symbol
    const size_t closeVariable = formatString.find('%', openVariable + 1);
    // Catch edge case of not having a close variable.  In this case, we are done.
    if (closeVariable == std::string::npos)
    {
      // Write remainder of string
      ss << formatString.substr(openVariable, std::string::npos);
      break;
    }

    // Decode the variable name
    const std::string& varName = formatString.substr(openVariable, 1 + closeVariable - openVariable);
    ss << evaluate_(varName, depth);

    // Remember how far we've written
    writtenUpTo = closeVariable + 1;

    // Break out of loop if we're at the end of the string
  } while (writtenUpTo != std::string::npos && writtenUpTo < formatString.length());

  return ss.str();
}

// on success, the TextReplacer will assume ownership of the Replaceable
int TextReplacer::addReplaceable(simCore::TextReplacer::Replaceable* r)
{
  if (!r)
    return 1;
  const std::string key = validateName_(r->getVariableName());
  if (key.empty())
  {
    // We own the memory here
    delete r;
    return 1;
  }

  auto i = replaceables_.find(key);
  // Key found; replace the old version
  if (i != replaceables_.end())
  {
    // Remember to delete old version before replacing it
    if (i->second != r)
    {
      delete i->second;
      i->second = r;
      return 0; // success
    }
    return 1; // duplicate, failure
  }
  replaceables_[key] = r;
  return 0;
}

int TextReplacer::deleteReplaceable(simCore::TextReplacer::Replaceable* r)
{
  if (!r)
    return 1;
  return deleteReplaceable(r->getVariableName());
}

int TextReplacer::deleteReplaceable(const std::string& variableName)
{
  const std::string& key = validateName_(variableName);
  auto i = replaceables_.find(key);
  if (i == replaceables_.end())
    return 1;
  delete i->second;
  replaceables_.erase(i);
  return 0;
}

std::string TextReplacer::evaluate_(const std::string& varNameWithPct, int depth) const
{
  // Code ensures that there's at a minimum "%%" for variable name.
  assert(varNameWithPct.size() >= 2);
  // Built-in: Replace "%%" (shows up as "" here) with just a single percent symbol
  if (varNameWithPct.size() <= 2)
    return "%";
  auto i = replaceables_.find(varNameWithPct);
  if (i == replaceables_.end())
  {
    if (undefinedHandler_)
    {
      const std::string& replacementText = undefinedHandler_->getText(varNameWithPct);
      // Recurse into formatImpl_(), with one less depth, only if needed
      if (replacementText.find('%') == std::string::npos)
        return replacementText;
      // Optimization: Avoid recursion if incoming value is same
      else if (replacementText == varNameWithPct)
        return replacementText;
      return formatImpl_(replacementText, depth - 1);
    }
    return "";
  }

  const std::string& replacementText = i->second->getText();
  // Recurse into formatImpl_(), with one less depth, only if needed
  if (replacementText.find('%') == std::string::npos)
    return replacementText;
  return formatImpl_(replacementText, depth - 1);
}

std::string TextReplacer::validateName_(const std::string& inputName) const
{
  // Error return if empty variable name
  if (inputName.empty())
    return "";
  // Case 1: % and % are on start and end of string
  if (inputName[0] == '%' && inputName[inputName.length() - 1] == '%')
  {
    // Make sure there are no other percent symbols in the string
    auto nextPct = inputName.find('%', 1);
    if (nextPct != inputName.length() - 1)
      return "";
    // Must have more than 2 characters, don't accept "%%" or "%"
    if (inputName.length() <= 2)
    {
      // Assertion failure means we have a logic error in code above.  This is postcondition
      assert(inputName == "%" || inputName == "%%");
      return "";
    }

    // Name should be valid by here
    return inputName;
  }

  // Case 2: % are not surrounding.  Should have no percents in the string
  if (inputName.find('%') != std::string::npos)
    return "";
  // Name is valid, but needs % added
  return "%" + inputName + "%";
}


void TextReplacer::setUndefinedVariableHandler(UndefinedVariableHandlerPtr handler)
{
  undefinedHandler_ = handler;
}

}
