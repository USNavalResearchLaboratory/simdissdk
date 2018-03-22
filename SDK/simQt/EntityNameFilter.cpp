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
    widget_(NULL)
{
}

EntityNameFilter::~EntityNameFilter()
{
  delete regExp_;
  regExp_ = NULL;
}

bool EntityNameFilter::acceptEntity(simData::ObjectId id) const
{
  if (model_ == NULL)
    return false;
  QString name = model_->data(model_->index(id)).toString();
  return regExp_->match(name.toStdString());;
}

QWidget* EntityNameFilter::widget(QWidget* newWidgetParent) const
{
  return NULL;
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
      // Update the GUI if it's valid
      if (widget_ != NULL)
      {
        ScopedSignalBlocker block(*widget_);
        widget_->configure(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
      }
      setRegExp(regExp);
    }
  }
}

void EntityNameFilter::bindToWidget(EntityFilterLineEdit* widget)
{
  widget_ = widget;
  if (widget_ == NULL)
    return;
  connect(widget_, SIGNAL(changed(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)), this, SLOT(setRegExpAttributes_(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)));
}

void EntityNameFilter::setModel(AbstractEntityTreeModel* model)
{
  model_ = model;
}

void EntityNameFilter::setRegExp(const QRegExp& regExp)
{
  setRegExpAttributes_(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
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

}
