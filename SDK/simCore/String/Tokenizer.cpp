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
#include <cstdlib>
#include "simCore/String/Tokenizer.h"

/**
* This function returns a substring portion of the line that spans between the given startPos and endWordPos values.
* Leading white space is removed from the returned substring.
*/
std::string simCore::extractWord(const std::string &line, size_t &endWordPos, size_t startPos)
{
  endWordPos = line.find_first_of(" \t", startPos);
  if (endWordPos == std::string::npos)
  {
    endWordPos = line.size();
  }

  return line.substr(startPos, endWordPos - startPos);
}

/**
* This function returns a substring portion of the line that spans between the given startPos and endWordPos values.
* Leading white space is removed from the returned substring and only double quotes are allowed.
*/
std::string simCore::extractWordWithQuotes(const std::string &line, size_t &endWordPos, size_t startPos)
{
  std::string delim = " \t\""; // standard delimiters are whitespace or quote

  size_t searchPos = startPos;
  // if the token starts with quote
  bool hasQuote = (line[startPos] == '"');
  if (hasQuote)
  {
    // and is at the end of the line
    if (line.size() == startPos + 1)
      return "\""; // that's all there is

    //else, start the search on the next char
    ++searchPos;
    delim = "\""; // in quote, stop only on quote
  }

  // find delimiter after start
  endWordPos = line.find_first_of(delim, searchPos);

  if (endWordPos == std::string::npos)
  {
    endWordPos = line.size();
  }
  else if (line[endWordPos] == '"')
  {
    if (hasQuote)
    {
      // found end quote, good to go
      ++endWordPos;
    }
    else if (endWordPos + 1 == line.size())
    {
      // quote at end of line
      ++endWordPos;
    }
    else // first quote
    {
      // find end quote
      endWordPos = line.find_first_of('"', endWordPos + 1);
      if (endWordPos == std::string::npos)
        endWordPos = line.size();
      else
        ++endWordPos;
    }
  }

  return line.substr(startPos, endWordPos - startPos);
}

/**
* Tokenization helper function that returns the proper termination sequence, respecting quotes.
* In the simple case, this returns an empty string, which implies that any whitespace breaks a
* token (exclusive).  Another case covers single quote and double quote tokenization.
* The complex case is a triple double quote, such as Excel uses.
*/
std::string simCore::getTerminateForStringPos(const std::string &str, size_t pos)
{
  if (pos > str.length())
    return "";

  // single quote
  if (str[pos] == '\'')
    return "'";

  // double quote, check for triple
  if (str[pos] == '"')
  {
    // if not enough characters for triple
    if (pos + 3 > str.length())
      return "\"";

    // match three but not four
    if (str[pos+1] == '"' && str[pos+2] == '"' && str[pos+3] != '"')
      return "\"\"\"";

    // double
    return "\"";
  }

  // not quoted
  return "";
}

/**
* Calculates the position of first character after a termination string.  This
* takes in as a parameter the termination string, which when empty means that any
* whitespace will serve as a delimiter.  When non-empty, this function returns the
* position of the first character after the termString is found.
* This is used by the quoteTokenizer function.
*/
size_t simCore::getFirstCharPosAfterString(const std::string &str, size_t start, const std::string &termString)
{
  if (termString.empty())
  {
    // no terminator, use whitespace
    return str.find_first_of(simCore::STR_WHITE_SPACE_CHARS, start);
  }

  size_t pos = str.find(termString, start);
  if (pos == std::string::npos)
  {
    // not found
    return std::string::npos;
  }

  // single quotes can be escaped with a leading back slash
  if ((termString == "\"") && (pos > 0))
  {
    // Need to search until an escaped quote is found
    while (str[pos - 1] == '\\')
    {
      // Need to count the number of preceding back slashes
      // If old number of preceding back slashes then the quote is escaped
      // If even number of preceding back slashes then the quote is NOT escaped.
      size_t counterPos = pos - 1;
      unsigned int counter = 1;
      while ((counterPos > 0) && (str[counterPos - 1] == '\\'))
      {
        ++counter;
        --counterPos;
      }

      // if even number of preceding back slashes then the quote is NOT escaped, so kick out
      if ((counter % 2) == 0)
        break;

      // look for the next possible quote
      pos = str.find(termString, pos+1);
      if (pos == std::string::npos)
      {
        // not found
        return std::string::npos;
      }
    }
  }

  const size_t endOfStr = pos + termString.length();
  if (endOfStr > str.length())
    return std::string::npos;

  return endOfStr;
}

/**
* Removes extraneous quotes from 'inString', on the outside only
*/
std::string simCore::removeQuotes(const std::string &inString)
{
  std::string::size_type lastPos = inString.size();
  if (lastPos <= 1) // short string
    return inString;

  // get quote type
  const char firstChar = inString[0];
  if (firstChar != '\'' && firstChar != '"')
    return inString; // not quoted

  // compare nth with nth from end
  --lastPos;
  std::string::size_type nowPos = 0;
  while (lastPos > nowPos &&
    inString[ nowPos] == firstChar &&
    inString[lastPos] == firstChar)
  {
    nowPos++; lastPos--;
  }

  // Substring it from [nowPos, lastPos] inclusive
  return (lastPos >= nowPos) ? inString.substr(nowPos, (lastPos - nowPos) + 1) : "";
}

void simCore::removeQuotes(std::vector<std::string>& strVec)
{
  for (std::vector<std::string>::iterator i = strVec.begin(); i != strVec.end(); ++i)
    *i = simCore::removeQuotes(*i);
}

/**
* Sets token name and token value from 'token' with a "name=value" pattern
*/
std::string simCore::getNameAndValueFromToken(const std::string &token, std::string &tokenName, std::string &tokenValue)
{
  const size_t index = token.find("=");
  if (index == std::string::npos)
  {
    std::string errorString = "(";
    errorString.append(token);
    errorString.append(") Incorrect Token Format (pattern format: name=value)");
    return errorString;
  }

  tokenName  = token.substr(0, index);
  tokenValue = simCore::removeQuotes(token.substr(index+1));

  return "";
}
