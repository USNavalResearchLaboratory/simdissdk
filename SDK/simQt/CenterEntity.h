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

#ifndef SIMQT_CENTERENTITY_H
#define SIMQT_CENTERENTITY_H

#include <QObject>

#include "simCore/Common/Common.h"

namespace simVis {
  class FocusManager;
  class ScenarioManager;
}

namespace simQt {

class EntityTreeComposite;

/**
  * A helper class to center the given entity in the current view port.
  * Two constructors are provided.  The generic constructor (QObject) provides just the centering feature.
  * The EntityTreeComposite version of the constructor provides the centering feature plus binding for
  * automatically making the connect call and for turning off the expand on double click.   With the automatic
  * binding a parent object just needs to instantiate the CenterEntity and "forget".  Since the Center Entity
  * is a child of the parent it will automatically get deleted when the parent is deleted.
  */
class SDKQT_EXPORT CenterEntity : public QObject
{
  Q_OBJECT;

public:
  /** Constructor for a generic parent */
  CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, QObject* parent=NULL);
  /** Constructor for EntityTreeComposite parent with an automatic call to bindTo */
  CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree);
  /** Auto configures the double click to center on platform and turns off the tree expansion on double click */
  void bindTo(EntityTreeComposite& tree);

  virtual ~CenterEntity();

public slots:
  /** Center the current view port on the given entity Unique ID */
  void centerOnEntity(uint64_t id);

private:
  simVis::FocusManager& focusManager_;
  simVis::ScenarioManager& scenarioManager_;
};

}

#endif
