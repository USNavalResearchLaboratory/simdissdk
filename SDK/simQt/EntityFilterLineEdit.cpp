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
#include <QMenu>
#include <QContextMenuEvent>
#include <QRegularExpression>
#include "simQt/EntityFilterLineEdit.h"

namespace simQt {

EntityFilterLineEdit::EntityFilterLineEdit(QWidget *parent)
  : QLineEdit(parent),
    caseSensitive_(Qt::CaseSensitive),
    expression_(QRegExp::RegExp),
    regexOnly_(false),
    valid_(true)
{
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(textFilterChanged()));

  caseSensitiveAction_ = new QAction(tr("Case Sensitive"), this);
  connect(caseSensitiveAction_, SIGNAL(triggered()), this, SLOT(caseSensitive()));
  caseSensitiveAction_->setCheckable(true);
  caseSensitiveAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
  addAction(caseSensitiveAction_);

  regularAction_ = new QAction(tr("&Regular Expression"), this);
  connect(regularAction_, SIGNAL(triggered()), this, SLOT(regularExpression()));
  regularAction_->setCheckable(true);
  regularAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
  addAction(regularAction_);

  wildcardAction_ = new QAction(tr("&Wildcard"), this);
  connect(wildcardAction_, SIGNAL(triggered()), this, SLOT(wildcard()));
  wildcardAction_->setCheckable(true);
  wildcardAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
  addAction(wildcardAction_);

  fixedAction_ = new QAction(tr("&Fixed String"), this);
  connect(fixedAction_, SIGNAL(triggered()), this, SLOT(fixedString()));
  fixedAction_->setCheckable(true);
  fixedAction_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
  addAction(fixedAction_);

  rightMouseClickMenu_ = new QMenu(this);
  rightMouseClickMenu_->addAction(caseSensitiveAction_);
  rightMouseClickMenu_->addSeparator();
  rightMouseClickMenu_->addAction(regularAction_);
  rightMouseClickMenu_->addAction(wildcardAction_);
  rightMouseClickMenu_->addAction(fixedAction_);
  rightMouseClickMenu_->addSeparator();
  standardClickMenu_ = createStandardContextMenu()->actions();   // Add the standard stuff to the end
  rightMouseClickMenu_->addActions(standardClickMenu_);
  rightMouseClickMenu_->setStyleSheet("QMenu::item:selected { background: palette(highlight); }");
}

EntityFilterLineEdit::~EntityFilterLineEdit()
{
  delete rightMouseClickMenu_;
  qDeleteAll(standardClickMenu_);
  delete fixedAction_;
  delete wildcardAction_;
  delete regularAction_;
  delete caseSensitiveAction_;
};

// Regular Expression Wildcard Fixed String
void EntityFilterLineEdit::contextMenuEvent(QContextMenuEvent *event)
{
  caseSensitiveAction_->setChecked(caseSensitive_ == Qt::CaseSensitive ? true : false);
  regularAction_->setChecked(expression_ == QRegExp::RegExp ? true : false);
  wildcardAction_->setChecked(expression_ == QRegExp::Wildcard ? true : false);
  fixedAction_->setChecked(expression_ == QRegExp::FixedString ? true : false);

  rightMouseClickMenu_->exec(event->globalPos());
}

void EntityFilterLineEdit::configure(const QString& filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression)
{
  bool revalidate = (text() != filter);

  if (revalidate)
    setText(filter);

  if (!regexOnly_)
  {
    // Do not overwrite a pending request for a revalidate
    if (revalidate == false)
      revalidate = ((caseSensitive_ != caseSensitive) || (expression_ != expression));

    // No cost in updating the member variable, so just do it
    caseSensitive_ = caseSensitive;
    expression_ = expression;
  }

  if (revalidate)
    revalidate_();
}

bool EntityFilterLineEdit::isValid() const
{
  return valid_;
}

void EntityFilterLineEdit::revalidate_()
{
  // Determine whether currently valid
  bool newValid = true;
  if (expression_ == QRegExp::RegExp)
  {
    const QRegularExpression re(text());
    newValid = re.isValid();
  }

  // Change validity
  if (newValid != valid_)
  {
    valid_ = newValid;
    emit isValidChanged(valid_);

    // Update palette
    if (!valid_)
    {
      QPalette pal;
      pal.setColor(QPalette::Text, QColor(145,0,0,255));
      setPalette(pal);
    }
    else
      setPalette(QPalette());
  }
}

void EntityFilterLineEdit::textFilterChanged()
{
  revalidate_();
  emit(changed(text(), caseSensitive_, expression_));
}

void EntityFilterLineEdit::caseSensitive()
{
  if (!regexOnly_)
  {
    if (caseSensitive_ == Qt::CaseSensitive)
      caseSensitive_ =  Qt::CaseInsensitive;
    else
      caseSensitive_ = Qt::CaseSensitive;
    // Validity cannot change on this, don't revalidate
    emit(changed(text(), caseSensitive_, expression_));
  }
}

void EntityFilterLineEdit::regularExpression()
{
  if (expression_ != QRegExp::RegExp)
  {
    expression_= QRegExp::RegExp;
    revalidate_();
    emit(changed(text(), caseSensitive_, expression_));
  }
}

void EntityFilterLineEdit::wildcard()
{
  if (!regexOnly_ && expression_ != QRegExp::Wildcard)
  {
    expression_ = QRegExp::Wildcard;
    revalidate_();
    emit(changed(text(), caseSensitive_, expression_));
  }
}

void EntityFilterLineEdit::fixedString()
{
  if (!regexOnly_ && expression_ != QRegExp::FixedString)
  {
    expression_ = QRegExp::FixedString;
    revalidate_();
    emit(changed(text(), caseSensitive_, expression_));
  }
}

void EntityFilterLineEdit::setRegexOnly(bool regexOnly)
{
  if (regexOnly == regexOnly_)
    return;
  regexOnly_ = regexOnly;

  // Need to set enabled flag to disable hotkeys, and visible flag to make sure it doesn't show

  caseSensitiveAction_->setEnabled(!regexOnly);
  caseSensitiveAction_->setVisible(!regexOnly);

  regularAction_->setEnabled(!regexOnly);
  regularAction_->setVisible(!regexOnly);

  wildcardAction_->setEnabled(!regexOnly);
  wildcardAction_->setVisible(!regexOnly);

  fixedAction_->setEnabled(!regexOnly);
  fixedAction_->setVisible(!regexOnly);

  // If we're going into regex mode, then we need to update values and emit a signal
  if (regexOnly && (caseSensitive_ != Qt::CaseInsensitive || expression_ != QRegExp::RegExp))
  {
    caseSensitive_ = Qt::CaseInsensitive;
    expression_ = QRegExp::RegExp;
    revalidate_();
    emit changed(text(), caseSensitive_, expression_);
  }
}

}
