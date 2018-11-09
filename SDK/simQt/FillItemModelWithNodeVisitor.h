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

namespace simQt {

/** Visitor that populates a QStandardItemModel with nodes and state set information for debugging OSG scenes. */
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
  QStandardItem* appendNode_(const osg::Node& node) const;
  /** Appends the state set, if any, to the parent passed in. */
  void buildStateSetTree_(const osg::StateSet* state, QStandardItem* item) const;

  /** Given a mode value (ON, OFF, etc), colors the item appropriately */
  void colorizeColumn_(QStandardItem& item, unsigned int mode) const;

  /** Retrieves the text string for a Uniform's value */
  QString uniformString_(const osg::Uniform& uniform) const;
  /** Returns the string representation of the state value (ON, OFF, etc) */
  QString stateValueString_(unsigned int value) const;
  /** Returns the string for the render mode.  Similar to stateValueString_(), but for the render bin enum. */
  QString renderBinModeString_(unsigned int mode) const;
  /** Returns the mode string, such as GL_BLEND, GL_CLIP_PLANE0, etc. */
  QString modeString_(unsigned int mode) const;

  /** Helper method to create the name item for a node.  Colors appropriately based on whether it's set. */
  QStandardItem* newNodeNameItem_(const osg::Node& node) const;

  /** Helper method to add a row to the given parent */
  QList<QStandardItem*> appendRow_(QStandardItem& parent, const QString& column1, const QString& column2, const QString& column3) const;

  /** Model being modified */
  QStandardItemModel* model_;
  /** Stack representing the current place in the tree for iteration */
  std::deque<QStandardItem*> stack_;
};

}

#endif /* SIMQT_FILLITEMMODELWITHNODEVISITOR_H */
