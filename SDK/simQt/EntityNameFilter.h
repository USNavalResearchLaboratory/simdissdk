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
#ifndef SIMQT_ENTITY_NAME_FILTER_H
#define SIMQT_ENTITY_NAME_FILTER_H

#include "simQt/EntityFilter.h"

namespace simQt {

class AbstractEntityTreeModel;
class EntityFilterLineEdit;
class RegExpImpl;

/**
* Class to implement a filter based on entity name using Qt regular expression implementation. This filter can bind to an EntityFilterLineEdit widget.
*/
class SDKQT_EXPORT EntityNameFilter : public EntityFilter
{
  Q_OBJECT;
public:

  /**
  * Constructor
  * @param model ptr to the entity tree model for finding the name by id
  */
  explicit EntityNameFilter(AbstractEntityTreeModel* model);

  /** Destructor */
  virtual ~EntityNameFilter();

  /**
  * Inherited from EntityFilter, determines if the specified entity passes this filter
  * @param id  entity id to test
  * @return bool  true if the entity passed this filter
  */
  virtual bool acceptEntity(simData::ObjectId id) const;

  /**
  * Inherited from EntityFilter, always returns NULL, as this class currently doesn't create a widget
  * @param newWidgetParent QWidget parent, useful for memory management purposes; may be NULL if desired
  * @return QWidget used for changing filter settings
  */
  virtual QWidget* widget(QWidget* newWidgetParent) const;

  /** @copydoc EntityFilter::getFilterSettings() */
  virtual void getFilterSettings(QMap<QString, QVariant>& settings) const;

  /** @copydoc EntityFilter::setFilterSettings() */
  virtual void setFilterSettings(const QMap<QString, QVariant>& settings);

  /** Returns the filter's QRegExp*/
  QRegExp regExp() const;

  /** Connect to the specified widget for updating and receiving reg exp filter */
  void bindToWidget(EntityFilterLineEdit* widget);

  /** Update the model ptr */
  void setModel(AbstractEntityTreeModel* model);

public slots:
  /// Set the filter's QRegExp
  void setRegExp(const QRegExp& regExp);

private slots:
  /// Set the attributes of the QRegExp filter
  void setRegExpAttributes_(QString filter, Qt::CaseSensitivity caseSensitive, QRegExp::PatternSyntax expression);

private:
  /// Recursively determines if the specified index or any of its children pass the filter
  bool acceptIndex_(const QModelIndex& idx) const;

  // reference to the entity tree model for looking up entity name
  AbstractEntityTreeModel* model_;
  /// regular expression filter to apply to entity name
  RegExpImpl* regExp_;
  /// widget that generates a reg exp filter
  EntityFilterLineEdit* widget_;
};

}

#endif

