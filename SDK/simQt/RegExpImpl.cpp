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
#include <cassert>
#include <QRegularExpression>
#include "simNotify/Notify.h"
#include "simQt/RegExpImpl.h"

namespace simQt {

//------------------------------------------------------------
RegExpImpl::RegExpImpl(const std::string& exp, CaseSensitivity caseSense, PatternSyntax patternSyntax)
  : givenExpression_(QString::fromStdString(exp)),
    caseSensitivity_(caseSense),
    patternSyntax_(patternSyntax)
{
  initializeQRegExp_();
}

//------------------------------------------------------------
RegExpImpl::RegExpImpl(const RegExpImpl& other)
  : givenExpression_(other.givenExpression_),
    caseSensitivity_(other.caseSensitivity_),
    patternSyntax_(other.patternSyntax_)
{
  initializeQRegExp_();
}

//------------------------------------------------------------
RegExpImpl::~RegExpImpl()
{
}

//------------------------------------------------------------
bool RegExpImpl::match(const std::string& test) const
{
  return matchQt_(test); // currently using Qt
}

//------------------------------------------------------------
std::string RegExpImpl::pattern() const
{
  return givenExpression_.toStdString();
}

//------------------------------------------------------------
void RegExpImpl::setPattern(const std::string& pattern)
{
  givenExpression_ = QString::fromStdString(pattern);
  actualExpression_ = getActual_(givenExpression_);
  fastRegex_->setPattern(actualExpression_);
}

//------------------------------------------------------------
RegExpImpl::CaseSensitivity RegExpImpl::caseSensitivity() const
{
  return caseSensitivity_;
}

//------------------------------------------------------------
void RegExpImpl::setCaseSensitivity(RegExpImpl::CaseSensitivity caseSensitivity)
{
  caseSensitivity_ = caseSensitivity;

  QFlags<QRegularExpression::PatternOption> newOpts = (caseSensitivity_ == CaseSensitive) ?
    (fastRegex_->patternOptions() & ~QRegularExpression::CaseInsensitiveOption) : (fastRegex_->patternOptions() | QRegularExpression::CaseInsensitiveOption);
  fastRegex_->setPatternOptions(newOpts);
}

//------------------------------------------------------------
RegExpImpl::PatternSyntax RegExpImpl::patternSyntax() const
{
  return patternSyntax_;
}

//------------------------------------------------------------
void RegExpImpl::setPatternSyntax(RegExpImpl::PatternSyntax patternSyntax)
{
  if (patternSyntax_ == patternSyntax)
    return;

  patternSyntax_ = patternSyntax;
  initializeQRegExp_();
}

//------------------------------------------------------------
bool RegExpImpl::isValid() const
{
  return fastRegex_->isValid();
}

//------------------------------------------------------------
std::string RegExpImpl::errors() const
{
  return fastRegex_->errorString().toStdString();
}

//------------------------------------------------------------
Qt::CaseSensitivity RegExpImpl::qtCaseSensitivity(CaseSensitivity simQtCaseSensitivity)
{
  switch (simQtCaseSensitivity)
  {
  case CaseSensitive :
    return Qt::CaseSensitive;
  case CaseInsensitive :
    return Qt::CaseInsensitive;
  }

  // To satisfy warnings.  Should never be reached
  return Qt::CaseSensitive;
}

//------------------------------------------------------------
RegExpImpl::CaseSensitivity RegExpImpl::simQtCaseSensitivity(Qt::CaseSensitivity qtCaseSensitivity)
{
  switch (qtCaseSensitivity)
  {
  case Qt::CaseSensitive :
    return CaseSensitive;
  case Qt::CaseInsensitive :
    return CaseInsensitive;
  }

  // To satisfy warnings.  Should never be reached
  return CaseSensitive;
}


//------------------------------------------------------------
void RegExpImpl::initializeQRegExp_()
{
  actualExpression_ = getActual_(givenExpression_);

  // Store case sensitivity in a Qt construct
  Qt::CaseSensitivity sense = qtCaseSensitivity(caseSensitivity_);

  QRegularExpression::PatternOptions patternOptions = QRegularExpression::DontCaptureOption;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
  patternOptions |= QRegularExpression::OptimizeOnFirstUsageOption;
#endif
  if (caseSensitivity_ == CaseInsensitive)
    patternOptions |= QRegularExpression::CaseInsensitiveOption;

  fastRegex_ = std::make_unique<QRegularExpression>(actualExpression_, patternOptions);

}

//------------------------------------------------------------
bool RegExpImpl::matchQt_(const std::string& test) const
{
  if ((actualExpression_.isEmpty()) || (actualExpression_ == ".*"))
    return true;

  if (!fastRegex_->isValid())
    return false;

  return fastRegex_->match(QString::fromStdString(test)).hasMatch();
}

QString RegExpImpl::getActual_(const QString& given) const
{
  switch (patternSyntax_)
  {
  case RegExp:
    return given;

  case Wildcard:
    return QRegularExpression::wildcardToRegularExpression(given);

  case FixedString:
    return QRegularExpression::escape(given);
  }

  // switch statement needs to be update
  assert(false);
  return given;
}

//------------------------------------------------------------
RegExpFilterFactoryImpl::RegExpFilterFactoryImpl()
{}

RegExpFilterFactoryImpl::~RegExpFilterFactoryImpl()
{}

simData::RegExpFilterPtr RegExpFilterFactoryImpl::createRegExpFilter(const std::string& expression)
{
  RegExpImpl* newRegExp = new RegExpImpl(expression);
  // check that regular expression is valid, return an empty ptr otherwise
  if (!newRegExp->isValid())
  {
    SIM_ERROR << "Failed to create regular expression: " << newRegExp->errors() << "\n";
    delete newRegExp;
    newRegExp = nullptr;
  }
  return simData::RegExpFilterPtr(newRegExp);
}
}
