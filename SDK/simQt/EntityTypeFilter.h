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
#ifndef SIMQT_ENTITY_TYPE_FILTER_H
#define SIMQT_ENTITY_TYPE_FILTER_H

#include "simData/ObjectId.h"
#include "simQt/EntityFilter.h"

namespace simData { class DataStore; }

namespace simQt {

  /**
  * Class to implement a filter based on entity type.  Define the simData::ObjectTypes that pass the filter,
  * either using the methods or passing in a bit mask of types.  This filter can also be updated using on the widget provided.
  */
  class SDKQT_EXPORT EntityTypeFilter : public EntityFilter
  {
    Q_OBJECT;
  public:

    /**
    * Constructor.  Pass in a bit mask of entity types that will pass the filter, a reference to the data
    * store for finding the entity type, and a flag to indicate if a widget should be created or not.
    * @param types  bit mask of entity types that pass the filter
    * @param dataStore  reference to the data store
    * @param showWidget  flag to indicate if a widget should be created
    */
    EntityTypeFilter(const simData::DataStore& dataStore, unsigned int types = simData::ALL, bool showWidget = false);

    /** Destructor */
    virtual ~EntityTypeFilter();

    /**
    * Inherited from EntityFilter, determines if the specified entity passes this filter
    * @param id  entity id to test
    * @return bool  true if the entity passed this filter
    */
    virtual bool acceptEntity(simData::ObjectId id) const;

    /**
    * Inherited from EntityFilter, returns a new instance of the widget to be displayed, otherwise returns NULL
    * @param newWidgetParent QWidget parent, useful for memory management purposes; may be NULL if desired
    * @return QWidget used for changing filter settings
    */
    virtual QWidget* widget(QWidget* newWidgetParent) const;

    /** @copydoc EntityFilter::getFilterSettings() */
    virtual void getFilterSettings(QMap<QString, QVariant>& settings) const;

    /** @copydoc EntityFilter::setFilterSettings() */
    virtual void setFilterSettings(const QMap<QString, QVariant>& settings);

    /**
    * Enable an entity type to pass the filter. Emits the filterUpdated signal
    * @param type The type to enable
    */
    void enableEntityType(simData::ObjectType type);

    /**
    * Disable an entity type to no longer pass the filter.  Emits the filterUpdated signal
    * @param type The type to disable
    */
    void disableEntityType(simData::ObjectType type);

  signals:
    /**
     * Emitted when type is changed to update the widget
     * @param types The types currently selected
     */
    void entityTypesChanged(unsigned int types);

  private slots:
    /** Manages updating the filterTypes_ based on the GUI widget updates, passes a bit mask of simData::ObjectType */
    void entityTypesChanged_(unsigned int types);

  private:
    unsigned int filterTypes_; ///< all the entity types that should pass this filter
    const simData::DataStore& dataStore_;  ///< reference to the data store
    bool showWidget_; ///< indicates whether this filter should produce a widget or not
  };
}

#endif

