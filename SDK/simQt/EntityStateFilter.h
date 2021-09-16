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
#ifndef SIMQT_ENTITY_STATE_FILTER_H
#define SIMQT_ENTITY_STATE_FILTER_H

#include "simData/ObjectId.h"
#include "simCore/Time/Clock.h"
#include "simQt/EntityFilter.h"

namespace simData { class CategoryFilter; class DataStore; }

namespace simQt {

  /**
  * Class to implement a filter based on entity state. This filter can also be updated using on the widget provided.
  */
  class SDKQT_EXPORT EntityStateFilter : public EntityFilter
  {
    Q_OBJECT;
  public:

    /// Type of filtering
    enum State { ACTIVE, INACTIVE, BOTH };

    /**
    * Constructor
    * @param dataStore reference to the data store
    * @param clock reference to clock object
    * @param showWidget flag to indicate if a widget should be created
    */
    EntityStateFilter(const simData::DataStore& dataStore, simCore::Clock& clock, bool showWidget = false);

    /** Destructor */
    virtual ~EntityStateFilter();

    /**
    * Inherited from EntityFilter, determines if the specified entity passes this filter
    * @param id  entity id to test
    * @return bool  true if the entity passed this filter
    */
    virtual bool acceptEntity(simData::ObjectId id) const;

    /**
    * Inherited from EntityFilter, returns a new instance of the widget to be displayed, otherwise returns nullptr
    * @param newWidgetParent QWidget parent, useful for memory management purposes; may be nullptr if desired
    * @return QWidget used for changing filter settings
    */
    virtual QWidget* widget(QWidget* newWidgetParent) const;

    /** @copydoc EntityFilter::getFilterSettings() */
    virtual void getFilterSettings(QMap<QString, QVariant>& settings) const;

    /** @copydoc EntityFilter::setFilterSettings() */
    virtual void setFilterSettings(const QMap<QString, QVariant>& settings);

    /** Set the state filter to the given state */
    void setStateFilter(State state);

    /** Returns the current state filter */
    State stateFilter() const;

  signals:
    /**
     * Emitted when state is changed to update the widget
     * @param state The current state
     */
    void entityStateChanged(int state);

  private slots:
    /** A new filter state picked by the user */
    void entityStateChanged_(int state);

  private:
    class TimeObserver;

    /// Updates the filtering when time changes
    void newTime_();

    const simData::DataStore& dataStore_;  ///< reference to the data store
    simCore::Clock& clock_; ///< reference to the clock
    bool showWidget_; ///< indicates whether this filter should produce a widget or not
    State state_;  ///< Type of entities to filter out
    std::shared_ptr<TimeObserver> clockAdapter_;  ///< Used to monitor for time changes
  };
}

#endif

