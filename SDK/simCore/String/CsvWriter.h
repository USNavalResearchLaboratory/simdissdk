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
#ifndef SIMCORE_CSVWRITER_H
#define SIMCORE_CSVWRITER_H

#include <iostream>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"

namespace simCore {

/** Responsible for writing out string vectors as CSV content */
class SDKCORE_EXPORT CsvWriter
{
public:
  /**
   * Constructs a new CsvWriter with the given output stream
   * @param os Output stream that all CSV content will be written to
   */
  explicit CsvWriter(std::ostream& os);

  /** Changes the delimiter between tokens. Typically ',' */
  void setDelimiterChar(char delim);

  /**
   * Sets the quote character, for fields containing special characters or newlines. This
   * character is used around tokens when a newline, delimiter, or quote character is
   * present in the token. This defaults to '"'.
   * @param quote Character for quoting tokens
   */
  void setQuoteChar(char quote);

  /**
   * Sets the escape character to escape the quote character when double-quote is false. This
   * is only used if double quote is off (double quote defaults on). When double-quote is off,
   * escape characters inside tokens must be escaped to avoid ambiguity. When double-quote
   * is on, escape characters inside tokens are left as-is since there is no escaping.
   * @param escape Character to prepend for escaping quote (and itself), only if double-quote off
   */
  void setEscapeChar(char escape);

  /**
   * If true, double internal quote characters. If false, escape internal quote characters.
   * This defaults to true, which is compatible with MS Excel. False is not compatible with Excel.
   * When double-quote mode is OFF, escape character is used for quotes. This means that also
   * the escape character in original strings get escaped. Escape characters in original string
   * @param doubleQuote If true, internal quotes are doubled. If false, internal quotes are
   *   escaped with the escape character.
   */
  void setDoubleQuote(bool doubleQuote);

  /**
   * Writes a vector of tokens into CSV format using the formatting options above. The output
   * will always end in a newline. The default configuration of the CSV Writer matches the
   * Python CSV Writer, and is compatible with loading files in Microsoft Excel.
   * @param vec Tokens to write to the output stream.
   */
  void write(const std::vector<std::string>& vec) const;

private:
  /** Writes a single token out to the stream. */
  void writeToken_(const std::string& token) const;

  std::ostream& os_;
  char delimiter_ = ',';
  char escape_ = '\\';
  char quote_ = '"';
  bool doubleQuote_ = true;
  bool alwaysEscapeEscapes_ = false;
};

}

#endif /* SIMCORE_CSVWRITER_H */
