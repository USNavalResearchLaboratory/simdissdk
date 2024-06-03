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
#include <deque>
#include <QStandardItemModel>
#include "osg/NodeVisitor"
#include "osg/Group"
#include "osgDB/Registry"
#include "simQt/FillItemModelWithNodeVisitor.h"

namespace simQt {

StateSetVisitor::StateSetVisitor(osg::NodeVisitor::TraversalMode tm)
  : NodeVisitor(tm)
{
}

void StateSetVisitor::apply(osg::Node& node)
{
  if (!node.getStateSet())
  {
    traverse(node);
    return;
  }

  // Render Bin
  osg::StateSet& stateSet = *node.getStateSet();
  applyRenderBin(stateSet, stateSet.getBinNumber(), stateSet.getBinName(), stateSet.getRenderBinMode(), stateSet.getNestRenderBins());

  // Modes
  for (auto i = stateSet.getModeList().begin(); i != stateSet.getModeList().end(); ++i)
    applyMode(stateSet, i->first, i->second);

  // Attributes
  for (auto i = stateSet.getAttributeList().begin(); i != stateSet.getAttributeList().end(); ++i)
  {
    if (i->second.first.valid())
      applyAttribute(stateSet, *i->second.first, i->second.second);
  }

  // Texture Modes
  for (unsigned int unit = 0; unit < stateSet.getTextureModeList().size(); ++unit)
  {
    const auto& modes = stateSet.getTextureModeList()[unit];
    for (auto i = modes.begin(); i != modes.end(); ++i)
      applyTextureMode(stateSet, unit, i->first, i->second);
  }

  // Texture attributes
  for (unsigned int unit = 0; unit < stateSet.getTextureAttributeList().size(); ++unit)
  {
    const auto& attribs = stateSet.getTextureAttributeList()[unit];
    for (auto i = attribs.begin(); i != attribs.end(); ++i)
    {
      if (i->second.first.valid())
        applyTextureAttribute(stateSet, unit, *i->second.first, i->second.second);
    }
  }

  // Uniforms
  for (auto i = stateSet.getUniformList().begin(); i != stateSet.getUniformList().end(); ++i)
  {
    if (i->second.first.valid())
      applyUniform(stateSet, *i->second.first, i->second.second);
  }

  // Defines
  for (auto i = stateSet.getDefineList().begin(); i != stateSet.getDefineList().end(); ++i)
    applyDefine(stateSet, i->first, i->second.first, i->second.second);
}

void StateSetVisitor::applyRenderBin(osg::StateSet& stateSet, int binNumber, const std::string& binName, osg::StateSet::RenderBinMode binMode, bool nestedBins)
{
  // noop
}

void StateSetVisitor::applyMode(osg::StateSet& stateSet, unsigned int mode, unsigned int value)
{
  // noop
}

void StateSetVisitor::applyAttribute(osg::StateSet& stateSet, osg::StateAttribute& attrib, unsigned int value)
{
  // noop
}

void StateSetVisitor::applyTextureMode(osg::StateSet& stateSet, unsigned int unit, unsigned int mode, unsigned int value)
{
  // noop
}

void StateSetVisitor::applyTextureAttribute(osg::StateSet& stateSet, unsigned int unit, osg::StateAttribute& attrib, unsigned int value)
{
  // noop
}

void StateSetVisitor::applyUniform(osg::StateSet& stateSet, osg::Uniform& uniform, unsigned int value)
{
  // noop
}

void StateSetVisitor::applyDefine(osg::StateSet& stateSet, const std::string& name, const std::string& definition, unsigned int value)
{
  // noop
}

std::string StateSetVisitor::renderBinModeToString(osg::StateSet::RenderBinMode binMode)
{
  if (binMode == osg::StateSet::INHERIT_RENDERBIN_DETAILS)
    return "Inherit";
  std::stringstream rv;
  rv << "ON";
  if (binMode & osg::StateSet::OVERRIDE_RENDERBIN_DETAILS)
    rv << " | OVERRIDE";
  if (binMode & osg::StateSet::PROTECTED_RENDERBIN_DETAILS)
    rv << " | PROTECTED";
  return rv.str();
}

std::string StateSetVisitor::modeToString(unsigned int mode)
{
  // Reuse the registry
  return osgDB::Registry::instance()->getObjectWrapperManager()->getString("GL", mode);
}

std::string StateSetVisitor::valueToString(unsigned int value)
{
  std::stringstream str;
  if (value & osg::StateAttribute::ON)
    str << "ON";
  else
    str << "OFF";
  if (value & osg::StateAttribute::OVERRIDE)
    str << " | OVERRIDE";
  if (value & osg::StateAttribute::PROTECTED)
    str << " | PROTECTED";
  if (value & osg::StateAttribute::INHERIT)
    str << " | INHERIT";
  return str.str();
}

std::string StateSetVisitor::uniformToString(const osg::Uniform& uniform)
{
  if (uniform.getNumElements() == 0)
    return "";
  std::stringstream ss;
  // Surround arrays with [ ]
  if (uniform.getNumElements() != 1)
    ss << "[ ";

  // Break out by data type
  if (uniform.getFloatArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      ss << (k == 0 ? "" : ", ") << uniform.getFloatArray()->at(k);
  }

  else if (uniform.getDoubleArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      ss << (k == 0 ? "" : ", ") << uniform.getDoubleArray()->at(k);
  }

  else if (uniform.getIntArray())
  {
    // Handle special case of booleans
    if (uniform.getType() == osg::Uniform::BOOL || uniform.getType() == osg::Uniform::BOOL_VEC2 ||
      uniform.getType() == osg::Uniform::BOOL_VEC3 || uniform.getType() == osg::Uniform::BOOL_VEC4)
    {
      for (size_t k = 0; k < uniform.getNumElements(); ++k)
        ss << (k == 0 ? "" : ", ") << (uniform.getIntArray()->at(k) == 0 ? "False" : "True");
    }
    else
    {
      for (size_t k = 0; k < uniform.getNumElements(); ++k)
        ss << (k == 0 ? "" : ", ") << uniform.getIntArray()->at(k);
    }
  }

  else if (uniform.getUIntArray())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      ss << (k == 0 ? "" : ", ") << uniform.getUIntArray()->at(k);
  }

  else if (uniform.getUInt64Array())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      ss << (k == 0 ? "" : ", ") << uniform.getUInt64Array()->at(k);
  }

  else if (uniform.getInt64Array())
  {
    for (size_t k = 0; k < uniform.getNumElements(); ++k)
      ss << (k == 0 ? "" : ", ") << uniform.getInt64Array()->at(k);
  }

  else
    return "Unknown Values";

  if (uniform.getNumElements() != 1)
    ss << " ]";
  return ss.str();
}


///////////////////////////////////////////////////////////////

FillTreeStateSetVisitor::FillTreeStateSetVisitor(QStandardItem& parent, osg::NodeVisitor::TraversalMode tm)
  : StateSetVisitor(tm),
    parent_(parent),
    modes_(nullptr),
    attributes_(nullptr),
    textureModes_(nullptr),
    textureAttributes_(nullptr),
    uniforms_(nullptr),
    defines_(nullptr)
{
}

void FillTreeStateSetVisitor::applyRenderBin(osg::StateSet& stateSet, int binNumber, const std::string& binName, osg::StateSet::RenderBinMode binMode, bool nestedBins)
{
  if (binMode != osg::StateSet::INHERIT_RENDERBIN_DETAILS)
  {
    const QString renderBinModeString = QString::fromStdString(StateSetVisitor::renderBinModeToString(binMode));
    appendRow_(parent_, QObject::tr("Bin Number"), QString::number(binNumber), renderBinModeString);
    appendRow_(parent_, QObject::tr("Bin Name"), QString::fromStdString(binName), renderBinModeString);
  }
  // Call out unusual nesting
  if (!nestedBins)
  {
    auto row = appendRow_(parent_, QObject::tr("Nested Bins"), QObject::tr("False"), "");
    for (auto i = row.begin(); i != row.end(); ++i)
      (*i)->setBackground(Qt::red);
  }
}

void FillTreeStateSetVisitor::applyMode(osg::StateSet& stateSet, unsigned int mode, unsigned int value)
{
  if (!modes_)
    modes_ = newStandardItem_(&parent_, QObject::tr("Modes"));
  auto row = appendRow_(*modes_,
    QString::fromStdString(StateSetVisitor::modeToString(mode)),
    "",
    QString::fromStdString(StateSetVisitor::valueToString(value)));
  colorizeItem_(*row[0], value);
}

void FillTreeStateSetVisitor::applyAttribute(osg::StateSet& stateSet, osg::StateAttribute& attrib, unsigned int value)
{
  if (!attributes_)
    attributes_ = newStandardItem_(&parent_, QObject::tr("Attributes"));
  appendRow_(*attributes_,
    QString::fromStdString(attrib.className()),
    "",
    QString::fromStdString(StateSetVisitor::valueToString(value)));
}

void FillTreeStateSetVisitor::applyTextureMode(osg::StateSet& stateSet, unsigned int unit, unsigned int mode, unsigned int value)
{
  if (!textureModes_)
    textureModes_ = newStandardItem_(&parent_, QObject::tr("Texture Modes"));
  QStandardItem* rowParent = getOrCreateStandardItem_(*textureModes_, QObject::tr("Unit %1").arg(unit));
  auto row = appendRow_(*rowParent,
    QString::fromStdString(StateSetVisitor::modeToString(mode)),
    "",
    QString::fromStdString(StateSetVisitor::valueToString(value)));
  colorizeItem_(*row[0], value);
}

void FillTreeStateSetVisitor::applyTextureAttribute(osg::StateSet& stateSet, unsigned int unit, osg::StateAttribute& attrib, unsigned int value)
{
  if (!textureAttributes_)
    textureAttributes_ = newStandardItem_(&parent_, QObject::tr("Texture Attributes"));
  QStandardItem* rowParent = getOrCreateStandardItem_(*textureAttributes_, QObject::tr("Unit %1").arg(unit));
  appendRow_(*rowParent,
    QString::fromStdString(attrib.className()),
    "",
    QString::fromStdString(StateSetVisitor::valueToString(value)));
}

void FillTreeStateSetVisitor::applyUniform(osg::StateSet& stateSet, osg::Uniform& uniform, unsigned int value)
{
  if (!uniforms_)
    uniforms_ = newStandardItem_(&parent_, QObject::tr("Uniforms"));
  appendRow_(*uniforms_,
    QString::fromStdString(uniform.getName()),
    QString::fromStdString(StateSetVisitor::uniformToString(uniform)),
    QString::fromStdString(StateSetVisitor::valueToString(value)));
}

void FillTreeStateSetVisitor::applyDefine(osg::StateSet& stateSet, const std::string& name, const std::string& definition, unsigned int value)
{
  if (!defines_)
    defines_ = newStandardItem_(&parent_, QObject::tr("Defines"));
  appendRow_(*defines_,
    QString::fromStdString(name),
    QString::fromStdString(definition),
    QString::fromStdString(StateSetVisitor::valueToString(value)));
}

QList<QStandardItem*> FillTreeStateSetVisitor::appendRow_(QStandardItem& parent, const QString& column1, const QString& column2, const QString& column3) const
{
  QList<QStandardItem*> row;
  row.push_back(newStandardItem_(nullptr, column1));
  row.push_back(newStandardItem_(nullptr, column2));
  row.push_back(newStandardItem_(nullptr, column3));
  parent.appendRow(row);
  return row;
}

QStandardItem* FillTreeStateSetVisitor::newStandardItem_(QStandardItem* parent, const QString& title) const
{
  QStandardItem* item = new QStandardItem(title);
  item->setEditable(false);
  if (parent)
    parent->appendRow(item);
  return item;
}

QStandardItem* FillTreeStateSetVisitor::getOrCreateStandardItem_(QStandardItem& parent, const QString& title) const
{
  for (int row = 0; row < parent.rowCount(); ++row)
  {
    QStandardItem* child = parent.child(row, 0);
    if (child && child->text() == title)
      return child;
  }
  return newStandardItem_(&parent, title);
}

void FillTreeStateSetVisitor::colorizeItem_(QStandardItem& item, unsigned int mode) const
{
  item.setForeground(((mode & osg::StateAttribute::ON) != 0) ? Qt::darkGreen : Qt::darkRed);
}

///////////////////////////////////////////////////////////////

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

QStandardItem* FillItemModelWithNodeVisitor::appendNode_(osg::Node& node) const
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

  // Build the state tree
  buildStateSetTree_(node, row[0]);
  return row[0];
}

void FillItemModelWithNodeVisitor::buildStateSetTree_(osg::Node& node, QStandardItem* item) const
{
  if (!node.getStateSet() || !item)
    return;
  QStandardItem* parent = new QStandardItem(QObject::tr("State Set"));
  parent->setEditable(false);
  parent->setForeground(Qt::darkBlue);

  FillTreeStateSetVisitor fillState(*parent);
  node.accept(fillState);

  // Add the item
  item->appendRow(parent);
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

}
