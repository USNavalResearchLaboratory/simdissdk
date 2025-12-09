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
#ifndef SIMQT_TABDROPDOWNBUTTON_H
#define SIMQT_TABDROPDOWNBUTTON_H

#include <QPointer>
#include <QToolButton>
#include "simCore/Common/Export.h"

class QMenu;
class QTabWidget;

namespace simQt {

/**
 * Creates a QToolButton with a drop down menu to set the current index on a QTabWidget.
 * Installs itself as part of construction, and memory is managed by Qt parentage. Example usage:
 *
 *   QTabWidget* tabWidget = new QTabWidget();
 *   new simQt::TabDropDownButton(tabWidget);
 */
class SDKQT_EXPORT TabDropDownButton : public QToolButton
{
  Q_OBJECT;

public:
  explicit TabDropDownButton(QTabWidget* parent = nullptr);
  virtual ~TabDropDownButton();

private Q_SLOTS:
  /**
   * Update the menu to the current tabs in tab widget.
   * Triggered when the menu is about to be shown.
   */
  void updateMenu_();

private:
  QPointer<QTabWidget> tabWidget_;
  QPointer<QMenu> menu_;
};

}

#endif /* SIMQT_TABDROPDOWNBUTTON_H */
