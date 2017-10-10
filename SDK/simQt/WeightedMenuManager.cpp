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

#include <cassert>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QHBoxLayout>
#include <QString>
#include <qalgorithms.h>
#include "simQt/ActionRegistry.h"
#include "simQt/WeightedMenuManager.h"

namespace
{
QDataStream &operator<<(QDataStream &out, const QList<int> &values)
{
  out << values.count();
  Q_FOREACH(int value, values)
    out << value;
  return out;
}

QDataStream &operator>>(QDataStream &in, QList<int> &values)
{
  int numItems = 0;
  in >> numItems;
  for (int k = 0; k < numItems; ++k)
  {
    int value;
    in >> value;
    values.push_back(value);
  }
  return in;
}
}

namespace simQt {

/**
 * We store the list of child weights for menus, tool bars, and menu bars in the
 * properties table (unique to all QWidgets).  Note that this must be a char*.
 */
static const char* WEIGHTS_PROPERTY = "weights";

WeightedMenuManager::WeightedMenuManager(bool debugMenuWeights)
  : debugMenuWeights_(debugMenuWeights),
    menuBar_(NULL),
    toolBar_(NULL),
    statusBar_(NULL)
{
}

WeightedMenuManager::~WeightedMenuManager()
{
}

QWidget* WeightedMenuManager::topLevelMenu()
{
  // Note that this may be overridden by some classes
  return menuBar_;
}

QMenu* WeightedMenuManager::getOrCreateMenu(QMenu* underMenu, int weight, const QString& menuName)
{
  if (underMenu == NULL)
    return findOrCreateMenu_(topLevelMenu(), weight, menuName);
  return findOrCreateMenu_(underMenu, weight, menuName);
}

void WeightedMenuManager::insertMenuAction(QMenu* menu, int weight, const simQt::Action* action)
{
  insertMenuAction(menu, weight, action->action());
}

void WeightedMenuManager::insertMenuAction(QMenu* menu, int weight, QAction* action)
{
  if (menu == NULL)
    insertBefore_(topLevelMenu(), weight, action);
  else
    insertBefore_(menu, weight, action);
}

void WeightedMenuManager::insertMenuSeparator(QMenu* menu, int weight)
{
  QAction* separator = NULL;
  if (menu == NULL)
    insertBefore_(topLevelMenu(), weight, separator);
  else
    insertBefore_(menu, weight, separator);
}

QMenu* WeightedMenuManager::findMenu_(QWidget* parent, const QString& title) const
{
  // Loop through each entry in the menu
  Q_FOREACH(QObject* menuHeader, parent->children())
  {
    // Only accept menus in our search (ignore actions)
    QMenu* topMenu = qobject_cast<QMenu*>(menuHeader);
    if (topMenu == NULL)
      continue;

    // Figure out the menu title (might be adjusted based on menu weights value)
    QString menuTitle = topMenu->title();
    if (debugMenuWeights_)
    {
      // There might be digits prepended here; remove them
      menuTitle = menuTitle.right(menuTitle.length() - menuTitle.indexOf(QRegExp("[A-Za-z]")));
    }

    // Return the menu if it matches
    if (withoutMnemonic_(menuTitle) == title)
      return topMenu;
  }
  return NULL;
}

/** Finds or creates a single menu item under a parent menu bar/menu */
QMenu* WeightedMenuManager::findOrCreateMenu_(QWidget* parent, int weight, const QString& title) const
{
  // Ensure the hierarchical notation from previous SIMDIS 10 iterations is not used here
  assert(!title.contains('\\'));
  QMenu* found = findMenu_(parent, withoutMnemonic_(title));
  if (found)
    return found;

  // At this point we need to create the menu with the given weight
  QMenu* rv = new QMenu(title, parent);
  if (debugMenuWeights_)
    rv->setTitle(QString("%1 %2").arg(weight).arg(title));

  // Put in a reasonable object name for debugging and introspection purposes
  rv->setObjectName(QString("Menu_%1_w%2_0x%3").arg(title).arg(weight).arg(reinterpret_cast<qulonglong>(rv), 0, 16));

  // Insert the menu into the right place
  insertBefore_(parent, weight, rv);
  return rv;
}

// Helper to insert separator into a QMenu or QToolBar
void WeightedMenuManager::insertSeparator_(QWidget* menuOrToolBar, QAction* beforeAction) const
{
  // Attempt to insert into a QMenu
  QMenu* menu = qobject_cast<QMenu*>(menuOrToolBar);
  if (menu != NULL)
  {
    menu->insertSeparator(beforeAction);
    return;
  }

  // Attempt to insert into a QToolBar
  QToolBar* toolbar = qobject_cast<QToolBar*>(menuOrToolBar);
  if (toolbar != NULL)
  {
    toolbar->insertSeparator(beforeAction);
    return;
  }

  // Can only put separators into QToolBar and QMenu
  assert(0);
}

// Retrieves the action child by index, safely without threat of out-of-range
QAction* WeightedMenuManager::actionByIndex_(const QWidget* widget, int index) const
{
  const QList<QAction*> actions = widget->actions();
  if (index >= actions.size())
    return NULL;
  return actions[index];
}

// Used to add actions to menus, tool bar, sub-menus, etc.
void WeightedMenuManager::insertBefore_(QWidget* widget, int weight, QAction* action) const
{
  if (widget == NULL)
    return;

  // Figure out the insert-before based on the weights
  QList<int> weights = menuWeights_(widget);
  QList<int>::iterator insertBefore = qUpperBound(weights.begin(), weights.end(), weight);

  // Insert the action before other actions
  QAction* beforeAct = actionByIndex_(widget, std::distance(weights.begin(), insertBefore));

  // Add the action to the menu or tool bar
  if (action == NULL)
    insertSeparator_(widget, beforeAct);
  else
    widget->insertAction(beforeAct, action);

  // Add the new weight to the previous text
  if (debugMenuWeights_ && action != NULL)
    action->setText(QString("%1 %2").arg(weight).arg(action->text()));

  // Update the weights
  weights.insert(insertBefore, weight);
  setMenuWeights_(widget, weights);
}

// QMenu and QMenuBar are not related (enough) and have different routines for inserting menus
void WeightedMenuManager::insertMenu_(QWidget* menuOrBar, QAction* beforeAction, QMenu* menu) const
{
  // QMenu and QMenuBar are not related (enough) and have different routines for inserting menus
  QMenu* asMenu = qobject_cast<QMenu*>(menuOrBar);
  if (asMenu != NULL)
  {
    asMenu->insertMenu(beforeAction, menu);
    return;
  }

  // Attempt with QMenuBar
  QMenuBar* asMenuBar = qobject_cast<QMenuBar*>(menuOrBar);
  if (asMenuBar != NULL)
  {
    asMenuBar->insertMenu(beforeAction, menu);
    return;
  }

  // Assertion failure implies that widget is not QMenu and not QMenuBar -- what is it?
  assert(asMenuBar != NULL);
}

void WeightedMenuManager::insertBefore_(QWidget* menuOrBar, int weight, QMenu* menu) const
{
  if (menuOrBar == NULL)
    return;

  // Figure out the insert-before based on the weights
  QList<int> weights = menuWeights_(menuOrBar);
  QList<int>::iterator insertBefore = qUpperBound(weights.begin(), weights.end(), weight);

  // Insert the action before other actions
  QAction* beforeAct = actionByIndex_(menuOrBar, std::distance(weights.begin(), insertBefore));
  insertMenu_(menuOrBar, beforeAct, menu);

  // Update the weights
  weights.insert(insertBefore, weight);
  setMenuWeights_(menuOrBar, weights);
}

QList<int> WeightedMenuManager::menuWeights_(QWidget* menuOrToolbar) const
{
  // We store the weights are stored in a named property in the QMenu, QMenuBar, or QToolBar
  QList<int> rv = menuOrToolbar->property(WEIGHTS_PROPERTY).value<QList<int> >();

  // Make sure the number of weights match the number of children, else we have a problem!
  int numChildren = menuOrToolbar->actions().count();
  // Special case: menu with uninitialized weights
  if (numChildren != 0 && rv.size() == 0)
  {
    // Default entries are spaced by 100 (e.g. 100, 200, 300)
    for (int k = 0; k < numChildren; ++k)
    {
      rv.push_back((k + 1) * 100);

      // If debugging weights, prepend the weight value
      if (debugMenuWeights_)
      {
        QAction* action = menuOrToolbar->actions()[k];
        action->setText(QString("%1 %2").arg((k + 1) * 100).arg(action->text()));
      }
    }

    setMenuWeights_(menuOrToolbar, rv);
    assert(rv == menuWeights_(menuOrToolbar));
  }

  // At this point, the number of children really needs to be matching the menu weights,
  // and if they don't that means we got a child added without a weight, which means the
  // weights are totally out of whack.  This shouldn't happen.
  assert(numChildren == rv.size());

  return rv;
}

void WeightedMenuManager::setMenuWeights_(QWidget* menuOrToolbar, QList<int> weights) const
{
  menuOrToolbar->setProperty(WEIGHTS_PROPERTY, QVariant::fromValue(weights));
}

void WeightedMenuManager::setMenuBar(QWidget* menuBar)
{
  menuBar_ = menuBar;
}

void WeightedMenuManager::setToolBar(QWidget* toolBar)
{
  toolBar_ = toolBar;
}

void WeightedMenuManager::setStatusBar(QWidget* statusBar)
{
  statusBar_ = statusBar;
}

void WeightedMenuManager::insertToolBarAction(int weight, const simQt::Action* action)
{
  if (toolBar_ == NULL)
    return;
  WeightedMenuManager::insertBefore_(toolBar_, weight, action->action());
}

void WeightedMenuManager::insertToolBarSeparator(int weight)
{
  if (toolBar_ == NULL)
    return;
  QAction* separator = NULL;
  WeightedMenuManager::insertBefore_(toolBar_, weight, separator);
}

void WeightedMenuManager::insertWidgetBefore_(QWidget* parentWidget, int weight, QWidget* newWidget)
{
  if (parentWidget == NULL || newWidget == NULL)
    return;
  // Pull out the layout, because that's really what we're working with
  QBoxLayout* layout = qobject_cast<QBoxLayout*>(parentWidget->layout());
  if (layout == NULL)
    return;

  // Figure out the insert-before based on the weights
  QList<int> weights = widgetWeights_(parentWidget);
  const QList<int>::iterator insertBefore = qUpperBound(weights.begin(), weights.end(), weight);
  const int indexOfInsertion = std::distance(weights.begin(), insertBefore);

  // Insert the widget before other widgets
  layout->insertWidget(indexOfInsertion, newWidget);

  // Add the new weight to the previous text
  if (debugMenuWeights_)
    newWidget->setToolTip(QString("%1 %2").arg(weight).arg(newWidget->toolTip()));

  // Update the weights
  weights.insert(insertBefore, weight);
  setWidgetWeights_(parentWidget, weights);
}

void WeightedMenuManager::insertStatusBarWidget(int weight, QWidget* widget)
{
  if (statusBar_ == NULL)
    return;
  insertWidgetBefore_(statusBar_, weight, widget);
}

void WeightedMenuManager::insertStatusBarAction(int weight, const simQt::Action* action)
{
  if (statusBar_ == NULL)
    return;
  // Set up a new QToolButton
  QToolButton* newButton = new QToolButton(statusBar_);
  newButton->setAutoRaise(true);
  newButton->setDefaultAction(action->action());
  insertStatusBarWidget(weight, newButton);
  // Note that the setDefaultAction trumps the setToolTip() in insertWidgetBefore_(), so
  // we have to set it again here if we want to debug the weights.
  if (debugMenuWeights_)
    action->action()->setToolTip(QString("%1 %2").arg(weight).arg(action->action()->toolTip()));
}

QList<int> WeightedMenuManager::widgetWeights_(QWidget* widget) const
{
  if (widget == NULL)
    return QList<int>();
  const QBoxLayout* layout = qobject_cast<QBoxLayout*>(widget->layout());
  if (layout == NULL)
    return QList<int>();

  // We store the weights are stored in a named property in the QWidget
  QList<int> rv = widget->property(WEIGHTS_PROPERTY).value<QList<int> >();

  // Make sure the number of weights match the number of children, else we have a problem!
  const int numChildren = layout->count();
  // Special case: widget with uninitialized weights
  if (numChildren != 0 && rv.size() == 0)
  {
    // Default entries are spaced by 100 (e.g. 100, 200, 300)
    for (int k = 0; k < numChildren; ++k)
    {
      rv.push_back((k + 1) * 100);

      // If debugging weights, prepend the weight value
      if (debugMenuWeights_)
      {
        QWidget* childWidget = layout->itemAt(k)->widget();
        if (childWidget)
          childWidget->setToolTip(QString("%1 %2").arg((k + 1) * 100).arg(childWidget->toolTip()));
      }
    }

    setWidgetWeights_(widget, rv);
    assert(rv == widgetWeights_(widget));
  }

  // At this point, the number of children really needs to be matching the widget weights,
  // and if they don't that means we got a child added without a weight, which means the
  // weights are totally out of whack.  This shouldn't happen.
  assert(numChildren == rv.size());

  return rv;
}

void WeightedMenuManager::setWidgetWeights_(QWidget* widget, QList<int> weights) const
{
  widget->setProperty(WEIGHTS_PROPERTY, QVariant::fromValue(weights));
}

QString WeightedMenuManager::withoutMnemonic_(const QString& text) const
{
  if (!text.contains("&"))
    return text;
  // This method does not correctly handle double ampersand
  assert(!text.contains("&&"));
  QString updated = text;
  return updated.replace("&", "");
}


//----------------------------------------------------------------------------------------------------

PopupMenuManager::PopupMenuManager(QMenu& menu, bool debugMenuWeights)
  : WeightedMenuManager(debugMenuWeights)
{
  setMenuBar(&menu);
}

}
