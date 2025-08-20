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
#if QT_VERSION_MAJOR == 5
#include <QRegExp>
#else
#include <QRegularExpression>
#endif
#include "simNotify/Notify.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/RegExpImpl.h"
#include "simQt/EntityNameFilter.h"

namespace simQt {

// key for the regular expression stored in settings
static const QString REGEXP_SETTING = "RegExp";
static const QString REGULAR_EXPRESSION_PATTERN_SETTING = "RegularExpressionPattern";
static const QString REGULAR_EXPRESSION_SENSITIVITY_SETTING = "RegularExpressionSensitivity";
static const QString REGULAR_EXPRESSION_SYNTAX_SETTING = "RegularExpressionSyntax";

#if QT_VERSION_MAJOR == 5
RegExpImpl::PatternSyntax simQtPatternSyntax(QRegExp::PatternSyntax qtPatternSyntax)
{
  switch (qtPatternSyntax)
  {
  case QRegExp::RegExp2:
    SIM_WARN << "Pattern syntax \"RegExp2\" unsupported for simQt::RegExpImpl.  Defaulting to \"RegExp\"";
    return RegExpImpl::RegExp;
  case QRegExp::W3CXmlSchema11:
    SIM_WARN << "Pattern syntax \"W3CXmlSchema11\" unsupported for simQt::RegExpImpl.  Defaulting to \"RegExp\"";
    return RegExpImpl::RegExp;
  case QRegExp::RegExp:
    return RegExpImpl::RegExp;
  case QRegExp::WildcardUnix:
    SIM_WARN << "Pattern syntax \"WildcardUnix\" unsupported for simQt::RegExpImpl.  Defaulting to \"Wildcard\"";
    return RegExpImpl::Wildcard;
  case QRegExp::Wildcard:
    return RegExpImpl::Wildcard;
  case QRegExp::FixedString:
    return RegExpImpl::FixedString;
  }

  // To satisfy warnings.  Should never be reached
  return RegExpImpl::RegExp;
}
#endif

//----------------------------------------------------------------------------------------------------

EntityNameFilter::EntityNameFilter(AbstractEntityTreeModel* model)
  : EntityFilter(),
    model_(model),
    regExp_(new RegExpImpl("")),
    widget_(nullptr)
{
}

bool EntityNameFilter::acceptEntity(simData::ObjectId id) const
{
  if (model_ == nullptr)
    return false;

  if ((regExp_ == nullptr) || regExp_->pattern().empty())
    return true;

  return acceptIndex_(model_->index(id));
}

QWidget* EntityNameFilter::widget(QWidget* newWidgetParent) const
{
  return nullptr;
}

void EntityNameFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  settings.insert(REGULAR_EXPRESSION_PATTERN_SETTING, QString::fromStdString(regExp_->pattern()));
  settings.insert(REGULAR_EXPRESSION_SENSITIVITY_SETTING, regExp_->caseSensitivity());
  settings.insert(REGULAR_EXPRESSION_SYNTAX_SETTING, regExp_->patternSyntax());
}

void EntityNameFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
#if QT_VERSION_MAJOR == 5
  // Qt5 supports filter settings in QRegExp, which Qt6 removes in favor of QRegularExpression.
  // QRegularExpression does not include enough information to rebuild, so this section is now
  // legacy only for reading old Qt5 values now. Modern application uses three settings instead.
  const auto regexpIter = settings.find(REGEXP_SETTING);
  if (regexpIter != settings.end())
  {
    const auto& regExp = regexpIter.value().toRegExp();
    setRegExp(regExp.pattern(), regExp.caseSensitivity(), simQtPatternSyntax(regExp.patternSyntax()));
    return;
  }
#endif

  QString pattern;
  auto it = settings.find(REGULAR_EXPRESSION_PATTERN_SETTING);
  if (it != settings.end())
    pattern = it.value().toString();

  Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
  it = settings.find(REGULAR_EXPRESSION_SENSITIVITY_SETTING);
  if (it != settings.end())
    sensitivity = static_cast<Qt::CaseSensitivity>(it.value().toInt());

  RegExpImpl::PatternSyntax syntax = RegExpImpl::RegExp;
  it = settings.find(REGULAR_EXPRESSION_SYNTAX_SETTING);
  if (it != settings.end())
    syntax = static_cast<RegExpImpl::PatternSyntax>(it.value().toInt());

  setRegExp(pattern, sensitivity, syntax);
}

void EntityNameFilter::bindToWidget(EntityFilterLineEdit* widget)
{
  widget_ = widget;
  if (widget_ == nullptr)
    return;

  // sync the widget to the current regExp
  widget_->configure(QString::fromStdString(regExp_->pattern()), static_cast<Qt::CaseSensitivity>(regExp_->caseSensitivity()), regExp_->patternSyntax());
  connect(widget_, &EntityFilterLineEdit::changed, this, &EntityNameFilter::setRegExpAttributes_);
}

void EntityNameFilter::setModel(AbstractEntityTreeModel* model)
{
  model_ = model;
}

void EntityNameFilter::setRegExp(const QString& expression, Qt::CaseSensitivity sensitivity, RegExpImpl::PatternSyntax pattern)
{
  // Update the GUI if it's valid
  if (widget_ != nullptr)
    widget_->configure(expression, sensitivity, pattern);

  setRegExpAttributes_(expression, sensitivity, pattern);
}

std::tuple<QString, Qt::CaseSensitivity, RegExpImpl::PatternSyntax> EntityNameFilter::regExp() const
{
  QString pattern = QString::fromStdString(regExp_->pattern());
  Qt::CaseSensitivity caseSensitivity = regExp_->caseSensitivity() == simQt::RegExpImpl::CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

  return { pattern, caseSensitivity, regExp_->patternSyntax() };
}

void EntityNameFilter::setRegExpAttributes_(QString filter, Qt::CaseSensitivity caseSensitive, RegExpImpl::PatternSyntax patternSyntax)
{
  bool changed = false;

  if (regExp_->pattern() != filter.toStdString())
  {
    regExp_->setPattern(filter.toStdString());
    changed = true;
  }

  if (regExp_->caseSensitivity() != RegExpImpl::simQtCaseSensitivity(caseSensitive))
  {
    regExp_->setCaseSensitivity(RegExpImpl::simQtCaseSensitivity(caseSensitive));
    changed = true;
  }

  if (regExp_->patternSyntax() != patternSyntax)
  {
    regExp_->setPatternSyntax(patternSyntax);
    changed = true;
  }

  if (changed)
    Q_EMIT filterUpdated();
}

bool EntityNameFilter::acceptIndex_(const QModelIndex& index) const
{
  // Should only pass in a valid index
  const QAbstractItemModel* model = index.model();

  // Make sure pointers are valid
  if ((model == nullptr) || (regExp_ == nullptr))
  {
    assert(false);
    return false;
  }

  // Check if this index passes the filter, return true if it does
  const QString name = model->data(index).toString();
  if (regExp_->match(name.toStdString()))
    return true;

  // Index didn't pass, check its children
  int numChildren = model->rowCount(index);
  for (int i = 0; i < numChildren; ++i)
  {
    const QModelIndex& childIdx = model->index(i, 0, index);
    // Check if this child index (or any of its children)
    // passes the filter, return true if any of them pass
    if (acceptIndex_(childIdx))
      return true;
  }

  return false;
}

}
