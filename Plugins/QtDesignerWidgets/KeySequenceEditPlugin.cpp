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
#include <QtCore/QtPlugin>
#include "KeySequenceEditPlugin.h"
#include "simQt/KeySequenceEdit.h"

KeySequenceEditPlugin::KeySequenceEditPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void KeySequenceEditPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool KeySequenceEditPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *KeySequenceEditPlugin::createWidget(QWidget *parent)
{
  return new simQt::KeySequenceEdit(parent);
}

QString KeySequenceEditPlugin::name() const
{
  return "simQt::KeySequenceEdit";
}

QString KeySequenceEditPlugin::group() const
{
  return "simQt";
}

QIcon KeySequenceEditPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Keyboard.png");
}

QString KeySequenceEditPlugin::toolTip() const
{
  return "Key sequence editor";
}

QString KeySequenceEditPlugin::whatsThis() const
{
  return "Key sequence editor";
}

bool KeySequenceEditPlugin::isContainer() const
{
  return false;
}

QString KeySequenceEditPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Key Sequence Edit\">"
    "<widget class=\"simQt::KeySequenceEdit\" name=\"keySequenceEdit\">\n"
    "</widget>\n"
    "</ui>";
}

QString KeySequenceEditPlugin::includeFile() const
{
  return "simQt/KeySequenceEdit.h";
}

