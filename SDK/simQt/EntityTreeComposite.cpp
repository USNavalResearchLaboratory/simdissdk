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
#include <QDialog>
#include <QGroupBox>
#include <QInputDialog>
#include <QToolButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QClipboard>
#include "simQt/QtConversion.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "ui_EntityTreeComposite.h"

namespace simQt {

const QString DEFAULT_FILTER_CONFIG_TOOLTIP = simQt::formatTooltip(QObject::tr("No Filter Configuration Saved"), "");
const QString CLEAR_CONFIG_ACTION_TEXT = QObject::tr("Clear Stored Configuration");
const QString INACTIVE_ICON_NAME = ":simQt/images/Data Inactive Filter.png";

FilterDialog::FilterDialog(QWidget* parent)
  :QDialog(parent)
{}

void FilterDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  emit(closedGui());
}

EntityTreeComposite::EntityTreeComposite(QWidget* parent)
: QWidget(parent),
  composite_(NULL),
  entityTreeWidget_(NULL),
  model_(NULL),
  filterDialog_(NULL),
  useCenterAction_(false)
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  composite_ = new Ui_EntityTreeComposite();
  composite_->setupUi(this);
  composite_->pushButton->setEnabled(false);
  composite_->filterButton->hide(); // start out hidden until filters are added
  entityTreeWidget_ = new EntityTreeWidget(composite_->treeView);
  connect(entityTreeWidget_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(onItemsChanged_(QList<uint64_t>)));
  connect(entityTreeWidget_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SIGNAL(itemsSelected(QList<uint64_t>)));  // Echo out the signal
  connect(entityTreeWidget_, SIGNAL(itemDoubleClicked(uint64_t)), this, SIGNAL(itemDoubleClicked(uint64_t))); // Echo out the signal
  connect(entityTreeWidget_, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>)), this, SIGNAL(filterSettingsChanged(QMap<QString, QVariant>))); // Echo out the signal

  // handle right-context menu (any actions will appear there)
  // Create a new QAction for copying data from the clipboard
  copyAction_ = new QAction(tr("&Copy"), composite_->treeView);
  copyAction_->setIcon(QIcon(":simQt/images/Copy.png"));
  copyAction_->setShortcut(QKeySequence::Copy);
  copyAction_->setShortcutContext(Qt::WidgetShortcut);
  copyAction_->setEnabled(false); // Should only be enabled when selections made
  connect(copyAction_, SIGNAL(triggered()), this, SLOT(copySelection_()));
  composite_->treeView->addAction(copyAction_);

  // Right click center action
  // NOTE: Use of this action must be enabled by the caller with setUseCenterAction()
  centerAction_ = new QAction(tr("Center On Entity"), composite_->treeView);
  centerAction_->setIcon(QIcon(":simQt/images/Find.png"));
  centerAction_->setEnabled(false); // Should only be enabled when selections made
  connect(centerAction_, SIGNAL(triggered()), this, SLOT(centerOnSelection_()));
  composite_->treeView->addAction(centerAction_);

  // Add separator
  QAction* sep = new QAction(this);
  sep->setSeparator(true);
  composite_->treeView->addAction(sep);

  // Switch tree mode action
  toggleTreeViewAction_ = new QAction("Tree View", composite_->treeView);
  toggleTreeViewAction_->setIcon(QIcon(":simQt/images/Tree View.png"));
  toggleTreeViewAction_->setCheckable(true);
  toggleTreeViewAction_->setChecked(entityTreeWidget_->isTreeView());
  toggleTreeViewAction_->setToolTip(simQt::formatTooltip(tr("Toggle Tree View"), tr("Toggles the display of entity types between a tree and a list view.")));
  toggleTreeViewAction_->setEnabled(false); // Disabled until entities are added
  connect(toggleTreeViewAction_, SIGNAL(triggered(bool)), this, SLOT(setTreeView_(bool)));
  composite_->treeView->addAction(toggleTreeViewAction_);

  // Collapse All and Expand All actions
  collapseAllAction_ = composite_->actionCollapse_All;
  collapseAllAction_->setEnabled(false); // Disabled until entities are added
  composite_->treeView->addAction(collapseAllAction_);
  expandAllAction_ = composite_->actionExpand_All;
  expandAllAction_->setEnabled(false); // Disabled until entities are added
  composite_->treeView->addAction(expandAllAction_);

  composite_->pushButton->setDefaultAction(toggleTreeViewAction_);
  connect(composite_->lineEdit, SIGNAL(changed(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)), this, SLOT(textFilterChanged_(QString, Qt::CaseSensitivity, QRegExp::PatternSyntax)));
  connect(composite_->filterButton, SIGNAL(clicked()), this, SLOT(showFilters_()));
  connect(entityTreeWidget_, SIGNAL(numFilteredItemsChanged(int, int)), this, SLOT(setNumFilteredItemsLabel_(int, int)));

  // Set up filter configuration buttons
  initFilterConfigurationButton_(composite_->filterConfigButton1, ":simQt/images/Data Blue Filter.png");
  initFilterConfigurationButton_(composite_->filterConfigButton2, ":simQt/images/Data Green Filter.png");
  initFilterConfigurationButton_(composite_->filterConfigButton3, ":simQt/images/Data Purple Filter.png");
  initFilterConfigurationButton_(composite_->filterConfigButton4, ":simQt/images/Data Red Filter.png");

  // Set tooltips
  composite_->pushButton->setToolTip(toggleTreeViewAction_->toolTip());
  composite_->filterButton->setToolTip(simQt::formatTooltip(tr("Entity Filter"),
  tr("Opens the Entity Filter dialog.<p>Used for filtering the display of entities shown in the Entity List.")));
  // Note: tool tip applied to magnifying glass icon (label); the lineEdit already has a comment in the text field
  composite_->label->setToolTip(simQt::formatTooltip(tr("Name Filter"),
  tr("Performs filtering based on entity names.<p>Right click in the text field to modify filtering options.")));
}

void EntityTreeComposite::initFilterConfigurationButton_(QToolButton* button, const QString& iconName)
{
  connect(button, SIGNAL(clicked()), this, SLOT(loadConfig_()));
  // Disable the menu indicator turned on by adding actions below
  button->setStyleSheet("QToolButton::menu-indicator { image: none; }");

  // Set initial icon
  button->setIcon(QIcon(INACTIVE_ICON_NAME));

  // Context menu actions
  QAction* saveConfig = new QAction(tr("Save Current Configuration..."), button);
  saveConfig->setIcon(QIcon(":simQt/images/Save.png"));
  connect(saveConfig, SIGNAL(triggered()), this, SLOT(saveConfig_()));

  QAction* clearConfig = new QAction(CLEAR_CONFIG_ACTION_TEXT, button);
  clearConfig->setIcon(QIcon(":simQt/images/Clean Up.png"));
  clearConfig->setEnabled(false); // Enabled when a config is saved
  connect(clearConfig, SIGNAL(triggered()), this, SLOT(clearConfig_()));

  // Add actions to button's context menu
  button->setContextMenuPolicy(Qt::ActionsContextMenu);
  button->addAction(saveConfig);
  button->addAction(clearConfig);

  // Set default tool tip
  button->setToolTip(DEFAULT_FILTER_CONFIG_TOOLTIP);

  // Set active (colored) icon for this button
  buttonNamesToIconNames_[button->objectName()] = iconName;
}

void EntityTreeComposite::loadConfig_()
{
  QToolButton* button = qobject_cast<QToolButton*>(sender());
  if (button == NULL)
  {
    assert(0); // Sender should be a QToolButton
    return;
  }

  // Find configuration stored by this button
  FilterConfiguration fc = buttonsToFilterConfigs_[button];

  // If the returned configuration is empty, then there's no configuration to load
  if (fc.configuration.isEmpty())
    return;

  // Apply filter configuration to the widget, which sends a call
  // out to the other Entity Filters and updates them to match
  entityTreeWidget_->setFilterSettings(fc.configuration);
}

void EntityTreeComposite::saveConfig_()
{
  // Cast to sender's parent because we want the QToolButton, not the QAction
  QToolButton* button = qobject_cast<QToolButton*>(sender()->parent());
  if (button == NULL)
  {
    assert(0); // Sender should be a QToolButton, possibly the QAction sender()'s parentage has changed?
    return;
  }

  // Show the previous filter configuration's description as the placeholder text, if it exists
  QString placeHolderText = tr("Custom Filter Configuration");
  FilterConfiguration fc = buttonsToFilterConfigs_[button];
  if (!fc.configuration.isEmpty())
    placeHolderText = fc.description;

  // Get description from user
  bool okay;
  QString desc = QInputDialog::getText(this, tr("Save Filter Configuration"), tr("Enter a description to save with this filter configuration:"), QLineEdit::Normal, placeHolderText, &okay);

  // If user clicked cancel, don't do anything further
  if (!okay)
    return;

  // Get current filter settings to save
  QMap<QString, QVariant> configuration;
  getFilterSettings(configuration);

  // Need to emit that the button has been updated
  updateButton_(button, desc, configuration);
}

void EntityTreeComposite::saveConfigToButton(const QString& buttonName, const QString& desc, const QMap<QString, QVariant>& configuration)
{
  // Find the button
  QToolButton* button = findChild<QToolButton*>(buttonName);

  if (button == NULL)
  {
    assert(0); // Didn't find the button
    return;
  }

  // This method is called when a button update occurs
  // in another instance of EntityTreeComposite, so
  // don't emit that the button has been updated
  updateButton_(button, desc, configuration, false);
}

void EntityTreeComposite::updateButton_(QToolButton* button, const QString& desc, const QMap<QString, QVariant>& configuration, bool fireSignal)
{
  // Check if we're overwriting a stored configuration
  FilterConfiguration fc = buttonsToFilterConfigs_[button];
  if (fc.configuration.isEmpty())
  {
    Q_FOREACH(QAction* action, button->actions())
    {
      // Need to enable the clear action if we're saving to a
      // button without a configuration saved to it already
      if (action->text() == CLEAR_CONFIG_ACTION_TEXT)
        action->setEnabled(true);
    }
  }

  // Store FilterConfiguration internally
  QString buttonName = button->objectName();
  fc.button = buttonName;
  fc.description = desc;
  fc.configuration = configuration;
  buttonsToFilterConfigs_[button] = fc;

  // Set the button's tool tip
  button->setToolTip(simQt::formatTooltip(tr("Filter Configuration Saved"), fc.description));

  // Update the button's icon to the active (colored) icon
  QString iconName = buttonNamesToIconNames_[buttonName];
  button->setIcon(QIcon(iconName));

  if (fireSignal)
    emit configurationButtonUpdated(buttonName, desc, configuration);
}

void EntityTreeComposite::clearConfig_()
{
  // Cast to sender's parent because we want the QToolButton, not the QAction
  QToolButton* button = qobject_cast<QToolButton*>(sender()->parent());
  if (button == NULL)
  {
    assert(0); // Sender should be a QToolButton, possibly the QAction sender()'s parentage has changed?
    return;
  }

  // Need to emit that we've cleared the button
  resetButton_(button);
}

void EntityTreeComposite::clearConfigFromButton(const QString& buttonName)
{
  // Find the button
  QToolButton* button = findChild<QToolButton*>(buttonName);

  if (button == NULL)
  {
    assert(0); // Didn't find the button
    return;
  }

  // This method is called when a button clear occurs
  // in another instance of EntityTreeComposite, so
  // don't emit that the button has been clear
  resetButton_(button, false);
}

void EntityTreeComposite::resetButton_(QToolButton* button, bool fireSignal)
{
  // Clear saved configuration for this button
  buttonsToFilterConfigs_.erase(button);

  // Reset default tool tip
  button->setToolTip(DEFAULT_FILTER_CONFIG_TOOLTIP);

  // Update the button's icon to the inactive
  button->setIcon(QIcon(INACTIVE_ICON_NAME));

  // Disable the clear action
  Q_FOREACH(QAction* action, button->actions())
  {
    if (action->text() == CLEAR_CONFIG_ACTION_TEXT)
      action->setEnabled(false);
  }

  if (fireSignal)
    emit configurationButtonCleared(button->objectName());
}

EntityTreeComposite::~EntityTreeComposite()
{
  closeFilters_(); // clean up filter dialog
  delete composite_;
  delete entityTreeWidget_;
  // we don't own model_ so don't delete it
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
  assert(model != NULL);

  model_ = model;
  entityTreeWidget_->setModel(model_);
  // If the tree is pre-loaded, enable the tree/list button
  if (model_->rowCount() != 0)
    toggleTreeViewAction_->setEnabled(true);
  connect((QObject*)model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(rowsInserted_(QModelIndex, int, int)));
}

/** Sets/clears the selected ID in the entity list */
void EntityTreeComposite::setSelected(uint64_t id, bool selected)
{
  entityTreeWidget_->setSelected(id, selected);
}

void EntityTreeComposite::setSelected(QList<uint64_t> list, bool selected)
{
  entityTreeWidget_->setSelected(list, selected);
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
  settings.insert("RegExp", entityTreeWidget_->regExp());
  entityTreeWidget_->getFilterSettings(settings);
}

void EntityTreeComposite::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  simQt::ScopedSignalBlocker blockSignals(*this);
  QMap<QString, QVariant>::const_iterator it = settings.find("RegExp");
  if (it != settings.end())
  {
    QRegExp regExp = it.value().toRegExp();
    // Update the GUI and signals will take care of the rest
    composite_->lineEdit->configure(regExp.pattern(), regExp.caseSensitivity(), regExp.patternSyntax());
  }
  entityTreeWidget_->setFilterSettings(settings);
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
  composite_->horizontalLayout->addWidget(button);
}

void EntityTreeComposite::setListTreeButtonDisplayed(bool value)
{
  composite_->pushButton->setVisible(value);
}

void EntityTreeComposite::setSettings(SettingsPtr settings)
{
  entityTreeWidget_->setSettings(settings);

  // make sure the composite's treeview/listview pushbutton state matches widget treeview/listview
  // state, suppress signal since the widget will have already done the toggle
  bool treeView = entityTreeWidget_->isTreeView();
  simQt::ScopedSignalBlocker blockSignals(*toggleTreeViewAction_);
  setTreeView_(treeView);
}

void EntityTreeComposite::initializeSettings(SettingsPtr settings)
{
  EntityTreeWidget::initializeSettings(settings);
}

void EntityTreeComposite::textFilterChanged_(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax syntax)
{
  QRegExp regExp(filter, caseSensitive, syntax);
  entityTreeWidget_->setRegExp(regExp);
}

void EntityTreeComposite::rowsInserted_(const QModelIndex & parent, int start, int end)
{
  toggleTreeViewAction_->setEnabled(true);
  updateActionEnables_();
}

void EntityTreeComposite::showFilters_()
{
  if (filterDialog_ != NULL)
  {
    filterDialog_->show();
    return;
  }
  // create a new filter dialog, using the filter widgets from the EntityTreeWidget's proxy model
  filterDialog_ = new FilterDialog(this);
  QList<QWidget*> filterWidgets = entityTreeWidget_->filterWidgets(filterDialog_);
  filterDialog_->setMinimumWidth(200);
  filterDialog_->setWindowTitle("Entity Filters");
  filterDialog_->setWindowFlags(filterDialog_->windowFlags() ^ Qt::WindowContextHelpButtonHint);
  QVBoxLayout* layout = new QVBoxLayout(filterDialog_);
  layout->setContentsMargins(2, 2, 2, 2);
  Q_FOREACH(QWidget* widget, filterWidgets)
  {
    // create a label for each widget, using the widget WindowTitle as text
    QGroupBox* groupBox = new QGroupBox(widget->windowTitle(), filterDialog_);
    QVBoxLayout* gbLayout = new QVBoxLayout(groupBox);
    gbLayout->setContentsMargins(2, 2, 2, 2);
    gbLayout->addWidget(widget);
    groupBox->setLayout(gbLayout);
    layout->addWidget(groupBox);
  }

  // connect to the close signal, to clean up resources
  connect(filterDialog_, SIGNAL(closedGui()), this, SLOT(closeFilters_()));
  filterDialog_->setLayout(layout);
  filterDialog_->show();
}

void EntityTreeComposite::closeFilters_()
{
  // we own all this memory, so we can delete it
  delete filterDialog_;
  filterDialog_ = NULL;
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

void EntityTreeComposite::setUseCenterAction(bool use)
{
  if (use == useCenterAction_)
    return;
  useCenterAction_ = use;
  if (!selectedItems().isEmpty())
    centerAction_->setEnabled(use); // Only enable if there's items in the tree
}

void EntityTreeComposite::onItemsChanged_(const QList<uint64_t>& ids)
{
  bool empty = ids.isEmpty();
  copyAction_->setEnabled(!empty);
  if (useCenterAction_)
    centerAction_->setEnabled(!empty);
}

void EntityTreeComposite::copySelection_()
{
  QList<uint64_t> ids =  entityTreeWidget_->selectedItems();

  if (ids.isEmpty() || (model_ == NULL))
    return;

  QString clipboardText;
  Q_FOREACH(uint64_t id, ids)
  {
    if (!clipboardText.isEmpty())
      clipboardText.append("\n");

    QModelIndex index = model_->index(id);
    clipboardText.append(model_->data(index, Qt::DisplayRole).toString());
  }

  QApplication::clipboard()->setText(clipboardText);
}

void EntityTreeComposite::centerOnSelection_()
{
  if (!selectedItems().empty())
    emit centerOnEntityRequested(selectedItems().first());
}

void EntityTreeComposite::setTreeView_(bool useTreeView)
{
  // Toggle the tree view
  entityTreeWidget_->toggleTreeView(useTreeView);
  // Update related UI components
  toggleTreeViewAction_->setChecked(useTreeView);
  updateActionEnables_();
}

void EntityTreeComposite::updateActionEnables_()
{
  bool enableIt = entityTreeWidget_->isTreeView() && model_->rowCount() > 0;
  collapseAllAction_->setEnabled(enableIt);
  expandAllAction_->setEnabled(enableIt);
}

bool EntityTreeComposite::useEntityIcons() const
{
  return model_->useEntityIcons();
}

void EntityTreeComposite::setUseEntityIcons(bool showIcons)
{
  model_->setUseEntityIcons(showIcons);
}

}
