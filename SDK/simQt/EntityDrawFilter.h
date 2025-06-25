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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMQT_ENTITY_DRAW_FILTER_H
#define SIMQT_ENTITY_DRAW_FILTER_H

#include <memory>
#include "simData/ObjectId.h"
#include "simQt/EntityFilter.h"

namespace simData { class DataStore; }

namespace simQt {

/**
* Class to implement a filter based on entity draw state. This filter can also be updated using the widget provided.
*/
class SDKQT_EXPORT EntityDrawFilter : public simQt::EntityFilter
{
  Q_OBJECT;
public:

  /// Type of filtering
  enum class Draw
  {
    DRAW_ON = 0,
    DRAW_OFF,
    BOTH
  };

  /**
  * Constructor
  * @param dataStore to access draw state of entities
  * @param showWidget flag to indicate if a widget should be created
  */
  explicit EntityDrawFilter(simData::DataStore& dataStore, bool showWidget = false);

  /** Destructor */
  virtual ~EntityDrawFilter();

  /**
  * Inherited from EntityFilter, determines if the specified entity passes this filter
  * @param id  entity id to test
  * @return bool  true if the entity passed this filter
  */
  virtual bool acceptEntity(simData::ObjectId id) const override;

  /**
  * Inherited from EntityFilter, returns a new instance of the widget to be displayed, otherwise returns nullptr; memory is owned by caller
  * @param newWidgetParent QWidget parent, useful for memory management purposes; may be nullptr if desired
  * @return QWidget used for changing filter settings
  */
  virtual QWidget* widget(QWidget* newWidgetParent) const override;

  /** @copydoc EntityFilter::getFilterSettings() */
  virtual void getFilterSettings(QMap<QString, QVariant>& settings) const override;

  /** @copydoc EntityFilter::setFilterSettings() */
  virtual void setFilterSettings(const QMap<QString, QVariant>& settings) override;

  /** Set the draw filter to the given state, either directly or through a connected widget if it exits */
  void setDrawFilter(Draw drawState);

  /** Returns the current draw filter state */
  Draw drawFilter() const;

Q_SIGNALS:
  /**
   * Emitted when draw filter is changed to update the widget
   * @param draw The current draw filter
   */
  void entityDrawChanged(int draw);

private Q_SLOTS:
  /** Sets the draw filter value, ignoring any widget that might be connected */
  void setDrawFilterInternal_(int draw);

private:
  class DataStoreListener;

  /** Check if the draw state changed for the specified entity */
  void checkDrawState_(simData::ObjectId entityId);
  /** Get the draw state for the specified entity */
  bool getDrawState_(simData::ObjectId entityId) const;

  simData::DataStore& dataStore_;
  bool showWidget_ = false; ///< indicates whether this filter should produce a widget or not
  Draw draw_ = Draw::BOTH;  ///< draw state of entities to filter on
  std::map<simData::ObjectId, bool> entityDrawStates_; ///< map of entity id to draw state
  std::shared_ptr<DataStoreListener> dsListener_; ///< Listener for datastore entity events
};
}


#endif

