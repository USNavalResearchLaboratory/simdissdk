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
#ifndef SIMQT_ENTITY_CATEGORY_FILTER_H
#define SIMQT_ENTITY_CATEGORY_FILTER_H

#include "simData/CategoryData/CategoryNameManager.h"
#include "simQt/EntityFilter.h"

namespace simData {
  class CategoryFilter;
  class DataStore;
}

namespace simQt {

class CategoryFilterWidget;
class Settings;

/**
 * Class to implement a filter based on entity category data, using the simQt::CategoryFilter.
 * This filter can also be updated using the widget provided.
 */
class SDKQT_EXPORT EntityCategoryFilter : public EntityFilter
{
  Q_OBJECT;
public:
  /** Enumeration of different ways we can create/display a widget for this filter. */
  enum WidgetType
  {
    /** widget() will return nullptr, creating nothing when integrated into Qt. */
    NO_WIDGET,
    /** widget() will return a CategoryFilterWidget, the new style of category filtering. */
    SHOW_WIDGET
  };

  /**
   * Constructor.  Pass in a reference to the data store and a flag to indicate if a widget should be created or not.
   * This will create a default CategoryFilter based on all CategoryData in the DataStore
   * @param dataStore Data store that maintains category data information
   * @param widgetType Enumeration to indicate if a widget should be created and what type
   */
  explicit EntityCategoryFilter(simData::DataStore* dataStore, WidgetType widgetType = NO_WIDGET);

  /** Destructor */
  virtual ~EntityCategoryFilter();

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

  /**
   * Bind this filter to a CategoryFilterWidget so that changes to either widget updates the other widget.
   * @param widget to be bound
   */
  void bindToWidget(CategoryFilterWidget* widget) const;

  /** Retrieves the current category filter. */
  const simData::CategoryFilter& categoryFilter() const;

  /** Set the settings object and key prefix that gets used by the CategoryFilterWidget */
  void setSettings(Settings* settings, const QString& settingsKeyPrefix);

public slots:
  /**
   * Set a new CategoryFilter for this filter. Emits the general filterUpdated() and more specific categoryFilterChanged() signals
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
  /**
   * Set a new CategoryFilter for this filter. Emits only the general filterUpdated().
   * @param categoryFilter New filter to install
   */
  void setCategoryFilterFromGui_(const simData::CategoryFilter& categoryFilter);

private:
  /// Data store that we have been configured with
  simData::DataStore* dataStore_;
  /// holds the current category filter
  simData::CategoryFilter* categoryFilter_;
  /// type of widget to create on call to widget(), if any
  WidgetType widgetType_;
  /// ptr to settings that gets passed to the category filter widget
  Settings* settings_;
  /// settings key prefix that gets passed to the category filter widget
  QString settingsKeyPrefix_;
};

}

#endif
