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

#include <fstream>
#include <vector>
#include "simCore/Common/Common.h"

namespace simCore
{

/**
 * Simple CSV Reader class. Open a file using open() then read each line as
 * needed using readLine(). This class is intended to mirror Python's csv
 * reader in that it allows forward-iteration through a csv file and gives
 * a vector of tokens for each line as it's read. If functionality similar
 * to Python's csv DictReader is desired, a new class will be needed.
 */
class SDKCORE_EXPORT CsvReader
{
public:
  CsvReader();
  virtual ~CsvReader();

  /** Set the char that denotes a comment line. Defaults to '#'. */
  void setCommentChar(char commentChar);

  /**
   * Open the given filename for reading as CSV.
   * @param[in] filename  Name of the file to open
   * @return  0 on success, non-zero otherwise
   */
  int open(const std::string& filename);
  /** Close the open file, if any. Called on destruction of this class. */
  void close();

  /**
   * Read the next line of the file into the given vector. Will always clear
   * the given vector. Will skip empty lines and lines that start with the
   * configured comment char. Note that comment detection is rudimentary.
   * Inline comments or indented comments will not be detected.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLine(std::vector<std::string>& tokens);
  /**
   * Reads the next line of the file into the given vector. This method reads
   * identically to readLine(), but trims leading and trailing whitespace from
   * each token before returning.
   * @param[out] tokens  Vector filled with tokens from the next line
   * @return 0 on successful line read, 1 when the end of the file is reached
   */
  int readLineTrimmed(std::vector<std::string>& tokens);

private:
  std::ifstream file_;
  char commentChar_;
};

}
#endif /* SIMCORE_CSV_READER_H */
