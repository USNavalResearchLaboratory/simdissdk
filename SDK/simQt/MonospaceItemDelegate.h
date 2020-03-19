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
#ifndef SIMQT_MONOSPACEITEMDELEGATE_H
#define SIMQT_MONOSPACEITEMDELEGATE_H

#include <QFont>
#include <QStyledItemDelegate>
#include "simCore/Common/Common.h"

namespace simQt {

/** Replaces the default font with a monospace one suitable for console output */
class SDKQT_EXPORT MonospaceItemDelegate : public QStyledItemDelegate
{
public:
  /** Allocates a single monospace font to use in drawing */
  MonospaceItemDelegate(QObject* parent = NULL);
  /** Deletes the font dynamic memory */
  virtual ~MonospaceItemDelegate();

  /** Replaces the font option with our monospace font */
  virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
  /** Returns an appropriate size for the selected font */
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  /** Set an integer offset to the size, e.g. -1 to shrink text by 1 point */
  void setPointSizeOffset(int offset);
  /** Retrieve the point size offset */
  int pointSizeOffset() const;

private:
  QFont* monospaceFont_;
  int pointSizeOffset_;
};

}

#endif /* SIMQT_MONOSPACEITEMDELEGATE_H */
