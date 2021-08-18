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
#ifndef SIMCORE_STRING_TOKENIZER_H
#define SIMCORE_STRING_TOKENIZER_H

#include <iostream>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/String/Constants.h"

namespace simCore
{
  /** fill 't' with tokens from 'str' defined by delimiters
  * @param[out] t STL container that supports push_back(), tokens inserted into this container.
  * @param[in ] str string to be split into tokens.
  * @param[in ] delimiters delimiter value(s) for tokenizing.
  * @param[in ] clear boolean for clearing STL container before new tokens are inserted.
  * @param[in ] skipMultiple boolean, if true skips multiple delimiters encountered as a group
  */
  template<class T>
  inline void stringTokenizer(T &t, const std::string &str, const std::string &delimiters = STR_WHITE_SPACE_CHARS, bool clear = true, bool skipMultiple = true)
  {
    if (clear)
      t.clear();

    // Skip delimiters at beginning.
    std::string::size_type lastPos = 0;
    if (skipMultiple)
      lastPos = str.find_first_not_of(delimiters);

    // go from non-delimiter to delimiter
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Found a token, add it to the STL container.
      t.push_back(str.substr(lastPos, pos - lastPos));

      // Skip delimiters.  Note: "not_of"
      if (skipMultiple)
        lastPos = str.find_first_not_of(delimiters, pos);
      else if (pos != std::string::npos)
        lastPos = pos + 1;
      else
        lastPos = std::string::npos;

      // get next token
      pos = str.find_first_of(delimiters, lastPos);
    }
  }

  /**
  * This function removes trailing white space from a string read from
  * a stream, then parses the string into tokens delimited by whitespace.
  * @param[in ] is input stream to read
  * @param[out] t STL container that supports push_back() and clear()
  * @param[in ] minTokens Minimum number of tokens expected
  * @param[in ] delimiters delimiter value(s) for tokenizing.
  * @param[in ] clear boolean for clearing STL container before new tokens are inserted.
  */
  template<class T>
  inline bool getTokens(std::istream& is, T &t, size_t minTokens=0, const std::string &delimiters = STR_WHITE_SPACE_CHARS, bool clear = true)
  {
    std::string str;
    if (!std::getline(is, str))
    {
      return false;
    }
    str.erase(str.find_last_not_of(STR_WHITE_SPACE_CHARS) + 1);
    stringTokenizer(t, str, delimiters, clear);
    return (t.size() >= minTokens) ? true : false;
  }

  /**
  * This function returns a substring portion of the line that spans between the given startPos and endWordPos values.
  * Leading white space is removed from the returned substring.
  * @param[in ] line text to parse
  * @param[out] endWordPos index of last character in the word
  * @param[in ] startPos starting position to use
  * @return the substring in line
  */
  SDKCORE_EXPORT std::string extractWord(const std::string &line, size_t &endWordPos, size_t startPos = 0);

  /**
  * This function returns a substring portion of the line that spans between the given startPos and endWordPos values.
  * Leading white space is removed from the returned substring and only double quotes are supported.
  * @param[in ] line text to parse
  * @param[out] endWordPos index of last character in the word
  * @param[in ] startPos starting position to use
  * @return the substring in line
  */
  SDKCORE_EXPORT std::string extractWordWithQuotes(const std::string &line, size_t &endWordPos, size_t startPos = 0);

  /**
  * Tokenization helper function that returns the proper termination sequence, respecting quotes.
  * In the simple case, this returns an empty string, which implies that any whitespace breaks a
  * token (exclusive).  Another case covers single quote and double quote tokenization.
  * The complex case is a triple double quote, such as Excel uses.
  * @param[in ] str Source string
  * @param[in ] pos Position to start the token, including any potential quotes
  * @return String that should be searched for in order to terminate the token;
  * Note, that if this is empty, any whitespace should terminate the token.
  */
  SDKCORE_EXPORT std::string getTerminateForStringPos(const std::string &str, size_t pos);

  /**
  * Calculates the position of first character after a termination string.  This
  * takes in as a parameter the termination string, which when empty means that any
  * whitespace will serve as a delimiter.  When non-empty, this function returns the
  * position of the first character after the termString is found.
  * This is used by the quoteTokenizer function.
  * @param[in ] str Input string
  * @param[in ] start Start position in the string to search for the termString
  * @param[in ] termString When empty, search for whitespace; when non-empty, search for the sequence of characters as specified in this string
  * @return Position of the first character after termString was found, or std::string::npos if the entire string was searched.
  */
  SDKCORE_EXPORT size_t getFirstCharPosAfterString(const std::string &str, size_t start, const std::string &termString);

  /**
  * Tokenizes 'str' based on white space while ignoring white space encountered within double quotes.
  * Leading white space is removed from the tokens and only double quotes are supported.
  * @param[out] t STL container that supports push_back() and clear()
  * @param[in ] str String to tokenize
  * @param[in ] clear Clears the STL structure if true
  */
  template<class T>
  void tokenizeWithQuotes(T &t, const std::string &str, bool clear = true)
  {
    if (clear)
      t.clear();

    size_t endWord = 0;
    do
    {
      // skip spaces before words
      size_t startWord = str.find_first_not_of(" \t", endWord);
      if (startWord == std::string::npos)
        return;

      // push the next token
      t.push_back(extractWordWithQuotes(str, endWord, startWord));

    }
    while (endWord != str.size());
  }

  /**
  * Tokenizes 'str', respecting single, double and triple quotes.
  * @param[out] t STL container that supports push_back() and clear()
  * @param[in ] str String to tokenize
  * @param[in ] clear Clears the STL structure if true
  */
  template<class T>
  inline void quoteTokenizer(T &t, const std::string &str, bool clear = true)
  {
    if (clear)
      t.clear();

    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(STR_WHITE_SPACE_CHARS, 0);
    if (lastPos == std::string::npos)
      return;

    // Determine what token should terminate pos
    std::string terminateString = getTerminateForStringPos(str, lastPos);
    std::string::size_type pos = getFirstCharPosAfterString(str, lastPos+1, terminateString);
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
      // Got a token, push it back
      t.push_back(str.substr(lastPos, pos - lastPos));

      // Skip until the next non-whitespace
      lastPos = str.find_first_not_of(STR_WHITE_SPACE_CHARS, pos);

      // Figure out the terminate string based on current position
      terminateString = getTerminateForStringPos(str, lastPos);
      pos = (std::string::npos == lastPos) ? lastPos : getFirstCharPosAfterString(str, lastPos+1, terminateString);
    }
  }

  /**
  * Processes a STL container of tokens to remove tokens that are impacted by a
  * comment string.  This function supports # and // comments, and comments
  * mid-token, as long as the token is not quoted.  Modifies the structure in place.
  * @param t STL ordered container of strings
  */
  template<class T>
  inline void removeCommentTokens(T &t)
  {
    for (typename T::iterator i = t.begin(); i != t.end(); ++i)
    {
      // Do not operate on quoted tokens
      if ((*i)[0] == '\'' || (*i)[0] == '"')
        continue;

      // Search for comments
      size_t poundAt = (*i).find("#");
      size_t slashAt = (*i).find("//");
      size_t commentAt = std::string::npos;
      if (std::string::npos != poundAt)
      {
        commentAt = (commentAt > poundAt) ? poundAt : commentAt;
      }

      if (std::string::npos != slashAt)
      {
        commentAt = (commentAt > slashAt) ? slashAt : commentAt;
      }

      if (std::string::npos != commentAt)
      {
        if (0 == commentAt)
        {
          // Cut off this whole vec
          t.erase(i, t.end());
          return;
        }

        // Cut off subset of this vec
        *i = (*i).substr(0, commentAt);
        ++i;
        t.erase(i, t.end());
        return;
      }
    }
  }

  /**
  * Removes extraneous quotes from 'inString', on the outside only
  * @param[in ] inString String from which quotes should be removed
  * @return A string without the extra quotes!
  */
  SDKCORE_EXPORT std::string removeQuotes(const std::string &inString);

  /**
   * Removes the quotes (simCore::removeQuotes()) on all tokens in the vector strVec.
   * Vectorized format to remove extraneous quotes from 'inString', on the outside only.
   * @param strVec Vector of tokens on which to call removeQuotes()
   */
  SDKCORE_EXPORT void removeQuotes(std::vector<std::string>& strVec);

  /**
  * Sets token name and token value from 'token' with a "name=value" pattern
  * @param[in ] token String from which pull token name and value
  * @param[out] tokenName Token name
  * @param[out] tokenValue Token value
  * @return error conditions (empty string on success)
  */
  SDKCORE_EXPORT std::string getNameAndValueFromToken(const std::string &token, std::string &tokenName, std::string &tokenValue);

  /** Performs tokenization using either white space, single, double or triple quotes.
  * Comment detection, using // or #, and removal is also performed.
  * @param[in ] str String to tokenize
  * @param[out] t STL container that supports push_back() and clear()
  */
  template<class T>
  inline void quoteCommentTokenizer(const std::string& str, T& t)
  {
    simCore::quoteTokenizer(t, str);

    if (!t.empty())
    {
      simCore::removeCommentTokens(t);
      for (typename T::iterator i = t.begin(); i != t.end(); ++i)
      {
        *i = simCore::removeQuotes(*i);
      }
    }
  }

  /** Tokenizes 'str' taking into account quoted strings containing escaped quotes
  * e.g. "My grooviest token is \"Token\"" oh-yea "that's the one"
  * (three tokens)
  * tokens include any original quotes and flattened escapes: \\ and \n
  * (simple substitution of \n would break the tokenizing).
  * This function flattens the escape chars added by StringUtils::addEscapeSlashes.
  * @param[out] t STL container that supports push_back() and clear()
  * @param[in ] str String to tokenize
  * @param[in ] clear Clears the STL structure if true
  * @param[in ] delims String listing all characters to use as token delimiters
  * @param[in ] skipEmptyTokens If true, skips multiple delimiters encountered as a group
  * @param[in ] testSingleQuote If true, single quotes will also prevent breaking into tokens
  * @param[in ] endTokenWithQuotes If true, when a quoted string is closed it will end the token
  */
  template<class T>
  inline void escapeTokenize(T& t, const std::string &str, bool clear = true, const std::string &delims = STR_WHITE_SPACE_CHARS,
    bool skipEmptyTokens = true, bool testSingleQuote = false, bool endTokenWithQuotes = true)
  {
    if (clear)
      t.clear();

    std::string::const_iterator i = str.begin();
    std::string curTok;
    bool inEscape = false; // \\ or \"
    bool inQuote  = false; // "quoted token"
    bool inSingleQuote = false; // 'quoted token'
    for (; i != str.end(); ++i)
    {
      if (*i == '\\' && !inEscape)
      {
        // start escape
        inEscape = true;
        continue;
      }

      if (inEscape)
      {
        // end escape, check for \n
        if (*i != 'n')
          curTok.append(1, *i);
        else
          curTok.append(1, '\n');

        inEscape = false;
        continue;
      }

      if (*i == '"')
      {
        // start or end of quoted token
        curTok.append(1, *i);

        if (inQuote)
        {
          // end
          if (endTokenWithQuotes)
          {
            t.push_back(curTok);
            curTok.clear();
          }

          inQuote = false;
        }
        else if (!inSingleQuote) // Ignore double quotes inside 'quoted "token'
        {
          // start "quoted token"
          inQuote = true;
        }

        continue;
      }

      // Only care about single quotes when instructed
      if (*i == '\'' && testSingleQuote)
      {
        // start or end of quoted token
        curTok.append(1, *i);

        if (inSingleQuote)
        {
          // end
          if (endTokenWithQuotes)
          {
            t.push_back(curTok);
            curTok.clear();
          }

          inSingleQuote = false;
        }
        else if (!inQuote)  // Ignore single quotes inside "quoted 'token"
        {
          // start 'quoted token'
          inSingleQuote = true;
        }

        continue;
      }

      // If we aren't in a quoted token, test if the current character is one of our delimiters
      if (!inQuote && !inSingleQuote && (delims.find(*i) != std::string::npos))
      {
        // Found delimiter
        if (!skipEmptyTokens || !curTok.empty())
        {
          t.push_back(curTok);
          curTok.clear();
        }
        continue;
      }

      curTok.append(1, *i);
    } //foreach char

    // Include any non-finished tokens when we hit the end of the string
    if (!skipEmptyTokens || !curTok.empty())
      t.push_back(curTok);
  }


} // namespace simCore

#endif /* SIMCORE_STRING_TOKENIZER_H */
