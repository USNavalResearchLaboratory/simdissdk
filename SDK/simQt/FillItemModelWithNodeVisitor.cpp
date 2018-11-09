// -*- mode: c++ -*-
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
*               EW Modeling & Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*/
#include <deque>
#include <QStandardItemModel>
#include "osg/NodeVisitor"
#include "osg/Group"
#include "osgDB/Registry"
#include "simQt/FillItemModelWithNodeVisitor.h"

namespace simQt {

FillItemModelWithNodeVisitor::FillItemModelWithNodeVisitor(QStandardItemModel* model, osg::NodeVisitor::TraversalMode tm)
  : NodeVisitor(tm),
    model_(model)
{
  if (model_)
  {
    model_->setColumnCount(3);
    model_->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("Name")));
    model_->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("Data")));
    model_->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("Value")));
  }
}

void FillItemModelWithNodeVisitor::apply(osg::Node& node)
{
  // Skip traversal if we have no model
  if (!model_)
    return;
  appendNode_(node);
  traverse(node);
}

void FillItemModelWithNodeVisitor::apply(osg::Group& group)
{
  // Skip traversal if we have no model
  if (!model_)
    return;
  stack_.push_back(appendNode_(group));
  traverse(group);
  stack_.pop_back();
}

QStandardItem* FillItemModelWithNodeVisitor::appendNode_(const osg::Node& node) const
{
  QList<QStandardItem*> row;
  row.push_back(newNodeNameItem_(node));
  row.push_back(new QStandardItem(node.className()));
  row.push_back(new QStandardItem(node.getNodeMask() == 0 ? QObject::tr("OFF") : ""));
  for (auto i = row.begin(); i != row.end(); ++i)
    (*i)->setEditable(false);

  if (stack_.empty())
    model_->appendRow(row);
  else
    (*stack_.rbegin())->appendRow(row);
  buildStateSetTree_(node.getStateSet(), row[0]);
  return row[0];
}

void FillItemModelWithNodeVisitor::buildStateSetTree_(const osg::StateSet* state, QStandardItem* item) const
{
  if (!state || !item)
    return;
  QStandardItem* parent = new QStandardItem(QObject::tr("State Set"));
  parent->setEditable(false);
  parent->setForeground(Qt::darkBlue);
  if (state->useRenderBinDetails())
  {
    const QString renderBinModeString = renderBinModeString_(state->getRenderBinMode());
    appendRow_(*parent, QObject::tr("Bin Number"), QString::number(state->getBinNumber()), renderBinModeString);
    appendRow_(*parent, QObject::tr("Bin Name"), QString::fromStdString(state->getBinName()), renderBinModeString);
  }
  // Call out unusual nesting
  if (!state->getNestRenderBins())
  {
    auto row = appendRow_(*parent, QObject::tr("Nested Bins"), QObject::tr("False"), "");
    for (auto i = row.begin(); i != row.end(); ++i)
      (*i)->setBackground(Qt::red);
  }

  // Modes
  auto& modes = state->getModeList();
  if (!modes.empty())
  {
    QStandardItem* modeParent = new QStandardItem(QObject::tr("Modes"));
    modeParent->setEditable(false);
    for (auto i = modes.begin(); i != modes.end(); ++i)
    {
      auto row = appendRow_(*modeParent, modeString_(i->first), "", stateValueString_(i->second));
      colorizeColumn_(*row[0], i->second);
    }
    parent->appendRow(modeParent);
  }

  // Attributes
  auto& attribs = state->getAttributeList();
  if (!attribs.empty())
  {
    QStandardItem* attribParent = new QStandardItem(QObject::tr("Attributes"));
    attribParent->setEditable(false);
    for (auto i = attribs.begin(); i != attribs.end(); ++i)
      appendRow_(*attribParent, QString::fromStdString(i->second.first->className()), "", stateValueString_(i->second.second));
    parent->appendRow(attribParent);
  }

  // Texture modes
  auto& textureModes = state->getTextureModeList();
  if (!textureModes.empty())
  {
    QStandardItem* modeParent = new QStandardItem(QObject::tr("Texture Modes"));
    modeParent->setEditable(false);
    unsigned int unit = 0;
    for (unit = 0; unit < textureModes.size(); ++unit)
    {
      const auto& modes = textureModes[unit];
      QStandardItem* unitParent = new QStandardItem(QObject::tr("Unit %1").arg(unit));
      for (auto i = modes.begin(); i != modes.end(); ++i)
      {
        auto row = appendRow_(*modeParent, modeString_(i->first), "", stateValueString_(i->second));
        colorizeColumn_(*row[0], i->second);
      }

      // Only add if the unit has values
      if (unitParent->rowCount() == 0)
        delete unitParent;
      else
        modeParent->appendRow(unitParent);
    }
    parent->appendRow(modeParent);
  }

  // Texture attributes
  auto& textureAttribs = state->getTextureAttributeList();
  if (!textureAttribs.empty())
  {
    QStandardItem* attribParent = new QStandardItem(QObject::tr("Texture Attributes"));
    attribParent->setEditable(false);
    unsigned int unit = 0;
    for (unit = 0; unit < textureAttribs.size(); ++unit)
    {
      const auto& attribs = textureAttribs[unit];
      QStandardItem* unitParent = new QStandardItem(QObject::tr("Unit %1").arg(unit));
      unitParent->setEditable(false);
      for (auto i = attribs.begin(); i != attribs.end(); ++i)
        appendRow_(*attribParent, QString::fromStdString(i->second.first->className()), "", stateValueString_(i->second.second));

      // Only add if the unit has values
      if (unitParent->rowCount() == 0)
        delete unitParent;
      else
        attribParent->appendRow(unitParent);
    }
    parent->appendRow(attribParent);
  }

  // Uniforms
  auto& uniforms = state->getUniformList();
  if (!uniforms.empty())
  {
    QStandardItem* uniformParent = new QStandardItem(QObject::tr("Uniforms"));
    uniformParent->setEditable(false);
    for (auto i = uniforms.begin(); i != uniforms.end(); ++i)
      appendRow_(*uniformParent, QString::fromStdString(i->first), uniformString_(*i->second.first), stateValueString_(i->second.second));
    parent->appendRow(uniformParent);
  }

  // Defines
  auto& defines = state->getDefineList();
  if (!defines.empty())
  {
    QStandardItem* defineParent = new QStandardItem(QObject::tr("Defines"));
    defineParent->setEditable(false);
    for (auto i = defines.begin(); i != defines.end(); ++i)
    {
      auto row = appendRow_(*defineParent, QString::fromStdString(i->first), QString::fromStdString(i->second.first), stateValueString_(i->second.second));
      colorizeColumn_(*row[0], i->second.second);
    }
    parent->appendRow(defineParent);
  }

  // Add the item
  if (parent->rowCount() == 0)
    parent->setBackground(Qt::red);
  item->appendRow(parent);
}

QString FillItemModelWithNodeVisitor::uniformString_(const osg::Uniform& uniform) const
{
  if (uniform.getNumElements() == 0)
    return "";
  QString rv;
  if (uniform.getNumElements() != 1)
    rv = "[ ";

  if (uniform.getFloatArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getFloatArray()->at(k));
  }

  else if (uniform.getDoubleArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getDoubleArray()->at(k));
  }

  else if (uniform.getIntArray())
  {
    // Handle special case of booleans
    if (uniform.getType() == osg::Uniform::BOOL || uniform.getType() == osg::Uniform::BOOL_VEC2 ||
      uniform.getType() == osg::Uniform::BOOL_VEC3 || uniform.getType() == osg::Uniform::BOOL_VEC4)
    {
      for (size_t k = 0; k < uniform.getNumElements(); ++k)
        rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getIntArray()->at(k) == 0 ? QObject::tr("False") : QObject::tr("True"));
    }
    else
    {
      for (size_t k = 0; k < uniform.getNumElements(); ++k)
        rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getIntArray()->at(k));
    }
  }

  else if (uniform.getUIntArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getUIntArray()->at(k));
  }

  else if (uniform.getUInt64Array())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getUInt64Array()->at(k));
  }

  else if (uniform.getInt64Array())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      rv += QString("%1%2").arg(k == 0 ? "" : ", ").arg(uniform.getInt64Array()->at(k));
  }

  else
    return QObject::tr("Unknown Values");

  if (uniform.getNumElements() != 1)
    rv += " ]";
  return rv;
}

void FillItemModelWithNodeVisitor::colorizeColumn_(QStandardItem& item, unsigned int mode) const
{
  item.setForeground(((mode & osg::StateAttribute::ON) != 0) ? Qt::darkGreen : Qt::darkRed);
}

QString FillItemModelWithNodeVisitor::stateValueString_(unsigned int value) const
{
  QString str;
  if (value & osg::StateAttribute::ON)
    str = QObject::tr("ON");
  else
    str = QObject::tr("OFF");
  if (value & osg::StateAttribute::OVERRIDE)
    str += QObject::tr(" | OVERRIDE");
  if (value & osg::StateAttribute::PROTECTED)
    str += QObject::tr(" | PROTECTED");
  if (value & osg::StateAttribute::INHERIT)
    str += QObject::tr(" | INHERIT");
  return str;
}

QString FillItemModelWithNodeVisitor::renderBinModeString_(unsigned int mode) const
{
  if (mode == osg::StateSet::INHERIT_RENDERBIN_DETAILS)
    return QObject::tr("Inherit");
  QString rv = QObject::tr("ON");
  if (mode & osg::StateSet::OVERRIDE_RENDERBIN_DETAILS)
    rv += QObject::tr(" | OVERRIDE");
  if (mode & osg::StateSet::PROTECTED_RENDERBIN_DETAILS)
    rv += QObject::tr(" | PROTECTED");
  return rv;
}

QString FillItemModelWithNodeVisitor::modeString_(unsigned int mode) const
{
  const std::string glString = osgDB::Registry::instance()->getObjectWrapperManager()->getString("GL", mode);
  return QString::fromStdString(glString);
}

QStandardItem* FillItemModelWithNodeVisitor::newNodeNameItem_(const osg::Node& node) const
{
  QStandardItem* newItem = new QStandardItem();
  if (node.getName().empty())
  {
    newItem->setText(QObject::tr("[none]"));
    newItem->setForeground(Qt::darkGray);
  }
  else
    newItem->setText(QString::fromStdString(node.getName()));
  newItem->setEditable(false);
  return newItem;
}

QList<QStandardItem*> FillItemModelWithNodeVisitor::appendRow_(QStandardItem& parent, const QString& column1, const QString& column2, const QString& column3) const
{
  QList<QStandardItem*> row;
  row.push_back(new QStandardItem(column1));
  row.push_back(new QStandardItem(column2));
  row.push_back(new QStandardItem(column3));
  for (auto i = row.begin(); i != row.end(); ++i)
    (*i)->setEditable(false);
  parent.appendRow(row);
  return row;
}

}
