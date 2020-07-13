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

#ifndef SIMQT_CENTERENTITY_H
#define SIMQT_CENTERENTITY_H

#include <memory>
#include <QObject>

#include "osg/observer_ptr"
#include "simCore/Common/Common.h"
#include "simCore/Time/Constants.h"
#include "simData/DataSlice.h"

namespace simCore { class TimeFormatterRegistry; }
namespace simData { class DataStore; }
namespace simVis {
  class EntityNode;
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
  virtual ~CenterEntity();

  /**
   * Sets the centroid manager for centering views
   * @param centroidManager the centroid manager for centering views
   */
  void setCentroidManager(simVis::CentroidManager* centroidManager);

  /**
   * Returns the view center-able node for the given id
   * @param id The entity to get a view center-able node
   * @return A node that a view can be centered on; returns NULL on error.
   */
  simVis::EntityNode* getViewCenterableNode(uint64_t id) const;

public slots:
  /**
   * Center the current view port on the given entity Unique ID
   * @param id The entity to center on
   * @param force Center on an invalid entity with the expectation it will soon become valid
   */
  void centerOnEntity(uint64_t id, bool force = false);
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
class SDKQT_EXPORT BindCenterEntityToEntityTreeComposite : public QObject
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

public slots:
  /** The format for displaying the time in the right click mouse menu */
  void setTimeFormat(simCore::TimeFormat timeFormat);
  /** The digits after the decimal point in the time format */
  void setTimePrecision(unsigned int precision);

private slots:
  void updateCenterEnable_();
  void centerOnEntity_(uint64_t id);

private:
  /** Returns the closet TSPI time to the current time if the platform is active and has TSPI points.  Return -1.0 on error */
  double getPlatformNearestTime_(uint64_t id) const;
  /** Returns the closest draw data time to the current time if the custom rendering is active.  Return -1.0 on error */
  double getCustomRenderingNearestTime_(uint64_t id) const;
  /** The valid time at or before the search time; returns -1.0 on error */
  double getCustomRenderingEarlierTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const;
  /** The valid time at or after the search time; returns -1.0 on error */
  double getCustomRenderingLaterTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const;

  CenterEntity& centerEntity_;
  EntityTreeComposite& tree_;
  simData::DataStore& dataStore_;
  std::unique_ptr<simCore::TimeFormatterRegistry> timeFormatter_;
  simCore::TimeFormat timeFormat_;
  unsigned short precision_;
  double newTime_;  ///< If not -1, it represents the time (seconds since reference year) needed to make the entity valid for a view center
};

}

#endif
