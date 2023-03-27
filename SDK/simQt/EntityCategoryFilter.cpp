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
#include "simData/CategoryData/CategoryFilter.h"
#include "simData/DataStore.h"
#include "simQt/CategoryFilterWidget.h"
#include "simQt/CategoryTreeModel.h"
#include "simQt/RegExpImpl.h"
#include "simQt/EntityCategoryFilter.h"

namespace simQt {

/// Monitor the category name manager for clear which will reset the category filter
class EntityCategoryFilter::CategoryNameListener : public simData::CategoryNameManager::Listener
{
public:
  /// Constructor
  explicit CategoryNameListener(EntityCategoryFilter* parent)
    : parent_(parent)
  {
  }

  virtual ~CategoryNameListener()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex) override
  {
    // noop
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex) override
  {
    // noop
  }

  /// Invoked when all data is cleared
  virtual void onClear() override
  {
    parent_->fireActiveChange_(false);
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing() override
  {
    // noop
  }

private:
  EntityCategoryFilter* parent_;
};

//---------------------------------------------------------------------------------------------------

EntityCategoryFilter::EntityCategoryFilter(simData::DataStore* dataStore, WidgetType widgetType)
  : EntityFilter(),
    dataStore_(dataStore),
    categoryFilter_(new simData::CategoryFilter(dataStore_, true)),
    widgetType_(widgetType),
    settings_(nullptr),
    active_(false)
{
  listenerPtr_.reset(new CategoryNameListener(this));
  dataStore_->categoryNameManager().addListener(listenerPtr_);
}

EntityCategoryFilter::~EntityCategoryFilter()
{
  dataStore_->categoryNameManager().removeListener(listenerPtr_);
  delete categoryFilter_;
  categoryFilter_ = nullptr;
}

bool EntityCategoryFilter::acceptEntity(simData::ObjectId id) const
{
  return !dataStore_ || categoryFilter_->match(*dataStore_, id);
}

QWidget* EntityCategoryFilter::widget(QWidget* newWidgetParent) const
{
  // only generate the widget if we are set to show a widget
  switch (widgetType_)
  {
  case NO_WIDGET:
    break;
  case SHOW_WIDGET:
  {
    CategoryFilterWidget* rv = new CategoryFilterWidget(newWidgetParent);
    rv->setDataStore(categoryFilter_->getDataStore());
    rv->setFilter(*categoryFilter_);
    rv->setSettings(settings_, settingsKeyPrefix_);
    bindToWidget(rv);
    return rv;
  }
  }

  return nullptr;
}

void EntityCategoryFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
{
  QString filter = QString::fromStdString(categoryFilter_->serialize(false));
  settings.insert("EntityCategoryFilter", filter);
}

void EntityCategoryFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
{
  QMap<QString, QVariant>::const_iterator it = settings.find("EntityCategoryFilter");
  if (it != settings.end())
  {
    std::string filter = it.value().toString().toStdString();
    if (filter != categoryFilter_->serialize(false))
    {
      simQt::RegExpFilterFactoryImpl reFactory;
      categoryFilter_->deserialize(filter, false, &reFactory);

      // send out signal to alert any guis bound to this filter
      Q_EMIT categoryFilterChanged(*categoryFilter_);
      // send out general filter update signal
      Q_EMIT filterUpdated();
      // Send out active flag
      fireActiveChange_(!categoryFilter_->isEmpty());
    }
  }
}

void EntityCategoryFilter::bindToWidget(CategoryFilterWidget* widget) const
{
  // Whenever the filter updates in the GUI, update our internal filter,
  // which then in turn emits filterUpdated().
  connect(widget, SIGNAL(filterEdited(simData::CategoryFilter)), this, SLOT(setCategoryFilterFromGui_(simData::CategoryFilter)));

  // When internal filter gets changed, make the widget reflect those values.
  connect(this, SIGNAL(categoryFilterChanged(simData::CategoryFilter)), widget, SLOT(setFilter(simData::CategoryFilter)));
}

void EntityCategoryFilter::setCategoryFilter(const simData::CategoryFilter& categoryFilter)
{
  if ((*categoryFilter_) == categoryFilter)
    return;

  // use assign so that categoryFilter_ keeps its auto update
  categoryFilter_->assign(categoryFilter, false);
  // send out signal to alert any guis bound to this filter
  Q_EMIT categoryFilterChanged(*categoryFilter_);
  // send out general filter update signal
  Q_EMIT filterUpdated();
  // Send out active flag
  fireActiveChange_(!categoryFilter_->isEmpty());
}

const simData::CategoryFilter& EntityCategoryFilter::categoryFilter() const
{
  return *categoryFilter_;
}

void EntityCategoryFilter::setSettings(Settings* settings, const QString& settingsKeyPrefix)
{
  settings_ = settings;
  settingsKeyPrefix_ = settingsKeyPrefix;
}

void EntityCategoryFilter::setCategoryFilterFromGui_(const simData::CategoryFilter& categoryFilter)
{
  // use assign so that categoryFilter_ keeps its auto update
  categoryFilter_->assign(categoryFilter, false);
  // the GUI has changed the filter, send out general filter update signal
  Q_EMIT filterUpdated();
  // Send out active flag
  fireActiveChange_(!categoryFilter_->isEmpty());
}

void EntityCategoryFilter::fireActiveChange_(bool active)
{
  if (active == active_)
    return;

  active_ = active;
  Q_EMIT categoryFilterActive(active_);
}

}

