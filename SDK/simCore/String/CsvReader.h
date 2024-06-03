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
#ifndef SIMCORE_CSV_READER_H
#define SIMCORE_CSV_READER_H

#include <istream>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "simCore/Common/Common.h"

namespace simCore
{

/**
 * Simple CSV Reader class. Pass in an istream on construction and read each
 * line as needed using readLine(). This class is intended to mirror Python's
 * csv reader in that it allows forward-iteration through a csv file and gives
 * a vector of tokens for each line as it's read.
 */
class SDKCORE_EXPORT CsvReader
{
public:
  explicit CsvReader(std::istream& stream);
  virtual ~CsvReader();

  /**
   * Get the line number of the most recently read line. Line number is incremented
   * during line reading and never reset, so if the std::istream& supplied during class
   * construction is modified externally to this class, this line number might not be correct.
   * @return line number of most recently read line
   */
  size_t lineNumber() const;

  /** Set the char that denotes a comment line. Defaults to '#'. */
  void setCommentChar(char commentChar);

  /** Sets the delimiter between tokens, typically comma */
  void setDelimiterChar(char delim);
  /**
   * Sets the quote character; when a token starts with a quote character, it must end with
   * a quote character. Quotes inside a token are double quoted by default (Excel style), or
   * could be escaped with an escape token (not supported by this reader).
   */
  void setQuoteChar(char quote);

  /**
   * Read the next line of the stream into the given vector. Will always clear
   * the given vector. May skip completely empty lines, but will not skip lines
   * with only whitespace. Comment detection is supported and comment tokens outside
   * of quoted strings will be respected properly.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @param[in] skipEmptyLines  If true, will skip empty lines when reading. If
   *    false, will break on empty lines and return 0 with an empty tokens vector.
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLine(std::vector<std::string>& tokens, bool skipEmptyLines = true);

  /**
   * Reads the next line of the stream into the given vector. This method reads
   * identically to readLine(), but trims leading and trailing whitespace from
   * each token before returning.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @param[in] skipEmptyLines  If true, will skip empty lines when reading. If
   *    false, will break on empty lines and return 0 with an empty tokens vector.
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLineTrimmed(std::vector<std::string>& tokens, bool skipEmptyLines = true);

private:
  /** Consumes a single character from the input stream, returning empty on EOF/invalid */
  std::optional<char> readNext_();
  /**
   * Lowest, base level of reading a tokenized CSV line from the input stream. This
   * line might be more than one line of text, if tokens have newlines. Respects quotes
   * similar to Excel rules: internal quotes must be double quoted, and delimiters
   * inside quotes are treated as regular characters. This also respects the comment
   * token by reading until end of line, but only if the comment is encountered outside
   * of a quoted token (just like newline). An empty line returns an empty vector.
   * @param tokens Output tokens for reading
   * @return 0 on success, non-zero on error.
   */
  int readLineImpl_(std::vector<std::string>& tokens);

  /** Wrapper around readLineImpl_() that will run readLineImpl_() multiple times, to skip empty lines */
  int readLineSkippingEmptyLines_(std::vector<std::string>& tokens);

  std::istream& stream_;
  char commentChar_ = '#';
  char delimiter_ = ',';
  char escape_ = '\\';
  char quote_ = '"';
  size_t lineNumber_ = 0;
};

/** Convenience interface into a CsvReader that can read headers and reference fields by header name */
class SDKCORE_EXPORT RowReader
{
public:
  explicit RowReader(simCore::CsvReader& reader);
  SDK_DISABLE_COPY(RowReader);

  /** Returns true if readHeader/readRow failed, or if there is no CSV reader  (end of file) */
  bool eof() const;

  /** Read the row as a header, storing values for access with string-based field() calls, returning 0 on success. Consumes line. */
  int readHeader();
  /** Reads a row into memory. Returns 0 on success, non-zero on file error/completion. Consumes line. */
  int readRow();

  /** Returns number of headers known */
  size_t numHeaders() const;
  /** Gets the header name by index from last call to readHeader() */
  std::string header(size_t colIndex) const;

  /** Retrieves the field index, given a header string. Returns -1 if not found. */
  int headerIndex(const std::string& key) const;

  /** Returns the tokens in vector format for most recently read header (empty if readHeaders() not called). */
  const std::vector<std::string>& headerTokens() const;
  /** Returns the tokens in vector format for most recently read row. */
  const std::vector<std::string>& rowTokens() const;

  /** Gets a field from the most recent readRow() call. */
  std::string field(size_t colIndex) const;

  /** Returns the field from most recent readRow() from given column, identified by name */
  std::string field(const std::string& key, const std::string& defaultValue = "") const;
  /** Returns the field from most recent readRow() from given column, identified by name (double version) */
  double fieldDouble(const std::string& key, double defaultValue = 0.0) const;
  /** Returns the field from most recent readRow() from given column, identified by name (int version) */
  int fieldInt(const std::string& key, int defaultValue = 0) const;

  /** Convenience operator for accessing keys by index */
  std::string operator[](size_t colIndex) const;
  /** Convenience operator for accessing keys by name */
  std::string operator[](const std::string& key) const;

private:
  /// CSV Reader may be null depending on construction method
  simCore::CsvReader& reader_;
  std::vector<std::string> row_;

  std::vector<std::string> headers_;
  std::map<std::string, size_t> headerMap_;

  bool eof_ = true;
};

}

#endif /* SIMCORE_CSV_READER_H */
