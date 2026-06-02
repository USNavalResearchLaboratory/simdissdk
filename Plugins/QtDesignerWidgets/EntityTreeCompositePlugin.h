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
  explicit EntityTreeCompositePlugin(QObject *parent = nullptr);

  bool isContainer() const override;
  bool isInitialized() const override;
  QIcon icon() const override;
  QString domXml() const override;
  QString group() const override;
  QString includeFile() const override;
  QString name() const override;
  QString toolTip() const override;
  QString whatsThis() const override;
  QWidget *createWidget(QWidget *parent) override;
  void initialize(QDesignerFormEditorInterface *core) override;

private:
  bool initialized;
};

/** The only purpose of this TreeModel is to make the columns appear in Qt Designer */
class QtDesignerDisplayTree : public simQt::AbstractEntityTreeModel
{
  Q_OBJECT

public:
  explicit QtDesignerDisplayTree(QObject* parent);
  virtual ~QtDesignerDisplayTree() = default;


  /** Remove an entity from the tree via its ID */
  virtual void removeTreeItem(uint64_t id) {}

  int columnCount(const QModelIndex &parent) const override { return 3; }
  QVariant data(const QModelIndex &index, int role) const override { return ""; }
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent) const override { return QModelIndex(); }
  QModelIndex index(uint64_t id) const override { return QModelIndex(); }
  QModelIndex index(uint64_t id) override { return QModelIndex(); }
  uint64_t uniqueId(const QModelIndex &index) const override { return 0; }
  QModelIndex parent(const QModelIndex &index) const override { return QModelIndex(); }
  int rowCount(const QModelIndex &parent) const override { return 0; }
  bool useEntityIcons() const override { return useEntityIcons_; }
  int countEntityTypes(simData::ObjectType type) const override { return 0; }
  virtual QAbstractItemView::SelectionMode selectionMode() const { return selectionMode_; }
  virtual bool useCenterAction() const { return useCenterAction_; }
  virtual bool expandsOnDoubleClick() const { return expandsOnDoubleClick_; }

public Q_SLOTS:
  /** Swaps the view to the hierarchy tree */
  void setToTreeView() override {}
  /** Swaps the view to a non-hierarchical list */
  void setToListView() override {}
  /** Swaps between tree and list view based on a Boolean */
  void toggleTreeView(bool useTree) override {}
  /** Updates the contents of the frame */
  void forceRefresh() override {}

  /** Turns on or off entity icons */
  void setUseEntityIcons(bool useIcons) override { useEntityIcons_ = useIcons; }

private:
  bool useEntityIcons_;
  QAbstractItemView::SelectionMode selectionMode_;
  bool useCenterAction_;
  bool expandsOnDoubleClick_;
};

#endif // ENTITY_TREE_COMPOSITE_PLUGIN_H

