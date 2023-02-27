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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include <QAbstractProxyModel>
#include <QDialog>
#include <QHeaderView>
#include <QVBoxLayout>

#include "simData/DataStoreHelpers.h"
#include "simCore/Time/Clock.h"
#include "simQt/CenterEntity.h"
#include "simQt/EntityCategoryFilter.h"
#include "simQt/EntityProxyModel.h"
#include "simQt/EntityStateFilter.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/EntityTreeModel.h"
#include "simQt/EntityTreeWidget.h"
#include "simQt/EntityTypeFilter.h"
#include "simQt/QtFormatting.h"
#include "simQt/ResourceInitializer.h"
#include "simQt/ScopedSignalBlocker.h"
#include "simQt/EntityLineEdit.h"
#include "ui_EntityLineEdit.h"

namespace simQt {

namespace {
  static const QString VALID_ENTITY = "";
  static const QString INVALID_ENTITY = "QLineEdit:enabled { color: red }";
}

EntityDialog::EntityDialog(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::ObjectType type, simCore::Clock* clock, SettingsPtr settings)
  : QDialog(parent),
    entityTreeModel_(entityTreeModel),
    entityStateFilter_(nullptr),
    centerBind_(nullptr)
{
  setWindowTitle(tr("Select Entity"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setObjectName("SelectEntity");

  tree_ = new simQt::EntityTreeComposite(this);
  tree_->setModel(entityTreeModel_);
  tree_->setExpandsOnDoubleClick(true);
  tree_->setSelectionMode(QAbstractItemView::SingleSelection);
  tree_->setTreeViewActionEnabled(false);  // The Entity Line Composite does not support the tree view
  tree_->setShowTreeOptionsInMenu(false);
  tree_->setShowCenterInMenu(false);  // will be turned back on if setCenterEntity() is called
  if (settings)
    tree_->setSettings(settings);

  if (clock != nullptr)
  {
    entityStateFilter_ = new simQt::EntityStateFilter(*entityTreeModel_->dataStore(), *clock, true);
    tree_->addEntityFilter(entityStateFilter_);
  }

  tree_->addEntityFilter(new simQt::EntityTypeFilter(*entityTreeModel_->dataStore(), type, type == simData::ALL));
  tree_->addEntityFilter(new simQt::EntityCategoryFilter(entityTreeModel_->dataStore(), simQt::EntityCategoryFilter::SHOW_WIDGET));

  connect(tree_, SIGNAL(itemsSelected(QList<uint64_t>)), this, SLOT(setSelected_(QList<uint64_t>)));
  connect(tree_, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(accept_())); // Have double click auto close the dialog

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setMargin(0);
  layout->addWidget(tree_);
  setLayout(layout);
}

EntityDialog::~EntityDialog()
{
}

void EntityDialog::closeEvent(QCloseEvent* ev)
{
  QDialog::closeEvent(ev);
  Q_EMIT(closedGui());
}

void EntityDialog::setItemSelected(uint64_t id)
{
  auto ids = tree_->selectedItems();
  if ((ids.size() == 1) && (ids.front() == id))
    return;

  tree_->clearSelection();
  if (id != 0)
  {
    tree_->setSelected(id);
    tree_->scrollTo(id);
  }
}

void EntityDialog::setStateFilter(EntityStateFilter::State state)
{
  if (entityStateFilter_ != nullptr)
  {
    entityStateFilter_->setStateFilter(state);
  }
}

EntityStateFilter::State EntityDialog::stateFilter() const
{
  if (entityStateFilter_ != nullptr)
    return entityStateFilter_->stateFilter();

  return EntityStateFilter::BOTH;
}

void EntityDialog::setCenterEntity(CenterEntity* centerEntity)
{
  if (centerBind_ != nullptr)
  {
    // Dev error, call only once
    assert(false);
    return;
  }

  // OK to pass in a nullptr
  if (centerEntity == nullptr)
    return;

  centerBind_ = new BindCenterEntityToEntityTreeComposite(*centerEntity, *tree_, *entityTreeModel_->dataStore(), tree_);
  centerBind_->bind(false);
  tree_->setShowCenterInMenu(true);
}

void EntityDialog::setSelected_(const QList<uint64_t>& ids)
{
  if (!ids.isEmpty())
  {
    Q_EMIT itemSelected(ids.front());
  }
}

void EntityDialog::accept_()
{
  accept();
  Q_EMIT closedGui();
}

//--------------------------------------------------------------------------------------------------

/// notify the tree model about data store changes
class EntityLineEdit::DataStoreListener : public simData::DataStore::DefaultListener
{
public:
  /// constructor
  explicit DataStoreListener(EntityLineEdit *parent)
    : parent_(parent)
  {
  }

  /// entity with the given id and type will be removed after all notifications are processed
  virtual void onRemoveEntity(simData::DataStore *source, simData::ObjectId removedId, simData::ObjectType ot)
  {
    if (parent_->unavailableId_ == removedId)
      parent_->unavailableId_ = 0;

    if (parent_->uniqueId_ == removedId)
    {
      parent_->uniqueId_ = 0;
      parent_->setTextStyle_(false);
    }
  }

  /// entity name has changed
  virtual void onNameChange(simData::DataStore *source, simData::ObjectId changeId)
  {
    if (parent_->uniqueId_ == changeId)
      parent_->composite_->lineEdit->setText(simData::DataStoreHelpers::nameOrAliasFromId(changeId, source).c_str());
  }

protected:
  EntityLineEdit *parent_; ///< model which receives notices
};

//--------------------------------------------------------------------------------------------------

EntityLineEdit::EntityLineEdit(QWidget* parent, simQt::EntityTreeModel* entityTreeModel, simData::ObjectType type)
: QWidget(parent),
  composite_(nullptr),
  entityTreeModel_(nullptr), // set below with the setModel call
  entityDialog_(nullptr),
  uniqueId_(0),
  unavailableId_(0),
  valid_(true),
  needToVerify_(false),
  type_(type),
  clock_(nullptr),
  entityStateFilter_(nullptr),
  state_(EntityStateFilter::BOTH),
  settings_(SettingsPtr()),
  centerEntity_(nullptr)
{
  ResourceInitializer::initialize();  // Needs to be here so that Qt Designer works.

  composite_ = new Ui_EntityLineEdit();
  composite_->setupUi(this);
  composite_->lineEdit->setToolTip(simQt::formatTooltip(tr("Entity Name"), tr("Either type or select an entity name.<p>Select from the popup or from the dialog by clicking the browser button.")));
  composite_->lineEdit->setPlaceholderText(tr("Enter entity name..."));
  composite_->toolButton->setToolTip(simQt::formatTooltip(tr("Entity Selection"), tr("Display an Entity selection dialog with filtering capabilities.")));
  connect(composite_->toolButton, SIGNAL(clicked()), this, SLOT(showEntityDialog_()));
  connect(composite_->lineEdit, SIGNAL(returnPressed()), this, SLOT(checkForReapply_()));
  connect(composite_->lineEdit, SIGNAL(editingFinished()), this, SLOT(editingFinished_()));
  connect(composite_->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(textEdited_(QString)));

  setModel(entityTreeModel, type_);
  // Double clicking on an empty text field will display the Entity Dialog
  composite_->lineEdit->installEventFilter(this);
}

EntityLineEdit::~EntityLineEdit()
{
  if ((entityTreeModel_ != nullptr) && (dataListenerPtr_ != nullptr))
    entityTreeModel_->dataStore()->removeListener(dataListenerPtr_);
  closeEntityDialog();
  delete composite_;
}

void EntityLineEdit::setModel(simQt::EntityTreeModel* model, simData::ObjectType type, simCore::Clock* clock)
{
  type_ = type;
  clock_ = clock;

  if (model != nullptr)
  {
    entityTreeModel_ = model;
    entityTreeModel_->setToListView();

    proxy_ = new EntityProxyModel(this);
    if (clock != nullptr)
    {
      entityStateFilter_ = new simQt::EntityStateFilter(*entityTreeModel_->dataStore(), *clock_);
      proxy_->addEntityFilter(entityStateFilter_);  // proxy takes ownership of entityStateFilter_
    }
    proxy_->addEntityFilter(new simQt::EntityTypeFilter(*entityTreeModel_->dataStore(), type, type == simData::ALL));
    proxy_->setSourceModel(entityTreeModel_);

    QCompleter* completer = new QCompleter(proxy_, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionRole(Qt::DisplayRole);

    QTreeView* view = new QTreeView(this);
    view->header()->hide();  // Need to hide the column headers because the popup height does not account for the header obscuring a line
    view->setRootIsDecorated(false);
    // It would be nice to set the column widths, but the commands seem to be ignored
    completer->setPopup(view);
    // If the EntityLineEdit starts off disabled than the view is always disabled (Qt bug?); if forced enabled here then the view follows the EntityLineEdit enable/disable
    view->setEnabled(true);

    connect(completer, SIGNAL(activated(QModelIndex)), this, SLOT(wasActivated_(QModelIndex)));

    composite_->lineEdit->setCompleter(completer);

    dataListenerPtr_ = simData::DataStore::ListenerPtr(new DataStoreListener(this));
    entityTreeModel_->dataStore()->addListener(dataListenerPtr_);
  }
  else if (entityTreeModel_ != nullptr)
  {
    entityTreeModel_->dataStore()->removeListener(dataListenerPtr_);
    dataListenerPtr_.reset();
    entityTreeModel_ = nullptr;
  }
}

void EntityLineEdit::setStateFilter(EntityStateFilter::State state)
{
  if (state_ == state)
    return;

  state_ = state;

  if (entityStateFilter_ != nullptr)
    entityStateFilter_->setStateFilter(state_);

  if (entityDialog_ != nullptr)
    entityDialog_->setStateFilter(state_);

  Q_EMIT stateFilterChanged(state_);
}

EntityStateFilter::State EntityLineEdit::stateFilter() const
{
  return state_;
}

void EntityLineEdit::wasActivated_(const QModelIndex& index)
{
  if (entityTreeModel_ == nullptr)
    return;

  QCompleter* completer = composite_->lineEdit->completer();
  QAbstractProxyModel* proxyModel = qobject_cast<QAbstractProxyModel*>(completer->completionModel());
  Q_ASSERT(proxyModel != 0);

  // Unwind the double proxy, ours and the build in proxy of the completer
  QModelIndex modelIndex = proxy_->mapToSource(proxyModel->mapToSource(index));

  const uint64_t newId = entityTreeModel_->uniqueId(modelIndex);
  if (newId == uniqueId_)
    return;
  uniqueId_ = newId;
  needToVerify_ = false;
  setTextStyle_(true);
  Q_EMIT itemSelected(uniqueId_);
  if (entityDialog_ != nullptr)
    entityDialog_->setItemSelected(uniqueId_);
}

uint64_t EntityLineEdit::selected() const
{
  return uniqueId_;
}

QString EntityLineEdit::selectedName() const
{
  QModelIndex index = entityTreeModel_->index(uniqueId_);
  if (!index.isValid())
    return QString();

  return entityTreeModel_->data(index, Qt::DisplayRole).toString();
}

int EntityLineEdit::setSelected(uint64_t id)
{
  if (entityTreeModel_ == nullptr || id == uniqueId_)
    return 1;

  bool doEmit = (uniqueId_ != id);

  // Allow zero to clear out the line Edit
  if (id == 0)
  {
    composite_->lineEdit->setText("");
    uniqueId_ = id;
    needToVerify_ = true;
    setTextStyle_(false);
    if (entityDialog_ != nullptr)
      entityDialog_->setItemSelected(uniqueId_);
    if (doEmit)
      Q_EMIT itemSelected(uniqueId_);
    return 0;
  }

  QModelIndex index = entityTreeModel_->index(id);
  if (!index.isValid())
    return 1;

  QString name = entityTreeModel_->data(index, Qt::DisplayRole).toString();
  composite_->lineEdit->setText(name);
  uniqueId_ = id;
  needToVerify_ = false;
  setTextStyle_(true);
  if (entityDialog_ != nullptr)
    entityDialog_->setItemSelected(uniqueId_);
  if (doEmit)
    Q_EMIT itemSelected(uniqueId_);
  return 0;
}

void EntityLineEdit::setSettings(SettingsPtr settings)
{
  settings_ = settings;
}

void EntityLineEdit::setCenterEntity(CenterEntity* centerEntity)
{
  centerEntity_ = centerEntity;
}

void EntityLineEdit::showEntityDialog_()
{
  if (entityTreeModel_ == nullptr)
    return;

  if (entityDialog_ == nullptr)
  {
    entityDialog_ = new EntityDialog(this, entityTreeModel_, type_, clock_, settings_);
    entityDialog_->setStateFilter(state_);
    entityDialog_->setCenterEntity(centerEntity_);

    connect(entityDialog_, SIGNAL(itemSelected(uint64_t)), this, SLOT(setSelected(uint64_t)));
    connect(entityDialog_, SIGNAL(closedGui()), this, SLOT(closeEntityDialog()));
  }

  entityDialog_->setItemSelected(uniqueId_);
  entityDialog_->show();
}

void EntityLineEdit::closeEntityDialog()
{
  if (entityDialog_)
  {
    entityDialog_->hide();
    entityDialog_->deleteLater();
    entityDialog_ = nullptr;
  }
}

void EntityLineEdit::setUnavailable(uint64_t id)
{
  unavailableId_ = id;
  setTextStyle_(valid_);
}

void EntityLineEdit::checkForReapply_()
{
  auto oldId = uniqueId_;
  editingFinished_();
  if ((oldId == uniqueId_) && (oldId != 0))
    Q_EMIT reapplied(uniqueId_);
}

void EntityLineEdit::editingFinished_()
{
  if (entityTreeModel_ == nullptr)
    return;

  // Clearing out the line Edit is a special case
  if (composite_->lineEdit->text().isEmpty())
  {
    bool doEmit = (uniqueId_ != 0);
    uniqueId_ = 0;
    needToVerify_ = true;
    setTextStyle_(false);
    if (entityDialog_ != nullptr)
      entityDialog_->setItemSelected(uniqueId_);
    if (doEmit)
      Q_EMIT itemSelected(uniqueId_);
    return;
  }

  if (needToVerify_)
  {
    needToVerify_ = false;
    auto oldId = uniqueId_;
    uniqueId_ = simData::DataStoreHelpers::idByName(composite_->lineEdit->text().toStdString(), entityTreeModel_->dataStore());
    if ((uniqueId_ == 0) && (!composite_->lineEdit->text().isEmpty()))
      setTextStyle_(false);
    else
    {
      setTextStyle_(true);
      if (entityDialog_ != nullptr)
        entityDialog_->setItemSelected(uniqueId_);
    }
    if (oldId != uniqueId_)
      Q_EMIT itemSelected(uniqueId_);
  }
}

void EntityLineEdit::textEdited_(const QString& text)
{
  needToVerify_ = true;
  setTextStyle_(true);

  if (uniqueId_ != 0)
  {
    uniqueId_ = 0;
    Q_EMIT itemSelected(uniqueId_);
  }
}

QString EntityLineEdit::tooltip() const
{
  return composite_->lineEdit->toolTip();
}

void EntityLineEdit::setTooltip(const QString& tooltip)
{
  composite_->lineEdit->setToolTip(tooltip);
}

QString EntityLineEdit::placeholderText() const
{
  return composite_->lineEdit->placeholderText();
}
void EntityLineEdit::setPlaceholderText(const QString& text)
{
  composite_->lineEdit->setPlaceholderText(text);
}

bool EntityLineEdit::includeDialogButton() const
{
  return composite_->toolButton->isVisible();
}

void EntityLineEdit::setIncludeDialogButton(bool value)
{
  composite_->toolButton->setVisible(value);
}

bool EntityLineEdit::eventFilter(QObject* obj, QEvent* evt)
{
  if (obj == composite_->lineEdit)
  {
    if ((evt->type() == QEvent::MouseButtonDblClick) && composite_->lineEdit->text().isEmpty())
    {
      showEntityDialog_();
      return true;
    }
  }
  return false;
}

void EntityLineEdit::setTextStyle_(bool valid)
{
  // Do not short out, need to test needToVerify_ and unavailableId_ down below
  valid_ = valid;

  if (needToVerify_)
    composite_->lineEdit->setStyleSheet(VALID_ENTITY);
  else if (!valid_)
    composite_->lineEdit->setStyleSheet(INVALID_ENTITY);
  else if (uniqueId_ == unavailableId_)
    composite_->lineEdit->setStyleSheet(INVALID_ENTITY);
  else
    composite_->lineEdit->setStyleSheet(VALID_ENTITY);
}


//----------------------------------------------------------------------------------------------------------------

BoundEntityLineEdit::BoundEntityLineEdit(EntityLineEdit* parent, simQt::Settings& settings, const QString& variableName, const simQt::Settings::MetaData& metaData)
  : BoundIntegerSetting(parent, settings, variableName, metaData)
{
  parent->setStateFilter(static_cast<EntityStateFilter::State>(value()));
  connect(parent, SIGNAL(stateFilterChanged(simQt::EntityStateFilter::State)), this, SLOT(setStateFromLineEdit_(simQt::EntityStateFilter::State)));
  connect(this, SIGNAL(valueChanged(int)), this, SLOT(setStateFromSettings_(int)));
}

BoundEntityLineEdit::~BoundEntityLineEdit()
{
}

void BoundEntityLineEdit::setStateFromLineEdit_(EntityStateFilter::State state)
{
  setValue(static_cast<int>(state));
}

void BoundEntityLineEdit::setStateFromSettings_(int state)
{
  static_cast<EntityLineEdit*>(parent())->setStateFilter(static_cast<EntityStateFilter::State>(state));
}

simQt::Settings::MetaData BoundEntityLineEdit::metaData()
{
  QMap<int, QString> state;
  state.insert(0, tr("Active"));
  state.insert(1, tr("Inactive"));
  state.insert(2, tr("Both"));

  return simQt::Settings::MetaData(simQt::Settings::MetaData::makeEnumeration(
    0, state, "Entities to display in various controls.", simQt::Settings::DEFAULT));
}

}

