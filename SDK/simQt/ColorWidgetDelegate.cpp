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
#include <cassert>
#include <QApplication>
#include <QPainter>
#include "simQt/ColorWidget.h"
#include "simQt/ColorWidgetDelegate.h"

namespace simQt {

ColorWidgetDelegate::ColorWidgetDelegate(bool showAlpha, QObject* parent)
  : QStyledItemDelegate(parent),
  showAlpha_(showAlpha)
{}

QWidget* ColorWidgetDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  simQt::ColorWidget* button = new simQt::ColorWidget(parent);
  button->setShowAlpha(showAlpha_);
  button->setIncludeText(false);
  connect(button, SIGNAL(colorChanged(QColor)), this, SLOT(commitAndCloseEditor_()));
  return button;
}

void ColorWidgetDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  QVariant dataVar = index.data();
  // Don't use invalid/unset data
  if (!dataVar.isValid())
    return;

  simQt::ColorWidget* button = dynamic_cast<simQt::ColorWidget*>(editor);
  // Somehow got an editor other than the one defined in createEditor()
  assert(button);
  if (button)
    button->setColor(dataVar.value<QColor>());
}

void ColorWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  simQt::ColorWidget button;
  button.setGeometry(option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height());
  button.setIncludeText(false);
  button.setColor(index.data().value<QColor>());

  paintItemBackground_(painter, option, index);
  painter->drawPixmap(option.rect.x(), option.rect.y(), button.grab());
}

void ColorWidgetDelegate::paintItemBackground_(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  painter->save();
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  opt.text.clear();
  QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
  painter->restore();
}

void ColorWidgetDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
  simQt::ColorWidget* button = static_cast<simQt::ColorWidget*>(editor);
  model->setData(index, QVariant(button->color()), Qt::DecorationRole);
}

void ColorWidgetDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  editor->setGeometry(option.rect);
}

void ColorWidgetDelegate::commitAndCloseEditor_()
{
  simQt::ColorWidget* editor = qobject_cast<simQt::ColorWidget*>(sender());
  emit QAbstractItemDelegate::commitData(editor);
  emit QAbstractItemDelegate::closeEditor(editor);
}

}
