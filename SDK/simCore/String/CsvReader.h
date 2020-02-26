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
#ifndef SIMCORE_CSV_READER_H
#define SIMCORE_CSV_READER_H

#include <istream>
#include <vector>
#include "simCore/Common/Common.h"

namespace simCore
{

/**
 * Simple CSV Reader class. Pass in an istream on construction and read each
 * line as needed using readLine(). This class is intended to mirror Python's
 * csv reader in that it allows forward-iteration through a csv file and gives
 * a vector of tokens for each line as it's read. If functionality similar
 * to Python's csv DictReader is desired, a new class will be needed.
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

  /**
   * Read the next line of the stream into the given vector. Will always clear
   * the given vector. Will skip empty lines and lines that start with the
   * configured comment char. Note that comment detection is rudimentary.
   * Inline comments or indented comments will not be detected.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLine(std::vector<std::string>& tokens);
  /**
   * Reads the next line of the stream into the given vector. This method reads
   * identically to readLine(), but trims leading and trailing whitespace from
   * each token before returning.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLineTrimmed(std::vector<std::string>& tokens);

private:
  std::istream& stream_;
  char commentChar_;
  size_t lineNumber_;
};

}
#endif /* SIMCORE_CSV_READER_H */
