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
  CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, QObject* parent=nullptr);
  virtual ~CenterEntity();

  /**
   * Sets the centroid manager for centering views
   * @param centroidManager the centroid manager for centering views
   */
  void setCentroidManager(simVis::CentroidManager* centroidManager);

  /**
   * Returns the view center-able node for the given id
   * @param id The entity to get a view center-able node
   * @return A node that a view can be centered on; returns nullptr on error.
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
  BindCenterEntityToEntityTreeComposite(CenterEntity& centerEntity, EntityTreeComposite& tree, simData::DataStore& dataStore, QObject* parent = nullptr);
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
  /** Returns the closest TSPI time to the given time if the platform is active and has TSPI points.  Returns -1.0 on error. */
  double getPlatformNearestTime_(double time, uint64_t id) const;
  /** Returns the closest draw data time to the given time if the custom rendering is active.  Returns -1.0 on error. */
  double getCustomRenderingNearestTime_(double time, uint64_t id) const;
  /** The valid time at or before the search time; returns -1.0 on error */
  double getCustomRenderingEarlierTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const;
  /** The valid time at or after the search time; returns -1.0 on error */
  double getCustomRenderingLaterTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const;
  /** Returns the closest RAE time to the given time if the beam is active.  Returns -1.0 on error. */
  double getBeamNearestTime_(double time, uint64_t id) const;
  /** Returns the closest RAE time to the given time if the gate is active.  Returns -1.0 on error. */
  double getGateNearestTime_(double time, uint64_t id) const;
  /** Returns the closest RAE time to the given time if the laser is active.  Returns -1.0 on error. */
  double getLaserNearestTime_(double time, uint64_t id) const;
  /** Returns the closest RAE time to the given time if the LOB is active.  Returns -1.0 on error. */
  double getLobGroupNearestTime_(double time, uint64_t id) const;
  /** Returns the closest FOV time to the given time if the projector is active.  Returns -1.0 on error. */
  double getProjectorNearestTime_(double time, uint64_t id) const;
  /** Target beams need different processing so they get their own routine. Returns -1.0 on error. */
  double getNearestTargetTime_(double searchTime, uint64_t id) const;
  /** Returns the closest time in update with data draw on */
  template<typename CommandSlice, typename UpdateSlice>
  double getNearestDrawTime_(double time, uint64_t id, const CommandSlice* commands, const UpdateSlice* updates) const;
  /** Returns the time closest to searchTime;  returns INVALID if both earlierTime and laterTime are invalid */
  double getNearestTime_(double searchTime, double earlierTime, double laterTime) const;

  /** Gets the draw state of host of id; returns 0 on success. */
  int getHostDrawState_(uint64_t id, std::map<double, bool>& hostDrawState) const;
  /** Gets the draw state of a target beam by its targets; returns 0 on success. */
  int getTargetDrawState_(uint64_t id, std::map<double, bool>& drawState) const;
  /** Gets the draw state from the given commands; returns 0 on success.*/
  template<typename CommandSlice>
  int getEntityDrawState_(const CommandSlice* commands, std::map<double, bool>& drawState) const;

  /** Gets the time range of id as limited by its data and the life span of its host; returns 0 on success. */
  int getHostTimeRange_(uint64_t id, double& beginTime, double& endTime) const;
  /** Gets the time range of id as limited by its data, if static returns the time span of the scenario; returns 0 on success. */
  int getPlatformTimeRange_(uint64_t id, double& beginTime, double& endTime) const;
  /** Gets the time range based on the beam's targets */
  int getTargetTimeRange_(uint64_t id, double& beginTime, double& endTime) const;
  /** Gets the time range of id as limited by its data; returns 0 on success. */
  template<typename UpdateSlice>
  int getTimeRange_(uint64_t id, double& beginTime, double& endTime, const UpdateSlice* updates) const;

  /** Returns true if time is active for the given time. */
  bool isActive_(double time, const std::map<double, bool>& drawState) const;
  /** Returns true if time is between the beginTime and the endTime */
  bool inHostedTimeRange_(double time, double beginTime, double endTime) const;
  /** Returns true if the given id is a target beam */
  bool isTargetBeam_(uint64_t id) const;

  CenterEntity& centerEntity_;
  EntityTreeComposite& tree_;
  simData::DataStore& dataStore_;
  std::unique_ptr<simCore::TimeFormatterRegistry> timeFormatter_;
  simCore::TimeFormat timeFormat_;
  unsigned short precision_;
  double newTime_;  ///< If not INVALID_TIME, it represents the time (seconds since reference year) needed to make the entity valid for a view center
};

}

#endif
