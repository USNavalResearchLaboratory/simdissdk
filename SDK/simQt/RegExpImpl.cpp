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
#include <QRegExp>
#include <QRegularExpression>
#include "simNotify/Notify.h"
#include "RegExpImpl.h"

namespace simQt {

//------------------------------------------------------------
RegExpImpl::RegExpImpl(const std::string& exp, CaseSensitivity caseSense, PatternSyntax patternSyntax)
  : exp_(exp),
  caseSensitivity_(caseSense),
  patternSyntax_(patternSyntax),
  qRegExp_(NULL),
  fastRegex_(NULL)
{
  initializeQRegExp_();
}

//------------------------------------------------------------
RegExpImpl::RegExpImpl(const RegExpImpl& other)
  : exp_(other.exp_),
  caseSensitivity_(other.caseSensitivity_),
  patternSyntax_(other.patternSyntax_),
  qRegExp_(NULL),
  fastRegex_(NULL)
{
  initializeQRegExp_();
}

//------------------------------------------------------------
RegExpImpl::~RegExpImpl()
{
  delete fastRegex_;
  delete qRegExp_;
}

//------------------------------------------------------------
bool RegExpImpl::match(const std::string& test) const
{
  return matchQt_(test); // currently using Qt
}

//------------------------------------------------------------
std::string RegExpImpl::pattern() const
{
  return exp_;
}

//------------------------------------------------------------
bool RegExpImpl::isValid() const
{
  if (fastRegex_ != NULL)
    return fastRegex_->isValid();
  if (qRegExp_ != NULL)
    return qRegExp_->isValid();
  return false;
}

//------------------------------------------------------------
std::string RegExpImpl::errors() const
{
  if (fastRegex_ != NULL)
    return fastRegex_->errorString().toStdString();
  if (qRegExp_ != NULL)
    return qRegExp_->errorString().toStdString();
  return "";
}

//------------------------------------------------------------
void RegExpImpl::initializeQRegExp_()
{
  // Store case sensitivity in a Qt construct
  Qt::CaseSensitivity sense = Qt::CaseInsensitive;
  if (caseSensitivity_ == CaseSensitive)
    sense = Qt::CaseSensitive;

  // Create different classes based on pattern syntax
  switch (patternSyntax_)
  {
  case RegExp:
  {
    QRegularExpression::PatternOptions patternOptions = QRegularExpression::DontCaptureOption | QRegularExpression::OptimizeOnFirstUsageOption;
    if (caseSensitivity_ == CaseInsensitive)
      patternOptions |= QRegularExpression::CaseInsensitiveOption;
    fastRegex_ = new QRegularExpression(QString::fromStdString(exp_), patternOptions);
    if (!fastRegex_->isValid())
    {
      delete fastRegex_;
      fastRegex_ = NULL;
    }
    break;
  }

  case FixedString:
    qRegExp_ = new QRegExp(QString::fromStdString(exp_), sense, QRegExp::FixedString);
    break;

  case Wildcard:
    qRegExp_ = new QRegExp(QString::fromStdString(exp_), sense, QRegExp::Wildcard);
    break;
  }
}

//------------------------------------------------------------
bool RegExpImpl::matchQt_(const std::string& test) const
{
  if (exp_.empty())
    return true;

  switch (patternSyntax_)
  {
  case RegExp:
    if (exp_ == ".*")
      return true;
    if (fastRegex_)
      return fastRegex_->match(QString::fromStdString(test)).hasMatch();
    break;

  case FixedString:
    // Failure means problem in initializeQRegExp_()
    assert(qRegExp_);
    if (qRegExp_)
      return qRegExp_->indexIn(QString::fromStdString(test)) >= 0;
    break;

  case Wildcard:
    // Failure means problem in initializeQRegExp_()
    assert(qRegExp_);
    if (qRegExp_)
      return qRegExp_->exactMatch(QString::fromStdString(test));
    break;
  }
  return false;
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
    newRegExp = NULL;
  }
  return simData::RegExpFilterPtr(newRegExp);
}
}
