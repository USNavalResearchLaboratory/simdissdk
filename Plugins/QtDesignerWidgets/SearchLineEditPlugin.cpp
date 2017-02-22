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
#include <QtCore/QtPlugin>
#include "SearchLineEditPlugin.h"
#include "simQt/SearchLineEdit.h"

SearchLineEditPlugin::SearchLineEditPlugin(QObject *parent)
  : QObject(parent)
{
  initialized_ = false;
}

void SearchLineEditPlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized_)
    return;

  initialized_ = true;
}

bool SearchLineEditPlugin::isInitialized() const
{
  return initialized_;
}

QWidget *SearchLineEditPlugin::createWidget(QWidget *parent)
{
  return new simQt::SearchLineEdit(parent);
}

QString SearchLineEditPlugin::name() const
{
  return "simQt::SearchLineEdit";
}

QString SearchLineEditPlugin::group() const
{
  return "simQt";
}

QIcon SearchLineEditPlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Search.png");
}

QString SearchLineEditPlugin::toolTip() const
{
  return "Search-ready version of QLineEdit";
}

QString SearchLineEditPlugin::whatsThis() const
{
  return "Search-ready version of QLineEdit";
}

bool SearchLineEditPlugin::isContainer() const
{
  return false;
}

QString SearchLineEditPlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Search Line Edit\">"
    "<widget class=\"simQt::SearchLineEdit\" name=\"searchLineEdit\">\n"
    "</widget>\n"
    "</ui>";
}

QString SearchLineEditPlugin::includeFile() const
{
  return "simQt/SearchLineEdit.h";
}

