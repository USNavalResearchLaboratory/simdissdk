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

#ifdef USE_DEPRECATED_SIMDISSDK_API
CenterEntity::CenterEntity(simVis::FocusManager& focusManager, simVis::ScenarioManager& scenarioManager, EntityTreeComposite& tree)
  : QObject(&tree),
    focusManager_(&focusManager),
    scenarioManager_(&scenarioManager)
{
  bindTo(tree);
}
#endif

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

#ifdef USE_DEPRECATED_SIMDISSDK_API
void CenterEntity::bindTo(EntityTreeComposite& tree)
{
  connect(&tree, SIGNAL(itemDoubleClicked(uint64_t)), this, SLOT(centerOnEntity(uint64_t)));
  tree.setExpandsOnDoubleClick(false); /// Turns off the tree expansion on double click.
}
#endif

void CenterEntity::centerOnEntity(uint64_t id)
{
  // Need the scenario and focus manager to continue
  if (!scenarioManager_ || !focusManager_)
    return;

  simVis::EntityNode* node = getViewCenterableNode(id);
  if ((node != NULL) && node->isActive() && node->isVisible())
  {
    if (focusManager_->getFocusedView() != NULL)
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
    return NULL;

  simVis::EntityNode* node = scenarioManager_->find(id);

  // tetherCamera works only with platforms, custom renderings and gates, so work up the chain until
  // a platform, custom rendering or gate is found.
  while ((node != NULL) && (dynamic_cast<simVis::PlatformNode*>(node) == NULL) &&
    (dynamic_cast<simVis::GateNode*>(node) == NULL) &&
    (dynamic_cast<simVis::CustomRenderingNode*>(node) == NULL))
  {
    uint64_t parentId;
    if (node->getHostId(parentId))
      node = scenarioManager_->find(parentId);
    else
      node = NULL;
  }

  return node;
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
    if ((node == NULL) || (node->isActive() && !node->isVisible()))
    {
      tree_.setUseCenterAction(false, tr("Inactive entity selected"));
      return;
    }

    // If there is one selected platform look for a time to make the center command valid
    if (!node->isActive())
    {
      // If more than one entity is selected don't try to find a time where all are active
      if (ids.size() != 1)
      {
        tree_.setUseCenterAction(false, tr("Inactive entity selected"));
        return;
      }

      // Make sure time controls are enabled and that the scenario is in file mode
      if ((dataStore_.getBoundClock() == NULL) || dataStore_.getBoundClock()->controlsDisabled() || dataStore_.getBoundClock()->isLiveMode())
      {
        tree_.setUseCenterAction(false, tr("Inactive entity selected"));
        return;
      }

      auto type = dataStore_.objectType(ids.front());
      if (type == simData::PLATFORM)
      {
        newTime_ = getPlatformNearestTime_(ids.front());
        if (newTime_ == -1.0)
        {
          tree_.setUseCenterAction(false, tr("Inactive entity selected"));
          return;
        }
      }
      else if (type == simData::CUSTOM_RENDERING)
      {
        newTime_ = getCustomRenderingNearestTime_(ids.front());
        if (newTime_ == -1.0)
        {
          tree_.setUseCenterAction(false, tr("Inactive entity selected"));
          return;
        }
      }
      else
      {
        tree_.setUseCenterAction(false, tr("Inactive entity selected"));
        return;
      }
    }
  }

  QString message;
  if (newTime_ != -1.0)
  {
    message = "Time ";
    int referenceYear = dataStore_.referenceYear();
    const simCore::TimeStamp time(referenceYear, newTime_);
    message += QString::fromStdString(timeFormatter_->toString(timeFormat_, time, referenceYear, precision_));
  }
  tree_.setUseCenterAction(true, message);
}

void BindCenterEntityToEntityTreeComposite::centerOnEntity_(uint64_t id)
{
  if ((newTime_ != -1.0) && (dataStore_.getBoundClock() != NULL) && !dataStore_.getBoundClock()->controlsDisabled() && !dataStore_.getBoundClock()->isLiveMode())
    dataStore_.getBoundClock()->setTime(simCore::TimeStamp(dataStore_.referenceYear(), newTime_));

  centerEntity_.centerOnEntity(id);
}

double BindCenterEntityToEntityTreeComposite::getPlatformNearestTime_(uint64_t id) const
{
  // First check the visible flag
  simData::DataStore::Transaction trans;
  auto pref = dataStore_.platformPrefs(id, &trans);
  if ((pref == NULL) || !pref->commonprefs().draw() || !pref->commonprefs().datadraw())
    return -1.0;
  trans.release(&pref);

  // Next check data points
  auto slice = dataStore_.platformUpdateSlice(id);
  if ((slice == NULL) || (slice->numItems() == 0))
    return -1.0;

  const auto time = dataStore_.updateTime();
  auto iter = slice->upper_bound(time);

  // Since there is a check above for at least one point, previous or next must be set

  if (iter.peekNext() == NULL)
    return iter.peekPrevious()->time();

  if (iter.peekPrevious() == NULL)
    return iter.peekNext()->time();

  const double nextDelta = iter.peekNext()->time() - time;
  const double previousDelta = time - iter.peekPrevious()->time();

  return nextDelta < previousDelta ? iter.peekNext()->time() : iter.peekPrevious()->time();
}

double BindCenterEntityToEntityTreeComposite::getCustomRenderingNearestTime_(uint64_t id) const
{
  // First check the visible flag
  simData::DataStore::Transaction trans;
  auto pref = dataStore_.customRenderingPrefs(id, &trans);
  if ((pref == NULL) || !pref->commonprefs().draw())
    return -1.0;
  trans.release(&pref);

  auto commands = dataStore_.customRenderingCommandSlice(id);
  if ((commands == NULL) || (commands->numItems() == 0))
    return -1.0;

  const auto time = dataStore_.updateTime();
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
  while (iter.peekPrevious() != NULL)
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
  while (iter.peekNext() != NULL)
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

}
