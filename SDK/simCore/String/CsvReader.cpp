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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
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

CsvReader::CsvReader(std::istream& stream, const std::string& delimiters)
  : stream_(stream),
  delimiters_(delimiters),
  parseQuotes_(true),
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

void CsvReader::setParseQuotes(bool parseQuotes)
{
  parseQuotes_ = parseQuotes;
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

    getTokens_(tokens, line);
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

void CsvReader::getTokens_(std::vector<std::string>& tokens, const std::string& line) const
{
  auto delimPos = line.find_first_of(delimiters_);
  if (delimPos == std::string::npos)
  {
    tokens.push_back(line);
    return;
  }

  auto quotePos = line.find_first_of("'\"");
  if (!parseQuotes_ || (quotePos == std::string::npos))
  {
    simCore::stringTokenizer(tokens, line, delimiters_, true, false);
    return;
  }

  simCore::escapeTokenize(tokens, line, true, delimiters_, false, true, false);
}

/////////////////////////////////////////////////////////////////

RowReader::RowReader(simCore::CsvReader& reader)
  : reader_(reader)
{
  // We care about comments for headers
  reader_.setCommentChar('\0');
  // SIM-14469, don't parse quotes
  reader_.setParseQuotes(false);
}

bool RowReader::eof() const
{
  return eof_;
}

int RowReader::readHeader()
{
  headerMap_.clear();
  const int rv = reader_.readLineTrimmed(headers_, false);

  size_t index = 0;
  for (std::string& header : headers_)
  {
    // Note that we're trimming the values in-place, editing the header value in vector
    header = simCore::StringUtils::trim(header);
    headerMap_[header] = index;
    ++index;
  }
  eof_ = (rv != 0);
  return rv;
}

int RowReader::readRow()
{
  const int rv = reader_.readLineTrimmed(row_, false);
  eof_ = (rv != 0);
  return rv;
}

size_t RowReader::numHeaders() const
{
  return headers_.size();
}

std::string RowReader::header(size_t colIndex) const
{
  if (colIndex < headers_.size())
    return headers_[colIndex];
  return {};
}

int RowReader::headerIndex(const std::string& key) const
{
  auto iter = headerMap_.find(key);
  return (iter == headerMap_.end()) ? -1 : static_cast<int>(iter->second);
}

const std::vector<std::string>& RowReader::headerTokens() const
{
  return headers_;
}

const std::vector<std::string>& RowReader::rowTokens() const
{
  return row_;
}

std::string RowReader::field(size_t colIndex) const
{
  if (colIndex < row_.size())
    return row_[colIndex];
  return {};
}

std::string RowReader::field(const std::string& key, const std::string& defaultValue) const
{
  auto iter = headerMap_.find(key);
  if (iter == headerMap_.end())
    return defaultValue;
  if (iter->second < row_.size())
    return row_[iter->second];
  return defaultValue;
}

double RowReader::fieldDouble(const std::string& key, double defaultValue) const
{
  auto iter = headerMap_.find(key);
  if (iter == headerMap_.end())
    return defaultValue;
  if (iter->second < row_.size())
    return atof(row_[iter->second].c_str());
  return defaultValue;
}

int RowReader::fieldInt(const std::string& key, int defaultValue) const
{
  auto iter = headerMap_.find(key);
  if (iter == headerMap_.end())
    return defaultValue;
  if (iter->second < row_.size())
    return atoi(row_[iter->second].c_str());
  return defaultValue;
}

std::string RowReader::operator[](size_t colIndex) const
{
  return field(colIndex);
}

std::string RowReader::operator[](const std::string& key) const
{
  return field(key);
}

}
