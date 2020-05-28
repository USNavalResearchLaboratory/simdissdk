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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <QtCore/QtPlugin>
#include "simQt/EntityTreeComposite.h"
#include "EntityTreeCompositePlugin.h"

EntityTreeCompositePlugin::EntityTreeCompositePlugin(QObject *parent)
  : QObject(parent)
{
  initialized = false;
}

void EntityTreeCompositePlugin::initialize(QDesignerFormEditorInterface *)
{
  if (initialized)
    return;

  initialized = true;
}

bool EntityTreeCompositePlugin::isInitialized() const
{
  return initialized;
}

QWidget *EntityTreeCompositePlugin::createWidget(QWidget *parent)
{
  simQt::EntityTreeComposite* rv = new simQt::EntityTreeComposite(parent);
  rv->setModel(new QtDesignerDisplayTree(NULL));
  return rv;
}

QString EntityTreeCompositePlugin::name() const
{
  return "simQt::EntityTreeComposite";
}

QString EntityTreeCompositePlugin::group() const
{
  return "simQt";
}

QIcon EntityTreeCompositePlugin::icon() const
{
  return QIcon(":/SDKPlugins/images/Hierarchy_3.png");
}

QString EntityTreeCompositePlugin::toolTip() const
{
  return "Entity tree view with filtering";
}

QString EntityTreeCompositePlugin::whatsThis() const
{
  return "Entity tree view with filtering";
}

bool EntityTreeCompositePlugin::isContainer() const
{
  return false;
}

QString EntityTreeCompositePlugin::domXml() const
{
  return
    "<ui language=\"c++\" displayname=\"Entity Tree Widget\">"
    "<widget class=\"simQt::EntityTreeComposite\" name=\"entityTreeComposite\">\n"
    "</widget>\n"
    "</ui>";
}

QString EntityTreeCompositePlugin::includeFile() const
{
  return "simQt/EntityTreeComposite.h";
}

QtDesignerDisplayTree::QtDesignerDisplayTree(QObject* parent)
  : simQt::AbstractEntityTreeModel(parent),
    useEntityIcons_(true),
    selectionMode_(QAbstractItemView::ExtendedSelection),
    useCenterAction_(false),
    expandsOnDoubleClick_(true)
  {
  }

// Needed to make the column appear
QVariant QtDesignerDisplayTree::headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
  {
    if (section == 0)
       return "Name";
    if (section == 1)
      return "Type";
    if (section == 2)
      return "ID";

    return QVariant();
  }

  // Isn't the bar across the top -- fall back to whatever Qt does
  return QAbstractItemModel::headerData(section, orientation, role);
}

