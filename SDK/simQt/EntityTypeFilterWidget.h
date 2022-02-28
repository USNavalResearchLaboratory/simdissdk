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
#ifndef SIMQT_ENTITY_TYPE_FILTERWIDGET_H
#define SIMQT_ENTITY_TYPE_FILTERWIDGET_H

#include <set>
#include <QWidget>
#include "simData/ObjectId.h"

class Ui_EntityTypeFilter;

namespace simQt {

  /**
  * Class to implement an entity type filter widget.
  */
  class SDKQT_EXPORT EntityTypeFilterWidget : public QWidget
  {
   Q_OBJECT;
  public:
    /** Constructor */
    EntityTypeFilterWidget(QWidget* parent = nullptr, unsigned int types = simData::ALL);

    /** Destructor */
    virtual ~EntityTypeFilterWidget();

    /**
     * Return a bit mask of currently selected entity types
     * @return unsigned int mask of simData::ObjectType
     */
    unsigned int getSelections() const;

    /** Retrieve the currently selected entity types in a std::set. */
    std::set<simData::ObjectType> getSelectionsSet() const;

  public slots:
    /**
    * Set the current selected types in the widget
    * @param types bitmask of all the entity types to set as selected
    */
    void setSelections(unsigned int types);

    /** Alternative signature to setSelections that uses a set */
    void setSelections(const std::set<simData::ObjectType>& types);

  signals:
    /** Emits a bit mask of the new entity types selected, sent out whenever the GUI is changed */
    void entityTypesChanged(unsigned int types);

  private slots:
    /** Manages updating the filterTypes_ based on the GUI widget updates */
    void entityTypeClicked_();
    /** Manages toggling all entity types on/off */
    void toggleAllTypes_(bool activateAllTypes);

  private:
    Ui_EntityTypeFilter* ui_; ///< the ui

  };

}

#endif
