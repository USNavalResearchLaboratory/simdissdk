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

#include "osg/observer_ptr"
#include "simCore/Common/Common.h"

namespace simData { class DataStore; }
namespace simVis {
  class FocusManager;
  class ScenarioManager;
  class CentroidManager;
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

// use BindCenterEntityToEntityTreeComposite instead
#ifdef USE_DEPRECATED_SIMDISSDK_API
  /** Constructor for EntityTreeComposite parent with an automatic call to bindTo */
  SDK_DEPRECATE(CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree), "Method will be removed in a future SDK release");
  /** Auto configures the double click to center on platform and turns off the tree expansion on double click */
  SDK_DEPRECATE(void bindTo(EntityTreeComposite& tree), "Method will be removed in a future SDK release");
#endif

  /**
   * Sets the centroid manager for centering views
   * @param centroidManager the centroid manager for centering views
   */
  void setCentroidManager(simVis::CentroidManager* centroidManager);

  virtual ~CenterEntity();

public slots:
  /** Center the current view port on the given entity Unique ID */
  void centerOnEntity(uint64_t id);
  /** Center the current view port on the given list of entity unique IDs */
  void centerOnSelection(const QList<uint64_t>& ids);

private:
  osg::observer_ptr<simVis::FocusManager> focusManager_;
  osg::observer_ptr<simVis::ScenarioManager> scenarioManager_;
  osg::observer_ptr<simVis::CentroidManager> centroidManager_;
};

/**
 * Class for managing the entity centering feature of the simQt::EntityTreeComposite.
 * The class will enable/disable the centering feature and does the actual centering with the CenterEntity object.
 */
class BindCenterEntityToEntityTreeComposite : public QObject
{
  Q_OBJECT;

public:
  BindCenterEntityToEntityTreeComposite(CenterEntity& centerEntity, EntityTreeComposite& tree, simData::DataStore& dataStore, QObject* parent = NULL);
  virtual ~BindCenterEntityToEntityTreeComposite();

  /**
   * Does the actual bind between the CenterEntity and EntityTreeComposite
   * @param centerOnDoubleClick If true, enables centering on an entity by double clicking it
   */
  void bind(bool centerOnDoubleClick);

private slots:
  void updateCenterEnable_();

private:
  CenterEntity& centerEntity_;
  EntityTreeComposite& tree_;
  simData::DataStore& dataStore_;
};

}

#endif
