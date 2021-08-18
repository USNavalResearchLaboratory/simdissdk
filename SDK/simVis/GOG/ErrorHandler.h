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
#ifndef SIMVIS_GOG_ERRORHANDLER_H
#define SIMVIS_GOG_ERRORHANDLER_H

#include <string>

namespace simVis { namespace GOG {

/** Implement your own ErrorHandler for custom error message reporting or output.  @see simVis::Parser::setErrorHandler(). */
class ErrorHandler
{
public:
  virtual ~ErrorHandler() {}

  /** Print a warning about the GOG being parsed */
  virtual void printWarning(size_t lineNumber, const std::string& warningText) = 0;
  /** Print an error about the GOG being parsed */
  virtual void printError(size_t lineNumber, const std::string& errorText) = 0;
};

} }

#endif /* SIMVIS_GOG_ERRORHANDLER_H */
