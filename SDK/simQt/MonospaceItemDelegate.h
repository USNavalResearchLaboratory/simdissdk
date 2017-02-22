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

private:
  QFont* monospaceFont_;
};

}

#endif /* SIMQT_MONOSPACEITEMDELEGATE_H */
