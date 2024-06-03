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
#ifndef ENTITY_TREE_COMPOSITE_PLUGIN_H
#define ENTITY_TREE_COMPOSITE_PLUGIN_H

#include <QDesignerCustomWidgetInterface>
#include "simQt/AbstractEntityTreeModel.h"

// Wrapper class for the FileSelectorWidget to provide QDesignerCustomWidgetInterface
class EntityTreeCompositePlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
  explicit EntityTreeCompositePlugin(QObject *parent = 0);

  bool isContainer() const;
  bool isInitialized() const;
  QIcon icon() const;
  QString domXml() const;
  QString group() const;
  QString includeFile() const;
  QString name() const;
  QString toolTip() const;
  QString whatsThis() const;
  QWidget *createWidget(QWidget *parent);
  void initialize(QDesignerFormEditorInterface *core);

private:
  bool initialized;
};

/** The only purpose of this TreeModel is to make the columns appear in Qt Designer */
class QtDesignerDisplayTree : public simQt::AbstractEntityTreeModel
{
  Q_OBJECT

public:
  explicit QtDesignerDisplayTree(QObject* parent);
  virtual ~QtDesignerDisplayTree() {}


  /** Remove an entity from the tree via its ID */
  virtual void removeTreeItem(uint64_t id) {}

  virtual int columnCount(const QModelIndex &parent) const { return 3; }
  virtual QVariant data(const QModelIndex &index, int role) const { return ""; }
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const { return QModelIndex(); }
  virtual QModelIndex index(uint64_t id) const { return QModelIndex(); }
  virtual QModelIndex index(uint64_t id) { return QModelIndex(); }
  virtual uint64_t uniqueId(const QModelIndex &index) const { return 0; }
  virtual QModelIndex parent(const QModelIndex &index) const { return QModelIndex(); }
  virtual int rowCount(const QModelIndex &parent) const { return 0; }
  virtual bool useEntityIcons() const { return useEntityIcons_; }
  virtual int countEntityTypes(simData::ObjectType type) const { return 0; }
  virtual QAbstractItemView::SelectionMode selectionMode() const { return selectionMode_; }
  virtual bool useCenterAction() const { return useCenterAction_; }
  virtual bool expandsOnDoubleClick() const { return expandsOnDoubleClick_; }

public Q_SLOTS:
  /** Swaps the view to the hierarchy tree */
  virtual void setToTreeView() {}
  /** Swaps the view to a non-hierarchical list */
  virtual void setToListView() {}
  /** Swaps between tree and list view based on a Boolean */
  virtual void toggleTreeView(bool useTree) {}
  /** Updates the contents of the frame */
  virtual void forceRefresh() {}

  /** Turns on or off entity icons */
  virtual void setUseEntityIcons(bool useIcons) { useEntityIcons_ = useIcons; }

private:
  bool useEntityIcons_;
  QAbstractItemView::SelectionMode selectionMode_;
  bool useCenterAction_;
  bool expandsOnDoubleClick_;
};

#endif // ENTITY_TREE_COMPOSITE_PLUGIN_H

