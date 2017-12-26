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
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* U.S. Naval Research Laboratory.
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*
*/
#ifndef SIMQT_REGEXPIMPL_H
#define SIMQT_REGEXPIMPL_H

#include <string>
#include "simCore/Common/Export.h"
#include "simData/CategoryData/CategoryFilter.h"

class QRegExp;
class QRegularExpression;

namespace simQt {

/**
* Class to abstract away the regular expression matching functionality
*/
class SDKQT_EXPORT RegExpImpl : public simData::RegExpFilter
{
public:

  /**
  * Enum representing case sensitivity of pattern matching
  */
  enum CaseSensitivity
  {
    CaseInsensitive,
    CaseSensitive
  };

  /**
  * Enum representing different styles of string matching
  *
  * RegExp - A rich Perl-like pattern matching syntax.
  * Wildcard - This provides a simple pattern matching syntax similar to that used by shells (command interpreters) for "file globbing"
  * FixedString - The pattern is a fixed string. This is equivalent to using the RegExp pattern on a string in which all metacharacters are escaped.
  */
  enum PatternSyntax
  {
    RegExp,
    Wildcard,
    FixedString
  };

  /** Constructor */
  RegExpImpl(const std::string& exp, CaseSensitivity caseSense = CaseInsensitive, PatternSyntax patternSyntax = RegExp);

  /** Copy Constructor */
  RegExpImpl(const RegExpImpl& other);

  /** Destructor */
  virtual ~RegExpImpl();

  /**
  * Returns true if the test string matches anything in the regular expression
  * @param[in] test String to test
  * @return true if test string matches
  */
  virtual bool match(const std::string& test) const;

  /**
  * Returns the regex pattern string
  * @return the regex pattern
  */
  virtual std::string pattern() const;

  /**
  * Returns true if the expression passed in defines a valid regular expression
  * @return true if the regular expression is valid, false otherwise
  */
  bool isValid() const;

  /**
  * Returns any errors that may have occurred while constructing the regular expression object
  * @return string explaining errors with the regular expression, empty string if no errors
  */
  std::string errors() const;

private:

  /// private initializer for QRegExp
  void initializeQRegExp_();

  /// private implementation of QRegExp
  bool matchQt_(const std::string& test) const;

  std::string exp_;
  CaseSensitivity caseSensitivity_;
  PatternSyntax patternSyntax_;
  QRegExp* qRegExp_;
  QRegularExpression* fastRegex_;
};

/** Implements a RegExpFilterFactory to create Qt based RegExpFilter objects */
class SDKQT_EXPORT RegExpFilterFactoryImpl : public simData::RegExpFilterFactory
{
public:
  RegExpFilterFactoryImpl();
  virtual ~RegExpFilterFactoryImpl();
  virtual simData::RegExpFilterPtr createRegExpFilter(const std::string& expression);
};

}

#endif
