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
void RegExpImpl::setPattern(const std::string& pattern)
{
  exp_ = pattern;
  if (fastRegex_)
    fastRegex_->setPattern(QString::fromStdString(pattern));
  if (qRegExp_)
    qRegExp_->setPattern(QString::fromStdString(pattern));
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
  if (fastRegex_)
  {
    QFlags<QRegularExpression::PatternOption> newOpts = (caseSensitivity_ == CaseSensitive) ?
      (fastRegex_->patternOptions() & ~QRegularExpression::CaseInsensitiveOption) : (fastRegex_->patternOptions() | QRegularExpression::CaseInsensitiveOption);
    fastRegex_->setPatternOptions(newOpts);
  }
  if (qRegExp_)
  {
    qRegExp_->setCaseSensitivity(qtCaseSensitivity(caseSensitivity));
  }

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

  PatternSyntax oldSyntax = patternSyntax_;
  patternSyntax_ = patternSyntax;

  // Going from not RegExp to RegExp requires reinitialization
  if (patternSyntax_ == RegExp)
  {
    delete fastRegex_;
    initializeQRegExp_();
  }
  // Going from RegExp to not RegExp requires reinitialization
  else if (oldSyntax == RegExp)
  {
    delete qRegExp_;
    initializeQRegExp_();
  }
  else if (qRegExp_)
    qRegExp_->setPatternSyntax(qtPatternSyntax(patternSyntax));
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
QRegExp::PatternSyntax RegExpImpl::qtPatternSyntax(PatternSyntax simQtPatternSyntax)
{
  switch (simQtPatternSyntax)
  {
  case RegExp :
    return QRegExp::RegExp;
  case Wildcard :
    return QRegExp::Wildcard;
  case FixedString :
    return QRegExp::FixedString;
  }

  // To satisfy warnings.  Should never be reached
  return QRegExp::RegExp;
}

//------------------------------------------------------------
RegExpImpl::PatternSyntax RegExpImpl::simQtPatternSyntax(QRegExp::PatternSyntax qtPatternSyntax)
{
  switch (qtPatternSyntax)
  {
  case QRegExp::RegExp2 :
    SIM_WARN << "Pattern syntax \"RegExp2\" unsupported for simQt::RegExpImpl.  Defaulting to \"RegExp\"";
    return RegExp;
  case QRegExp::W3CXmlSchema11:
    SIM_WARN << "Pattern syntax \"W3CXmlSchema11\" unsupported for simQt::RegExpImpl.  Defaulting to \"RegExp\"";
    return RegExp;
  case QRegExp::RegExp :
    return RegExp;
  case QRegExp::WildcardUnix:
    SIM_WARN << "Pattern syntax \"WildcardUnix\" unsupported for simQt::RegExpImpl.  Defaulting to \"Wildcard\"";
    return Wildcard;
  case QRegExp::Wildcard :
    return Wildcard;
  case QRegExp::FixedString :
    return FixedString;
  }

  // To satisfy warnings.  Should never be reached
  return RegExp;
}

//------------------------------------------------------------
void RegExpImpl::initializeQRegExp_()
{
  // Store case sensitivity in a Qt construct
  Qt::CaseSensitivity sense = qtCaseSensitivity(caseSensitivity_);

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
    if (fastRegex_ && fastRegex_->isValid())
      return fastRegex_->match(QString::fromStdString(test)).hasMatch();
    break;

  case FixedString:
    // Failure means problem in initializeQRegExp_()
    assert(qRegExp_);
    if (qRegExp_ && qRegExp_->isValid())
      return qRegExp_->indexIn(QString::fromStdString(test)) >= 0;
    break;

  case Wildcard:
    // Failure means problem in initializeQRegExp_()
    assert(qRegExp_);
    if (qRegExp_ && qRegExp_->isValid())
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
