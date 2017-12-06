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
#include "simQt/EntityNameFilter.h"

namespace simQt {

// key for the regular expression stored in settings
static const QString REGEXP_SETTING = "RegExp";

//----------------------------------------------------------------------------------------------------

EntityNameFilter::EntityNameFilter(AbstractEntityTreeModel* model)
  : EntityFilter(),
    model_(model),
    regExp_(new QRegExp()),
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
  return name.contains(*regExp_);
}

QWidget* EntityNameFilter::widget(QWidget* newWidgetParent) const
{
  return NULL;
}

void EntityNameFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  settings.insert(REGEXP_SETTING, *regExp_);
}

void EntityNameFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  QMap<QString, QVariant>::const_iterator it = settings.find(REGEXP_SETTING);
  if (it != settings.end())
  {
    const auto& regExp = it.value().toRegExp();
    if (*regExp_ != regExp)
    {
      *regExp_ = regExp;
      // Update the GUI if it's valid
      if (widget_ != NULL)
      {
        ScopedSignalBlocker block(*widget_);
        widget_->configure(regExp_->pattern(), regExp_->caseSensitivity(), regExp_->patternSyntax());
      }
      emit filterUpdated();
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
  *regExp_ = regExp;
  emit filterUpdated();
}

void EntityNameFilter::setRegExpAttributes_(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression)
{
  regExp_->setPattern(filter);
  regExp_->setCaseSensitivity(caseSensitive);
  regExp_->setPatternSyntax(expression);
  emit filterUpdated();
}

}
