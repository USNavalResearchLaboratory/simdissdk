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
#include <iostream>
#include <QAction>
#include <QApplication>
#include <QPushButton>
#include "simCore/Common/SDKAssert.h"
#include "simVis/Headless.h"
#include "simQt/ActionRegistry.h"

using namespace simQt;

namespace
{

struct NewRegistry
{
  ActionRegistry* reg;
  QAction* superform;
  QAction* rangeTool;
  QAction* views;
  QAction* pluginManager;
  QAction* help;
  NewRegistry()
   : reg(new ActionRegistry(nullptr)),
     superform(new QAction("superform", nullptr)),
     rangeTool(new QAction("rangeTool...", nullptr)),
     views(new QAction("views...", nullptr)),
     pluginManager(new QAction("pluginManager...", nullptr)),
     help(new QAction("help...", nullptr))
  {
    superform->setShortcuts(QList<QKeySequence>() << QKeySequence("Alt+S") << QKeySequence("Ctrl+S"));
    reg->registerAction("Tools", "SuperForm", superform);
    reg->registerAction("Tools", "Range Tool", rangeTool);
    reg->registerAction("View", "Views", views);
    reg->registerAction("Plugins", "Plugin Manager", pluginManager);
    reg->registerAction("Help", "Help", help);
  }
  ~NewRegistry()
  {
    delete reg;
    delete superform;
    delete rangeTool;
    delete views;
    delete pluginManager;
    delete help;
  }

private:
  /** Not implemented */
  NewRegistry(const NewRegistry& noCopyConstructor);
};

int testFind()
{
  int rv = 0;

  NewRegistry reg;
  rv += SDK_ASSERT(!reg.reg->actions().empty());

  Action* action = reg.reg->findAction("Views");
  rv += SDK_ASSERT(action != nullptr);
  if (action != nullptr)
  {
    rv += SDK_ASSERT(action->description() == "Views");
    rv += SDK_ASSERT(action->hotkeys().empty());
  }
  action = reg.reg->findAction("View");
  rv += SDK_ASSERT(action == nullptr);

  // Re-register should not work; throw an exception for testing?
  //Action* badAction = reg.reg->registerAction("Unknown", "Views", reg.superform);
  //rv += SDK_ASSERT(badAction != nullptr);
  //rv += SDK_ASSERT(badAction->action() != reg.superform);
  //rv += SDK_ASSERT(badAction->group() != "Unknown");

  // Re-register of same action with different name should also fail; throw an exception?
  //Action* badAction2 = reg.reg->registerAction("Another", "Another", reg.superform);
  //if (badAction2 != nullptr)
  //{
  //  rv += SDK_ASSERT(badAction2->action() == reg.superform);
  //  rv += SDK_ASSERT(badAction2->description() != "Another");
  //  rv += SDK_ASSERT(badAction2->group() != "Another");
  //}

  action = reg.reg->findAction(QKeySequence("Alt+S"));
  rv += SDK_ASSERT(action != nullptr);
  if (action != nullptr)
    rv += SDK_ASSERT(action->description() == "SuperForm");
  action = reg.reg->findAction(QKeySequence("Ctrl+S"));
  rv += SDK_ASSERT(action != nullptr);
  if (action != nullptr)
  {
    rv += SDK_ASSERT(action->description() == "SuperForm");
    rv += SDK_ASSERT(action->hotkeys().size() == 2);
  }

  // Remove hotkey from SuperForm and re-search
  reg.reg->removeHotKey(action, 0); // should remove Alt+S
  action = reg.reg->findAction(QKeySequence("Alt+S"));
  rv += SDK_ASSERT(action == nullptr);
  action = reg.reg->findAction(QKeySequence("Ctrl+S"));
  rv += SDK_ASSERT(action != nullptr);
  if (action != nullptr)
    rv += SDK_ASSERT(action->hotkeys().size() == 1);

  // Search for nonexistent hotkey
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q")) == nullptr);

  return rv;
}

int testSetHotKey()
{
  int rv = 0;
  NewRegistry reg;
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("V")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction("Views") != nullptr);
  rv += SDK_ASSERT(reg.reg->setHotKey(reg.reg->findAction("Views"), QKeySequence("V")) == 0);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("V")) != nullptr);
  rv += SDK_ASSERT(reg.reg->setHotKey(nullptr, QKeySequence("V")) != 0);
  Action* views = reg.reg->findAction(QKeySequence("V"));
  rv += SDK_ASSERT(views != nullptr);
  if (views)
    rv += SDK_ASSERT(views->description() == "Views");

  // Test an override
  reg.reg->setHotKey(views, QKeySequence("Alt+S"));
  views = reg.reg->findAction(QKeySequence("Alt+S"));
  rv += SDK_ASSERT(views != nullptr);
  if (views)
  {
    rv += SDK_ASSERT(views->description() == "Views");
    rv += SDK_ASSERT(views->hotkeys().size() == 1);
  }
  return rv;
}

bool hasKey(const NewRegistry& reg, const QString& desc, const QString& hotkey)
{
  Action* act = reg.reg->findAction(QKeySequence(hotkey));
  return act != nullptr && act->description() == desc;
}

int testAddHotKey()
{
  int rv = 0;
  NewRegistry reg;

  // Did action exist before add? (most interested in the "no" case; this covers the "yes" cases)
  reg.reg->addHotKey("Range Tool", QKeySequence("A"));
  rv += SDK_ASSERT(hasKey(reg, "Range Tool", "A"));
  reg.reg->addHotKey("Range Tool", QKeySequence("B"));
  rv += SDK_ASSERT(hasKey(reg, "Range Tool", "A"));
  rv += SDK_ASSERT(hasKey(reg, "Range Tool", "B"));
  reg.reg->addHotKey("SuperForm", QKeySequence("C"));
  rv += SDK_ASSERT(hasKey(reg, "SuperForm", "C"));
  rv += SDK_ASSERT(hasKey(reg, "SuperForm", "Alt+S"));
  rv += SDK_ASSERT(hasKey(reg, "SuperForm", "Ctrl+S"));

  // Now look at the action-doesn't-exist cases...

  // case 1: action has no shortcuts during add
  QAction test1("test1", nullptr);
  reg.reg->addHotKey("test1", QKeySequence("D"));
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("D")) == nullptr);
  reg.reg->registerAction("test1", "test1", &test1);
  rv += SDK_ASSERT(hasKey(reg, "test1", "D"));

  // case 2: action has shortcuts during add
  QAction test2("test2", nullptr);
  reg.reg->addHotKey("test2", QKeySequence("E"));
  test2.setShortcut(QKeySequence("F"));
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("E")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("F")) == nullptr);
  reg.reg->registerAction("test2", "test2", &test2);
  rv += SDK_ASSERT(hasKey(reg, "test2", "E"));
  rv += SDK_ASSERT(!hasKey(reg, "test2", "F"));

  // case 3: action has conflicting shortcuts during add
  QAction test3("test3", nullptr);
  reg.reg->addHotKey("test3", QKeySequence("G"));
  test3.setShortcut(QKeySequence("E")); // Belongs to test2
  rv += SDK_ASSERT(hasKey(reg, "test2", "E"));
  Action* t3act = reg.reg->registerAction("test3", "test3", &test3);
  rv += SDK_ASSERT(hasKey(reg, "test2", "E"));
  rv += SDK_ASSERT(hasKey(reg, "test3", "G"));
  rv += SDK_ASSERT(t3act->hotkeys().size() == 1); // only G

  // case 4: addAction's hotkey ends up being in use already
  QAction test4("test4", nullptr);
  reg.reg->addHotKey("test4", QKeySequence("G"));
  rv += SDK_ASSERT(!hasKey(reg, "test3", "G"));
  auto t4act = reg.reg->registerAction("test4", "test4", &test4);
  rv += SDK_ASSERT(!hasKey(reg, "test3", "G"));
  rv += SDK_ASSERT(t3act->hotkeys().empty()); // G got assigned to test4
  rv += SDK_ASSERT(t4act->hotkeys().size() == 1);

  // Test remove action while we're at it, since there's a potential for crashing here due
  // to the out of order destruction.  Note the intentional excessive checking for side effects
  int oldSize = reg.reg->actions().size();
  rv += SDK_ASSERT(hasKey(reg, "test1", "D"));
  rv += SDK_ASSERT(reg.reg->removeAction("test1") == 0);
  rv += SDK_ASSERT(!hasKey(reg, "test1", "D"));
  rv += SDK_ASSERT(reg.reg->actions().size() == oldSize - 1);
  rv += SDK_ASSERT(hasKey(reg, "test2", "E"));
  rv += SDK_ASSERT(reg.reg->removeAction("test2") == 0);
  rv += SDK_ASSERT(!hasKey(reg, "test2", "E"));
  rv += SDK_ASSERT(hasKey(reg, "test4", "G"));
  rv += SDK_ASSERT(reg.reg->removeAction("test3") == 0);
  rv += SDK_ASSERT(reg.reg->removeAction("test4") == 0);
  rv += SDK_ASSERT(reg.reg->removeAction("test5") != 0);

  // Re-add the test3 and make sure its hotkeys were saved
  test4.setShortcuts(QList<QKeySequence>()); // make sure it's empty and coming from the action registry
  rv += SDK_ASSERT(!hasKey(reg, "test3", "G"));
  rv += SDK_ASSERT(!hasKey(reg, "test4", "G"));
  t4act = reg.reg->registerAction("test4", "test4", &test4);
  rv += SDK_ASSERT(!hasKey(reg, "test2", "E"));
  rv += SDK_ASSERT(hasKey(reg, "test4", "G"));
  rv += SDK_ASSERT(t4act->hotkeys().size() == 1); // only G

  // Re-remove it
  rv += SDK_ASSERT(reg.reg->removeAction("test4") == 0);
  rv += SDK_ASSERT(reg.reg->removeAction("test4") != 0);

  return rv;
}

int testExecute()
{
  int rv = 0;
  NewRegistry reg;
  QPushButton testButton;
  testButton.setCheckable(true);
  QAction action("exec", nullptr);
  QObject::connect(&action, SIGNAL(triggered()), &testButton, SLOT(toggle()));
  rv += SDK_ASSERT(!testButton.isChecked());
  action.trigger();
  rv += SDK_ASSERT(testButton.isChecked());
  action.trigger();
  rv += SDK_ASSERT(!testButton.isChecked());
  // Test using registry
  Action* act = reg.reg->registerAction("test", "test", &action);
  rv += SDK_ASSERT(reg.reg->execute("test") == 0);
  rv += SDK_ASSERT(testButton.isChecked());
  rv += SDK_ASSERT(reg.reg->execute("foobar") != 0);
  rv += SDK_ASSERT(testButton.isChecked());
  act->execute();
  rv += SDK_ASSERT(!testButton.isChecked());
  return rv;
}

int testMemento()
{
  int rv = 0;
  NewRegistry reg;
  ActionRegistry::SettingsMemento* defaultSettings = reg.reg->createMemento();
  rv += SDK_ASSERT(defaultSettings != nullptr);
  rv += SDK_ASSERT(reg.superform->shortcuts().size() == 2);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.superform);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);
  // Make some changes
  rv += SDK_ASSERT(0 == reg.reg->addHotKey("Views", QKeySequence("Q")));
  rv += SDK_ASSERT(0 == reg.reg->addHotKey("Plugin Manager", QKeySequence("Alt+S")));
  rv += SDK_ASSERT(reg.superform->shortcuts().size() == 1);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.pluginManager);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q"))->action() == reg.views);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);
  // Save a new memento
  ActionRegistry::SettingsMemento* newSettings = reg.reg->createMemento();
  // Restore last one
  defaultSettings->restore(*reg.reg);
  // Test the original conditions again
  rv += SDK_ASSERT(reg.superform->shortcuts().size() == 2);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.superform);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);
  // Test restoring the new one
  newSettings->restore(*reg.reg);
  rv += SDK_ASSERT(reg.superform->shortcuts().size() == 1);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.pluginManager);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q"))->action() == reg.views);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);
  // Go back to default; restore an action that doesn't exist (i.e. delete views)
  defaultSettings->restore(*reg.reg);
  rv += SDK_ASSERT(0 == reg.reg->removeAction("Views"));
  // Restore the configuration that had a views hotkey, make sure no crash
  newSettings->restore(*reg.reg);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.pluginManager);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);
  delete newSettings;
  // Restore the original settings, after deleting the new ones
  defaultSettings->restore(*reg.reg);
  rv += SDK_ASSERT(reg.superform->shortcuts().size() == 2);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Alt+S"))->action() == reg.superform);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("Q")) == nullptr);
  rv += SDK_ASSERT(reg.reg->findAction(QKeySequence("R")) == nullptr);

  // Looks good, return
  delete defaultSettings;
  return rv;
}

int testAlias()
{
  int rv = 0;

  ActionRegistry* ar = new ActionRegistry(nullptr);

  // Make an action
  QAction* action1 = new QAction("acttion1", nullptr);
  simQt::Action* firstAction = ar->registerAction("Test", "First", action1);
  rv += SDK_ASSERT(firstAction != nullptr);

  // Add an alias; which should work
  rv += SDK_ASSERT(ar->registerAlias("First", "FirstAlias1") == 0);
  // Add the same alias again which should fail
  rv += SDK_ASSERT(ar->registerAlias("First", "FirstAlias1") != 0);

  // Add a second alias; which should work
  rv += SDK_ASSERT(ar->registerAlias("First", "FirstAlias2") == 0);
  // Add the same alias again which should fail
  rv += SDK_ASSERT(ar->registerAlias("First", "FirstAlias1") != 0);

  // Execute via name and alias
  rv += SDK_ASSERT(ar->execute("First") == 0);
  rv += SDK_ASSERT(ar->execute("FirstAlias1") == 0);
  rv += SDK_ASSERT(ar->execute("FirstAlias2") == 0);

  // Execute a bogus name that should fail
  rv += SDK_ASSERT(ar->execute("ShouldNotWork") != 0);

  // Find via name and alias
  rv += SDK_ASSERT(ar->findAction("First") == firstAction);
  rv += SDK_ASSERT(ar->findAction("FirstAlias1") == firstAction);
  rv += SDK_ASSERT(ar->findAction("FirstAlias2") == firstAction);

  // Find a bogus name that should fail
  rv += SDK_ASSERT(ar->findAction("ShouldNotWork") == nullptr);

  delete ar;
  delete action1;

  return rv;
}

int testHotKeyAssignment()
{
  int rv = 0;
  NewRegistry reg;

  QAction exec1("exec1", nullptr);
  const QKeySequence ks1("1");
  exec1.setShortcut(ks1);

  // Nothing in here yet, should be empty
  QString actionName = "NotEmpty";
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());

  // Add action; it should recognize shortcut and be correct
  simQt::Action* qAction = reg.reg->registerAction("Temp", "exec1", &exec1);
  rv += SDK_ASSERT(qAction != nullptr);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec1");

  // Swap hot key
  const QKeySequence ks2("2");
  rv += SDK_ASSERT(reg.reg->setHotKey(qAction, ks2) == 0);
  rv += SDK_ASSERT(qAction != nullptr);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks2, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec1");

  // Remove hot key
  rv += SDK_ASSERT(reg.reg->removeAction("exec1") == 0);
  qAction = nullptr;
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());
  // ks2 should have transitioned to unknown
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks2, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec1");

  // Add hot key for unknown action
  const QKeySequence ks3("3");
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());
  rv += SDK_ASSERT(reg.reg->addHotKey("exec3", ks3) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec3");

  // Reassign it to something that does exist
  QAction exec2("exec2", nullptr);
  qAction = reg.reg->registerAction("Temp", "exec2", &exec2);
  rv += SDK_ASSERT(qAction != nullptr);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec3");
  rv += SDK_ASSERT(reg.reg->addHotKey("exec2", ks3) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec2");

  // Remove that action; it should be assigned to unknown, but to exec2 now and not exec3
  rv += SDK_ASSERT(reg.reg->removeAction("exec2") == 0);
  qAction = nullptr;
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec2");
  // Fix it back to exec3
  rv += SDK_ASSERT(reg.reg->addHotKey("exec3", ks3) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec3");

  // Add exec3 and it should still be on exec3
  QAction exec3("exec3", nullptr);
  qAction = reg.reg->registerAction("Temp", "exec3", &exec3);
  rv += SDK_ASSERT(qAction != nullptr);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks3, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec3");

  // Make sure removeUnknownAction() works
  const QKeySequence ks4("4");
  rv += SDK_ASSERT(reg.reg->addHotKey("exec4", ks4) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks4, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec4");
  rv += SDK_ASSERT(reg.reg->removeUnknownAction("exec4") == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks4, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());

  // Adding another action that takes a hot key should supersede old saved action
  rv += SDK_ASSERT(reg.reg->addHotKey("exec4", ks4) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks4, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec4");
  QAction exec5("exec5", nullptr);
  exec5.setShortcut(ks4);
  qAction = reg.reg->registerAction("Temp", "exec5", &exec5);
  rv += SDK_ASSERT(qAction != nullptr);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks4, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec5");
  rv += SDK_ASSERT(reg.reg->setHotKey(qAction, ks1) == 0);
  // This should not be assigned to unknown exec4 at this point
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks4, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());

  // If an existing action has a hot key, and that hot key is assigned to a non-existing action,
  // it should remove that hot key from the real action
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::ASSIGNED_TO_ACTION);
  rv += SDK_ASSERT(actionName == "exec5");
  rv += SDK_ASSERT(reg.reg->addHotKey("exec6", ks1) == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::ASSIGNED_TO_UNKNOWN);
  rv += SDK_ASSERT(actionName == "exec6");
  rv += SDK_ASSERT(reg.reg->removeUnknownAction("exec6") == 0);
  rv += SDK_ASSERT(reg.reg->getKeySequenceAssignment(ks1, actionName) == simQt::ActionRegistry::UNASSIGNED);
  rv += SDK_ASSERT(actionName.isEmpty());

  return rv;
}

}

int ActionRegistryTest(int argc, char* argv[])
{
  if (simVis::isHeadless())
  {
    std::cerr << "Headless display detected; aborting test." << std::endl;
    return 0;
  }
  int rv = 0;
  QApplication app(argc, argv);
  rv += testFind();
  rv += testSetHotKey();
  rv += testAddHotKey();
  rv += testExecute();
  rv += testMemento();
  rv += testAlias();
  rv += testHotKeyAssignment();
  return rv;
}

