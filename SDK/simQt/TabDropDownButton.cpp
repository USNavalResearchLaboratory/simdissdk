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

#include <QAction>
#include <QMenu>
#include <QTabWidget>
#include "simQt/TabDropDownButton.h"

namespace simQt {

TabDropDownButton::TabDropDownButton(QTabWidget* parent)
  : QToolButton(parent),
  tabWidget_(parent)
{
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setAutoRaise(true);
  setPopupMode(QToolButton::InstantPopup);
  setIcon(QIcon(":/simQt/images/DropDown.png"));
  setStyleSheet("QToolButton::menu-indicator { image: none; }"); // Hide the arrow via stylesheet because QToolButton::setArrowType() is ignored when QMenu is added
  show(); // required to be visible
  assert(tabWidget_); // tab widget parent should not be NULL
  if (tabWidget_)
    tabWidget_->setCornerWidget(this);

  menu_ = new QMenu(this);
  menu_->setToolTipsVisible(true);

  setMenu(menu_);
  connect(menu_, &QMenu::aboutToShow, this, &TabDropDownButton::updateMenu_);
}

TabDropDownButton::~TabDropDownButton()
{
}

void TabDropDownButton::updateMenu_()
{
  menu_->clear();
  if (tabWidget_ == nullptr)
    return;

  bool hasIcon = false;
  for (int i = 0; i < tabWidget_->count(); ++i)
  {
    const QIcon& icon = tabWidget_->tabIcon(i);
    hasIcon = (hasIcon || !icon.isNull());
    QAction* a = new QAction(icon, tabWidget_->tabText(i), menu_);
    a->setToolTip(tabWidget_->tabToolTip(i));
    connect(a, &QAction::triggered, this, [this, i]() { tabWidget_->setCurrentIndex(i); });
    menu_->addAction(a);
  }

  if (hasIcon)
    menu_->setStyleSheet("QMenu::item { padding: 4px 8px 4px 4px; } QMenu::item:selected { background-color: palette(highlight); color: palette(highlighted-text); } QMenu::icon { padding-left: 8px; }");
  else
    menu_->setStyleSheet("QMenu::item { padding: 4px 15px 4px 15px; } QMenu::item:selected { background-color: palette(highlight); color: palette(highlighted-text); }");
}

}
