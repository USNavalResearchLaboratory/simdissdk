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
#include "simCore/String/Utils.h"
#include "simCore/String/CsvWriter.h"

namespace simCore {

CsvWriter::CsvWriter(std::ostream& os)
  : os_(os)
{
}

void CsvWriter::setDelimiterChar(char delim)
{
  delimiter_ = delim;
}

void CsvWriter::setEscapeChar(char escape)
{
  escape_ = escape;
}

void CsvWriter::setQuoteChar(char quote)
{
  quote_ = quote;
}

void CsvWriter::setDoubleQuote(bool doubleQuote)
{
  doubleQuote_ = doubleQuote;
}

void CsvWriter::write(const std::vector<std::string>& vec) const
{
  bool firstTime = true;
  for (const auto& str : vec)
  {
    if (!firstTime)
      os_ << delimiter_;
    writeToken_(str);
    firstTime = false;
  }
  os_ << "\n";
}

void CsvWriter::writeToken_(const std::string& token) const
{
  const bool needsQuotes = (token.find('\n') != std::string::npos)
    || (token.find(delimiter_) != std::string::npos)
    || (token.find(quote_) != std::string::npos);
  if (needsQuotes)
    os_ << quote_;

  const std::string escapeStr(1, escape_);
  const std::string quoteStr(1, quote_);

  if (doubleQuote_)
  {
    // Need to replace single quotes with double. No escapes, so no need to escape any escapes
    os_ << simCore::StringUtils::substitute(token, quoteStr, quoteStr + quoteStr, true);
  }
  else
  {
    // Double quote needs to escape pre-existing escape sequences first
    std::string finalString = simCore::StringUtils::substitute(token, escapeStr, escapeStr + escapeStr, true);
    // Next need to replace quotes with escaped quotes
    os_ << simCore::StringUtils::substitute(finalString, quoteStr, escapeStr + quoteStr, true);
  }

  if (needsQuotes)
    os_ << quote_;
}

}
