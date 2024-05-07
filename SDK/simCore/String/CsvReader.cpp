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
#include <algorithm>
#include <cassert>
#include "simCore/String/Format.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simCore/String/CsvReader.h"

namespace simCore
{

CsvReader::CsvReader(std::istream& stream)
  : stream_(stream)
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

void CsvReader::setDelimiterChar(char delim)
{
  delimiter_ = delim;
}

void CsvReader::setQuoteChar(char quote)
{
  quote_ = quote;
}

std::optional<char> CsvReader::readNext_()
{
  if (!stream_)
    return {};
  char ch = '\0';
  if (!stream_.read(&ch, 1))
    return {};
  return ch;
}

int CsvReader::readLine(std::vector<std::string>& tokens, bool skipEmptyLines)
{
  if (skipEmptyLines)
    return readLineSkippingEmptyLines_(tokens);
  return readLineImpl_(tokens);
}

int CsvReader::readLineSkippingEmptyLines_(std::vector<std::string>& tokens)
{
  while (readLineImpl_(tokens) == 0)
  {
    // Return success only if the line is non-empty
    if (tokens.size() > 0)
      return 0;
  }
  // readLineImpl_() returned non-zero, so we do too
  return 1;
}

int CsvReader::readLineImpl_(std::vector<std::string>& tokens)
{
  tokens.clear();

  // Algorithm adapted from https://stackoverflow.com/questions/843997

  auto ch = readNext_();
  // Skip linefeed characters
  while (ch.has_value() && ch == '\r')
    ch = readNext_();
  // Invalid read, done
  if (!ch.has_value())
    return 1;
  ++lineNumber_;

  std::string currentToken;
  bool insideQuote = false;
  bool started = false;

  while (ch.has_value())
  {
    // Special processing if inside quote
    if (insideQuote)
    {
      started = true;
      if (*ch == quote_)
        insideQuote = false;
      else
        currentToken.append(1, *ch);
      ch = readNext_();
      continue;
    }

    if (*ch == quote_)
    {
      insideQuote = true;
      // Handle double quote inside
      if (started)
        currentToken += std::string(1, quote_);
    }
    else if (*ch == delimiter_)
    {
      tokens.push_back(currentToken);
      currentToken.clear();
      started = false;
    }
    else if (*ch == '\r')
    {
      // noop
    }
    else if (*ch == '\n')
    {
      // end of line, break out of loop
      break;
    }
    else if (*ch == commentChar_)
    {
      // Treat like end of line, and read until end of line
      while (ch.has_value() && *ch != '\n')
        ch = readNext_();
      break;
    }
    else // save character
      currentToken.append(1, *ch);
    ch = readNext_();
  }

  // Only save empty token, if ending in a delimiter (i.e. tokens is non-empty)
  if (!currentToken.empty() || !tokens.empty())
    tokens.push_back(currentToken);
  return 0;
}

int CsvReader::readLineTrimmed(std::vector<std::string>& tokens, bool skipEmptyLines)
{
  const int rv = readLine(tokens, skipEmptyLines);
  if (rv != 0)
    return rv;

  // Remove leading and trailing whitespace from all tokens
  std::transform(tokens.cbegin(), tokens.cend(), tokens.begin(), [](const std::string& str) {
    return simCore::StringUtils::trim(str); });
  return 0;
}

/////////////////////////////////////////////////////////////////

RowReader::RowReader(simCore::CsvReader& reader)
  : reader_(reader)
{
  // We care about comments for headers
  reader_.setCommentChar('\0');
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
