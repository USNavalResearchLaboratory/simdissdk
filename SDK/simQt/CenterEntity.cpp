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
#include <limits>
#include "simCore/Time/Clock.h"
#include "simCore/Time/String.h"
#include "simVis/CentroidManager.h"
#include "simVis/CustomRendering.h"
#include "simVis/Scenario.h"
#include "simVis/Platform.h"
#include "simVis/Gate.h"
#include "simVis/View.h"
#include "simQt/EntityTreeComposite.h"
#include "simQt/CenterEntity.h"

namespace simQt {

// The amount of time, in seconds, to back into a custom rendering valid time range
static const double TIME_DELTA = 1e-6;

CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, QObject* parent)
  : QObject(parent),
    focusManager_(&focusManager),
    scenarioManager_(&scenarioManager)
{
}

CenterEntity::~CenterEntity()
{
}

void CenterEntity::centerOnSelection(const QList<uint64_t>& ids)
{
  if (ids.empty())
    return;
  // Use center entity if only one id is selected
  if (ids.size() == 1)
    centerOnEntity(ids[0]);
  else
  {
    // Need the centroid, scenario, and focus managers to continue
    if (!centroidManager_ || !scenarioManager_ || !focusManager_)
    {
      return;
    }

    // Create a centroid node about the selected ids
    std::vector<simVis::EntityNode*> nodes;
    // Get the entity nodes involved
    for (auto it = ids.begin(); it != ids.end(); ++it)
    {
      simVis::EntityNode* node = scenarioManager_->find<simVis::EntityNode>(*it);
      if (node)
        nodes.push_back(node);
    }
    // Center the focused view on the given ids
    centroidManager_->centerViewOn(nodes, focusManager_->getFocusedView());
  }
}

void CenterEntity::centerOnEntity(uint64_t id, bool force)
{
  // Need the scenario and focus manager to continue
  if (!scenarioManager_ || !focusManager_)
    return;

  simVis::EntityNode* node = getViewCenterableNode(id);
  if ((node != nullptr) && (force  || (node->isActive() && node->isVisible())))
  {
    if (focusManager_->getFocusedView() != nullptr)
    {
      focusManager_->getFocusedView()->tetherCamera(node);
    }
  }
}

void CenterEntity::setCentroidManager(simVis::CentroidManager* centroidManager)
{
  centroidManager_ = centroidManager;
}

simVis::EntityNode* CenterEntity::getViewCenterableNode(uint64_t id) const
{
  if (!scenarioManager_ || !focusManager_)
    return nullptr;

  auto node = focusManager_->getFocusedView()->getModelNodeForTether(scenarioManager_->find(id));
  return focusManager_->getFocusedView()->getEntityNode(node);
}

//--------------------------------------------------------------------------------------------------

BindCenterEntityToEntityTreeComposite::BindCenterEntityToEntityTreeComposite(CenterEntity& centerEntity, EntityTreeComposite& tree, simData::DataStore& dataStore, QObject* parent)
  : QObject(parent),
    centerEntity_(centerEntity),
    tree_(tree),
    dataStore_(dataStore),
    timeFormatter_(new simCore::TimeFormatterRegistry),
    timeFormat_(simCore::TIMEFORMAT_ORDINAL),
    precision_(3),
    newTime_(-1.0)
{
}

BindCenterEntityToEntityTreeComposite::~BindCenterEntityToEntityTreeComposite()
{
}

void BindCenterEntityToEntityTreeComposite::bind(bool centerOnDoubleClick)
{
  connect(&tree_, SIGNAL(rightClickMenuRequested()), this, SLOT(updateCenterEnable_()));
  connect(&tree_, SIGNAL(centerOnEntityRequested(uint64_t)), this, SLOT(centerOnEntity_(uint64_t)));
  connect(&tree_, SIGNAL(centerOnSelectionRequested(QList<uint64_t>)), &centerEntity_, SLOT(centerOnSelection(QList<uint64_t>)));
  if (centerOnDoubleClick)
  {
    connect(&tree_, SIGNAL(itemDoubleClicked(uint64_t)), &centerEntity_, SLOT(centerOnEntity(uint64_t)));
    tree_.setExpandsOnDoubleClick(false); // Turns off the tree expansion on double click.
  }
}

void BindCenterEntityToEntityTreeComposite::setTimeFormat(simCore::TimeFormat timeFormat)
{
  timeFormat_ = timeFormat;
}

void BindCenterEntityToEntityTreeComposite::setTimePrecision(unsigned int precision)
{
  precision_ = static_cast<unsigned short>(precision);
}

void BindCenterEntityToEntityTreeComposite::updateCenterEnable_()
{
  // Clear out any previous center on inactive platform
  newTime_ = -1.0;

  QList<uint64_t> ids = tree_.selectedItems();
  if (ids.empty())
  {
    tree_.setUseCenterAction(false, tr("No entities selected"));
    return;
  }

  // Make sure all entities are active
  for (auto it = ids.begin(); it != ids.end(); ++it)
  {
    auto node = centerEntity_.getViewCenterableNode(*it);
    if ((node == nullptr) || (node->isActive() && !node->isVisible()))
    {
      tree_.setUseCenterAction(false, tr("Inactive entity selected"));
      return;
    }

    // If there is one selected entity look for a time to make the center command valid
    if (!node->isActive())
    {
      // If more than one entity is selected don't try to find a time where all are active
      if (ids.size() != 1)
      {
        tree_.setUseCenterAction(false, tr("Inactive entity selected"));
        return;
      }

      // Make sure time controls are enabled and that the scenario is in file mode
      if ((dataStore_.getBoundClock() == nullptr) || dataStore_.getBoundClock()->controlsDisabled() || dataStore_.getBoundClock()->isLiveMode())
      {
        tree_.setUseCenterAction(false, tr("Inactive entity selected"));
        return;
      }

      const auto time = dataStore_.updateTime();
      auto id = ids.front();
      switch (dataStore_.objectType(id))
      {
      case simData::PLATFORM:
        newTime_ = getPlatformNearestTime_(time, id);
        break;
      case simData::CUSTOM_RENDERING:
        newTime_ = getCustomRenderingNearestTime_(time, id);
        break;
      case simData::BEAM:
        newTime_ = getBeamNearestTime_(time, id);
        break;
      case simData::GATE:
        newTime_ = getGateNearestTime_(time, id);
        break;
      case simData::LASER:
        newTime_ = getLaserNearestTime_(time, id);
        break;
      case simData::LOB_GROUP:
        newTime_ = getLobGroupNearestTime_(time, id);
        break;
      case simData::PROJECTOR:
        newTime_ = getProjectorNearestTime_(time, id);
        break;
      case simData::NONE:
      case simData::ALL:
        break;
      }
    }
  }

  if (newTime_ == -1.0)
  {
    tree_.setUseCenterAction(false, tr("Inactive entity selected"));
    return;
  }

  QString message;
  message = "Time ";
  int referenceYear = dataStore_.referenceYear();
  const simCore::TimeStamp time(referenceYear, newTime_);
  message += QString::fromStdString(timeFormatter_->toString(timeFormat_, time, referenceYear, precision_));

  tree_.setUseCenterAction(true, message);
}

void BindCenterEntityToEntityTreeComposite::centerOnEntity_(uint64_t id)
{
  if ((newTime_ != -1.0) && (dataStore_.getBoundClock() != nullptr) && !dataStore_.getBoundClock()->controlsDisabled() && !dataStore_.getBoundClock()->isLiveMode())
    dataStore_.getBoundClock()->setTime(simCore::TimeStamp(dataStore_.referenceYear(), newTime_));

  // Need to force the center because the setTime has not been process so the entity may not yet be valid
  centerEntity_.centerOnEntity(id, true);
}

double BindCenterEntityToEntityTreeComposite::getPlatformNearestTime_(double time, uint64_t id) const
{
  // First check the visible flag
  simData::DataStore::Transaction trans;
  auto pref = dataStore_.platformPrefs(id, &trans);
  if ((pref == nullptr) || !pref->commonprefs().draw() || !pref->commonprefs().datadraw())
    return -1.0;
  trans.release(&pref);

  // Next check data points
  auto slice = dataStore_.platformUpdateSlice(id);
  if ((slice == nullptr) || (slice->numItems() == 0))
    return -1.0;

  auto iter = slice->upper_bound(time);

  // Since there is a check above for at least one point, previous or next must be set

  if (iter.peekNext() == nullptr)
    return iter.peekPrevious()->time();

  if (iter.peekPrevious() == nullptr)
    return iter.peekNext()->time();

  const double nextDelta = iter.peekNext()->time() - time;
  const double previousDelta = time - iter.peekPrevious()->time();

  return nextDelta < previousDelta ? iter.peekNext()->time() : iter.peekPrevious()->time();
}

double BindCenterEntityToEntityTreeComposite::getCustomRenderingNearestTime_(double time, uint64_t id) const
{
  // First check the visible flag
  simData::DataStore::Transaction trans;
  auto pref = dataStore_.customRenderingPrefs(id, &trans);
  if ((pref == nullptr) || !pref->commonprefs().draw())
    return -1.0;
  trans.release(&pref);

  auto commands = dataStore_.customRenderingCommandSlice(id);
  if ((commands == nullptr) || (commands->numItems() == 0))
    return -1.0;

  const auto earlierTime = getCustomRenderingEarlierTime_(time, commands);
  const auto laterTime = getCustomRenderingLaterTime_(time, commands);

  if ((earlierTime == -1.0) && (laterTime == -1.0))
    return -1.0;

  if (earlierTime == -1.0)
    return laterTime;

  if (laterTime == -1.0)
    return earlierTime;

  const double previousDelta = time - earlierTime;
  const double nextDelta = laterTime - time;

  return nextDelta < previousDelta ? laterTime : earlierTime;
}

double BindCenterEntityToEntityTreeComposite::getCustomRenderingEarlierTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const
{
  auto iter = slice->upper_bound(searchTime);

  // Custom Render code enforces no repeats on data draw, so this is safe
  while (iter.peekPrevious() != nullptr)
  {
    auto previous = iter.previous();
    if (previous->has_updateprefs() &&
      previous->updateprefs().has_commonprefs() &&
      previous->updateprefs().commonprefs().has_datadraw())
    {
      // If in a valid time range return the search time
      if (previous->updateprefs().commonprefs().datadraw())
        return searchTime;
      // Return the time right before the end of the previous time range
      return previous->time() - TIME_DELTA;
    }
  }

  // did not find a data draw command
  return -1.0;
}

double BindCenterEntityToEntityTreeComposite::getCustomRenderingLaterTime_(double searchTime, const simData::CustomRenderingCommandSlice* slice) const
{
  auto iter = slice->upper_bound(searchTime);

  // Custom Render code enforces no repeats on data draw, so this is safe
  while (iter.peekNext() != nullptr)
  {
    auto next = iter.next();
    if (next->has_updateprefs() &&
      next->updateprefs().has_commonprefs() &&
      next->updateprefs().commonprefs().has_datadraw())
    {
      // Start of a new time range so return its time
      if (next->updateprefs().commonprefs().datadraw())
        return next->time();
      // Turning off so that means the search time was in a valid time range so return the search time
      return searchTime;
    }
  }

  // did not find a data draw command
  return -1.0;
}

double BindCenterEntityToEntityTreeComposite::getBeamNearestTime_(double time, uint64_t id) const
{
  return getNearestTime_(time, id, dataStore_.beamCommandSlice(id), dataStore_.beamUpdateSlice(id));
}

double BindCenterEntityToEntityTreeComposite::getGateNearestTime_(double time, uint64_t id) const
{
  return getNearestTime_(time, id, dataStore_.gateCommandSlice(id), dataStore_.gateUpdateSlice(id));
}

double BindCenterEntityToEntityTreeComposite::getLaserNearestTime_(double time, uint64_t id) const
{
  return getNearestTime_(time, id, dataStore_.laserCommandSlice(id), dataStore_.laserUpdateSlice(id));
}

double BindCenterEntityToEntityTreeComposite::getLobGroupNearestTime_(double time, uint64_t id) const
{
  return getNearestTime_(time, id, dataStore_.lobGroupCommandSlice(id), dataStore_.lobGroupUpdateSlice(id));
}

double BindCenterEntityToEntityTreeComposite::getProjectorNearestTime_(double time, uint64_t id) const
{
  return getNearestTime_(time, id, dataStore_.projectorCommandSlice(id), dataStore_.projectorUpdateSlice(id));
}

template<typename CommandSlice, typename UpdateSlice>
double BindCenterEntityToEntityTreeComposite::getNearestTime_(double time, uint64_t id, const CommandSlice* commands, const UpdateSlice* updates) const
{
  double hostBeginTime;
  double hostEndTime;
  if (hostTimeRange_(id, hostBeginTime, hostEndTime) != 0)
    return -1;

  // Find the times when the entity is on/off
  if ((commands == nullptr) || (commands->numItems() == 0))
    return -1.0;

  std::map<double, bool> drawState;
  auto commandIter = commands->lower_bound(-1.0);
  while (commandIter.peekNext() != nullptr)
  {
    auto next = commandIter.next();
    if (next->has_updateprefs() &&
      next->updateprefs().has_commonprefs() &&
      next->updateprefs().commonprefs().has_datadraw())
    {
      drawState[next->time()] = next->updateprefs().commonprefs().datadraw();
    }
  }

  // Next check data points
  if ((updates == nullptr) || (updates->numItems() == 0))
    return 1;

  double earlierTime = -1.0;
  double laterTime = -1.0;
  auto updateIter = updates->upper_bound(-1.0);
  while (updateIter.peekNext() != nullptr)
  {
    auto next = updateIter.next();
    if (isActive_(next->time(), drawState) && inHostedTimeRange_(next->time(), hostBeginTime, hostEndTime))
    {
      if (next->time() <= time)
        earlierTime = next->time();
      else
      {
        laterTime = next->time();
        break;
      }
    }
  }

  if ((earlierTime == -1.0) && (laterTime == -1.0))
    return -1.0;

  if (earlierTime == -1.0)
    return laterTime;

  if (laterTime == -1.0)
    return earlierTime;

  const double previousDelta = time - earlierTime;
  const double nextDelta = laterTime - time;

  return nextDelta < previousDelta ? laterTime : earlierTime;
}

bool BindCenterEntityToEntityTreeComposite::isActive_(double time, const std::map<double, bool>& drawState) const
{
  if (drawState.empty())
    return false;

  auto it = drawState.upper_bound(time);
  if (it == drawState.begin())
    return false;

  --it;
  return it->second;
}

bool BindCenterEntityToEntityTreeComposite::inHostedTimeRange_(double time, double beginTime, double endTime) const
{
  return ((time >= beginTime) && (time <= endTime));
}

int BindCenterEntityToEntityTreeComposite::hostTimeRange_(uint64_t id, double& beginTime, double& endTime) const
{
  beginTime = -std::numeric_limits<double>::max();
  endTime = std::numeric_limits<double>::max();

  // An entity's life span is limited by its host(s),
  // so walk up the host chain to calculate the life span
  do
  {
    auto type = dataStore_.objectType(id);
    if (type == simData::PLATFORM)
    {
      double begin;
      double end;
      if (platformTimeRange_(id, begin, end) != 0)
        return 1;
      // might need to truncate children
      if (begin > beginTime)
        beginTime = begin;
      if (end < endTime)
        endTime = end;
    }
    else if (type == simData::BEAM)
    {
      double begin;
      double end;
      if (timeRange_(id, begin, end, dataStore_.beamUpdateSlice(id)) != 0)
        return 1;
      // might need to truncate children (gates and projectors)
      if (begin > beginTime)
        beginTime = begin;
      // If not set then beam is the target
      if (endTime == std::numeric_limits<double>::max())
        endTime = end;
    }
    else if (type == simData::GATE)
    {
      if (timeRange_(id, beginTime, endTime, dataStore_.gateUpdateSlice(id)) != 0)
        return 1;
    }
    else if (type == simData::LASER)
    {
      if (timeRange_(id, beginTime, endTime, dataStore_.laserUpdateSlice(id)) != 0)
        return 1;
    }
    else if (type == simData::LOB_GROUP)
    {
      if (timeRange_(id, beginTime, endTime, dataStore_.lobGroupUpdateSlice(id)) != 0)
        return 1;
    }
    else if (type == simData::PROJECTOR)
    {
      if (timeRange_(id, beginTime, endTime, dataStore_.projectorUpdateSlice(id)) != 0)
        return 1;
    }
    else if (type == simData::CUSTOM_RENDERING)
    {
      // Dev error, already handled by other routines
      assert(false);
      return 1;
    }

    id = dataStore_.entityHostId(id);
  } while (id != 0);

  return 0;
}

int BindCenterEntityToEntityTreeComposite::platformTimeRange_(uint64_t id, double& beginTime, double& endTime) const
{
  auto slice = dataStore_.platformUpdateSlice(id);
  if ((slice == nullptr) || (slice->numItems() == 0))
    return 1;

  // Check for static platform
  if (slice->firstTime() == -1)
  {
    // return the bounds of the scenario
    auto bounds = dataStore_.timeBounds(0);
    beginTime = bounds.first;
    endTime = bounds.second;
    return 0;
  }

  beginTime = slice->firstTime();
  endTime = slice->lastTime();
  return 0;
}

template<typename UpdateSlice>
int BindCenterEntityToEntityTreeComposite::timeRange_(uint64_t id, double& beginTime, double& endTime, const UpdateSlice* updates) const
{
  if ((updates == nullptr) || (updates->numItems() == 0))
    return 1;

  beginTime = updates->firstTime();
  endTime = updates->lastTime();
  return 0;
}

}
