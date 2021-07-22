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

#include <QRegExp>
#include "simQt/AbstractEntityTreeModel.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/RegExpImpl.h"
#include "simQt/EntityNameFilter.h"

namespace simQt {

// key for the regular expression stored in settings
static const QString REGEXP_SETTING = "RegExp";

//----------------------------------------------------------------------------------------------------

EntityNameFilter::EntityNameFilter(AbstractEntityTreeModel* model)
  : EntityFilter(),
    model_(model),
    regExp_(new RegExpImpl("")),
    widget_(nullptr)
{
}

EntityNameFilter::~EntityNameFilter()
{
  delete regExp_;
  regExp_ = nullptr;
}

bool EntityNameFilter::acceptEntity(simData::ObjectId id) const
{
  if (model_ == nullptr)
    return false;

  return acceptIndex_(model_->index(id));
}

QWidget* EntityNameFilter::widget(QWidget* newWidgetParent) const
{
  return nullptr;
}

void EntityNameFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  // Store regex as a QRegExp
  settings.insert(REGEXP_SETTING, QRegExp(QString::fromStdString(regExp_->pattern()),
    RegExpImpl::qtCaseSensitivity(regExp_->caseSensitivity()), RegExpImpl::qtPatternSyntax(regExp_->patternSyntax())));
}

void EntityNameFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  QMap<QString, QVariant>::const_iterator it = settings.find(REGEXP_SETTING);
  if (it != settings.end())
  {
    const auto& regExp = it.value().toRegExp();
    if (QString::fromStdString(regExp_->pattern()) != regExp.pattern() ||
      regExp_->patternSyntax() != RegExpImpl::simQtPatternSyntax(regExp.patternSyntax()) ||
      regExp_->caseSensitivity() != RegExpImpl::simQtCaseSensitivity(regExp.caseSensitivity()))
    {
      setRegExp(regExp);
    }
  }
}

void EntityNameFilter::bindToWidget(EntityFilterLineEdit* widget)
{
  widget_ = widget;
  if (widget_ == nullptr)
    return;
  connect(widget_, SIGNAL(changed(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)), this, SLOT(setRegExpAttributes_(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)));
}

void EntityNameFilter::setModel(AbstractEntityTreeModel* model)
{
  model_ = model;
}

void EntityNameFilter::setRegExp(const QRegExp& regExp)
{
  // Update the GUI if it's valid
  if (widget_ != nullptr)
  {
    ScopedSignalBlocker block(*widget_);
    widget_->configure(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
  }

  setRegExpAttributes_(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
}

QRegExp EntityNameFilter::regExp() const
{
  QString pattern = QString::fromStdString(regExp_->pattern());
  Qt::CaseSensitivity caseSensitivity = regExp_->caseSensitivity() == simQt::RegExpImpl::CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  QRegExp::PatternSyntax patternSyntax = RegExpImpl::qtPatternSyntax(regExp_->patternSyntax());

  return QRegExp(pattern, caseSensitivity, patternSyntax);
}

void EntityNameFilter::setRegExpAttributes_(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression)
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
  if (regExp_->patternSyntax() != RegExpImpl::simQtPatternSyntax(expression))
  {
    regExp_->setPatternSyntax(RegExpImpl::simQtPatternSyntax(expression));
    changed = true;
  }
  if (changed)
    emit filterUpdated();
}

bool EntityNameFilter::acceptIndex_(const QModelIndex& index) const
{
  // Should only pass in a valid index
  const QAbstractItemModel* model = index.model();
  assert(model != nullptr);
  // Make sure pointers are valid
  if ((model == nullptr) || (regExp_ == nullptr))
    return false;

  // Check if this index passes the filter, return true if it does
  const QString name = model->data(index).toString();
  bool rv = regExp_->match(name.toStdString());
  if (rv)
    return rv;

  // Index didn't pass, check its children
  int numChildren = model->rowCount(index);
  for (int i = 0; i < numChildren; ++i)
  {
    const QModelIndex& childIdx = model->index(i, 0, index);
    // Check if this child index (or any of its children)
    // passes the filter, return true if any of them pass
    rv = acceptIndex_(childIdx);
    if (rv)
      break;
  }
  return rv;
}

}
