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

#include "simData/DataStore.h"
#include "simQt/EntityTypeFilterWidget.h"
#include "simQt/EntityTypeFilter.h"

namespace simQt {

  EntityTypeFilter::EntityTypeFilter(const simData::DataStore& dataStore, unsigned int types, bool showWidget)
    :EntityFilter(),
    filterTypes_(types),
    dataStore_(dataStore),
    showWidget_(showWidget)
  {
  }

  EntityTypeFilter::~EntityTypeFilter()
  {
  }

  bool EntityTypeFilter::acceptEntity(simData::ObjectId id) const
  {
    simData::ObjectType type = dataStore_.objectType(id);
    return filterTypes_ & type;
  }

  QWidget* EntityTypeFilter::widget(QWidget* newWidgetParent) const
  {
    // only generate the widget if we are set to show a widget
    if (showWidget_)
    {
      EntityTypeFilterWidget* rv = new EntityTypeFilterWidget(newWidgetParent, filterTypes_);
      // connect to the signal so we can update the filter based on GUI changes
      connect(rv, SIGNAL(entityTypesChanged(unsigned int)), this, SLOT(entityTypesChanged_(unsigned int)));
      connect(this, SIGNAL(entityTypesChanged(unsigned int)), rv, SLOT(setSelections(unsigned int)));
      return rv;
    }
    return nullptr;
  }

  void EntityTypeFilter::getFilterSettings(QMap<QString, QVariant>& settings) const
  {
    settings.insert("EntityTypeFilter", static_cast<unsigned int>(filterTypes_));
  }

  void EntityTypeFilter::setFilterSettings(const QMap<QString, QVariant>& settings)
  {
    QMap<QString, QVariant>::const_iterator it = settings.find("EntityTypeFilter");
    if (it != settings.end())
    {
      // If no GUI, update internally, otherwise update GUI
      if (receivers(SIGNAL(entityTypesChanged(unsigned int))) == 0)
        entityTypesChanged_(it.value().toUInt());
      else
        Q_EMIT entityTypesChanged(it.value().toUInt());
    }
  }

  void EntityTypeFilter::enableEntityType(simData::ObjectType type)
  {
    filterTypes_ |= type;
    // we changed the filter, emit the signal
    Q_EMIT(filterUpdated());
  }

  void EntityTypeFilter::disableEntityType(simData::ObjectType type)
  {
    filterTypes_ =  filterTypes_ & (filterTypes_ ^ type);
    // we changed the filter, emit the signal
    Q_EMIT(filterUpdated());
  }

  void EntityTypeFilter::entityTypesChanged_(unsigned int types)
  {
    // the GUI has changed the filter, now emit the signal (users will want to know)
    filterTypes_ = types;
    Q_EMIT(filterUpdated());
  }

}

