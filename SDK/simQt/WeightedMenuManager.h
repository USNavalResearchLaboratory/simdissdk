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

#ifndef SIMQT_WEIGHTED_MENU_MANAGER_H
#define SIMQT_WEIGHTED_MENU_MANAGER_H

#include <QList>
#include <QMetaType>
#include "simCore/Common/Common.h"

class QAction;
class QMenu;
class QString;
class QToolBar;
class QWidget;

namespace simQt {

class Action;

/**
 * Enforces an ordering on menu items according to weight.  This base class
 * can be used by either a QMenuBar or a QMenu (for a popup menu) which is
 * provided by the topLevel method
 */
class SDKQT_EXPORT WeightedMenuManager
{
public:
  /** Constructor */
  WeightedMenuManager(bool debugMenuWeights);
  virtual ~WeightedMenuManager();

  // Set the menu, tool, and status bars to use; may be nullptr
  void setMenuBar(QWidget* menuBar);
  void setToolBar(QWidget* toolBar);
  void setStatusBar(QWidget* statusBar);

  /** Gets or creates menuName to underMenu at the given weight location */
  QMenu* getOrCreateMenu(QMenu* underMenu, int weight, const QString& menuName);
  /** Inserts action into underMenu at the given weight location */
  void insertMenuAction(QMenu* underMenu, int weight, const simQt::Action* action);
  /** Inserts action into underMenu at the given weight location */
  void insertMenuAction(QMenu* underMenu, int weight, QAction* action);
  /** Inserts a separator into underMenu at the given weight location */
  void insertMenuSeparator(QMenu* underMenu, int weight);

  /// Adds a toolbar action to the simMainWindow
  void insertToolBarAction(int weight, const simQt::Action* action);
  /// Adds a toolbar action to the simMainWindow
  void insertToolBarAction(int weight, QAction* action);
  /// Adds a toolbar separator to the simMainWindow
  void insertToolBarSeparator(int weight);
  /// Adds a status bar action to the simMainWindow
  void insertStatusBarAction(int weight, const simQt::Action* action);
  /// Adds a status bar widget to the simMainWindow
  void insertStatusBarWidget(int weight, QWidget* widget);

  /**
   * Returns the top level menu or menu bar (QMenu or QMenuBar) that
   * is used as the hierarchical parent of created menu items.  This
   * is the first level for menus.
   */
  virtual QWidget* topLevelMenu();

protected:
  /** If true adds debug information to the menu text */
  const bool debugMenuWeights_;

  // Pointers provided for menu, tool, and status bars
  QWidget* menuBar_;
  QWidget* toolBar_;
  QWidget* statusBar_;

  /** Returns the named menu under the given widget parent; will not create a menu */
  QMenu* findMenu_(QWidget* parent, const QString& title) const;
  /** Finds or creates a single menu item under a parent menubar/menu */
  QMenu* findOrCreateMenu_(QWidget* parent, int weight, const QString& title) const;

  /** Inserts an action into a widget using the weight as a guideline for insertion */
  void insertBefore_(QWidget* widget, int weight, QAction* action) const;
  /** Inserts a menu into a QMenu or a QMenuBar, using the weight as a guideline for insertion */
  void insertBefore_(QWidget* menuOrBar, int weight, QMenu* menu) const;
  /** Inserts an action into a widget using the weight as a guideline for insertion */
  void insertBefore_(QToolBar* toolBar, int weight, QWidget* widget) const;

  /** Inserts a separator into a QMenu or QToolBar, before the given action; does not change weights */
  void insertSeparator_(QWidget* menuOrToolBar, QAction* beforeAction) const;
  /** Inserts a submenu under a QMenu or QMenuBar before given action; does not change weights */
  void insertMenu_(QWidget* menuOrBar, QAction* beforeAction, QMenu* menu) const;
  /** Retrieves the action child by index, safely without threat of out-of-range */
  QAction* actionByIndex_(const QWidget* widget, int index) const;

  /** Retrieves the list of weights for a given menu */
  QList<int> menuWeights_(QWidget* menuOrToolbar) const;
  /** Sets the weights for a given menu */
  void setMenuWeights_(QWidget* menuOrToolbar, QList<int> weights) const;

  /** Like WeightedMenuManager::insertBefore_() but for widgets in widgets */
  void insertWidgetBefore_(QWidget* parentWidget, int weight, QWidget* newWidget);
  /** Like WeightedMenuManager::menuWeight_() but for Widgets with layouts */
  QList<int> widgetWeights_(QWidget* widget) const;
  /** Like WeightedMenuManager::setMenuWeights_() but for Widgets with layouts */
  void setWidgetWeights_(QWidget* widget, QList<int> weights) const;

  /** Utility method to return the string without mnemonic */
  QString withoutMnemonic_(const QString& decorated) const;
};


/**
 * Enforces an ordering on menu items according to weight.  This class helps
 * build a popup menu.
 */
class SDKQT_EXPORT PopupMenuManager : public WeightedMenuManager
{
public:
  /** Constructor */
  PopupMenuManager(QMenu& menu, bool debugMenuWeights);
};

}

Q_DECLARE_METATYPE(QList<int>);

#endif
