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
#include <QSignalMapper>
#include <QClipboard>
#include "simQt/QtFormatting.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityFilterLineEdit.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/SettingsGroup.h"
#include "simQt/AbstractEntityTreeModel.h"
#include "ui_EntityTreeComposite.h"

namespace simQt {

const QString SETTING_NAME_FILTER = "/FilterSettings/";


FilterDialog::FilterDialog(QWidget* parent)
  :QDialog(parent)
{}

void FilterDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  emit(closedGui());
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
      button_(button)
  {
    // No tooltip needed for clear because it's never a standalone button via setDefaultButton()
    saveAction_->setToolTip(simQt::formatTooltip(tr("Save"), tr("Saves the current filter configuration to a button.")));
    setLoadTextAndTooltips_("");

    // We start without a filter configuration, so default mode is "save"
    button_.setDefaultAction(saveAction_);
    button_.addAction(loadAction_);
    button_.addAction(saveAction_);
    button_.addAction(clearAction_);
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
      button_.setDefaultAction(saveAction_);
    }
    else
    {
      loadAction_->setEnabled(true);
      clearAction_->setEnabled(true);
      button_.setDefaultAction(loadAction_);
    }
    filterConfig_ = filter;
  }

private:
  QAction* loadAction_;
  QAction* saveAction_;
  QAction* clearAction_;
  QToolButton& button_;
  FilterConfiguration filterConfig_;

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
    ButtonActions* actions = NULL;
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

  // Set tooltips
  composite_->pushButton->setToolTip(toggleTreeViewAction_->toolTip());
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
    simQt::ScopedSignalBlocker blockSignals(*toggleTreeViewAction_);
    setTreeView_(treeView);
  }

  // Can only set the setting once
  assert(settings_ == NULL);

  settings_ = settings;

  if (settings_ == NULL)
    return;

  if (observer_ == NULL)
    observer_.reset(new Observer(*this));

  // Filter configuration buttons use signal mappers to convey index
  QSignalMapper* loadMapper = new QSignalMapper(this);
  connect(loadMapper, SIGNAL(mapped(int)), this, SLOT(loadFilterConfig_(int)));
  QSignalMapper* saveMapper = new QSignalMapper(this);
  connect(saveMapper, SIGNAL(mapped(int)), this, SLOT(saveFilterConfig_(int)));
  QSignalMapper* clearMapper = new QSignalMapper(this);
  connect(clearMapper, SIGNAL(mapped(int)), this, SLOT(clearFilterConfig_(int)));
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
    connect(actions->loadAction(), SIGNAL(triggered()), loadMapper, SLOT(map()));
    saveMapper->setMapping(actions->saveAction(), k);
    connect(actions->saveAction(), SIGNAL(triggered()), saveMapper, SLOT(map()));
    clearMapper->setMapping(actions->clearAction(), k);
    connect(actions->clearAction(), SIGNAL(triggered()), clearMapper, SLOT(map()));

    // Initialize the button with the filter data from settings
    QVariant defaultValue;
    defaultValue.setValue(FilterConfiguration());
    Settings::MetaData metaData(Settings::VARIANT_MAP, defaultValue, "", Settings::PRIVATE);
    auto filter = qvariant_cast<FilterConfiguration>(settings_->value(actions->settingsKey(), metaData, observer_));
    actions->setFilterConfiguration(filter);

    // Save the action for later
    buttonActions_.push_back(actions);
  }

  // Show buttons
  composite_->filterConfigWidget->setVisible(true);
}

void EntityTreeComposite::loadFilterConfig_(int index)
{
  setFilterSettings(buttonActions_[index]->filterConfiguration().configuration());
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
  if (settings_ != NULL)
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
  if (settings_ != NULL)
  {
    QVariant value;
    value.setValue(action->filterConfiguration());
    settings_->setValue(action->settingsKey(), value, observer_);
  }
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
  filterDialog_->setWindowTitle(tr("Entity Filters"));
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

