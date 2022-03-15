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
#include <QApplication>
#include "simQt/MonospaceItemDelegate.h"

namespace simQt {

MonospaceItemDelegate::MonospaceItemDelegate(QObject* parent)
  : QStyledItemDelegate(parent),
    monospaceFont_(new QFont("Monospace")),
    pointSizeOffset_(0)
{
  monospaceFont_->setStyleHint(QFont::TypeWriter);
}

MonospaceItemDelegate::~MonospaceItemDelegate()
{
  delete monospaceFont_;
}

int MonospaceItemDelegate::pointSizeOffset() const
{
  return pointSizeOffset_;
}

void MonospaceItemDelegate::setPointSizeOffset(int offset)
{
  pointSizeOffset_ = offset;
}

void MonospaceItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyleOptionViewItem opt(option);
  // Adjust the pixel size based on the incoming pixel size
  if (option.font.pointSize() > 0)
  {
    monospaceFont_->setPointSize(option.font.pointSize() + pointSizeOffset_);
  }
  opt.font = *monospaceFont_;
  QStyledItemDelegate::paint(painter, opt, index);
}

QSize MonospaceItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QVariant value = index.data(Qt::SizeHintRole);
  if (value.isValid())
    return value.toSize();

  // Create a non-const style option
  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  // Initialize the style with the font we use
  if (option.font.pointSize() > 0)
  {
    monospaceFont_->setPointSize(option.font.pointSize());
  }
  opt.font = *monospaceFont_;

  // Pull out the style information and ask it to give us a size
  QStyle* style = (opt.widget ? opt.widget->style() : QApplication::style());
  return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
}

}
