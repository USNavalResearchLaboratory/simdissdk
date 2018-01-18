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
#include "simData/CategoryData/CategoryFilter.h"
#include "simQt/CategoryFilterWidget.h"
#include "simQt/CategoryTreeModel2.h"
#include "simQt/EntityCategoryFilter.h"

namespace simQt {

EntityCategoryFilter::EntityCategoryFilter(simData::DataStore* dataStore, WidgetType widgetType)
  : EntityFilter(),
    categoryFilter_(new simData::CategoryFilter(dataStore, true)),
    widgetType_(widgetType)
{
}

EntityCategoryFilter::EntityCategoryFilter(const simData::CategoryFilter& categoryFilter, WidgetType widgetType)
  : EntityFilter(),
    categoryFilter_(new simData::CategoryFilter(categoryFilter)),
    widgetType_(widgetType)
{
}

EntityCategoryFilter::~EntityCategoryFilter()
{
  delete categoryFilter_;
  categoryFilter_ = NULL;
}

bool EntityCategoryFilter::acceptEntity(simData::ObjectId id) const
{
  return categoryFilter_->match(id);
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
    CategoryFilterWidget2* rv = new CategoryFilterWidget2(newWidgetParent);
    rv->setDataStore(categoryFilter_->getDataStore());
    rv->setFilter(*categoryFilter_);
    bindToWidget(rv);
    return rv;
  }
  case SHOW_LEGACY_WIDGET:
  {
    CategoryFilterWidget* rv = new CategoryFilterWidget(newWidgetParent);
    rv->setProviders(categoryFilter_->getDataStore());
    rv->setFilter(*categoryFilter_);
    bindToWidget(rv);
    return rv;
  }
  }

  return NULL;
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
      categoryFilter_->deserialize(filter, false);

      // send out signal to alert any guis bound to this filter
      emit categoryFilterChanged(*categoryFilter_);
      // send out general filter update signal
      emit filterUpdated();
    }
  }
}

void EntityCategoryFilter::bindToWidget(CategoryFilterWidget2* widget) const
{
  // Whenever the filter updates in the GUI, update our internal filter,
  // which then in turn emits filterUpdated().
  connect(widget, SIGNAL(filterEdited(simData::CategoryFilter)), this, SLOT(setCategoryFilterFromGui_(simData::CategoryFilter)));

  // When internal filter gets changed, make the widget reflect those values.
  connect(this, SIGNAL(categoryFilterChanged(simData::CategoryFilter)), widget, SLOT(setFilter(simData::CategoryFilter)));
}

void EntityCategoryFilter::bindToWidget(CategoryFilterWidget* widget) const
{
  // connect to the signals/slots between the gui and the filter so changes to one will update the other
  connect(widget, SIGNAL(categoryFilterChanged(const simData::CategoryFilter&)), this, SLOT(setCategoryFilterFromGui_(const simData::CategoryFilter&)));
  connect(this, SIGNAL(categoryFilterChanged(simData::CategoryFilter)), widget, SLOT(setFilter(simData::CategoryFilter)));
}

void EntityCategoryFilter::setCategoryFilter(const simData::CategoryFilter& categoryFilter)
{
  if (categoryFilter_->getCategoryFilter() == categoryFilter.getCategoryFilter())
    return;

  *categoryFilter_ = categoryFilter;
  // send out signal to alert any guis bound to this filter
  emit categoryFilterChanged(*categoryFilter_);
  // send out general filter update signal
  emit filterUpdated();
}

const simData::CategoryFilter& EntityCategoryFilter::categoryFilter() const
{
  return *categoryFilter_;
}

void EntityCategoryFilter::setCategoryFilterFromGui_(const simData::CategoryFilter& categoryFilter)
{
  *categoryFilter_ = categoryFilter;
  // the GUI has changed the filter, send out general filter update signal
  emit filterUpdated();
}

}

