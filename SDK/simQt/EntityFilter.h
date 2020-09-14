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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_ENTITY_FILTER_H
#define SIMQT_ENTITY_FILTER_H

#include <cassert>
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include "simData/ObjectId.h"

class QWidget;

namespace simQt {

  /**
  * Base class for all filters used in the EntityProxyModel.  They must override all the methods.
  * NOTE: this class cannot be a pure virtual interface, since signals in a pure virtual class are
  * not supported by Qt.
  *
  * The acceptEntity method will be called by EntityProxyModel in its acceptLine method, so the application of
  * the filter happens in this method.
  * The implementation must also provide a means for EntityProxyModel to obtain a QWidget for this filter
  * by implementing the widget() method.  The caller is responsible for memory of the newly created widget.
  * Implementations should create the widget if they want it to be shown, otherwise, return nullptr.
  *
  * Implementations should emit the filterUpdated signal when the filter has changed, in case the user wants
  * to re-apply the filter to its model
  */
  class SDKQT_EXPORT EntityFilter : public QObject
  {
    Q_OBJECT;
  public:

    /** Destructor */
    virtual ~EntityFilter(){}

    /**
    * Determine if this entity id passes the entity filter
    * @param id  entity id
    * @return true if the entity passed the filter
    */
    virtual bool acceptEntity(simData::ObjectId id) const = 0;

    /**
    * Returns a new widget for this filter.  If the filter has no widget, or does not wish it to be shown,
    * this will return nullptr.  The memory for this new widget is now owned by the caller
    * @param newWidgetParent QWidget parent, useful for memory management purposes; may be nullptr if desired
    * @return QWidget used for changing filter settings
    */
    virtual QWidget* widget(QWidget* newWidgetParent) const = 0;

    /**
     * Get the settings for the filter
     * @param settings Adds to setting using a globally unique key to add data
     */
    virtual void getFilterSettings(QMap<QString, QVariant>& settings) const = 0;

    /**
     * Sets the filter with data from settings
     * @param settings Accesses settings with a globally unique key to get data
     */
    virtual void setFilterSettings(const QMap<QString, QVariant>& settings) = 0;

  signals:
    /** This signal should be emitted by the filter when an update has occurred */
    void filterUpdated();
  };
}

#endif
