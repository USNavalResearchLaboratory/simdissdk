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
#ifndef SDKEXAMPLE_ACTIONITEMMODELTEST_H
#define SDKEXAMPLE_ACTIONITEMMODELTEST_H

#include <QObject>
#include <QLineEdit>
#include <QList>
#include <QKeySequence>
#include "simQt/ActionRegistry.h"

class Engine : public QObject
{
  Q_OBJECT;
public:
  Engine(QLineEdit* edit, simQt::ActionRegistry* registry)
    : edit_(edit),
      registry_(registry),
      nextAction_(0)
  {
  }
  void addAction(simQt::ActionRegistry& registry, const QString& group, const QString& desc,
    QObject* target, const QString& slot, QList<QKeySequence> shortcuts = QList<QKeySequence>());

public Q_SLOTS:
  // Responds to a cyclical button press
  void doNext();
  void superForm() { edit_->setText("superForm"); }
  void rangeTool() { edit_->setText("rangeTool"); }
  void gogEditor() { edit_->setText("gogEditor"); }
  void preferenceRules() { edit_->setText("preferenceRules"); }
  void legendManager() { edit_->setText("legendManager"); }
  void toggleLabels() { edit_->setText("toggleLabels"); }
  void toggleDynamicScale() { edit_->setText("toggleDynamicScale"); }
  void togglePlatforms() { edit_->setText("togglePlatforms"); }
  void toggleBeams() { edit_->setText("toggleBeams"); }
  void terrainEditor() { edit_->setText("terrainEditor"); }
  void helpAbout() { edit_->setText("helpAbout"); }
  void hotkeyEditor() { edit_->setText("hotkeyEditor"); }
  void baz() { edit_->setText("baz"); }
  void baz2() { edit_->setText("baz2"); }
  void newAction() { edit_->setText("newAction"); }
  void newGroup() { edit_->setText("newGroup"); }

private:
  QLineEdit* edit_;
  simQt::ActionRegistry* registry_;
  int nextAction_;
};

#endif /* SDKEXAMPLE_ACTIONITEMMODELTEST_H */

