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
#include <QObject>
#include <QApplication>
#include <QAction>
#include <QMainWindow>
#include <QTreeView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include "simCore/System/Utils.h"
#include "simQt/ActionRegistry.h"
#include "simQt/ActionItemModel.h"
#include "ActionItemModelTest.h"

void Engine::doNext()
{
  switch (nextAction_)
  {
  case 0:
    addAction(*registry_, "Foobar", "New Action", this, SLOT(newAction()));
    edit_->setText("Just added Foobar\\New Action");
    break;
  case 1:
    addAction(*registry_, "New Group", "New Group", this, SLOT(newGroup()));
    edit_->setText("Just added New Group\\New Group");
    break;
  case 2:
    registry_->addHotKey("Toggle Labels", QKeySequence("N")); // Adds N
    edit_->setText("Just added N hotkey to Toggle Labels");
    break;
  case 3:
    registry_->removeHotKey(registry_->findAction("Baz"), 0); // removes A
    edit_->setText("Just removed first hotkey from Baz");
    break;
  case 4:
    registry_->removeAction("Preference Rules");
    edit_->setText("Just removed Tools\\Preference Rules");
    break;
  case 5:
    registry_->removeAction("About");
    edit_->setText("Just removed Help\\About");
    break;
  default:
    edit_->setText("No more actions.  You can try shortcuts.");
    break;
  }
  nextAction_++;
}

void Engine::addAction(simQt::ActionRegistry& registry, const QString& group, const QString& desc,
               QObject* target, const QString& slot, QList<QKeySequence> shortcuts)
{
  QAction* newAction = new QAction("Dummy Text", target);
  QObject::connect(newAction, SIGNAL(triggered()), target, slot.toStdString().c_str());
  simQt::Action* action = registry.registerAction(group, desc, newAction);
  registry.setHotKeys(action, shortcuts);
}

// Example demonstration of the item model
int main(int argc, char* argv[])
{
  simCore::initializeSimdisEnvironmentVariables();
  QApplication app(argc, argv);
  QWidget mainWindow;

  simQt::ActionRegistry registry(&mainWindow);

  QLineEdit edit("Press hotkey or click button...", &mainWindow);
  edit.setReadOnly(true);
  QTreeView view(&mainWindow);
  simQt::ActionItemModel model(&view);
  view.setAlternatingRowColors(true);
  view.setUniformRowHeights(true);
  QPushButton button("Do Next Action", &mainWindow);

  Engine engine(&edit, &registry);

  // Create the various actions and hotkeys before the model is set
  engine.addAction(registry, "Tools", "SuperForm", &engine, SLOT(superForm()));
  engine.addAction(registry, "Tools", "Range Tool", &engine, SLOT(rangeTool()));
  engine.addAction(registry, "Tools", "GOG Editor", &engine, SLOT(gogEditor()));
  engine.addAction(registry, "Tools", "Preference Rules", &engine, SLOT(preferenceRules()));
  engine.addAction(registry, "Tools", "Legend Manager", &engine, SLOT(legendManager()));
  engine.addAction(registry, "Display", "Toggle Labels", &engine, SLOT(toggleLabels()));
  engine.addAction(registry, "Display", "Toggle Dynamic Scale", &engine, SLOT(toggleDynamicScale()));
  engine.addAction(registry, "Display", "Toggle Platforms", &engine, SLOT(togglePlatforms()));
  engine.addAction(registry, "Display", "Toggle Beams", &engine, SLOT(toggleBeams()));
  engine.addAction(registry, "Map", "Terrain Editor", &engine, SLOT(terrainEditor()));
  engine.addAction(registry, "Help", "About", &engine, SLOT(helpAbout()));
  engine.addAction(registry, "Tools", "Hotkey Editor", &engine, SLOT(hotkeyEditor()));
  engine.addAction(registry, "Foobar", "Baz", &engine, SLOT(baz()), QList<QKeySequence>() << QKeySequence("A"));
  engine.addAction(registry, "Foobar", "Baz 2", &engine, SLOT(baz2()), QList<QKeySequence>() << QKeySequence("Alt+Q") << QKeySequence("Ctrl+Shift+F4"));

  model.setRegistry(&registry);
  view.setModel(&model);
  simQt::ActionItemModelDelegate editor(&view);
  view.setItemDelegate(&editor);
  view.expandAll();

  QObject::connect(&model, SIGNAL(groupAdded(QModelIndex)), &view, SLOT(expand(QModelIndex)));
  QObject::connect(&button, SIGNAL(pressed()), &engine, SLOT(doNext()));

  QVBoxLayout layout(&mainWindow);
  mainWindow.setLayout(&layout);
  layout.addWidget(&view);
  layout.addWidget(&edit);
  layout.addWidget(&button);

  mainWindow.show();
  return app.exec();
}

