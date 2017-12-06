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
#include "simQt/EntityCategoryFilter.h"

namespace simQt {

EntityCategoryFilter::EntityCategoryFilter(simData::DataStore* dataStore, bool showWidget)
  : EntityFilter(),
    categoryFilter_(new simData::CategoryFilter(dataStore, true)),
    showWidget_(showWidget)
{
}

EntityCategoryFilter::EntityCategoryFilter(const simData::CategoryFilter& categoryFilter, bool showWidget)
  : EntityFilter(),
    categoryFilter_(new simData::CategoryFilter(categoryFilter)),
    showWidget_(showWidget)
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
  if (showWidget_)
  {
    CategoryFilterWidget* rv = new CategoryFilterWidget(newWidgetParent);
    rv->setProviders(categoryFilter_->getDataStore());
    rv->setFilter(*categoryFilter_);
    // connect to the signal so we can update the filter based on GUI changes
    connect(rv, SIGNAL(categoryFilterChanged(const simData::CategoryFilter&)), this, SLOT(setCategoryFilter(const simData::CategoryFilter&)));
    connect(this, SIGNAL(categoryFilterChanged(simData::CategoryFilter)), rv, SLOT(setFilter(simData::CategoryFilter)));
    return rv;
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
      if (receivers(SIGNAL(categoryFilterChanged(simData::CategoryFilter))) != 0)
        emit categoryFilterChanged(*categoryFilter_);
      // Intentionally no else because CategoryFilterWidget does not send out a signal so send it out here
      emit filterUpdated();
    }
  }
}

void EntityCategoryFilter::setCategoryFilter(const simData::CategoryFilter& categoryFilter)
{
  *categoryFilter_ = categoryFilter;
  // the GUI has changed the filter, now emit the signal (users will want to know)
  emit filterUpdated();
}

}

