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
#ifndef SIMQT_ENTITY_CATEGORY_FILTER_H
#define SIMQT_ENTITY_CATEGORY_FILTER_H

#include "simData/CategoryData/CategoryNameManager.h"
#include "simQt/EntityFilter.h"

namespace simData { class CategoryFilter; }

namespace simQt {

  /**
  * Class to implement a filter based on entity category data, using the simQt::CategoryFilter.
  * This filter can also be updated using the widget provided.
  */
  class SDKQT_EXPORT EntityCategoryFilter : public EntityFilter
  {
    Q_OBJECT;
  public:

   /**
    * Constructor.  Pass in a reference to the data store and a flag to indicate if a widget should be created or not.
    * This will create a default CategoryFilter based on all CategoryData in the DataStore
    * @param dataStore
    * @param showWidget  flag to indicate if a widget should be created
    */
    EntityCategoryFilter(simData::DataStore* dataStore, bool showWidget = false);

    /**
    * Constructor.  Pass in a CategoryFilter object and a flag to indicate if a widget should be created or not.
    * @param categoryFilter
    * @param showWidget  flag to indicate if a widget should be created
    */
    EntityCategoryFilter(const simData::CategoryFilter& categoryFilter, bool showWidget = false);

    /** Destructor */
    virtual ~EntityCategoryFilter();

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
    * Set a new CategoryFilter for this filter. Emits the entityTypesChanged_ signal
    * @param categoryFilter New filter to install
    */
    void setCategoryFilter(const simData::CategoryFilter& categoryFilter);

  signals:
    /**
     * Emitted when category filter is changed to update the widget
     * @param categoryFilter The filter
     */
    void categoryFilterChanged(const simData::CategoryFilter& categoryFilter);

  private slots:
    /** Manages updating the categoryFilter_ based on the GUI widget updates, passes a CategoryFilter object */
    void categoryFilterChanged_(const simData::CategoryFilter& categoryFilter);

  private:
    /// holds the current category filter
    simData::CategoryFilter* categoryFilter_;
    /// indicates whether this filter should produce a widget or not
    bool showWidget_;
  };
}

#endif
