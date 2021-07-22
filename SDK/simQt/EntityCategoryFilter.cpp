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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "simData/CategoryData/CategoryFilter.h"
#include "simQt/CategoryFilterWidget.h"
#include "simQt/CategoryTreeModel.h"
#include "simQt/RegExpImpl.h"
#include "simQt/EntityCategoryFilter.h"

namespace simQt {

EntityCategoryFilter::EntityCategoryFilter(simData::DataStore* dataStore, WidgetType widgetType)
  : EntityFilter(),
    dataStore_(dataStore),
    categoryFilter_(new simData::CategoryFilter(dataStore_, true)),
    widgetType_(widgetType),
    settings_(nullptr)
{
}

EntityCategoryFilter::~EntityCategoryFilter()
{
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
      emit categoryFilterChanged(*categoryFilter_);
      // send out general filter update signal
      emit filterUpdated();
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
  emit categoryFilterChanged(*categoryFilter_);
  // send out general filter update signal
  emit filterUpdated();
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
  emit filterUpdated();
}

}

