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
#ifndef SIMQT_FILLITEMMODELWITHNODEVISITOR_H
#define SIMQT_FILLITEMMODELWITHNODEVISITOR_H

#include <deque>
#include <QList>
#include <QString>
#include "osg/NodeVisitor"
#include "simCore/Common/Export.h"

class QStandardItem;
class QStandardItemModel;
namespace osg {
  class StateSet;
  class Uniform;
}

namespace simQt {

/**
 * Simple NodeVisitor that, given a node, will call out the various apply__() methods to
 * process the StateSet.  This is intended to help give a framework for iterating through
 * various properties of a StateSet.  It is intended that the developer will subclass this
 * and override applyRenderBin(), applyMode(), etc. as required.
 *
 * By default, this visitor acts only on the node being accepted, and does not recurse.
 */
class SDKQT_EXPORT StateSetVisitor : public osg::NodeVisitor
{
public:
  /** Construct a visitor.  Set to TRAVERSE_NONE by default so that only passed-in node is visited. */
  explicit StateSetVisitor(osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_NONE);

  /** Override from osg::NodeVisitor */
  virtual void apply(osg::Node& node);

  // Override these methods in your StateSetVisitor instance.

  /** Process the render bin details for the stateset */
  virtual void applyRenderBin(osg::StateSet& stateSet, int binNumber, const std::string& binName, osg::StateSet::RenderBinMode binMode, bool nestedBins);
  /** Process a single mode on the stateset */
  virtual void applyMode(osg::StateSet& stateSet, unsigned int mode, unsigned int value);
  /** Process a single attribute on the stateset */
  virtual void applyAttribute(osg::StateSet& stateSet, osg::StateAttribute& attrib, unsigned int value);
  /** Process a single texture mode on the stateset */
  virtual void applyTextureMode(osg::StateSet& stateSet, unsigned int unit, unsigned int mode, unsigned int value);
  /** Process a single texture attribute on the stateset */
  virtual void applyTextureAttribute(osg::StateSet& stateSet, unsigned int unit, osg::StateAttribute& attrib, unsigned int value);
  /** Process a single uniform on the stateset */
  virtual void applyUniform(osg::StateSet& stateSet, osg::Uniform& uniform, unsigned int value);
  /** Process a single define on the stateset */
  virtual void applyDefine(osg::StateSet& stateSet, const std::string& name, const std::string& definition, unsigned int value);

  /** Given a render bin's mode value (USE_RENDERBIN_DETAILS, etc), convert to a human-readable string. */
  static std::string renderBinModeToString(osg::StateSet::RenderBinMode binMode);
  /** Given an OpenGL mode enumeration value (GL_BLEND, etc.), convert to a human-readable string. */
  static std::string modeToString(unsigned int mode);
  /** Given a mode, attribute, uniform, or define's value (ON, OFF, etc), convert to a human-readable string. */
  static std::string valueToString(unsigned int value);
  /** Given a uniform, convert the value into a human-readable string. */
  static std::string uniformToString(const osg::Uniform& uniform);
};

/** Instance of StateSetVisitor used by FillItemModelWithNodeVisitor to fill out a QStandardItem */
class SDKQT_EXPORT FillTreeStateSetVisitor : public StateSetVisitor
{
public:
  explicit FillTreeStateSetVisitor(QStandardItem& parent, osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_NONE);

  // Overrides from StateSetVisitor

  /** Adds the render bin details only if not inheriting */
  virtual void applyRenderBin(osg::StateSet& stateSet, int binNumber, const std::string& binName, osg::StateSet::RenderBinMode binMode, bool nestedBins);
  /** Creates a Modes sub-tree and adds the parameters to it. */
  virtual void applyMode(osg::StateSet& stateSet, unsigned int mode, unsigned int value);
  /** Creates an Attributes sub-tree and adds the parameters to it. */
  virtual void applyAttribute(osg::StateSet& stateSet, osg::StateAttribute& attrib, unsigned int value);
  /** Creates a Texture Modes sub-tree and the texture unit and adds the parameters to it. */
  virtual void applyTextureMode(osg::StateSet& stateSet, unsigned int unit, unsigned int mode, unsigned int value);
  /** Creates a Texture Attributes sub-tree for the texture unit and adds the parameters to it. */
  virtual void applyTextureAttribute(osg::StateSet& stateSet, unsigned int unit, osg::StateAttribute& attrib, unsigned int value);
  /** Creates a Uniforms sub-tree and adds the parameters to it. */
  virtual void applyUniform(osg::StateSet& stateSet, osg::Uniform& uniform, unsigned int value);
  /** Creates a Defines sub-tree and adds the parameters to it. */
  virtual void applyDefine(osg::StateSet& stateSet, const std::string& name, const std::string& definition, unsigned int value);

private:
  /** Helper method to add a row to the given parent */
  QList<QStandardItem*> appendRow_(QStandardItem& parent, const QString& column1, const QString& column2, const QString& column3) const;
  /** Factory method for a QStandardItem that is not editable. If parent is non-NULL, add to parent. */
  QStandardItem* newStandardItem_(QStandardItem* parent, const QString& title) const;
  /** Finds or creates an item child with the given name, always adding to parent if not existing. */
  QStandardItem* getOrCreateStandardItem_(QStandardItem& parent, const QString& title) const;
  /** Given a mode value (ON, OFF, etc), colors the item appropriately */
  void colorizeItem_(QStandardItem& item, unsigned int mode) const;

  QStandardItem& parent_;
  QStandardItem* modes_;
  QStandardItem* attributes_;
  QStandardItem* textureModes_;
  QStandardItem* textureAttributes_;
  QStandardItem* uniforms_;
  QStandardItem* defines_;
};

/**
 * Visitor that populates a QStandardItemModel with nodes and state set information for debugging
 * OSG scenes.  Uses FillTreeStateSetVisitor instance internally to also fill out state values.
 * Example usage:
 *
 * <code>
 * QTreeView* treeView = new QTreeView;
 * QStandardItemModel* model = new QStandardItemModel(treeView);
 * simQt::FillItemModelWithNodeVisitor fillTree(model);
 * sceneNode->accept(fillTree);
 * treeView->setModel(model);
 * </code>
 */
class SDKQT_EXPORT FillItemModelWithNodeVisitor : public osg::NodeVisitor
{
public:
  explicit FillItemModelWithNodeVisitor(QStandardItemModel* model, osg::NodeVisitor::TraversalMode tm=osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

  /** From osg::NodeVisitor */
  virtual void apply(osg::Node& node);
  /** From osg::NodeVisitor */
  virtual void apply(osg::Group& group);

private:
  /** Builds a node and its stateset, adding it to the current stack item.  Returns the 0th item in row, for children. */
  QStandardItem* appendNode_(osg::Node& node) const;
  /** Appends the state set, if any, to the parent passed in. */
  void buildStateSetTree_(osg::Node& node, QStandardItem* item) const;
  /** Helper method to create the name item for a node.  Colors appropriately based on whether it's set. */
  QStandardItem* newNodeNameItem_(const osg::Node& node) const;

  /** Model being modified */
  QStandardItemModel* model_;
  /** Stack representing the current place in the tree for iteration */
  std::deque<QStandardItem*> stack_;
};

}

#endif /* SIMQT_FILLITEMMODELWITHNODEVISITOR_H */
