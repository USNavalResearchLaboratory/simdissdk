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
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/CsvReader.h"

namespace simCore
{

CsvReader::CsvReader(std::istream& stream)
  : stream_(stream),
  commentChar_('#'),
  lineNumber_(0)
{
}

CsvReader::~CsvReader()
{
}

size_t CsvReader::lineNumber() const
{
  return lineNumber_;
}

void CsvReader::setCommentChar(char commentChar)
{
  commentChar_ = commentChar;
}

int CsvReader::readLine(std::vector<std::string>& tokens, bool skipEmptyLines)
{
  tokens.clear();
  std::string line;
  while (simCore::getStrippedLine(stream_, line))
  {
    lineNumber_++;

    if (line.empty())
    {
      if (skipEmptyLines)
        continue;
      // Not skipping empty lines, return successfully with empty tokens vector
      return 0;
    }

    // Ignore comments
    if (line[0] == commentChar_)
      continue;

    simCore::stringTokenizer(tokens, line, ",", true, false);
    return 0;
  }
  return 1;
}

int CsvReader::readLineTrimmed(std::vector<std::string>& tokens, bool skipEmptyLines)
{
  const int rv = readLine(tokens, skipEmptyLines);
  if (rv != 0)
    return rv;

  // Remove leading and trailing whitespace from all tokens
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    const std::string tok = tokens[i];
    if (tok.empty())
      continue;
    tokens[i] = simCore::StringUtils::trim(tok);
  }
  return 0;
}

}
