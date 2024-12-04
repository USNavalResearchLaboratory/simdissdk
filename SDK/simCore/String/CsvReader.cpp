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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
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

/**
 * Internal wrapper for std::istream, to buffer reads so that we can read whole lines
 * from the stream for our buffer, rather than reading one character at a time. On
 * at least MSVC, the std::istream::read() of 1 byte is quite slow and this buffering
 * drastically improves performance.
 */
class CsvReader::BufferedReader
{
public:
  explicit BufferedReader(std::istream& is)
    : stream_(is)
  {
  }

  /** Mirror for std::istream::good() that considers buffer validity */
  bool good() const
  {
    // If stream is good, we can keep reading. If there are bytes still
    // in the buffer, we also can keep reading.
    return stream_.good() || bufferPos_ < buffer_.size();
  }

  /** Returns position within the current line of the next character to be read */
  size_t bufferPosition() const
  {
    return bufferPos_;
  }

  /** Returns false on stream error; reads a single character */
  bool read(char* ch)
  {
    size_t bufferSize = buffer_.size();
    if (bufferPos_ >= bufferSize)
    {
      // Read next line
      bufferPos_ = 0;
      // Break early on getline failure
      if (!std::getline(stream_, buffer_))
        return false;
      // Append a newline for the reader; strip the CRLF for it if it's present, else append
      if (!buffer_.empty() && buffer_[buffer_.size() - 1] == '\r')
        buffer_[buffer_.size() - 1] = '\n';
      else
        buffer_.append(1, '\n');
      bufferSize = buffer_.size();

    }
    // Not possible here to have bufferPos_ past buffer size
    assert(bufferPos_ < bufferSize);

    *ch = buffer_[bufferPos_];
    ++bufferPos_;
    return true;
  }

private:
  std::istream& stream_;

  /** Buffer, which is only ever empty on construction, or error. */
  std::string buffer_;
  /** Position in the buffer for next byte to read. */
  size_t bufferPos_ = 0;
};

////////////////////////////////////////////////////////

CsvReader::CsvReader(std::istream& stream)
  : buffer_(std::make_unique<BufferedReader>(stream))
{
}

CsvReader::~CsvReader()
{
}

size_t CsvReader::lineNumber() const
{
  return lineNumber_;
}

std::string CsvReader::lineText() const
{
  return lineText_;
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

void CsvReader::setLimitReadToSingleLine(bool singleLine)
{
  limitToSingleLine_ = singleLine;
}

void CsvReader::setAllowMidlineComments(bool allow)
{
  allowMidlineComments_ = allow;
}

std::optional<char> CsvReader::readNext_()
{
  if (!buffer_->good())
    return {};
  char ch = '\0';
  if (!buffer_->read(&ch))
    return {};

  lineText_ += ch;
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
  lineText_.clear();
  tokens.clear();
  // Algorithm adapted from https://stackoverflow.com/questions/843997

  auto ch = readNext_();
  // Skip linefeed characters
  while (ch.has_value() && ch == '\r')
    ch = readNext_();
  // Invalid read, done
  if (!ch.has_value())
    return 1;
  lineNumber_ += linesFoundInRead_;
  // reset lines found in read now that new read is starting
  linesFoundInRead_ = 1;

  std::string currentToken;
  // Whether the entire current token is enclosed in quotes. Set true if first char is a quote, set false when encountering any character after closing the quote
  bool wholeTokenQuoted = false;
  // Whether current character processing is in a quoted string. Only possible if token started with a quote (wholeTokenQuoted == true)
  bool insideQuote = false;
  // Whether a character has been read after the initial quote. Ensures that in cases of internal quotes "Test"" the extra quote character is added back to the current token
  bool started = false;

  while (ch.has_value())
  {
    // Special processing if inside quote
    if (insideQuote)
    {
      // No way to do special quote tokenization unless the token is quoted. If it's not
      // quoted, then we just treat quote like any other character.
      assert(wholeTokenQuoted);

      if (*ch == '\n')
      {
        if (limitToSingleLine_)
          break;
        // keep line number updated correctly
        ++linesFoundInRead_;
      }

      started = true;
      if (*ch == quote_)
        insideQuote = false;
      else
        currentToken.append(1, *ch);
      ch = readNext_();
      continue;
    }

    // Whole-token quoting can be stopped, with the close of the quote. This catches cases like:
    //   a,"quote " ends early,c
    // Where token[1] should be "quote  ends early"
    if (wholeTokenQuoted && *ch != quote_)
      wholeTokenQuoted = false;

    if (*ch == quote_)
    {
      if (currentToken.empty())
        wholeTokenQuoted = true;

      if (wholeTokenQuoted)
      {
        insideQuote = true;
        // Handle double quote inside
        if (started)
          currentToken += std::string(1, quote_);
      }
      else
      {
        // Quote is inside the string, like in the second token of:
        //    a, quote " here, c
        // So we just append the quote
        currentToken.append(1, *ch);
      }
    }
    else if (*ch == delimiter_)
    {
      tokens.push_back(currentToken);
      currentToken.clear();
      started = false;
      wholeTokenQuoted = false;
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
      // If we encounter a commentChar_ mid-line and don't allow comments, then add it to token and move on
      if (buffer_->bufferPosition() > 1 && !allowMidlineComments_)
      {
        currentToken.append(1, *ch);
        ch = readNext_();
        continue;
      }

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
  while (1)
  {
    const int rv = readLine(tokens, skipEmptyLines);
    if (rv != 0)
      return rv;

    // Remove leading and trailing whitespace from all tokens
    std::transform(tokens.cbegin(), tokens.cend(), tokens.begin(), [](const std::string& str) {
      return simCore::StringUtils::trim(str); });

    // If there is only one token and it's empty, we need to clear the token
    if (tokens.size() == 1 && tokens[0].empty())
      tokens.clear();
    // If we skip empty lines, and this was an empty line, then we keep going
    if (!skipEmptyLines || !tokens.empty())
      break;
  }
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
    const std::string& lower = simCore::lowerCase(header);
    headerMap_[lower] = index;
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
  auto iter = headerMap_.find(simCore::lowerCase(key));
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
  auto iter = headerMap_.find(simCore::lowerCase(key));
  if (iter == headerMap_.end())
    return defaultValue;
  if (iter->second < row_.size())
    return row_[iter->second];
  return defaultValue;
}

double RowReader::fieldDouble(const std::string& key, double defaultValue) const
{
  auto iter = headerMap_.find(simCore::lowerCase(key));
  if (iter == headerMap_.end())
    return defaultValue;
  if (iter->second < row_.size())
    return atof(row_[iter->second].c_str());
  return defaultValue;
}

int RowReader::fieldInt(const std::string& key, int defaultValue) const
{
  auto iter = headerMap_.find(simCore::lowerCase(key));
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
