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
#ifndef SIMQT_REGEXPIMPL_H
#define SIMQT_REGEXPIMPL_H

#include <string>
#include <QRegExp>
#include "simCore/Common/Common.h"
#include "simData/CategoryData/CategoryFilter.h"

#ifdef _MSC_VER // [
#pragma warning(push)
// Disable C4275: non-DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#pragma warning(disable : 4275)
#endif // _MSC_VER ]

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
  explicit RegExpImpl(const std::string& exp, CaseSensitivity caseSense = CaseInsensitive, PatternSyntax patternSyntax = RegExp);

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
  * Sets the regex pattern string
  * @param[in] pattern new regex pattern string
  */
  void setPattern(const std::string& pattern);

  /**
  * Returns the regex case sensitivity
  * @return the regex case sensitivity
  */
  CaseSensitivity caseSensitivity() const;

  /**
  * Sets the regex case sensitivity
  * @param[in] caseSensitivity new case sensitivity
  */
  void setCaseSensitivity(CaseSensitivity caseSensitivity);

  /**
  * Returns the regex pattern syntax
  * @return the regex pattern syntax
  */
  PatternSyntax patternSyntax() const;

  /**
  * Sets the regex pattern syntax
  * @param[in] patternSyntax new pattern syntax
  */
  void setPatternSyntax(PatternSyntax patternSyntax);

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

  /** Static function to convert values of the simQt CaseSensitivity enum to values of the Qt CaseSensitivity enum*/
  static Qt::CaseSensitivity qtCaseSensitivity(CaseSensitivity simQtCaseSensitivity);
  /** Static function to convert values of the Qt CaseSensitivity enum to values of the simQt CaseSensitivity enum*/
  static CaseSensitivity simQtCaseSensitivity(Qt::CaseSensitivity qtCaseSensitivity);

  /** Static function to convert values of the simQt PatternSyntax enum to values of the Qt PatternSyntax enum*/
  static QRegExp::PatternSyntax qtPatternSyntax(PatternSyntax simQtPatternSyntax);
  /** Static function to convert values of the Qt PatternSyntax enum to values of the simQt PatternSyntax enum*/
  static PatternSyntax simQtPatternSyntax(QRegExp::PatternSyntax qtPatternSyntax);

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

#ifdef _MSC_VER // [
#pragma warning(pop)
#endif // _MSC_VER ]

#endif
