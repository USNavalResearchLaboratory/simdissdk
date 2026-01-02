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
#include <QDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <QSignalMapper>
#include <QClipboard>
#include "simQt/AbstractEntityTreeModel.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/EntityNameFilter.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/QtFormatting.h"
#include "simQt/QtUtils.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/SettingsGroup.h"
#include "simQt/WeightedMenuManager.h"
#include "simQt/WidgetSettings.h"
#include "simQt/EntityTreeComposite.h"
#include "ui_EntityTreeComposite.h"

namespace simQt {

const QString SETTING_NAME_FILTER = "/FilterSettings/";
const QString FILTER_DIALOG_GEOMETRY = "/FilterDialogGeometry";
const QString PINNED_CUSTOM_FILTER = "/PinnedCustomFilter";

FilterDialog::FilterDialog(SettingsPtr settings, QWidget* parent)
  :QDialog(parent),
   settings_(settings)
{
  setObjectName("Entity Tree Composite Filter Dialog");
  // Since the object saves its own geometry, skip having the Widget Setting save the geometry
  setProperty(DO_NOT_SAVE_GEOMETRY, true);

  // restore geometry if settings is valid
  if (settings_)
  {
    QVariant geom = settings_->value(FILTER_DIALOG_GEOMETRY);
    if (geom.isValid())
      restoreGeometry(geom.toByteArray());
  }
}

FilterDialog::~FilterDialog()
{
  // save geometry if settings is valid
  if (settings_)
    settings_->setValue(FILTER_DIALOG_GEOMETRY, saveGeometry());
}

void FilterDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  Q_EMIT(closedGui());
}


//-----------------------------------------------------------------------------------

/** Manages a single group of filter configurations associated with a tool button */
class EntityTreeComposite::ButtonActions
{
public:
  ButtonActions(QToolButton& button, const QIcon& icon)
    : loadAction_(new QAction(icon, tr("Load"), &button)),
      saveAction_(new QAction(QIcon(":simQt/images/Save.png"), tr("Save..."), &button)),
      clearAction_(new QAction(QIcon(":simQt/images/Delete.png"), tr("Clear"), &button)),
      pinAction_(new QAction(QIcon(":simQt/images/Push Pin.png"), tr("Pin"), &button)),
      button_(button)
  {
    // No tooltip needed for clear because it's never a standalone button via setDefaultButton()
    saveAction_->setToolTip(simQt::formatTooltip(tr("Save"), tr("Saves the current filter configuration to a button.")));
    clearAction_->setToolTip(simQt::formatTooltip(tr("Clear"), tr("Clears the button's filter configuration.")));
    pinAction_->setToolTip(simQt::formatTooltip(tr("Pin"), tr("Pins the button's filter configuration to persist in the display.")));
    setLoadTextAndTooltips_("");

    // We start without a filter configuration, so default mode is "save"

    QMenu* menu = new QMenu(&button_);
    menu->setDefaultAction(saveAction_);
    menu->addAction(loadAction_);
    menu->addAction(saveAction_);
    menu->addAction(clearAction_);
    menu->addAction(pinAction_);
    menu->setToolTipsVisible(true);
    button_.setMenu(menu);
  }

  QToolButton& button() const
  {
    return button_;
  }

  QAction* loadAction() const
  {
    return loadAction_;
  }

  QAction* saveAction() const
  {
    return saveAction_;
  }

  QAction* clearAction() const
  {
    return clearAction_;
  }

  QAction* pinAction() const
  {
    return pinAction_;
  }

  const FilterConfiguration& filterConfiguration() const
  {
    return filterConfig_;
  }

  QString description() const
  {
    return filterConfig_.description();
  }

  QString settingsKey() const
  {
    return SETTING_NAME_FILTER + button_.objectName();
  }

  void setFilterConfiguration(const FilterConfiguration& filter)
  {
    setLoadTextAndTooltips_(filter.description());
    if (filter.description().isEmpty())
    {
      loadAction_->setEnabled(false);
      clearAction_->setEnabled(false);
      pinAction_->setEnabled(false);
      button_.setDefaultAction(saveAction_);
    }
    else
    {
      loadAction_->setEnabled(true);
      clearAction_->setEnabled(true);
      pinAction_->setEnabled(true);
      button_.setDefaultAction(loadAction_);
    }
    filterConfig_ = filter;
  }

private:
  /** Declared but not defined to keep cppCheck warning free */
  ButtonActions(const ButtonActions& rhs);
  ButtonActions& operator=(ButtonActions& rhs);

  /** Sets the text and tooltip on the "Load" button */
  void setLoadTextAndTooltips_(const QString& filterName)
  {
    if (filterName.isEmpty())
    {
      loadAction_->setText(tr("Load"));
      loadAction_->setToolTip(simQt::formatTooltip(tr("Load"), tr("Loads the saved filter configuration.")));
    }
    else
    {
      loadAction_->setText(tr("Load %1").arg(filterName));
      loadAction_->setToolTip(simQt::formatTooltip(tr("Load"), tr("Loads the saved filter configuration: %1").arg(filterName)));
    }
  }

  QAction* loadAction_;
  QAction* saveAction_;
  QAction* clearAction_;
  QAction* pinAction_;
  QToolButton& button_;
  FilterConfiguration filterConfig_;
};

//-----------------------------------------------------------------------------------

/** Watch for setting changes for the buttons */
class EntityTreeComposite::Observer : public simQt::Settings::Observer
{
public:
  explicit Observer(EntityTreeComposite& parent)
    : parent_(parent)
  {
  }

  virtual ~Observer()
  {
  }

  virtual void onSettingChange(const QString& name, const QVariant& value)
  {
    ButtonActions* actions = nullptr;
    for (size_t index = 0; index < parent_.buttonActions_.size(); ++index)
    {
      if (parent_.buttonActions_[index]->settingsKey() == name)
      {
        actions = parent_.buttonActions_[index];

        // Set the filter
        auto filter = qvariant_cast<FilterConfiguration>(value);
        actions->setFilterConfiguration(filter);
        return;
      }
    }

    // Settings are being changed, but we don't have buttonActions_ for them
    assert(0);
  }

private:
  EntityTreeComposite& parent_;
};

//-----------------------------------------------------------------------------------

EntityTreeComposite::EntityTreeComposite(QWidget* parent)
: QWidget(parent)
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  composite_ = new Ui_EntityTreeComposite();
  composite_->setupUi(this);
  composite_->filterButton->hide(); // start out hidden until filters are added
  entityTreeWidget_ = new EntityTreeWidget(composite_->treeView);
  connect(entityTreeWidget_, &EntityTreeWidget::itemsSelected, this, &EntityTreeComposite::onItemsChanged_);
  connect(entityTreeWidget_, &EntityTreeWidget::itemsSelected, this, &EntityTreeComposite::itemsSelected);  // Echo out the signal
  connect(entityTreeWidget_, &EntityTreeWidget::itemDoubleClicked, this, &EntityTreeComposite::itemDoubleClicked); // Echo out the signal
  connect(entityTreeWidget_, &EntityTreeWidget::filterSettingsChanged, this, &EntityTreeComposite::filterSettingsChanged); // Echo out the signal

  // model is null at startup. Will be updated in the name filter in the call to setModel()
  nameFilter_ = new EntityNameFilter(nullptr);
  nameFilter_->bindToWidget(composite_->lineEdit);
  addEntityFilter(nameFilter_);

  composite_->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(composite_->treeView, &simQt::DndTreeView::customContextMenuRequested, this, &EntityTreeComposite::makeAndDisplayMenu_);

  // handle right-context menu (any actions will appear there)
  // Create a new QAction for copying data from the clipboard
  copyAction_ = new QAction(tr("&Copy"), composite_->treeView);
  copyAction_->setIcon(QIcon(":simQt/images/Copy.png"));
  copyAction_->setShortcut(QKeySequence::Copy);
  copyAction_->setShortcutContext(Qt::WidgetShortcut);
  copyAction_->setEnabled(false); // Should only be enabled when selections made
  connect(copyAction_, &QAction::triggered, this, &EntityTreeComposite::copySelection_);

  // Right click center action
  // NOTE: Use of this action must be enabled by the caller with setUseCenterAction()
  centerAction_ = new QAction(tr("Center On Selection"), composite_->treeView);
  centerAction_->setIcon(QIcon(":simQt/images/Find.png"));
  centerAction_->setEnabled(false); // Should only be enabled when selections made
  connect(centerAction_, &QAction::triggered, this, &EntityTreeComposite::centerOnSelection_);

  // Switch tree mode action
  toggleTreeViewAction_ = new QAction("Tree View", composite_->treeView);
  toggleTreeViewAction_->setIcon(QIcon(":simQt/images/Tree View.png"));
  toggleTreeViewAction_->setCheckable(true);
  toggleTreeViewAction_->setChecked(entityTreeWidget_->isTreeView());
  toggleTreeViewAction_->setToolTip(simQt::formatTooltip(tr("Toggle Tree View"), tr("Toggles the display of entity types between a tree and a list view.")));
  toggleTreeViewAction_->setEnabled(false); // Disabled until entities are added
  connect(toggleTreeViewAction_, &QAction::triggered, this, &EntityTreeComposite::setTreeView_);

  // Collapse All and Expand All actions
  collapseAllAction_ = composite_->actionCollapse_All;
  collapseAllAction_->setEnabled(false); // Disabled until entities are added
  expandAllAction_ = composite_->actionExpand_All;
  expandAllAction_->setEnabled(false); // Disabled until entities are added

  connect(composite_->filterButton, &QToolButton::clicked, this, &EntityTreeComposite::showFilters_);
  connect(entityTreeWidget_, &simQt::EntityTreeWidget::numFilteredItemsChanged, this, &EntityTreeComposite::setNumFilteredItemsLabel_);

  // Set tooltips
  composite_->filterButton->setToolTip(simQt::formatTooltip(tr("Entity Filter"),
  tr("Opens the Entity Filter dialog.<p>Used for filtering the display of entities shown in the Entity List.")));
  // Note: tool tip applied to magnifying glass icon (label); the lineEdit already has a comment in the text field
  composite_->label->setToolTip(simQt::formatTooltip(tr("Name Filter"),
  tr("Performs filtering based on entity names.<p>Right click in the text field to modify filtering options.")));

  // Default to off until a settings is passed in
  composite_->filterConfigWidget->setVisible(false);
}

EntityTreeComposite::~EntityTreeComposite()
{
  if (settings_)
  {
    for (auto i = buttonActions_.begin(); i != buttonActions_.end(); ++i)
      settings_->removeObserver((*i)->settingsKey(), observer_);
  }

  closeFilters_(); // clean up filter dialog
  qDeleteAll(buttonActions_);
  buttonActions_.clear();
  delete composite_;
  delete entityTreeWidget_;
  // entityTreeWidget_ owns nameFilter_, so don't delete it
  // we don't own model_ so don't delete it
}

void EntityTreeComposite::setMargins(int left, int top, int right, int bottom)
{
  composite_->verticalLayout->layout()->setContentsMargins(left, top, right, bottom);
}

void EntityTreeComposite::makeAndDisplayMenu_(const QPoint& pos)
{
  auto realMenu = std::make_unique<QMenu>(composite_->treeView);
  simQt::WeightedMenuManager menu(false);
  menu.setMenuBar(realMenu.get());

  menu.insertMenuAction(nullptr, EntityTreeComposite::WEIGHT_COPY, copyAction_);
  if (showCenterInMenu_)
    menu.insertMenuAction(nullptr, EntityTreeComposite::WEIGHT_CENTER, centerAction_);

  menu.insertMenuSeparator(nullptr, EntityTreeComposite::WEIGHT_POST_CENTER_SEPARATOR);

  if (showTreeOptionsInMenu_)
  {
    menu.insertMenuAction(nullptr, EntityTreeComposite::WEIGHT_TOGGLE_TREE_VIEW, toggleTreeViewAction_);
    menu.insertMenuAction(nullptr, EntityTreeComposite::WEIGHT_COLLAPSE_ALL, collapseAllAction_);
    menu.insertMenuAction(nullptr, EntityTreeComposite::WEIGHT_EXPAND_ALL, expandAllAction_);
  }

  // Give outside code a chance to update the menu before showing the menu
  Q_EMIT rightClickMenuRequested(realMenu.get());

  // Show the menu with exec(), making sure the position is correctly relative
  realMenu->exec(composite_->treeView->viewport()->mapToGlobal(pos));

  // Implicitly delete the menu, do not use ::aboutToHide()).  The menu->execute() can call code
  // that displays a progress dialog after the menu is hidden. The progress dialog can cause an
  // event loop processing which will delete the hidden menu while it is still in use.
}

void EntityTreeComposite::addEntityFilter(EntityFilter* entityFilter)
{
  entityTreeWidget_->addEntityFilter(entityFilter);
  // If filter button is hidden see if this filter will transition it to shown
  if (composite_->filterButton->isHidden())
  {
    QList<QWidget*> filterWidgets = entityTreeWidget_->filterWidgets(this);
    if (!filterWidgets.empty())
      composite_->filterButton->show();
    // Since we own the filter widgets, we should remove them now so they don't leak or accumulate
    qDeleteAll(filterWidgets);
  }
}

void EntityTreeComposite::setModel(AbstractEntityTreeModel* model)
{
  // Must pass in a valid model
  assert(model != nullptr);

  // SDK-120: If useEntityIcons_ is set, then apply it to the model
  model_ = model;
  if (useEntityIconsSet_ && model_)
    model_->setUseEntityIcons(useEntityIcons_);

  nameFilter_->setModel(model_);
  entityTreeWidget_->setModel(model_);
  // If the tree is pre-loaded, enable the tree/list button
  if (treeViewUsable_ && model_->rowCount() != 0)
    toggleTreeViewAction_->setEnabled(true);
  connect(model_, &QAbstractItemModel::rowsInserted, this, &EntityTreeComposite::rowsInserted_);
  connect(model_, &QAbstractItemModel::modelReset, this, &EntityTreeComposite::updateActionEnables_);
}


int EntityTreeComposite::setSelected(uint64_t id)
{
  return entityTreeWidget_->setSelected(id);
}

int EntityTreeComposite::setSelected(const QList<uint64_t>& list)
{
  return entityTreeWidget_->setSelected(list);
}

void EntityTreeComposite::scrollTo(uint64_t id, QAbstractItemView::ScrollHint hint)
{
  entityTreeWidget_->scrollTo(id, hint);
}

QAbstractItemView::SelectionMode EntityTreeComposite::selectionMode() const
{
  return entityTreeWidget_->selectionMode();
}

void EntityTreeComposite::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
  entityTreeWidget_->setSelectionMode(mode);
}

QTreeView* EntityTreeComposite::view() const
{
  return entityTreeWidget_->view();
}

simData::ObjectId EntityTreeComposite::alwaysShow() const
{
  return entityTreeWidget_->alwaysShow();
}

void EntityTreeComposite::setAlwaysShow(simData::ObjectId id)
{
  entityTreeWidget_->setAlwaysShow(id);
}

void EntityTreeComposite::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  entityTreeWidget_->getFilterSettings(settings);
}

void EntityTreeComposite::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  const QSignalBlocker blockSignals(*this);
  entityTreeWidget_->setFilterSettings(settings);
}

void EntityTreeComposite::setShowCenterInMenu(bool show)
{
  showCenterInMenu_ = show;
}
void EntityTreeComposite::setShowTreeOptionsInMenu(bool show)
{
  showTreeOptionsInMenu_ = show;
}

void EntityTreeComposite::setCountEntityType(simData::ObjectType type)
{
  entityTreeWidget_->setCountEntityType(type);
}

simData::ObjectType EntityTreeComposite::countEntityTypes() const
{
  return entityTreeWidget_->countEntityTypes();
}

/** Clears all selections */
void EntityTreeComposite::clearSelection()
{
  entityTreeWidget_->clearSelection();
}

/** Gets a list of all the selected IDs in the entity list */
QList<uint64_t> EntityTreeComposite::selectedItems() const
{
  return entityTreeWidget_->selectedItems();
}

/** Allows the developer to customize the look by adding buttons after the filter text **/
void EntityTreeComposite::addButton(QWidget* button)
{
  composite_->horizontalLayout_2->addWidget(button);
}

void EntityTreeComposite::setTreeViewActionEnabled(bool value)
{
  treeViewUsable_ = value;
  updateActionEnables_();
}

QIcon EntityTreeComposite::configIconForIndex_(int index) const
{
  switch (index)
  {
  case 0:
    return QIcon(":simQt/images/Data Blue Filter.png");
  case 1:
    return QIcon(":simQt/images/Data Green Filter.png");
  case 2:
    return QIcon(":simQt/images/Data Purple Filter.png");
  default:
    break;
  }
  return QIcon(":simQt/images/Data Red Filter.png");
}

QToolButton* EntityTreeComposite::configButtonForIndex_(int index) const
{
  switch (index)
  {
  case 0:
    return composite_->f1Button;
  case 1:
    return composite_->f2Button;
  case 2:
    return composite_->f3Button;
  default:
    break;
  }
  return composite_->f4Button;
}

void EntityTreeComposite::setSettings(SettingsPtr settings)
{
  entityTreeWidget_->setSettings(settings);

  // make sure the composite's treeview/listview pushbutton state matches widget treeview/listview
  // state, suppress signal since the widget will have already done the toggle
  {
    bool treeView = entityTreeWidget_->isTreeView();
    const QSignalBlocker blockSignals(*toggleTreeViewAction_);
    setTreeView_(treeView);
  }

  // Can only set the setting once
  assert(settings_ == nullptr);

  settings_ = settings;

  if (settings_ == nullptr)
    return;

  if (observer_ == nullptr)
    observer_.reset(new Observer(*this));

  // Filter configuration buttons use signal mappers to convey index
  QSignalMapper* loadMapper = new QSignalMapper(this);
  connect(loadMapper, &QSignalMapper::mappedInt, this, &EntityTreeComposite::loadFilterConfig_);
  QSignalMapper* saveMapper = new QSignalMapper(this);
  connect(saveMapper, &QSignalMapper::mappedInt, this, &EntityTreeComposite::saveFilterConfig_);
  QSignalMapper* clearMapper = new QSignalMapper(this);
  connect(clearMapper, &QSignalMapper::mappedInt, this, &EntityTreeComposite::clearFilterConfig_);
  QSignalMapper* pinMapper = new QSignalMapper(this);
  connect(pinMapper, &QSignalMapper::mappedInt, this, &EntityTreeComposite::pinFilterConfig_);

  auto pinned = settings_->value(PINNED_CUSTOM_FILTER).toString();
  for (int k = 0; k < 4; ++k)
  {
    QToolButton* button = configButtonForIndex_(k);
    // Failure here can cause indexing issues
    assert(button);
    if (!button)
      break;
    ButtonActions* actions = new ButtonActions(*button, configIconForIndex_(k));

    // Configure all signals to our signal mappers
    loadMapper->setMapping(actions->loadAction(), k);
    connect(actions->loadAction(), &QAction::triggered, loadMapper, qOverload<>(&QSignalMapper::map));
    saveMapper->setMapping(actions->saveAction(), k);
    connect(actions->saveAction(), &QAction::triggered, saveMapper, qOverload<>(&QSignalMapper::map));
    clearMapper->setMapping(actions->clearAction(), k);
    connect(actions->clearAction(), &QAction::triggered, clearMapper, qOverload<>(&QSignalMapper::map));
    pinMapper->setMapping(actions->pinAction(), k);
    connect(actions->pinAction(), &QAction::triggered, pinMapper, qOverload<>(&QSignalMapper::map));

    // Initialize the button with the filter data from settings
    QVariant defaultValue;
    defaultValue.setValue(FilterConfiguration());
    Settings::MetaData metaData(Settings::VARIANT_MAP, defaultValue, "", Settings::PRIVATE);
    auto filter = qvariant_cast<FilterConfiguration>(settings_->value(actions->settingsKey(), metaData, observer_));
    actions->setFilterConfiguration(filter);
    setPinnedState_(*actions, pinned == actions->settingsKey());

    // Save the action for later
    buttonActions_.push_back(actions);
  }

  // Show buttons
  composite_->filterConfigWidget->setVisible(true);
  applyPinnedFilterConfiguration();
}

void EntityTreeComposite::loadFilterConfig_(int index)
{
  entityTreeWidget_->setFilterSettings(buttonActions_[index]->filterConfiguration().configuration());
}

void EntityTreeComposite::saveFilterConfig_(int index)
{
  ButtonActions* action = buttonActions_[index];
  bool okay = false;
  const QString desc = QInputDialog::getText(this, tr("Save Filter Configuration"),
    tr("Enter a description to save with this filter configuration:"),
    QLineEdit::Normal, action->description(), &okay,
    Qt::WindowCloseButtonHint | Qt::WindowTitleHint | Qt::Dialog );

  // If user clicked cancel or did not enter in a description, don't do anything further
  if ((!okay) || desc.isEmpty())
    return;
  // Get current filter settings to save
  QMap<QString, QVariant> variantMap;
  getFilterSettings(variantMap);
  FilterConfiguration newConfig(desc, variantMap);
  action->setFilterConfiguration(newConfig);
  // Save the value also to settings
  if (settings_ != nullptr)
  {
    QVariant value;
    value.setValue(action->filterConfiguration());
    settings_->setValue(action->settingsKey(), value, observer_);
  }
}

void EntityTreeComposite::clearFilterConfig_(int index)
{
  ButtonActions* action = buttonActions_[index];
  FilterConfiguration emptyConfig;
  action->setFilterConfiguration(emptyConfig);
  if (settings_ != nullptr)
  {
    QVariant value;
    value.setValue(action->filterConfiguration());
    settings_->setValue(action->settingsKey(), value, observer_);

    // unpin this filter configuration if it's pinned
    auto pinned = settings_->value(PINNED_CUSTOM_FILTER).toString();
    if (pinned == action->settingsKey())
    {
      setPinnedState_(*action, false);
      settings_->setValue(PINNED_CUSTOM_FILTER, "");
    }
  }
}

void EntityTreeComposite::pinFilterConfig_(int index)
{
  if (settings_ == nullptr)
    return;

  for (int i = 0; i < static_cast<int>(buttonActions_.size()); ++i)
  {
    ButtonActions* action = buttonActions_[i];
    auto pinned = settings_->value(PINNED_CUSTOM_FILTER).toString();
    if (i == index)
    {
      // toggle pinned setting
      bool alreadyPinned = (pinned == action->settingsKey());
      settings_->setValue(PINNED_CUSTOM_FILTER, alreadyPinned ? "" : action->settingsKey());
      setPinnedState_(*action, !alreadyPinned);
    }
    else // update text of other pin actions
      setPinnedState_(*action, false);
  }
}

void EntityTreeComposite::initializeSettings(SettingsPtr settings)
{
  EntityTreeWidget::initializeSettings(settings);
}

void EntityTreeComposite::rowsInserted_(const QModelIndex & parent, int start, int end)
{
  updateActionEnables_();
}

void EntityTreeComposite::showFilters_()
{
  if (filterDialog_ != nullptr)
  {
    filterDialog_->show();
    return;
  }
  // create a new filter dialog, using the filter widgets from the EntityTreeWidget's proxy model
  // Qt6 has problems with QDialogs that aren't parented to the QMainWindow, so attempt to set main window as the parent
  filterDialog_ = new FilterDialog(settings_, QtUtils::getMainWindowParent(this));
  QList<QWidget*> filterWidgets = entityTreeWidget_->filterWidgets(filterDialog_);
  filterDialog_->setMinimumWidth(200);
  filterDialog_->setWindowTitle(tr("Entity Filters"));
  filterDialog_->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
  QVBoxLayout* layout = new QVBoxLayout(filterDialog_);
  layout->setContentsMargins(2, 2, 2, 2);
  for (auto it = filterWidgets.begin(); it != filterWidgets.end(); ++it)
  {
    layout->addWidget(*it);
  }

  // connect to the close signal, to clean up resources
  connect(dynamic_cast<FilterDialog*>(filterDialog_), &FilterDialog::closedGui, this, &EntityTreeComposite::closeFilters_);
  filterDialog_->setLayout(layout);
  filterDialog_->show();
}

void EntityTreeComposite::closeFilters_()
{
  if (filterDialog_)
  {
    filterDialog_->hide();
    filterDialog_->deleteLater();
    filterDialog_ = nullptr;
  }
}

void EntityTreeComposite::setNumFilteredItemsLabel_(int numFilteredItems, int numTotalItems)
{
  composite_->countLabel->setText(QString("%1 of %2 Filtered Entity Names").arg(numFilteredItems).arg(numTotalItems));
}

void EntityTreeComposite::setExpandsOnDoubleClick(bool value)
{
  composite_->treeView->setExpandsOnDoubleClick(value);
}

bool EntityTreeComposite::expandsOnDoubleClick() const
{
  return composite_->treeView->expandsOnDoubleClick();
}

bool EntityTreeComposite::useCenterAction() const
{
  return useCenterAction_;
}

void EntityTreeComposite::setUseCenterAction(bool use, const QString& reason)
{
  if (!reason.isEmpty())
    centerAction_->setText(tr("Center On Selection (%1)").arg(reason));
  else
    centerAction_->setText(tr("Center On Selection"));

  if (use == useCenterAction_)
    return;
  useCenterAction_ = use;
  if (!selectedItems().isEmpty())
    centerAction_->setEnabled(use); // Only enable if there's items in the tree
  else
    centerAction_->setEnabled(false);
}

void EntityTreeComposite::setTreeView(bool useTreeView)
{
  if (!treeViewUsable_)
    return;

  setTreeView_(useTreeView);
}

void EntityTreeComposite::applyPinnedFilterConfiguration()
{
  if (settings_ == nullptr)
    return;
  Settings::MetaData pinMetaData(Settings::STRING, QVariant(), "", Settings::PRIVATE);
  auto pinned = settings_->value(PINNED_CUSTOM_FILTER, pinMetaData).toString();
  if (pinned.isEmpty())
    return;
  for (const auto& button : buttonActions_)
  {
    if (pinned == button->settingsKey())
    {
      entityTreeWidget_->setFilterSettings(button->filterConfiguration().configuration());
      // call twice to ensure correct final state of category filters, since category name manager should now be updated from previous call to setFilterSettings
      entityTreeWidget_->setFilterSettings(button->filterConfiguration().configuration());
      break;
    }
  }
}

void EntityTreeComposite::setPinnedState_(ButtonActions& actions, bool pinned)
{
  actions.button().setStyleSheet(pinned ? "QToolButton { background-color: rgb(138, 255, 138) }" : "");
  if (pinned)
  {
    actions.pinAction()->setText("Unpin");
    actions.pinAction()->setToolTip(simQt::formatTooltip(tr("Unpin"), tr("Unpin the button's filter configuration to stop persisting in the display.")));
  }
  else
  {
    actions.pinAction()->setText("Pin");
    actions.pinAction()->setToolTip(simQt::formatTooltip(tr("Pin"), tr("Pin the button's filter configuration to persist in the display.")));
  }
}

void EntityTreeComposite::onItemsChanged_(const QList<uint64_t>& ids)
{
  const bool empty = ids.isEmpty();
  copyAction_->setEnabled(!empty);
  if (useCenterAction_)
    centerAction_->setEnabled(!empty);
}

void EntityTreeComposite::copySelection_()
{
  QList<uint64_t> ids =  entityTreeWidget_->selectedItems();

  if (ids.isEmpty() || (model_ == nullptr))
    return;

  QString clipboardText;
  for (auto it = ids.begin(); it != ids.end(); ++it)
  {
    if (!clipboardText.isEmpty())
      clipboardText.append("\n");

    QModelIndex index = model_->index(*it);
    clipboardText.append(model_->data(index, Qt::DisplayRole).toString());
  }

  QApplication::clipboard()->setText(clipboardText);
}

void EntityTreeComposite::centerOnSelection_()
{
  if (selectedItems().size() == 1)
    Q_EMIT centerOnEntityRequested(selectedItems().front());
  else if (!selectedItems().empty())
    Q_EMIT centerOnSelectionRequested(selectedItems());
}

void EntityTreeComposite::setTreeView_(bool useTreeView)
{
  // Return early if nothing changed
  if (entityTreeWidget_->isTreeView() == useTreeView && toggleTreeViewAction_->isChecked() == useTreeView)
    return;

  // Toggle the tree view
  entityTreeWidget_->toggleTreeView(useTreeView);
  // Update related UI components
  toggleTreeViewAction_->setChecked(useTreeView);
  updateActionEnables_();

  Q_EMIT treeViewChanged(useTreeView);
}

void EntityTreeComposite::updateActionEnables_()
{
  bool enableToggleAction = treeViewUsable_ && model_ && model_->rowCount() > 0;
  toggleTreeViewAction_->setEnabled(enableToggleAction);

  bool enableTreeActions = entityTreeWidget_->isTreeView() && model_ && model_->rowCount() > 0;
  collapseAllAction_->setEnabled(enableTreeActions);
  expandAllAction_->setEnabled(enableTreeActions);
}

bool EntityTreeComposite::useEntityIcons() const
{
  if (!model_)
    return useEntityIcons_;
  return model_->useEntityIcons();
}

void EntityTreeComposite::setUseEntityIcons(bool showIcons)
{
  useEntityIconsSet_ = true;
  useEntityIcons_ = showIcons;
  if (model_)
    model_->setUseEntityIcons(showIcons);
}

//---------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------

EntityTreeComposite::FilterConfiguration::FilterConfiguration()
{
}

EntityTreeComposite::FilterConfiguration::~FilterConfiguration()
{
}

EntityTreeComposite::FilterConfiguration::FilterConfiguration(const FilterConfiguration& rhs)
{
  description_ = rhs.description_;
  configuration_ = rhs.configuration_;
}

EntityTreeComposite::FilterConfiguration::FilterConfiguration(const QString& description, const QMap<QString, QVariant>& configuration)
  : description_(description),
    configuration_(configuration)
{
}

QString EntityTreeComposite::FilterConfiguration::description() const
{
  return description_;
}

void EntityTreeComposite::FilterConfiguration::setDescription(const QString& description)
{
  description_ = description;
}

QMap<QString, QVariant> EntityTreeComposite::FilterConfiguration::configuration() const
{
  return configuration_;
}

void EntityTreeComposite::FilterConfiguration::setConfiguration(const QMap<QString, QVariant>& configuration)
{
  configuration_ = configuration;
}

}

//---------------------------------------------------------------------------------------------

QDataStream &operator<<(QDataStream& out, const simQt::EntityTreeComposite::FilterConfiguration& myObj)
{
  out << myObj.description() << myObj.configuration();
  return out;
}

QDataStream &operator>>(QDataStream& in, simQt::EntityTreeComposite::FilterConfiguration& myObj)
{
  QString description;
  in >> description;
  myObj.setDescription(description);
  QMap<QString, QVariant> configuration;
  in >> configuration;
  myObj.setConfiguration(configuration);
  return in;
}

